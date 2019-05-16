#pragma once

#define MAX_CALLLOG					30
#define MAX_ALARM_RECORD_NUMBER		30
#define MAX_DEFENSE_RECORD_NUMBER	30
#define MAX_TELBOOK_NUMBER			30
#define MAX_PHOTO_NUMBER			6
#define MAX_MESSAGE_NUMBER			6
#define MAX_LIUYING_NUMBER			6
#define MAX_LIUYAN_NUMBER			6
#define MAX_IPCAMERA_NUMBER			6

#define	SCREEN_SAVER_BLACK		0	// 屏保时黑屏
#define	SCREEN_SAVER_CLOCK		1	// 屏保时显示时钟
#define	SCREEN_SAVER_TIME		2	// 屏保时显示数字时间
#define	SCREEN_SAVER_MAX		3	

#define LANGUAGE_CH				0	// 中文
#define LANGUAGE_EN				1	// 英文

#define DELAY_SAFE		0
#define DELAY_ALARM		1
#define DELAY_DURATION	2
#define DELAY_CALLIN	3
#define DELAY_SCREEN	4
#define DELAY_UNLOCK	5
#define DELAY_MAX		6

#define DEFAULT_RING_MP3			"/FlashDev/ring/musicring.mp3"
#define DEFAULT_BK_JPG				"/Windows/bk.jpg"

// DBSet.cpp
void InitSystemSet(void);
void ResetSystemSet(void);
void SetUpdateEn(BOOL bEnable);
BOOL SetScreenOnOff(BOOL bOn);

char* GetMsgTime(void);
void SetMsgTime(char* time);
char* GetWeatherTime(void);
void SetWeatherTime(char* time);
void SetWeatherPng(DWORD index);
DWORD GetWeatherPng(void);
void SetWeatherTemp(char* temp);
void GetWeatherTemp(char* temp);
void SetWeatherHum(char* hum);
void GetWeatherHum(char* hum);
void GetTermId(char* TermId);
void SetTermId(char* TermId, int ip);
void GetProjectPwd(char* pwd);
void SetProjectPwd(char* pwd);
void GetSafePwd(char* pwd);
void SetSafePwd(char* pwd);
void GetUserPwd(char* pwd);
void SetUserPwd(char* pwd);
void GetHostagePwd(char* pwd);
void SetHostagePwd(char* pwd);
void GetPublicUserPwd(char* pwd);
void SetPublicUserPwd(char* pwd);
void GetPublicHostagePwd(char* pwd);
void SetPublicHostagePwd(char* pwd);
BOOL GetIsExistNewMail();
void SetIsExistNewMail(BOOL isExistNewMail);
void GetLastTermId(char* TermId);
char GetTermIdByIndex(int index);
void SetScreenType(DWORD type);
DWORD GetScreenType(void);
BOOL GetTrusteeshipEn(void);
void SetTrusteeshipEn(BOOL val);
DWORD GetTalkVol(void);
void SetTalkVol(DWORD vol);
DWORD GetKeyVol(void);
void SetKeyVol(DWORD vol);
DWORD GetRingVol(void);
void SetRingVol(DWORD vol);
BOOL GetMuteMode(void);
void SetMuteMode(BOOL isEn);
BOOL GetCameraEn(void);
void SetCameraEn(BOOL isEn);
char* GetBgName(void);
void SetBgName(char* ring);
char* GetRingName( BOOL bCallOut );
void SetRingName(BOOL bCallOut, char* ring);
void SetLeaveMsgType(int type);
int GetLeaveMsgType(void);
BOOL GetElevator();
void SetElevator(BOOL);
DWORD GetLanguage(void);
void SetLanguage(DWORD langid);
int GetLocalIp();
int GetLocalMask();
int GetLocalGw();
void SetLocalIP(int ip, int mask, int gw);
//DWORD GetDelay(int index);
void GetDelay(DWORD* delay);
void SetDelay(DWORD* delay);
void GetDisplayParam(DWORD* param);
void SetDisplayParam(DWORD* param);

// DBMessage
typedef struct
{
	FILETIME time;
	char title[256];
	BOOL bRead;		
}MsgRecord;

void InitMessage();
void ResetMessage();
DWORD GetMessageCount(BOOL isArea);
DWORD GetMessageUnreadCount();
BOOL AddMessage(char* ptime, char* ptitle, char* pbody, char* purl, BOOL isArea);
//BOOL AddMessage(BYTE isArea, char* title, char* pbuf, int length, char* time);
//BOOL AddMessage(char* userPwd, char* hostagePwd);
void DeleteMessage(BOOL isArea, int index);
void UpdateMessageEn(BOOL bEnable);
void DeleteMessageAll(BOOL isArea);
int GetMessageRecord(BOOL isArea, MsgRecord* pRecord);
int GetMessageData(BOOL isArea, int index, char** buf);

// DBSafeSet
typedef struct
{
	BYTE Area;			// 防区位置
	BYTE Type;			// 防区类型
	BYTE OnOff;			// 禁用启用
	BYTE Level;			// 常开常闭	0 常开 1常闭
	BYTE AlarmDelay;	// 报警延时（只有延时防区有用）
	BYTE Status;		// 0未布防 1布防 2报警
}AreaSet;

