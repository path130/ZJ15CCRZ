#include "roomlib.h"

#define PHOTOFILE0			"Photo0.ext"
#define PHOTOFILE1			"Photo1.ext"
#define	PHOTODIR			"Photo"
#define PHOTOENDID			0x5d674753

typedef struct
{
	PhotoRecord rec[MAX_PHOTO_NUMBER];
	DWORD count;
	DWORD countunread;	
	DWORD VERSION;
	DWORD Endid;
}PhotoList;

static PhotoList* g_PhotoList = NULL;
static StaticLock g_PhotoCS;

static BOOL CheckSpaceOfAddPhoto(int len)
{
	UINT64 freeSize;

	do
	{
		freeSize = CheckSpareSpace(USERDIR);
		if(freeSize > len)
			return TRUE;

		if(g_PhotoList->count == 0)
			return FALSE;

		DeletePhoto(g_PhotoList->count - 1);
	}while(1);
}

static void UpdatePhoto(void)
{
	PhotoList* pSet;
	BOOL ret;

	pSet = (PhotoList*)malloc(sizeof(PhotoList));
	memcpy(pSet, g_PhotoList, sizeof(PhotoList));
	if(g_PhotoList->VERSION & 1)
		ret = WriteServerFile(PHOTOFILE1, sizeof(PhotoList), pSet);
	else
		ret = WriteServerFile(PHOTOFILE0, sizeof(PhotoList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_PhotoList->VERSION & 1)
			DeleteServerFile(PHOTOFILE0);
		else
			DeleteServerFile(PHOTOFILE1);
		g_PhotoList->VERSION++;
	}
}

static DWORD IsPhotoFileOk(char * pfilename, char* mark)
{
	char filename[32];
	DWORD i;

	for(int i = 0; i < g_PhotoList->count; i++)
	{
		sprintf(filename, "%08x%08x.dat", g_PhotoList->rec[i].time.dwHighDateTime, g_PhotoList->rec[i].time.dwLowDateTime);
		if(strcmp(filename, pfilename) == 0)
		{
			mark[i] = 1;
			return TRUE;
		}
	}

	return FALSE;
}

static void CheckPhotoFile(char* mark)
{
	char name[64];
	char findData[64];
	int index;

	sprintf(name, "%s/%s", USERDIR, PHOTODIR);
	HANDLE hFile = DPFindFirstFile(name, findData);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if((strcmp(findData, "..") == 0) || (strcmp(findData, ".") == 0))
				continue;

			if(!IsPhotoFileOk(findData, mark))
			{
				printf("find invalid file.%s\r\n", findData);
				sprintf(name, "%s/%s", PHOTODIR, findData); 
				DeleteServerFile(name);
			}
		}while(DPFindNextFile(hFile, findData));
		DPFindClose(hFile);
	}	
}

