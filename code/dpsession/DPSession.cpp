#include "roomlib.h"
#include "AES.h"
#include "dpmsg.h"
#include "dpxml.h"
#include "dpgpio.h"
#include "dpsession.h"
#include "PhoneApp.h"

//#define AUTO_CALLOUT
//#define AUTO_HANGUP
//#define AUTO_VIDEO

#define DP_PORT				6600
#define USER_PORT			6606

#define MAX_PACKET_BUF		4096
#define DPMSGQ			"__DPMsgQueue"		

typedef struct
{
	int sessionId;
	int remoteIp;
	DPPacket* pPacket;
}DPSessionMsg;

typedef struct
{
	DWORD cmd;
	DWORD id;
	DWORD wParam;
}DPUserData;

static BYTE prtl_initkey[AES_BLOCK_SIZE] =
{
	0xf8,0x0c,0x43,0x02,0x1b,0x65,0x1a,0xe1,
	0xa7,0x18,0xca,0x2a,0x0c,0x21,0x79,0xe5
};

static BYTE prtl_setkey[AES_BLOCK_SIZE] = 
{
	0x2c,0xde,0x02,0xe6,0xf9,0x48,0xaf,0xae,
	0x1a,0xb4,0x57,0x2b,0x7c,0xcb,0x4a,0x91
};

const DWORD TimeOut[] = 
{
	1000,	// 呼叫出去，等待对方回复
	2000,	// 等待对方回复呼叫
	300000,	// 响铃，上层应用处理
	300000	// 通话超时，上层应用处理
};

static DWORD g_dwMsgID = 0;
static DWORD g_dwSessionId = 1000;
static SOCKET g_pSock[2];
static SOCKET g_sockUser;
static BOOL g_bRun = FALSE;
static AES* g_pAes = NULL;
static HANDLE g_hSession = NULL;
static HANDLE g_hSendThread = NULL;
static HANDLE g_hSendMsgQ = NULL;
static HANDLE g_hWaitAck = NULL;
static HANDLE g_hTalkSemap = NULL;
static DWORD g_dwCurMsgID = 0;			// 当前正在发送的消息ID
static DPMedia* g_pMedia;	
static char* g_pPacketBuf = NULL;
static DPSession* g_pSession = NULL;

static StaticLock g_SessionCS;
static StaticLock g_SockCS;

static DPPacket* BuildPacket(DWORD cmd, int lid, int rid, int msgid)
{
	DPPacket* pPacket = (DPPacket*)g_pPacketBuf;
	pPacket->head.check = DP_CHECK_ID;
	pPacket->head.msgid = msgid;

	BuildPacketXml(pPacket->pContent, cmd, lid, rid, msgid, g_pSession);
	pPacket->length = strlen(pPacket->pContent) + 1 + sizeof(DWORD);

	AES_SetEncInitVec(g_pAes, prtl_setkey);
	AES_DataEncrypt(g_pAes, (BYTE*)pPacket->pContent, (BYTE*)pPacket->pContent, pPacket->length - sizeof(DWORD));

	return pPacket;
}

static void UpdateMediaInfo(int calltype)
{
	// 户户通话为 480*360 其他为 640*480
	if(calltype == ROOM_TYPE)
	{
		g_pMedia->video.width = 480;
		g_pMedia->video.height = 360;
	}
	else
	{
		g_pMedia->video.width = 640;
		g_pMedia->video.height = 480;
	}
}

static void SessionWriteMsg(DPPacket* pPacket, int remoteip, int id, BOOL bDirect)
{
	if(bDirect)
	{
		// 直接发送，不写进消息队列
		g_SockCS.lockon();
		UdpSend(g_pSock[0], remoteip, DP_PORT, (char*)pPacket, pPacket->length + sizeof(DPPacketHead));	
		g_SockCS.lockoff();
	}
	else
	{
		DPSessionMsg msg;
		msg.sessionId = id;
		msg.remoteIp = remoteip;
		msg.pPacket = (DPPacket*)malloc(pPacket->length + sizeof(DPPacketHead));
		memcpy(msg.pPacket, pPacket, pPacket->length + sizeof(DPPacketHead));

		if(DPWriteMsgQueue(g_hSendMsgQ, &msg, sizeof(DPSessionMsg), 0) == FALSE)
		{
			// 写消息失败，超出范围？
			free(msg.pPacket);
		}
	}
}

