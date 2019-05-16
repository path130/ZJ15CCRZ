#pragma once

/* Open Type Define */
#define	IDCARD_OPEN				0x01	// 用户卡开锁，记录卡号
#define	USERPWD_OPEN			0x02	// 住户密码开门，记录房号
#define	PUBPWD_OPEN				0x03	// 公共密码开锁，不记录其他信息
#define	ROOM_OPEN				0x04	// 室内机开锁，记录住户房号
#define	MCENTER_OPEN			0x05	// 管理中心开锁， 记录管理员号码
#define	XG_OPEN					0x06	// 巡更卡开锁  记录卡号
#define	INVALID_CARD			0x07	// 非法卡

#define	SYNC_SEND_OPENRECORD	1		// 发送开锁记录

#define	SYNC_SEND_ALARM			2		// 发送报警消息
#define	MAGNETIC_ALARM			1		// 门磁报警
#define	HOSTAGE_ALARM			2		// 挟持报警
#define	TAMPER_ALARM			3		// 防拆报警

#define	SYNC_SEND_ELEVATOR		3		// 发送梯控消息给主门口机

typedef struct 
{
	char CompInfo[512];
	char CompInfoEn[512];
	char HardInfo[512];
} HardInfo_T;

typedef struct
{
	DWORD msg;
	DWORD wParam;
	DWORD lParam;
	UINT64 roomid;
} SyncMSG;

