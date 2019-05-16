#include "roomlib.h"
#include "DPFilterSilk.h"
#include "dpsession.h"

#define LIUYINGFILE0			"Liuying0.ext"
#define LIUYINGFILE1			"Liuying1.ext"
#define	LIUYINGDIR				"Liuying"
#define	LIUYANDIR				"Liuyan"
#define LIUYINGENDID			0x5a674784

typedef struct
{
	LiuyingRecord rec[MAX_LIUYING_NUMBER];
	DWORD count;
	DWORD countunread;	
	DWORD VERSION;
	DWORD Endid;
}LiuyingList;

static LiuyingList* g_LiuyingList = NULL;
static StaticLock g_LiuyingCS;

static BOOL CheckSpaceOfAddLiuying(int len)
{
	UINT64 freeSize;

	do
	{
		freeSize = CheckSpareSpace(USERDIR);
		if(freeSize > len)
			return TRUE;

		if(g_LiuyingList->count == 0)
			return FALSE;

		DeleteLiuying(g_LiuyingList->count - 1);
	}while(1);
}

static void UpdateLiuying(void)
{
	LiuyingList* pSet;
	BOOL ret;

	pSet = (LiuyingList*)malloc(sizeof(LiuyingList));
	memcpy(pSet, g_LiuyingList, sizeof(LiuyingList));
	if(g_LiuyingList->VERSION & 1)
		ret = WriteServerFile(LIUYINGFILE1, sizeof(LiuyingList), pSet);
	else
		ret = WriteServerFile(LIUYINGFILE0, sizeof(LiuyingList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_LiuyingList->VERSION & 1)
			DeleteServerFile(LIUYINGFILE0);
		else
			DeleteServerFile(LIUYINGFILE1);
		g_LiuyingList->VERSION++;
	}
}

static DWORD IsLiuyingFileOk(char * pfilename, char* mark)
{
	char filename[32];
	for(int i = 0; i < g_LiuyingList->count; i++)
	{
		sprintf(filename, "%08x%08x.ly", g_LiuyingList->rec[i].time.dwHighDateTime, g_LiuyingList->rec[i].time.dwLowDateTime);
		if(strcmp(filename, pfilename) == 0)
		{
			mark[i] = 1;
			return TRUE;
		}
	}

	return FALSE;
}

static void CheckLiuyingFile(char* mark)
{
	char name[64];
	char findData[64];
	int index;

	sprintf(name, "%s/%s", USERDIR, LIUYINGDIR);
	HANDLE hFile = DPFindFirstFile(name, findData);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if((strcmp(findData, "..") == 0) || (strcmp(findData, ".") == 0))
				continue;

			if(!IsLiuyingFileOk(findData, mark))
			{
				printf("find invalid file.%s\r\n", findData);
				sprintf(name, "%s/%s", LIUYINGDIR, findData); 
				DeleteServerFile(name);
			}
		}while(DPFindNextFile(hFile, findData));
		DPFindClose(hFile);
	}	
}


static DWORD IsLiuyanFileOk(char * pfilename)
{
	char filename[32];
	for(int i = 0; i < g_LiuyingList->count; i++)
	{
		sprintf(filename, "%08x%08x.lw", g_LiuyingList->rec[i].time.dwHighDateTime, g_LiuyingList->rec[i].time.dwLowDateTime);
		if(strcmp(filename, pfilename) == 0)
			return TRUE;
	}

	return FALSE;
}

static void CheckLiuyanFile()
{
	char name[64];
	char findData[64];
	int index;

	sprintf(name, "%s/%s", USERDIR, LIUYANDIR);
	HANDLE hFile = DPFindFirstFile(name, findData);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if((strcmp(findData, "..") == 0) || (strcmp(findData, ".") == 0))
				continue;

			if(!IsLiuyanFileOk(findData))
			{
				printf("find invalid file.%s\r\n", findData);
				sprintf(name, "%s/%s", LIUYANDIR, findData); 
				DeleteServerFile(name);
			}
		}while(DPFindNextFile(hFile, findData));
		DPFindClose(hFile);
	}	
}

