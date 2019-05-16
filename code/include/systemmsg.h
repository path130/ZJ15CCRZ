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

#define	TIME_MESSAGE			10000		// 时间消息

#define HARDKBD_MESSAGE  		10001		// 按键的原始消息
#define	KBD_MESSAGE				10002
#define	KBD_DOWN				0
#define	KBD_UP					1
#define KBD_CTRL				2			// 键盘控件消息

#define TOUCH_RAW_MESSAGE		10003		// 触摸屏的原始消息，用于屏幕校正或黑屏
#define TOUCH_DOWN				0			// 某个按键被按下
#define TOUCH_VALID				1			// 某个按键按下后移动触摸点
#define TOUCH_UP				2			// 被按下的按键被释放
#define	TOUCH_MOVEOUT			3			// 被按下的按键被移出

#define	TOUCH_MESSAGE			10004		// 某个控件被触发的消息

#define	MSG_SYSTEM				10005		// 系统消息
#define	REBOOT_MACH				1			// 重启终端
#define	UPDATE_NETCFG			2			// 更新配置
#define	CODE_CHANGE				3			// 号码变化
#define	RESET_MACH				4			// 恢复出厂设置
#define WATCHDOG_CHANGE			5			// 是否开始看门狗			

#define	MSG_BROADCAST			10006		// 广播消息，当前存在的所有窗口都可以收到
#define	NETWORK_CHANGE			1			// 网络状态变化
#define	PHOTO_CHANGE			2			// 留影状态变化
#define	MESSAGE_CHANGE			3			// 消息变化
#define	SAFE_CHANGE				4			// 安防变化
#define ALARM_CHANGE			5			// 报警变化
#define	CONFIG_GET				6			// 获取配置
#define ELEVATOR_RET            7			// 梯控消息
#define VOLUEME_CHANGE			8			// 声音改变
#define WEATHER_CHAGE			9			// 温度变化
#define CALLLOG_CHAGE			9			// 未接通话

#define	MSG_PRIVATE				10007		// 私有消息，发送给指定的窗口, wparam为窗口的id

#define	MSG_SHOW_STATUE			10008
#define	OPERATE_FAIL			0
#define	OPERATE_OK				1
#define	EXTERNAL_ALARM			2			// SOS 防拆报警

#define	MSG_PHONECALL			10010		// 呼叫消息

#define	MSG_START_APP			10020		// 启动新的窗口
#define	MSG_END_APP				10021		// 结束指定的窗口
#define	MSG_START_FROM_ROOT		10022		// 启动指定的窗口，并结束当前所有的窗口
#define	MSG_EXTSTART_APP		10023		// 其他线程往窗口线程发送的窗口启动消息

void InitSysMessage(void);
BOOL DPPostMessage(DWORD msgtype, DWORD wParam, DWORD lParam, DWORD zParam, DWORD type);
DWORD DPGetMessage(SYS_MSG* msg);
void CleanUserInput(void);