static void HangUp()
{
	if(g_pSession)
	{
		DPTransLiuyanStop();
		DPTransAudioStop();
		DPTransVideoStop();
		delete g_pSession;
		g_pSession = NULL;
	}
}

static void HandleNewCallIn(char* pContent, int remoteip, int msgid)
{
	int lid;
	char number[16];
	DPMedia rmedia;
	if(GetXmlString(pContent, "Number", number, sizeof(number))
		&& GetXmlNumber(pContent, "lid", &lid)
		&& GetXmlMediaInfo(&rmedia, pContent))
	{
		if(!RequestTalk())
		{
			DPPacket* pPacket = BuildPacket(DPMSG_BUSY, 0, lid, msgid);
			SessionWriteMsg(pPacket, remoteip, 0, TRUE);
		}
		else
		{
			HangUp();

			DWORD curtick = DPGetTickCount();

			g_pSession = new DPSession;
			g_pSession->status = DPCALL_RING;
			strcpy(g_pSession->dst_number, number);
			g_pSession->remoteip = remoteip;
			g_pSession->lid = GetSessionId();
			g_pSession->rid = lid;
			g_pSession->starttick = curtick;
			g_pSession->heartSend = curtick - 2000;
			g_pSession->heartRecv = curtick;
			g_pSession->bCallOut = FALSE;
			g_pSession->calltype = g_pSession->dst_number[0] - '0';
			memcpy(&g_pSession->rmedia, &rmedia, sizeof(DPMedia));

			DPPacket* pPacket = BuildPacket(DPMSG_IDLE, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
			SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, FALSE);

			DPPostMessage(MSG_EXTSTART_APP, TALKING_APPID, GetCallInPkt(remoteip, number, g_pSession->lid), 0);
		}
	}
}

static void HandleBusy(char* pContent)
{
	int lid;
	if(g_pSession)
	{
		if((GetXmlNumber(pContent, "rid", &lid)) 
			&& (lid == g_pSession->lid))
		{
			DBGMSG(DPINFO, "DPSession Recv Busy!\r\n");

			HangUp();
			DPPostMessage(MSG_PHONECALL, DPMSG_BUSY, 0, 0);
		}
	}
}

static void HandleIdle(char* pContent)
{
	int lid, rid;
	if(g_pSession)
	{
		if(GetXmlNumber(pContent, "rid", &lid)
			&& (lid == g_pSession->lid)
			&& GetXmlNumber(pContent, "lid", &rid))
		{
			DBGMSG(DPINFO, "DPSession Recv Idle!\r\n");

			g_pSession->starttick = DPGetTickCount();
			g_pSession->status = DPCALL_RING;
			g_pSession->rid = rid;
			DPPostMessage(MSG_PHONECALL, DPMSG_IDLE, 0, 0);
		}
	}
}

static void HandleAccept(char* pContent)
{
	DPMedia rmedia;
	if(CompareXmlID(pContent, g_pSession)
		&& GetXmlMediaInfo(&rmedia, pContent))
	{
		DBGMSG(DPINFO, "DPSession Recv Accept!\r\n");
		memcpy(&g_pSession->rmedia, &rmedia, sizeof(DPMedia));

		g_pSession->starttick = DPGetTickCount();
		g_pSession->status = DPCALL_TALK;

		if(g_pSession->calltype == MONITOR_TYPE)
		{
			DPTransVideoStart(g_pSession->rmedia.video.addr.ip, g_pSession->rmedia.video.addr.port, g_pSession->rmedia.video.addr.cport, g_pMedia->video.addr.port, g_pMedia->video.addr.cport, TRUE);
		}
		else
		{
			DPTransVideoStart(g_pSession->rmedia.video.addr.ip, g_pSession->rmedia.video.addr.port, g_pSession->rmedia.video.addr.cport, g_pMedia->video.addr.port, g_pMedia->video.addr.cport, FALSE);
			DPTransAudioStart(g_pSession->rmedia.audio.addr.ip, g_pSession->rmedia.audio.addr.port, g_pMedia->audio.addr.port);
		}
		DPPostMessage(MSG_PHONECALL, DPMSG_ACCEPT, 0, 0);
	}
}