typedef	struct
{
	AreaSet		m_areaSet[SAFE_MAX_NUMBER];	// 
	DWORD		m_setDelay;					// 布防延时
	DWORD		m_alarmDelay;				// 报警延时
	DWORD		m_alarmDuration;			// 报警声持续时间
	DWORD		m_isSetDefense;				// 是否布防
	DWORD		m_currentMode;				// 当前布防模式
	DWORD		m_modeDeley[AREAMODE_MAX];	// 安防模式布防延时
	DWORD		VERSION;					//
	DWORD		Endid;						//
} SafeSet;

BOOL InitSafeSet();
void* GetSafeSetCS();
BOOL SetSafeAreaSet(AreaSet* pset);
void GetSafeAreaSet(AreaSet* pset);
void ResetSafeSet(void);
void GetSafeSet(SafeSet** pset);
BOOL UpdataSafeSet(void);
void SetDefenseStatus(BOOL isSetDefense);
BOOL GetDefenseStatus();
BOOL GetDefenseIsAlarming();
void WriteFileSafeSet();
void SetSafeMode(int mode);
int GetSafeMode();

// DBAlarm
typedef struct
{
	FILETIME time;
	BYTE area;
	BYTE type;
}AlarmRecord;

BOOL InitAlarmRecord(void);
void AddAlarmRecord(BYTE area, BYTE type);
void DeleteAlarmRecord(DWORD index);
void ResetAlarmRecord();
DWORD GetAlarmRecordCount();
void GetAlarmRecordData(DWORD index, AlarmRecord* pItem);

// DBDefense
typedef struct
{
	FILETIME time;
	BYTE newMode;
	BYTE oldMode;
}DefenseRecord;

BOOL InitDefenseRecord(void);
void AddDefenseRecord(BYTE newMode, BYTE oldMode);
void DeleteDefenseRecord(DWORD index);
void ResetDefenseRecord();
DWORD GetDefenseRecordCount();
void GetDefenseRecordData(DWORD index, DefenseRecord* pItem);
void WriteFileDefenseRec();

// DBTelephoeBook
typedef struct
{
	char strName[32];
	char strCode[16];
}TelBookRecord;

BOOL InitTelBook(void);
void AddTelBook(char* strName, char* strRoom);
void DeleteTelBook(DWORD index);
void ResetTelBook();
DWORD GetTelBookCount();
void GetTelBookData(DWORD index, TelBookRecord* pItem);
BOOL GetTelBookName(char* strCode, char* strName);

// DBPhoto
typedef struct
{
	FILETIME time;
	char name[32];
	DWORD enctype;
	BOOL bRead;
}PhotoRecord;

void InitPhoto();
void ResetPhoto();
DWORD GetPhotoCount();
DWORD GetPhotoUnreadCount();
BOOL AddPhoto(char* name, DWORD len, char* pdata, DWORD enctype);
void DeletePhoto(int index);
int GetPhotoRecord(PhotoRecord* rec);
int GetPhotoData(int index, char** buf);

// DBLiuying
typedef struct
{
	FILETIME time;
	char code[16];
	DWORD enctype;
	BOOL bRead;
}LiuyingRecord;

void InitLiuying();
void ResetLiuying();
DWORD GetLiuyingCount();
DWORD GetLiuyingUnreadCount();
BOOL AddLiuying(char* name, DWORD len, char* pdata, DWORD enctype);
void DeleteLiuying(int index);
int GetLiuyingRecord(LiuyingRecord* rec);
int GetLiuyingData(int index, char** buf, int* lwLen, char** pLwBuf);

// DBLiuyan
//typedef struct
//{
//	FILETIME time;
//	char code[16];
//	DWORD enctype;
//	BOOL bRead;
//}LiuyanRecord;
//
//void InitLiuyan();
//void ResetLiuyan();
//DWORD GetLiuyanCount();
//DWORD GetLiuyanUnreadCount();
//BOOL AddLiuyan(char* name, DWORD len, char* pdata, DWORD enctype);
//void DeleteLiuyan(int index);
//int GetLiuyanRecord(LiuyanRecord* rec);
//int GetLiuyanData(int index, char** buf);

// DBIPCamera
typedef struct
{
	char name[32];
	char ip[16];
	char user[16];
	char pwd[16];
}IPCameraRecord;

BOOL InitIPCameraDB(void);
void ResetIPCameraDB();
void AddIPCamera(char* name, char* ip, char* user, char* pwd);
void DeleteIPCamera(DWORD index);
DWORD GetIPCameraCount();
void GetIPCameraData(DWORD index, IPCameraRecord* pItem);

// DBCall

typedef struct
{
	FILETIME time;
	char code[16];
	BOOL bRead;
}CallRecord;

void InitCallRecord();
void ResetCallRecord();
void AddCallRecord(char* code, BOOL bCallOut, BOOL bAccept);
void DeleteCallRecord(int type, int index);
void DeleteCallRecordAll(int type);
int GetCallRecord(int type, CallRecord* pRecord);
void GetCallRecordCode(char* code, int type, int index);
int GetCallRecordUnread();
void SetCallRecordRead(int index);
