#pragma once

#include "dpplatform.h"
#include "dpdebug.h"
#include "dptcpip.h"
#include "dpaudio.h"
#include "dpsound.h"
#include "config.h"
#include "systemmsg.h"
#include "loadstring.h"
#include "database.h"
#include "server.h"

//class StaticLock
//{
//public:
//	StaticLock();
//	~StaticLock();
//	void lockon();
//	void lockoff();
//private:
//	CRITICAL_SECTION m_cs;
//};

DWORD hexconvert(char* data);
void unicode2utf8(BYTE* dst, wchar_t* unicode);
int utf82unicode(WORD* dst, BYTE* utf8);
void unicode2wchar(wchar_t* dst, WORD* unicode);
int utf8len(char* str);
FILETIME timeToFileTime(const time_t *ptime);
BOOL dp_inet_addr(char* string, int *ip);

int BReadFile(char* filename, char** buf);
char* CFindContent(char* start, char* match, char* ret, BOOL remove);
void FindFileFromDirectory(char* dir, void (*func)(char* directory, char* fileName, void* param), void* param);

BOOL CalFileMd5(char* output, char* filename);
BOOL CalStrMd5(char* output, unsigned char* input, int len);

BOOL InitNetCfg(void);
char* GetNetCfgMD5(void);
int GetNetCfgVersion();