static void HandleHangUp(char* pContent)
{
	if(CompareXmlID(pContent, g_pSession))
	{
		DBGMSG(DPINFO, "DPSession Recv HangUp!\r\n");

		DPPacket* pPacket = BuildPacket(DPMSG_HANGUP, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
		SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, TRUE);

		HangUp();
		DPPostMessage(MSG_PHONECALL, DPMSG_HANGUP, 0, 0);
	}
}

static void HandleInfo(char* pContent)
{

}

static void HandleHeartBeat(char* pContent)
{
	if(CompareXmlID(pContent, g_pSession))
	{
		g_pSession->heartRecv = DPGetTickCount();
	}
}

static void RecvDataProc(char* pContent, int remoteip, int msgid)
{
	int cmd = ParseCmd(pContent);
	switch(cmd)
	{
		case DPMSG_NEWCALLIN:
			HandleNewCallIn(pContent, remoteip, msgid);
			break;
		case DPMSG_BUSY:
			HandleBusy(pContent);
			break;
		case DPMSG_IDLE:
			HandleIdle(pContent);
			break;
		case DPMSG_ACCEPT:
			HandleAccept(pContent);
			break;
		case DPMSG_HANGUP:
			HandleHangUp(pContent);
			break;
		case DPMSG_INFO:
			HandleInfo(pContent);
			break;
		case DPMSG_HEARTBEAT:
			HandleHeartBeat(pContent);
			break;
		case DPMSG_NONE:
			break;
	}
}

static void DoCallOut(PhonePkt* pPkt)
{
	DBGMSG(DPINFO, "DoUserOp CallOut %x\r\n", pPkt->ip);
	DWORD curtick = DPGetTickCount();

	g_pSession = new DPSession;
	g_pSession->status = DPCALL_WAIT;
	strcpy(g_pSession->dst_number, pPkt->code);
	g_pSession->remoteip = pPkt->ip;
	g_pSession->lid = pPkt->dwSessionId;
	g_pSession->rid = 0;
	g_pSession->starttick = curtick;
	g_pSession->heartSend = curtick - 2000;
	g_pSession->heartRecv = curtick;
	g_pSession->bCallOut = TRUE;
	g_pSession->calltype = pPkt->code[0] - '0';
	g_pSession->calltype = g_pSession->dst_number[0] - '0';
	UpdateMediaInfo(g_pSession->calltype);

	DPPacket* pPacket = BuildPacket(DPMSG_CALL, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
	SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, FALSE);
}

static void DoMonitor(PhonePkt* pPkt)
{
	if(g_pSession == NULL)
	{
		DBGMSG(DPINFO, "DoUserOp Monitor\r\n");
		DWORD curtick = DPGetTickCount();

		g_pSession = new DPSession;
		g_pSession->status = DPCALL_WAIT;
		strcpy(g_pSession->dst_number, pPkt->code);
		g_pSession->remoteip = pPkt->ip;
		g_pSession->lid = pPkt->dwSessionId;
		g_pSession->rid = 0;
		g_pSession->starttick = curtick;
		g_pSession->heartSend = curtick - 2000;
		g_pSession->heartRecv = curtick;
		g_pSession->bCallOut = TRUE;
		g_pSession->calltype = MONITOR_TYPE;

		DPPacket* pPacket = BuildPacket(DPMSG_MONITOR, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
		SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, FALSE);
	}
}

static void DoAccept(int id)
{
	DBGMSG(DPINFO, "DoUserOp Accept %d\r\n", id);
	if(g_pSession
		&& (id == g_pSession->lid)
		&& (!g_pSession->bCallOut)
		&& (g_pSession->status == DPCALL_RING))
	{
		UpdateMediaInfo(g_pSession->calltype);
		DPPacket* pPacket = BuildPacket(DPMSG_ACCEPT, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
		SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, FALSE);

		g_pSession->starttick = DPGetTickCount();
		g_pSession->status = DPCALL_TALK;
		DPTransVideoStart(g_pSession->rmedia.video.addr.ip, g_pSession->rmedia.video.addr.port, g_pSession->rmedia.video.addr.cport, g_pMedia->video.addr.port, g_pMedia->video.addr.cport, FALSE);
		DPTransAudioStart(g_pSession->rmedia.audio.addr.ip, g_pSession->rmedia.audio.addr.port, g_pMedia->audio.addr.port);
	}
}

