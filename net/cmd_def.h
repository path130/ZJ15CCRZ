/*
 * cmd_def.h
 *
 *  Created on: 2012-6-18
 *
 */

#ifndef CMD_DEF_H_
#define CMD_DEF_H_
#define CALL_3G
#define DOOR_ACCESS_CARD_LIST

#define	SPOR	"826\0"		//更新程序
#define	RPOR	"827\0"		//应答更新程序

#define	GREF	"828\0"    	//得到参数信息
#define	SREF	"829\0"		//送参数信息

#define	CREF	"830\0"		//远程配置参数信息
#define	RCRE	"831\0"		//应答远程序配置

#define	GLJB	"832\0"		//送管理机列表信息		节点→PC	（长度:12）
#define	RGLJ	"833\0"		//送管理机列表信息		PC→节点	 (IPINSTALL)

#define	SIOR	"510\0"		//信息发布
#define	RIOR	"511\0"		//应答信息发布

#define	BEOR	"512\0"		//主控备份进出记录
#define	RBOR	"513\0"		//应答主控备份

#define	SKJL	"514\0"		//主机发送门禁刷卡记录
#define	JLYD	"515\0"		//应答主机发送门禁刷卡记录

#define	FJSJ	"520\0"		//分机那边过来的网络数据
#define	SJYD	"521\0"		//应答分机那边过来的网络数据

#define	CJBD	"522\0"		//层间解码器列表报道
#define	RCBD	"523\0"		//PC应答

#define	CJLB	"524\0"		//PC设置层间解码器列表
#define	LBYD	"525\0"		//PC应答

#define	WXST	"526\0"		//删除分机上无线探头
#define	STYD	"527\0"		//分机应答成功

#define	WXLY	"528\0"		//网上留言,pc端发送访客留言
#define	LYYD	"529\0"		//分机应答接收成功

#define	WXJD	"530\0"		//红外家电控制
#define	JDYD	"531\0"		//分机应答接收成功

#define	PZLY	"532\0"		//取拍录留影
#define	RPZL	"533\0"		//拍录留影应答

#define	RTPC	"534\0"		//分机记录系统
#define	RRPC	"535\0"		//应答

#define	GDST	"536\0"		//取门状态		PC→节点
#define 	RDST	"537\0"		//应答

#define	GCJT	"538\0"		//取层间解码器列表		PC→主机控制器
#define 	RCJT	"539\0"		//层间解码器列表		主机控制器→PC

#define	CJLYP	"540\0"		//层间解码器列表		PC→主机控制器
#define 	RLYYP	"541\0"		//取层间解码器列表		主机控制器→PC

#define 	ZALR	"542\0"		//主机门禁报警
#define 	RZAR	"543\0" 	   //pc应答报警成功

#define 	BFZT	"544\0"		//分机发送各个布防模式下布防状态		节点→PC
#define 	RBFZ	"545\0" 	   //PC应答报警成功		PC→节点

#define 	SQFW	"546\0"		//分机取社区服务		节点→PC
#define 	FWYD	"547\0" 	   //PC应答社区服务列表 	PC→节点

#define	GFQS	"550\0"		//数字终端获得各防区布防状态
#define	RFQS	"551\0"		//PC发送模式下各防区布防状态

#define	RGOD	"552\0"		//Remote guard or deguard
#define 	RRGD	"553\0"		//Respond guard or deguard

#define 	FALR	"554\0"		//数字终端防区报警
#define	RFLR	"555\0"		//PC应答报警成功

#define	EALR	"556\0"		//数字终端紧急报警
#define	RELR	"557\0"		//服务器应答数字终端紧急报警

#define	DALR	"558\0"		//Deal Alarm
#define	RDAR	"559\0"		//Respond Deal Alarm

#define	CALR	"560\0"		//Cancel Alarm
#define	RCAR	"561\0"		//Respond Cancel Alarm

