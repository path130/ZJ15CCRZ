#include "roomlib.h"

#define MSGFILE0			"Message0.ext"
#define MSGFILE1			"Message1.ext"
#define	MESSAGEDIR			"Message"
#define MESSAGEENDID		0x4d534753

#define PUBLIC_TYPE			0		
#define PERSON_TYPE			1		
#define MAX_TYPE			2

typedef struct
{
	MsgRecord msg[MAX_TYPE][MAX_MESSAGE_NUMBER];
	DWORD count[MAX_TYPE];
	DWORD countUnread;	
	DWORD VERSION;
	DWORD Endid;
}MessageList;

static MessageList* G_Message = NULL;
static StaticLock g_MessageCS;
static BOOL g_bUpdateEn = TRUE;

static BOOL CheckSpaceOfAddMessage(BOOL isArea, int len)
{
	UINT64 freeSize;
	int type = isArea ? PUBLIC_TYPE : PERSON_TYPE;

	do
	{
		freeSize = CheckSpareSpace(USERDIR);
		if(freeSize > len)
			return TRUE;

		if(G_Message->count[type] == 0)
			return FALSE;

		DeleteMessage(isArea, G_Message->count[type] - 1);
	}while(1);
}

static void UpdateMessage(void)
{
	if(!g_bUpdateEn)
		return;

	MessageList* pSet;
	BOOL ret;

	pSet = (MessageList*)malloc(sizeof(MessageList));
	memcpy(pSet, G_Message, sizeof(MessageList));
	if(G_Message->VERSION & 1)
		ret = WriteServerFile(MSGFILE1, sizeof(MessageList), pSet);
	else
		ret = WriteServerFile(MSGFILE0, sizeof(MessageList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(G_Message->VERSION & 1)
			DeleteServerFile(MSGFILE0);
		else
			DeleteServerFile(MSGFILE1);
		G_Message->VERSION++;
	}
}

static DWORD IsMessageFileOk(char * pfilename, char* mark)
{
	char filename[32];
	DWORD i;

	for(int j = 0; j < MAX_TYPE; j++)
	{
		for(int i = 0; i < G_Message->count[j]; i++)
		{
			sprintf(filename, "%08x%08x.msg", G_Message->msg[j][i].time.dwHighDateTime, G_Message->msg[j][i].time.dwLowDateTime);
			if(strcmp(filename, pfilename) == 0)
			{
				mark[i + j * MAX_MESSAGE_NUMBER] = 1;
				return TRUE;
			}
		}
	}

	return FALSE;
}

static void CheckMessageFile(char* mark)
{
	char name[64];
	char findData[64];
	int index;

	sprintf(name, "%s/%s", USERDIR, MESSAGEDIR);
	HANDLE hFile = DPFindFirstFile(name, findData);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if((strcmp(findData, "..") == 0) || (strcmp(findData, ".") == 0))
				continue;

			if(!IsMessageFileOk(findData, mark))
			{
				printf("find invalid file:%s\r\n", findData);
				sprintf(name, "%s/%s", MESSAGEDIR, findData); 
				DeleteServerFile(name);
			}
		}while(DPFindNextFile(hFile, findData));
		DPFindClose(hFile);
	}	
}

