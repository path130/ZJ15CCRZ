#include "roomlib.h"
#include "ipfunc.h"

static int netcfg_version;
static char netcfg_md5[33] = {0};
DWORD IpMatchMode[7];

BOOL InitNetCfg(void)
{
	char fileName[3][128];
	sprintf(fileName[0], "%s/%s", USERDIR, IPTABLE_NAME);
	sprintf(fileName[1], "%s/%s", USERDIR, IPTABLE_BAK);
	sprintf(fileName[2], "%s/%s", FLASHDIR, IPTABLE_NAME);
	
	int i;
	for(i = 0; i < 3; i++)
	{
		if(InitNetcfgFile(&netcfg_version, fileName[i]))
		{
			memset(IpMatchMode, 0, sizeof(IpMatchMode));
			char strMatchMode[7];
			DWORD MatchMode = GetMatchMode();
			sprintf(strMatchMode, "%06x", MatchMode);
			IpMatchMode[6] = 0;
			for(int j = 0; j < 6; j++)
			{
				IpMatchMode[j] = strMatchMode[j] - '0';
				IpMatchMode[6] +=IpMatchMode[j];
			} 

			if(!CalFileMd5(netcfg_md5, fileName[i]))
				DBGMSG(DPERROR, "Get NetCfg %s MD5 Fail!\r\n", fileName);
			break;
		}
	}

	return (i < 3);
}

char* GetNetCfgMD5(void)
{
	return netcfg_md5;
}

int GetNetCfgVersion()
{
	return netcfg_version;
}