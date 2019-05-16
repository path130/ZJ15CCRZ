#pragma once

// ServiceFile
BOOL InitFileServer(void);
void DeinitFileServer(void);
BOOL WriteServerFile(const char* name, int len, void* buf);
void DeleteServerFile(const char* name);

// HttpSocket
BOOL HttpDownloadFile(char *strFileName, char *strHttpUrl);

// ServiceSyncXmlSend
#define	OPEN_LOCK					1
#define	ALARM_REPORT				2
#define	DISALARM_REPORT				3
#define SET_DEFENSE_REPORT			4
#define CANCEL_DEFENSE_REPORT		5
#define OPEN_LOCKTWO				6

BOOL StartSyncSendServer(void);
void StopSyncSendServer(void);
BOOL SendXmlSyncMsg(DWORD msg, DWORD wParam, DWORD lParam, UINT64 roomid);

void GetHardInfo(char* info);
void GetCompanyInfo(int language, char* info);

// ServiceSyncRecv
void StartSyncRecvServer(void);
void StopSyncRecvServer(void);

// ServicePC
void StartPCServer(void);
void StopPCServer(void);
BOOL DownLoadFile(SOCKET sockfd, char* filename, int nFileSize);

// ServiceSafe
#define	SMSG_SETTING_CHANGE		1
#define	SMSG_DISALARM			2
#define	SMSG_HOSTAGE			3
#define	SMSG_EXIST				4
#define SMSG_ALARM_RING			5
#define SMSG_SOS				6
#define SMSG_DELAY				7
#define SMSG_SET_SAFE			8	// ²¼·À

void StarSafeServer(void);
void StopSafeServer(void);
BOOL PostSafeMessage(DWORD msg, DWORD wParam, DWORD lParam, DWORD zParam);
BOOL CheckSafeGpio();

// ServiceSyncSend
BOOL SetSecDoorCode(char* doorcode, UINT64 newid);
BOOL SetSecDoorDelay(char* doorcode, DWORD level, DWORD delay);
BOOL ReqDoorAddCard(DWORD type);
BOOL ReqDoorDelCard(DWORD type);
BOOL ChangeUserPwd(char* pwd);
BOOL ChangeHostagePwd(char* pwd);
BOOL CallElevator(void);
BOOL SyncCellDoor(void);

