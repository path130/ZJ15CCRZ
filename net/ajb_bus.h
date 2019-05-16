/*
 * ajb_bus.h
 *
 *  Created on: 2012-7-5
 *
 */

#ifndef AJB_BUS_H_
#define AJB_BUS_H_

/*安居智能系统通讯协议*/
#define SNUL	0x01     //线路空、清线路使线路各设备都处在允许发送状态
#define SASK	0x02     //发送请求
#define AMAY	0x03     //发送允许
#define SERR	0x04     //发送错误*
#define SDAT	0x05     //数据报*
#define SEND	0x06     //发送结束
#define SOK 	0x07     //正确回答
#define STRN	0x08     //请求发送报警信息*
#define SORD	0x09     //接收报警数据*
#define SANS	0x0A     //提机
#define SFAL	0x0B	 //发送失败*
#define NC01	0x0C	 //
#define NC02	0x0D	 //
#define SLED	0x0E	 //点亮指示灯*
#define SCLO	0x0F	 //全清除*
/**********************************************************************
*:表明该命令目前已在系统中取消
NC0*:为该命令码没有使用
**********************************************************************/
#define MVCD	0x10	//转动球  //
#define STEL	0x11     //呼叫
#define SONE	0x12     //主机在管理模式下的呼叫
#define NC11	0x13     //
#define NC12	0x14     //
#define SSINT	0x15     //非正常结束
#define SDIS	0x16     //显示主机呼叫号码　//查询分机的号码
#define STRS	0x17     //在有主机分机中分机呼叫
#define SSION	0x18     //紧急（群呼）
#define SSCS	0x19     //给分机送密码开锁与撤防信息
#define SSSK	0x1A     //密码开锁
#define SSIC	0x1B     //刷卡后，主机开锁与专分机撤防
#define FJSC	0x1C     //副门口机刷卡
#define SSSS	0x1D     //管理模式
#define SCRE	0x1E     //分机发出关监视
#define SCRT	0x1F     //分机发出开监视
/**********************************************************************
NC1*:为暂时没有使用
**********************************************************************/
#define NC20 	0x20     //
#define SMFC 	0x21     //副卡头编码
#define SKEY	0x22     //开锁

#define SGLC	0x23	//管理机监视
//大门口机
#define SBIG	0x24
#define NC22 	0x25     //
 //#define NC23 	0x26
#define SGLT	0x26  	// 管理机关闭监视
#define NC24	0x27     //
#define NC25	0x28     //
#define NC26	0x29     //
#define NC27	0x2A     //
#define STSY	0x2B     //电话密码验证
#define STOK	0x2C     //电话密码正确
#define STBF	0x2D     //电话布防
#define NC28	0x2E     //
#define SDEF	0x2F     //同号分机自呼
/**********************************************************************
NC2*:为暂时没有使用
**********************************************************************/
#define SPOL	0x30     //早期四防区报警
#define SPOL1	0x31     //
#define SPOL2	0x32     //
#define SPOL3	0x33     //
#define SPOL4	0x34     //
#define SPOL5	0x35     //
#define SPOL6	0x36     //
#define SPOL7	0x37     //
#define SPOL8	0x38     //
#define SPOL9	0x39     //
#define SPOLA	0x3A     //
#define SPOLB	0x3B	 //
#define SPOLC	0x3C	 //
#define SPOLD	0x3D	 //
#define SPOLE	0x3E	 //
#define SPOLF	0x3F	 //
/**********************************************************************
说明:
  命令字节:　　 0 0 1 1 X1 X2 X3 X4━━┓
          　　┌0 0 0 0  0  0  0  0　　┃
　管理机地址　┫0 0 0 0  0  0  0  0　　┃
		　　　└0 0 0 0  0  0  0  0　　┃
		　　　┌X X X X  X  X  X  X　　┃
　报警分机地址┫X X X X  X  X  X  X　　┃
		　　　└X X X X  X  X  X  X　　┃
　　　　　　　　　　　　　　　　　 　　┃
　　　　　  X1    X2    X3    X4 ＜━━┛
            ┳　　┳　　┳　　┳
			┃　　┃　　┃　　┗━━━━第四防区
			┃　　┃　　┗━━━━━━━第三防区
			┃　　┗━━━━━━━━━━第二防区
			┗━━━━━━━━━━━━━第一防区
  注：其中30H代替33H
**********************************************************************/
#define NC40	0x40     //
#define NC41	0x41     //
#define SPCL	0x42     //报警*1
#define SSID	0x43     //边界报警
#define SIC 	0x44     //刷卡/发卡
#define SSG 	0x45     //巡更
#define SWRI	0x46     //写卡
#define SDEL	0x47     //删除卡
#define SFOR	0x48     //格式化
#define SRED	0x49     //读卡
#define SSOK	0x4A     //写卡回答
#define SSCB	0x4B     //分机发向计算机的撤/布防信息*2
#define NC42	0x4C     //
#define NC43	0x4D     //
#define FAPF	0x4E     //分机给PC发防区布撤防状态
#define NC45	0x4F     //
#define PCBC	0x51	//PC遥控分机布撤防(含应答)
#define QZTY	0x52	//各种求助类型

#define CKFQ	0x5B	//分机八防区状态查询(含应答)
#define CKFS	0x5C	//查询防区异常状态

#define SCCS  0x5D //设置卡头扇区 wrm 20150619
#define SCCP  0x5E //设置卡头密码 wrm 20150619

#define SCODE	0x88	//向控制器发送主机编码指令
#define SBPC	0x93	//报警命令

#define RINF	0x64	//信息查询请求
#define SINF	0x65	//通知分机有信息
#define SQIM	0x66    //分机对主机的拍录图像查询协议
#define SVIF	0x67	//查询个人信息
#define SUIF	0x68	//查询共公信息
#define SECM	0x69	//查询进出及刷卡的图像
#define SLYF  0x6A	//查询访客留言

#define SQIF	0xDA	//存储共公信息

#define SBUY	0x87	//线路忙的应答
#define ICJC	0x7A	//允许呼叫

#define SHFI	0x8A	//配置楼层信息
#define SCNO	0x8C	//设置层解的PORT所对应的分机

#define SSCT	0x9A	//监视小门口机
#define SSCE	0x9B	//关闭监视

#define ADEL	0xDB	//杠六主机删除卡 3开始为卡号 5开始为卡号

#define DWRT	0x90	//无线防区数据

#define CHSH	0xDC	//巡检独立门禁
#define XTMM	0xDD	//系统设置密码
//////////////////////////////////////////////家电控制命命令字////////////////////////////////////////
#define LIGHT_BL		0x00	//保留
#define LIGHT_ON		0xF0    //Turn On
#define LIGHT_OF		0xF1    //Turn Off
#define LIGHT_TG		0xF4    //单键开关/调光
#define LIGHT_AD		0xF8    //调光加
#define LIGHT_MI     0xF9    //调光减
#define LIGHT_JM     0xE0    //进入情景拍照模式
#define LIGHT_CZ     0xE1    //情景控制
#define LIGHT_PZ     0xE2    //情景拍照
#define LIGHT_AO     0xE8    //全开
#define LIGHT_AF     0xE9    //全关
#define LIGHT_ST		0xD0    //红外学习


#define GETVI    	  0XFE
#define GETUK     0XFD

#define R_UNLOCK    0x70
#endif /* AJB_BUS_H_ */