#define 	GNCP	"562\0"		//获取网络摄相机
#define 	RGNC	"563\0"		//PC返回网络摄相的配置

#define 	FJJS	"564\0"		//校时		节点→PC
#define 	RFJJ	"565\0"		//PC应答校时	 PC→节点

#define 	RTXL	"566\0"		//通讯录		节点→PC
#define 	GTXL	"567\0"		//PC应答通讯录	PC→节点

#define 	CHWL	"570\0"		//检测网络
#define 	RCHW	"571\0"		//pc应答检测网络信息

#define	GPCM	"572\0"		//数字终端获取pc上定义好的信息
#define	RPCM	"573\0"		//pc发送定义好的信息

#define	SPCM	"574\0"		//数字终端发信息给pc
#define	RPCD	"575\0"		//pc应答

#define	SPTE	"576\0"		//分机获取pc上定义好的一些服务电话
#define	RPTE	"577\0"		//pc发送定义好的一些服务电话

#define	CRDD	"578\0"		//远程电灯控制
#define	RCDD	"579\0"		//应答远程电灯控制

#define	RPFK	"580\0"		//pc获取数字终端存储的访客留影
#define	SPFK	"581\0"		//数字终端返回数据

#define	RPJQ	"582\0"		//pc设置情景控制命令
#define	SPJQ	"583\0"		//情景控制应答

#define	RPLM	"584\0"		//pc获取数字终端存储的留言
#define	SPLM	"585\0"		//数字终端返回数据

#define	RPDM	"586\0"		//数字终端向pc获取室名
#define	SPDM	"587\0"		//pc应答数据

#define	RPKT	"588\0"		//空调控制
#define	SPKT	"589\0"		//pc应答

#define	RPCH	"590\0"		//窗户控制
#define	SPCH	"591\0"		//pc应答

#define	WXBJ	"592\0"		//分机无线防区报警
#define	BJYD	"593\0"		//pc应答

#define	WXHZ	"594\0"		//分机获得各无线探头布防状态
#define	HZYD	"595\0"		//pc应答

#define	WXBF	"596\0"		//无线探头远程布撤防
#define	BFYD	"597\0"		//pc应答

#define	WXBD	"598\0"		//分机探头报道
#define	BDYD	"599\0"		//pc应答

#define	SKPZ	"600\0"		//取刷卡拍照		PC→节点
#define	PZYD	"601\0"		//刷卡拍照应答		节点→PC

#define	LQFW	"602\0"		//linux分机取社区服务		节点→PC
#define	LQYD	"603\0"		//PC应答社区服务列表 	PC→节点

#define	FJTJ	"604\0"		//linux分机取天气		节点→PC
#define	TJYD	"605\0"		//PC应答天气	PC→节点

#define	FJYM	"650\0"		//数字终端向pc软件要室名（特殊）		节点→PC
#define	YMYD	"651\0"		//数字终端接收数据		PC→节点

#define	RBFJ	"660\0"		//分机布撤防记录传后台		节点→PC090
#define	BFJD	"661\0"		//PC应答报警成功		PC→节点

#define	GCJ2	"662\0"		//取层间解码器列表（2200系统）		PC→主机控制器
#define	RCJ2	"663\0"		//层间解码器列表（2200系统）		主机控制器→PC

#define	ZCJ2	"664\0"		//层间解码器列表（2200系统）		PC→主机控制器
#define	RZCJ2	"665\0"		//取层间解码器列表（2200系统）		主机控制器→PC

#define	LTXL	"670\0"		//linux通讯录		节点→PC
#define	RLTX	"671\0"		//PC应答通讯录	PC→节点

#define	TFTP	"672\0"		//更新365 FTP 数据	节点→PC
#define	RFTP	"673\0"		//PC应答

#define	JDPZ	"674\0"		//家电配置	PC→节点
#define	RJDP	"675\0"		//应答PC

