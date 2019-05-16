#include "roomlib.h"
#include "AES.h"
#include "dpvideo.h"
#include "dpgpio.h"
#include "dpmsg.h"

typedef struct
{
	BYTE seq;			// 帧序列号
	BYTE type;			// 帧类型 1 表示 IFrame
	BYTE pdata[];		// 帧数据
}DPFramePacket;			// 

typedef	struct
{
	WORD type;			// 0x02 丢包重传请求
	WORD seq_no;		// 丢包序号
}DPCtrlPacket;			// 

#define	MSGTYPE_RESEND			0x02	// 通知对方有丢包出现，需要重传
#define	MSGTYPE_KEYFRAME		0x03	// 通知对方需要关键帧

#define _FRAME_HEAD_LEN			2		// sizeof(DPFramePacket)
#define _FRAME_PACKET_LEN		2788	// _PACKET_LEN - _PACKET_HEAD_LEN
#define _FRAME_MAX_LEN			0x20000

static BYTE INIT_KEY[AES_BLOCK_SIZE] =
{
	0xf8,0x0c,0x43,0xa2,0x1b,0x85,0x1a,0xe1,
	0xa7,0x18,0xca,0x2a,0x0c,0x21,0x79,0xe5
};

static BYTE SET_KEY[AES_BLOCK_SIZE] = 
{
	0x8c,0xde,0x02,0x16,0xf9,0x48,0xaf,0x1e,
	0x1a,0x24,0x57,0x2b,0x7c,0xcb,0x4a,0x11
};

static HANDLE g_hRecvThread = NULL;
static HANDLE g_hSendThread = NULL;
static HANDLE g_hCtrlThread = NULL;
static AES* g_pAes = NULL;
static BOOL g_bVTransRun = FALSE;
static BOOL g_bVideoEnable = FALSE;
static StaticLock g_VideoCS;

static DWORD g_ip = 0;
static DWORD g_lcport = 0;
static SOCKET g_sockfd = INVALID_SOCKET;
static SOCKET g_ctrlSock = INVALID_SOCKET;

static BYTE* g_pIFrameBuf = NULL;
static DWORD g_dwIFrameLen = 0;
static BOOL g_bGetIFrame = FALSE;
static StaticLock g_IFrameCS;

static int CheckFrameType(BYTE* pdata, int len)
{
	DWORD flag = 0;
	BYTE* p = pdata;
	while(p < pdata + len)
	{
		flag = (flag << 8) | (*p++);
		if(flag == 0x00000167)
			return I_VOP;
		else if(flag == 0x00000127)
			return I_VOP;
		else if(flag == 0x141)
			return P_VOP;			// PFrame
		else if(flag == 0x121)
			return P_VOP;			// PFrame
	}
	return P_VOP + 1;
}

static DWORD CtrlThread(HANDLE pParam)
{
	// 对方丢帧，马上编I帧发送过去
	DPCtrlPacket packet = {0};
	int remoteip = 0;

	DBGMSG(DPINFO, "CtrlThread start\r\n");
	while(g_bVTransRun)
	{
		int ret = UdpRecv(g_ctrlSock, (char*)&packet, sizeof(DPCtrlPacket), 100000, &remoteip);
		if(ret == sizeof(DPCtrlPacket))
		{
			switch(packet.type)
			{
			case MSGTYPE_KEYFRAME:
				g_bGetIFrame = TRUE;
				printf("MSGTYPE_KEYFRAME\r\n");
				break;
			case MSGTYPE_RESEND:
				//printf("MSGTYPE_RESEND\r\n");
				break;
			default:
				printf("CtrlThread recv Unknow\r\n");
				break;
			}
		}
	}
	DBGMSG(DPINFO, "CtrlThread end\r\n");
	return 0;
}

