#pragma once

#define	BLANK_TYPE			0
#define	ROOM_TYPE			1
#define	CELL_DOOR_TYPE		2
#define	SECOND_DOOR_TYPE	3
#define	MONITOR_TYPE		4
#define	ZONE_DOOR_TYPE		5
#define	GUARD_TYPE			6
#define	AREA_DOOR_TYPE		7
#define	MANAGER_TYPE		8
#define	MAX_TERMTYPE		9

#define	SUB_FIELD			0
#define	ROOM_FIELD			1
#define	FLOOR_FIELD			2
#define	CELL_FIELD			3
#define	BUILD_FIELD			4
#define	ZONE_FIELD			5
#define	TYPE_FIELD			6
#define	MAX_FIELD			7

typedef struct
{
	UINT64 id;
	int	ip;
	int mask;
	int gw;
	char code[16];
	char name[128];
} ip_par;

typedef struct
{
	int num;
	ip_par* param;
} ip_get;

typedef struct
{
	int num;		// 支持语言个数
	int id[255];	// 语言的id
} lang_get;

__declspec(dllexport) BOOL InitNetcfgFile(int* pversion, char* xmlfile);
__declspec(dllexport) BOOL InitNetcfgBuf(int* pversion, char* pbuf, int filelen);
__declspec(dllexport) void ReleaseNetcfg();
__declspec(dllexport) BOOL Code2ID(char* code, UINT64* id);
__declspec(dllexport) BOOL ID2Code(UINT64 id, char* code);
__declspec(dllexport) BOOL ID2Name(UINT64 id, char* name, BOOL isAbsolute = FALSE);
__declspec(dllexport) DWORD Code2Type(char* code);
__declspec(dllexport) BOOL InitTerm(char* code, DWORD* ip, DWORD* mask, DWORD* gw);
// 只能获得本栋房号，code 长度可填 3-10  
__declspec(dllexport) BOOL RoomGet(ip_get* pget, char* code);	
__declspec(dllexport) BOOL CellDoorGet(ip_get* pget, char* code = NULL);
__declspec(dllexport) BOOL SecDoorGet(ip_get* pget, char* code = NULL);
__declspec(dllexport) BOOL GuardGet(ip_get* pget, char* code = NULL);
__declspec(dllexport) BOOL MonitorGet(ip_get* pget, char* code = NULL);
__declspec(dllexport) BOOL AreadoorGet(ip_get* pget, char* code = NULL);
__declspec(dllexport) BOOL ManagerGet(ip_get* pget, char* code = NULL);
__declspec(dllexport) BOOL GetLocalCode(char* code);
__declspec(dllexport) BOOL GetLocalID(UINT64* code);
__declspec(dllexport) BOOL TermGet(ip_get* pget, char* code = NULL);
__declspec(dllexport) BOOL GetAllTerm(ip_get* pget);
__declspec(dllexport) BOOL IsMainTerm(void);
__declspec(dllexport) BOOL GetMainTerm(ip_get* pget);
__declspec(dllexport) BOOL GetDefaultTerm(int type, ip_get* pget);
__declspec(dllexport) BOOL GetFieldLen(WORD* pget);
__declspec(dllexport) DWORD GetMatchMode(void);

void Code2Name(char* name, char* code);
