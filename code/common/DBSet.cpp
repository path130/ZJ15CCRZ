#include "roomlib.h"
#include "ipfunc.h"

#define	SETFILE0		"SystemSet0.ext"
#define	SETFILE1		"SystemSet1.ext"
#define	SETENDID		0x55595301		// SYSS

typedef struct
{
	char	TermId[16];				// 0	终端号码
	char	msgtime[32];			// 16	最后一次接收消息的时间
	char	weathertime[32];        // 48	最后一次接收天气时间
	int     nImage;                 // 80	天气图片
	char	temp[8];                // 84	温度
	char	hum[8];                 // 92	湿度
	char	ProjetPwd[8];			// 100	工程密码
	char	SafePwd[8];				// 108	安防密码
	char	HostagePwd[8];			// 116	挟持码
	char	UserPwd[8];				// 124	用户密码,即开锁密码
	char	PublicUserPwd[8];		// 132	公共开锁密码
	char	PublicHostagePwd[8];	// 140	公共挟持开锁密码
	//DWORD	Screensaver;			// 148	屏保时间
	DWORD	ScreenType;				// 152	屏保方式： 0：黑屏屏保 1：时间屏保 2：图片屏保
	DWORD   TalkVol;				// 156	通话音量
	DWORD	KeyVol;					// 160	按键音量
	DWORD	RingVol;				// 164	铃声音量
	BOOL	CameraEn;				// 168	对讲中是否使用摄像头 缺省FALSE
	BOOL	bMute;					// 172	TRUE 静音 
	char	CallInRingName[128];	// 176	铃声文件,铃声文件为MP3文件
	char	CallOutRingName[128];	// 304	铃声文件,铃声文件为MP3文件
	char	BkName[128];			// 432	背景图片,背景图片为JPG文件，在Image目录下
	BOOL	Elevator_Support;		// 560	是否支持梯控，缺省不支持
	DWORD	Language;				// 564	语言设置 0:中文 1:英文
	int     LvMsgType;				// 568	留言方式  0:默认留言，1:业主留言 2：不留言		
	//int	CallInTimeOut;			// 572	被呼延时时间，时间到，挂断
	BOOL	isExistNewMail;			// 576	是否有新的信件未接收
	char	LastTermId[16];			// 580	设置之前的终端号码
	//DWORD	UnlockDelay;			// 596	开锁延时 单位 s
	BOOL	bTrusteeship;			// 600	是否启用托管模式
	int		ip;						// 604
	int		mask;					// 608
	int		gateway;				// 612
	DWORD	dwDelay[DELAY_MAX];		// 616
	DWORD	displayParam[7];		//		显示参数 0 亮度 1 对比度 2 饱和度 3视频亮度 4视频对比度 5视频饱和度 6视频色彩度
	BYTE	reserved[2040-640];		// 640	
	DWORD	VERSION;				// 2040
	DWORD	Endid;					// 2044
} SystemSet_t;						// 2048

static SystemSet_t* m_gSystemSet = NULL;
static StaticLock g_SystemSetCS;
static BOOL g_bUpdateEn = TRUE;

static void UpdataSystemSet(void)   //IP冲突检测
{
	if(!g_bUpdateEn)
		return;

	SystemSet_t* pSet;
	BOOL ret;

	pSet = (SystemSet_t*)malloc(sizeof(SystemSet_t));
	memcpy(pSet, m_gSystemSet, sizeof(SystemSet_t));
	if(m_gSystemSet->VERSION & 1)
		ret = WriteServerFile(SETFILE1, sizeof(SystemSet_t), (char*)pSet);
	else
		ret = WriteServerFile(SETFILE0, sizeof(SystemSet_t), (char*)pSet);
	if(!ret)
		free(pSet);
	else
	{
		if(m_gSystemSet->VERSION & 1)
			DeleteServerFile(SETFILE0);
		else
			DeleteServerFile(SETFILE1);
		m_gSystemSet->VERSION++;
	}
}

