#pragma once

#define SCREEN_CALIBRATE

#ifdef DPCE
#define SDCARD			"/CardA"
#endif

#ifdef DPLINUX
#define SDCARD			"/sdcard"
#endif

#define FRAME_WIDTH				800
#define FRAME_HEIGHT			480

#define V_NORMAL_LEFT			134
#define V_NORMAL_TOP			80
#define V_NORMAL_WIDTH			532
#define V_NORMAL_HEIGHT			320

#define SELECT_LEFT				55
#define SELECT_LEFT2			165
#define SELECT_TOP				95
#define SELECT_HEIGHT			45

#define USERDIR					"/UserDev"
#define FLASHDIR				"/FlashDev"
#define WINDOWSDIR				"/Windows"
#define LAYOUT_DIR				"/FlashDev/Layout"

#define DOWNLOAD_FILE_PATH		"/Windows/temp.dat"
#define	DOWNLOAD_IMAGE_PATH		"/Windows/Image.dd"
#define	IPTABLE_NAME			"NetCfg.dat"
#define	IPTABLE_BAK				"NetCfg.bak"
#define	TEMP_MSG_PATH 			"/Windows/msg.tmp"
#define LIUYAN_FILE_PATH		"/Windows/liuyan.wav"

#define IMAGE_OBJECT_SIZE		(6 * 1024 * 1024)	// 6M
#define NORMAL_OBJECT_SIZE		(2 * 1024 * 1024)	// 6M

#define WARNINGINFO				"============================>>>>>>>>>>>>>>>>>>>>>"


/************************************************************************/
/*                                                                      */
/************************************************************************/
#define	SAFE_MAX_NUMBER			8

#define SAREA_KITCHEN			0	// 厨房
#define SAREA_BEDROOM			1	// 卧室
#define SAREA_HALL				2	// 大厅	
#define SAREA_WINDOW			3	// 窗户
#define SAREA_DOOR				4	// 大门
#define SAREA_BALCONY			5	// 阳台
#define SAREA_GUEST				6	// 客房	
#define SAREA_MAX				7	
#define SAREA_LOCALHOST			89	// 室内机

#define STYPE_URGENT			0	// 紧急
#define STYPE_FIRE				1	// 烟感
#define STYPE_GAS				2	// 煤气	
#define STYPE_MAGNETIC			3	// 门磁
#define STYPE_IR				4	// 红外
#define STYPE_WINDOW			5	// 窗磁
#define STYPE_CLASS				6	// 玻璃	
#define STYPE_MAX				7	

#define STYPE_HOSTAGE	        19	// 挟持
#define STYPE_TAMPER			20	// 防拆
#define STYPE_VOLTAGE			21	// 低压

#define SSTATUS_CANCEL_DEFENSE		0	//未布防
#define SSTATUS_SET_DEFENSE			1	//已布防
#define SSTATUS_ALARM_DELAY			2	//报警延时中	
#define SSTATUS_ALARMING			3	//报警中

#define AREAMODE_HOME		0
#define AREAMODE_LEAVE		1
#define AREAMODE_MAX		2

#define	SMODE_TEST		0		// 测试模式
#define	SMODE_UNSET		1		// 撤防模式
#define	SMODE_INHOME	2		// 在家模式
#define	SMODE_NIGHT		3		// 夜间模式
#define	SMODE_LEAVE		4		// 离家模式
#define	SMODE_MAX		5		//

#define	SLEVEL_LOW		0		// 低电平
#define	SLEVEL_HIGH		1		// 高电平

#define	TANTOU_IDLE		0		// 正常
#define	TANTOU_DELAY	1		// 报警延时
#define	TANTOU_ALARM	2		// 报警中
#define	TANTOU_DISALARM	3		// 管理中心消警

	