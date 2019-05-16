//#include "roomlib.h"
//
//#define LIUYANFILE0				"Liuyan0.ext"
//#define LIUYANFILE1				"Liuyan1.ext"
//#define	LIUYANDIR				"Liuyan"
//#define LIUYANENDID				0x5a876684
//
//typedef struct
//{
//	LiuyanRecord rec[MAX_LIUYAN_NUMBER];
//	DWORD count;
//	DWORD countunread;	
//	DWORD VERSION;
//	DWORD Endid;
//}LiuyanList;
//
//static LiuyanList* g_LiuyanList = NULL;
//static StaticLock g_LiuyanCS;
//
//static BOOL CheckSpaceOfAddLiuyan(int len)
//{
//	UINT64 freeSize;
//
//	do
//	{
//		freeSize = CheckSpareSpace(USERDIR);
//		if(freeSize > len)
//			return TRUE;
//
//		if(g_LiuyanList->count == 0)
//			return FALSE;
//
//		DeleteLiuyan(g_LiuyanList->count - 1);
//	}while(1);
//}
//
//static void UpdateLiuyan(void)
//{
//	LiuyanList* pSet;
//	BOOL ret;
//
//	pSet = (LiuyanList*)malloc(sizeof(LiuyanList));
//	memcpy(pSet, g_LiuyanList, sizeof(LiuyanList));
//	if(g_LiuyanList->VERSION & 1)
//		ret = WriteServerFile(LIUYANFILE1, sizeof(LiuyanList), pSet);
//	else
//		ret = WriteServerFile(LIUYANFILE0, sizeof(LiuyanList), pSet);
//	if(!ret)
//		free(pSet);
//	else
//	{
//		if(g_LiuyanList->VERSION & 1)
//			DeleteServerFile(LIUYANFILE0);
//		else
//			DeleteServerFile(LIUYANFILE1);
//		g_LiuyanList->VERSION++;
//	}
//}
//
//static DWORD IsLiuyanFileOk(char * pfilename, char* mark)
//{
//	char filename[32];
//	DWORD i;
//
//	for(int i = 0; i < g_LiuyanList->count; i++)
//	{
//		sprintf(filename, "%08x%08x.dat", g_LiuyanList->rec[i].time.dwHighDateTime, g_LiuyanList->rec[i].time.dwLowDateTime);
//		if(strcmp(filename, pfilename) == 0)
//		{
//			mark[i] = 1;
//			return TRUE;
//		}
//	}
//
//	return FALSE;
//}
//
//static void CheckLiuyanFile(char* mark)
//{
//	char name[64];
//	char findData[64];
//	int index;
//
//	sprintf(name, "%s/%s", USERDIR, LIUYANDIR);
//	HANDLE hFile = DPFindFirstFile(name, findData);
//	if (hFile != INVALID_HANDLE_VALUE)
//	{
//		do
//		{
//			if((strcmp(findData, "..") == 0) || (strcmp(findData, ".") == 0))
//				continue;
//
//			if(!IsLiuyanFileOk(findData, mark))
//			{
//				printf("find invalid file.%s\r\n", findData);
//				sprintf(name, "%s/%s", LIUYANDIR, findData); 
//				DeleteServerFile(name);
//			}
//		}while(DPFindNextFile(hFile, findData));
//		DPFindClose(hFile);
//	}	
//}
//
//void InitLiuyan()
//{
//	LiuyanList* pSet0 = NULL;
//	LiuyanList* pSet1 = NULL;
//	FILE* fd;
//	DWORD i;
//
//	char filename[64];
//	sprintf(filename, "%s/%s", USERDIR, LIUYANDIR);
//	CheckAndCreateDir(filename);
//
//	sprintf(filename, "%s/%s", USERDIR, LIUYANFILE0);
//	fd = fopen(filename, "rb");
//	if(fd != NULL)
//	{
//		pSet0 = (LiuyanList*)malloc(sizeof(LiuyanList));
//		memset(pSet0, 0, sizeof(LiuyanList));
//		if(fread(pSet0, 1, sizeof(LiuyanList), fd) != sizeof(LiuyanList))
//		{
//			free(pSet0);
//			pSet0 = NULL;
//		}
//		else
//		{
//			if(pSet0->Endid != LIUYANENDID)
//			{
//				free(pSet0);
//				pSet0 = NULL;
//			}
//		}
//		fclose(fd);
//	}
//
//	sprintf(filename, "%s/%s", USERDIR, LIUYANFILE1);
//	fd = fopen(filename, "rb");
//	if(fd != NULL)
//	{
//		pSet1 = (LiuyanList*)malloc(sizeof(LiuyanList));
//		memset(pSet1, 0, sizeof(LiuyanList));
//		if(fread(pSet1, 1, sizeof(LiuyanList), fd) != sizeof(LiuyanList))
//		{
//			free(pSet1);
//			pSet1 = NULL;
//		}
//		else
//		{
//			if(pSet1->Endid != LIUYANENDID)
//			{
//				free(pSet1);
//				pSet1 = NULL;
//			}
//		}
//		fclose(fd);
//	}
//
//	if((pSet0 == NULL) && (pSet1 == NULL))
//	{
//		g_LiuyanList = (LiuyanList*)malloc(sizeof(LiuyanList));
//		memset(g_LiuyanList, 0, sizeof(LiuyanList));
//		g_LiuyanList->Endid = LIUYANENDID;
//		UpdateLiuyan();
//	}
//	else if((pSet0 != NULL) && (pSet1 != NULL))
//	{
//		if(pSet0->VERSION > pSet1->VERSION)
//		{
//			g_LiuyanList = pSet0;
//			free(pSet1);
//		}
//		else
//		{
//			g_LiuyanList = pSet1;
//			free(pSet0);
//		}
//		g_LiuyanList->VERSION++;
//	}
//	else if(pSet0 != NULL)
//	{
//		g_LiuyanList = pSet0;
//		g_LiuyanList->VERSION++;
//	}
//	else
//	{
//		g_LiuyanList = pSet1;
//		g_LiuyanList->VERSION++;
//	}
//
//	char mark[MAX_LIUYAN_NUMBER];
//	CheckLiuyanFile(mark);
//
//	g_LiuyanList->countunread = 0;
//	for(i = 0; i < g_LiuyanList->count; )
//	{
//		if(mark[i] == 0)
//		{
//			memmove(&mark[i], &mark[i + 1], (g_LiuyanList->count - i - 1));
//			memmove(&g_LiuyanList->rec[i], &g_LiuyanList->rec[i + 1], sizeof(LiuyanRecord) * (g_LiuyanList->count - i - 1));
//			g_LiuyanList->count--;
//		}
//		else
//		{
//			if(g_LiuyanList->rec[i].bRead == 0)
//				g_LiuyanList->countunread++;
//			i++;
//		}
//	}
//}
//
//void ResetLiuyan()
//{
//	g_LiuyanCS.lockon();
//	memset(g_LiuyanList, 0, sizeof(LiuyanList));
//	g_LiuyanList->Endid = LIUYANENDID;
//	UpdateLiuyan();
//	char mark[MAX_LIUYAN_NUMBER];
//	CheckLiuyanFile(mark);
//	g_LiuyanCS.lockoff();
//}
//
//DWORD GetLiuyanCount()
//{
//	return g_LiuyanList->count;
//}
//
//DWORD GetLiuyanUnreadCount()
//{
//	return g_LiuyanList->countunread;
//}
//
//BOOL AddLiuyan(char* code, DWORD len, char* pdata, DWORD enctype)
//{
//	if(!CheckSpaceOfAddLiuyan(len))
//		return FALSE;
//
//	SYSTEMTIME systime = {0};
//	FILETIME ftime;
//	DPGetLocalTime(&systime);
//	DPSystemTimeToFileTime(&systime, &ftime); 
//
//	char filename[128];
//	sprintf(filename, "%s/%08x%08x.dat", LIUYANDIR, ftime.dwHighDateTime, ftime.dwLowDateTime);
//
//	g_LiuyanCS.lockon();
//	if(WriteServerFile(filename, len, pdata))
//	{
//		if(g_LiuyanList->count == MAX_LIUYAN_NUMBER)
//		{
//			//É¾³ý×î¾ÉÒ»Ìõ
//			sprintf(filename, "%s/%08x%08x.dat", LIUYANDIR, g_LiuyanList->rec[MAX_LIUYAN_NUMBER - 1].time.dwHighDateTime, g_LiuyanList->rec[MAX_LIUYAN_NUMBER - 1].time.dwLowDateTime);
//			DeleteServerFile(filename);
//			g_LiuyanList->count--;
//			if(g_LiuyanList->rec[MAX_LIUYAN_NUMBER - 1].bRead == 0)
//				g_LiuyanList->countunread--;
//
//		}
//		memmove(&g_LiuyanList->rec[1], &g_LiuyanList->rec[0], sizeof(LiuyanRecord)*(MAX_LIUYAN_NUMBER-1));
//
//		g_LiuyanList->rec[0].time = ftime;
//		strncpy(g_LiuyanList->rec[0].code, code, sizeof(g_LiuyanList->rec[0].code));
//		g_LiuyanList->rec[0].enctype = enctype;
//		g_LiuyanList->rec[0].bRead = 0;
//		g_LiuyanList->count++;
//		g_LiuyanList->countunread++;
//		UpdateLiuyan();
//	}
//	else
//	{
//		free(pdata);
//	}
//	g_LiuyanCS.lockoff();
//	return TRUE;
//}
//
//void DeleteLiuyan(int index)
//{
//	char filename[64];
//	if(index >= g_LiuyanList->count)
//		return;
//
//	g_LiuyanCS.lockon();
//	sprintf(filename, "%s/%08x%08x.dat", LIUYANDIR, g_LiuyanList->rec[index].time.dwHighDateTime, g_LiuyanList->rec[index].time.dwLowDateTime);
//	DeleteServerFile(filename);
//	if(g_LiuyanList->rec[index].bRead == 0)
//		g_LiuyanList->countunread--;
//	memmove(&g_LiuyanList->rec[index] , &g_LiuyanList->rec[index + 1] , (g_LiuyanList->count - index - 1)*sizeof(LiuyanRecord));
//	g_LiuyanList->count--;
//	UpdateLiuyan();
//	g_LiuyanCS.lockoff();
//}
//
//int GetLiuyanRecord(LiuyanRecord* rec)
//{
//	int count = 0;
//	g_LiuyanCS.lockon();
//	count = g_LiuyanList->count;
//	memcpy(rec, g_LiuyanList->rec, count * sizeof(LiuyanRecord));
//	g_LiuyanCS.lockoff();
//	return count;
//}
//
//int GetLiuyanData(int index, char** buf)
//{
//	if(index >= g_LiuyanList->count)
//	{
//		*buf = NULL;
//		return 0;
//	}
//
//	g_LiuyanCS.lockon();
//	char filename[64];
//	sprintf(filename, "%s/%s/%08x%08x.dat", USERDIR, LIUYANDIR, g_LiuyanList->rec[index].time.dwHighDateTime, g_LiuyanList->rec[index].time.dwLowDateTime);
//	char* pbuf;
//	int len = BReadFile(filename, &pbuf);
//	if(len > 0)
//		*buf = pbuf;
//	else
//		*buf = NULL;
//	g_LiuyanCS.lockoff();
//	return len;
//}