/*
 * global_data.h
 *
 *  Created on: 2012-10-30 上午10:04:28
 *  
 */

#ifndef GLOBAL_DATA_H_
#define GLOBAL_DATA_H_

#include <sys/types.h>
#include "dev_info.h"
#include "character.h"




#define UINT8 u_int8_t
#define UINT16 u_int16_t

#define G_DATA_PATH "global_data.bin"
#define SAVE_OK 0xffffffff

#define   MODETYPE	5                                   //5种模式
#define   MAXFANGQU	32

#define ENG		0
#define SIMP    1
#define TRAD    2

#define		CON_2		0//二位码编码
#define		CON_3		1//三位主机码
#define		CON_4		2//四位码主机

typedef	struct FangMode
{
	unsigned int FQLise;
	unsigned int BFdelay;
}TModeFang;

//可视分机结构
typedef	struct tagFangQu
{
	unsigned char	GuardStatus;						//1,布防，,0不布防
	unsigned char	SelectWire;						//线路选择
	unsigned char	FangQuStatus;						//防区的各种状态  异常, 正常，未安装
	unsigned char	Name[12];							//防区名称	,最多五个字符
	unsigned char	Ticks;								//防区消抖
}FangQuInfo;


typedef struct	SYSTEM_DATA			//	系统数据图
{
	unsigned short	SaveFlagBegin;
	unsigned char		MyMac[6];						//本机Mac地址
	unsigned char		MyIP[4];						//本机IP地址
	unsigned char		NodeServerIP[4];				//节点服务器IP地址
	unsigned char		ServerIP[4];					//PC IP地址
	unsigned short	SaveFlagEnd;
}system_data;

typedef struct TagHomeElectric
{
	u_int8_t	Type;         //0:空,1:灯,2:空调,3:窗帘
	u_int8_t	DataType;		//发送数据的类型 1:普通型 2:节能型
	u_int8_t 	state;			//1:开,0:关
	u_int8_t 	Number;		//设备的编号
	u_int8_t	Name[4];      //设备名
}ElectricEquipment,*P_ElectricEquipment;

//定义一个全局的结构体，此主要是用来存储程序中需要保存的数据，以便于管理
typedef struct TagGlobalData
{
	unsigned char 	DoorNo[4];						//当前主机的编码,一位房号等
	unsigned char		MyMac[6];						//本机Mac地址
	unsigned char		MyIP[4];						//本机IP地址
	unsigned char		ServerIP[4];					//PC IP地址

	unsigned char		MAC_PassWord1[8];				//第一次修改Mac地址密码
	unsigned char		MAC_PassWord2[8];				//一次以后修改Mac地址密码
	int 				MAC_PassWordTime;				//修改Mac地址次数
	unsigned char		IP_PassWord1[8];				//第一、二次修改IP地址密码
	unsigned char		IP_PassWord2[8];				//二次以后修改IP地址密码
	int 				IP_PassWordTime;				//修改IP地址次数

	unsigned char		PW[4];							//用户密码
	unsigned char		SortSelect;					//选择本机是主机还是副机1为主0为副机
	FangQuInfo			tmpgFangQu[8];              //防区
	TModeFang    		ModeFangQuStatus[MODETYPE]; //防区状态

	u_int8_t			LightSort[15][12][1];
	unsigned char		FQState;
	unsigned char		CloseScrTime;
	unsigned char		TalkSelect;
	unsigned short	TouSX;
	unsigned short	TouSY;
	unsigned short	TouSW;
	unsigned short	TouSH;
	int					ScreenSort;
	u_int8_t			RoomName[15][6];
	u_int8_t			WatchFlag;

	int16_t			WirlessData[3][48];
	u_int8_t			WirlessFQAdress[138][7];         //类型1 地此3 状态1
	u_int8_t			WirlessDataNumber;
	//u_int8_t 		JJANNumber;
	u_int8_t			BlackLevel;
	u_int8_t			Brightness;
	u_int8_t			SongIndex[8];				//0的Index表示 门口主机 1表示管理机 2表示的小门口机 3表示的是大门口机，其它暂无用
	u_int8_t			SongVolcancel;
	u_int8_t			HdVersion;					//硬件版本除出厂可设置外，其它的时候都不允许设置,版本号从1开始
	u_int8_t			SfVersion;					//软件做修改
	u_int8_t			Address[2];
	u_int8_t			Endflag;
	FangQuInfo			gFangQu[MAXFANGQU];

	unsigned char		VoTx;
	unsigned char		VoRx;
	u_int8_t			Language;
	unsigned char		PreFQState;

	u_int8_t			mdv_gw[6];
	u_int8_t			mdv_mask[6];
	u_int8_t			RdVer;
	u_int8_t			RoomRfNo[15];//15个房间无线发射器的编码
	u_int8_t			SongVol[5];
	u_int8_t			EDdevLight[15][28];//记录每个电灯设备的通讯地址//每个房间的前4个为电灯后一个为转发器
	u_int8_t			EDdevLightvalue[15][4]; //为两位开关设计.记录是两们开关中的哪一位.
}GlobalData,*P_GlobalData;

