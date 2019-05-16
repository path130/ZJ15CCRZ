/*
 * dev_info.h
 *
 *  Created on: 2012-8-9 上午10:41:09
 *
 */

#ifndef DEV_INFO_H_
#define DEV_INFO_H_


typedef struct _PDU_HEADER
{
	int op_code;
	int dhcp_ebl;
	unsigned char mac_addr[8];
	unsigned char ip_addr[4];
	unsigned char subnet_mask[4];
	unsigned char gateway_addr[4];
	unsigned char dns1[4];
	unsigned char dns2[4];
	unsigned char http_port[4];

	char equ_description[64];	//设备描述
	char equ_name[32];			//设备名称
	int  equ_type;				//设备类型
	char prod_name[64];			//生产商名称
	char type_name[32];			//类型名称

	char hardware_version[16];	//固件版本
	char software_version[16];	//软件版本
	char position[32];			//位置
	char description[64];		//描述

}pdu_head,*ppdu_head;

typedef struct {
	int dhcp_ebl;
	unsigned char mac_addr[8];
	unsigned char ip_addr[4];
	unsigned char subnet_mask[4];
	unsigned char gateway_addr[4];
	unsigned char dns1[4];
	unsigned char dns2[4];
}eth_para_t;

typedef struct tagDevList
{
	unsigned char 	DevNo[4];		// 设备别名
	unsigned char 	DevIp[4];		// 设备网络地址
	unsigned short	DevType;		// 设备类型
	unsigned int		DevStatus;		// 设备状态
	unsigned short	Default;
}DevList;


typedef struct DEVARRARY_T
{
	DevList dev;
	struct DEVARRARY_T *next;
}dev_arrary_t;
//#endif


#define	ZJCFJ			0x00000001L//门口机呼叫分机
#define	ZJCGJ			0x00000002L//门口机呼叫管理机
#define	FJCGJ			0x00000004L//分机呼管理机
#define	BFJCBFJ		0x00000008L//分机呼分机

#define	BFJCWFJ		0x00000010L//本分机呼其它楼栋分机
#define	DJCBFJ			0x00000020L//大门口机呼本分机
#define 	MAMOTOR		0x00000040L//管理机监视
#define	FJMOTOR		0x00000080L//分机监视

#define 	MAQUERRY		0x00000100L//管理机查询
#define	FJQUERRY		0x00000200L//分机查询
#define 	XZJCGJ 		0x00000400L//小门口机呼管理机
#define	OTHCALL		0x00000800L//其它呼叫，需要自锁

#define 	ALARMOUTPUT	0x00001000L//报警输出延时
#define 	XZJCFJ 		0x00002000L//小门口机呼分机
#define 	GJCGJ			0x00004000L//管理机呼管理机
#define	ROOMMOTOR		0x00008000L//监视摄相枪

#define	ZJDRCFJ		0x00010000L//本楼栋的主或副呼分机
#define	ZJDRCGJ 		0x00020000L//本楼栋的主或副呼管机
#define 	GJCFJ			0x00040000L//管理机呼分机
#define 	WFJCBFJ		0x00080000L//外分机呼本分机

#define 	CALLPRI		0
#define 	MOTORPRI		1


#define 	AUDIONRES		0x00000001L//管理机声音总线1被占用
#define 	VIDEORES		0x00000002L//管理机声音总线2被占用

//给层间解码器通道控制
#define	CONVIDEO		0x80
#define 	OPENVIDEO		0x40
#define 	CLOSEVIDEO		0x00
#define 	VIDEOONE		0x00
#define	VIDEOTWO		0x10
#define 	VIDEOTHR		0x20
#define	VIDEOFOU		0x30

#define	CONAUDIO		0x08
#define	OPENAUDIO		0x04
#define 	CLOSEAUDIO		0x00
#define 	AUDIOONE		0x00
#define	AUDIOTWO		0x01
#define 	AUDIOTHR		0x02
#define	ADUIOFOU		0x03

#define	ZJTYPE	0
#define	FJTYPE	1
#define	GJTYPE	2
#define	DJTYPE 3


// 365平台视频格式

#define 	VIDEO_640		0x01000000L
#define 	VIDEO_720		0x02000000L
#define 	VIDEO_1024		0x04000000L
#define 	VIDEO_720P		0x08000000L
#define 	VIDEO_OTH		0x00000000L
#define 	VIDEO_MASK		(VIDEO_640|VIDEO_720|VIDEO_1024|VIDEO_720P|VIDEO_OTH)

// 365和兼容平台区分标志

#define 	TYPE_JDM365	0x00000000
#define 	TYPE_OTHER		0x00000001
#define 	TYPE_D2100		0x00000002
#define 	TYPE_D3G		0x00000004

//错语类型
#define	CALLSUCESS		0
#define	BUSYLINE		-2//线忙
#define	OFFLINE		-3//被呼设备不在
#define	INPUTERROR 	-4
#define 	WAITCALLACK	-5 //走网络的系统正在等应答,还没有确定是否呼叫

#define	DEV_NO_NULL      0xffeeffee
#endif /* DEV_INFO_H_ */