void InitPhoto()
{
	PhotoList* pSet0 = NULL;
	PhotoList* pSet1 = NULL;
	FILE* fd;
	DWORD i;

	char filename[64];
	sprintf(filename, "%s/%s", USERDIR, PHOTODIR);
	CheckAndCreateDir(filename);

	sprintf(filename, "%s/%s", USERDIR, PHOTOFILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (PhotoList*)malloc(sizeof(PhotoList));
		memset(pSet0, 0, sizeof(PhotoList));
		if(fread(pSet0, 1, sizeof(PhotoList), fd) != sizeof(PhotoList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != PHOTOENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}

	sprintf(filename, "%s/%s", USERDIR, PHOTOFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (PhotoList*)malloc(sizeof(PhotoList));
		memset(pSet1, 0, sizeof(PhotoList));
		if(fread(pSet1, 1, sizeof(PhotoList), fd) != sizeof(PhotoList))
		{
			free(pSet1);
			pSet1 = NULL;
		}
		else
		{
			if(pSet1->Endid != PHOTOENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}

	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_PhotoList = (PhotoList*)malloc(sizeof(PhotoList));
		memset(g_PhotoList, 0, sizeof(PhotoList));
		g_PhotoList->Endid = PHOTOENDID;
		UpdatePhoto();
	}
	else if((pSet0 != NULL) && (pSet1 != NULL))
	{
		if(pSet0->VERSION > pSet1->VERSION)
		{
			g_PhotoList = pSet0;
			free(pSet1);
		}
		else
		{
			g_PhotoList = pSet1;
			free(pSet0);
		}
		g_PhotoList->VERSION++;
	}
	else if(pSet0 != NULL)
	{
		g_PhotoList = pSet0;
		g_PhotoList->VERSION++;
	}
	else
	{
		g_PhotoList = pSet1;
		g_PhotoList->VERSION++;
	}

	char mark[MAX_PHOTO_NUMBER];
	CheckPhotoFile(mark);

	g_PhotoList->countunread = 0;
	for(i = 0; i < g_PhotoList->count; )
	{
		if(mark[i] == 0)
		{
			memmove(&mark[i], &mark[i + 1], (g_PhotoList->count - i - 1));
			memmove(&g_PhotoList->rec[i], &g_PhotoList->rec[i + 1], sizeof(PhotoRecord) * (g_PhotoList->count - i - 1));
			g_PhotoList->count--;
		}
		else
		{
			if(g_PhotoList->rec[i].bRead == 0)
				g_PhotoList->countunread++;
			i++;
		}
	}
}

void ResetPhoto()
{
	g_PhotoCS.lockon();
	memset(g_PhotoList, 0, sizeof(PhotoList));
	g_PhotoList->Endid = PHOTOENDID;
	UpdatePhoto();
	char mark[MAX_PHOTO_NUMBER];
	CheckPhotoFile(mark);
	g_PhotoCS.lockoff();
}

DWORD GetPhotoCount()
{
	return g_PhotoList->count;
}

DWORD GetPhotoUnreadCount()
{
	return g_PhotoList->countunread;
}

BOOL AddPhoto(char* name, DWORD len, char* pdata, DWORD enctype)
{
	if(!CheckSpaceOfAddPhoto(len))
		return FALSE;

	SYSTEMTIME systime = {0};
	FILETIME ftime;
	DPGetLocalTime(&systime);
	DPSystemTimeToFileTime(&systime, &ftime); 

	char filename[128];
	sprintf(filename, "%s/%08x%08x.dat", PHOTODIR, ftime.dwHighDateTime, ftime.dwLowDateTime);

	g_PhotoCS.lockon();
	if(WriteServerFile(filename, len, pdata))
	{
		if(g_PhotoList->count == MAX_PHOTO_NUMBER)
		{
			//É¾³ý×î¾ÉÒ»Ìõ
			sprintf(filename, "%s/%08x%08x.dat", PHOTODIR, g_PhotoList->rec[MAX_PHOTO_NUMBER - 1].time.dwHighDateTime, g_PhotoList->rec[MAX_PHOTO_NUMBER - 1].time.dwLowDateTime);
			DeleteServerFile(filename);
			g_PhotoList->count--;
			if(g_PhotoList->rec[MAX_PHOTO_NUMBER - 1].bRead == 0)
				g_PhotoList->countunread--;

		}
		memmove(&g_PhotoList->rec[1], &g_PhotoList->rec[0], sizeof(PhotoRecord)*(MAX_PHOTO_NUMBER-1));

		g_PhotoList->rec[0].time = ftime;
		strncpy(g_PhotoList->rec[0].name, name, sizeof(g_PhotoList->rec[0].name));
		g_PhotoList->rec[0].enctype = enctype;
		g_PhotoList->rec[0].bRead = 0;
		g_PhotoList->count++;
		g_PhotoList->countunread++;
		UpdatePhoto();

		StartPlayMp3(PHOTO_MP3, GetRingVol(), 1);
	}
	else
	{
		free(pdata);
	}
	g_PhotoCS.lockoff();
	return TRUE;
}

void DeletePhoto(int index)
{
	char filename[64];
	if(index >= g_PhotoList->count)
		return;

	g_PhotoCS.lockon();
	sprintf(filename, "%s/%08x%08x.dat", PHOTODIR, g_PhotoList->rec[index].time.dwHighDateTime, g_PhotoList->rec[index].time.dwLowDateTime);
	DeleteServerFile(filename);
	if(g_PhotoList->rec[index].bRead == 0)
		g_PhotoList->countunread--;
	memmove(&g_PhotoList->rec[index] , &g_PhotoList->rec[index + 1] , (g_PhotoList->count - index - 1)*sizeof(PhotoRecord));
	g_PhotoList->count--;
	UpdatePhoto();
	g_PhotoCS.lockoff();
}

int GetPhotoRecord(PhotoRecord* rec)
{
	int count = 0;
	g_PhotoCS.lockon();
	count = g_PhotoList->count;
	memcpy(rec, g_PhotoList->rec, count * sizeof(PhotoRecord));
	g_PhotoCS.lockoff();
	return count;
}

int GetPhotoData(int index, char** buf)
{
	if(index >= g_PhotoList->count)
	{
		*buf = NULL;
		return 0;
	}

	g_PhotoCS.lockon();
	char filename[64];
	sprintf(filename, "%s/%s/%08x%08x.dat", USERDIR, PHOTODIR, g_PhotoList->rec[index].time.dwHighDateTime, g_PhotoList->rec[index].time.dwLowDateTime);
	char* pbuf;
	int len = BReadFile(filename, &pbuf);
	if(len > 0)
		*buf = pbuf;
	else
		*buf = NULL;

	if(!g_PhotoList->rec[index].bRead)
	{
		g_PhotoList->rec[index].bRead = TRUE;
		g_PhotoList->countunread--;
		UpdatePhoto();
	}

	g_PhotoCS.lockoff();
	return len;
}