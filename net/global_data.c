/*
 * global_data.c
 *
 *  Created on: 2012-8-9 上午10:28:35
 *  equ_description_s
 */
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "public.h"
#include "dev_info.h"
#include "dev_pro.h"
#include "global_data.h"
#include "global_def.h"
#include "character.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//											全局结构体GlobalData的管理											//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////



#define	INVHDVER	0xff	//无效硬件版本
#define	NULHDVER	0x00	//不存在硬件版本


#if DEV_TERMINAL
GlobalData	gData;
#endif

#if DEV_GATE
GateData	gData;
#endif

#if DEV_CONTROL
ControlData	gData;
#endif

#if DEV_GLJKZQ
ControlData	gData;
#endif

#if DEV_TERMINAL
GlobalData	gData_t=
{
		.DoorNo={0x88,0x88,0x88,0x88},
		.MAC_PassWord1={"AJB888"},		//第一次修改Mac地址密码
		.MAC_PassWord2={"888888"},		//一次以后修改Mac地址密码
		.MAC_PassWordTime=0,				//修改Mac地址次数
		.IP_PassWord1={"AJB888"},		//第一、二次修改IP地址密码
		.IP_PassWord2={"888888"},		//二次以后修改IP地址密码
		.IP_PassWordTime=0,				//修改IP地址次数
		.SortSelect=0,					//选择本机是主机还是副机1为主0为副机
		.TouSX=0,
		.TouSY=0,
		.TouSW=0,
		.TouSH=0,
		.ScreenSort=0,
		.HdVersion=0x00,					//硬件版本除出厂可设置外，其它的时候都不允许设置,版本号从1开始
		.SfVersion=0xff,					//软件做修改
		.Language=SIMP,
};

#endif

#if DEV_GATE
GateData gData_t=
{
/*		.DoorNo[3];
		.MyMac[6];
		.MyIP[4];
		.erverIP[4];*/
		.DoorNo={0x88,0x88,0x00},
		.MAC_PassWord1={"AJB888"},		//第一次修改Mac地址密码
		.MAC_PassWord2={"888888"},		//一次以后修改Mac地址密码
		.MAC_PassWordTime=0,				//修改Mac地址次数
		.IP_PassWord1={"AJB888"},		//第一、二次修改IP地址密码
		.IP_PassWord2={"888888"},		//二次以后修改IP地址密码
		.IP_PassWordTime=0,				//修改IP地址次数

		.InfoStor = 0,
		.GoMainScreenDelay = 60,
		.ScreenDelay=60,					//关屏的延时操作
		.GoMainScreenDelay=0,			//返回主界面的延时间
		.PubPassword={
				{"111111"},
				{"666666"},
				{"8888\0\0"},
				{"999999"}
		},
		.OperaCard={0,0},
		.DayOrNight = 0xFF,
		.AnimaWord={AnimaWord_s},
		.Codeflag = 2,
		.TouSX=0,
		.TouSY=0,
		.TouSW=0,
		.TouSH=0,

		.ScreenSort=1,
		.VoRx = 0x60,
		.VoTx = 0x40,
		.HdVersion=0x01,					//硬件版本除出厂可设置外，其它的时候都不允许设置,版本号从1开始
		.SfVersion=0x01,					//软件做修改
		.Language=SIMP,
/*
		.mdv_gw[6];
		.mdv_mask[6];
		.addr_gw[6];
*/
};
#endif

#if DEV_CONTROL
ControlData gData_t=
{
		.DoorNo={0x88,0x88,0x00},
		.MAC_PassWord1={"AJB888"},		//第一次修改Mac地址密码
		.MAC_PassWord2={"888888"},		//一次以后修改Mac地址密码
		.MAC_PassWordTime=0,				//修改Mac地址次数
		.IP_PassWord1={"AJB888"},		//第一、二次修改IP地址密码
		.InfoStor = 0,
		.Codeflag = 2,
		.VoRx = 0x60,
		.VoTx = 0x40,
};
#endif

#if DEV_GLJKZQ
ControlData gData_t=
{
		.DoorNo={0x00,0x00,0x00},
		.MAC_PassWord1={"AJB888"},		//第一次修改Mac地址密码
		.MAC_PassWord2={"888888"},		//一次以后修改Mac地址密码
		.MAC_PassWordTime=0,				//修改Mac地址次数
		.IP_PassWord1={"AJB888"},		//第一、二次修改IP地址密码
		.InfoStor = 0,
		.Codeflag = 2,
		.VoRx = 0x60,
		.VoTx = 0x40,
};
#endif


