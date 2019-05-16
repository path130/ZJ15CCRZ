#include "roomlib.h"

#define IPCAMERAFILE0		"IPCamera0.ext"
#define IPCAMERAFILE1		"IPCamera1.ext"
#define IPCAMERAENDID        0x54534577

typedef struct
{
	IPCameraRecord rec[MAX_IPCAMERA_NUMBER];
	DWORD count;         		 
	DWORD Version;
	DWORD Endid;
}IPCameraList;

static IPCameraList* g_pIPCamera = NULL;
static StaticLock g_alarmRecCS;

static BOOL UpdateIPCameraRecord(void)
{
	IPCameraList* pSet;
	BOOL ret;

	pSet = (IPCameraList*)malloc(sizeof(IPCameraList));
	memcpy(pSet, g_pIPCamera, sizeof(IPCameraList));
	if(g_pIPCamera->Version & 1)
		ret = WriteServerFile(IPCAMERAFILE0, sizeof(IPCameraList), pSet);
	else 
		ret = WriteServerFile(IPCAMERAFILE1, sizeof(IPCameraList), pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(g_pIPCamera->Version & 1)
			DeleteServerFile(IPCAMERAFILE1);
		else
			DeleteServerFile(IPCAMERAFILE0);
		g_pIPCamera->Version++;
	}
	return ret;
}

BOOL InitIPCameraDB(void)
{
	g_alarmRecCS.lockon();

	IPCameraList* pSet0 = NULL;
	IPCameraList* pSet1 = NULL;
	char filename[64];
	FILE* fd;

	sprintf(filename, "%s/%s", USERDIR, IPCAMERAFILE0);

	fd = fopen(filename, "rb");
	if(fd !=NULL)
	{
		pSet0 = (IPCameraList*)malloc(sizeof(IPCameraList));
		if(fread(pSet0, 1, sizeof(IPCameraList), fd) != sizeof(IPCameraList))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != IPCAMERAENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}
	sprintf(filename, "%s/%s", USERDIR ,IPCAMERAFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (IPCameraList*)malloc(sizeof(IPCameraList));
		if(fread(pSet1, 1, sizeof(IPCameraList), fd)!= sizeof(IPCameraList))
		{
			free(pSet1);
			pSet1= NULL;
		}
		else
		{
			if(pSet1->Endid != IPCAMERAENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}
	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pIPCamera = (IPCameraList*)malloc(sizeof(IPCameraList));
		memset(g_pIPCamera, 0, sizeof(IPCameraList));
		g_pIPCamera->Endid = IPCAMERAENDID;
		UpdateIPCameraRecord();
	}
	else 
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->Version > pSet1->Version)
			{
				g_pIPCamera = pSet0;
				free(pSet1);
			}
			else
			{
				g_pIPCamera = pSet1;
				free(pSet0);
			}
		}
		else if(pSet0 != NULL)
			g_pIPCamera = pSet0;
		else 
			g_pIPCamera = pSet1;
		g_pIPCamera->Version++;
	}

	g_alarmRecCS.lockoff();
	return TRUE;
}

void AddIPCamera(char* name, char* ip, char* user, char* pwd)
{
	g_alarmRecCS.lockon();

	IPCameraRecord item;
	strncpy(item.name, name, sizeof(item.name));
	strncpy(item.ip, ip, sizeof(item.ip));
	strncpy(item.user, user, sizeof(item.user));
	strncpy(item.pwd, pwd, sizeof(item.pwd));

	memmove( &g_pIPCamera->rec[1], &g_pIPCamera->rec[0], sizeof(IPCameraRecord) * (MAX_IPCAMERA_NUMBER - 1) );
	memcpy( &g_pIPCamera->rec[0], &item, sizeof(IPCameraRecord) );
	if(g_pIPCamera->count < MAX_IPCAMERA_NUMBER)
		g_pIPCamera->count++;
	UpdateIPCameraRecord();

	g_alarmRecCS.lockoff();
	return;
}

void DeleteIPCamera(DWORD index)
{
	g_alarmRecCS.lockon();
	if(index < g_pIPCamera->count)
	{
		if(index != g_pIPCamera->count - 1)
			memmove( &g_pIPCamera->rec[index], &g_pIPCamera->rec[index+1], sizeof(IPCameraRecord) * (g_pIPCamera->count - index - 1) );
		g_pIPCamera->count--;
		UpdateIPCameraRecord();
	}
	g_alarmRecCS.lockoff();
}

void ResetIPCameraDB()
{
	g_alarmRecCS.lockon();
	memset(g_pIPCamera, 0, sizeof(IPCameraList));
	g_pIPCamera->Endid = IPCAMERAENDID;
	UpdateIPCameraRecord();
	g_alarmRecCS.lockoff();
}

DWORD GetIPCameraCount()
{
	return g_pIPCamera->count;
}

void GetIPCameraData(DWORD index, IPCameraRecord* pItem)
{
	g_alarmRecCS.lockon();
	if(index >= g_pIPCamera->count)
		memcpy(pItem, 0, sizeof(IPCameraRecord));
	else
		memcpy(pItem, &g_pIPCamera->rec[index], sizeof(IPCameraRecord));
	g_alarmRecCS.lockoff();
}
