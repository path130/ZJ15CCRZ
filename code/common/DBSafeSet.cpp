#include "roomlib.h"

#define SAFESETFILE0		"SafeSet0.ext"
#define SAFESETFILE1		"SafeSet1.ext"
#define SAFESETENDID		0x53414653	

#define	DEFAULT_SETDELAY		60
#define	DEFAULT_ALARMDELAY		60
#define	DEFAULT_ALARMDURING		300000

static SafeSet *g_pSafeSet = NULL;
static StaticLock g_safeSetCs;

void InitDefaultSafeSet(void)
{
	memset(g_pSafeSet, 0, sizeof(SafeSet));

	g_pSafeSet->m_isSetDefense = FALSE;
	g_pSafeSet->Endid = SAFESETENDID;

	for(int i = 0; i < SAFE_MAX_NUMBER; i++)
	{
		g_pSafeSet->m_areaSet[i].Area = SAREA_KITCHEN;
		g_pSafeSet->m_areaSet[i].Type = STYPE_URGENT;
		g_pSafeSet->m_areaSet[i].OnOff = FALSE;
		g_pSafeSet->m_areaSet[i].Level = 0;
		g_pSafeSet->m_areaSet[i].Status = SSTATUS_SET_DEFENSE;
	}
}

BOOL InitSafeSet()
{
	SafeSet* pSet0 = NULL;
	SafeSet* pSet1 = NULL;
	char filename[64];
	FILE* fd;

	g_safeSetCs.lockon();
	if(g_pSafeSet != NULL)
	{
		free(g_pSafeSet);
		g_pSafeSet = NULL;
	}
	sprintf(filename, "%s/%s", USERDIR, SAFESETFILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (SafeSet*)malloc(sizeof(SafeSet));
		if(fread(pSet0, 1, sizeof(SafeSet), fd) != sizeof(SafeSet))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != SAFESETENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}

	sprintf(filename, "%s/%s", USERDIR, SAFESETFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (SafeSet*)malloc(sizeof(SafeSet));
		if(fread(pSet1, 1, sizeof(SafeSet), fd) != sizeof(SafeSet))
		{
			free(pSet1);
			pSet1 = NULL;
		}
		else
		{
			if(pSet1->Endid != SAFESETENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}

	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pSafeSet = (SafeSet*)malloc(sizeof(SafeSet));
		InitDefaultSafeSet();
		UpdataSafeSet();
	}
	else
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->VERSION > pSet1->VERSION)
			{
				g_pSafeSet = pSet0;
				free(pSet1);
			}
			else
			{
				g_pSafeSet = pSet1;
				free(pSet0);
			}
		}
		else if(pSet0 != NULL)
			g_pSafeSet = pSet0;
		else
			g_pSafeSet = pSet1;
		g_pSafeSet->VERSION++;
	}

	for(int i = 0; i < SAFE_MAX_NUMBER; i++)
	{
		if(g_pSafeSet->m_areaSet[i].Status > SSTATUS_SET_DEFENSE)
			g_pSafeSet->m_areaSet[i].Status = SSTATUS_SET_DEFENSE;
	}

	// 获取一次延时设置
	g_pSafeSet->m_setDelay = GetDelay(DELAY_SAFE);
	g_pSafeSet->m_alarmDelay = GetDelay(DELAY_ALARM);
	g_pSafeSet->m_alarmDuration = GetDelay(DELAY_DURATION) * 60 * 1000;

	g_safeSetCs.lockoff();
	return TRUE;
}

BOOL UpdataSafeSet(void)
{
	SafeSet* pSet;
	BOOL ret;

	pSet = (SafeSet*)malloc(sizeof(SafeSet));
	memcpy(pSet, g_pSafeSet, sizeof(SafeSet));
	if(g_pSafeSet->VERSION & 1)
		ret = WriteServerFile(SAFESETFILE1, sizeof(SafeSet), pSet);
	else
		ret = WriteServerFile(SAFESETFILE0, sizeof(SafeSet), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_pSafeSet->VERSION & 1)
			DeleteServerFile(SAFESETFILE0);
		else
			DeleteServerFile(SAFESETFILE1);
		g_pSafeSet->VERSION++;
	}
	return ret;
}

void GetSafeAreaSet(AreaSet* pset)
{
	g_safeSetCs.lockon();
	if(g_pSafeSet != NULL)
	{
		memcpy(pset, g_pSafeSet->m_areaSet, sizeof(AreaSet) * SAFE_MAX_NUMBER);
	}
	g_safeSetCs.lockoff();
}