#define	GDAT	"518\0"		//管理机那边过来的网络数据
#define	RGDA	"519\0"		//应答管理机那边过来的网络数据

#define	HTOG	"700\0"		//设备报到
#define 	RTOG	"701\0"		//设备报到应答

#define 	NTEL	"704\0"		//主叫终端请求与被叫端点建立呼叫
#define	RTEL	"705\0"		//被叫终端许可确认主叫终端

#define	NEND	"708\0"		//主叫终端或被叫终端释放请求
#define	REND	"709\0"		//被叫终端许可确认主叫终端

#define 	NANS	"710\0"		//被叫终端向主叫终端请求提机
#define	RANS	"711\0"		//被叫终端许可确认主叫终端
#define 	CJIN	"712\0"		//设置层间控制器所接分机列表
#define	RCJI	"713\0"		//

#define 	HQFJ	"720\0"		//获取主机控制器分机列表
#define	RHQF	"721\0"		//

#define	SCTL	"714\0"		//设置通讯地址
#define 	RCTL	"715\0"		//应答


#define	GCLG	"724\0"		//获取主机名录
#define 	RGCL	"725\0"		//应答

#define	SCLG	"726\0"		//设置主机名录
#define 	RSCL	"727\0"		//应答

#define	GCRD	"740\0"		//获取主机卡头数据
#define RGCR	"741\0"		//应答
#define	SCRD	"742\0"	    //设置主机卡头数据
#define RSCR	"743\0"	    //应答

#define		SMJKN	"750\0"
#define		RMJKN	"751\0"
#define		UMJKN	"752\0"
#define		AMJKN	"753\0"

#define		QCPN	"756\0"
#define		RCPN	"757\0"
#define		SCPN	"758\0"
#define		ACPN	"759\0"

#define	FCBJ	"548\0"		//防拆报警
#define 	FCYD	"549\0"		//应答

#define 	CJINC	0x712  	//设置层间控制器所接分机列表
#define	RCJIC	0x713		//

#define 	HQFJC	0x720  	//获取主机控制器分机列表
#define	RHQFC	0x721		//

#define	GCIT	"835\0"		//获取IP编码对应表
#define 	RGCI	"836\0"		//应答

#define	SCIT	"837\0"		//设置IP编码对应表
#define 	RSCI	"838\0"		//应答

#define	STAS	"990\0"		//报告网络状态
#define 	RSTA	"991\0"		//应答

#ifdef CALL_3G
#define 	NTEL3	"804\0"   //NTEL
#define	RTEL3	"805\0"   //RTEL

#define	NEND3	"808\0"	//NEND
#define	REND3	"809\0"   //REND

#define 	NANS3	"810\0"   //NANS
#define	RANS3	"811\0"   //RANS

#define 	ULOCK3	"812\0"   //NANS
#define	RLOCK3	"813\0"   //RANS
#endif

#define   GVNUM	"814\0"     //主机→云对讲服务器 发送获取小区号请求
#define   RVNUM	"815\0"      //云对讲服务器→主机 应答小区号

#define   SEDUNLOCK	"816\0"       //主机→PC 发送开锁请求
#define   GETUNLOCK	"817\0"       //PC→主机 应答提机开锁

#ifdef DOOR_ACCESS_CARD_LIST

#define		NSMJK	"760\0"
#define		NRMJK	"761\0"
#define		NUMJK	"762\0"
#define		NAMJK	"763\0"

#define		NUMJK1	"764\0"
#define		NAMJK1	"765\0"

#define		NUMJK2	"766\0"
#define		NAMJK2	"767\0"       
#endif

#define     NTLFT	"918\0"
#define     RNTLFT	"919\0"

#define   SENDV          "888\0"
#define   GETDV          "889\0"

#define  JSON_UPLOAD      "918\0"
#define  RJSON_UPLOAD     "919\0"
#define  JSON_AUTH        "920\0"
#define  RJSON_AUTH       "921\0"
#define  JSON_DEL         "922\0"
#define  RJSON_DEL        "923\0"