static DWORD SendThread(HANDLE pParam)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((int)pParam);
	addr.sin_addr.s_addr = g_ip;

	DWORD dwQuality = 10;
	DWORD type;

	HANDLE hVideoEnc = VideoEncStart(ENCODE_MP4, 720, 576, dwQuality);
	if(hVideoEnc == INVALID_HANDLE_VALUE)
		return 0;

	char buf[_PACKET_LEN];
	BYTE *pFrameBuf = (BYTE*)malloc(0x10000);
	DWORD dwFrameLen = 0;

	DPTransPacket *pPacket = (DPTransPacket*)buf;
	DPFramePacket *pFramePacket = (DPFramePacket*)pFrameBuf;

	WORD seq_no = 0;
	BYTE frame_seq_no = 0;

	DWORD dwBitrate = 0;
	DWORD dwCount = 0;

	DBGMSG(DPINFO, "SendThread start\r\n");
	BOOL bLastEnable = TRUE;
	while(g_bVTransRun)
	{
		if(g_bVideoEnable != bLastEnable)
		{
			bLastEnable = g_bVideoEnable;
			VideoEncEnable(hVideoEnc, bLastEnable);
		}

		if(bLastEnable && g_bGetIFrame)
		{
			dwBitrate = 0;
			dwCount = 0;
			while(1)
			{
				type = P_VOP;
				dwFrameLen = VideoEncRead(hVideoEnc, pFrameBuf + _FRAME_HEAD_LEN, 0xFFFE, &type);
				if(type == I_VOP && dwFrameLen > 0)
				{
					// 发送完毕再把 g_bGetIFrame 置为 FALSE
					break;
				}
			}
		}
		else
		{
			type = I_VOP;
			dwFrameLen = VideoEncRead(hVideoEnc, pFrameBuf + _FRAME_HEAD_LEN, 0xFFFE, &type);
		}
		if(dwFrameLen == 0)
		{
			switch(type)
			{
			case ENC_ERR_MODE:
				DBGMSG(DPINFO, "DPTrans VideoEncRead error mode!\r\n");
				break;
			case ENC_ERR_TMOUT:
				DBGMSG(DPINFO, "DPTrans VideoEncRead timeout!\r\n");
				break;
			case ENC_ERR_BUFNULL:
				DBGMSG(DPINFO, "DPTrans VideoEncRead pFrameBuf NULL!\r\n");
				break;
			case ENC_ERR_BUFSHORT:
				// 设置量化系数
				//VideoEncSetQuality(hVideoEnc, --dwQuality);
				dwBitrate = 0;
				dwCount = 0;
				break;
			}
		}
		else
		{
			// 计算每秒钟的数据量
			dwBitrate += dwFrameLen;
			dwCount++;
			if(dwCount == 25)
			{
				if(dwBitrate < 0x10000)
				{
					//VideoEncSetQuality(hVideoEnc, ++dwQuality);
				}

				if(dwBitrate > 0x20000)
				{
					//VideoEncSetQuality(hVideoEnc, --dwQuality);
				}

				printf("one second data:%x\r\n", dwBitrate);
				dwBitrate = 0;
				dwCount = 0;
			}

			DWORD tick = DPGetTickCount();
			BYTE *ptr = pFrameBuf;
			DWORD dwTotalLen = dwFrameLen + _FRAME_HEAD_LEN;

			int nCopyLen = 0;
			int nLeftLen = dwTotalLen;

			// 赋值帧头数据
			pFramePacket->seq = frame_seq_no++;
			if(type == I_VOP)
				pFramePacket->type = 1;
			else
				pFramePacket->type = 0;

			int count = 0;
			while(nLeftLen)
			{
				if(count > 4)
				{
					// 发送了4个包之后，暂停一会儿
					DPSleep(10);
					count = 0;
				}
				count++;

				pPacket->check = DP_CHECK_ID;
				pPacket->seq_no = seq_no++;
				pPacket->timestamp = tick;
				pPacket->flag = 0;

				if(nLeftLen > _FRAME_PACKET_LEN)
					nCopyLen = _FRAME_PACKET_LEN;
				else
					nCopyLen = nLeftLen;
				nLeftLen -= nCopyLen;

				if(nLeftLen + nCopyLen == dwTotalLen)
					pPacket->flag |= 1;
				if(nLeftLen == 0)
					pPacket->flag |= 2;

				memcpy(pPacket->pdata, ptr, nCopyLen);
				ptr += nCopyLen;

				AES_SetEncInitVec(g_pAes, SET_KEY);
				AES_DataEncrypt(g_pAes, (BYTE*)pPacket, (BYTE*)pPacket, nCopyLen + _PACKET_HEAD_LEN);

				sendto(g_sockfd, (char*)pPacket, nCopyLen + _PACKET_HEAD_LEN, 0, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
			}

			if(type == I_VOP)
			{
				g_bGetIFrame = FALSE;
			}
		}
	}

	VideoEncStop(hVideoEnc);
	free(pFrameBuf);
	DBGMSG(DPINFO, "SendThread end\r\n");
	return 0;
}