BOOL SetSafeAreaSet(AreaSet* pset)
{
	BOOL ret = FALSE;
	if( GetDefenseIsAlarming() == FALSE && GetDefenseStatus() == FALSE )		//如果同步了布防或在报警状态中
	{
		g_safeSetCs.lockon();
		if(g_pSafeSet != NULL)
		{
			memcpy(g_pSafeSet->m_areaSet, pset, sizeof(AreaSet) * SAFE_MAX_NUMBER);
			UpdataSafeSet();
			ret = TRUE;
		}
		g_safeSetCs.lockoff();
	}
	return ret;
}

void ResetSafeSet(void)
{
	g_safeSetCs.lockon();
	InitDefaultSafeSet();
	UpdataSafeSet();
	g_safeSetCs.lockoff();
}

void GetSafeSet(SafeSet** pset)
{
	g_safeSetCs.lockon();
	if(g_pSafeSet != NULL)
	{
		*pset = g_pSafeSet;
	}
	g_safeSetCs.lockoff();
}

static void SetDefense()
{
	g_safeSetCs.lockon();
	if(g_pSafeSet != NULL)
	{
		if(g_pSafeSet->m_isSetDefense == FALSE)
		{
			for(int i = 0; i < SAFE_MAX_NUMBER; i++)
			{
				if(g_pSafeSet->m_areaSet[i].Status == SSTATUS_CANCEL_DEFENSE)
					g_pSafeSet->m_areaSet[i].Status = SSTATUS_SET_DEFENSE;
			}
		}
		g_pSafeSet->m_isSetDefense = TRUE;
		UpdataSafeSet();
	}
	g_safeSetCs.lockoff();
}

static void CancelDefense()
{
	g_safeSetCs.lockon();
	if(g_pSafeSet != NULL)
	{
		for(int i = 0; i < SAFE_MAX_NUMBER; i++)
		{
			if(g_pSafeSet->m_areaSet[i].Type < STYPE_MAGNETIC || g_pSafeSet->m_areaSet[i].Type > STYPE_MAX)
				g_pSafeSet->m_areaSet[i].Status = SSTATUS_SET_DEFENSE;
			else
				g_pSafeSet->m_areaSet[i].Status = SSTATUS_CANCEL_DEFENSE;
		}
		g_pSafeSet->m_isSetDefense = FALSE;
		UpdataSafeSet();
	}
	g_safeSetCs.lockoff();
}

BOOL GetDefenseIsAlarming()
{
	BOOL ret = FALSE;
	g_safeSetCs.lockon();
	if(g_pSafeSet != NULL)
	{
		for(int i = 0; i < SAFE_MAX_NUMBER; i++)
		{
			if(g_pSafeSet->m_areaSet[i].Status == SSTATUS_ALARM_DELAY || g_pSafeSet->m_areaSet[i].Status == SSTATUS_ALARMING)
			{
				if(g_pSafeSet->m_areaSet[i].Type != STYPE_URGENT)
					ret = TRUE;
			}
		}
	}
	g_safeSetCs.lockoff();
	return ret;
}

void SetDefenseStatus(BOOL isSetDefense)
{
	if(isSetDefense)
		SetDefense();
	else
		CancelDefense();
	SendXmlSyncMsg((isSetDefense ? SET_DEFENSE_REPORT : CANCEL_DEFENSE_REPORT), 0, 0, 0);
}

BOOL GetDefenseStatus()
{
	BOOL ret;
	g_safeSetCs.lockon();
	if(g_pSafeSet != NULL)
	{
		ret = g_pSafeSet->m_isSetDefense;
	}
	g_safeSetCs.lockoff();
	return ret;
}

void SetSafeMode(int mode)
{
	SetDefenseStatus(mode != SMODE_UNSET);
}

int GetSafeMode()
{
	return GetDefenseStatus() ? SMODE_LEAVE : SMODE_UNSET;
}

void* GetSafeSetCS()
{
	return &g_safeSetCs;
}

void WriteFileSafeSet()
{
	char fileName[64];
	if(g_pSafeSet->VERSION & 1)
		sprintf(fileName, "%s/%s", USERDIR, SAFESETFILE1);
	else
		sprintf(fileName, "%s/%s", USERDIR, SAFESETFILE0);

	DBGMSG(DPINFO, "WriteFile %s\r\n", fileName);
	FILE* pFile = fopen(fileName, "wb");
	if(pFile)
	{
		fwrite(g_pSafeSet, 1, sizeof(SafeSet), pFile);
		fclose(pFile);

		if(g_pSafeSet->VERSION & 1)
			DeleteServerFile(SAFESETFILE0);
		else
			DeleteServerFile(SAFESETFILE1);
		g_pSafeSet->VERSION++;
	}
}