static void InitDefaultSystemSet(void)
{
	memset(m_gSystemSet, 0, sizeof(SystemSet_t));

	strcpy(m_gSystemSet->ProjetPwd, "666666" );
	strcpy(m_gSystemSet->SafePwd, "123456" );
	strcpy(m_gSystemSet->HostagePwd, "654321" );
	strcpy(m_gSystemSet->UserPwd, "123456" );
	strcpy(m_gSystemSet->PublicUserPwd, "666666" );	//无用的
	strcpy(m_gSystemSet->PublicHostagePwd, "666666" );	//无用的
	m_gSystemSet->TalkVol = 6;
	m_gSystemSet->KeyVol = 6;
	m_gSystemSet->RingVol = 6;
	strcpy(m_gSystemSet->CallInRingName, DEFAULT_RING_MP3);
	strcpy(m_gSystemSet->CallOutRingName, DEFAULT_RING_MP3);
	strcpy(m_gSystemSet->BkName, "/FlashDev/wallpaper/bk2.jpg");
	m_gSystemSet->Language = 0;
	m_gSystemSet->LvMsgType = 0; //默认为默认留言
	m_gSystemSet->ScreenType = SCREEN_SAVER_BLACK;
	m_gSystemSet->isExistNewMail = FALSE;
	m_gSystemSet->dwDelay[DELAY_SAFE] = 60;
	m_gSystemSet->dwDelay[DELAY_ALARM] = 60;
	m_gSystemSet->dwDelay[DELAY_DURATION] = 5;
	m_gSystemSet->dwDelay[DELAY_CALLIN] = 30;
	m_gSystemSet->dwDelay[DELAY_SCREEN] = 60;
	m_gSystemSet->dwDelay[DELAY_UNLOCK] = 1;
	m_gSystemSet->displayParam[0] = 60;
	m_gSystemSet->displayParam[1] = 50;
	m_gSystemSet->displayParam[2] = 50;
	m_gSystemSet->displayParam[3] = 50;
	m_gSystemSet->displayParam[4] = 50;
	m_gSystemSet->displayParam[5] = 50;
	m_gSystemSet->displayParam[6] = 50;
	m_gSystemSet->Endid = SETENDID;
	m_gSystemSet->bTrusteeship = FALSE;
	SYSTEMTIME systime;
	DPGetLocalTime(&systime);
	sprintf(m_gSystemSet->msgtime, "%04d-%02d-%02d %02d:%02d:%02d", 
		systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	sprintf(m_gSystemSet->weathertime, "%04d-%02d-%02d %02d:%02d:%02d",
		systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

#ifdef _DEBUG
	strcpy(m_gSystemSet->msgtime, "2016-11-27 14:05");
#endif
	m_gSystemSet->nImage = 0;
	strcpy(m_gSystemSet->temp, "5-10℃");
	strcpy(m_gSystemSet->hum, "85");
	strcpy(m_gSystemSet->TermId, "1999999999901");
}

void InitSystemSet(void)
{
	SystemSet_t* pSet0 = NULL;
	SystemSet_t* pSet1 = NULL;
	char filename[64];
	FILE* fd;

	g_SystemSetCS.lockon();
	sprintf(filename, "%s/%s", USERDIR, SETFILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (SystemSet_t*)malloc(sizeof(SystemSet_t));
		memset(pSet0, 0, sizeof(SystemSet_t));
		if(fread(pSet0, 1, sizeof(SystemSet_t), fd) != sizeof(SystemSet_t))
		{
			free(pSet0);
			pSet0 = NULL;
		}
		else
		{
			if(pSet0->Endid != SETENDID)
			{
				free(pSet0);
				pSet0 = NULL;
			}
		}
		fclose(fd);
	}

	sprintf(filename, "%s/%s", USERDIR, SETFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (SystemSet_t*)malloc(sizeof(SystemSet_t));
		memset(pSet1, 0, sizeof(SystemSet_t));
		if(fread(pSet1, 1, sizeof(SystemSet_t), fd) != sizeof(SystemSet_t))
		{
			free(pSet1);
			pSet1 = NULL;
		}
		else
		{
			if(pSet1->Endid != SETENDID)
			{
				free(pSet1);
				pSet1 = NULL;
			}
		}
		fclose(fd);
	}

	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		m_gSystemSet = (SystemSet_t*)malloc(sizeof(SystemSet_t));
		InitDefaultSystemSet();
		UpdataSystemSet();
	}
	else
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->VERSION > pSet1->VERSION)
			{
				m_gSystemSet = pSet0;
				free(pSet1);
			}
			else
			{
				m_gSystemSet = pSet1;
				free(pSet0);
			}
		}
		else if(pSet0 != NULL)
			m_gSystemSet = pSet0;
		else
			m_gSystemSet = pSet1;
		m_gSystemSet->VERSION++;
	}

	if(-1 == DPGetFileAttributes(m_gSystemSet->BkName))
	{
		strcpy(m_gSystemSet->BkName, "/FlashDev/wallpaper/bk2.jpg");
		UpdataSystemSet();
	}

	DPDeleteFile(DEFAULT_BK_JPG);
	DPCopyFile(DEFAULT_BK_JPG, m_gSystemSet->BkName);

	g_SystemSetCS.lockoff();
}
void ResetSystemSet(void)
{
	g_SystemSetCS.lockon(); 
	InitDefaultSystemSet();
	UpdataSystemSet();
	g_SystemSetCS.lockoff(); 
}