unsigned char MakeHex( unsigned char buf0 )
{
	unsigned char Hex = 0;
	Hex = (((buf0/10)<<4)&0xF0)+(buf0%10&0x0f);
	return Hex;
}


int GetgDataLen()
{
	return sizeof(gData);
}

unsigned char *GetgData()
{
	return (unsigned char*)(&gData);
}

unsigned char *GetgDataNo()
{
	return (unsigned char*)gData.DoorNo;
}


unsigned char *GetMyMac()
{
	return (unsigned char*)gData.MyMac;
}

unsigned char *GetServerIP()
{
	return (unsigned char*)gData.ServerIP;
}

unsigned char *GetMyIp()
{
	return (unsigned char*)gData.MyIP;
}


unsigned char IsMyMac(unsigned char *Buf)
{
	//printf("MyMac:%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n",gData.MyMac[0],gData.MyMac[1],gData.MyMac[2],gData.MyMac[3],gData.MyMac[4],gData.MyMac[5]);
	//printf("Buf:%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n",Buf[0],Buf[1],Buf[2],Buf[3],Buf[4],Buf[5]);

	if(memcmp( Buf,gData.MyMac,6) == 0)
		return 1;
	return 0;

}
unsigned char IsMyIp(unsigned char *Buf)
{
	if(memcmp( Buf,gData.MyIP,4) == 0)
		return 1;
	return 0;

}

unsigned char *GetMacPsWd1()
{
	return (unsigned char*)gData.MAC_PassWord1;
}
unsigned char *GetMacPsWd2()
{
	return (unsigned char*)gData.MAC_PassWord2;
}
/*unsigned char *GetIpPsWd2()
{
	return (unsigned char*)gData.IP_PassWord2;
}*/
unsigned char *GetIpPsWd1()
{
	return (unsigned char*)gData.IP_PassWord1;
}

unsigned char IsHostDoor()//本主机是主门口机
{
	if(gData.DoorNo[2]==0)
		return 1;

	return 0;
}

/*
unsigned short GetTouSX()
{
	return gData.TouSX;
}
void SetTouSX(unsigned short TouSX)
{
	gData.TouSX = TouSX;
}

unsigned short	GetTouSY()
{
	return gData.TouSY;
}
void SetTouSY(unsigned short TouSY)
{
	gData.TouSY = TouSY;
}

unsigned short	GetTouSW()
{
	return gData.TouSW;
}
void SetTouSW(unsigned short TouSW)
{
	gData.TouSW = TouSW;
}


unsigned short	GetTouSH()
{
	return gData.TouSH;
}
void SetTouSH(unsigned short TouSH)
{
	gData.TouSH = TouSH;
}

int GetScreenSort()
{
	return gData.ScreenSort;
}
void SetScreenSort(int ScreenSort)
{
	gData.ScreenSort = ScreenSort;
}

unsigned char GetHdVerion()
{
	return gData.HdVersion;
}

void SetHdver( unsigned char HdVer )
{
	gData.HdVersion = HdVer;
}

*/

int GetMacPassTime()
{
	return gData.MAC_PassWordTime;
}

void SetMyMac(unsigned char * Mac)
{
	memcpy(gData.MyMac,Mac,6);
}

void SetMyIp(unsigned char * Ip)
{
	memcpy(gData.MyIP,Ip,4);
}
void SetgDataNo(unsigned char * DoorNo)
{
	memcpy(gData.DoorNo,DoorNo,3);
}
void InMacPassTime()
{
	gData.MAC_PassWordTime++;
}

unsigned char GetVoTx()
{
	return gData.VoTx;
}

unsigned char GetVoRx()
{
	return gData.VoRx;
}

void SetTRx(unsigned char VoTx,unsigned char VoRx)
{
	gData.VoTx = VoTx;
	gData.VoRx = VoRx;
}



int save_global_data_t()
{

	int fd=open(G_DATA_PATH,O_WRONLY|O_CREAT,0664);
	if(fd<0){
		app_debug(DBG_FATAL,"open %s failure!\n",G_DATA_PATH);
		return -1;
	}

	if(write(fd,&gData,sizeof(gData))!=sizeof(gData)){
		app_debug(DBG_FATAL,"save_global_data failure!\n");
		close(fd);
		return -1;
	}

	int save_status=SAVE_OK;
	if(write(fd,&save_status,sizeof(save_status))!=sizeof(save_status)){
		app_debug(DBG_FATAL,"save_global_data failure!\n");
		close(fd);
		return -1;
	}

	if(fsync(fd)<0)
		perror("fsync save_global_data:");
	close(fd);
	return 0;
}