#define	SPORC	0x826	//SPORC
#define	RPORC	0x827	//RPORC
#define	GREFC	0x828   //GREFC
#define	SREFC	0x829	//SREFC
#define	CREFC	0x830	//CREFC
#define	RCREC	0x831	//RCREC
#define	GLJBC	0x832
#define	RGLJC	0x833

#define	SIORC	0x510	//SIORC
#define	RIORC	0x511	//RIORC
#define	BEORC	0x512	//BEORC
#define	RBORC	0x513	//RBORC

#define	SKJLC	0x514
#define	JLYDC	0x515
#define	FJSJC	0x520
#define	SJYDC	0x521
#define	CJBDC	0x522
#define	RCBDC	0x523
#define	CJLBC	0x524
#define	LBYDC	0x525

#define	GFQSC	0x550	//GFQSC
#define	RFQSC	0x551	//RFQSC
#define	RGODC	0x552	//RGODC
#define 	RRGDC	0x553	//RRGDC
#define 	FALRC	0x554	//FALRC
#define	RFLRC	0x555	//RFLRC
#define	EALRC	0x556	//EALRC
#define	RELRC	0x557	//RELRC
#define	DALRC	0x558	//DALRC
#define	RDARC	0x559	//RDARC
#define	CALRC	0x560	//CALRC
#define	RCARC	0x561	//RCARC
#define 	GNCPC	0x562	//GNCPC
#define	RGNCC	0x563	//RGNCC
#define 	FJJSC	0x564
#define 	RFJJC	0x565
#define 	RTXLC	0x566
#define 	GTXLC	0x567

#define 	CHWLC	0x570	//CHWLC
#define 	RCHWC	0x571	//RCHWC
#define	GPCMC	0x572	//GPCMC
#define	RPCMC	0x573	//RPCMC
#define	SPCMC	0x574	//SPCMC
#define	RPCDC	0x575	//RPCDC

#define	SPTEC	0x576	//SPTEC
#define	RPTEC	0x577	//RPTEC

#define	CRDDC	0x578	//CRDD
#define	RCDDC	0x579	//RCDD

#define	RPFKC	0x580	//RPFK
#define	SPFKC	0x581	//SPFK

#define	RPJQC	0x582	//RPJQ
#define	SPJQC	0x583	//SPJQ

#define	RPLMC	0x584	//RPLM
#define	SPLMC	0x585	//SPLM

#define	RPDMC	0x586	//RPDM
#define	SPDMC	0x587	//SPDM

#define	RPKTC	0x588	//RPKT
#define	SPKTC	0x589	//SPKT

#define	RPCHC	0x590	//RPCH
#define	SPCHC	0x591	//SPCH

#define	WXBJC	0x592	//WXBJ
#define	BJYDC	0x593	//BJYD

#define	WXHZC	0x594	//WXHZ
#define	HZYDC	0x595	//HZYD

#define	WXBDC	0x598	//WXBD
#define	BDYDC	0x599	//BDYD

#define	SKPZC	0x600
#define	PZYDC	0x601

#define	LQFWC	0x602
#define	LQYDC	0x603

#define	FJTJC	0x604
#define	TJYDC	0x605

#define	FJYMC	0x650
#define	YMYDC	0x651

#define	RBFJC	0x660
#define	BFJDC	0x661

#define	GCJ2C	0x662
#define	RCJ2C	0x663

#define	ZCJ2C	0x664
#define	RZCJ2C	0x665

#define	LTXLC	0x670
#define	RLTXC	0x671

#define	TFTPC	0x672
#define	RFTPC	0x673

#define	JDPZC	0x674
#define	RJDPC	0x675

#define	GDATC	0x518	//GDAT
#define	RGDAC	0x519	//RGDA

#define	HTOGC	0x700   //HTOG
#define 	RTOGC	0x701   //RTOG