static DWORD RecvThread(HANDLE pParam)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((int)pParam);
	addr.sin_addr.s_addr = g_ip;

	g_pIFrameBuf = (BYTE*)malloc(_FRAME_MAX_LEN);
	g_dwIFrameLen = 0;

	BOOL bDecStart = FALSE;
	int dwTimeOutTick = 0;

	int seq_no = 0;
	BOOL bFindIFrame = TRUE;

	int nRecvLen;
	char buf[_PACKET_LEN];
	DPTransPacket *pPacket = (DPTransPacket*)buf;
	DPFramePacket* pFrame = (DPFramePacket*)pPacket->pdata;

	DWORD dwFrameLen = 0;
	char* pFrameData = (char*)malloc(_FRAME_MAX_LEN);
	DPFramePacket* pCompleteFrame = (DPFramePacket*)pFrameData;

	DBGMSG(DPINFO, "RecvThread start\r\n");
	while(g_bVTransRun)
	{
		nRecvLen = UdpRecv(g_sockfd, (char*)pPacket, _PACKET_LEN, 200, NULL);
		if(nRecvLen > _PACKET_HEAD_LEN)
		{
			AES_SetDecInitVec(g_pAes, SET_KEY);
			AES_DataDecrypt(g_pAes, (BYTE*)pPacket, (BYTE*)pPacket, nRecvLen);

			if(pPacket->check != DP_CHECK_ID)
			{
				continue;
			}

			if(pPacket->seq_no != seq_no)
			{
				DBGMSG(DPINFO, "lost except packet:%d, %d\r\n", seq_no, pPacket->seq_no);
				if(pPacket->seq_no < seq_no)
				{
					// 偶然情况，不用处理，直接获取下一包数据
					continue;
				}

				// 发送命令，请求IFrame
				DPCtrlPacket msg;
				msg.type = MSGTYPE_KEYFRAME;
				sendto(g_ctrlSock, (char*)&msg, sizeof(DPCtrlPacket), 0, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));

				bFindIFrame = TRUE;
			}
			seq_no = pPacket->seq_no + 1;

#if 0
			if(pPacket->flag & 1)
			{
				printf("recv seq_no:%d, frame_type:%d\r\n", seq_no, CheckFrameType(pPacket->pdata + _FRAME_HEAD_LEN, 64));
			}
#endif

			if(bFindIFrame)
			{
				if((pPacket->flag & 1)	// 包头
					&& (pFrame->type == 1))	// IFrame
				{
					bFindIFrame = FALSE;
					dwFrameLen = 0;
				}
				else
				{
					continue;
				}
			}

			if(dwFrameLen + nRecvLen > 0x1FFF4)
			{
				DBGMSG(DPINFO, "frame is too big\r\n");
				// 发送命令，请求IFrame
				DPCtrlPacket msg;
				msg.type = MSGTYPE_KEYFRAME;
				sendto(g_ctrlSock, (char*)&msg, sizeof(DPCtrlPacket), 0, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
				bFindIFrame = TRUE;
				continue;
			}

			memcpy(pFrameData + dwFrameLen, pPacket->pdata, nRecvLen - _PACKET_HEAD_LEN);
			dwFrameLen += nRecvLen - _PACKET_HEAD_LEN;

			if(pPacket->flag & 2)
			{
				if(!bDecStart)
				{
					dwTimeOutTick = 0;
					bDecStart = TRUE;
					H264DecStart(V_NORMAL_LEFT, V_NORMAL_TOP, V_NORMAL_WIDTH, V_NORMAL_HEIGHT);
				}

				if((pCompleteFrame->type == 1)
					&& (dwFrameLen > 0x3000))
				{
					// 保存IFrame
					g_IFrameCS.lockon();
					memcpy(g_pIFrameBuf, pFrameData + _FRAME_HEAD_LEN, dwFrameLen - _FRAME_HEAD_LEN);
					g_dwIFrameLen = dwFrameLen - _FRAME_HEAD_LEN;
					g_IFrameCS.lockoff();
				}

				H264WriteData(pFrameData + _FRAME_HEAD_LEN, dwFrameLen - _FRAME_HEAD_LEN, 0);
				dwFrameLen = 0;
			}
		}
		else
		{
			if(bDecStart)
			{
				dwTimeOutTick++;
				if(dwTimeOutTick == 10)
				{
					bDecStart = FALSE;
					H264DecStop();
					DBGMSG(DPINFO, "trans timeout\r\n");
				}
			}
		}
	}

	H264DecStop();

	free(pFrameData);
	free(g_pIFrameBuf);
	g_dwIFrameLen = 0;
	DBGMSG(DPINFO, "RecvThread end\r\n");
	return 0;
}