//主机控制器结构
typedef struct TagControlData
{
	unsigned char 	DoorNo[3];						//当前主机的编码,一位房号等
	unsigned char		MyMac[6];						//本机Mac地址
	unsigned char		MyIP[4];						//本机IP地址
	unsigned char		ServerIP[4];					//PC IP地址

	unsigned char		MAC_PassWord1[8];				//第一次修改Mac地址密码
	unsigned char		MAC_PassWord2[8];				//一次以后修改Mac地址密码
	int 				MAC_PassWordTime;				//修改Mac地址次数
	unsigned char		IP_PassWord1[8];				//第一、二次修改IP地址密码
	//unsigned char		IP_PassWord2[8];				//二次以后修改IP地址密码

	unsigned char		ZJFlag;
	unsigned char		InfoStor;						//信息存储器
	unsigned char 	DefStr[6];						//暂时不用，可做其它用
	int 				VideoFormat;				//视频接法

	unsigned char		Codeflag;

	unsigned char		VoTx;
	unsigned char		VoRx;
	unsigned char		UsdFlag;				//当前管理机是否有有人什班
	UINT8				mdv_gw[6];
	UINT8				mdv_mask[6];
	UINT8				addr_gw[6];//地址解析网关
}ControlData,*P_ControlData;


typedef struct TagHouseData{
 	unsigned char 	DoorNo[4];						//当前主机的编码,一位房号等
	unsigned char		MyMac[6];						//本机Mac地址
	unsigned char		MyIP[4];						//本机IP地址
	unsigned char		ServerIP[4];					//PC IP地址

	unsigned char		MAC_PassWord1[8];				//第一次修改Mac地址密码
	unsigned char		MAC_PassWord2[8];				//一次以后修改Mac地址密码
	int 				MAC_PassWordTime;				//修改Mac地址次数
	unsigned char		IP_PassWord1[8];				//第一、二次修改IP地址密码
	unsigned char		IP_PassWord2[8];				//二次以后修改IP地址密码
	int 				IP_PassWordTime;				//修改IP地址次数

	unsigned char		PW[4];							//用户密码
	unsigned char		SortSelect;					//选择本机是主机还是副机1为主0为副机
	FangQuInfo			tmpgFangQu[8];
	TModeFang			ModeFangQuStatus[MODETYPE];

	UINT8               LightSort[15][12][1];
	unsigned char       FQState;
	unsigned char       CloseScrTime;
	unsigned char       TalkSelect;
	unsigned short      TouSX;
	unsigned short      TouSY;
	unsigned short      TouSW;
	unsigned short      TouSH;
	int    				ScreenSort;
	UINT8               RoomName[15][6];
	UINT8               WatchFlag;

	UINT16               WirlessData[3][48];
	UINT8               WirlessFQAdress[138][7];         //类型1 地此3 状态1
	UINT8               WirlessDataNumber;
	//UINT8               JJANNumber;
	UINT8               BlackLevel;
	UINT8				Brightness;
	UINT8				SongIndex[8];				//0的Index表示 门口主机 1表示管理机 2表示的小门口机 3表示的是大门口机，其它暂无用
	UINT8				SongVol;
	UINT8				HdVersion;					//硬件版本除出厂可设置外，其它的时候都不允许设置,版本号从1开始
	UINT8				SfVersion;					//软件做修改
	UINT8				Address[2];
	UINT8       		Endflag;
	FangQuInfo			gFangQu[MAXFANGQU];

	unsigned char		VoTx;
	unsigned char		VoRx;
	UINT8 				Language;
	UINT8				mdv_gw[6];
	UINT8				mdv_mask[6];
	UINT8				RdVer;
	UINT8				RoomRfNo[15];//15个房间无线发射器的编码
	UINT8				addr_gw[6];//地址解析网关
}HouseData,*P_HouseData;


