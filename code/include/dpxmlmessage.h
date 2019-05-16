#pragma once

#include "dpsyncmsg.h"

//服务器与终端
#define MC_TYPE_SYNCHRO			1001	//同步
#define MC_TYPE_SYNCHRO_ACK		1002
#define MC_TYPE_UPDATE			1003	//更新
#define MC_TYPE_UPDATE_ACK		1004	
#define MC_TYPE_ALARM			1005	//报警
#define MC_TYPE_CLEANALARM		1007	//取消报警
#define MC_TYPE_LOCKLOG			1009	//开锁记录

#define	MC_TYPE_OPEN_LOCK		1101
#define MC_TYPE_OPEN_LOCKTWO	1301
#define	MC_TYPE_ADD_CARD		1201
#define	MC_TYPE_DEL_CARD		1203
#define	MC_TYPE_SYNC_CARD		1207

#define	MC_TYPE_PRIVATE_MSG		1205
#define	MC_TYPE_MAIL_BOX		1501	//信报箱 seda专用功能,提示住户有新的信件

#define XMLMSG_CHECKID			0xabcd1234
#pragma pack(1)
typedef struct 
{
	DWORD head;					//消息开始编码, 0xabcd1234
	DWORD length;				//消息主体的长度，包括消息头
	WORD content[];				//消息主体,XML格式	WCHAR
} MSGHEAD,*LPMSGHEAD;

typedef struct
{
	BYTE ictype;	// IC卡类型，1=住户卡，2=巡更卡
	DWORD iccode; 	// IC卡卡号
	UINT64 roomid;	// 室内机号码
	DWORD ExpDate;	// 到期日，如20130918 ，如果为0，则不限制到期（目前为保留字段）
} tagCard;
#pragma pack()