BOOL DPTransVideoStart(int ip, int port, int cport, int lport, int lcport, BOOL bMulticast)
{
	if(ip == 0)
		return TRUE;

	DBGMSG(DPINFO, "DPTransVideoStart start ip:%x, port:%d, cport:%d, lport:%d, lcport:%d, bMulticast:%d\r\n", ip, port, cport, lport, lcport, bMulticast);
	g_VideoCS.lockon();

	if(!g_bVTransRun)
	{
		g_bVTransRun = TRUE;
		if(g_pAes == NULL)
		{
			g_pAes = AES_New();
			AES_SetMode(g_pAes, AES_MODE_SIC);
			AES_SetKey(g_pAes, INIT_KEY, 128);
		}

		g_ip = ip;
		if(g_lcport != lcport)
		{
			SocketClose(g_ctrlSock);
			g_lcport = lcport;
			g_ctrlSock = UdpCreate(lcport);
		}

		if(bMulticast)
		{
			g_sockfd = UdpCreate(port);
			if(!UdpSetRecvBuf(g_sockfd, 60000))
			{
				DBGMSG(DPERROR, "DPTrans UdpSetRecvBuf\r\n");
				return 0;
			}

			int mul_ip = g_ip & 0xFFFF0000;
			mul_ip += 0xC0E6;
			UdpJoinGroup(g_sockfd, mul_ip);
			g_hRecvThread = DPThreadCreate(0x4000, RecvThread, (VOID*)cport, TRUE, 1);
		}
		else
		{
			g_sockfd = UdpCreate(lport);
			if(!UdpSetRecvBuf(g_sockfd, 60000))
			{
				DBGMSG(DPERROR, "DPTrans UdpSetRecvBuf\r\n");
				return 0;
			}

			g_hCtrlThread = DPThreadCreate(0x4000, CtrlThread, (VOID*)lcport, TRUE, 1);
			g_hSendThread = DPThreadCreate(0x4000, SendThread, (VOID*)port, TRUE, 1);
			g_hRecvThread = DPThreadCreate(0x4000, RecvThread, (VOID*)cport, TRUE, 1);
		}
	}

	g_VideoCS.lockoff();
	return TRUE;
}

BOOL DPTransVideoStop()
{
	g_VideoCS.lockon();
	if(g_bVTransRun)
	{
		DWORD tick = DPGetTickCount();
		g_bVTransRun = FALSE;
		DPThreadJoin(g_hRecvThread);
		if(g_hSendThread)
		{
			UdpSend(g_ctrlSock, "127.0.0.1", g_lcport, "0", 1);
			DPThreadJoin(g_hCtrlThread);
			g_hCtrlThread = NULL;

			DPThreadJoin(g_hSendThread);
			g_hSendThread = NULL;
		}
		SocketClose(g_sockfd);
		g_sockfd = INVALID_SOCKET;
		DBGMSG(DPINFO, "DPTransVideoStop cost %dms\r\n", DPGetTickCount() - tick);
	}
	g_VideoCS.lockoff();
	return TRUE;
}

void DPTransVideoSetRect(int left, int top, int width, int height)
{
	g_VideoCS.lockon();
	if(g_bVTransRun)
	{
		H264SetDisplayRect(left, top, width, height);
	}
	g_VideoCS.lockoff();
}

void DPTransVideoEnable(BOOL bEnable)
{
	g_bVideoEnable = bEnable;
}

DWORD GetIFrameData(char** pdata)
{
	DWORD len = 0;
	char* pFrameBuf = NULL;
	g_IFrameCS.lockon();
	if(g_dwIFrameLen > 0)
	{
		len = g_dwIFrameLen;
		pFrameBuf = (char*)malloc(len);
		memcpy(pFrameBuf, g_pIFrameBuf, len);
	}
	g_IFrameCS.lockoff();

	*pdata = pFrameBuf;
	return len;
}