#define 	NTELC	0x704   //NTEL
#define	RTELC	0x705   //RTEL

#define	NENDC	0x708	//NEND
#define	RENDC	0x709   //REND

#define 	NANSC	0x710   //NANS
#define	RANSC	0x711   //RANS

#define	SCTLC	0x714	//SCTL
#define 	RCTLC	0x715	//RCTL


#define	GCLGC	0x724		//获取主机名录
#define RGCLC	0x725		//应答
#define	SCLGC	0x726       //设置主机名录
#define RSCLC	0x727       //应答

#define	GCRDC	0x740		//获取主机卡头数据
#define RGCRD	0x741		//应答
#define	SCRDC	0x742       //设置主机卡头数据
#define RSCRD	0x743       //应答

//#define		AMJKC	0x743

#define		SMJKCN	0x750
#define		RMJKCN	0x751
#define		UMJKCN	0x752
#define		AMJKCN	0x753

#define		QCPNC	0x756
#define		RCPNC	0x757
#define		SCPNC	0x758
#define		ACPNC	0x759

#define	WXBFC	0x596	//WXBF
#define	BFYDC	0x597	//BFYD

#define	WXSTC	0x526	//WXST
#define	STYDC	0x527	//STYD

#define	WXLYC	0x528	//WXLY
#define	LYYDC	0x529	//LYYD

#define	WXJDC	0x530	//WXJD
#define	JDYDC	0x531	//JDYD

#define	PZLYC	0x532
#define	RPZLC	0x533

#define	RTPCC	0x534	//RTPC
#define	RRPCC	0x535	//RRPC

#define 	GDSTC	0x536	//GDST
#define 	RDSTC	0x537	//RDST

#define	GCJTC	0x538
#define 	RCJTC	0x539

#define	CJLYPC	0x540
#define 	RLYYPC	0x541

#define 	ZALRC	0x542
#define 	RZARC	0x543

#define 	BFZTC	0x544
#define 	RBFZC	0x545

#define 	SQFWC	0x546
#define 	FWYDC	0x547

#define 	FCBJC	0x548	//FCBJ
#define 	FCYDC	0x549	//FCYD

#define 	GCITC	0x835	//FCBJ
#define 	SCITC	0x837	//FCYD

#define	STASC	0x990	//报告网络状态
#define 	RSTAC	0x991	//应答

#ifdef CALL_3G
#define 	NTELC3	0x804   //NTEL 
#define	RTELC3	0x805   //RTEL

#define	NENDC3	0x808	//NEND
#define	RENDC3	0x809   //REND

#define 	NANSC3	0x810   //NANS
#define	RANSC3	0x811   //RANS

#define 	ULOCKC3	0x812   //NANS
#define	RLOCKC3	0x813   //RANS
#endif

#define  GVNUMC	0x814
#define  RVNUMC	0x815

#define  SEDUNLOCKC	0x816
#define  GETUNLOCKC	0x817

//added by hgj 180302
#define	NTLFTC	0x918	//GDAT  by mkq 20171017 netlift
#define	RNTLFTC	0x919	//RGDA by mkq 20171017 netlift
//end
#ifdef DOOR_ACCESS_CARD_LIST      

#define  SNMJKC	0x760
#define  RNMJKC	0x761

#define  UNMJKC	0x762
#define  ANMJKC	0x763

#define  UNMJKC1	0x764
#define  ANMJKC1	0x765

#define  UNMJKC2	0x766
#define  ANMJKC2	0x767
#endif

#define  SENDVC       0x888
#define  GETDVC       0x889

#define  JSON_UPLOAD_C      0x918
#define  RJSON_UPLOAD_C     0x919
#define  JSON_AUTH_C        0x920
#define  RJSON_AUTH_C       0x921
#define  JSON_DEL_C         0x922
#define  RJSON_DEL_C         0x923

#endif /* CMD_DEF_H_ */
