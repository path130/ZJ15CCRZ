#include "roomlib.h"
#include "dpcommmsg.h"
#include "ipfunc.h"

#define	SYNC_PORT			0x8888

BOOL SetSecDoorCode(char* doorcode, UINT64 newid)
{
	ip_get iptmp;
	CTcpClientSock Csock;
	MsgSetDoornumReq_T msgreq;
	MsgRspStatue_T msgack;

	BOOL ret = FALSE;
	if(!TermGet(&iptmp, doorcode))
		return FALSE;

	msgreq.msgHeader.head = DPMSG_CHECK_ID;
	msgreq.msgHeader.type = REQ_SET_DOOR_NUM;
	GetLocalID(&msgreq.msgHeader.id);
	msgreq.msgHeader.length = sizeof(MsgSetDoornumReq_T) - sizeof(MsgHeader_T);
	msgreq.newid = newid;

	if(Csock.Connect(iptmp.param->ip, SYNC_PORT))
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgSetDoornumReq_T)))
		{
			if(Csock.Recv((char*)&msgack, sizeof(MsgRspStatue_T)))
			{
				if((msgack.msgHeader.type == RSP_SET_DOOR_NUM)
					&& (msgack.statue == RSP_STATUS_OK))
				{
					ret = TRUE;
				}
			}
		}
	}
	free(iptmp.param);

	return ret;
}


BOOL SetSecDoorDelay(char* doorcode, DWORD level, DWORD delay)
{
	MsgLockSetting_T msgreq;
	MsgRspStatue_T msgack;
	ip_get iptmp;
	CTcpClientSock Csock;
	BOOL ret = FALSE;

	msgreq.msgHeader.head = DPMSG_CHECK_ID;
	msgreq.msgHeader.type = REQ_MODIFY_LOCK_SETTING;
	GetLocalID(&msgreq.msgHeader.id);
	msgreq.msgHeader.length = sizeof(MsgLockSetting_T) - sizeof(MsgHeader_T);
	msgreq.MagicDelay = -1;
	msgreq.LockDelay = delay;
	msgreq.LockLevel = level;

	if(!TermGet(&iptmp, doorcode))
		return FALSE;

	if(Csock.Connect(iptmp.param->ip, SYNC_PORT))
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgLockSetting_T)))
		{
			if(Csock.Recv((char*)&msgack, sizeof(MsgRspStatue_T)))
			{
				if((msgack.msgHeader.type == RSP_MODIFY_LOCK_SETTING)
					&& (msgack.statue == RSP_STATUS_OK))
				{
					ret = TRUE;
				}
			}
		}
	}
	free(iptmp.param);
	return ret;
}

BOOL ReqDoorAddCard(DWORD type)
{
	ip_get iptmp;
	CTcpClientSock Csock;
	MsgHeader_T msgreq;
	MsgRspStatue_T msgack;
	BOOL ret = FALSE;

	if(type == CELL_DOOR_TYPE)
	{
		if(CellDoorGet(&iptmp, NULL) <= 0)
			return FALSE;
	}
	else if(type == SECOND_DOOR_TYPE)
	{
		if(SecDoorGet(&iptmp, NULL) <= 0)
			return FALSE;
	}
	else
		return FALSE;

	msgreq.head = DPMSG_CHECK_ID;
	msgreq.type = REQ_START_ADD_CALL;
	GetLocalID(&msgreq.id);
	msgreq.length = 0;

	if(Csock.Connect(iptmp.param->ip, SYNC_PORT))
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgHeader_T)))
		{
			if(Csock.Recv((char*)&msgack, sizeof(MsgRspStatue_T)))
			{
				if((msgack.msgHeader.type == RSP_START_ADD_CALL)
					&& (msgack.statue == RSP_STATUS_OK))
					ret = TRUE;
			}
		}
	}
	free(iptmp.param);
	return ret;
}

BOOL ReqDoorDelCard(DWORD type)
{
	ip_get iptmp;
	CTcpClientSock Csock;
	MsgHeader_T msgreq;
	MsgRspStatue_T msgack;
	BOOL ret = FALSE;

	if(type == CELL_DOOR_TYPE)
	{
		if(CellDoorGet(&iptmp, NULL) <= 0)
			return FALSE;
	}
	else if(type == SECOND_DOOR_TYPE)
	{
		if(SecDoorGet(&iptmp, NULL) <= 0)
			return FALSE;
	}
	else
		return FALSE;

	msgreq.head = DPMSG_CHECK_ID;
	msgreq.type = REQ_CLEAR_ALLCARD;
	GetLocalID(&msgreq.id);
	msgreq.length = 0;

	if(Csock.Connect(iptmp.param->ip, SYNC_PORT))
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgHeader_T)))
		{
			if(Csock.Recv((char*)&msgack, sizeof(MsgRspStatue_T)))
			{
				if((msgack.msgHeader.type == RSP_CLEAR_ALLCARD)
					&& (msgack.statue == RSP_STATUS_OK))
					ret = TRUE;
			}
		}
	}
	free(iptmp.param);
	return ret;
}

