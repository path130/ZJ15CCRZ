#include "roomlib.h"
#include "dpcommmsg.h"
#include "dpxmlmessage.h"
#include "DPXmlParse.h"

#define	SYNC_MSGQ			"NEWSYNCMSG"
#define	XML_SYNC_PORT		  0x8881

typedef struct  
{
	DWORD				DoorType;
	DPCHAR				PublicPwd[16];
	DPCHAR	 			HostagePwd[16];
}UnlockPwdMsg;

static BOOL g_bMsgStart = FALSE;
static HANDLE g_hMsgThread = NULL;
static StaticLock g_MsgCS;

static void RecvPersonMsg(MSGHEAD* pRecvMsg, int len)
{
	if(len != pRecvMsg->length)
		return;

	char buf[2048];
	unicode2utf8((BYTE*)buf, (wchar_t*)pRecvMsg->content);

	CXmlParse parse;
	parse.Init(buf);
	if(!parse.RootCheck("msg"))
		return;

	char* pcontent = parse.GetNodeContent("type");
	if(pcontent == NULL)
		return;
	
	if(MC_TYPE_PRIVATE_MSG != strtol(pcontent, NULL, 10))
		return;

	if(parse.SetLocate("info"))
	{
		AddMessage(parse.GetNodeContent("time"),
			parse.GetNodeContent("title"),
			parse.GetNodeContent("body"),
			parse.GetNodeContent("jpg"),
			FALSE);
	}
}

static void RecvDoorMsg(char* pdata, int len)
{	
	if(len < sizeof(MsgHeader_T))
		return;

	MsgHeader_T *phead = (MsgHeader_T*)pdata;
	switch(phead->type)
	{
	case REQ_MODIFY_DOOR_PWD:
		{
			UnlockPwdMsg *pMsg = (UnlockPwdMsg*)(pdata + sizeof(MsgHeader_T));
			char userPwd[64];
			char hostagePwd[64];
			unicode2utf8((BYTE*)userPwd, (wchar_t*)pMsg->PublicPwd);
			unicode2utf8((BYTE*)hostagePwd, (wchar_t*)pMsg->HostagePwd);
			AddMessage(userPwd, hostagePwd);
		}
		break;
	case REQ_SETDEFENSE:
		{
			DWORD bSafe = *(DWORD*)(pdata + sizeof(MsgHeader_T));
			if(bSafe)
			{
				//SetDefenseStatus(TRUE);
				//AddDefenseRecord(SMODE_LEAVE, SMODE_UNSET);
				//DPPostMessage(MSG_BROADCAST, SMODE_CHANGE, 0, 0);
				//DPPostMessage( MSG_SHOW_STATUE, 2014, 0, 0 );
			}
			else
			{
				//if(!GetDefenseIsAlarming()  &&  GetDefenseStatus() == TRUE)//探头报警不撤防
				//{
				//	PostSafeMessage(SMSG_PAUSE_RING, 0, 0, 0);
				//	SetDefenseStatus(FALSE);
				//	AddDefenseRecord(SMODE_UNSET, SMODE_LEAVE);
				//	DPPostMessage(MSG_BROADCAST, SMODE_CHANGE, FALSE, 0);
				//	DPPostMessage(MSG_SHOW_STATUE, 2015, 0, 0);
				//}
			}
		}
		break;
	}
}

static DWORD MsgThread(HANDLE pParam)
{
	DBGMSG(DPINFO, "MsgThread start\r\n");

	SOCKET listenSock = TcpListen(NULL, XML_SYNC_PORT);
	if(INVALID_SOCKET == listenSock)
	{
		DBGMSG(DPERROR, "MsgThread Socket TcpListen fail:%d\r\n", DPGetLastError());
		return 0;
	}

	char buf[1024] = {0};
	MSGHEAD* pRecvMsg = (MSGHEAD*)buf;
	while(g_bMsgStart)
	{
		SOCKET sockfd = TcpAccpet(listenSock, 2000, NULL);
		if(INVALID_SOCKET == sockfd)
			continue;

		SocketUnblock(sockfd);
		int ret = TcpRecvDataTry(sockfd, buf, 1024, 1000);
		switch(pRecvMsg->head)
		{
			case XMLMSG_CHECKID:
				RecvPersonMsg(pRecvMsg, ret);
				break;
			case DPMSG_CHECK_ID:
				RecvDoorMsg(buf, ret);
				break;
			default:
				DBGMSG(DPWARNING, "Recv Unknow Msg %x\r\b", pRecvMsg->head);
				break;
		}

		SocketClose(sockfd);
		pRecvMsg->head = 0;	// 重置
	}

	return 0;
}

void StartSyncRecvServer(void)
{
	g_MsgCS.lockon();
	DBGMSG(DPINFO, "StartSyncRecvServer start\r\n");

	if(!g_bMsgStart)
	{
		g_bMsgStart = TRUE;
		g_hMsgThread = DPThreadCreate(0x4000, MsgThread, NULL, TRUE, 5);
	}
	g_MsgCS.lockoff();
}

void StopSyncRecvServer(void)
{
	g_MsgCS.lockon();
	DBGMSG(DPINFO, "StopSyncRecvServer start\r\n");

	if(g_bMsgStart)
		g_bMsgStart = FALSE;

	if(g_hMsgThread)
	{
		DPThreadJoin(g_hMsgThread);
		g_hMsgThread = NULL;
	}
	g_MsgCS.lockoff();
}