//主机结构
typedef struct TagGateData
{
	unsigned char 	DoorNo[3];						//当前主机的编码,一位房号等
	unsigned char		MyMac[6];						//本机Mac地址
	unsigned char		MyIP[4];						//本机IP地址
	unsigned char		ServerIP[4];					//PC IP地址

	unsigned char		MAC_PassWord1[8];				//第一次修改Mac地址密码
	unsigned char		MAC_PassWord2[8];				//一次以后修改Mac地址密码
	int 				MAC_PassWordTime;				//修改Mac地址次数
	unsigned char		IP_PassWord1[8];				//第一、二次修改IP地址密码
	unsigned char		IP_PassWord2[8];				//二次以后修改IP地址密码
	int 				IP_PassWordTime;				//修改IP地址次数
	unsigned char		InfoStor;
	unsigned short	ScreenDelay;					//关屏的延时操作
	unsigned short	GoMainScreenDelay;			//返回主界面的延时间
	unsigned char		PubPassword[4][7];			//用户开锁密码 门口操作员密码 管理密码 出厂密码
	unsigned int		OperaCard[2];					//门口机操作员卡 管理员卡
	unsigned char 	DayOrNight;					//模式设置白天OR晚上
	unsigned char		AnimaWord[24];				//楼栋地址
	unsigned char		Codeflag;

	unsigned short	TouSX;
	unsigned short	TouSY;
	unsigned short	TouSW;
	unsigned short	TouSH;

	int 				ScreenSort;					//修改IP地址次数

	unsigned char		VoTx;
	unsigned char		VoRx;
	unsigned char		HdVersion;						//硬件版本除出厂可设置外，其它的时候都不允许设置,版本号从1开始
	unsigned char		SfVersion;						//软件做修改
	unsigned char		Language;						//语言种类

	unsigned char		mdv_gw[6];
	unsigned char		mdv_mask[6];
	UINT8				addr_gw[6];					//地址解析网关
	unsigned char 		Dns1[4];
	unsigned char 		Dns2[4];
	unsigned char 	My_Village_No[4];				//当前主机所在小区号
       unsigned char        Seed_Data[16];
}GateData,*P_GateData;



#if DEV_GATE
extern GateData	gData;

#elif DEV_CONTROL
extern ControlData	gData;

#elif DEV_GLJKZQ
extern ControlData	gData;

#else // DEV_TERMINAL
extern GlobalData	gData;
#endif


int GetgDataLen();
unsigned char *GetgData();
unsigned char Dec2Bcd(unsigned char dec);
unsigned char Bcd2Dec(unsigned char buf0);
unsigned char *GetgDataNo();
unsigned char *GetMyMac();
unsigned char *GetServerIP();
unsigned char *GetMyIp();
unsigned char IsMyMac(unsigned char *Buf);
unsigned char IsMyIp(unsigned char *Buf);
unsigned char *GetMacPsWd1();
unsigned char *GetMacPsWd2();
unsigned char *GetIpPsWd2();
unsigned char *GetIpPsWd1();
unsigned char IsHostDoor();//本主机是主门口机
unsigned short GetTouSX();
unsigned short	GetTouSY();
unsigned short	GetTouSW();
unsigned short	GetTouSH();
int GetScreenSort();
unsigned char GetHdVerion();

int is_vlan_enable(void);
int is_my_subnet_mask(unsigned char *buf);
int is_my_gateway_addr(unsigned char *buf);


void SetTouSX(unsigned short TouSX);
void SetTouSY(unsigned short TouSY);
void SetTouSW(unsigned short TouSW);
void SetTouSH(unsigned short TouSH);
void SetScreenSort(int ScreenSort);
void SetHdver( unsigned char HdVer );

int load_global_data();
int save_global_data();
int save_eth_para(eth_para_t *para);
int save_dev_eth_para(eth_para_t *para);


int save_global_data_t();
int load_global_data_t();

void get_dns1(unsigned char*dns);
void get_dns2(unsigned char*dns);

#endif /* GLOBAL_DATA_H_ */
