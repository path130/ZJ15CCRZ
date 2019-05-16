#pragma once

typedef struct
{
	DWORD x;
	DWORD y;
	DWORD flag;
} TOUCHDATA;

typedef struct
{
	DWORD key;
	DWORD flag;
} KBDDATA;

typedef struct
{
	DWORD msg;
	DWORD wParam;
	DWORD lParam;
	DWORD zParam;
} SYS_MSG;

#include "appid.h"
#define	MSG_URGENT_TYPE			0
#define	MSG_USER_TYPE			1
#define	MSG_TIME_TYPE			2
#define	MSG_TOUCH_TYPE			3
#define	MSG_KEY_TYPE			4
#define	MSG_MAX_TYPE			5

#define	TIME_MESSAGE			10000		// ʱ����Ϣ

#define HARDKBD_MESSAGE  		10001		// ������ԭʼ��Ϣ
#define	KBD_MESSAGE				10002
#define	KBD_DOWN				0
#define	KBD_UP					1
#define KBD_CTRL				2			// ���̿ؼ���Ϣ

#define TOUCH_RAW_MESSAGE		10003		// ��������ԭʼ��Ϣ��������ĻУ�������
#define TOUCH_DOWN				0			// ĳ������������
#define TOUCH_VALID				1			// ĳ���������º��ƶ�������
#define TOUCH_UP				2			// �����µİ������ͷ�
#define	TOUCH_MOVEOUT			3			// �����µİ������Ƴ�

#define	TOUCH_MESSAGE			10004		// ĳ���ؼ�����������Ϣ

#define	MSG_SYSTEM				10005		// ϵͳ��Ϣ
#define	REBOOT_MACH				1			// �����ն�
#define	UPDATE_NETCFG			2			// ��������
#define	CODE_CHANGE				3			// ����仯
#define	RESET_MACH				4			// �ָ���������
#define WATCHDOG_CHANGE			5			// �Ƿ�ʼ���Ź�			

#define	MSG_BROADCAST			10006		// �㲥��Ϣ����ǰ���ڵ����д��ڶ������յ�
#define	NETWORK_CHANGE			1			// ����״̬�仯
#define	PHOTO_CHANGE			2			// ��Ӱ״̬�仯
#define	MESSAGE_CHANGE			3			// ��Ϣ�仯
#define	SAFE_CHANGE				4			// �����仯
#define ALARM_CHANGE			5			// �����仯
#define	CONFIG_GET				6			// ��ȡ����
#define ELEVATOR_RET            7			// �ݿ���Ϣ
#define VOLUEME_CHANGE			8			// �����ı�
#define WEATHER_CHAGE			9			// �¶ȱ仯
#define CALLLOG_CHAGE			9			// δ��ͨ��

#define	MSG_PRIVATE				10007		// ˽����Ϣ�����͸�ָ���Ĵ���, wparamΪ���ڵ�id

#define	MSG_SHOW_STATUE			10008
#define	OPERATE_FAIL			0
#define	OPERATE_OK				1
#define	EXTERNAL_ALARM			2			// SOS ���𱨾�

#define	MSG_PHONECALL			10010		// ������Ϣ

#define	MSG_START_APP			10020		// �����µĴ���
#define	MSG_END_APP				10021		// ����ָ���Ĵ���
#define	MSG_START_FROM_ROOT		10022		// ����ָ���Ĵ��ڣ���������ǰ���еĴ���
#define	MSG_EXTSTART_APP		10023		// �����߳��������̷߳��͵Ĵ���������Ϣ

void InitSysMessage(void);
BOOL DPPostMessage(DWORD msgtype, DWORD wParam, DWORD lParam, DWORD zParam, DWORD type);
DWORD DPGetMessage(SYS_MSG* msg);
void CleanUserInput(void);