void GetTermId(char* TermId)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
		strcpy(TermId, m_gSystemSet->TermId);
	g_SystemSetCS.lockoff();
}

char* GetMsgTime(void)
{
	char* time;
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
		time = m_gSystemSet->msgtime;
	g_SystemSetCS.lockoff();
	return time;
}
void SetMsgTime(char* time)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(time, m_gSystemSet->msgtime) > 0)
		{
			strncpy(m_gSystemSet->msgtime, time, sizeof(m_gSystemSet->msgtime));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}
char* GetWeatherTime(void)
{
	char* time;
	g_SystemSetCS.lockon();
	time = m_gSystemSet->weathertime;
	g_SystemSetCS.lockoff();
	return time;
}
void SetWeatherTime(char* time)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(time, m_gSystemSet->weathertime) > 0)
		{
			strncpy(m_gSystemSet->weathertime, time, sizeof(m_gSystemSet->weathertime));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}
void SetWeatherTemp(char* temp)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL && temp != NULL)
	{
		strncpy(m_gSystemSet->temp, temp, sizeof(m_gSystemSet->temp));
		UpdataSystemSet();
	}
	g_SystemSetCS.lockoff();
}

void GetWeatherTemp(char* temp)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet && temp)
		strcpy(temp, m_gSystemSet->temp);
	g_SystemSetCS.lockoff();
}

void SetWeatherHum(char* hum)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet && hum)
	{
		strncpy(m_gSystemSet->hum, hum, sizeof(m_gSystemSet->hum));
		UpdataSystemSet();
	}
	g_SystemSetCS.lockoff();
}
void GetWeatherHum(char* hum)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet && hum)
	{
		strcpy(hum, m_gSystemSet->hum);
	}
	g_SystemSetCS.lockoff();
}
void SetWeatherPng(DWORD index)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->nImage != index)
		{
			m_gSystemSet->nImage = index;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

DWORD GetWeatherPng(void)
{
	int index;
	g_SystemSetCS.lockon();
	index = m_gSystemSet->nImage;
	g_SystemSetCS.lockoff();
	return index;
}

void SetTermId(char* TermId, int ip)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(TermId, m_gSystemSet->TermId) != 0)
		{
			m_gSystemSet->ip = ip;
			strncpy(m_gSystemSet->LastTermId, m_gSystemSet->TermId, sizeof(m_gSystemSet->LastTermId));
			strncpy(m_gSystemSet->TermId, TermId, sizeof(m_gSystemSet->TermId));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

void GetProjectPwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		strcpy(pwd, m_gSystemSet->ProjetPwd);
	}
	g_SystemSetCS.lockoff();
}

void SetProjectPwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(pwd, m_gSystemSet->ProjetPwd) != 0)
		{
			strncpy(m_gSystemSet->ProjetPwd, pwd, sizeof(m_gSystemSet->ProjetPwd));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

void GetSafePwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		strcpy(pwd, m_gSystemSet->SafePwd);
	}
	g_SystemSetCS.lockoff();
}

void SetSafePwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(pwd, m_gSystemSet->SafePwd) != 0)
		{
			strncpy(m_gSystemSet->SafePwd, pwd, sizeof(m_gSystemSet->SafePwd));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

void GetUserPwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		strcpy(pwd, m_gSystemSet->UserPwd);
	}
	g_SystemSetCS.lockoff();
}

void SetUserPwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(pwd, m_gSystemSet->UserPwd) != 0)
		{
			strncpy(m_gSystemSet->UserPwd, pwd, sizeof(m_gSystemSet->UserPwd));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}
void GetHostagePwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		strcpy(pwd, m_gSystemSet->HostagePwd);
	}
	g_SystemSetCS.lockoff();
}

void SetHostagePwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(pwd, m_gSystemSet->HostagePwd) != 0)
		{
			strncpy(m_gSystemSet->HostagePwd, pwd, sizeof(m_gSystemSet->HostagePwd));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

DWORD GetScreenType(void)
{
	int ret = 0;
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->ScreenType;
	}
	g_SystemSetCS.lockoff();
	return ret;
}