static void DoHangup(int id)
{
	DBGMSG(DPINFO, "DoUserOp Hangup %d\r\n", id);
	if(g_pSession
		&& (id == g_pSession->lid))
	{
		DPPacket* pPacket = BuildPacket(DPMSG_HANGUP, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
		SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, TRUE);

		HangUp();
	}
}

static void DoLiuYan(int id)
{
	DBGMSG(DPINFO, "DoUserOp LiuYan %d\r\n", id);
	if(g_pSession
		&& (id == g_pSession->lid))
	{
		DPPacket* pPacket = BuildPacket(DPMSG_ACCEPT, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
		SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, TRUE);

		DPTransLiuyanStart(g_pSession->rmedia.audio.addr.ip, g_pSession->rmedia.audio.addr.port, g_pMedia->audio.addr.port);
	}
}

static void DoStartVideo(int id)
{
	DBGMSG(DPINFO, "DoUserOp StartVideo %d\r\n", id);
	if(g_pSession
		&& (id == g_pSession->lid))
	{
		switch(g_pSession->calltype)
		{
		case CELL_DOOR_TYPE:
		case SECOND_DOOR_TYPE:
		case ZONE_DOOR_TYPE:
		case AREA_DOOR_TYPE:
			DPTransVideoStart(g_pSession->rmedia.video.addr.ip, g_pSession->rmedia.video.addr.port, g_pSession->rmedia.video.addr.cport, g_pMedia->video.addr.port, g_pMedia->video.addr.cport, TRUE);
			break;
		default:
			DBGMSG(DPINFO, "DoUserOp StartVideo fail:%s\r\n", g_pSession->dst_number);
			break;
		}
	}
}

static void DoOffLine(int id)
{
	DBGMSG(DPINFO, "DoUserOp OffLine %d\r\n", id);
	if(g_pSession
		&& (id == g_pSession->lid))
	{
		DPPacket* pPacket = BuildPacket(DPMSG_HANGUP, g_pSession->lid, g_pSession->rid, g_dwMsgID++);
		SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, TRUE);

		HangUp();
		DPPostMessage(MSG_PHONECALL, DPMSG_OFFLINE, 0, 0);
	}
}

static void TimeOutOp()
{
	if(g_pSession == NULL)
		return;

	DWORD curtick = DPGetTickCount();
	if(curtick - g_pSession->heartSend > 4000)
	{
		// 4秒发送一次心跳包
		g_pSession->heartSend = curtick;

		DPPacket* pPacket = BuildPacket(DPMSG_HEARTBEAT, g_pSession->lid, g_pSession->rid, 0);
		SessionWriteMsg(pPacket, g_pSession->remoteip, g_pSession->lid, FALSE);

		DBGMSG(DPINFO, "DoTimeOut ip:%x, status:%d, lid:%d, rid:%d, starttick:%d, heartrecv:%d, curtick:%d\r\n", 
			g_pSession->remoteip, g_pSession->status, g_pSession->lid, g_pSession->rid, g_pSession->starttick, g_pSession->heartRecv, curtick);
	}

	if(curtick - g_pSession->heartRecv > 5000)
	{
		// 超过5秒没有收到对方的心跳包
		DoOffLine(g_pSession->lid);
		return;
	}

	if(curtick - g_pSession->starttick > TimeOut[g_pSession->status])
	{
		switch(g_pSession->status)
		{
		case DPCALL_WAIT:
			DPPostMessage(MSG_PHONECALL, DPMSG_OFFLINE, 0, 0);
			break;
		case DPCALL_RING:
			// 上层应用判断超时
			break;
		case DPCALL_TALK:
			// 上层应用判断超时
			break;
		}

		HangUp();
	}
}

