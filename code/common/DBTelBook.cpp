#include "roomlib.h"

#define TelBookRecFILE0		"TelBookRec0.ext"
#define TelBookRecFILE1		"TelBookRec1.ext"
#define TelBookRecENDID     0x53524572

typedef struct
{
	TelBookRecord rec[MAX_TELBOOK_NUMBER];
	DWORD count;         		 
	DWORD Version;
	DWORD Endid;
}TelBookRecList;

static TelBookRecList* g_pTelBookRec = NULL;
static StaticLock g_TelBookRecCS;

static BOOL UpdateTelBook(void)
{
	TelBookRecList* pSet;
	BOOL ret;

	pSet = (TelBookRecList*)malloc(sizeof(TelBookRecList));
	memcpy(pSet, g_pTelBookRec, sizeof(TelBookRecList));
	if(g_pTelBookRec->Version & 1)
		ret = WriteServerFile(TelBookRecFILE0, sizeof(TelBookRecList), pSet);
	else 
		ret = WriteServerFile(TelBookRecFILE1, sizeof(TelBookRecList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_pTelBookRec->Version & 1)
			DeleteServerFile(TelBookRecFILE1);
		else
			DeleteServerFile(TelBookRecFILE0);
		g_pTelBookRec->Version++;
	}
	return ret;
}

BOOL InitTelBook(void)
{
	g_TelBookRecCS.lockon();

	TelBookRecList* pSet0 = NULL;
	TelBookRecList* pSet1 = NULL;
	char filename[64];
	FILE* fd;

	if(g_pTelBookRec != NULL)
	{
		free(g_pTelBookRec);
		g_pTelBookRec = NULL;
	}
	sprintf(filename, "%s/%s", USERDIR, TelBookRecFILE0);

	fd = fopen(filename, "rb");
	if(fd !=NULL)
	{
		pSet0 = (TelBookRecList*)malloc(sizeof(TelBookRecList));
		if(fread(pSet0, 1, sizeof(TelBookRecList), fd) != sizeof(TelBookRecList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != TelBookRecENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}
	sprintf(filename, "%s/%s", USERDIR ,TelBookRecFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (TelBookRecList*)malloc(sizeof(TelBookRecList));
		if(fread(pSet1, 1, sizeof(TelBookRecList), fd)!= sizeof(TelBookRecList))
		{
			free(pSet1);
			pSet1= NULL;
		}
		else
		{
			if(pSet1->Endid != TelBookRecENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}
	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pTelBookRec = (TelBookRecList*)malloc(sizeof(TelBookRecList));
		memset(g_pTelBookRec, 0, sizeof(TelBookRecList));
		g_pTelBookRec->Endid = TelBookRecENDID;
		UpdateTelBook();
	}
	else 
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->Version > pSet1->Version)
			{
				g_pTelBookRec = pSet0;
				free(pSet1);
			}
			else
			{
				g_pTelBookRec = pSet1;
				free(pSet0);
			}
		}
		else if(pSet0 != NULL)
			g_pTelBookRec = pSet0;
		else 
			g_pTelBookRec = pSet1;
		g_pTelBookRec->Version++;
	}

	g_TelBookRecCS.lockoff();
	return TRUE;

}

void AddTelBook(char* strName, char* strCode)
{
	g_TelBookRecCS.lockon();

	TelBookRecord item;
	strcpy(item.strName, strName);
	strcpy(item.strCode, strCode);

	memmove( &g_pTelBookRec->rec[1], &g_pTelBookRec->rec[0], sizeof(TelBookRecord) * (MAX_TELBOOK_NUMBER - 1) );
	memcpy( &g_pTelBookRec->rec[0], &item, sizeof(TelBookRecord) );
	if(g_pTelBookRec->count < MAX_TELBOOK_NUMBER)
		g_pTelBookRec->count++;
	UpdateTelBook();

	g_TelBookRecCS.lockoff();
	return;
}

void DeleteTelBook(DWORD index)
{
	g_TelBookRecCS.lockon();
	if(index < g_pTelBookRec->count)
	{
		if(index != g_pTelBookRec->count - 1)
			memmove( &g_pTelBookRec->rec[index], &g_pTelBookRec->rec[index+1], sizeof(TelBookRecord) * (g_pTelBookRec->count - index - 1) );
		g_pTelBookRec->count--;
		UpdateTelBook();
	}
	g_TelBookRecCS.lockoff();
}

void ResetTelBook()
{
	g_TelBookRecCS.lockon();
	memset(g_pTelBookRec, 0, sizeof(TelBookRecList));
	g_pTelBookRec->Endid = TelBookRecENDID;
	UpdateTelBook();
	g_TelBookRecCS.lockoff();
}

DWORD GetTelBookCount()
{
	return g_pTelBookRec->count;
}

void GetTelBookData(DWORD index, TelBookRecord* pItem)
{
	g_TelBookRecCS.lockon();
	if(index >= g_pTelBookRec->count)
		memcpy(pItem, 0, sizeof(TelBookRecord));
	else
		memcpy(pItem, &g_pTelBookRec->rec[index], sizeof(TelBookRecord));
	g_TelBookRecCS.lockoff();
}

BOOL GetTelBookName(char* strCode, char* strName)
{
	for(int i = 0; i < g_pTelBookRec->count; i++)
	{
		if(strncmp(strCode, g_pTelBookRec->rec[i].strCode, 11) == 0)
		{
			strcpy(strName, g_pTelBookRec->rec[i].strName);
			return TRUE;
		}
	}

	return FALSE;
}