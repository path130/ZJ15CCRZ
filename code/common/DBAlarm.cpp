#include "roomlib.h"

#define ALARMRECFILE0		"AlarmRec0.ext"
#define ALARMRECFILE1		"AlarmRec1.ext"
#define ALARMRECENDID        0x53524567

typedef struct
{
	AlarmRecord rec[MAX_ALARM_RECORD_NUMBER];
	DWORD count;         		 
	DWORD Version;
	DWORD Endid;
}AlarmRecList;

static AlarmRecList* g_pAlarmRec = NULL;
static StaticLock g_alarmRecCS;

static BOOL UpdateAlarmRecord(void)
{
	AlarmRecList* pSet;
	BOOL ret;

	pSet = (AlarmRecList*)malloc(sizeof(AlarmRecList));
	memcpy(pSet, g_pAlarmRec, sizeof(AlarmRecList));
	if(g_pAlarmRec->Version & 1)
		ret = WriteServerFile(ALARMRECFILE0, sizeof(AlarmRecList), pSet);
	else 
		ret = WriteServerFile(ALARMRECFILE1, sizeof(AlarmRecList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_pAlarmRec->Version & 1)
			DeleteServerFile(ALARMRECFILE1);
		else
			DeleteServerFile(ALARMRECFILE0);
		g_pAlarmRec->Version++;
	}
	return ret;
}

BOOL InitAlarmRecord(void)
{
	g_alarmRecCS.lockon();

	AlarmRecList* pSet0 = NULL;
	AlarmRecList* pSet1 = NULL;
	char filename[64];
	FILE* fd;

	sprintf(filename, "%s/%s", USERDIR, ALARMRECFILE0);

	fd = fopen(filename, "rb");
	if(fd !=NULL)
	{
		pSet0 = (AlarmRecList*)malloc(sizeof(AlarmRecList));
		if(fread(pSet0, 1, sizeof(AlarmRecList), fd) != sizeof(AlarmRecList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != ALARMRECENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}
	sprintf(filename, "%s/%s", USERDIR ,ALARMRECFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (AlarmRecList*)malloc(sizeof(AlarmRecList));
		if(fread(pSet1, 1, sizeof(AlarmRecList), fd)!= sizeof(AlarmRecList))
		{
			free(pSet1);
			pSet1= NULL;
		}
		else
		{
			if(pSet1->Endid != ALARMRECENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}
	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pAlarmRec = (AlarmRecList*)malloc(sizeof(AlarmRecList));
		memset(g_pAlarmRec, 0, sizeof(AlarmRecList));
		g_pAlarmRec->Endid = ALARMRECENDID;
		UpdateAlarmRecord();
	}
	else 
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->Version > pSet1->Version)
			{
				g_pAlarmRec = pSet0;
				free(pSet1);
			}
			else
			{
				g_pAlarmRec = pSet1;
				free(pSet0);
			}
		}
		else if(pSet0 != NULL)
			g_pAlarmRec = pSet0;
		else 
			g_pAlarmRec = pSet1;
		g_pAlarmRec->Version++;
	}

	g_alarmRecCS.lockoff();
	return TRUE;
}

void AddAlarmRecord(BYTE area, BYTE type)
{
	g_alarmRecCS.lockon();

	AlarmRecord item;
	item.area = area;
	item.type = type;

	SYSTEMTIME stime;
	DPGetLocalTime(&stime);
	DPSystemTimeToFileTime(&stime,&item.time);

	memmove( &g_pAlarmRec->rec[1], &g_pAlarmRec->rec[0], sizeof(AlarmRecord) * (MAX_ALARM_RECORD_NUMBER - 1) );
	memcpy( &g_pAlarmRec->rec[0], &item, sizeof(AlarmRecord) );
	if(g_pAlarmRec->count < MAX_ALARM_RECORD_NUMBER)
		g_pAlarmRec->count++;
	UpdateAlarmRecord();

	g_alarmRecCS.lockoff();
	return;
}

void DeleteAlarmRecord(DWORD index)
{
	g_alarmRecCS.lockon();
	if(index < g_pAlarmRec->count)
	{
		if(index != g_pAlarmRec->count - 1)
			memmove( &g_pAlarmRec->rec[index], &g_pAlarmRec->rec[index+1], sizeof(AlarmRecord) * (g_pAlarmRec->count - index - 1) );
		g_pAlarmRec->count--;
		UpdateAlarmRecord();
	}
	g_alarmRecCS.lockoff();
}

void ResetAlarmRecord()
{
	g_alarmRecCS.lockon();
	memset(g_pAlarmRec, 0, sizeof(AlarmRecList));
	g_pAlarmRec->Endid = ALARMRECENDID;
	UpdateAlarmRecord();
	g_alarmRecCS.lockoff();
}

DWORD GetAlarmRecordCount()
{
	return g_pAlarmRec->count;
}

void GetAlarmRecordData(DWORD index, AlarmRecord* pItem)
{
	g_alarmRecCS.lockon();
	if(index >= g_pAlarmRec->count)
		memcpy(pItem, 0, sizeof(AlarmRecord));
	else
		memcpy(pItem, &g_pAlarmRec->rec[index], sizeof(AlarmRecord));
	g_alarmRecCS.lockoff();
}