static void UserOp()
{
	DPUserData data;
	int nlen = UdpRecv(g_pSock[1], (char*)&data, sizeof(DPUserData), 0, NULL);
	if(nlen == sizeof(DPUserData))
	{
		switch(data.cmd)
		{
			case DPMSG_CALL:
				DoCallOut((PhonePkt*)data.wParam);
				break;
			case DPMSG_MONITOR:
				DoMonitor((PhonePkt*)data.wParam);
				break;
			case DPMSG_ACCEPT:
				DoAccept(data.id);
				break;
			case DPMSG_HANGUP:
				DoHangup(data.id);
				break;
			case DPMSG_LIUYAN:
				DoLiuYan(data.id);
				break;
			case DPMSG_START_VIDEO:
				DoStartVideo(data.id);
				break;
			case DPMSG_OFFLINE:
				DoOffLine(data.id);
				break;
			case DPMSG_IP_CHANGE:
				g_pMedia->video.addr.ip = data.wParam;
				g_pMedia->audio.addr.ip = data.wParam;
				break;
		}
	}
}

static void SessionOp()
{
	int remoteip;
	int nlen = UdpRecv(g_pSock[0], g_pPacketBuf, 4096, 0, &remoteip);
	if(nlen >= sizeof(DPPacket))
	{
		DPPacket *pPacket = (DPPacket*)g_pPacketBuf;
		if(pPacket->head.check == 0xabcd1235)
		{
			// 对方命令回复
			if(pPacket->head.msgid == g_dwCurMsgID)
			{
				g_dwCurMsgID = 0;
				DPSetSemaphore(g_hWaitAck);
			}
			return;
		}

		if(pPacket->head.check != DP_CHECK_ID)
		{
			DBGMSG(DPINFO, "TransThread Recv Packet check:%x fail\r\n", pPacket->head.check);
			return;
		}

		if(pPacket->length != nlen - sizeof(DPPacketHead))
		{
			DBGMSG(DPINFO, "TransThread Recv Packet length:%x fail\r\n", pPacket->length);
			return;
		}

		//回复ACK消息
		DPPacket ack;
		ack.head.check = 0xabcd1235;
		ack.head.msgid = pPacket->head.msgid;
		g_SockCS.lockon();
		UdpSend(g_pSock[0], remoteip, DP_PORT, (char*)&ack, sizeof(DPPacket));
		g_SockCS.lockoff();

		AES_SetDecInitVec(g_pAes, prtl_setkey);
		AES_DataDecrypt(g_pAes, (BYTE*)pPacket->pContent, (BYTE*)pPacket->pContent, nlen);

//#ifdef _DEBUG
//		printf("%s\r\n", pPacket->pContent);
//#endif
		RecvDataProc(pPacket->pContent, remoteip, pPacket->head.msgid);
	}
	else if(nlen > 0)
	{
		DBGMSG(DPINFO, "Session Recv Unknow Data len:%d, ip:%x\r\n", nlen, remoteip);
	}
}

static DWORD SessionThread(HANDLE pParam)
{
	DBGMSG(DPINFO, "SessionThread Start\r\n");

	DWORD dwTimeOut;
	while(g_bRun)
	{
		if(g_pSession)
			dwTimeOut = 1000;
		else
			dwTimeOut = 10000000;

		int ret = SocketSelect(g_pSock, 2, dwTimeOut);
		if(ret == -1)
		{
			DBGMSG(DPINFO, "SessionThread SocketSelect fail:%d, sock[0]:%d, sock[1]:%d\r\n", DPGetLastError(), g_pSock[0], g_pSock[1]);
			break;
		}
		else if(ret == 0)
		{
			TimeOutOp();
		}
		else if(ret == 1)
		{
			SessionOp();
		}
		else if(ret == 2)
		{
			UserOp();
		}
	}

	DBGMSG(DPINFO, "SessionThread Stop\r\n");
	return 0;
}

