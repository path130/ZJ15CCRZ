#include <roomlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/sysinfo.h>

void DPDeleteFile(const char* pfile)
{
	if(NULL != pfile)
		remove(pfile);
}

void DPMoveFile(const char* dstfile, const char* srcfile)
{
	if(NULL != dstfile && NULL != srcfile)
	{
		char pCommand[512] = {0};
		sprintf(pCommand, "mv %s %s", srcfile, dstfile);
		system(pCommand);
	}
}

void DPCopyFile(const char* dstfile, const char* srcfile)
{
	char pCommand[512] = {0};
	sprintf(pCommand, "cp -r %s %s", srcfile, dstfile);
	system(pCommand);
}

HANDLE DPFindFirstFile(char* dir, char* pfile)
{
	DIR * pDir;
	struct dirent *entry;

	DBGMSG(PORT_MOD, "open %s\r\n", dir);
	if((pDir = opendir(dir)) == NULL)
	{
		DBGMSG(DPERROR, "opendir %s error", dir);
		return INVALID_HANDLE_VALUE;
	}

	while((entry = readdir(pDir)) != NULL)
	{
		strcpy(pfile, entry->d_name);
		break;
	}

	if(entry == NULL)
	{
		closedir(pDir);
		return INVALID_HANDLE_VALUE;
	}
    return pDir;
}

BOOL DPFindNextFile(HANDLE hFind, char* pfile)
{
	DIR* pDir = (DIR*)hFind;
    struct dirent *entry;

	while((entry = readdir(pDir)) != NULL)
	{
		strcpy(pfile, entry->d_name);
		break;
	}
	if(entry == NULL)
		return FALSE;
	return TRUE;
}

void DPFindClose(HANDLE hFind)
{
	closedir((DIR *)hFind);
}

int DPGetFileAttributes(const char* filename)
{
	struct stat attributes;
	return stat(filename, &attributes);
}

BOOL CheckAndCreateDir(const char *cDir)
{
	DIR* pDir;

    if((pDir = opendir(cDir)) == NULL)
    {
    	mkdir(cDir, 777);
    }
	else
		closedir(pDir);
	return TRUE;
}

UINT64 CheckSpareSpace(const char* dir)
{
#ifndef _DEBUG
	char dirname[32];
	struct statfs diskInfo;

	if(strstr(dir, "Windows") != NULL)
		strcpy(dirname, "/Windows/");
	else if(strstr(dir, "FlashDev")!=NULL)
		strcpy(dirname, "/FlashDev/");
	else if(strstr(dir, "UserDev")!=NULL)
		strcpy(dirname, "/UserDev/");

	statfs(dirname, &diskInfo);
	unsigned long long totalsize = diskInfo.f_bsize * diskInfo.f_blocks;	//总的字节数，f_blocks为block的数目
	unsigned long long freeDisk = diskInfo.f_bfree * diskInfo.f_bsize; //剩余空间的大小
	printf("total = %llu KB Free =  %llu KB \n", totalsize>>10, freeDisk>>10);
		
	return (UINT64)freeDisk;
#endif
	return 0x1000000;
}

void StopLogo(void)
{
}

void ScanDisk(void)
{
}

DWORD DumpMemory(DWORD* total, DWORD* left)
{
    struct sysinfo tSysInfo;
    if(sysinfo(&tSysInfo) == 0)
    {
		*total = tSysInfo.totalram;
		*left = tSysInfo.freeram + tSysInfo.bufferram;
    }
    else
    {
        printf("%s(%d): sysinfo error:%d\n", __FILE__, __LINE__, errno);
		*total = 1;
		*left = 1;
    }
	return 0;
}

void DumpPhysicalMemory()
{
}

DWORD SetObjectMemorySpace_GWF(int nSize,DWORD dwNewPage)
{
	return nSize;
}

void DPUnloadFile(const char* filename)
{
	printf("fuser -k /sdcard\r\n");
	system("fuser -k /sdcard");
	usleep(500);
	printf("umount /sdcard\r\n");
	system("umount /sdcard");
	usleep(500);
	printf("rm -rf /sdcard\r\n");
	system("rm -rf /sdcard");
	usleep(500);
	printf("sync\r\n");
	sync();
	usleep(1000 * 50);
	printf("DPUnloadFile over\r\n");
	return;

	char pCommand[128];
	sprintf(pCommand, "fuser -k %s", filename);
	printf("cmd %s\r\n", pCommand);
	system(pCommand);
	DPSleep(1000);

	sprintf(pCommand, "umount -l %s", filename);
	printf("cmd %s\r\n", pCommand);
	system(pCommand);
	DPSleep(1000);

	printf("cmd sync\r\n");
	sync();
	DPSleep(1000);
	printf("DPUnloadFile %s\r\n", filename);
}