void InitMessage()
{
	MessageList* pSet0 = NULL;
	MessageList* pSet1 = NULL;
	FILE* fd;
	DWORD i;

	char filename[64];
	sprintf(filename, "%s/%s", USERDIR, MESSAGEDIR);
	CheckAndCreateDir(filename);

	sprintf(filename, "%s/%s", USERDIR, MSGFILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (MessageList*)malloc(sizeof(MessageList));
		memset(pSet0, 0, sizeof(MessageList));
		if(fread(pSet0, 1, sizeof(MessageList), fd) != sizeof(MessageList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != MESSAGEENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}

	sprintf(filename, "%s/%s", USERDIR, MSGFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (MessageList*)malloc(sizeof(MessageList));
		memset(pSet1, 0, sizeof(MessageList));
		if(fread(pSet1, 1, sizeof(MessageList), fd) != sizeof(MessageList))
		{
			free(pSet1);
			pSet1 = NULL;
		}
		else
		{
			if(pSet1->Endid != MESSAGEENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}

	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		G_Message = (MessageList*)malloc(sizeof(MessageList));
		memset(G_Message, 0, sizeof(MessageList));
		G_Message->Endid = MESSAGEENDID;
		UpdateMessage();
	}
	else if((pSet0 != NULL) && (pSet1 != NULL))
	{
		if(pSet0->VERSION > pSet1->VERSION)
		{
			G_Message = pSet0;
			free(pSet1);
		}
		else
		{
			G_Message = pSet1;
			free(pSet0);
		}
		G_Message->VERSION++;
	}
	else if(pSet0 != NULL)
	{
		G_Message = pSet0;
		G_Message->VERSION++;
	}
	else
	{
		G_Message = pSet1;
		G_Message->VERSION++;
	}

	char mark[MAX_MESSAGE_NUMBER * 2];
	CheckMessageFile(mark);

	G_Message->countUnread = 0;
	for(int j = 0; j < MAX_TYPE; j++)
	{
		for(i = 0; i < G_Message->count[j]; )
		{
			if(mark[i] == 0)
			{
				memmove(&mark[i], &mark[i + 1], (G_Message->count[j] - i - 1));
				memmove(&G_Message->msg[j][i], &G_Message->msg[j][i + 1], sizeof(MsgRecord) * (G_Message->count[j] - i - 1));
				G_Message->count[j]--;
			}
			else
			{
				if(G_Message->msg[j][i].bRead == 0)
					G_Message->countUnread++;
				i++;
			}
		}
	}
}

void ResetMessage()
{
	g_MessageCS.lockon();
	memset(G_Message, 0, sizeof(MessageList));
	G_Message->Endid = MESSAGEENDID;
	UpdateMessage();
	char mark[MAX_MESSAGE_NUMBER * 2];
	CheckMessageFile(mark);
	g_MessageCS.lockoff();
}

DWORD GetMessageCount(BOOL isArea)
{
	if(isArea)
		return G_Message->count[PUBLIC_TYPE];
	else
		return G_Message->count[PERSON_TYPE];
}

DWORD GetMessageUnreadCount()
{
	return G_Message->countUnread;
}

BOOL AddMessage(char* ptime, char* ptitle, char* pbody, char* purl, BOOL isArea)
{
	char* pbuf;
	int len;
	if(HttpDownloadFile(TEMP_MSG_PATH, purl))
	{
		len = BReadFile(TEMP_MSG_PATH, &pbuf);
		if(len != 0)
		{
			if (AddMessage(isArea, ptitle, pbuf, len, ptime))
			{
				SetMsgTime(ptime);
				//DBGMSG(DPINFO, "SaveMessage AddMessageEx Successful\r\n");
			}
			else
			{
				free(pbuf);
			}
			DPPostMessage(MSG_BROADCAST, MESSAGE_CHANGE, 0, 0);
		}

		DPDeleteFile(TEMP_MSG_PATH);
		return TRUE;
	}
	else
	{
		printf("AddMessage HttpDownloadFile fail!\r\n");
	}

	return FALSE;
}

BOOL AddMessage(BYTE isArea, char* title, char* pbuf, int length, char* time)
{
	if(!CheckSpaceOfAddMessage(isArea, length + 256))
		return FALSE;

	SYSTEMTIME systime = {0};
	FILETIME ftime;

	if(6 != sscanf(time, "%d-%d-%d %d:%d:%d",&systime.wYear,&systime.wMonth,
		&systime.wDay,&systime.wHour,&systime.wMinute,&systime.wSecond))
	{
		DPGetLocalTime(&systime);
	}

	DPSystemTimeToFileTime(&systime, &ftime); 

	char filename[128];
	sprintf(filename, "%s/%08x%08x.msg", MESSAGEDIR, ftime.dwHighDateTime, ftime.dwLowDateTime);

	int type = isArea ? PUBLIC_TYPE : PERSON_TYPE;
	g_MessageCS.lockon();
	if(WriteServerFile(filename, length, pbuf))
	{
		if(G_Message->count[type] == MAX_MESSAGE_NUMBER)
		{
			//删除最旧一条
			sprintf(filename, "%s/%08x%08x.msg", MESSAGEDIR, G_Message->msg[type][MAX_MESSAGE_NUMBER - 1].time.dwHighDateTime, G_Message->msg[type][MAX_MESSAGE_NUMBER - 1].time.dwLowDateTime);
			DeleteServerFile(filename);
			G_Message->count[type]--;
			if(G_Message->msg[type][MAX_MESSAGE_NUMBER - 1].bRead == 0)
				G_Message->countUnread--;

		}
		memmove(&G_Message->msg[type][1], &G_Message->msg[type][0], sizeof(MsgRecord)*(MAX_MESSAGE_NUMBER-1));

		strncpy(G_Message->msg[type][0].title, title, 256);
		G_Message->msg[type][0].time = ftime;
		G_Message->msg[type][0].bRead = 0;
		G_Message->count[type]++;
		G_Message->countUnread++;
		UpdateMessage();
		PlayWav(MESSAGE_INDEX, GetRingVol());
		DPPostMessage(MSG_BROADCAST, MESSAGE_CHANGE, 0, 0);
	}
	else
	{
		free(pbuf);
	}
	g_MessageCS.lockoff();
	return TRUE;
}

BOOL AddMessage(char* userPwd, char* hostagePwd)
{
	char *pbuf = (char*)malloc(128);
	sprintf(pbuf, "%s %s", userPwd, hostagePwd );
	// 门口机开锁密码更改
	AddMessage(TRUE, GetStringByID(30041), pbuf, 128, "");
	return TRUE;
}

void DeleteMessage(BOOL isArea, int index)
{
	char filename[64];
	int type = isArea ? PUBLIC_TYPE : PERSON_TYPE;

	if(index >= G_Message->count[type])
		return;

	g_MessageCS.lockon();
	sprintf(filename, "%s/%08x%08x.msg", MESSAGEDIR, G_Message->msg[type][index].time.dwHighDateTime, G_Message->msg[type][index].time.dwLowDateTime);
	DeleteServerFile(filename);
	if(G_Message->msg[type][index].bRead == FALSE)
		G_Message->countUnread--;
	memmove(&G_Message->msg[type][index] , &G_Message->msg[type][index + 1] , (G_Message->count[type] - index - 1)*sizeof(MsgRecord));
	G_Message->count[type]--;
	UpdateMessage();
	g_MessageCS.lockoff();
}

void DeleteMessageAll(BOOL isArea)
{
	char filename[64];
	int type = isArea ? PUBLIC_TYPE : PERSON_TYPE;
	g_MessageCS.lockon();
	for(int i = 0; i < G_Message->count[type]; i++)
	{
		sprintf(filename, "%s/%08x%08x.msg", MESSAGEDIR, G_Message->msg[type][i].time.dwHighDateTime, G_Message->msg[type][i].time.dwLowDateTime);
		DeleteServerFile(filename);
		if(G_Message->msg[type][i].bRead == 0)
			G_Message->countUnread--;
	}
	
	G_Message->count[type] = 0;
	UpdateMessage();
	g_MessageCS.lockoff();
}

int GetMessageData(BOOL isArea, int index, char** buf)
{
	int type = isArea ? PUBLIC_TYPE : PERSON_TYPE;
	if(index >= G_Message->count[type])
	{
		*buf = NULL;
		return 0;
	}

	g_MessageCS.lockon();
	char filename[64];
	sprintf(filename, "%s/%s/%08x%08x.msg", USERDIR, MESSAGEDIR, G_Message->msg[type][index].time.dwHighDateTime, G_Message->msg[type][index].time.dwLowDateTime);
	char* pbuf;
	int len = BReadFile(filename, &pbuf);
	if(len > 0)
		*buf = pbuf;
	else
		*buf = NULL;

	if(G_Message->msg[type][index].bRead == FALSE)
	{
		G_Message->msg[type][index].bRead = TRUE;
		G_Message->countUnread--;
		UpdateMessage();
	}

	g_MessageCS.lockoff();
	return len;
}

int GetMessageRecord(BOOL isArea, MsgRecord* pRecord)
{
	int count = 0;
	int type = isArea ? PUBLIC_TYPE : PERSON_TYPE;
	g_MessageCS.lockon();
	count = G_Message->count[type];
	memcpy(pRecord, G_Message->msg[type], sizeof(MsgRecord) * MAX_MESSAGE_NUMBER);
	g_MessageCS.lockoff();
	return count;
}

void UpdateMessageEn(BOOL bEnable)
{
	g_bUpdateEn = bEnable;
	if(g_bUpdateEn)
		UpdateMessage();
}