static DWORD MsgSendThread(HANDLE pParam)
{
	DBGMSG(DPINFO, "Session MsgSendThread Start\r\n");

	HANDLE hRecvMsgQ = pParam;
	DPSessionMsg msg = {0};
	DWORD dwTimes = 0;
	DWORD length = 0;

	char buffer[1024];

	while(g_bRun)
	{
		if(msg.pPacket)
		{
			g_SockCS.lockon();
			UdpSend(g_pSock[0], msg.remoteIp, DP_PORT, (char*)msg.pPacket, length);
			g_SockCS.lockoff();
			// 等待对方回复
			if(DPGetSemaphore(g_hWaitAck, 1000))
			{
				// 收到回复，马上删除
				free(msg.pPacket);
				msg.pPacket = NULL;
			}
			else
			{
				dwTimes++;
				if(dwTimes == 3)
				{
#if 0
					AES_SetDecInitVec(g_pAes, prtl_setkey);
					AES_DataDecrypt(g_pAes, (BYTE*)msg.pPacket->pContent, (BYTE*)buffer, 10);
					buffer[10] = 0;
					DBGMSG(DPINFO, "Session Msg UnAck OffLine msg:%s, id:%d, unack:%d\r\n", buffer, msg.sessionId, dwTimes + 1);
#endif
					// 发送3次，没有回复, 则判断出对方不在线，结束通话
					free(msg.pPacket);
					msg.pPacket = NULL;
					DBGMSG(DPINFO, "Session Msg UnAck id:%d\r\n", msg.sessionId);
					DPSessionUserOp(DPMSG_OFFLINE, msg.sessionId, 0);
				}
			}
		}
		else
		{
			// 阻塞等待消息发送
			if(DPReadMsgQueue(hRecvMsgQ, &msg, sizeof(DPSessionMsg), INFINITE))
			{
				// 收到消息
				dwTimes = 0;
				DPGetSemaphore(g_hWaitAck, 0);	// 消除信号
				if((g_pSession == NULL)
					|| (msg.sessionId < g_dwSessionId))
				{
					// 非当前SessionID，则不用再发送这个消息
					free(msg.pPacket);
					msg.pPacket = NULL;
				}
				else
				{
					g_dwCurMsgID = msg.pPacket->head.msgid;
					length = msg.pPacket->length + sizeof(DPPacketHead);
				}
			}
		}
	}

	DPCloseMsgQueue(hRecvMsgQ);
	DBGMSG(DPINFO, "Session MsgSendThread Stop\r\n");
	return 0;
}

void InitMedia(int ip)
{
	if(g_pMedia == NULL)
	{
		g_pMedia = (DPMedia*)malloc(sizeof(DPMedia));

		// Video
		g_pMedia->video.addr.ip = ip;
		g_pMedia->video.addr.port = 15000;
		g_pMedia->video.addr.cport = 15001;
		g_pMedia->video.enc_type = ENCODE_H264;
		g_pMedia->video.trans_type = TRANS_TYPE_SEND | TRANS_TYPE_RECV;
		// 户户通话为 480*360 其他为 640*480	UpdateMediaInfo
		g_pMedia->video.width = 640;
		g_pMedia->video.height = 480;
		g_pMedia->video.bitrate = 1024 * 1024;

		// Audio
		g_pMedia->audio.addr.ip = ip;
		g_pMedia->audio.addr.port = 15100;
		g_pMedia->audio.addr.cport = 15101;
		g_pMedia->audio.enc_type = 3;
		g_pMedia->audio.trans_type = TRANS_TYPE_SEND | TRANS_TYPE_RECV;
	}
}

BOOL RequestTalk()
{
	return DPGetSemaphore(g_hTalkSemap, 0);
}

void ReleaseTalk()
{
	DPSetSemaphore(g_hTalkSemap);
}

DWORD GetSessionId()
{
	DWORD id = 0;
	g_SessionCS.lockon();
	g_dwSessionId++;
	if(g_dwSessionId > 0xFFFFFFF0)
		g_dwSessionId = 1000;
	id = g_dwSessionId;
	g_SessionCS.lockoff();
	return id;
}

void DPSessionUserOp(DWORD cmd, int id, DWORD wParam)
{
	g_SessionCS.lockon();
	DPUserData data;
	data.cmd = cmd;
	data.id = id;
	data.wParam = wParam;
	UdpSend(g_sockUser, "127.0.0.1", USER_PORT, (char*)&data, sizeof(DPUserData));
	g_SessionCS.lockoff();
}

