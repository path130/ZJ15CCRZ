#include "roomlib.h"

#define DEFENSEFILE0		"DefenseRec0.ext"
#define DEFENSEFILE1		"DefenseRec1.ext"
#define DEFENSEENDID		0x53524571

typedef struct
{
	DefenseRecord rec[MAX_DEFENSE_RECORD_NUMBER];
	DWORD count;         		 
	DWORD Version;
	DWORD Endid;
}DefenseRecList;

static DefenseRecList* g_pDefenseRec = NULL;
static StaticLock g_DefenseRecCS;

static BOOL UpdateDefenseRecord(void)
{
	DefenseRecList* pSet;
	BOOL ret;

	pSet = (DefenseRecList*)malloc(sizeof(DefenseRecList));
	memcpy(pSet, g_pDefenseRec, sizeof(DefenseRecList));
	if(g_pDefenseRec->Version & 1)
		ret = WriteServerFile(DEFENSEFILE0, sizeof(DefenseRecList), pSet);
	else 
		ret = WriteServerFile(DEFENSEFILE1, sizeof(DefenseRecList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_pDefenseRec->Version & 1)
			DeleteServerFile(DEFENSEFILE1);
		else
			DeleteServerFile(DEFENSEFILE0);
		g_pDefenseRec->Version++;
	}
	return ret;
}


BOOL InitDefenseRecord(void)
{
	g_DefenseRecCS.lockon();

	DefenseRecList* pSet0 = NULL;
	DefenseRecList* pSet1 = NULL;
	char filename[64];
	FILE* fd;

	if(g_pDefenseRec != NULL)
	{
		free(g_pDefenseRec);
		g_pDefenseRec = NULL;
	}
	sprintf(filename, "%s/%s", USERDIR, DEFENSEFILE0);

	fd = fopen(filename, "rb");
	if(fd !=NULL)
	{
		pSet0 = (DefenseRecList*)malloc(sizeof(DefenseRecList));
		if(fread(pSet0, 1, sizeof(DefenseRecList), fd) != sizeof(DefenseRecList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != DEFENSEENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}
	sprintf(filename, "%s/%s", USERDIR ,DEFENSEFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (DefenseRecList*)malloc(sizeof(DefenseRecList));
		if(fread(pSet1, 1, sizeof(DefenseRecList), fd)!= sizeof(DefenseRecList))
		{
			free(pSet1);
			pSet1= NULL;
		}
		else
		{
			if(pSet1->Endid != DEFENSEENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}
	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pDefenseRec = (DefenseRecList*)malloc(sizeof(DefenseRecList));
		memset(g_pDefenseRec, 0, sizeof(DefenseRecList));
		g_pDefenseRec->Endid = DEFENSEENDID;
		UpdateDefenseRecord();
	}
	else 
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->Version > pSet1->Version)
			{
				g_pDefenseRec = pSet0;
				free(pSet1);
			}
			else
			{
				g_pDefenseRec = pSet1;
				free(pSet0);
			}
		}
		else if(pSet0 != NULL)
			g_pDefenseRec = pSet0;
		else 
			g_pDefenseRec = pSet1;
		g_pDefenseRec->Version++;
	}

	g_DefenseRecCS.lockoff();
	return TRUE;

}

void AddDefenseRecord(BYTE newMode, BYTE oldMode)
{
	g_DefenseRecCS.lockon();

	DefenseRecord item;
	item.newMode = newMode;
	item.oldMode = oldMode;

	SYSTEMTIME stime;
	DPGetLocalTime(&stime);
	DPSystemTimeToFileTime(&stime,&item.time);

	memmove( &g_pDefenseRec->rec[1], &g_pDefenseRec->rec[0], sizeof(DefenseRecord) * (MAX_DEFENSE_RECORD_NUMBER - 1) );
	memcpy( &g_pDefenseRec->rec[0], &item, sizeof(DefenseRecord) );
	if(g_pDefenseRec->count < MAX_DEFENSE_RECORD_NUMBER)
		g_pDefenseRec->count++;
	UpdateDefenseRecord();

	g_DefenseRecCS.lockoff();
	return;
}

void DeleteDefenseRecord(DWORD index)
{
	g_DefenseRecCS.lockon();
	if(index < g_pDefenseRec->count)
	{
		memmove(&g_pDefenseRec->rec[index], &g_pDefenseRec->rec[index + 1], sizeof(DefenseRecord) * (g_pDefenseRec->count - index - 1) );
		g_pDefenseRec->count--;
		UpdateDefenseRecord();
	}
	g_DefenseRecCS.lockoff();
}

void ResetDefenseRecord()
{
	g_DefenseRecCS.lockon();
	memset(g_pDefenseRec, 0, sizeof(DefenseRecList));
	g_pDefenseRec->Endid = DEFENSEENDID;
	UpdateDefenseRecord();
	g_DefenseRecCS.lockoff();
}

DWORD GetDefenseRecordCount()
{
	return g_pDefenseRec->count;
}

void GetDefenseRecordData(DWORD index, DefenseRecord* pItem)
{
	g_DefenseRecCS.lockon();
	memset(pItem, 0, sizeof(DefenseRecord));
	if (index < g_pDefenseRec->count)
		memcpy(pItem, &g_pDefenseRec->rec[index], sizeof(DefenseRecord));
	g_DefenseRecCS.lockoff();
}

void WriteFileDefenseRec()
{
	char fileName[64];
	if(g_pDefenseRec->Version & 1)
		sprintf(fileName, "%s/%s", USERDIR, DEFENSEFILE1);
	else
		sprintf(fileName, "%s/%s", USERDIR, DEFENSEFILE0);

	DBGMSG(DPINFO, "WriteFile %s\r\n", fileName);
	FILE* pFile = fopen(fileName, "wb");
	if(pFile)
	{
		fwrite(g_pDefenseRec, 1, sizeof(DefenseRecList), pFile);
		fclose(pFile);

		if(g_pDefenseRec->Version & 1)
			DeleteServerFile(DEFENSEFILE0);
		else
			DeleteServerFile(DEFENSEFILE1);
		g_pDefenseRec->Version++;
	}
}