BOOL ChangeUserPwd(char* pwd)
{
	ip_get iptmp;
	CTcpClientSock Csock;
	MsgModUserPwdReq_T msgreq;
	MsgRspStatue_T msgack;
	BOOL ret = FALSE;

	if(CellDoorGet(&iptmp, NULL) < 0)
		return FALSE;

	DBGMSG(DPINFO, "ChangeUserPwd Door ip:%x\r\n", iptmp.param->ip);

	msgreq.msgHeader.head = DPMSG_CHECK_ID;
	msgreq.msgHeader.type = REQ_MODIFY_USER_PWD;
	GetLocalID(&msgreq.msgHeader.id);
	msgreq.msgHeader.length = sizeof(MsgModUserPwdReq_T) - sizeof(MsgHeader_T);

	utf82unicode((WORD*)msgreq.Password, (BYTE*)pwd);
	if(Csock.Connect(iptmp.param->ip, SYNC_PORT))
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgModUserPwdReq_T)))
		{
			if(Csock.Recv((char*)&msgack, sizeof(MsgRspStatue_T)))
			{
				if((msgack.msgHeader.type == REP_MODIFY_USER_PWD)
					&& (msgack.statue == RSP_STATUS_OK))
					ret = TRUE;
			}
		}
	}
	free(iptmp.param);
	return ret;
}


BOOL ChangeHostagePwd(char* pwd)
{
	ip_get iptmp;
	CTcpClientSock Csock;
	MsgModUserPwdReq_T msgreq;
	MsgRspStatue_T msgack;
	BOOL ret = FALSE;

	if(CellDoorGet(&iptmp, NULL) < 0)
		return FALSE;

	DBGMSG(DPINFO, "ChangeHostagePwd Door ip:%x\r\n", iptmp.param->ip);

	msgreq.msgHeader.head = DPMSG_CHECK_ID;
	msgreq.msgHeader.type = REQ_MODIFY_HOSTAGE_PWD;
	GetLocalID(&msgreq.msgHeader.id);
	msgreq.msgHeader.length = sizeof(MsgModUserPwdReq_T) - sizeof(MsgHeader_T);

	utf82unicode((WORD*)msgreq.Password, (BYTE*)pwd);

	if(Csock.Connect(iptmp.param->ip, SYNC_PORT))
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgModUserPwdReq_T)))
		{
			if(Csock.Recv((char*)&msgack, sizeof(MsgRspStatue_T)))
			{
				if((msgack.msgHeader.type == RSP_MODIFY_HOSTAGE_PWD)
					&& (msgack.statue == RSP_STATUS_OK))
					ret = TRUE;
			}
		}
	}
	free(iptmp.param);
	return ret;
}

BOOL CallElevator(void)
{
	ip_get iptmp;
	CTcpClientSock Csock;
	MsgHeader_T msgreq;
	MsgRspStatue_T msgack;

	BOOL ret = FALSE;

	if(CellDoorGet(&iptmp, NULL) <= 0)
		return ret;

	msgreq.head = DPMSG_CHECK_ID;
	msgreq.type = REQ_CALL_ELEVATOR;
	if (!GetLocalID(&msgreq.id))
		return ret;

	msgreq.length = 0;

	if(Csock.Connect(iptmp.param->ip, SYNC_PORT))
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgHeader_T)))
		{
			Csock.SetTimeout(2000);
			if(Csock.Recv((char*)&msgack, sizeof(MsgRspStatue_T)))
			{
				if((msgack.msgHeader.type == RSP_CALL_ELEVATOR)
					&& (msgack.statue == RSP_STATUS_OK))
					ret = TRUE;
			}
		}
	}

	DBGMSG(DPINFO, "CallElevator ip:%x, code:%s\r\n", iptmp.param[0].ip, iptmp.param[0].code);
	free(iptmp.param);
	return ret;
}

BOOL SyncCellDoor(void)
{
	ip_get iptmp;
	MsgHeader_T msgreq;
	MsgRoomSyncResp_T msgack;

	BOOL ret = FALSE;

	if(!CellDoorGet(&iptmp, NULL))
	{
		DBGMSG(DPINFO, "SyncCellDoor CellDoorGet failed\r\n");
		return FALSE;
	}

	msgreq.head = DPMSG_CHECK_ID;
	msgreq.type = REQ_ROOM_SYNC_MSG;
	GetLocalID(&msgreq.id);
	msgreq.length = 0;

	CTcpClientSock Csock;
	int i;
	for (i = 0; i < iptmp.num; ++i)
	{
		if(Csock.Connect(iptmp.param[i].ip, SYNC_PORT))
			break;
		else
			DBGMSG(DPINFO, "SyncCellDoor Connect %x failed\r\n", iptmp.param[i].ip);
	}

	if (i != iptmp.num)
	{
		if(Csock.Send((char*)&msgreq, sizeof(MsgHeader_T)))
		{
			if(Csock.Recv((char*)&msgack, sizeof(MsgRoomSyncResp_T)))
			{
				if(msgack.msgHeader.type == RSP_ROOM_SYNC_MSG)
				{
					time_t tmptime = (time_t)msgack.time;
					FILETIME t = timeToFileTime(&tmptime);
					SYSTEMTIME syst;
					DPFileTimeToSystemTime(&t,&syst);
					DPSetLocalTime(&syst);
					ret = TRUE;
				}
				else
					DBGMSG(DPWARNING, "SyncCellDoor type error\r\n");
			}
			else
				DBGMSG(DPWARNING, "SyncCellDoor Recv failed\r\n");
		}
		else
			DBGMSG(DPWARNING, "SyncCellDoor Send failed\r\n");
	}

	free(iptmp.param);

	return ret;
}