#ifdef AUTO_CALLOUT
DWORD AutoCallOut(HANDLE pParam)
{
	PhonePkt pkt;
	while(1)
	{
		printf("=============================================\r\n");
		strcpy(pkt.code, "1010101010101");
		pkt.ip = inet_addr("192.168.1.5");
		pkt.dwSessionId = GetSessionId();
		DPSessionUserOp(DPMSG_CALL, 0, (DWORD)&pkt);
		DPSleep(15000);
	}
	return 0;
}
#endif

#ifdef AUTO_HANGUP
extern void AutoPostMsg(BOOL bAccept);
DWORD AutoHangUp(HANDLE pParam)
{
	while(1)
	{
		DPSleep(1000);
		if(g_pSession)
		{
			DWORD tick = DPGetTickCount() - g_pSession->starttick;
			if(g_pSession->status == DPCALL_RING
				&& (tick >= 2000 && tick <= 3000))
			{
				AutoPostMsg(TRUE);
			}

			if(g_pSession->status == DPCALL_TALK
				&& (tick >= 5000 && tick <= 6000))
			{
				AutoPostMsg(FALSE);
			}
		}
	}
	return 0;
}
#endif

#ifdef AUTO_VIDEO
DWORD AutoVideo(HANDLE pParam)
{
	while(1)
	{
		DPTransVideoStart(inet_addr("127.0.0.1"), 15000, 15001, 15000, 15001, FALSE);
		DPSleep(5000);
		DPTransVideoStop();
		DPSleep(1000);
	}
	return 0;
}
#endif

void DPSessionInit(int ip)
{
	DBGMSG(DPINFO, "DPSessionInit Start\r\n");
	g_SessionCS.lockon();
	if(!g_bRun)
	{
		g_pAes = AES_New();
		AES_SetMode(g_pAes, AES_MODE_SIC);
		AES_SetKey(g_pAes, prtl_initkey, 128);
		if(g_pAes == NULL)
		{
			DBGMSG(DPERROR, "AES_New Fail!\r\n");
			g_SessionCS.lockoff();
			return;
		}

		if(g_hTalkSemap == NULL)
			g_hTalkSemap = DPCreateSemaphore(1, 1);

		InitMedia(ip);
		InitXmlServer(g_pMedia);

		g_bRun = TRUE;
		g_pPacketBuf = (char*)malloc(MAX_PACKET_BUF);
		g_pSock[0] = UdpCreate(DP_PORT);
		g_pSock[1] = UdpCreate(USER_PORT);
		g_sockUser = UdpCreate();
		if(g_hWaitAck == NULL)
			g_hWaitAck = DPCreateSemaphore(0, 1);

		HANDLE hRecvQ;
		DPCreateMsgQueue(DPMSGQ, 512, sizeof(DPSessionMsg), &hRecvQ, &g_hSendMsgQ);
		g_hSession = DPThreadCreate(0x4000, SessionThread, NULL, TRUE, 5);
		g_hSendThread = DPThreadCreate(0x4000, MsgSendThread, hRecvQ, TRUE, 5);

#ifdef AUTO_CALLOUT
		DPThreadCreate(0x4000, AutoCallOut, NULL, FALSE, 5);
#endif
#ifdef AUTO_HANGUP
		DPThreadCreate(0x4000, AutoHangUp, NULL, FALSE, 5);
#endif
#ifdef AUTO_VIDEO
		DPThreadCreate(0x4000, AutoVideo, NULL, FALSE, 5);
#endif

	}
	g_SessionCS.lockoff();
}

void DPSessionUninit()
{
	DBGMSG(DPINFO, "DPSessionUninit Start\r\n");
	g_SessionCS.lockon();
	if(g_bRun)
	{
		g_bRun = FALSE;
		DPSessionUserOp(DPMSG_QUIT, 0);
		DPThreadJoin(g_hSession);
		SocketClose(g_pSock[0]);
		g_pSock[0] = INVALID_SOCKET;
		SocketClose(g_pSock[1]);
		g_pSock[1] = INVALID_SOCKET;
		SocketClose(g_sockUser);
		g_sockUser = INVALID_SOCKET;
		DPCloseMsgQueue(g_hSendMsgQ);
		free(g_pPacketBuf);
		AES_Free(g_pAes);
		UnInitXmlServer();
	}
	g_SessionCS.lockoff();
	DBGMSG(DPINFO, "DPSessionUninit Stop\r\n");
}