int load_global_data_t()
{
#if DEV_TERMINAL
	P_GlobalData	pData = &gData;
#endif
#if DEV_GATE
	P_GateData pData = &gData;
#endif
#if DEV_CONTROL
	P_ControlData pData = &gData;
#endif
#if DEV_GLJKZQ
	P_ControlData pData = &gData;
#endif

	unsigned char SaveFlag = 0;
	int save_status;
	int fd=open(G_DATA_PATH,O_RDONLY);
	if(fd<0){
		app_debug(DBG_FATAL,"open %s failure!\n",G_DATA_PATH);
		goto G_DATA_RECOVER;
	}
	off_t pos;
	pos=lseek(fd,sizeof(gData),SEEK_SET);
	if(pos!=sizeof(gData))
	{
		app_debug(DBG_ERROR,"%s format error!\n",G_DATA_PATH);
		close(fd);
		goto G_DATA_RECOVER;
	}else
	{
		if(read(fd,&save_status,sizeof(save_status))!=sizeof(save_status)){
			app_debug(DBG_FATAL,"read global_data failure!\n");
			close(fd);
			goto G_DATA_RECOVER;
		}else
		{
			if(SAVE_OK==save_status)
			{
				pos=lseek(fd,0,SEEK_SET);
				if(pos!=0)
				{
					app_debug(DBG_ERROR,"%s format error!\n",G_DATA_PATH);
					close(fd);
					goto G_DATA_RECOVER;
				}
				if(read(fd,pData,sizeof(gData))!=sizeof(gData)){
					app_debug(DBG_FATAL,"read global_data failure!\n");
					close(fd);
					goto G_DATA_RECOVER;
				}
			}
		}
	}
	close(fd);
	return 0;

G_DATA_RECOVER:
	SaveFlag=1;
	int serverip=get_server_ip();
#if DEV_TERMINAL
	memcpy(gData_t.DoorNo,my_devlist.DevNo,4);
#endif
#if DEV_GATE
	memcpy(gData_t.DoorNo,my_devlist.DevNo,3);
#endif
#if DEV_CONTROL
	memcpy(gData_t.DoorNo,my_devlist.DevNo,3);
#endif
#if DEV_GLJKZQ
	memcpy(gData_t.DoorNo,my_devlist.DevNo,3);
#endif
	memcpy(gData_t.MyIP,my_devlist.DevIp,4);
	memcpy(gData_t.ServerIP,&serverip,4);
	//get_local_mac(gData_t.MyMac);
	memcpy(&gData,&gData_t,sizeof(gData_t));
	if(save_global_data()<0)
		return -1;
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																											//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//static
DevList	gTmpDevList;
//static
DevList gComDevList;


void InitDevList()
{
	gTmpDevList.DevNo[0]=gData.DoorNo[0];
	gTmpDevList.DevNo[1]=gData.DoorNo[1];
	gTmpDevList.DevNo[2]=0x00;
	gTmpDevList.DevNo[3]=gData.DoorNo[2];

	gTmpDevList.DevIp[0]=gData.MyIP[0];
	gTmpDevList.DevIp[1]=gData.MyIP[1];
	gTmpDevList.DevIp[2]=gData.MyIP[2];
	gTmpDevList.DevIp[3]=gData.MyIP[3];
}

unsigned short GetMyDevNo(unsigned char *PtrDevNo)
{
	memcpy(PtrDevNo,gTmpDevList.DevNo,4);
	return 4;
}

unsigned char IsMyDevList(unsigned char *PtrDev)
{
	DevList RecDevList;

	memcpy(&RecDevList,PtrDev,sizeof(DevList));

	if( RecDevList.DevNo[0] == gTmpDevList.DevNo[0] && RecDevList.DevNo[1] == gTmpDevList.DevNo[1]&&
 	    RecDevList.DevNo[2] == gTmpDevList.DevNo[2] && RecDevList.DevNo[3] == gTmpDevList.DevNo[3] )
	{
		return 1;
	}

	return 0;
}

unsigned short MemCpyMyDevList(unsigned char *PtrDev)
{
	memcpy(PtrDev,&gTmpDevList,sizeof(DevList));

	return sizeof(DevList);
}

unsigned short GetComDevIp(unsigned char *PtrDevIp)
{
	memcpy(PtrDevIp,gTmpDevList.DevIp,4);
	return 4;
}

unsigned short SetComDevList(unsigned char *PtrDev)
{
	memcpy(&gComDevList,PtrDev,sizeof(DevList));

	return sizeof(DevList);
}



void SetExAddr(unsigned char *Addr,unsigned char *TmpAddr)
{

		Addr[0]= TmpAddr[0];
		Addr[1]= TmpAddr[1];
		Addr[2]= TmpAddr[2];
		Addr[3]= TmpAddr[3];
}



void SetAddr(unsigned char *Addr,unsigned char *TmpAddr,char MAddr)
{
		Addr[0]= TmpAddr[MAddr+0];
		Addr[1]= TmpAddr[MAddr+1];
		Addr[2]= TmpAddr[MAddr+2];
		Addr[3]= TmpAddr[MAddr+3];
}

void AddDooNo(unsigned char *Addr,char MAddr)
{
		Addr[0+MAddr]= gData.DoorNo[0];//TmpAddr[MAddr+0];
		Addr[1+MAddr]= gData.DoorNo[1];//TmpAddr[MAddr+1];
}

void SetSrcAddr(unsigned char *SrcAddr)
{
	SrcAddr[0] = gData.DoorNo[0];  //楼栋号
	SrcAddr[1] = gData.DoorNo[1];  //单元号,bit4~bit7为3位码单元号,bit0~bit3为4位码单元号
	SrcAddr[2] = 0x00;
	SrcAddr[3] = gData.DoorNo[2];  //主副号
}



void SetFjAddr(unsigned char *Addr,unsigned char *TmpAddr)
{
	Addr[0] = gData.DoorNo[0];
	Addr[1] = gData.DoorNo[1];
	Addr[2]= TmpAddr[1];
	Addr[3]= TmpAddr[2];
}

//把BCD分成十位和个位
unsigned char Make2TODec(unsigned char Bcd,unsigned char *Dec0,unsigned char *Dec1)
{
	*Dec0 = (Bcd>>4)&0x0f;
	*Dec1 = Bcd&0x0f;
	return 0;
}

unsigned char MakeDec(unsigned char buf0)
{
	unsigned char Dec = 0;
	Dec = ((buf0>>4)&0x0f)*10+(buf0&0x0f);
	return Dec;
}

unsigned char AddTotal(unsigned char *Data,unsigned char Len)
{
	unsigned char i;
	unsigned char result=0;

	for( i = 0 ; i < Len; i++ )
		result+=*(Data+i);
	return result;
}

unsigned char MemCmpInt( const unsigned char * SrcAddr,const unsigned char * DstAddr,unsigned short Count )
{
	unsigned short i;

	for( i = 0; i < Count; i++ )
	{
		if( SrcAddr[i] != DstAddr[i] )
			return i+1;
	}
	return 0;
}


unsigned char MakeBcd(unsigned char buf0,unsigned char buf1)
{
	unsigned char Bcd = 0;

	Bcd = ( buf0-'0') ;
	Bcd = ((Bcd <<4) & 0xf0)+(buf1-'0');
	return Bcd;
}


unsigned char Bcd2Dec(unsigned char buf0)
{
	unsigned char Dec = 0;
	Dec = ((buf0>>4)&0x0f)*10+(buf0&0x0f);
	return Dec;
}

unsigned char Dec2Bcd(unsigned char dec)
{
	return (unsigned char)(((dec/10)<<4) + dec%10) ;
}





/************************************************************************
*
*   FUNCTION
*
*       DBC_Chang_IP
*
*   DESCRIPTION
*
*       修改MAC地址
*
*   INPUTS
*
*       string              Pointer to where the item name will be placed
*
*   OUTPUTS
*
*       None
*
************************************************************************/

unsigned char *Getmdv_gw()
{
	return (unsigned char*)gData.mdv_gw;
}

void Setmdv_gw(unsigned char *mdv_gw )
{
	memcpy(gData.mdv_gw,mdv_gw,4);
}

unsigned char *Getmdv_mask()
{
	return (unsigned char*)gData.mdv_mask;
}

void Setmdv_mask(unsigned char *mdv_mask )
{
	memcpy(gData.mdv_mask,mdv_mask,4);
}



