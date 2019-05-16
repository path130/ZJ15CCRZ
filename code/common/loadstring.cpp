#include "roomlib.h"
#include "list.h"

typedef struct tagQSOBJ {
	LISTOBJ lpObj;
	int	 id;
	char	string[64];
} NAMEOBJ,  * LPNAMEOBJ;

static CMyList m_namelist;

static void DestroyStringList()
{
	LPNAMEOBJ pItemNext;
	LPNAMEOBJ pItem = (LPNAMEOBJ) m_namelist.Head();
	while (pItem)
	{
		pItemNext = (LPNAMEOBJ) m_namelist.Next(&(pItem->lpObj));
		m_namelist.Disconnect(&(pItem->lpObj));
		delete pItem;
		pItem = pItemNext;
	}
}

BOOL LoadAllString(char * pPath,int type)
{
	char wszLine[256];
	FILE *  fin;
	fin = fopen(pPath, "rb");

	if(fin == NULL)
		return FALSE;

	if(m_namelist.GetCount() > 0)
		DestroyStringList();

	fgets(wszLine, 4, fin);
	while (fgets(wszLine, 256, fin))
	{
		char* pstr = strchr(wszLine, ':');
		if(pstr == NULL)
			continue;

		int id = atoi(wszLine);
		if(id == 0)
			continue;

		LPNAMEOBJ pobj = new NAMEOBJ;
		if(pobj == NULL)
			break;

		*pstr = 0;
		pobj->id = id;
		strcpy(pobj->string,pstr+1);

		pstr = strchr(pobj->string, '\n');
		if(pstr)
		{
			if(pstr[-1] == '\r')
				pstr[-1] = '\0';
			pstr[0] = '\0';
		}

		m_namelist.AddHead(&pobj->lpObj);
	}
	fclose(fin);
	printf("LoadString add %d string\r\n", m_namelist.GetCount());
	return TRUE;
}

char * GetStringByID(int id)
{
	LPNAMEOBJ pItem = (LPNAMEOBJ) m_namelist.Head();
	while (pItem)
	{
		if(pItem->id == id)
			return &pItem->string[0];
		pItem = (LPNAMEOBJ) m_namelist.Next(&(pItem->lpObj));
	}

	printf("GetStringByID don't find %d\r\n", id);
	return NULL;
}