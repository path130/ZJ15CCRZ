#include "roomlib.h"

#define	FS_MSGQ			"FILESERVER"
#define	MSG_WRITEFILE	1
#define	MSG_DELFILE		2
#define	MSG_ENDFILE		3

typedef struct
{
	DWORD msgtype;
	char filename[64];
	int filelen;
	void* filedata;
} FILEMSG;

static BOOL FileServerStart = FALSE;
static HANDLE hFileServerSendQ = NULL;
static HANDLE hFileServerThd = NULL;
static StaticLock g_FileServerCS;

static BOOL FileMsgProc(FILEMSG* msg)
{
	FILE* fd;
	char wcDelFilePath[MAX_PATH]={0};

	switch(msg->msgtype)
	{
	case MSG_WRITEFILE:
	case MSG_DELFILE:
		sprintf(wcDelFilePath, "%s/%s", USERDIR, msg->filename);
		if(msg->msgtype == MSG_WRITEFILE)
		{
			DBGMSG(DPINFO, "WriteFile %s\r\n", wcDelFilePath);
			if(msg->filelen != 0)
			{
				fd = fopen(wcDelFilePath, "wb");
				if(fd != NULL)
				{
					fwrite(msg->filedata, 1, msg->filelen, fd);
					fclose(fd);
				}
				else
				{
					DBGMSG(DPINFO, "WriteFile %s Fail\r\n", wcDelFilePath);
				}
				free(msg->filedata);
			}
		}
		else
		{
			DBGMSG(DPINFO, "DeleteFile %s\r\n", wcDelFilePath);
			DPDeleteFile(wcDelFilePath);
		}
		break;
	case MSG_ENDFILE:
		return FALSE;
		break;
	}
	return TRUE;
}

static DWORD WINAPI FileServerThread(HANDLE	hMsgQR)
{
	FILEMSG smsg;

	while(FileServerStart)
	{
		if(DPReadMsgQueue(hMsgQR, &smsg, sizeof(FILEMSG), 1000000))
		{
			if(!FileMsgProc(&smsg))
				break;
		}
	}
	while(DPReadMsgQueue(hMsgQR, &smsg, sizeof(FILEMSG), 0))
		FileMsgProc(&smsg);
	DPCloseMsgQueue(hMsgQR);
	DBGMSG(DPINFO, "FileMsgProc end\r\n");
	return 0;
}

BOOL WriteServerFile(const char* name, int len, void* buf)
{
	FILEMSG smsg;
	BOOL ret = FALSE;

	g_FileServerCS.lockon(); 
	if(FileServerStart)
	{
		smsg.msgtype = MSG_WRITEFILE;
		strncpy(smsg.filename, name, 30);
		smsg.filelen = len;
		smsg.filedata = buf;
		if(DPWriteMsgQueue(hFileServerSendQ, &smsg, sizeof(FILEMSG), 0))
		{
			ret = TRUE;
		}
		else
		{
			DBGMSG(SRV_MOD|DPERROR, "WriteServerFile error:%d\n", DPGetLastError());
		}
	}
	g_FileServerCS.lockoff(); 
	return ret;
}

void DeleteServerFile(const char* name)
{
	FILEMSG smsg;

	g_FileServerCS.lockon(); 
	if(FileServerStart)
	{
		smsg.msgtype = MSG_DELFILE;
		strncpy(smsg.filename, name, 63);
		if(!DPWriteMsgQueue(hFileServerSendQ, &smsg, sizeof(FILEMSG), 0))
		{
			DBGMSG(SRV_MOD|DPERROR, "DeleteServerFile error:%d\n", DPGetLastError());
		}
	}
	g_FileServerCS.lockoff(); 
	return;
}

BOOL InitFileServer(void)
{
	HANDLE	hMsgQR;
	BOOL ret;

	DBGMSG(DPINFO, "InitFileServer start\r\n");
	g_FileServerCS.lockon(); 
	if(!FileServerStart)
	{
		DPCreateMsgQueue(FS_MSGQ, 100, sizeof(FILEMSG), &hMsgQR, &hFileServerSendQ);
		hFileServerThd = DPThreadCreate(0x4000, FileServerThread, hMsgQR, TRUE, 5);
		FileServerStart = TRUE;
	}
	ret = FileServerStart;
	g_FileServerCS.lockoff(); 
	DBGMSG(DPINFO, "InitFileServer end\r\n");
	return ret;
}

void DeinitFileServer(void)
{
	FILEMSG smsg;

	DBGMSG(DPINFO, "DeinitFileServer start\r\n");
	g_FileServerCS.lockon(); 
	if(FileServerStart)
	{
		FileServerStart = FALSE;
		smsg.msgtype = MSG_ENDFILE;
		if(!DPWriteMsgQueue(hFileServerSendQ, &smsg, sizeof(FILEMSG), 0))
		{
			DBGMSG(SRV_MOD|DPERROR, "DenitFileServer WriteMsgQueue error:%d\n", DPGetLastError());
		}
		DPThreadJoin(hFileServerThd);
		DPCloseMsgQueue(hFileServerSendQ);
	}
	g_FileServerCS.lockoff(); 
	DBGMSG(DPINFO, "DeinitFileServer end\r\n");
}

