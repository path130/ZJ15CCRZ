#include "roomlib.h"

#define CALLRECFILE0			"CallRec0.ext"
#define CALLRECFILE1			"CallRec1.ext"
#define CALLRECENDID			0x4d534753

#define MISS_TYPE		0
#define ACCEPT_TYPE		1
#define CALLOUT_TYPE	2
#define MAX_TYPE		3

typedef struct
{
	CallRecord msg[MAX_TYPE][MAX_CALLLOG];
	DWORD count[MAX_TYPE];
	DWORD unreadCount;	
	DWORD VERSION;
	DWORD Endid;
}CallRecordList;

static CallRecordList* g_pCallRec = NULL;
static StaticLock g_CallRecCS;

static void UpdateCallRecord(void)
{
	CallRecordList* pSet;
	BOOL ret;

	pSet = (CallRecordList*)malloc(sizeof(CallRecordList));
	memcpy(pSet, g_pCallRec, sizeof(CallRecordList));
	if(g_pCallRec->VERSION & 1)
		ret = WriteServerFile(CALLRECFILE1, sizeof(CallRecordList), pSet);
	else
		ret = WriteServerFile(CALLRECFILE0, sizeof(CallRecordList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_pCallRec->VERSION & 1)
			DeleteServerFile(CALLRECFILE0);
		else
			DeleteServerFile(CALLRECFILE1);
		g_pCallRec->VERSION++;
	}
}

void InitCallRecord()
{
	CallRecordList* pSet0 = NULL;
	CallRecordList* pSet1 = NULL;
	FILE* fd;
	DWORD i;

	char filename[64];
	sprintf(filename, "%s/%s", USERDIR, CALLRECFILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (CallRecordList*)malloc(sizeof(CallRecordList));
		memset(pSet0, 0, sizeof(CallRecordList));
		if(fread(pSet0, 1, sizeof(CallRecordList), fd) != sizeof(CallRecordList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != CALLRECENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}

	sprintf(filename, "%s/%s", USERDIR, CALLRECFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (CallRecordList*)malloc(sizeof(CallRecordList));
		memset(pSet1, 0, sizeof(CallRecordList));
		if(fread(pSet1, 1, sizeof(CallRecordList), fd) != sizeof(CallRecordList))
		{
			free(pSet1);
			pSet1 = NULL;
		}
		else
		{
			if(pSet1->Endid != CALLRECENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}

	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pCallRec = (CallRecordList*)malloc(sizeof(CallRecordList));
		memset(g_pCallRec, 0, sizeof(CallRecordList));
		g_pCallRec->Endid = CALLRECENDID;
		UpdateCallRecord();
	}
	else if((pSet0 != NULL) && (pSet1 != NULL))
	{
		if(pSet0->VERSION > pSet1->VERSION)
		{
			g_pCallRec = pSet0;
			free(pSet1);
		}
		else
		{
			g_pCallRec = pSet1;
			free(pSet0);
		}
		g_pCallRec->VERSION++;
	}
	else if(pSet0 != NULL)
	{
		g_pCallRec = pSet0;
		g_pCallRec->VERSION++;
	}
	else
	{
		g_pCallRec = pSet1;
		g_pCallRec->VERSION++;
	}
}

void ResetCallRecord()
{
	g_CallRecCS.lockon();
	memset(g_pCallRec, 0, sizeof(CallRecordList));
	g_pCallRec->Endid = CALLRECENDID;
	UpdateCallRecord();
	g_CallRecCS.lockoff();
}

void AddCallRecord(char* code, BOOL bCallOut, BOOL bAccept)
{
	g_CallRecCS.lockon();

	CallRecord rec;
	rec.bRead = FALSE;
	strcpy(rec.code, code);

	SYSTEMTIME stime;
	DPGetLocalTime(&stime);
	DPSystemTimeToFileTime(&stime,&rec.time);
	
	int type;
	if(bCallOut)
		type = CALLOUT_TYPE;
	else if(bAccept)
		type = ACCEPT_TYPE;
	else
	{
		type = MISS_TYPE;
		if(g_pCallRec->unreadCount < MAX_CALLLOG)
			g_pCallRec->unreadCount++;
		DPPostMessage(MSG_BROADCAST, CALLLOG_CHAGE, 0, 0);
	}

	memmove(&g_pCallRec->msg[type][1], &g_pCallRec->msg[type][0], sizeof(CallRecord) * (MAX_CALLLOG - 1));
	memcpy(&g_pCallRec->msg[type][0], &rec, sizeof(CallRecord));
	if(g_pCallRec->count[type] < MAX_CALLLOG)
		g_pCallRec->count[type]++;
	UpdateCallRecord();

	g_CallRecCS.lockoff();
}

void DeleteCallRecord(int type, int index)
{
	if(type >= MAX_TYPE)
		return;
	if(index >= g_pCallRec->count[type])
		return;

	g_CallRecCS.lockon();
	if(type == MISS_TYPE
		&& !g_pCallRec->msg[MISS_TYPE][index].bRead)
	{
		g_pCallRec->unreadCount--;
	}
	memmove(&g_pCallRec->msg[type][index], &g_pCallRec->msg[type][index + 1], sizeof(CallRecord) * (MAX_CALLLOG - 1 - index));
	g_pCallRec->count[type]--;
	UpdateCallRecord();
	g_CallRecCS.lockoff();
}

void DeleteCallRecordAll(int type)
{
	if(type >= MAX_TYPE)
		return;

	g_CallRecCS.lockon();
	g_pCallRec->count[type] = 0;
	if(type == MISS_TYPE)
		g_pCallRec->unreadCount = 0;
	UpdateCallRecord();
	g_CallRecCS.lockoff();
}

int GetCallRecord(int type, CallRecord* pRecord)
{
	int count = 0;
	g_CallRecCS.lockon();
	count = g_pCallRec->count[type];
	memcpy(pRecord, g_pCallRec->msg[type], sizeof(CallRecord) * MAX_CALLLOG);
	g_CallRecCS.lockoff();
	return count;
}

void GetCallRecordCode(char* code, int type, int index)
{
	g_CallRecCS.lockon();
	if((type < MAX_TYPE)
		&& (index < g_pCallRec->count[type])
		&& (code != NULL))
	{
		strcpy(code, g_pCallRec->msg[type][index].code);
	}
	g_CallRecCS.lockoff();
}

int GetCallRecordUnread()
{
	return g_pCallRec->unreadCount;
}

void SetCallRecordRead(int index)
{
	g_CallRecCS.lockon();
	if(index < g_pCallRec->count[MISS_TYPE])
	{
		if(g_pCallRec->msg[MISS_TYPE][index].bRead == FALSE)
		{
			g_pCallRec->msg[MISS_TYPE][index].bRead = TRUE;
			g_pCallRec->unreadCount--;
			UpdateCallRecord();
		}
	}
	g_CallRecCS.lockoff();
}