void InitLiuying()
{
	LiuyingList* pSet0 = NULL;
	LiuyingList* pSet1 = NULL;
	FILE* fd;
	DWORD i;

	char filename[64];
	sprintf(filename, "%s/%s", USERDIR, LIUYINGDIR);
	CheckAndCreateDir(filename);

	sprintf(filename, "%s/%s", USERDIR, LIUYANDIR);
	CheckAndCreateDir(filename);

	sprintf(filename, "%s/%s", USERDIR, LIUYINGFILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (LiuyingList*)malloc(sizeof(LiuyingList));
		memset(pSet0, 0, sizeof(LiuyingList));
		if(fread(pSet0, 1, sizeof(LiuyingList), fd) != sizeof(LiuyingList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != LIUYINGENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}

	sprintf(filename, "%s/%s", USERDIR, LIUYINGFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (LiuyingList*)malloc(sizeof(LiuyingList));
		memset(pSet1, 0, sizeof(LiuyingList));
		if(fread(pSet1, 1, sizeof(LiuyingList), fd) != sizeof(LiuyingList))
		{
			free(pSet1);
			pSet1 = NULL;
		}
		else
		{
			if(pSet1->Endid != LIUYINGENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}

	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_LiuyingList = (LiuyingList*)malloc(sizeof(LiuyingList));
		memset(g_LiuyingList, 0, sizeof(LiuyingList));
		g_LiuyingList->Endid = LIUYINGENDID;
		UpdateLiuying();
	}
	else if((pSet0 != NULL) && (pSet1 != NULL))
	{
		if(pSet0->VERSION > pSet1->VERSION)
		{
			g_LiuyingList = pSet0;
			free(pSet1);
		}
		else
		{
			g_LiuyingList = pSet1;
			free(pSet0);
		}
		g_LiuyingList->VERSION++;
	}
	else if(pSet0 != NULL)
	{
		g_LiuyingList = pSet0;
		g_LiuyingList->VERSION++;
	}
	else
	{
		g_LiuyingList = pSet1;
		g_LiuyingList->VERSION++;
	}

	char mark[MAX_LIUYING_NUMBER];
	CheckLiuyingFile(mark);
	CheckLiuyanFile();

	g_LiuyingList->countunread = 0;
	for(i = 0; i < g_LiuyingList->count; )
	{
		if(mark[i] == 0)
		{
			memmove(&mark[i], &mark[i + 1], (g_LiuyingList->count - i - 1));
			memmove(&g_LiuyingList->rec[i], &g_LiuyingList->rec[i + 1], sizeof(LiuyingRecord) * (g_LiuyingList->count - i - 1));
			g_LiuyingList->count--;
		}
		else
		{
			if(g_LiuyingList->rec[i].bRead == 0)
				g_LiuyingList->countunread++;
			i++;
		}
	}
}

void ResetLiuying()
{
	g_LiuyingCS.lockon();
	memset(g_LiuyingList, 0, sizeof(LiuyingList));
	g_LiuyingList->Endid = LIUYINGENDID;
	UpdateLiuying();
	char mark[MAX_LIUYING_NUMBER];
	CheckLiuyingFile(mark);
	CheckLiuyanFile();
	g_LiuyingCS.lockoff();
}

DWORD GetLiuyingCount()
{
	return g_LiuyingList->count;
}

DWORD GetLiuyingUnreadCount()
{
	return g_LiuyingList->countunread;
}

BOOL AddLiuying(char* code, DWORD len, char* pdata, DWORD enctype)
{
	if(!CheckSpaceOfAddLiuying(len))
		return FALSE;

	SYSTEMTIME systime = {0};
	FILETIME ftime;
	DPGetLocalTime(&systime);
	DPSystemTimeToFileTime(&systime, &ftime); 

	char filename[128];
	sprintf(filename, "%s/%08x%08x.ly", LIUYINGDIR, ftime.dwHighDateTime, ftime.dwLowDateTime);

	g_LiuyingCS.lockon();
	if(WriteServerFile(filename, len, pdata))
	{
		if(g_LiuyingList->count == MAX_LIUYING_NUMBER)
		{
			//删除最旧一条
			sprintf(filename, "%s/%08x%08x.ly", LIUYINGDIR, g_LiuyingList->rec[MAX_LIUYING_NUMBER - 1].time.dwHighDateTime, g_LiuyingList->rec[MAX_LIUYING_NUMBER - 1].time.dwLowDateTime);
			DeleteServerFile(filename);
			g_LiuyingList->count--;
			if(g_LiuyingList->rec[MAX_LIUYING_NUMBER - 1].bRead == 0)
				g_LiuyingList->countunread--;

		}
		memmove(&g_LiuyingList->rec[1], &g_LiuyingList->rec[0], sizeof(LiuyingRecord)*(MAX_LIUYING_NUMBER-1));

		// 保存留言
		DPTransLiuyanWait();
		sprintf(filename, "%s/%s/%08x%08x.lw", USERDIR, LIUYANDIR, ftime.dwHighDateTime, ftime.dwLowDateTime);
		DPMoveFile(filename, LIUYAN_FILE_PATH);

		g_LiuyingList->rec[0].time = ftime;
		strncpy(g_LiuyingList->rec[0].code, code, sizeof(g_LiuyingList->rec[0].code));
		g_LiuyingList->rec[0].enctype = enctype;
		g_LiuyingList->rec[0].bRead = 0;
		g_LiuyingList->count++;
		g_LiuyingList->countunread++;
		UpdateLiuying();
	}
	else
	{
		free(pdata);
	}
	g_LiuyingCS.lockoff();
	return TRUE;
}

void DeleteLiuying(int index)
{
	char filename[64];
	if(index >= g_LiuyingList->count)
		return;

	g_LiuyingCS.lockon();
	sprintf(filename, "%s/%08x%08x.ly", LIUYINGDIR, g_LiuyingList->rec[index].time.dwHighDateTime, g_LiuyingList->rec[index].time.dwLowDateTime);
	DeleteServerFile(filename);

	sprintf(filename, "%s/%08x%08x.lw", LIUYANDIR, g_LiuyingList->rec[index].time.dwHighDateTime, g_LiuyingList->rec[index].time.dwLowDateTime);
	DeleteServerFile(filename);

	if(g_LiuyingList->rec[index].bRead == 0)
		g_LiuyingList->countunread--;
	memmove(&g_LiuyingList->rec[index] , &g_LiuyingList->rec[index + 1] , (g_LiuyingList->count - index - 1)*sizeof(LiuyingRecord));
	g_LiuyingList->count--;
	UpdateLiuying();
	g_LiuyingCS.lockoff();
}

int GetLiuyingRecord(LiuyingRecord* rec)
{
	int count = 0;
	g_LiuyingCS.lockon();
	count = g_LiuyingList->count;
	memcpy(rec, g_LiuyingList->rec, MAX_LIUYING_NUMBER * sizeof(LiuyingRecord));
	g_LiuyingCS.lockoff();
	return count;
}

static int CreateLiuyanFile(int index, char** pBuf)
{
	char filename[64];
	sprintf(filename, "%s/%s/%08x%08x.lw", USERDIR, LIUYANDIR, g_LiuyingList->rec[index].time.dwHighDateTime, g_LiuyingList->rec[index].time.dwLowDateTime);

	FILE* pFile = fopen(filename, "rb");
	if(pFile == NULL)
		return 0;

	int nCount = 0;
	fread(&nCount, 1, 4, pFile);

	char* pbuf = (char*)malloc(nCount * 640);
	char* pdata = pbuf;

	char buf[640];
	int len = 0;
	int pcmLen = 0;
	HANDLE hSilkDec = SilkDecCreate();
	
	while(1)
	{
		if(2 != fread(&len, 1, 2, pFile))
			break;

		fread(buf, 1, len, pFile);
		pcmLen = SilkDecRun(hSilkDec, buf, len, pdata);
		pdata += pcmLen * 2;
	}

	SilkDecDestroy(hSilkDec);
	fclose(pFile);

	*pBuf = pbuf;
	return nCount * 640;
}

int GetLiuyingData(int index, char** buf, int* lwLen, char** pLwBuf)
{
	if(index >= g_LiuyingList->count)
	{
		*buf = NULL;
		return 0;
	}

	*pLwBuf = NULL;

	g_LiuyingCS.lockon();
	char filename[64];
	sprintf(filename, "%s/%s/%08x%08x.ly", USERDIR, LIUYINGDIR, g_LiuyingList->rec[index].time.dwHighDateTime, g_LiuyingList->rec[index].time.dwLowDateTime);
	char* pbuf;
	int len = BReadFile(filename, &pbuf);
	if(len > 0)
	{
		*buf = pbuf;
		*lwLen = CreateLiuyanFile(index, pLwBuf);
	}
	else
		*buf = NULL;

	if(!g_LiuyingList->rec[index].bRead)
	{
		g_LiuyingList->rec[index].bRead = TRUE;
		g_LiuyingList->countunread--;
		UpdateLiuying();
	}
	g_LiuyingCS.lockoff();
	return len;
}