void SetScreenType(DWORD type)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->ScreenType != type)
		{
			m_gSystemSet->ScreenType = type;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

BOOL GetTrusteeshipEn(void)
{
	BOOL ret = 0;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->bTrusteeship;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetTrusteeshipEn(BOOL val)
{
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->bTrusteeship != val)
		{
			m_gSystemSet->bTrusteeship = val;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}


DWORD GetTalkVol(void)
{
	DWORD ret = 0;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->TalkVol;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetTalkVol(DWORD vol)
{
	if(vol > 15)
		return;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->TalkVol != vol)
		{
			m_gSystemSet->TalkVol = vol;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}

DWORD GetKeyVol(void)
{
	DWORD ret = 0;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->KeyVol;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetKeyVol(DWORD vol)
{
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->KeyVol != vol)
		{
			m_gSystemSet->KeyVol = vol;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}

DWORD GetRingVol(void)
{
	DWORD ret = 0;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->RingVol;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetRingVol(DWORD vol)
{
	g_SystemSetCS.lockon(); 

	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->RingVol != vol)
		{
			m_gSystemSet->RingVol = vol;
			UpdataSystemSet();
		}
		DPPostMessage(MSG_BROADCAST, VOLUEME_CHANGE, 0, 0);
	}
	g_SystemSetCS.lockoff(); 
}

BOOL GetMuteMode(void)
{
	BOOL ret = FALSE;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->bMute;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetMuteMode(BOOL isEn)
{
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->bMute != isEn)
		{
			m_gSystemSet->bMute = isEn;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}

BOOL GetCameraEn(void)
{
	BOOL ret = FALSE;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->CameraEn;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetCameraEn(BOOL isEn)
{
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->CameraEn != isEn)
		{
			m_gSystemSet->CameraEn = isEn;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}

char* GetBgName(void)
{
	return m_gSystemSet->BkName;
}

void SetBgName(char* fname)
{
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(strcmp(m_gSystemSet->BkName, fname) != 0)
		{
			strncpy(m_gSystemSet->BkName, fname, sizeof(m_gSystemSet->BkName));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}

//char* GetRingName(void)
//{
//	return m_gSystemSet->RingName;
//}
//
//void SetRingName(char* ring)
//{
//	g_SystemSetCS.lockon(); 
//	if(m_gSystemSet != NULL)
//	{
//		if(strcmp(m_gSystemSet->RingName, ring) != 0)
//		{
//			strcpy(m_gSystemSet->RingName, ring);
//			UpdataSystemSet();
//		}
//	}
//	g_SystemSetCS.lockoff(); 
//}

char* GetRingName( BOOL bCallOut )
{
	if(bCallOut)
	{
		if(strcmp(m_gSystemSet->CallOutRingName, DEFAULT_RING_MP3) != 0)
		{
			DWORD attributes = DPGetFileAttributes(m_gSystemSet->CallOutRingName);
			if(attributes == -1)
				strcpy(m_gSystemSet->CallOutRingName, DEFAULT_RING_MP3);
		}

		return m_gSystemSet->CallOutRingName;
	}
	else
	{
		if(strcmp(m_gSystemSet->CallInRingName, DEFAULT_RING_MP3) != 0)
		{
			DWORD attributes = DPGetFileAttributes(m_gSystemSet->CallInRingName);
			if(attributes == -1)
				strcpy(m_gSystemSet->CallInRingName, DEFAULT_RING_MP3);
		}

		return m_gSystemSet->CallInRingName;
	}
}

void SetRingName(BOOL bCallOut, char* ringName)
{
	g_SystemSetCS.lockon(); 
	if( m_gSystemSet != NULL )
	{
		char* wcRingName = (bCallOut) ? m_gSystemSet->CallOutRingName : m_gSystemSet->CallInRingName;
		if( strcmp(wcRingName, ringName) == 0 )
			return;
		strncpy(wcRingName, ringName, sizeof(m_gSystemSet->CallInRingName));
		UpdataSystemSet();
	}
	g_SystemSetCS.lockoff(); 
}

BOOL GetElevator(void)
{
	BOOL ret = FALSE;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->Elevator_Support;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetElevator(BOOL isEn)
{
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->Elevator_Support != isEn)
		{
			m_gSystemSet->Elevator_Support = isEn;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}


DWORD GetLanguage(void)
{
	DWORD ret = 0;
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->Language;
	}
	g_SystemSetCS.lockoff(); 
	return ret;
}

void SetLanguage(DWORD langid)
{
	g_SystemSetCS.lockon(); 
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->Language != langid)
		{
			m_gSystemSet->Language = langid;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff(); 
}

int GetLeaveMsgType(void)
{
	int ret = 0;
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->LvMsgType;
	}
	g_SystemSetCS.lockoff();
	return ret;
}

void SetLeaveMsgType(int type)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->LvMsgType != type)
		{
			m_gSystemSet->LvMsgType = type;
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

void GetPublicUserPwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		strcpy(pwd, m_gSystemSet->PublicUserPwd);
	}
	g_SystemSetCS.lockoff();
}

void SetPublicUserPwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(pwd, m_gSystemSet->PublicUserPwd) != 0)
		{
			strncpy(m_gSystemSet->PublicUserPwd, pwd, sizeof(m_gSystemSet->PublicUserPwd));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}


void GetPublicHostagePwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		strcpy(pwd, m_gSystemSet->PublicHostagePwd);
	}
	g_SystemSetCS.lockoff();
}

void SetPublicHostagePwd(char* pwd)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(strcmp(pwd, m_gSystemSet->PublicHostagePwd) != 0)
		{
			strncpy(m_gSystemSet->PublicHostagePwd, pwd, sizeof(m_gSystemSet->PublicHostagePwd));
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

BOOL GetIsExistNewMail()
{
	BOOL ret = FALSE;
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		ret = m_gSystemSet->isExistNewMail;
	}
	g_SystemSetCS.lockoff();
	return ret;
}

void SetIsExistNewMail(BOOL isExistNewMail)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		if(m_gSystemSet->isExistNewMail != isExistNewMail)
		{
			m_gSystemSet->isExistNewMail = isExistNewMail;	
			UpdataSystemSet();
		}
	}
	g_SystemSetCS.lockoff();
}

void GetLastTermId(char* TermId)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
		strcpy(TermId, m_gSystemSet->LastTermId);
	g_SystemSetCS.lockoff();
}

char GetTermIdByIndex(int index)
{
	char code = '1';
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL && index < 13)
		code = m_gSystemSet->TermId[index];
	g_SystemSetCS.lockoff();
	return code;
}

void SetLocalIP(int ip, int mask, int gw)
{
	g_SystemSetCS.lockon();
	m_gSystemSet->ip = ip;
	m_gSystemSet->mask = mask;
	m_gSystemSet->gateway = gw;
	g_SystemSetCS.lockoff();
}

int GetLocalIp()
{
#ifdef _DEBUG
#ifdef DPLINUX
	return inet_addr("192.168.251.157");
#endif
#endif
	return m_gSystemSet->ip;
}

int GetLocalMask()
{
	return m_gSystemSet->mask;
}

int GetLocalGw()
{
	return m_gSystemSet->gateway;
}

DWORD GetDelay(int index)
{
	DWORD ret = 0;
	if(index < DELAY_MAX)
		ret = m_gSystemSet->dwDelay[index];
	return ret;
}

void GetDelay(DWORD* delay)
{
	g_SystemSetCS.lockon();
	memcpy(delay, m_gSystemSet->dwDelay, sizeof(DWORD) * DELAY_MAX);
	g_SystemSetCS.lockoff();
}

void SetDelay(DWORD* delay)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		memcpy(m_gSystemSet->dwDelay, delay, sizeof(DWORD) * DELAY_MAX);
		UpdataSystemSet();
		PostSafeMessage(SMSG_DELAY, m_gSystemSet->dwDelay[DELAY_SAFE], m_gSystemSet->dwDelay[DELAY_ALARM], m_gSystemSet->dwDelay[DELAY_DURATION]);
	}
	g_SystemSetCS.lockoff();
}

void GetDisplayParam(DWORD* param)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
		memcpy(param, m_gSystemSet->displayParam, sizeof(DWORD) * 7);
	g_SystemSetCS.lockoff();
}

void SetDisplayParam(DWORD* param)
{
	g_SystemSetCS.lockon();
	if(m_gSystemSet != NULL)
	{
		memcpy(m_gSystemSet->displayParam, param, sizeof(DWORD) * 7);
		UpdataSystemSet();
	}
	g_SystemSetCS.lockoff();
}

void SetUpdateEn(BOOL bEnable)
{
	g_bUpdateEn = bEnable;
	if(g_bUpdateEn)
		UpdataSystemSet();
}

BOOL SetScreenOnOff(BOOL bOn)
{
	DWORD dwVal[7];
	GetDisplayParam(dwVal);
	if(!bOn)
	{
		dwVal[0] = 0;
	}

	return AdjustScreen(dwVal[0], dwVal[1], dwVal[2]);
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
//
//
//void DBSetTest()
//{
//	InitSystemSet();
//	SystemSet_t* p = m_gSystemSet;
//
//	m_gSystemSet->Endid = SETENDID;
//
//	SetMsgTime("2016-11-24 18:02");
//	SetWeatherTime("2016-11-24 18:02");
//	SetWeatherPng(5);
//	SetWeatherTemp("1度");
//	SetWeatherHum("2度");
//	SetTermId("1010101010101");
//	SetProjectPwd("666666");
//	SetSafePwd("555555");
//	SetUserPwd("333333");
//	SetHostagePwd("888888");
//	SetPublicUserPwd("654321");
//	SetPublicHostagePwd("123456");
//	SetIsExistNewMail(TRUE);
//	SetScreensaver(2);
//	SetScreenType(1);
//	SetTrusteeshipEn(1);
//	SetTalkVol(3);
//	SetKeyVol(4);
//	SetRingVol(5);
//	SetMuteMode(TRUE);
//	SetCameraEn(TRUE);
//	SetBgName("123456");
//	SetRingName(TRUE, "CallOut.mp3");
//	SetRingName(FALSE, "CallIn.mp3");
//	SetLeaveMsgType(1);
//	SetElevator(TRUE);
//	SetLanguage(0);
//	SetCallInTimeOut(120);
//	SetUnlockDelay(120, TRUE);
//
//}