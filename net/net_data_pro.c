/*
 * net_data_pro.c
 *
 *  Created on: 2012-6-25
 */
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include "net_com.h"
#include "ajb_bus.h"
#include "cmd_def.h"
#include "data_struct.h"
#include "net_data_pro.h"
#include "data_com_ui.h"
#include "udp_task.h"
#include "global_def.h"
#include "dev_pro.h"
#include "udp_task.h"
#include "public.h"
#include "tcp_task.h"
#include "ret_type.h"
#include "event_num.h"
#include "net_event.h"
#include "call_event.h"
#include "dev_config.h"
#include "formats.h"
#include "info_container.h"
#include "my_debug.h"

#include "database/sqlite_data.h"
#include "ui.h"

#define RLEAVE_PATH "./msg/wav/"
#define MSGPIC_PATH "./msg/pic/"

static int try_call=0;
static int hold_on=0;

struct net_cmd_struct  net_commands[] =  {

		{FJSJ,FJSJC},//520
		{SJYD,SJYDC},//521
		{GDAT,GDATC},
		{RGDA,RGDAC},
		{WXLY,WXLYC},//528
		{LYYD,LYYDC},//529
		{GDST,GDSTC},//536
		{RDST,RDSTC},//537
		{NTEL,NTELC},
		{RTEL,RTELC},
		{NEND,NENDC},
		{REND,RENDC},
		{NANS,NANSC},
		{RANS,RANSC},
		{RPFK,RPFKC},//580
#ifdef CALL_3G//add by wrm 20150305 for cloud
		{NTEL3,NTELC3},
		{RTEL3,RTELC3},
		{NEND3,NENDC3},
		{REND3,RENDC3},
		{NANS3,NANSC3},
		{RANS3,RANSC3},
		{ULOCK3,ULOCKC3},
		{RLOCK3,RLOCKC3},
#endif	

		{GVNUM,GVNUMC},			//获取小区号
		{RVNUM,RVNUMC},

		{SEDUNLOCK,SEDUNLOCKC}, //随机密码开锁
		{GETUNLOCK,GETUNLOCKC},

		{FJJS,FJJSC},//564               //向主机校时
		{RFJJ,RFJJC},//565
              {SENDV,SENDVC},              //向主机获取小区号
              {GETDV,GETDVC},
		{0, -1}
};

struct net_cmd_struct  net_commands_pc[] =  {
		{SIOR,SIORC},//510
		{GDAT,GDATC},//518
		{RGDA,RGDAC},//519
		{RZAR,RZARC},//543
		{RBOR,RBORC},//513
		{JLYD,JLYDC},//515
		{CJBD,CJBDC},//522
		{RCBD,RCBDC},//523
		{CJLB,CJLBC},//524
		{LBYD,LBYDC},//525
		{WXST,WXSTC},//526
		{STYD,STYDC},//527
		{WXLY,WXLYC},//528
		{LYYD,LYYDC},//529
		{WXJD,WXJDC},//530
		{JDYD,JDYDC},//531
		{PZLY,PZLYC},//532
		{RPZL,RPZLC},//533
		{RTPC,RTPCC},//534
		{RRPC,RRPCC},//535
		{GDST,GDSTC},//536
		{GCJT,GCJTC},//538
		{CJLYP,CJLYPC},//540
		{ZALR,ZALRC},//542
		{BFZT,BFZTC},//544
		{RBFZ,RBFZC},//545
		{SQFW,SQFWC},//546
		{FWYD,FWYDC},//547
		{FCBJ,FCBJC},//548
		{FCYD,FCYDC},//549
		{GFQS,GFQSC},//550
		{RFQS,RFQSC},//551
		{RGOD,RGODC},//552
		{RRGD,RRGDC},//553
		{RFLR,RFLRC},//555
		{EALR,EALRC},//556
		{RELR,RELRC},//557
		{DALR,DALRC},//558
		{RDAR,RDARC},//559
		{CALR,CALRC},//560
		{RCAR,RCARC},//561
		{GNCP,GNCPC},//562
		{RGNC,RGNCC},//563
		{FJJS,FJJSC},//564
		{RFJJ,RFJJC},//565
		{RTXL,RTXLC},//566
		{GTXL,GTXLC},//567
		{CHWL,CHWLC},//570
		{RCHW,RCHWC},//571
		{RPCM,RPCMC},//573
		{SPCM,SPCMC},//574
		{RPCD,RPCDC},//575
		{SPTE,SPTEC},//576
		{RPTE,RPTEC},//577
		{CRDD,CRDDC},//578
		{RCDD,RCDDC},//579
		{RPFK,RPFKC},//580
		{SPFK,SPFKC},//581
		{RPJQ,RPJQC},//582
		{SPJQ,SPJQC},//583
		{RPLM,RPLMC},//584
		{SPLM,SPLMC},//585
		{RPDM,RPDMC},//586
		{SPDM,SPDMC},//587
		{RPKT,RPKTC},//588
		{SPKT,SPKTC},//589
		{RPCH,RPCHC},//590
		{SPCH,SPCHC},//591
		{WXBJ,WXBJC},//592
		{BJYD,BJYDC},//593
		{WXHZ,WXHZC},//594
		{HZYD,HZYDC},//595
		{WXBF,WXBFC},//596
		{BFYD,BFYDC},//597
		{WXBD,WXBDC},//598
		{BDYD,BDYDC},//599
		{SKPZ,SKPZC},//600
		{PZYD,PZYDC},//601
		{LQFW,LQFWC},//602
		{LQYD,LQYDC},//603
		{FJTJ,FJTJC},//604
		{TJYD,TJYDC},//605
		{FJYM,FJYMC},//650
		{YMYD,YMYDC},//651
		{RBFJ,RBFJC},//660
		{BFJD,BFJDC},//661
		{GCJ2,GCJ2C},//662
		{RCJ2,RCJ2C},//663
		{ZCJ2,ZCJ2C},//664
		{RZCJ2,RZCJ2C},//665
		//666 669
		{LTXL,LTXLC},//670
		{RLTX,RLTXC},//671
		{TFTP,TFTPC},//672
		{RFTP,RFTPC},//673
		{CJIN,CJINC},//712
		{HQFJ,HQFJC},//720
             {GCLG,GCLGC},//724
             {SCLG,SCLGC},//726
		{GCRD,GCRDC},//740
             {SCRD,SCRDC},//742
        #ifdef DOOR_ACCESS_CARD_LIST
             {NSMJK,SNMJKC}, // 760
		{NRMJK,RNMJKC}, // 761
		{NUMJK,UNMJKC}, // 762
		{NAMJK,ANMJKC}, // 763

		{NUMJK1,UNMJKC1}, // 764
		{NAMJK1,ANMJKC1}, // 765
			
		{NUMJK2,UNMJKC2}, // 766
		{NAMJK2,ANMJKC2}, // 767
        #endif		

//===================================
		//{SMJK,SMJKC},	// 740
		//{RMJK,RMJKC},	// 741
		//{UMJK,UMJKC},	// 742
		//{AMJK,AMJKC},	// 743
		{SMJKN,SMJKCN},	// 750
		{RMJKN,RMJKCN},	// 751
		{UMJKN,UMJKCN},	// 752
		{AMJKN,AMJKCN},	// 753
//===================================		
		{QCPN, QCPNC}, // 756
		{RCPN, RCPNC}, // 757
		{SCPN, SCPNC}, // 758
		{ACPN, ACPNC}, // 759    
		
		{SPOR,SPORC},//826
		{RPOR,RPORC},//827
		{GREF,GREFC},//828
		{CREF,CREFC},//830
		{RCRE,RCREC},//831
		{GLJB,GLJBC},//832
		{RGLJ,RGLJC},//833
		//831------838
		{GCIT,GCITC},
		{SCIT,SCITC},
		{JDPZ,JDPZC},
		{RNTLFT,RNTLFTC},// 919 网络呼梯应答
		{0, -1}
};

struct net_cmd_struct  net_commands_pc_json[] =  {
    {JSON_UPLOAD,JSON_UPLOAD_C}, 
    {RJSON_UPLOAD,RJSON_UPLOAD_C},
    {JSON_AUTH,JSON_AUTH_C},   
    {RJSON_AUTH,RJSON_AUTH_C},  
    {JSON_DEL,JSON_DEL_C},    
    {RJSON_DEL,RJSON_DEL_C},   
    {0, -1}
};

#ifdef HAVE_TONGHAO
pthread_mutex_t  arp_lock=PTHREAD_MUTEX_INITIALIZER;
#endif
pthread_mutex_t  ntelc_cache_lock=PTHREAD_MUTEX_INITIALIZER;
volatile int netlc_arp;
static pthread_mutex_t lock_for_ip_search = PTHREAD_MUTEX_INITIALIZER;

void set_netlc_arp_value(int val)
	{
    pthread_mutex_lock(&ntelc_cache_lock);
    netlc_arp=val;
	pthread_mutex_unlock(&ntelc_cache_lock);
}


static int lock_pthread_mutex_lock(pthread_mutex_t*lock)
{
	int ret=0;
	ret=pthread_mutex_lock(lock);
	if (ret< 0)
	{
		fprintf(stderr,"lock_pthread_mutex_lock  failure !");
		return ret;
	}
	return ret;
}

static void unlock_pthread_mutex_lock(pthread_mutex_t *lock)
{
	if(pthread_mutex_unlock(lock)<0)
	{
		fprintf(stderr,"unlock_pthread_mutex_lock  failure !");
	}
}


static int lock_ntelc_cache_lock(void)
{
	return lock_pthread_mutex_lock(&arp_lock);
}

static void unlock_ntelc_cache_lock(void)
{
	unlock_pthread_mutex_lock(&arp_lock);
}

/**
 *
 * 设置系统时间，未调用hwclock保存
 * @param [in] ucTime 输入时间BCD码，从低到高依次为 秒 分 时 日 月 星期 年
 * @return 成功返回0，否则返回-1
*/

int set_system_time(unsigned char ucTime[7])
{
	return dev_set_time(2000+Bcd2Dec(ucTime[6]), Bcd2Dec(ucTime[4]), Bcd2Dec(ucTime[3]), \
                       Bcd2Dec(ucTime[2]), Bcd2Dec(ucTime[1]), Bcd2Dec(ucTime[0]));
}

/**
 *
 * 组建呼叫对讲数据TCP数据包
 * @param [in] data ntel_data结构呼叫对讲数据
 * @param [in] cmd 网络命令字符串
 * @param [out] buf 返回的数据包
 * @return 返回数据包的长度
*/

int build_call_data(unsigned char *buf,ntel_data* data,unsigned char cmd[3])
{
    int data_len=sizeof(ntel_data)+14;
    buf[0]=0x7;
    buf[1]=0xb8;
    memcpy((void *)(&buf[2]),&data_len,4);
    memcpy((void *)(&buf[6]),"req=",4);
    memcpy((void *)(&buf[10]),cmd,3);
    memcpy((void *)(&buf[13]),"&query=",7);
    memcpy((void *)(&buf[20]),data,sizeof(ntel_data));
    return (data_len+6);
}


/**
 *
 * 组建呼叫对讲数据TCP数据包,&query后为*用于TYPE_JDM365
 * @param [in] data ntel_data结构呼叫对讲数据
 * @param [in] cmd 网络命令字符串
 * @param [out] buf 返回的数据包
 * @return 返回数据包的长度
*/

int build_call_data_j365(unsigned char *buf,ntel_data* data,unsigned char cmd[3])
{
    int data_len=sizeof(ntel_data)+14;
    buf[0]=0x7;
    buf[1]=0xb8;
    memcpy((void *)(&buf[2]),&data_len,4);
    memcpy((void *)(&buf[6]),"req=",4);
    memcpy((void *)(&buf[10]),cmd,3);
    memcpy((void *)(&buf[13]),"&query=",7);
    buf[19]=RES_VAL;
    memcpy((void *)(&buf[20]),data,sizeof(ntel_data));
    return (data_len+6);
}

/*
static inline int build_net_cmd(unsigned char *buf,int len,unsigned char cmd[3])
{
    int data_len=len+14+1;
    buf[0]=0x7;
    buf[1]=0xb8;
    memcpy(&buf[2],(&data_len),4);
    memcpy(&buf[6],"req=",4);
    memcpy(&buf[10],cmd,3);
    memcpy(&buf[13],"&query=",7);
    return 20;
}

static inline int build_net_cmd_j365(unsigned char *buf,int len,unsigned char cmd[3])
{
    int data_len=len+14+1;
    buf[0]=0x7;
    buf[1]=0xb8;
    memcpy(&buf[2],(&data_len),4);
    memcpy((&buf[6]),"req=",4);
    memcpy(&buf[10],cmd,3);
    memcpy(&buf[13],"&query=",7);
    buf[19]=RES_VAL;
    return 20;
}
*/

/**
 *
 * 组建TCP数据包
 * @param [in] data 待发送数据
 * @param [in] cmd 网络命令字符串
 * @param [out] buf 返回的数据包
 * @return 返回数据包的长度
*/

int build_net_data(unsigned char *buf,void * data,int len,char *cmd)
{
    int data_len=len+14;
    buf[0]=0x7;
    buf[1]=0xb8;
    memcpy(&buf[2],(&data_len),4);
    memcpy(&buf[6],"req=",4);
    memcpy(&buf[10],cmd,3);
    memcpy(&buf[13],"&query=",7);
    memcpy(&buf[20],data,len);
    return (data_len+6);
}

/**
 *
 * 组建TCP数据包,&query后为*用于TYPE_JDM365
 * @param [in] data 待发送数据
 * @param [in] cmd 网络命令字符串
 * @param [out] buf 返回的数据包
 * @return 返回数据包的长度
*/

int build_net_data_j365(unsigned char *buf,void * data,int len,char *cmd)
{
    int data_len=len+14;
    buf[0]=0x7;
    buf[1]=0xb8;
    memcpy(&buf[2],(&data_len),4);
    memcpy((void *)(&buf[6]),"req=",4);
    memcpy(&buf[10],cmd,3);
    memcpy(&buf[13],"&query=",7);
    buf[19]=RES_VAL;
    memcpy(&buf[20],data,len);
    return (data_len+6);
}

/**
 *
 * 组建json版TCP数据包头
 * @param [in] len 待发送数据长度
 * @param [out] buf 返回的数据包
 * @return 包头数据的长度
*/


int build_json_net_package_head(char *buf,int len,char *cmd,unsigned int pack_no)
{
    int data_len=len+14;
    buf[0]=0x0a;
    buf[1]=0xb8;
    memcpy(&buf[2],(void *)(&data_len),4);
    memcpy(&buf[6],(char *)(&pack_no),4);
    memcpy(&buf[10],cmd,3);
    buf[13] = 1;
    buf[14] = buf[15] = 0;
    memcpy(&buf[16],"pld=",4);
    return (data_len+6);
}

/**
 *
 * 组建TCP数据包头
 * @param [in] len 待发送数据长度
 * @param [out] buf 返回的数据包
 * @return 包头数据的长度
*/


int build_net_package_head(char *buf,int len,char *cmd)
{
    int data_len=len+14;
    buf[0]=0x07;
    buf[1]=0xb8;
    memcpy(&buf[2],(void *)(&data_len),4);
    memcpy(&buf[6],"req=",4);
    memcpy(&buf[10],cmd,3);
    memcpy(&buf[13],"&query=",7);
    return (data_len+6);
}

/**
 *
 * 组建TCP数据包头,&query后为*用于TYPE_JDM365
 * @param [in] len 待发送数据长度
 * @param [out] buf 返回的数据包
 * @return 包头数据的长度
*/

int build_net_package_head_j365(char *buf,int len,char *cmd)
{
    int data_len=len+14;
    buf[0]=0x07;
    buf[1]=0xb8;
    memcpy(&buf[2],(void *)(&data_len),4);
    memcpy((void *)(&buf[6]),"req=",4);
    memcpy(&buf[10],cmd,3);
    memcpy(&buf[13],"&query=",7);
    buf[19]=RES_VAL;
    return (data_len+6);
}

/**
 *
 * 组建呼叫对讲数据返回TCP数据
 * @param [in] data rtel_data结构呼叫对讲数据
 * @param [in] cmd 网络命令字符串
 * @param [out] buf 返回的数据包
 * @return 包头数据的长度
*/

int build_rcall_data(unsigned char **buf,rtel_data* data,char cmd[3])
{
    unsigned char *tmp=*buf;
    int data_len=sizeof(rtel_data)+14;
    *tmp=0x07;
    *(tmp+1)=0xb8;
    memcpy((tmp+2),&data_len,4);
    memcpy((tmp+6),"req=",4);
    memcpy((tmp+10),cmd,3);
    memcpy((tmp+13),"&query=",7);
    memcpy((tmp+20),(unsigned char *)data,sizeof(rtel_data));
    return (data_len+6);
}

/**
 *
 * 组建呼叫对讲数据返回TCP数据,&query后为*用于TYPE_JDM365
 * @param [in] data rtel_data结构呼叫对讲数据
 * @param [in] cmd 网络命令字符串
 * @param [out] buf 返回的数据包
 * @return 包头数据的长度
*/

int build_rcall_data_j365(unsigned char **buf,rtel_data* data,char cmd[3])
{
    unsigned char *tmp=*buf;
    int data_len=sizeof(rtel_data)+14;
    *tmp=0x07;
    *(tmp+1)=0xb8;
    memcpy((tmp+2),&data_len,4);
    memcpy((tmp+6),"req=",4);
    memcpy((tmp+10),cmd,3);
    memcpy((tmp+13),"&query=",7);
    *(tmp+19)=RES_VAL;
    memcpy((tmp+20),(unsigned char *)data,sizeof(rtel_data));
    return (data_len+6);
}

/**
 *
 * 组建呼叫对讲数据返回TCP数据,&query后一位的值为type
 * @param [in] data rtel_data结构呼叫对讲数据
 * @param [in] cmd 网络命令字符串
 * @param [in] type &query后一位的值
 * @param [out] buf 返回的数据包
 * @return 包头数据的长度
*/

int build_rcall_data_bytype(unsigned char **buf,rtel_data* data,char cmd[3],unsigned char type)
{
    unsigned char *tmp=*buf;
    int data_len=sizeof(rtel_data)+14;
    *tmp=0x07;
    *(tmp+1)=0xb8;
    memcpy((tmp+2),&data_len,4);
    memcpy((tmp+6),"req=",4);
    memcpy((tmp+10),cmd,3);
    memcpy((tmp+13),"&query=",7);
    *(tmp+19)=type;
    memcpy((tmp+20),(unsigned char *)data,sizeof(rtel_data));
    return (data_len+6);
}

/**
 *
 * 创建接收链表
 * @param [out] head 返回链表头
 * @return 成功返回0，失败返回-1
*/

int creat_net_mem(net_mem **head)  //
{
    *head=(net_mem *)malloc(sizeof(net_mem ));
    if(*head==NULL)
    {
    	app_debug(DBG_FATAL,"creat net_mem failure!");
        return -1;
    }
    (*head)->next=NULL;
    return 0;
}

/**
 *
 * 创建接收链表
 * @param [in] current_node 在当前节点下添加指定数目的节点
 * @param [in] num 待添加节点的数目
 * @return 成功返回0，失败返回-1
*/

int add_net_mem(net_mem **current_node,int num)//为接收链表增加长度
{
    int i=0;
    net_mem *next_node;
    net_mem *prv_node=*current_node;
    for(i=0;i<num;i++)
    {
        next_node=(net_mem *)calloc(sizeof(net_mem ),1);
        if(next_node==NULL)
        {
            app_debug(DBG_FATAL,"malloc net_mem failure!");
            return -1;
        }

        next_node->data_count=0;
        prv_node->next=next_node;
        next_node->next=NULL;
        prv_node=next_node;
    }
    return i;
}

/**
 *
 * 从链表中读取数据
 * @param [in] node 链表头
 * @param [out] buf从链表中获取的数据
 * @return 成功返回0，失败返回-1
*/

int get_net_mem_data(net_mem *node,void *buf)
{
    net_mem *next;
    net_mem *prv_node=node;
    int i=0;
    while(prv_node!=NULL)
    {
    	// app_debug(DBG_INFO, "get_net_mem_data %d\n",prv_node->data_count);
        memcpy(buf+i,prv_node->net_mem,prv_node->data_count);
        i+=prv_node->data_count;
        next=prv_node->next;
        prv_node=next;
/*        if(prv_node==NULL)
        	 app_debug(DBG_INFO, "get_net_mem_data 03\n");*/
    }
    app_debug(DBG_INFO, "get_net_mem_data3\n");
    return i;
}


int send_get_village_data_to_server(unsigned int net_cmd,void *buf,int len)
{
	proc_msg msg_send;
	net_data_msg msg;
	memcpy(msg.buf,buf,len);
	msg.net_cmd=net_cmd;
	msg.flag_addr=SERVER_DATA;
	msg.data_len=len;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_data_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}

/**
 *
 *发送TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度,不能大于10
 * @return 成功返回0，失败返回-1
*/

int send_data_to_server(unsigned int net_cmd,void *buf,int len)
{
	proc_msg msg_send;
	net_data_msg msg;
	memcpy(msg.buf,buf,len);
	msg.net_cmd=net_cmd;
	msg.flag_addr=PC_DATA;
	msg.data_len=len;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_data_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}

/**
 *
 *回复TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度,不能大于10
 * @param [in] num 网络节点编号
 * @return 成功返回0，失败返回-1
*/

int respond_data_to_server(unsigned int net_cmd,void *buf,int len,int num)
{
	proc_msg msg_send;
	net_data_msg msg;
	memcpy(msg.buf,buf,len);
	msg.net_cmd=net_cmd;
	msg.flag_addr=PC_DATA_RET;
	msg.data_len=len;
	msg.num=num;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_data_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}

/**
 *
 *发送TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，失败返回-1
*/

int send_addr_to_server(unsigned int net_cmd,void *buf,int len)
{
	proc_msg msg_send;
	net_addr_msg msg;
	net_mem *mem_head ,*current_node;
	int ret,items=0,bytes=0,i=0;
	items=COUNT_ITEM(len);
	ret = request_list_head_mem(&mem_head);
	if (ret == 0) {
		current_node=mem_head;
		if(items>1){
			ret=add_net_mem(&mem_head,items-1);
			if(ret<0) return -1;

			for(i=0;i<items-1;i++){
				memcpy(current_node->net_mem,buf+MAX_LEN*i,MAX_LEN);
				bytes=+MAX_LEN;
				current_node->data_count=MAX_LEN;
				current_node=current_node->next;
			}
		}
		memcpy(current_node->net_mem,buf+MAX_LEN*i,len-MAX_LEN*i);
		current_node->data_count=len-MAX_LEN*i;
		//mem_head->data_count=len;
	} else
	{
		app_debug(DBG_INFO, "cmd:0x%x request_list_head_mem error\n",net_cmd);
		return -1 ;
	}

	msg.net_cmd=net_cmd;
	msg.flag_addr=PC_ADDR;
	msg.data_len=len;
	msg.list_num=mem_head->mem_num;
	msg.num=DEV_NO_NULL;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_addr_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}

/**
 *
 *发送json版TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，失败返回-1
*/

int send_json_addr_to_server(unsigned int net_cmd,void *buf,int len)
{
	proc_msg msg_send;
	net_addr_msg msg;
	net_mem *mem_head ,*current_node;
	int ret,items=0,bytes=0,i=0;
	items=COUNT_ITEM(len);
	app_debug(DBG_INFO,"amount of net_mem = %d\n",items);
	ret = request_list_head_mem(&mem_head);
	if (ret == 0) {
		current_node=mem_head;
		if(items>1){
			ret=add_net_mem(&mem_head,items-1);
			if(ret<0) return -1;

			for(i=0;i<items-1;i++){
				memcpy(current_node->net_mem,buf+MAX_LEN*i,MAX_LEN);
				bytes=+MAX_LEN;
				current_node->data_count=MAX_LEN;
				current_node=current_node->next;
			}
		}
		memcpy(current_node->net_mem,buf+MAX_LEN*i,len-MAX_LEN*i);
		current_node->data_count=len-MAX_LEN*i;
		//mem_head->data_count=len;
	} else
	{
		app_debug(DBG_INFO, "cmd:0x%x request_list_head_mem error\n",net_cmd);
		return -1 ;
	}

	msg.net_cmd=net_cmd;
	msg.flag_addr=PC_ADDR_JSON;
	msg.data_len=len;
	msg.list_num=mem_head->mem_num;
	msg.num=DEV_NO_NULL;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_addr_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	app_debug(DBG_INFO,"send_json_addr_to_pc cmd:0x%x\n",net_cmd);
	return 0;
}

/**
 *
 *发送TCP数据到18020端口，IP通过devdest查询
 * @param [in] net_cmd 网络命令字
 * @param [in] devdest 四位目标房号
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，失败返回-1
*/

int send_addr_to_server_bydev_NO(unsigned int net_cmd,unsigned char devdest[4],void *buf,int len)
{
	proc_msg msg_send;
	net_addr_msg msg;
	net_mem *mem_head ,*current_node;
	int ret,items=0,bytes=0,i=0;
	items=COUNT_ITEM(len);
	ret = request_list_head_mem(&mem_head);
	if (ret == 0) {
		current_node=mem_head;
		if(items>1){
			ret=add_net_mem(&mem_head,items-1);
			if(ret<0) return -1;

			for(i=0;i<items-1;i++){
				memcpy(current_node->net_mem,buf+MAX_LEN*i,MAX_LEN);
				bytes=+MAX_LEN;
				current_node->data_count=MAX_LEN;
				current_node=current_node->next;
			}
		}
		memcpy(current_node->net_mem,buf+MAX_LEN*i,len-MAX_LEN*i);
		current_node->data_count=len-MAX_LEN*i;
		//mem_head->data_count=len;
	} else
	{
		app_debug(DBG_INFO, "cmd:0x%x request_list_head_mem error\n",net_cmd);
		return -1 ;
	}

	msg.net_cmd=net_cmd;
	msg.flag_addr=PC_ADDR;
	msg.data_len=len;
	msg.list_num=mem_head->mem_num;
	memcpy(&msg.num,devdest,4);
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_addr_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}

/**
 *
 *发送TCP数据到18022端口，IP通过dest_NO查询
 * @param [in] net_cmd 网络命令字
 * @param [in] dest_NO 四位目标房号
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，失败返回-1
*/

int send_addr_to_tml(unsigned int net_cmd,void *dest_NO,void *buf,int len)
{
	proc_msg msg_send;
	net_addr_msg msg;
	net_mem *mem_head ,*current_node;
	int ret,items=0,bytes=0,i=0;
	items=COUNT_ITEM(len);
	ret = request_list_head_mem(&mem_head);
	if (ret == 0) {
		current_node=mem_head;
		if(items>1){
			ret=add_net_mem(&mem_head,items-1);
			if(ret<0) return -1;

			for(i=0;i<items-1;i++){
				memcpy(current_node->net_mem,buf+MAX_LEN*i,MAX_LEN);
				bytes=+MAX_LEN;
				current_node->data_count=MAX_LEN;
				current_node=current_node->next;
			}
		}
		memcpy(current_node->net_mem,buf+MAX_LEN*i,len-MAX_LEN*i);
		current_node->data_count=len-MAX_LEN*i;
		//mem_head->data_count=len;
	} else
	{
		app_debug(DBG_INFO, "cmd:0x%x request_list_head_mem error\n",net_cmd);
		return -1 ;
	}

	msg.net_cmd=net_cmd;
	msg.flag_addr=MEM_ADDR;
	msg.data_len=len;
	msg.list_num=mem_head->mem_num;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg.num,dest_NO,4);
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_addr_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}

/**
 *
 *发送TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，失败返回-1
*/

int send_to_server(unsigned int net_cmd,void *buf,int len)
{

	if(len<=10)
		return send_data_to_server(net_cmd,buf,len);
	else
		return send_addr_to_server(net_cmd,buf,len);
}

/**
 *
 *发送UDP数据
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，失败返回-1
*/
int send_data_to_udp(void *buf,int len)
{
	proc_msg msg_send;
	net_addr_msg msg;
	net_mem *mem_head ,*current_node;
	int ret,items=0,bytes=0,i=0;
	items=COUNT_ITEM(len);
	ret = request_list_head_mem(&mem_head);
	if (ret == 0) {
		current_node=mem_head;
		if(items>1){
			ret=add_net_mem(&mem_head,items-1);
			if(ret<0) return -1;
			for(i=0;i<items-1;i++){
				memcpy(current_node->net_mem,buf+MAX_LEN*i,MAX_LEN);
				bytes=+MAX_LEN;
				current_node->data_count=MAX_LEN;
				current_node=current_node->next;
			}
		}
		memcpy(current_node->net_mem,buf+MAX_LEN*i,len-MAX_LEN*i);
		current_node->data_count=len-MAX_LEN*i;
		//mem_head->data_count=len;
	} else
	{
		app_debug(DBG_INFO, " request_list_head_mem error\n");
		return -1 ;
	}

	msg.net_cmd=0;
	msg.flag_addr=GLJ_RQ_RET;
	msg.data_len=len;
	msg.list_num=mem_head->mem_num;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
#ifdef CFG_SUPPORT_INFO_STORE
	if(send_ui_data_to_udp(&msg_send, MSG_LEN_NET)<0){
#else
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
#endif
		app_debug(DBG_INFO, "send_addr_to_pc cmd error\n");
		return -1;
	}
	return 0;
}

/**
 *
 *回复TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度,不能大于10
 * @param [in] num 网络节点编号
 * @return 成功返回0，失败返回-1
 */
int respond_addr_to_server(unsigned int net_cmd,void *buf,int len,int num)
{	
    proc_msg msg_send;
    net_addr_msg msg;	
    net_mem *mem_head ,*current_node;	
    int ret,items=0,bytes=0,i=0;	
    items=COUNT_ITEM(len);	
    ret = request_list_head_mem(&mem_head);	
    if (ret == 0)	
        {		
            current_node=mem_head;		
            if(items>1){			
                    ret=add_net_mem(&mem_head,items-1);			
                    if(ret<0)	return -1;					
                    for(i=0;i<items-1;i++){				
                            memcpy(current_node->net_mem,buf+MAX_LEN*i,MAX_LEN);				
                            bytes=+MAX_LEN;				
                            current_node->data_count=MAX_LEN;				
                            current_node=current_node->next;			
                            }		
                    }		
            memcpy(current_node->net_mem,buf+MAX_LEN*i,len-MAX_LEN*i);		
            current_node->data_count=len-MAX_LEN*i;		//mem_head->data_count=len;	
            } else	
        {		
            app_debug(DBG_INFO, "cmd:0x%x request_list_head_mem error\n",net_cmd);		
            return -1;	
            }	
    msg.net_cmd=net_cmd;	
    msg.flag_addr=PC_ADDR_RET;	
    msg.data_len=len;	
    msg.list_num=mem_head->mem_num;	
    msg.num=num;	
    msg_send.msg_type=MSG_FROM_PROC;	
    memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));	
    if(send_data_to_net(&msg_send, MSG_LEN_NET)<0)	{		
        app_debug(DBG_INFO, "respond_addr_to_server cmd:0x%x error\n",net_cmd);		
        return -1;	
        }	
    return 0;
    }

/**
 *
 *回复TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度,不能大于10
 * @param [in] num 网络节点编号
 * @return 成功返回0，失败返回-1
*/

int respond_to_server(unsigned int net_cmd,void *buf,int len,int num)
{

		if(len<=10)
		return respond_data_to_server(net_cmd,buf,len,num);
		else
		return respond_addr_to_server(net_cmd,buf,len,num);
}


/**
 *
 * 从链表的指定位置获取指定长度的数据
 * @param [in] node 链表头
 * @param [in] start 开始获取数据的位置
 * @param [in] len 期望获取数据的长度
 * @param [out] buf 获取到的数据
 * @return 成功返回0，失败返回-1
*/

static int get_net_mem_data_bypos(net_mem *node,void *buf,int start,int len)//从链表中读取数据,start：起始位置从0开始
{
	net_mem *next;
	net_mem *prv_node=node;
	int item_count=0;
	int total_len=0,pos=start,buf_pos=0,last_len=len;
	int item_pos=start% prv_node->data_count;
	int item_num=start/ prv_node->data_count;
	while(prv_node!=NULL)
	{
		if(item_count<item_num){
			pos+=prv_node->data_count;
			next=prv_node->next;
			prv_node=next;
			item_count++;
		}else
		{
			if(prv_node->data_count-item_pos<last_len){
				memcpy(buf+buf_pos,prv_node->net_mem+item_pos,prv_node->data_count-item_pos);
				buf_pos+=prv_node->data_count-item_pos;
				total_len+=prv_node->data_count-item_pos;
				last_len-=prv_node->data_count-item_pos;
				item_pos=0;
			}
			else
			{
				memcpy(buf+buf_pos,prv_node->net_mem+item_pos,last_len);
				buf_pos+=last_len;
				total_len+=last_len;
				last_len-=last_len;
				item_pos=0;
			}
			if(last_len<=0)
				break;
			else
			{
				next=prv_node->next;
				prv_node=next;
				item_count++;
				continue;
			}
		}
	}
	app_debug(DBG_INFO, "get_net_mem_data_bypos :%d\n",total_len);
	if(total_len==0) return -1;
	return total_len;
}




#if 0

/**
 *
 * 保存接收到的升级程序数据
 * @param [in] node 链表头
 * @param [in] file_name 数据保存的文件名
 * @return 成功返回0，失败为OTHER_ERR
*/

//收到的升级包先保存成临时文件，校验无误后再重命名为待升级文件
int save_program(net_mem *node,char *update_name)
{
#define UPDATE_TYPE_APP   0x00424A41   //AJB
#define UPDATE_TYPE_RES   0x52424A41   //AJBR
#define UPDATE_TMP_FILE   "updata_tmp"

    printf("save_program  START \n");
    unsigned int  program_len = 5*1024*1024;
    unsigned int  proflag, funtype, version, proauth, chkdata;
    char *recv_buf = alloca(program_len+100);
    char file_name[60] = {0};
    int  recv_bytes = get_net_mem_data_bypos(node,recv_buf,0,program_len+100);
    printf("recv_bytes:%d\n", recv_bytes);
    if(recv_bytes < 48){
        app_debug(DBG_ERROR, "get data error\n");
        return OTHER_ERR;
    }
    unsigned int  i = 0, checkcalc = 0;
    unsigned int  update_len = recv_bytes-28-20;

    char *head_buf   = &recv_buf[28];
    char *update_buf = &recv_buf[48];

    memcpy(&proflag, head_buf ,   4);
    memcpy(&funtype, head_buf+4,  4);
    memcpy(&version, head_buf+8,  4);
    memcpy(&proauth, head_buf+12, 4);
    memcpy(&chkdata, head_buf+16, 4);

    if (proflag == UPDATE_TYPE_APP) {
        strcpy (file_name, UPDATE_APP);
    } else if (proflag == UPDATE_TYPE_RES) {
        strcpy (file_name, UPDATE_RES);
    } else return PRAG_TYPE_ERR;
    //printf("proflag:%08X\n", proflag);
    /* 
    else 
    if (funtype == 0) {         //功能类型
        return VERSION_ERR;
    }
    */
    if ((version & 0x00FFFFFF) != 0x00423131) //11Bn
        return VERSION_ERR;

    for (i = 0; i < update_len; i++)
        checkcalc += (unsigned char)*(update_buf+i);

    if (chkdata != checkcalc)
        return OTHER_ERR;

    if (proflag == UPDATE_TYPE_APP) {
        if((update_buf[1]!='E')||(update_buf[2]!='L')||(update_buf[3]!='F')){
            return OTHER_ERR;
        }
    }

    int exec_fd=open(UPDATE_TMP_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if (exec_fd<0)return OTHER_ERR;

    //printf("update_len:%d\n", update_len);
    int write_bytes=write(exec_fd,update_buf,update_len);
    if (write_bytes!=update_len){
        close(exec_fd);
        return OTHER_ERR;
    }

    fsync(exec_fd);
    close(exec_fd);

    char *read_buf = alloca(update_len+100);
    exec_fd = open(UPDATE_TMP_FILE, O_RDONLY, 0664);
    if(exec_fd < 0)  return OTHER_ERR;

    int read_bytes=read(exec_fd, read_buf, update_len);
    if ((read_bytes != update_len) || (0 != memcmp(read_buf, update_buf, update_len))) {
        close(exec_fd);
        remove(file_name);
        return OTHER_ERR;
    }

    close(exec_fd);
    rename(UPDATE_TMP_FILE, file_name);
    system("sync");
    return 0;
}
#endif
//#else

#if 0

//收到的升级包先保存成临时文件，校验无误后再重命名为待升级文件
int save_program(net_mem *node,char *update_name)
{
#define UPDATE_TYPE_APP   0x00424A41   //AJB
#define UPDATE_TYPE_RES   0x52424A41   //AJBR
#define UPDATE_TMP_FILE   "updata_tmp"

    printf("save_program  START \n");
    unsigned int  program_len = 5*1024*1024;
    unsigned int  proflag, funtype, version, proauth, chkdata;
    char *recv_buf = alloca(program_len+100);
    char file_name[60] = {0};
    int  recv_bytes = get_net_mem_data_bypos(node,recv_buf,0,program_len+100);

    printf("recv_bytes:%d\n", recv_bytes);
    if(recv_bytes < 48){
        app_debug(DBG_ERROR, "get data error\n");
        return OTHER_ERR;
    }

    unsigned int  i = 0, checkcalc = 0;
    unsigned int  update_len = recv_bytes-28-20;

    char *head_buf   = &recv_buf[28];
    char *update_buf = &recv_buf[48];

    memcpy(&proflag, head_buf ,   4);
    memcpy(&funtype, head_buf+4,  4);
    memcpy(&version, head_buf+8,  4);
    memcpy(&proauth, head_buf+12, 4);
    memcpy(&chkdata, head_buf+16, 4);

    if (proflag == UPDATE_TYPE_APP) {
        strcpy (file_name, UPDATE_APP);
    } else if (proflag == UPDATE_TYPE_RES) {
        strcpy (file_name, UPDATE_RES);
    } else return PRAG_TYPE_ERR;

    if (memcmp(&version, DEV_VERSION, 4))
        return VERSION_ERR;

    if ((funtype & 0x000000FF) != 'P') //must be ipnc kernel
        return VERSION_ERR;

    for (i = 0; i < update_len; i++)
        checkcalc += (unsigned char)*(update_buf+i);

    if (chkdata != checkcalc)
        return OTHER_ERR;

    if (proflag == UPDATE_TYPE_APP) {
        if((update_buf[1]!='E')||(update_buf[2]!='L')||(update_buf[3]!='F')){
            return OTHER_ERR;
        }
    }

    int exec_fd=open(UPDATE_TMP_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if (exec_fd<0)return OTHER_ERR;

    //printf("update_len:%d\n", update_len);
    int write_bytes=write(exec_fd,update_buf,update_len);
    if (write_bytes!=update_len){
        close(exec_fd);
        return OTHER_ERR;
    }

    fsync(exec_fd);
    close(exec_fd);

    char *read_buf = alloca(update_len+100);
    exec_fd = open(UPDATE_TMP_FILE, O_RDONLY, 0664);
    if(exec_fd < 0)  return OTHER_ERR;

    int read_bytes=read(exec_fd, read_buf, update_len);
    if ((read_bytes != update_len) || (0 != memcmp(read_buf, update_buf, update_len))) {
        close(exec_fd);
        remove(file_name);
        return OTHER_ERR;
    }
    close(exec_fd);
    rename(UPDATE_TMP_FILE, file_name);
    system("sync");
    return SUCCESS;
}
#endif

/**
 *
 * 保存接收到的升级程序数据
 * @param [in] node 链表头
 * @param [in] file_name 数据保存的文件名
 * @return 成功返回0，失败为OTHER_ERR
*/

//收到的升级包先保存成临时文件，校验无误后再重命名为待升级文件

int save_program(net_mem *node,char *update_name)
{
#define UPDATE_TYPE_APP   0x00424A41   //AJB
#define UPDATE_TYPE_RES   0x52424A41   //AJBR
#define UPDATE_TMP_FILE   "updata_tmp"

    printf("save_program  START \n");
    unsigned int  program_len = 6*1024*1024;
    unsigned int  proflag, funtype, version, proauth, chkdata;
    char *recv_buf = malloc(program_len+100);
    char file_name[60] = {0};
    int  recv_bytes = get_net_mem_data_bypos(node,recv_buf,0,program_len+100);

    printf("recv_bytes:%d\n", recv_bytes);
    if(recv_bytes < 48){
        app_debug(DBG_ERROR, "get data error\n");
		free(recv_buf);
        return OTHER_ERR;
    }

    unsigned int  i = 0, checkcalc = 0;
    unsigned int  update_len = recv_bytes-28-20;

    char *head_buf   = &recv_buf[28];
    char *update_buf = &recv_buf[48];

    memcpy(&proflag, head_buf ,   4);
    memcpy(&funtype, head_buf+4,  4);
    memcpy(&version, head_buf+8,  4);
    memcpy(&proauth, head_buf+12, 4);
    memcpy(&chkdata, head_buf+16, 4);

    if (proflag == UPDATE_TYPE_APP) {
        strcpy (file_name, UPDATE_APP);
    } else if (proflag == UPDATE_TYPE_RES) {
        strcpy (file_name, UPDATE_RES);
    } else {
    	free(recv_buf);
    	return PRAG_TYPE_ERR;
    	}
    if (memcmp(&version, DEV_VERSION, 4))
    	{
    	printf("VERSION_ERR-------\n");
		free(recv_buf);

        return VERSION_ERR;
    	}

    if ((funtype & 0x000000FF) != 'P') //must be ipnc kernel
        	{
    	printf("VERSION_ERR\n");
		free(recv_buf);
        return VERSION_ERR;
    	}
       

    for (i = 0; i < update_len; i++)
        checkcalc += (unsigned char)*(update_buf+i);

    if (chkdata != checkcalc)
    	{
    	printf("check error:%d\n", checkcalc);
		free(recv_buf);
        return OTHER_ERR;
    	}

    if (proflag == UPDATE_TYPE_APP) {
        if((update_buf[1]!='E')||(update_buf[2]!='L')||(update_buf[3]!='F')){
			printf("elf error:%d\n", 0);
			free(recv_buf);
            return OTHER_ERR;
        }
    }

    int exec_fd=open(UPDATE_TMP_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if (exec_fd<0)
	{
	printf("open error:%d\n", exec_fd);
	free(recv_buf);
	return OTHER_ERR;
    	}

    //printf("update_len:%d\n", update_len);
    int write_bytes=write(exec_fd,update_buf,update_len);
    if (write_bytes!=update_len){
        close(exec_fd);
		printf("write error:%d\n", write_bytes);
		free(recv_buf);
        return OTHER_ERR;
    }

    fsync(exec_fd);
    close(exec_fd);

    char *read_buf = malloc(update_len+100);
    exec_fd = open(UPDATE_TMP_FILE, O_RDONLY, 0664);
    if(exec_fd < 0)
	{
		printf("open error:%d\n", exec_fd);
		free(recv_buf);
		free(read_buf);

	return OTHER_ERR;
    	}

    int read_bytes=read(exec_fd, read_buf, update_len);
    if ((read_bytes != update_len) || (0 != memcmp(read_buf, update_buf, update_len))) {
        close(exec_fd);
        remove(file_name);
		printf("read error:%d\n", read_bytes);
		free(recv_buf);
		free(read_buf);

        return OTHER_ERR;
    }
    close(exec_fd);
    rename(UPDATE_TMP_FILE, file_name);
    system("sync");
	free(recv_buf);
	free(read_buf);
   // return SUCCESS;
   return 0;
}



/**
 *
 * 保存接收到文本信息
 * @param [in] fpath 数据保存的文件名
 * @param [in] buf 待保存数据
 * @param [in] len 待保存数据长度
 * @return 成功返回保存的数据长度，失败为-1
*/

int write_msg_to_txt_file(char *fpath,void *buf,ssize_t len)
{
	int fd=0,bytes=0;

	fd=open(fpath,O_WRONLY|O_CREAT,0664);
	if(fd<0){
		printf("create %s failure\n",fpath);
		return -1;
	}else
	{
    bytes= write(fd,buf,len);
    close(fd);
    return bytes;
	}
}

/**
 *
 * 写wav文件的头数据，只针对8000Hz Alaw mono数据
 * @param [in] fd wav文件的文件描述符
 * @param [in] total_bytes wav数据的总长度
 * @param [in] len 待保存数据长度
 * @return 成功返回0，失败为-1
*/

int wrte_leave_msg_to_wave_head(int fd,int total_bytes)
{
	app_debug(DBG_INFO,"total_bytes=%d\n",total_bytes);
	unsigned int tmp;
	unsigned short tmp2;
	WaveHeader h;
	WaveFmtBody f;
	WaveChunkHeader cf, cd;

	h.magic = WAV_RIFF;
	tmp = total_bytes + sizeof(WaveHeader) + sizeof(WaveChunkHeader) + sizeof(WaveFmtBody) + sizeof(WaveChunkHeader) - 8;
	h.length = LE_INT(tmp);
	h.type = WAV_WAVE;

	cf.type = WAV_FMT;
	cf.length = LE_INT(16);

	f.format = LE_SHORT(WAV_FMT_ALAW);
	f.channels = LE_SHORT(1);
	f.sample_fq = LE_INT(8000);

	tmp2 = 1 * 8 / 8;
	f.byte_p_spl = LE_SHORT(tmp2);
	tmp = (unsigned int) tmp2 * 8000;

	f.byte_p_sec = LE_INT(tmp);
	f.bit_p_spl = LE_SHORT(8);

	cd.type = WAV_DATA;
	cd.length = LE_INT(total_bytes);

	if (write(fd, &h, sizeof(WaveHeader)) != sizeof(WaveHeader))
		goto write_err;
	if(write(fd, &cf, sizeof(WaveChunkHeader)) != sizeof(WaveChunkHeader))
		goto write_err;
	if(write(fd, &f, sizeof(WaveFmtBody)) != sizeof(WaveFmtBody))
		goto write_err;
	if(write(fd, &cd, sizeof(WaveChunkHeader)) != sizeof(WaveChunkHeader))
		goto write_err;
	return 0;
write_err:
	perror("write to sound'head wrong!");
	return -1;
}

/**
 *
 * 写wav文件数据，只针对8000Hz Alaw mono数据
 * @param [in] fpath 数据保存的文件名
 * @param [in] head wav数据链表头
 * @param [in] total_bytes wav数据数据长度
 * @return 成功返回0，失败为-1
*/
int wrte_leave_msg_to_wave_file(char *fpath,net_mem*head,int total_bytes)
{
	int fd=-1;
	if(total_bytes<=0){
		printf("err:total_bytes=%d\n",total_bytes);
		goto write_err;
	}

	fd=open(fpath,O_RDWR|O_CREAT,0664);
	if(fd<0)
	{
		printf("create %s failure\n",fpath);
		goto write_err;
	}
	else{
		app_debug(DBG_INFO,"total_bytes=%d\n",total_bytes);
		if(wrte_leave_msg_to_wave_head(fd,total_bytes)<0)
			goto write_err;

		net_mem *next;
		net_mem *prv_node=head;
		int i=0;
		int bytes=0;
		while(prv_node!=NULL)
		{
			if(i==0)
			{
				int pos=28;
				while(pos>0)
				{
					if(prv_node->data_count<28)
					{
						pos-=prv_node->data_count;
						prv_node=prv_node->next;
						if(prv_node==NULL){
							printf("err:prv_node is NULL!\n");
							goto write_err;
						}
					}
					else
					{
						if(pos-prv_node->data_count<=0){
							break;
						}
					}
				}
				if((prv_node->data_count-pos)>0)
					bytes=write(fd,&prv_node->net_mem[pos],prv_node->data_count-pos);

			}
			else
			{
					bytes=write(fd,prv_node->net_mem,prv_node->data_count);
			}

			if(bytes>0){
				i+=bytes;
			}
			bytes=0;

			if(prv_node!=NULL)
			{
				next=prv_node->next;
				prv_node=next;
			}
		}

		if(fd>0){
			close(fd);
		}
	}
	return 0;

write_err:
	perror("write wave data wrong!");
	if(fd>0){
		close(fd);
	}
	return -1;
}

/**
 *
 * 查询链表的长度
 * @param [in] node 链表头
 * @return 返回数据长度
*/

int get_net_mem_size(net_mem *node)
{
    net_mem *next;
    net_mem *prv_node=node;
    int i=0;
    while(prv_node!=NULL)
    {
        i+=prv_node->data_count;
        next=prv_node->next;
        prv_node=next;
    }
    return i;
}

/**
 *
 * 释放链表内存空间
 * @param [in] node 链表头
*/

void del_all_net_mem(net_mem **head)
{
    net_mem *next;
    while(*head!=NULL)
    {
        next=(*head)->next;
        free(*head);
        (*head)=next;
    }
    //app_debug(DBG_INFO,"delete all net_mem!!!!!!!!!!!\n");
}


int net_cmd_compare(char *string1,char *string2)//字符串转数字
{
    if (memcmp(string1, string2, 3) == 0)
        return 1;
    else
        return 0;
}


int respond_to_socket(int socket,const void *buf,int len)
{

    int ret=-1;
    int send_len,i;
    int send_item=COUNT_ITEM(len);

    for(i=0;i<send_item-1;i++){
        send_len=send_data(socket,buf+MAX_LEN*i,MAX_LEN);
        if(send_len<0)
            ret+=send_len;
    }
    send_len=send_data(socket,buf+MAX_LEN*i,len-MAX_LEN*i);
    ret+=send_len;
    return ret;
}

void close_socket(struct connect_node_t* runtime)
{
    if(runtime->socketfd >=(-1)){
    	 app_debug(DBG_INFO,"close socket!\n");
    	 close(runtime->socketfd);
    	 runtime->socketfd=-1;
    }
}

struct rtelc_cache_t
{
	volatile int ntelc_count;
	volatile int rtelc_count;
	unsigned char ntelc_NO[4];
};


/*pthread_mutex_t  rtelc_cache_lock=PTHREAD_MUTEX_INITIALIZER;
static struct rtelc_cache_t rtelc_cache;

int add_dev_to_rtelc_cache(unsigned int ipaddr,short allow)
{
	int ret=0;
	if(allow<0)
	{
		set_dest_dev_status(rtelc_cache.ntelc_NO,ipaddr,TONGHAO_BUSY);
	}

	if (pthread_mutex_lock(&rtelc_cache_lock) < 0)
	{
		fprintf(stderr,"lock rtelc_cache_lock failure !");
		return ret;
	}
	rtelc_cache.rtelc_count++;
	ret=rtelc_cache.rtelc_count;
	pthread_mutex_unlock(&rtelc_cache_lock);
	return ret;
}*/

/**
 *
 * TCP 18022端口数据接收与处理函数
 * @param [in] net_runtime 网络节点
*/
#define TCP_HEAD_LENGTH_18020 (20)
#define TCP_HEAD_LENGTH_18022 (20)

void tcp_process_task(connect_node *net_runtime)
{
    app_debug(DBG_INFO,"tcp_process_task starting !\n");
	
    static unsigned short number=0;
    int recv_bytes=0;
    int current_len=0;
    int use_addr=0;
    int total_len=0;
    int tmp_len =0;    
    unsigned char *recv_buf;
    unsigned char *recv_ptr;
    char command[3];

    net_mem head,*current_node,*current_head,*new_head=NULL;

    memset(&head,0,sizeof(net_mem));
    head.next=NULL;
    current_head=current_node=&head;
    proc_msg net2proc_data;
    proc_msg_ret net2proc_ret_data={};
    net2proc_data.msg.num=net_runtime->event_num;
    net2proc_ret_data.msg.num=net_runtime->event_num;

    ntel_data *call_data=NULL;
    rtel_data rcall_data;

    net2proc_data.msg_type=MSG_FROM_NET;
    net2proc_ret_data.msg_type=MSG_FROM_NET;
    struct timeval time_out;
    time_out.tv_sec=0;
    time_out.tv_usec=100000;
    app_debug(DBG_INFO,"net_runtime->event_num  %d\n",net_runtime->event_num);
    int ret=setsockopt(net_runtime->socketfd,SOL_SOCKET,SO_RCVTIMEO,&time_out,sizeof(time_out));
    if(ret<0)
    {
    	app_debug(DBG_FATAL,"Set SO_RCVTIMEO failure!");
    	goto error;
    }

    int count=0;
    int max_recv_length=MAX_LEN;
    recv_buf=current_node->net_mem;
    while(net_runtime->connect_on)
    {
    	recv_bytes=recv_data(net_runtime->socketfd,recv_buf,max_recv_length);
    	if(recv_bytes<0)
        {
        	if(errno==EWOULDBLOCK)
        	{
        		if (count > 30) {
        			net_runtime->connect_on = NET_STOP;
        			app_debug(DBG_INFO,"socket timeout exit !\n");
        		}
        		count++;
        		continue;
        	}
        	else
        	{
        		app_debug(DBG_INFO,"socket timeout 11 !\n");
        		net_runtime->connect_on = NET_STOP;
        		continue;
        	}
        }
        else if(recv_bytes==0)
        {
        	app_debug(DBG_INFO,"recv_bytes=0 exit !\n");
        	count = 0;
        	net_runtime->connect_on = NET_STOP;
        	continue;
        }
        else
        {
        	count = 0;
        	current_node->data_count+=recv_bytes;
        	if(current_head->data_count<TCP_HEAD_LENGTH_18022)//只判断链表的头结点
        	{
        		recv_buf=current_head->net_mem+current_node->data_count;
        		max_recv_length=MAX_LEN-current_node->data_count;
        		continue;
        	}

        	if ((total_len==0)&&(current_head->net_mem[0] == 0x07) && (current_head->net_mem[1] == 0xB8) )
            {
                recv_ptr=current_head->net_mem;                
                current_len=0;
                memcpy((char*)&total_len,recv_ptr+2,4);  // 数据长度
                total_len+=6;
        		current_len += current_node->data_count;
                //app_debug(DBG_INFO,"total_len=%d current_len=%d\n",total_len,current_len);
                printf("total_len=%d current_len=%d\n",total_len,current_len);
                if( total_len!= current_len )
                {
                	int ret;
                	//数据存储，用于外发地址
                	app_debug(DBG_INFO,"COUNT_ITEM(total_len)=%d\n",COUNT_ITEM(total_len));
                	ret=request_list_head_mem(&new_head);
                	if(ret==0){
                		current_node->mem_num=new_head->mem_num;
                		memcpy(new_head,current_node,sizeof(net_mem));
                		current_head=current_node=new_head;
                		app_debug(DBG_INFO,"current_head->mem_num=%d\n",current_head->mem_num);
                		ret=add_net_mem(&current_node,COUNT_ITEM(total_len)+10);
                		if(ret<0) {
                			goto error;
                		}
                		current_node=current_node->next;
                		memset(current_node->net_mem,0x00,1024);
        				recv_buf=current_node->net_mem;
        				max_recv_length=MAX_LEN;                        
                	}else
                		goto error;
                }
            }
            else
            {
            	if(current_node->next==NULL){
            		ret=add_net_mem(&current_node,2);
            		if(ret<0) {
            			goto error;
            		}
            	}
            	current_node=current_node->next;
            	current_len+= recv_bytes;
            	memset(current_node->net_mem,0x00,1024);
        		recv_buf=current_node->net_mem;
        		max_recv_length=MAX_LEN;                
            	app_debug(DBG_INFO,"current_len=%d\n",current_len);
            }

            if(total_len== current_len )
            {
        		net_mem *prv_node=current_head->next;
        		if(total_len<=MAX_LEN)
        		{
        			while(prv_node!=NULL)
        			{
        				memcpy(current_head->net_mem+current_head->data_count,prv_node->net_mem,prv_node->data_count);
        				current_head->data_count+=prv_node->data_count;
        				prv_node->data_count=0;
        				prv_node=prv_node->next;
        			}
        		}
        		else
        		{
        			while((prv_node!=NULL)&&(current_head->data_count<MAX_LEN))
        			{
        				int copy_length=((MAX_LEN-current_head->data_count)>=prv_node->data_count)?prv_node->data_count:(MAX_LEN-current_head->data_count);
        				memcpy(current_head->net_mem+current_head->data_count,prv_node->net_mem,copy_length);
        				current_head->data_count+=copy_length;
        				prv_node->data_count-=copy_length;
        				if(prv_node->data_count<=0)
        				{
        					prv_node=prv_node->next;
        				}
        				else
        				{
        					memmove(prv_node->net_mem,prv_node->net_mem+copy_length,prv_node->data_count);
        					break;
        				}
        			}
        		}
                unsigned char *file_add=current_head->net_mem;
                unsigned char res_val=*(file_add+19);
                file_add+= 10;
                memcpy(command,file_add,3);			//命令字
                file_add+= 10;
                int i=0;
                while ((net_commands[i].string)&&(net_cmd_compare(command,net_commands[i].string) == 0))
                    i++;
		  printf("net_commands:%02x \n",net_commands[i].id);
            switch(net_commands[i].id)
            {
            case GDATC:
            {
            	net2proc_data.msg.flag_addr=MEM_DATA;
            	net2proc_data.msg.data.net_cmd=GDATC;
            	memcpy(&net2proc_data.msg.data.cmd, file_add, total_len-20);
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
             	if(netlc_arp>0)
            	{
            		if(net2proc_data.msg.data.cmd==SSSK)
            		{
            			set_netlc_arp_value(0);
            		}
            	}

            }
                break;	
            case RGDAC:
                app_debug(DBG_INFO,"519\n");
                memcpy(&rcall_data,file_add,sizeof(rtel_data));
                ajb_msg_ret_build(&net2proc_ret_data.msg,RGDAC,SCODE,rcall_data.allow,\
                                   rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
                net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
                net_runtime->connect_on = NET_STOP;
                break;

	     case SENDVC:
            {
		app_debug(DBG_INFO,"888\n");
            	net2proc_data.msg.flag_addr=MEM_DATA;
            	net2proc_data.msg.data.net_cmd=SENDVC;
            	memcpy(&net2proc_data.msg.data.cmd, file_add, total_len-20);
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
             	if(netlc_arp>0)
            	{
            		if(net2proc_data.msg.data.cmd==SSSK)
            		{
            			set_netlc_arp_value(0);
            		}
            	}
            }
            break;
            case WXLYC://528
            {
            	app_debug(DBG_INFO,"留言 528\n");
            	int cmd_len=14+4;
            	unsigned char cmdbuf[30];
            	cmdbuf[0]=0x7;
            	cmdbuf[1]=0xb8;
            	memcpy(&cmdbuf[2],&cmd_len,4);
            	memcpy(&cmdbuf[6],"req=",4);
            	memcpy(&cmdbuf[10],"529",3);
            	memcpy(&cmdbuf[13],"&query=",7);
            	memcpy(&cmdbuf[20],my_devlist.DevNo,4);

            	char fpath[100] = {'\0'};
            	time_t now;
            	struct tm *tnow;
            	number++;
            	time(&now);
            	tnow = localtime(&now);
            	sprintf(fpath, "./rleave/%d.%02d.%02d.%02d.%02d.%02d_%.4x.wav", 1900+tnow->tm_year, \
            			tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min,tnow->tm_sec,number);

            	wrte_leave_msg_to_wave_file(fpath,current_head,total_len-28);

            	net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,cmd_len+6);

            	net2proc_data.msg_type=MSG_FROM_NET;
            	net_data_msg net2proc_msg;
            	net2proc_msg.flag_addr=MEM_DATA;
            	net2proc_msg.net_cmd=WXLYC;
            	memcpy(net2proc_msg.buf,file_add,8);
            	net2proc_msg.data_len=8;
            	net2proc_msg.num=net_runtime->event_num;
            	memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
            	net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
            }
            break;
            case LYYDC:
            {
            	app_debug(DBG_INFO,"留言 529\n");
            	net2proc_data.msg_type=MSG_FROM_NET;
            	net_data_msg net2proc_msg;
            	net2proc_msg.flag_addr=MEM_DATA;
            	net2proc_msg.net_cmd=LYYDC;
            	memcpy(net2proc_msg.buf,file_add,4);
            	net2proc_msg.data_len=4;
            	net2proc_msg.num=net_runtime->event_num;
            	memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
            	net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
            }
            break;
            case GDSTC:// 536
            {
            	app_debug(DBG_INFO,"536\n");
            	net_data_msg net2proc_msg;
            	memcpy(net2proc_msg.buf,file_add,4);
            	net2proc_msg.net_cmd=GDSTC;
            	net2proc_msg.flag_addr=MEM_DATA;
            	net2proc_msg.data_len=4;
            	net2proc_msg.num=net_runtime->event_num;
            	memcpy(&net2proc_data.msg,&net2proc_msg,sizeof(net_data_msg));
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            }
            break;
            case RDSTC:// 537
            {
            	app_debug(DBG_INFO,"537\n");
            	net_data_msg net2proc_msg;
            	memcpy(net2proc_msg.buf,file_add,5);
            	net2proc_msg.net_cmd=RDSTC;
            	net2proc_msg.flag_addr=MEM_DATA;
            	net2proc_msg.data_len=5;
            	net2proc_msg.num=net_runtime->event_num;
            	memcpy(&net2proc_data.msg,&net2proc_msg,sizeof(net_data_msg));
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            }
            break;
            case RPFKC:
            {
            	app_debug(DBG_INFO,"580\n");
            	net_data_msg net2proc_msg;
            	memcpy(net2proc_msg.buf,file_add,6);
            	net2proc_msg.net_cmd=RPFKC;
            	net2proc_msg.flag_addr=MEM_DATA;
            	net2proc_msg.data_len=6;
            	net2proc_msg.num=net_runtime->event_num;
            	memcpy(&net2proc_data.msg,&net2proc_msg,sizeof(net_data_msg));
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);

            	app_debug(DBG_INFO,"580\n");
            	unsigned char cmdbuf[40];
            	int cmd_len=0;
/*            	memcpy(msg.buf,file_add,6);
            	msg.net_cmd=RPFKC;
            	msg.flag_addr=PC_DATA;
            	msg.data_len=6;*/
            	int pic_num=0;
            	if(total_len>=26)
            	{
            		if(is_call_me(file_add))
            		{

            			unsigned short int num=0;
            			memcpy(&num,file_add+4,2);
            			app_debug(DBG_INFO,"num=%d\n",num);
            			char name_buf[64][256];
            			DIR *dp;
            			struct dirent *ep;
            			dp = opendir (MSGPIC_PATH);
            			if (dp != NULL)
            			{
            				while ((ep=readdir(dp))!=NULL)
            				{
            					strcpy(name_buf[pic_num],ep->d_name);
            					if((strcasecmp(".jpg",strrchr(ep->d_name,'.'))==0)||(strcasecmp(".jpeg",strrchr (ep->d_name,'.'))==0))
            					{
            						app_debug(DBG_INFO,"pos@:%d  filename:%s\n",pic_num,ep->d_name);
            						pic_num++;
            					}
            				}
            				closedir (dp);
            			}
            			else
            				perror ("Couldn’t open the directory");
            			app_debug(DBG_INFO,"pic_num=%d\n",pic_num);
            			if(num<=pic_num)
            			{

            				char *jpg_buf=alloca(300*1024);
            				int pos=pic_num-num;
            				app_debug(DBG_INFO,"pos=%d\n",pos);
            				int pic_fd=-1;
            				char pic_path[256];
            				strcpy(pic_path,MSGPIC_PATH);
            				strcat(pic_path,name_buf[pos]);
            				pic_fd=open(pic_path,O_RDONLY);
            				if(pic_fd>=0){
            					int pic_len=read(pic_fd,jpg_buf,300*1024);
            					cmd_len=14+4+pic_len;
            					cmdbuf[0]=0x07;
            					cmdbuf[1]=0xb8;
            					memcpy(&cmdbuf[2],&cmd_len,4);
            					memcpy(&cmdbuf[6],"req=",4);
            					memcpy(&cmdbuf[10],"581",3);
            					memcpy(&cmdbuf[13],"&query=",7);
            					memcpy(&cmdbuf[20],file_add,4);
            					net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20+4);
            					net_runtime->respond_fun(net_runtime->socketfd,jpg_buf,pic_len);
            					app_debug(DBG_INFO,"pos@:%d \n",pos);
            					break;
            				}
            			}

            			cmd_len=14+4;
            			cmdbuf[0]=0x07;
            			cmdbuf[1]=0xb8;
            			memcpy(&cmdbuf[2],&cmd_len,4);
            			memcpy(&cmdbuf[6],"req=",4);
            			memcpy(&cmdbuf[10],"581",3);
            			memcpy(&cmdbuf[13],"&query=",7);
            			memcpy(&cmdbuf[20],file_add,4);
            			net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20+4);
            		}
            	}
            }
            break;
            case RPZLC:
            {
            	app_debug(DBG_INFO,"533\n");
            	net_addr_msg net2proc_addr;
            	net2proc_addr.flag_addr=MEM_ADDR;
            	net2proc_addr.net_cmd=RPZLC;
            	net2proc_addr.data_len=total_len-20;
            	net2proc_addr.num=net_runtime->event_num;
            	use_addr=1;
            	if (new_head == NULL)
            	{
            		int ret;
            		ret = request_list_head_mem(&new_head);
            		if (ret == 0)
            		{
            			if((total_len- 20)<MAX_LEN)
            			{
            				memcpy(new_head->net_mem,file_add,total_len- 20);
            				new_head->data_count=total_len;
            			}
            		}
            		else
            		{
            			goto error;
            		}
            	}
            	else
            	{
            		memmove(new_head->net_mem,file_add,new_head->data_count-20);
            	}

            	net2proc_addr.list_num = new_head->mem_num;
            	memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
            	new_head->data_count = new_head->data_count - 20;
            	net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
            	net_runtime->connect_on = NET_STOP;
            }
            break;
            case SPFKC:
            {
            	app_debug(DBG_INFO,"581\n");
            	net_addr_msg net2proc_addr;
            	net2proc_addr.flag_addr=MEM_ADDR;
            	net2proc_addr.net_cmd=SPFKC;
            	net2proc_addr.data_len=total_len-20;
            	net2proc_addr.num=net_runtime->event_num;
            	use_addr=1;
            	if (new_head == NULL)
            	{
            		int ret;
            		ret = request_list_head_mem(&new_head);
            		if (ret == 0)
            		{
            			if((total_len- 20)<MAX_LEN)
            			{
            				memcpy(new_head->net_mem,file_add,total_len- 20);
            				new_head->data_count=total_len;
            			}
            		}
            		else
            		{
            			goto error;
            		}
            	}
            	else
            	{
            		memmove(new_head->net_mem,file_add,new_head->data_count-20);
            	}

            	net2proc_addr.list_num = new_head->mem_num;
            	memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
            	new_head->data_count = new_head->data_count - 20;
            	net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
            	net_runtime->connect_on = NET_STOP;
            }
            break;

#ifdef CALL_3G //add by wrm 20150305 for cloud

            case NTELC3:
            {
            	app_debug(DBG_INFO,"804\n");
            	call_data=(ntel_data *)file_add;
            	ajb_msg_build_whole(&net2proc_data.msg,NTELC3,STEL,call_data->dest_addr,call_data->src_addr,call_data->oper_type,call_data->resstaus,MEM_DATA);
            	net2proc_ret_data.msg.data.dev_type=TYPE_D3G;
            	add_call_item(call_data->src_addr,net_runtime->dest_ip);
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            	netlc_arp=0;
            }
            break;
            case RTELC3:
            {
            	app_debug(DBG_INFO,"805\n");
            	memcpy(&rcall_data,file_add,sizeof(rtel_data));
            	ajb_msg_ret_build(&net2proc_ret_data.msg,RTELC3,SCODE,rcall_data.allow,\
            			rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
            	net2proc_ret_data.msg.data.dev_type=TYPE_D3G;
              net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
            	net_runtime->connect_on = NET_STOP;
            }
        break;
        case NENDC3:
        {
            try_call=0;
            app_debug(DBG_INFO,"808------------\n");
            call_data=(ntel_data *)file_add;
            ajb_msg_build_whole(&net2proc_data.msg,NENDC3,SEND,call_data->dest_addr,call_data->src_addr,call_data->oper_type,call_data->resstaus,MEM_DATA);
            net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
        }
        break;
        case RENDC3:
            try_call=0;
            app_debug(DBG_INFO,"809\n");
            memcpy(&rcall_data,file_add,sizeof(rtel_data));
            ajb_msg_ret_build(&net2proc_ret_data.msg,RENDC3,SCODE,rcall_data.allow,\
                    rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
            net2proc_ret_data.msg.data.dev_type=TYPE_D3G;
            net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
            net_runtime->connect_on = NET_STOP;
            break;
            case NANSC3:
            {
                app_debug(DBG_INFO,"810 \n");
                call_data=(ntel_data *)file_add;
#ifdef HAVE_TONGHAO
                set_cur_talk_dev_ip(call_data->src_addr,net_runtime->dest_ip);
#endif
                set_netlc_arp_value(0);
                // ajb_msg_build(&net2proc_data.msg,NANSC,SANS,call_data->dest_addr,call_data->src_addr,MEM_DATA);
                ajb_msg_build_whole(&net2proc_data.msg,NANSC3,SANS,call_data->dest_addr,call_data->src_addr,call_data->oper_type,call_data->resstaus,MEM_DATA);
                net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            }
            break;
            case RANSC3:
            {
                app_debug(DBG_INFO,"811\n");
                memcpy(&rcall_data,file_add,sizeof(rtel_data));
                ajb_msg_ret_build(&net2proc_ret_data.msg,RANSC3,SCODE,rcall_data.allow,\
                        rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
                net2proc_ret_data.msg.data.dev_type=TYPE_D3G;
                net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
                net_runtime->connect_on = NET_STOP;
            }
            break;
            case ULOCKC3:
            {
                app_debug(DBG_INFO,"812\n");
                net2proc_data.msg.flag_addr=MEM_DATA;
                net2proc_data.msg.data.net_cmd=ULOCKC3;
                net2proc_data.msg.data.cmd=0;
                memcpy(net2proc_data.msg.data.dest_No, file_add, total_len-20);
                net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
                if(netlc_arp>0)
                {
                    if(net2proc_data.msg.data.cmd==SSSK)
                    {
                        netlc_arp=0;
                    }
                }
            }
            break;
            case RLOCKC3:
            {
                app_debug(DBG_INFO,"813\n");
                memcpy(&rcall_data,file_add,sizeof(rtel_data));
                ajb_msg_ret_build(&net2proc_ret_data.msg,RLOCKC3,SCODE,*file_add,\
                        rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
                net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
                net_runtime->connect_on = NET_STOP;
            }
            break;
#endif

           case FJJSC:
            {
                app_debug(DBG_INFO,"564\n");
                net2proc_data.msg.flag_addr=MEM_DATA;
                net2proc_data.msg.data.net_cmd=FJJSC;
                net2proc_data.msg.data.cmd=0;
                memcpy(net2proc_data.msg.data.dest_No, file_add, total_len-20);
                net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
                if(netlc_arp>0)
                {
                    if(net2proc_data.msg.data.cmd==SSSK)
                    {
                        netlc_arp=0;
                    }
                }
            }
            break;
#if 1
            case NTELC:
            {
            	app_debug(DBG_INFO,"ruan_704\n");
            ///	set_com_dev_ip(net_runtime->dest_ip);
            	call_data=(ntel_data *)file_add;
            	ajb_msg_build_whole(&net2proc_data.msg,NTELC,STEL,call_data->dest_addr,call_data->src_addr,call_data->oper_type,call_data->resstaus,MEM_DATA);

				printf("res_val= %c\n",res_val);
            	switch(res_val)
            	{
            	case RES_VAL:
            		net2proc_data.msg.data.check=TYPE_JDM365;
            		break;
            	case RES_2100:
            		net2proc_data.msg.data.check=TYPE_D2100;
            		break;
                case RES_926:
            		net2proc_data.msg.data.check=TYPE_J926;
					printf("RES_926!!!!\n");
					break;		            
            	default:
            		net2proc_data.msg.data.check=TYPE_OTHER;
            		break;
            	}
            	add_call_item(call_data->src_addr,net_runtime->dest_ip);
                net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            }
            break;
#else    
            case NTELC:
            {

            	printf("ruan_704\n");
            	app_debug(DBG_INFO,"704\n");
            ///	set_com_dev_ip(net_runtime->dest_ip);
            	call_data=(ntel_data *)file_add;
            	ajb_msg_build_whole(&net2proc_data.msg,NTELC,STEL,call_data->dest_addr,call_data->src_addr,call_data->oper_type,call_data->resstaus,MEM_DATA);

            	switch(res_val)
            	{
            	case RES_VAL:
            		net2proc_data.msg.data.check=TYPE_JDM365;
            		break;
            	case RES_2100:
            		net2proc_data.msg.data.check=TYPE_D2100;
            		break;
				case RES_926:
            		net2proc_data.msg.data.check=TYPE_J926;
				break;		
            	default:
            		net2proc_data.msg.data.check=TYPE_OTHER;
            		break;
            	}
            	add_call_item(call_data->src_addr,net_runtime->dest_ip);
                net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
				set_netlc_arp_value(0);
            }
                break;
 #endif
            case RTELC:
                app_debug(DBG_INFO,"705\n");
                memcpy(&rcall_data,file_add,sizeof(rtel_data));
                ajb_msg_ret_build(&net2proc_ret_data.msg,RTELC,SCODE,rcall_data.allow,\
                                   rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
            	switch(res_val)
            	{
            	case RES_VAL:
            		net2proc_ret_data.msg.data.dev_type=TYPE_JDM365;
            		break;
            	case RES_2100:
            		net2proc_ret_data.msg.data.dev_type=TYPE_D2100;
            		break;
		case RES_926:
            		net2proc_ret_data.msg.data.dev_type=TYPE_J926;
			break;
            	default:
            		net2proc_ret_data.msg.data.dev_type=TYPE_OTHER;
            		break;
            	}
                	//add_dest_dev_type_by_ip(net_runtime->dest_ip,TYPE_OTHER);
            	//if(add_dev_to_rtelc_cache(net_runtime->dest_ip,rcall_data.allow)>=rtelc_cache.ntelc_count)
            	{
                    printf(" 705_send_to_proc\n");
            		net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
                    printf("705_already_send_to_proc\n");
            	}
                net_runtime->connect_on = NET_STOP;
                break;
            case NENDC:
            {
                app_debug(DBG_INFO,"708-----\n");
                hold_on=0;
                call_data=(ntel_data *)file_add;
              //  set_com_dev_ip(net_runtime->dest_ip);
                //set_cur_talk_dev_ip(call_data->dest_addr,net_runtime->dest_ip);
                //ajb_msg_build(&net2proc_data.msg,NENDC,SEND,call_data->dest_addr,call_data->src_addr,MEM_DATA);
               // set_dest_dev_status(call_data->src_addr,net_runtime->dest_ip,TONGHAO_DOWN);
                ajb_msg_build_whole(&net2proc_data.msg,NENDC,SEND,call_data->dest_addr,call_data->src_addr,call_data->oper_type,call_data->resstaus,MEM_DATA);
                net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            }
                break;
            case RENDC:
                app_debug(DBG_INFO,"709\n");
                hold_on=0;
                memcpy(&rcall_data,file_add,sizeof(rtel_data));
                ajb_msg_ret_build(&net2proc_ret_data.msg,RENDC,SCODE,rcall_data.allow,\
                                   rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
            	switch(res_val)
            	{
            	case RES_VAL:
            		net2proc_ret_data.msg.data.dev_type=TYPE_JDM365;
            		break;
            	case RES_2100:
            		net2proc_ret_data.msg.data.dev_type=TYPE_D2100;
            		break;
            	default:
            		net2proc_ret_data.msg.data.dev_type=TYPE_OTHER;
            		break;
            	}
            	/*add_dest_dev_type_by_ip(net_runtime->dest_ip,TYPE_JDM365);
                else
                	add_dest_dev_type_by_ip(net_runtime->dest_ip,TYPE_OTHER);*/
                net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
                net_runtime->connect_on = NET_STOP;
                break;
            case NANSC:
            {
              if(try_call==1)break;
            	app_debug(DBG_INFO,"710______ \n");
             hold_on=1;
            	call_data=(ntel_data *)file_add;
#ifdef HAVE_TONGHAO
                set_netlc_arp_value(0);
            	set_cur_talk_dev_ip(call_data->src_addr,net_runtime->dest_ip);
#endif
            	// ajb_msg_build(&net2proc_data.msg,NANSC,SANS,call_data->dest_addr,call_data->src_addr,MEM_DATA);
				
            	ajb_msg_build_whole(&net2proc_data.msg,NANSC,SANS,call_data->dest_addr,call_data->src_addr,call_data->oper_type,call_data->resstaus,MEM_DATA);
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            }
                break;
            case RANSC:
                app_debug(DBG_INFO,"711\n");

                //struct timeval now;
              //  gettimeofday (&now,NULL);
                //printf("sec:%ld usec:%ld \n",now.tv_sec,now.tv_usec);

                memcpy(&rcall_data,file_add,sizeof(rtel_data));
                ajb_msg_ret_build(&net2proc_ret_data.msg,RANSC,SCODE,rcall_data.allow,\
                                   rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
            	switch(res_val)
            	{
            	case RES_VAL:
            		net2proc_ret_data.msg.data.dev_type=TYPE_JDM365;
            		break;
            	case RES_2100:
            		net2proc_ret_data.msg.data.dev_type=TYPE_D2100;
            		break;
            	default:
            		net2proc_ret_data.msg.data.dev_type=TYPE_OTHER;
            		break;
            	}
/*                	add_dest_dev_type_by_ip(net_runtime->dest_ip,TYPE_JDM365);
                else
                	add_dest_dev_type_by_ip(net_runtime->dest_ip,TYPE_OTHER);*/
                net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
                net_runtime->connect_on = NET_STOP;
                break;
            case FJSJC:
            {
            	app_debug(DBG_INFO,"520 cmd:0x%2.2x dest_No: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x src_No: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x \n",\
            			*(file_add+0),*(file_add+1),*(file_add+2),*(file_add+3),*(file_add+4),*(file_add+5),\
            			*(file_add+6),*(file_add+7),*(file_add+8));
            	net2proc_data.msg.data.net_cmd = FJSJC;
            	memcpy(&net2proc_data.msg.data.cmd,file_add,total_len-20);
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
            }
            break;
            case SJYDC:
            {
            	app_debug(DBG_INFO,"521\n");
            	memcpy(&rcall_data,file_add,sizeof(rtel_data));
            	ajb_msg_ret_build(&net2proc_ret_data.msg,RGDAC,SCODE,rcall_data.allow,\
            			rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
            	net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
            	net_runtime->connect_on = NET_STOP;
            }
            break;

	case GVNUMC:
		{
		app_debug(DBG_INFO,"814\n");
		
		}
		break;
	case RVNUMC:
		{
              app_debug(DBG_INFO,"815\n");
            	app_debug(DBG_INFO,"815 data:0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x \n",\
            			*(file_add+0),*(file_add+1),*(file_add+2),*(file_add+3),*(file_add+4),*(file_add+5),\
            			*(file_add+6),*(file_add+7),*(file_add+8));
            	net2proc_data.msg.data.net_cmd = RVNUMC;
            	memcpy(&net2proc_data.msg.data.cmd,file_add,total_len-20);
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
		net_runtime->connect_on = NET_STOP;
            }
		break; 
	case GETUNLOCKC:
		{
              app_debug(DBG_INFO,"817\n");
            	app_debug(DBG_INFO,"817 data:0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x \n",\
            			*(file_add+0),*(file_add+1),*(file_add+2),*(file_add+3),*(file_add+4),*(file_add+5),\
            			*(file_add+6),*(file_add+7),*(file_add+8));
            	net2proc_data.msg.data.net_cmd = GETUNLOCKC;
            	memcpy(&net2proc_data.msg.data.cmd,file_add,total_len-20);
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
		net_runtime->connect_on = NET_STOP;
            }
		break;

    		case RNTLFTC:
    		{
                net_runtime->connect_on = NET_STOP;
    		}
    		break;
    		
            default:
            	net_runtime->connect_on = NET_STOP;
            break;
            }
            current_node=&head;
            if((new_head!=NULL)&&(use_addr==0)){
            		delete_list_mem(new_head);
            		new_head=NULL;
            }
            total_len=0;
            memset(&head,0,sizeof(net_mem));
        }
    }
}

error:
if((new_head!=NULL)&&(use_addr==0))
		delete_list_mem(new_head);
	close(net_runtime->socketfd);
	unregister_connect_node(net_runtime);
	app_debug(DBG_INFO,"TCP data process pthread exited !\n");
return;
}

/**
 *
 * TCP 18020端口数据接收与处理函数
 * @param [in] net_runtime 网络节点
*/

void tcp_pc_process_task(connect_node *net_runtime)
{
    app_debug(DBG_INFO,"tcp_pc_process_task starting !\n");
    static unsigned short number=0;
    unsigned int again_flag=0;
    unsigned int exit_flag=0;
    int recv_bytes=0;
    int current_len=0;
    int total_len=0;
    int use_addr=0;
    int tmp_len =0;
    unsigned char *recv_buf;
    unsigned char *recv_ptr;
    char command[3];

    net_mem head,*current_node,*current_head,*new_head=NULL;
    memset(&head,0,sizeof(net_mem));

    head.next=NULL;
    current_head=current_node=&head;
    proc_msg net2proc_data;
    proc_msg_ret net2proc_ret_data;
    net_data_msg msg;
    msg.num=net_runtime->event_num;
    net2proc_data.msg_type=MSG_FROM_NET;
    net2proc_ret_data.msg_type=MSG_FROM_NET;
    struct timeval time_out;
    time_out.tv_sec=0;
    time_out.tv_usec=100000;
    app_debug(DBG_INFO,"[%s] net_runtime->event_num  %d\n",__func__,net_runtime->event_num);
    int ret=setsockopt(net_runtime->socketfd,SOL_SOCKET,SO_RCVTIMEO,&time_out,sizeof(time_out));
    if(ret<0)
    {
    	app_debug(DBG_FATAL,"Set SO_RCVTIMEO failure!");
    	goto error;
    }
    int max_recv_length=MAX_LEN;
    recv_buf=current_node->net_mem;
    while(net_runtime->connect_on)
    {

    	//struct sockaddr src_addr;
    	//socklen_t length=sizeof(struct sockaddr_in);
    	recv_bytes=recv_data(net_runtime->socketfd,recv_buf,max_recv_length);
    	//recv_bytes=recvfrom(net_runtime->socketfd,recv_buf,MAX_LEN,0,&src_addr,&length);
    	if(recv_bytes==0)
    	{
    		net_runtime->connect_on = NET_STOP;
    		app_debug(DBG_INFO,"recv_bytes=0 exit !\n");
    		again_flag=0;
    		if(exit_flag==0x11){
    			dev_app_restart();
    		}
    		continue;
    	}
    	else if(recv_bytes<0)
    	{
    		//int num=errno;
    		if(errno!=EWOULDBLOCK)
    		{
    			//	app_debug(DBG_INFO,"recv_bytes=0 exit !\n");
    			net_runtime->connect_on = NET_STOP;
    			again_flag=0;
    			if(exit_flag==0x11){
    				dev_app_restart();
    			}
    		}
    		continue;
    	}
    	else
    	{
    		current_node->data_count+=recv_bytes;
    		if(current_head->data_count<TCP_HEAD_LENGTH_18020)
    		{
    			recv_buf=current_head->net_mem+current_node->data_count;
    			max_recv_length=MAX_LEN-current_node->data_count;
    			continue;
    		}

    		if ((total_len==0)&&(current_head->net_mem[0] == 0x07) && (current_head->net_mem[1] == 0xB8) )
    		{
    			recv_ptr=current_head->net_mem;
    			current_len=0;
    			memcpy((char*)&total_len,recv_ptr+2,4);  // 数据长度
    			total_len+=6;
    			current_len += current_node->data_count;
    			//app_debug(DBG_INFO,"total_len=%d current_len=%d\n",total_len,current_len);
    			printf("total_len=%d current_len=%d\n",total_len,current_len);
    			if( total_len!= current_len )
    			{
    				int ret;
    				//数据存储，用于外发地址
    				app_debug(DBG_INFO,"COUNT_ITEM(total_len)=%d\n",COUNT_ITEM(total_len));
    				ret=request_list_head_mem(&new_head);
    				if(ret==0){
    					current_node->mem_num=new_head->mem_num;
    					memcpy(new_head,current_node,sizeof(net_mem));
    					current_head=current_node=new_head;
    					app_debug(DBG_INFO,"current_head->mem_num=%d\n",current_head->mem_num);
    					ret=add_net_mem(&current_node,COUNT_ITEM(total_len));
    					if(ret<0) goto error;
    					current_node=current_node->next;
    					recv_buf=current_node->net_mem;
    					max_recv_length=MAX_LEN;
    				}else
    					goto error;
    			}
    		}
    		else
    		{
    			if(current_node->next==NULL){
    				ret=add_net_mem(&current_node,2);
    				if(ret<0) {
    					goto error;
    				}
    			}
    			current_node=current_node->next;
    			current_len+= recv_bytes;
    			recv_buf=current_node->net_mem;
    			max_recv_length=MAX_LEN;
    			//	app_debug(DBG_INFO,"current_len=%d\n",current_len);
    		}

    		if(total_len== current_len )
    		{
    			net_mem *prv_node=current_head->next;
    			if(total_len<=MAX_LEN)
    			{
    				while(prv_node!=NULL)
    				{
    					memcpy(current_head->net_mem+current_head->data_count,prv_node->net_mem,prv_node->data_count);
    					current_head->data_count+=prv_node->data_count;
    					prv_node->data_count=0;
    					prv_node=prv_node->next;
    				}
    			}
    			else
    			{
    				while((prv_node!=NULL)&&(current_head->data_count<MAX_LEN))
    				{
    					int copy_length=((MAX_LEN-current_head->data_count)>=prv_node->data_count)?prv_node->data_count:(MAX_LEN-current_head->data_count);
    					memcpy(current_head->net_mem+current_head->data_count,prv_node->net_mem,copy_length);
    					current_head->data_count+=copy_length;
    					prv_node->data_count-=copy_length;
    					if(prv_node->data_count<=0)
    					{
    						prv_node=prv_node->next;
    					}
    					else
    					{
    						memmove(prv_node->net_mem,prv_node->net_mem+copy_length,prv_node->data_count);
    						break;
    					}

    				}
    			}



    			unsigned char *file_add=current_head->net_mem;
    			file_add+= 10;
    			memcpy(command,file_add,3);			//命令字
			printf("COMMANDS: %x,%x,%x\n",command[0],command[1],command[2]);
			file_add+= 10;
    			int i=0;
    			while ((net_commands_pc[i].string)&&(net_cmd_compare(command,net_commands_pc[i].string) == 0))
    				i++;
    			unsigned char *cmdbuf=(unsigned char [40]){};
    			switch(net_commands_pc[i].id)
    			{
    			case SIORC://信息发布 510
    			{
    				app_debug(DBG_INFO,"510\n");
    				number++;
    				int send_len=20;
    				cmdbuf[0]=0x07;
    				cmdbuf[1]=0xb8;
    				memcpy(&cmdbuf[2],&send_len,4);
    				memcpy(&cmdbuf[6],"req=",4);
    				memcpy(&cmdbuf[10],"511",3);
    				memcpy(&cmdbuf[13],"&query=",7);
    				memcpy(&cmdbuf[20],file_add,6);

    				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len+6);
    				net_runtime->connect_on = NET_STOP;
#if DEV_TERMINAL|DEV_GATE
    				char fpath[100] = {'\0'};
    				time_t now;
    				struct tm *tnow;

    				time(&now);
    				tnow = localtime(&now);
    				/*struct timeval time_now;
            	gettimeofday(&time_now,NULL);*/

    				short type;
    				memcpy(&type,file_add,2);
    				printf("type=%d,len:%d\n",type,sizeof(long int));
    				switch(type)
    				{
    				case 1:
    					//sprintf(fpath, "./rpcpic/%8x%8x.txt", time_now.tv_sec,time_now.tv_usec);
    					sprintf(fpath, "./rpcpic/%d.%02d.%02d.%02d.%02d.%02d_%.4x.txt", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min,tnow->tm_sec,number);
    					break;
    				case 2:
    					sprintf(fpath, "./rpcpic/%d.%02d.%02d.%02d.%02d.%02d_%.4x.jpg", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min,tnow->tm_sec,number);
    					break;
    				default:
    					break;
    				}
    				int fd=0,bytes=0;
    				fd=open(fpath,O_WRONLY|O_CREAT,0664);
    				if(fd<0)
    				{
    					printf("create %s failure\n",fpath);
    				}
    				else{
    					net_mem *next;
    					net_mem *prv_node=current_head;
    					int i=0;
    					while(prv_node!=NULL)
    					{
    						if(i==0)
    						{
    							switch(type)
    							{
    							case 1:
    								bytes=write(fd,file_add+6,prv_node->data_count-26);
    								if(bytes>0)
    									i+=bytes;
    								break;
    							case 2:
    								bytes=write(fd,file_add+6,prv_node->data_count-26);
    								if(bytes>0)
    									i+=bytes;
    								break;
    							}
    						}
    						else
    						{
    							bytes=write(fd,prv_node->net_mem,prv_node->data_count);
    						}
    						if(bytes>0)
    							i+=bytes;
    						printf("i=%d\n",i);
    						next=prv_node->next;
    						prv_node=next;
    					}
    				}
    				close(fd);
#endif
    				net2proc_data.msg_type=MSG_FROM_NET;
    				net_data_msg net2proc_msg;
    				net2proc_msg.flag_addr=PC_DATA;
    				net2proc_msg.net_cmd=SIORC;
    				memcpy(net2proc_msg.buf,file_add,6);
    				net2proc_msg.data_len=6;
    				net2proc_msg.num=net_runtime->event_num;
    				memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    			}
    			break;

    			case RBORC://513
    			{
    				app_debug(DBG_INFO,"513\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=RBORC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;
    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						if((total_len- 20)<MAX_LEN){
    							memcpy(new_head->net_mem,file_add,total_len- 20);
    							new_head->data_count=total_len;
    						}
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case JLYDC://515
    			{
    				app_debug(DBG_INFO,"515:%d %d %d %d\n",*(file_add+0),*(file_add+1),*(file_add+2),*(file_add+3));
    				///	net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=JLYDC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case GDATC://518
    			{
    				app_debug(DBG_INFO,"518 cmd:0x%2.2x dest_No: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x src_No: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x \n",\
    						*(file_add+0),*(file_add+1),*(file_add+2),*(file_add+3),*(file_add+4),*(file_add+5),\
    						*(file_add+6),*(file_add+7),*(file_add+8));
    				/*               net2proc_data.msg.data.net_cmd = GDATC;
                 memcpy(&net2proc_data.msg.data.cmd, file_add, total_len-20);
                 net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);*/

    				//	net_data_msg msg;
    				if(total_len<=30){
    					memcpy(msg.buf,file_add,total_len-20);
    					msg.net_cmd=GDATC;
    					msg.flag_addr=PC_DATA;
    					msg.data_len=total_len-20;
    					memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    					net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				}
    			}
    			break;
    			case RGDAC://519
    			{
    				//	rtel_data rcall_data;
    				app_debug(DBG_INFO,"519\n");
    				/*             	memcpy(&rcall_data,file_add,sizeof(rtel_data));
             	ajb_msg_ret_build(&net2proc_ret_data.msg,RGDAC,SCODE,rcall_data.allow,\
             			rcall_data.oper_type,rcall_data.resstaus,MEM_DATA);
             	net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);*/
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,2);
    				msg.net_cmd=RGDAC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=2;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RCBDC://523
    			{
    				app_debug(DBG_INFO,"523\n");
    				//	net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=RCBDC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case CJLBC://524
    			{
    				app_debug(DBG_INFO,"524\n");
    				//	net_data_msg msg;
    				memcpy(msg.buf,file_add,6);
    				msg.net_cmd=CJLBC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=6;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case WXSTC://526
    			{
    				app_debug(DBG_INFO,"526\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,6);
    				msg.net_cmd=WXSTC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=6;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case WXLYC://528
    			{
    				app_debug(DBG_INFO,"528\n");
    				int cmd_len=14+4;
    				unsigned char cmdbuf[30];
    				cmdbuf[0]=0x7;
    				cmdbuf[1]=0xb8;
    				memcpy(&cmdbuf[2],&cmd_len,4);
    				memcpy(&cmdbuf[6],"req=",4);
    				memcpy(&cmdbuf[10],"529",3);
    				memcpy(&cmdbuf[13],"&query=",7);
    				memcpy(&cmdbuf[20],file_add,4);

    				char fpath[100] = {'\0'};
    				time_t now;
    				struct tm *tnow;
    				number++;
    				time(&now);
    				tnow = localtime(&now);
    				sprintf(fpath, "./rleave/%d.%02d.%02d.%02d.%02d.%02d_%.4x.wav", 1900+tnow->tm_year, \
    						tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min,tnow->tm_sec,number);
    				int fd=-1,bytes=0;
    				fd=open(fpath,O_WRONLY|O_CREAT,0664);
    				if(fd<0)
    					printf("create %s failure\n",fpath);
    				else{
    					net_mem *next;
    					net_mem *prv_node=current_head;
    					int i=0;
    					while(prv_node!=NULL)
    					{
    						if(i==0)
    						{
    							unsigned int magic=0;
    							memcpy(&magic,file_add+8,4);
    							if(magic!=WAV_RIFF)
    							{
    								app_debug(DBG_INFO,"528 no wave head\n");
    								int data_len=0;
    								memcpy(&data_len,file_add+4,4);
    								wrte_leave_msg_to_wave_head(fd,data_len);
    							}
    							bytes=write(fd,file_add+8,prv_node->data_count-28);
    						}
    						else
    						{
    							bytes=write(fd,prv_node->net_mem,prv_node->data_count);
    						}
    						if(bytes>0)
    							i+=bytes;
    						next=prv_node->next;
    						prv_node=next;
    					}
    				}
    				if(fd>0)
    					close(fd);

    				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,cmd_len+6);

    				net2proc_data.msg_type=MSG_FROM_NET;
    				net_data_msg net2proc_msg;
    				net2proc_msg.flag_addr=PC_DATA;
    				net2proc_msg.net_cmd=WXLYC;
    				memcpy(net2proc_msg.buf,file_add,8);
    				net2proc_msg.data_len=8;
    				net2proc_msg.num=net_runtime->event_num;
    				memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    			}
    			break;
    			case WXJDC://530
    			{
    				app_debug(DBG_INFO,"530\n");
    				/*            	char recv_buf[4];
            	app_debug(DBG_INFO,"528\n");
            	int cmd_len=14+5;
            	unsigned char cmdbuf[30];
            	cmdbuf[0]=0x7;
            	cmdbuf[1]=0xb8;
            	memcpy(&cmdbuf[2],&cmd_len,4);
            	memcpy(&cmdbuf[6],"req=",4);
            	memcpy(&cmdbuf[10],"529",3);
            	memcpy(&cmdbuf[13],"&query=",7);
            	memcpy(&cmdbuf[20],file_add,4);
            	char state=0;
            	memcpy(&cmdbuf[24],&state,1);
            	memcpy(&recv_buf,file_add+4,4);
            	app_debug(DBG_INFO,"530 recv_buf:%2.2x,%2.2x,%2.2x,%2.2x\n",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3]);
            	net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,cmd_len+6);*/

    				net2proc_data.msg_type=MSG_FROM_NET;
    				net_data_msg net2proc_msg;
    				net2proc_msg.flag_addr=PC_DATA;
    				net2proc_msg.net_cmd=WXJDC;
    				memcpy(net2proc_msg.buf,file_add,8);
    				net2proc_msg.data_len=8;
    				net2proc_msg.num=net_runtime->event_num;
    				memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);

    			}
    			break;
    			case PZLYC://532
    			{
    				app_debug(DBG_INFO,"532\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=PZLYC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case RRPCC://535
    			{
    				app_debug(DBG_INFO,"535\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=RRPCC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case GDSTC://536
    			{
    				app_debug(DBG_INFO,"536\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=GDSTC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				/*            	int cmd_len=14+5;
            	unsigned char cmdbuf[30];
            	cmdbuf[0]=0x7;
            	cmdbuf[1]=0xb8;
            	memcpy(&cmdbuf[2],&cmd_len,4);
            	memcpy(&cmdbuf[6],"req=",4);
            	memcpy(&cmdbuf[10],"537",3);
            	memcpy(&cmdbuf[13],"&query=",7);
            	memcpy(&cmdbuf[20],file_add,4);
            	cmdbuf[25]=1;//门状态
            	cmd_len+=1;
            	net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,cmd_len+6);*/
    			}
    			break;
    			case GCJTC://538
    			{
    				app_debug(DBG_INFO,"538 \n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,2);
    				msg.flag_addr=PC_DATA;
    				msg.net_cmd=GCJTC;
    				msg.data_len=2;
    				memcpy(&net2proc_ret_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
    				//	net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case CJLYPC://540
    			{
    				app_debug(DBG_INFO,"540\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=CJLYPC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    			}
    			break;
    			case RZARC://543
    			{
    				app_debug(DBG_INFO,"543 %d \n",*file_add);
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.flag_addr=PC_DATA;
    				msg.net_cmd=RZARC;
    				msg.data_len=4;
    				memcpy(&net2proc_ret_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RBFZC://545
    			{
    				app_debug(DBG_INFO,"545\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=RBFZC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case FWYDC://547
    			{
    				app_debug(DBG_INFO,"547\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=FWYDC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case FCYDC://549
    			{
    				app_debug(DBG_INFO,"549:%d %d %d %d\n",*(file_add+0),*(file_add+1),*(file_add+2),*(file_add+3));
    				memcpy(msg.buf,file_add,4);
    				msg.flag_addr=PC_DATA;
    				msg.net_cmd=FCYDC;
    				msg.data_len=4;
    				memcpy(&net2proc_ret_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_ret_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RFQSC://551
    			{
    				app_debug(DBG_INFO,"551\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=RFQSC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RGODC://552
    			{
    				app_debug(DBG_INFO,"552\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=RGODC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;
    				file_add+=10;
    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);

    				/*            	int cmd_len=14+5;
            	unsigned char cmdbuf[30];
            	cmdbuf[0]=0x7;
            	cmdbuf[1]=0xb8;
            	memcpy(&cmdbuf[2],&cmd_len,4);
            	memcpy(&cmdbuf[6],"req=",4);
            	memcpy(&cmdbuf[10],"553",3);
            	memcpy(&cmdbuf[13],"&query=",7);
            	memcpy(&cmdbuf[20],file_add,4);
            	memcpy(&cmdbuf[25],&fqzt,4);//取防区状态
            	cmd_len+=4;
            	net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,cmd_len+6);*/

    			}
    			break;
    			case RFLRC://555
    			{
    				app_debug(DBG_INFO,"555\n");
    				/*            unsigned int fqh;//防区号
            	memcpy(&fqh,file_add+4,4);
            	app_debug(DBG_INFO,"555 fqh:0x%x\n",fqh);*/

    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,8);
    				msg.net_cmd=RFLRC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=8;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RELRC://557
    			{
    				app_debug(DBG_INFO,"557\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=RELRC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case DALRC://558
    			{
    				app_debug(DBG_INFO,"558\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=DALRC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				app_debug(DBG_FATAL,"net_runtime->event_num:%d!",net2proc_data.msg.num);
    				/*            	int cmd_len=14+4;
            	unsigned char cmdbuf[30];
            	cmdbuf[0]=0x7;
            	cmdbuf[1]=0xb8;
            	memcpy(&cmdbuf[2],&cmd_len,4);
            	memcpy(&cmdbuf[6],"req=",4);
            	memcpy(&cmdbuf[10],"559",3);
            	memcpy(&cmdbuf[13],"&query=",7);
            	memcpy(&cmdbuf[20],file_add,4);
            	net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,cmd_len+6);*/
    			}
    			break;
    			case RCARC://561
    			{
    				app_debug(DBG_INFO,"561\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=RCARC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RGNCC://563
    			{
    				app_debug(DBG_INFO,"563\n");
    				/*            	short count=0;
            	memcpy(&count,file_add+4,2);
            	app_debug(DBG_INFO,"563 count:%d\n",count);*/

    				/*            	char IP[4];
            	short Port;
            	short Channel;
            	char Name[22];*/
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=RGNCC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RFJJC://565
    			{
    				app_debug(DBG_INFO,"565\n");
    				/*
            	char time_buf[6];
            	memcpy(&time_buf,file_add+4,6);
            	app_debug(DBG_INFO,"565 %d %d %d %d %d %d\n",time_buf[0],time_buf[1],time_buf[2],time_buf[3],time_buf[4],time_buf[5]);
    				 */
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,10);
    				msg.net_cmd=RFJJC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=10;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;

    			}
    			break;
    			case GTXLC://567
    			{
    				app_debug(DBG_INFO,"567\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=GTXLC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RCHWC://571
    			{
    				app_debug(DBG_INFO,"571\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=RCHWC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RPCMC://573
    			{
    				app_debug(DBG_INFO,"573\n");
    				char fpath[100] = {'\0'};
    				char msg_count=0;
    				time_t now;
    				struct tm *tnow;
    				time(&now);
    				tnow = localtime(&now);
    				msg_count=*(file_add+4);
    				int j=0;
    				int bytes=0;
    				char msg_buf[71*msg_count];
    				int pos=25;
    				int byte_count=0;
    				int msg_begin=0;
    				int msg_num=1;
    				number++;
    				bytes=get_net_mem_data_bypos(current_head,msg_buf,pos,70*msg_count);
    				for(;j<bytes;j++)
    				{
    					app_debug(DBG_INFO,"%02x ",msg_buf[j]);
    					if('\0'==msg_buf[j])
    					{
    						sprintf(fpath, "./rpctxt/%d.%02d.%02d.%02d.%02d.%02d_%02d_%.4x.txt", \
    								1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, \
    								tnow->tm_min,tnow->tm_sec,msg_num,number);
    						app_debug(DBG_INFO,"fpath:%s\n",fpath);
    						/*            			int ret=write_msg_to_txt_file(fpath,&msg_buf[msg_begin],byte_count);
            			app_debug(DBG_INFO,"ret=%d\n",ret);*/
    						msg_begin=j+1;
    						byte_count=0;
    						msg_num++;
    					}
    					else{
    						byte_count++;
    						continue;
    					}
    				}
    				net2proc_data.msg_type=MSG_FROM_NET;
    				net_data_msg net2proc_msg;
    				net2proc_msg.flag_addr=PC_DATA;
    				net2proc_msg.net_cmd=RPCMC;
    				memcpy(net2proc_msg.buf,file_add,5);
    				net2proc_msg.data_len=5;
    				net2proc_msg.num=net_runtime->event_num;
    				memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}

    			break;
    			case RPCDC://575
    			{
    				app_debug(DBG_INFO,"575\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=RPCDC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RPTEC://577
    			{
    				app_debug(DBG_INFO,"577\n");
    				/*            	char fpath[100] = {'\0'};
            	char msg_count=0;
            	time_t now;
            	struct tm *tnow;
            	time(&now);
            	tnow = localtime(&now);
            	msg_count=*(file_add+4);
            	int j=0;
            	int bytes=0;
            	char msg_buf[71*msg_count];
            	int pos=25;
            	int byte_count=0;
            	int msg_begin=0;
            	int msg_num=1;
            	bytes=get_net_mem_data_bypos(current_head,msg_buf,pos,70*msg_count);
            	for(;j<bytes;j++)
            	{
 //           		app_debug(DBG_INFO,"%c ",msg_buf[j]);
            		if('\0'==msg_buf[j])
            		{
 //           			app_debug(DBG_INFO,"\n ");
            			sprintf(fpath, "./rpctxt/%d.%02d.%02d.%02d.%02d.%02d_%02d.txt", \
            					1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, \
            					tnow->tm_min,tnow->tm_sec,msg_num);
//            			app_debug(DBG_INFO,"~~~~~~~~~buf_pos:%d byte_count:%d fpath:%s\n",j,byte_count,fpath);
            		//	int ret=write_msg_to_txt_file(fpath,&msg_buf[msg_begin],byte_count);
//            			app_debug(DBG_INFO,"ret=%d\n",ret);
            			msg_begin=j+1;
            			byte_count=0;
            			msg_num++;
            		}
            		else{
            			byte_count++;
            			continue;
            		}
            	}*/
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=RPTEC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case CRDDC://578
    			{
    				app_debug(DBG_INFO,"578\n");

    				/*            	命令字+室号+灯号
            	(命令字：1开2关3增4减，室号从1开始，灯号0开始)*/
    				/*	net_data_msg msg;
            	memcpy(msg.buf,file_add,7);
            	msg.net_cmd=CRDDC;
            	msg.flag_addr=PC_DATA;
            	msg.data_len=7;
            	memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
            	net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);*/
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=CRDDC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);
    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    			}
    			break;
    			case RPFKC://580
    			{
    				app_debug(DBG_INFO,"580\n");
    				unsigned char cmdbuf[40];
    				int cmd_len=0;
    				memcpy(msg.buf,file_add,6);
    				msg.net_cmd=RPFKC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=6;
    				int pic_num=0;
    				if(total_len>=26)
    				{
    					if(is_call_me(file_add))
    					{

    						unsigned short int num=0;
    						memcpy(&num,file_add+4,2);
    						app_debug(DBG_INFO,"num=%d\n",num);
    						char name_buf[64][256];
    						DIR *dp;
    						struct dirent *ep;
    						dp = opendir (MSGPIC_PATH);
    						if (dp != NULL)
    						{
    							while ((ep=readdir(dp))!=NULL)
    							{
    								strcpy(name_buf[pic_num],ep->d_name);
    								if((strcasecmp(".jpg",strrchr(ep->d_name,'.'))==0)||(strcasecmp(".jpeg",strrchr (ep->d_name,'.'))==0))
    								{
    									app_debug(DBG_INFO,"pos@:%d  filename:%s\n",pic_num,ep->d_name);
    									pic_num++;
    								}
    							}
    							closedir (dp);
    						}
    						else
    							perror ("Couldn’t open the directory");
    						app_debug(DBG_INFO,"pic_num=%d\n",pic_num);
    						if(num<=pic_num)
    						{

    							char *jpg_buf=alloca(300*1024);
    							int pos=pic_num-num;
    							app_debug(DBG_INFO,"pos=%d\n",pos);
    							int pic_fd=-1;
    							char pic_path[256];
    							strcpy(pic_path,MSGPIC_PATH);
    							strcat(pic_path,name_buf[pos]);
    							pic_fd=open(pic_path,O_RDONLY);
    							if(pic_fd>=0){
    								int pic_len=read(pic_fd,jpg_buf,300*1024);
    								cmd_len=14+4+pic_len;
    								cmdbuf[0]=0x07;
    								cmdbuf[1]=0xb8;
    								memcpy(&cmdbuf[2],&cmd_len,4);
    								memcpy(&cmdbuf[6],"req=",4);
    								memcpy(&cmdbuf[10],"581",3);
    								memcpy(&cmdbuf[13],"&query=",7);
    								memcpy(&cmdbuf[20],file_add,4);
    								net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20+4);
    								net_runtime->respond_fun(net_runtime->socketfd,jpg_buf,pic_len);
    								break;
    							}
    						}

    						cmd_len=14+4;
    						cmdbuf[0]=0x07;
    						cmdbuf[1]=0xb8;
    						memcpy(&cmdbuf[2],&cmd_len,4);
    						memcpy(&cmdbuf[6],"req=",4);
    						memcpy(&cmdbuf[10],"581",3);
    						memcpy(&cmdbuf[13],"&query=",7);
    						memcpy(&cmdbuf[20],file_add,4);
    						net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20+4);
    					}
    				}
    			}
    			break;
    			case RPJQC://582
    			{
    				app_debug(DBG_INFO,"582\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,5);
    				msg.net_cmd=RPJQC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=5;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case RPLMC://584
    			{
    				app_debug(DBG_INFO,"584\n");
    				unsigned char cmdbuf[40];
    				int cmd_len=0;
    				int wav_num=0;
    				if(total_len>=26)
    				{
    					if(is_call_me(file_add))
    					{
    						unsigned short int num=0;
    						memcpy(&num,file_add+4,2);
    						char name_buf[64][256];
    						DIR *dp;
    						struct dirent *ep;
    						dp = opendir (RLEAVE_PATH);
    						if (dp != NULL)
    						{
    							while ((ep=readdir(dp))!=NULL)
    							{
    								strcpy(name_buf[wav_num],ep->d_name);
    								if(strcasecmp(".wav",strrchr (ep->d_name,'.'))==0){
    									app_debug(DBG_INFO,"pos@:%d  filename:%s\n",wav_num,ep->d_name);
    									wav_num++;
    								}
    							}
    							closedir (dp);
    						}
    						else
    							perror ("Couldn’t open the directory");
    						app_debug(DBG_INFO,"pic_num=%d\n",wav_num);
    						if(num<=wav_num)
    						{
    							char *jpg_buf=alloca(160*1024);
    							int pos=wav_num-num+1;
    							int pic_fd=-1;
    							char pic_path[256];
    							strcpy(pic_path,MSGPIC_PATH);
    							strcat(pic_path,name_buf[pos]);
    							pic_fd=open(pic_path,O_RDONLY);
    							if(pic_fd>=0){
    								int pic_len=read(pic_fd,jpg_buf,160*1024);
    								cmd_len=14+4+pic_len;
    								cmdbuf[0]=0x07;
    								cmdbuf[1]=0xb8;
    								memcpy(&cmdbuf[2],&cmd_len,4);
    								memcpy(&cmdbuf[6],"req=",4);
    								memcpy(&cmdbuf[10],"585",3);
    								memcpy(&cmdbuf[13],"&query=",7);
    								memcpy(&cmdbuf[20],file_add,4);
    								net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20+4);
    								net_runtime->respond_fun(net_runtime->socketfd,jpg_buf,pic_len);
    								break;
    							}
    						}

    						cmd_len=14+4;
    						cmdbuf[0]=0x07;
    						cmdbuf[1]=0xb8;
    						memcpy(&cmdbuf[2],&cmd_len,4);
    						memcpy(&cmdbuf[6],"req=",4);
    						memcpy(&cmdbuf[10],"585",3);
    						memcpy(&cmdbuf[13],"&query=",7);
    						memcpy(&cmdbuf[20],file_add,4);
    						net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20+4);
    					}
    				}

    			}
    			break;
    			case SPDMC://587
    			{
    				app_debug(DBG_INFO,"587\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=SPDMC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{

    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RPKTC://588
    			{
    				app_debug(DBG_INFO,"588\n");
    				//	net_data_msg msg;
    				memcpy(msg.buf,file_add,6);
    				msg.net_cmd=RPKTC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=6;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case RPCHC://590
    			{
    				app_debug(DBG_INFO,"590\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,6);
    				msg.net_cmd=RPCHC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=6;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case BJYDC://593
    			{
    				app_debug(DBG_INFO,"593\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=BJYDC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case HZYDC://595
    			{
    				app_debug(DBG_INFO,"595\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=HZYDC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{

    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case WXBFC://596
    			{
    				app_debug(DBG_INFO,"596\n");

    				//	net_data_msg msg;
    				memcpy(msg.buf,file_add,10);
    				msg.net_cmd=WXBFC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=10;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);

    				/*	net_addr_msg net2proc_addr;
            	net2proc_addr.flag_addr=PC_ADDR;
            	net2proc_addr.net_cmd=WXBFC;
            	net2proc_addr.data_len=total_len-20;
            	net2proc_addr.num=net_runtime->event_num;
            	use_addr=1;

            	if (new_head == NULL)
            	{

            		int ret;
            		ret = request_list_head_mem(&new_head);
            		if (ret == 0) {
            			memcpy(new_head->net_mem,file_add,total_len- 20);
            			new_head->data_count=total_len;
            		} else
            			goto error;
            	}
            	else
            		memmove(new_head->net_mem,file_add,new_head->data_count-20);

            	net2proc_addr.list_num = new_head->mem_num;
            	memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
            	new_head->data_count = new_head->data_count - 20;
            	net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);*/
    			}
    			break;
    			case BDYDC://599
    			{
    				app_debug(DBG_INFO,"599\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=BDYDC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case SKPZC://600
    			{
    				app_debug(DBG_INFO,"600\n");
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,9);
    				msg.net_cmd=SKPZC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=9;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    			}
    			break;
    			case LQYDC://603
    			{
    				app_debug(DBG_INFO,"603\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=LQYDC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{

    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case TJYDC://605
    			{
    				app_debug(DBG_INFO,"605\n");
    				if(total_len<48)
    					goto error;
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=TJYDC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case YMYDC://651
    			{
    				app_debug(DBG_INFO,"651\n");
    				if(total_len<174)
    					goto error;
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=YMYDC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case BFJDC://661
    			{
    				app_debug(DBG_INFO,"661\n");
    				if(total_len<24)
    					goto error;
    				//net_data_msg msg;
    				memcpy(msg.buf,file_add,4);
    				msg.net_cmd=BFJDC;
    				msg.flag_addr=PC_DATA;
    				msg.data_len=4;
    				memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case GCJ2C://662
    			{
    				app_debug(DBG_INFO,"662\n");
    			}
    			break;
    			case ZCJ2C://664
    			{
    				app_debug(DBG_INFO,"664\n");
    			}
    			break;
    			case RLTXC://671
    			{
    				app_debug(DBG_INFO,"671\n");
    				/*            	if(total_len<174)
            		goto error;*/
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=RLTXC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			case RFTPC://673
    			{
    				app_debug(DBG_INFO,"673\n");
    			}
    			break;
    			case SPORC://826
    			{
    				app_debug(DBG_INFO,"826\n");
    				/*            unsigned short int package_count=0;
            	unsigned short int package_num=0;
            	unsigned short int package_len=0;*/
    				unsigned short int package_type=0;
    				short int ret_status=0;
    				/*            memcpy(&package_count,file_add,2);
            	memcpy(&package_num,file_add+2,2);
            	memcpy(&package_len,file_add+4,2);*/
    				memcpy(&package_type,file_add+6,2);

    				switch(package_type)
    				{
    				case UpdateSetupPro://1
            		ret_status=PRAG_TYPE_ERR;
            		break;
                	case UpdateMainPro://2
                       case UpdateResData://3
                	{
                		//ret_status=SUCCESS;
                		ret_status=0;
                		ret_status=save_program(current_head,UPDATE_NAME);
                	}
                	break;
                	default :
                		ret_status=OTHER_ERR;
                		break;
                	}
    				unsigned char cmdbuf[40];
    				int cmd_len=14+10;
    				cmdbuf[0]=0x7;
    				cmdbuf[1]=0xb8;
    				memcpy(&cmdbuf[2],&cmd_len,4);
    				memcpy(&cmdbuf[6],"req=",4);
    				memcpy(&cmdbuf[10],"827",3);
    				memcpy(&cmdbuf[13],"&query=",7);
    				memcpy(&cmdbuf[20],file_add,8);
    				memcpy(&cmdbuf[28],&ret_status,2);
    				printf("ruan ret_status=%d\n",ret_status);
    				printf("ruan ret_status=%d,%2x,%2x\n",ret_status,&cmdbuf[28],&cmdbuf[29]);
    				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20+10);
    				usleep(100000);
    				use_addr=0;
    				//if (ret_status == SUCCESS) {
    				if (ret_status == 0) {
    								   printf("*****ruan*********\n");
                                       usleep(100000);
                                       dev_sys_reboot();
                                       //dev_app_restart();
                                   }
    			}
    			break;
                           case CJINC: // 712 PC设置层间控制器所接分机列表
				case SCLGC: // 726 PC设置名录
				case SCRDC: // 742 PC设置主机卡头数据
				case UMJKCN:
                           case UNMJKC:
                {
	                net_addr_msg net2proc_addr;
	                net2proc_addr.flag_addr=PC_ADDR;
	                net2proc_addr.net_cmd=net_commands_pc[i].id;
	                net2proc_addr.data_len=total_len-20;
	                net2proc_addr.num=net_runtime->event_num;
	                use_addr=1;

	                if (new_head == NULL)
	                {
		                int ret;
		                ret = request_list_head_mem(&new_head);
		                if (ret == 0) {
			                memcpy(new_head->net_mem,file_add,total_len- 20);
			                new_head->data_count=total_len;
		                } else
			                goto error;
	                }
	                else
		                memmove(new_head->net_mem,file_add,new_head->data_count-20);

	                net2proc_addr.list_num = new_head->mem_num;
	                memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
	                new_head->data_count = new_head->data_count - 20;
	                net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
                }
                break;
                case HQFJC:	// 获取主机控制器分机列表 720
				case GCLGC:	// 获取名录 724
				case GCRDC: // 获取主机卡头数据

                {
	                msg.net_cmd=net_commands_pc[i].id;
	                msg.flag_addr=PC_DATA;
	                msg.data_len=0;
	                memcpy(&net2proc_data.msg,&msg,sizeof(net_data_msg));
	                net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
                }

                break;
			case SMJKCN:
                    case SNMJKC:
					{
						net_data_msg net2proc_msg;
						app_debug(DBG_INFO,"%03X\n", net_commands_pc[i].id);
						net2proc_data.msg_type=MSG_FROM_NET;
						net2proc_msg.flag_addr=PC_DATA;
						net2proc_msg.net_cmd=net_commands_pc[i].id;
						memcpy(net2proc_msg.buf,file_add,4);
						net2proc_msg.data_len=4;
						net2proc_msg.num=net_runtime->event_num;
						memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
						net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
					} 
				break;
    			case GREFC:
    			{
    				app_debug(DBG_INFO,"828 \n");
    				unsigned char ucTime[10]={0x01,0x02,0x03,0x04,0x5,0x06,0x07};
    				unsigned short Len=sizeof(gData);
    				unsigned char str[Len+100];
    				time_t now;
    				struct tm *tnow;
    				time(&now);
    				tnow = localtime(&now);
    				ucTime[0]=Dec2Bcd(tnow->tm_sec);
    				ucTime[1]=Dec2Bcd(tnow->tm_min);
    				ucTime[2]=Dec2Bcd(tnow->tm_hour);
    				ucTime[3]=Dec2Bcd(tnow->tm_mday);
    				ucTime[4]=Dec2Bcd(tnow->tm_mon+1);
    				if(tnow->tm_wday==0)
    					ucTime[5]=Dec2Bcd(7);
    				else
    					ucTime[5]=Dec2Bcd(tnow->tm_wday);
    				ucTime[6]=Dec2Bcd(tnow->tm_year-100);

    				load_global_data();
    				memcpy(&str[0],&Len,2);//配置信息长度
    				Len=Len+2;
    				memcpy(&str[2],&gData,sizeof(gData));//配置信息
    				memcpy(&str[Len],ucTime,sizeof(ucTime));
    				Len=Len+sizeof(ucTime);

    				unsigned char cmdbuf[21];
    				cmdbuf[0]=0x7;
    				cmdbuf[1]=0xb8;
    				memcpy(&cmdbuf[2],&Len,4);
    				memcpy(&cmdbuf[6],"req=",4);
    				memcpy(&cmdbuf[10],"829",3);
    				memcpy(&cmdbuf[13],"&query=",7);
    				app_debug(DBG_INFO,"829 cmd_len:%d exit_flag:%d\n",Len,again_flag);

    				if(	exit_flag==0x01)
    					exit_flag=0x11;
    				else
    					exit_flag=0;

#if DEV_CONTROL|DEV_GLJKZQ|DEV_GATE|DEV_TSGLJKZQ

    				if(	again_flag==0x01){
    					send_data_again(cmdbuf,20);
    					send_data_again(str,Len);
    					break;
    				}
    				int ret=0;
    				int try = 0;
    				tcp_data *tcp_buf = NULL;
    				unsigned int ip_addr=net_runtime->dest_ip;
    				do {
    					if (try > 3){
    						if(ret<0){
    							app_debug(DBG_ERROR, "request tcp_buf\n");
    						}
    						else
    							break;
    					}
    					ret = request_send_buf(&tcp_buf,1024,TCP_PC);
    					try++;
    				} while ((ret < 0) && (NULL == tcp_buf));
    				if((tcp_buf==NULL)||(ret<0))
    					break;
    				tcp_buf->dest_ip=ip_addr;
    				tcp_buf->length=Len+20;
    				memcpy(tcp_buf->buf,cmdbuf,20);
    				memcpy(&tcp_buf->buf[20],str,Len);
    				int items = ITEM_ONE;
    				ret = send_tcp_data(&items,TCP_PC);
    				again_flag=0x01;

#else //DEV_TERMINAL
    				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20);
    				net_runtime->respond_fun(net_runtime->socketfd,str,Len);
#endif
    			}
    			break;
    			case CREFC:
    			{
    				app_debug(DBG_INFO,"830\n");
    				unsigned short Len;
    				unsigned char ucTime[7]={};
    				memcpy(&Len,(char *)file_add,2);//配置信息长度
    				get_net_mem_data_bypos(current_head,&gData,22,sizeof(gData));
    				get_net_mem_data_bypos(current_head,ucTime,Len+22,7);
    				set_system_time(ucTime);
    				if(save_global_data()<0)
    					app_debug(DBG_INFO,"save_global_data error\n");

    				exit_flag=0x01;
    				unsigned char cmdbuf[21];
    				int cmd_len=14;
    				cmdbuf[0]=0x7;
    				cmdbuf[1]=0xb8;
    				memcpy(&cmdbuf[2],&cmd_len,4);
    				memcpy(&cmdbuf[6],"req=",4);
    				memcpy(&cmdbuf[10],"831",3);
    				memcpy(&cmdbuf[13],"&query=",7);

#if DEV_CONTROL|DEV_GLJKZQ|DEV_GATE|DEV_TSGLJKZQ
    				send_data_again(cmdbuf,20);
#else //DEV_TERMINAL
    				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,20);
#endif
    			}
    			break;
    			case RCREC://831
    			{
    				app_debug(DBG_INFO,"831\n");
    				net_data_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_DATA;
    				net2proc_addr.net_cmd=RCREC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				memcpy((char *)&net2proc_data.msg,(char *)&net2proc_addr,sizeof(net_data_msg));
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    			}
    			break;
    			case RGLJC://833
    			{
    				app_debug(DBG_INFO,"833\n");

    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=RGLJC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    			}
    			break;
    			case GCITC://835
    			{
    				app_debug(DBG_INFO,"835\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=GCITC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;


    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {

    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);
					debug_info("ret = %d \n",ret);
    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
									debug_info("ret0 = %d \n",ret);	
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
										debug_info("ret1 = %d \n",ret);
    			}
    			break;
    			case SCITC:
    			{
    				app_debug(DBG_INFO,"837\n");
    				net_addr_msg net2proc_addr;
    				net2proc_addr.flag_addr=PC_ADDR;
    				net2proc_addr.net_cmd=SCITC;
    				net2proc_addr.data_len=total_len-20;
    				net2proc_addr.num=net_runtime->event_num;
    				use_addr=1;

    				if (new_head == NULL)
    				{
    					int ret;
    					ret = request_list_head_mem(&new_head);
    					if (ret == 0) {
    						memcpy(new_head->net_mem,file_add,total_len- 20);
    						new_head->data_count=total_len;
    					} else
    						goto error;
    				}
    				else
    					memmove(new_head->net_mem,file_add,new_head->data_count-20);

    				net2proc_addr.list_num = new_head->mem_num;
    				memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
    				new_head->data_count = new_head->data_count - 20;
    				net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
    			}
    			break;
				case JDPZC:
				{
					net2proc_data.msg_type=MSG_FROM_NET;
					net_data_msg net2proc_msg;
					net2proc_msg.flag_addr=PC_DATA;
					net2proc_msg.net_cmd=JDPZC;
					memcpy(net2proc_msg.buf,file_add,4);
					net2proc_msg.data_len=4;
					net2proc_msg.num=net_runtime->event_num;
					memcpy((char *)&net2proc_data.msg,(char *)&net2proc_msg,sizeof(net_data_msg));
					net_runtime->send_to_proc(&net2proc_data, MSG_LEN_PROC);
				}
    			default:{
    				net_runtime->connect_on = NET_STOP;
    			}
    			break;
    			}
    			// del_all_net_mem(&head.next);
    			current_node=&head;
    			//net_runtime->connect_on = NET_STOP;

    			if((new_head!=NULL)&&(use_addr==0)){
    				delete_list_mem(new_head);
    				new_head=NULL;
    			}
    			total_len=0;
    			memset(&head,0,sizeof(net_mem));
    		}
    	}

}

error:
	if((new_head!=NULL)&&(use_addr==0)){
		delete_list_mem(new_head);
	}
	close(net_runtime->socketfd);
	unregister_connect_node(net_runtime);
	app_debug(DBG_INFO,"TCP_PC data process pthread exited !\n");
return;
}


/**
 *
 * TCP 18026端口数据接收与处理函数
 * @param [in] net_runtime 网络节点
*/

void tcp_pc_json_process_task(connect_node *net_runtime)
{
    app_debug(DBG_INFO,"tcp_pc_json_process_task starting !\n");
    static unsigned short number=0;
    unsigned int again_flag=0;
    unsigned int exit_flag=0;
    int recv_bytes=0;
    int current_len=0;
    int total_len=0;
    int use_addr=0;
    int tmp_len =0;
    unsigned char *recv_buf;
    unsigned char *recv_ptr;
    char command[3];

    net_mem head,*current_node,*current_head,*new_head=NULL;
    memset(&head,0,sizeof(net_mem));

    head.next=NULL;
    current_head=current_node=&head;
    proc_msg net2proc_data;
    proc_msg_ret net2proc_ret_data;
    net_data_msg msg;
    msg.num=net_runtime->event_num;
    net2proc_data.msg_type=MSG_FROM_NET;
    net2proc_ret_data.msg_type=MSG_FROM_NET;
    struct timeval time_out;
    time_out.tv_sec=1;
    time_out.tv_usec=0;
    app_debug(DBG_INFO,"[%s] net_runtime->event_num  %d\n",__func__,net_runtime->event_num);
    int ret=setsockopt(net_runtime->socketfd,SOL_SOCKET,SO_RCVTIMEO,&time_out,sizeof(time_out));
    if(ret<0)
    {
    	app_debug(DBG_FATAL,"Set SO_RCVTIMEO failure!");
    	goto error;
    }
    int max_recv_length=MAX_LEN;
    int count = 0;
    recv_buf=current_node->net_mem;
    while(net_runtime->connect_on)
    {

    	//struct sockaddr src_addr;
    	//socklen_t length=sizeof(struct sockaddr_in);
    	recv_bytes=recv_data(net_runtime->socketfd,recv_buf,max_recv_length);
    	//recv_bytes=recvfrom(net_runtime->socketfd,recv_buf,MAX_LEN,0,&src_addr,&length);
    	if(recv_bytes==0)
    	{
    		net_runtime->connect_on = NET_STOP;
    		app_debug(DBG_INFO,"%s: recv_bytes=0 exit !\n",__func__);
    		again_flag=0;
    		if(exit_flag==0x11){
    			dev_app_restart();
    		}
    		continue;
    	}
    	else if(recv_bytes<0)
    	{
    		//int num=errno;
			perror("perror");//EWOULDBLOCK和EAGAIN的值相同
        	if(errno==EWOULDBLOCK)
        	{
        		count++;        	
        		if (count > 2) {
                    /*应对后台不主动关闭tcp连接的情形，超时就释放资源、断开tcp*/        		
        			app_debug(DBG_INFO,"socket timeout exit !\n");
        			net_runtime->connect_on = NET_STOP;
        			again_flag=0;
        			if(exit_flag==0x11){
        				dev_app_restart();
        			}        			
        		}
        		continue;
        	}
        	else
        	{
        		app_debug(DBG_INFO,"socket exit!\n");
        		net_runtime->connect_on = NET_STOP;
        		continue;
        	}			
    	}
    	else
    	{
    		current_node->data_count+=recv_bytes;
    		if(current_head->data_count<TCP_HEAD_LENGTH_18020)
    		{
    			recv_buf=current_head->net_mem+current_node->data_count;
    			max_recv_length=MAX_LEN-current_node->data_count;
    			continue;
    		}

    		if ((total_len==0)&&(current_head->net_mem[0] == 0x0a) && (current_head->net_mem[1] == 0xB8) )
    		{
    			recv_ptr=current_head->net_mem;
    			current_len=0;
    			memcpy((char*)&total_len,recv_ptr+2,4);  // 数据长度
    			total_len+=6;
    			current_len += current_node->data_count;
    			//app_debug(DBG_INFO,"total_len=%d current_len=%d\n",total_len,current_len);
    			printf("total_len=%d current_len=%d\n",total_len,current_len);
    			if( total_len!= current_len )
    			{
    				int ret;
    				//数据存储，用于外发地址
    				app_debug(DBG_INFO,"COUNT_ITEM(total_len)=%d\n",COUNT_ITEM(total_len));
    				ret=request_list_head_mem(&new_head);
    				if(ret==0){
    					current_node->mem_num=new_head->mem_num;
    					memcpy(new_head,current_node,sizeof(net_mem));
    					current_head=current_node=new_head;
    					app_debug(DBG_INFO,"current_head->mem_num=%d\n",current_head->mem_num);
    					ret=add_net_mem(&current_node,COUNT_ITEM(total_len));
    					if(ret<0) goto error;
    					current_node=current_node->next;
    					recv_buf=current_node->net_mem;
    					max_recv_length=MAX_LEN;
    				}else
    					goto error;
    			}
    		}
    		else
    		{
    			if(current_node->next==NULL){
    				ret=add_net_mem(&current_node,2);
    				if(ret<0) {
    					goto error;
    				}
    			}
    			current_node=current_node->next;
    			current_len+= recv_bytes;
    			recv_buf=current_node->net_mem;
    			max_recv_length=MAX_LEN;
    			//	app_debug(DBG_INFO,"current_len=%d\n",current_len);
    		}

    		if(total_len== current_len )
    		{
    			net_mem *prv_node=current_head->next;
    			if(total_len<=MAX_LEN)
    			{
    				while(prv_node!=NULL)
    				{
    					memcpy(current_head->net_mem+current_head->data_count,prv_node->net_mem,prv_node->data_count);
    					current_head->data_count+=prv_node->data_count;
    					prv_node->data_count=0;
    					prv_node=prv_node->next;
    				}
    			}
    			else
    			{
    				while((prv_node!=NULL)&&(current_head->data_count<MAX_LEN))
    				{
    					int copy_length=((MAX_LEN-current_head->data_count)>=prv_node->data_count)?prv_node->data_count:(MAX_LEN-current_head->data_count);
    					memcpy(current_head->net_mem+current_head->data_count,prv_node->net_mem,copy_length);
    					current_head->data_count+=copy_length;
    					prv_node->data_count-=copy_length;
    					if(prv_node->data_count<=0)
    					{
    						prv_node=prv_node->next;
    					}
    					else
    					{
    						memmove(prv_node->net_mem,prv_node->net_mem+copy_length,prv_node->data_count);
    						break;
    					}

    				}
    			}



    			unsigned char *file_add=current_head->net_mem;
    			file_add+= 10;
    			memcpy(command,file_add,3);			//命令字
			printf("COMMANDS: %x,%x,%x\n",command[0],command[1],command[2]);
			file_add+= 10;
    			int i=0;
    			while ((net_commands_pc_json[i].string)&&(net_cmd_compare(command,net_commands_pc_json[i].string) == 0))
    				i++;
    			unsigned char *cmdbuf=(unsigned char [40]){};
    			switch(net_commands_pc_json[i].id)
    			{
    			case RJSON_UPLOAD_C:
                {
                    app_debug(DBG_INFO,"919\n");
                    app_debug(DBG_INFO,"919 data:0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x \n",\
                    		*(file_add+0),*(file_add+1),*(file_add+2),*(file_add+3),*(file_add+4),*(file_add+5),\
                    		*(file_add+6),*(file_add+7),*(file_add+8));
                    net2proc_data.msg.data.net_cmd = RJSON_UPLOAD_C;
                    memcpy(&net2proc_data.msg.data.cmd,file_add,total_len-20);
                    net_runtime->send_to_proc(&net2proc_data,MSG_LEN_PROC);
                    net_runtime->connect_on = NET_STOP;
                    break;
                }

                default:{
    				net_runtime->connect_on = NET_STOP;
    				break;
    			}                    
    			}
    			// del_all_net_mem(&head.next);
    			current_node=&head;
    			//net_runtime->connect_on = NET_STOP;

    			if((new_head!=NULL)&&(use_addr==0)){
    				delete_list_mem(new_head);
    				new_head=NULL;
    			}
    			total_len=0;
    			memset(&head,0,sizeof(net_mem));
    		}
    	}

}

error:
	if((new_head!=NULL)&&(use_addr==0)){
		delete_list_mem(new_head);
	}
	close(net_runtime->socketfd);
	unregister_connect_node(net_runtime);
	app_debug(DBG_INFO,"TCP process thread for 18026 exited !\n");
return;
}


volatile int get_ip_flag=0;
static pthread_mutex_t ip_flag_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ip_addr_mutex=PTHREAD_MUTEX_INITIALIZER;
int is_get_ip()
{
	int flag=0;
    if(pthread_mutex_lock(&ip_flag_mutex)<0){
        app_debug(DBG_FATAL,"lock ip_flag_mutex failure!");
        return -1;
    }
    flag=get_ip_flag;
    pthread_mutex_unlock(&ip_flag_mutex);
    return flag;
}

void set_ip_flag()
{
    if(pthread_mutex_lock(&ip_flag_mutex)<0){
        app_debug(DBG_FATAL,"lock ip_flag_mutex failure!");
        return ;
    }
    get_ip_flag=1;
    pthread_mutex_unlock(&ip_flag_mutex);
}

void clr_ip_flag()
{
    if(pthread_mutex_lock(&ip_flag_mutex)<0){
    	 app_debug(DBG_FATAL,"lock ip_flag_mutex failure!");
    	 return;
    }
    get_ip_flag=0;
    pthread_mutex_unlock(&ip_flag_mutex);
    app_debug(DBG_INFO,"ip_flag=%d\n",get_ip_flag);
}

//同号呼叫功能
#ifdef HAVE_TONGHAO


typedef struct{
	int max_dev;
	int toatl_dev;
	int cur_pos;
	dev_arrary_t *arrary_head;
}dev_arrary_list_t;

pthread_rwlock_t dev_arrary_list_lock=PTHREAD_RWLOCK_INITIALIZER;

static dev_arrary_list_t com_dev_arrary;
static DevList null_dev;

/**
 *
 * udp 6672端口数据接收与处理函数
 * @return 返回0表示成功，-1表示失败
*/

int create_dev_arrary(void)
{
	int i=0;
	dev_arrary_t *cur=NULL,*new=NULL;

	com_dev_arrary.cur_pos=0;
	com_dev_arrary.max_dev=TONG_HAOIP_MAX*6;
	com_dev_arrary.toatl_dev=0;

	com_dev_arrary.arrary_head=(dev_arrary_t*)calloc(1,sizeof(dev_arrary_t));
	if(com_dev_arrary.arrary_head==NULL)
	{
		perror("calloca for dev_arrary_head failure:");
		return -1;
	}
	com_dev_arrary.arrary_head->next=NULL;
	com_dev_arrary.max_dev++;
	cur=com_dev_arrary.arrary_head;
	for(i=0;i<TONG_HAOIP_MAX*6;i++)
	{
		new=(dev_arrary_t*)calloc(1,sizeof(dev_arrary_t));
		new->next=NULL;
		cur->next=new;
		cur=cur->next;
		com_dev_arrary.max_dev++;
	}
	null_dev.DevNo[0]=0xef;
	null_dev.DevNo[1]=0xef;
	null_dev.DevNo[2]=0xef;
	null_dev.DevNo[3]=0xef;
	*(unsigned int *)null_dev.DevIp=INADDR_NONE;
	return 0;
}


void clear_com_dev_arrary(void)
{
	com_dev_arrary.cur_pos=0;
	com_dev_arrary.toatl_dev=0;
	if(pthread_rwlock_wrlock (&dev_arrary_list_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return ;
	}

	dev_arrary_t *cur=com_dev_arrary.arrary_head;

	int i=0;
	for(i=0;(i<com_dev_arrary.max_dev);i++)
	{
		if(cur!=NULL)
		{
			cur->dev=null_dev;
			cur=cur->next;
		}
	}
	pthread_rwlock_unlock(&dev_arrary_list_lock);
	return ;
}



/**
 *
 * 添加查询到的目标设备信息
 *  @param [in] dev ARP回复的设备信息
 * @return 返回0表示成功，-1表示失败
*/

int add_com_devlist(void *dev)
{
	if(dev==NULL)
	{
		fprintf(stderr,"add_com_devlist dev is NULL!");
		return -1;
	}
	if(pthread_rwlock_wrlock (&dev_arrary_list_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return -1;
	}

	unsigned int *newip_ptr,*cur_ptr;
	DevList *dev_ptr=(DevList*)dev;
	newip_ptr=(unsigned int *)dev_ptr->DevIp;

	dev_arrary_t *cur=com_dev_arrary.arrary_head;
	dev_arrary_t *prv=com_dev_arrary.arrary_head;

	int i=0;
	for(i=0;(i<com_dev_arrary.cur_pos)&&(i<com_dev_arrary.max_dev);i++)
	{
		if(cur!=NULL)
		{
			cur_ptr=(unsigned int *)cur->dev.DevIp;
			if(newip_ptr==cur_ptr)
			{
				pthread_rwlock_unlock(&dev_arrary_list_lock);
				fprintf(stderr,"ip is already in list!");
				return -1;
			}
			prv=cur;
			cur=cur->next;
		}
	}

	if(cur==NULL)
	{
		pthread_rwlock_unlock(&dev_arrary_list_lock);
		fprintf(stderr,"too many tong hao dev!\n");
		return -1;
	}

	if(cur!=NULL)
	{
		memcpy(&cur->dev,dev,sizeof(DevList));
		com_dev_arrary.cur_pos++;
		com_dev_arrary.toatl_dev++;
	}

	pthread_rwlock_unlock(&dev_arrary_list_lock);
	return 0;
}



 int get_com_ip_list_devno(DevList *dev_list,unsigned char *devno) //add by wrm 20150518
{
	int amount=0;
	if(pthread_rwlock_rdlock (&dev_arrary_list_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return -1;
	}

	dev_arrary_t*com_dev_list=com_dev_arrary.arrary_head;
	DevList*ptr=dev_list;
	int i=0;
	for(i=0;(i<TONG_HAOIP_MAX)&&(i<com_dev_arrary.cur_pos)&&(i<com_dev_arrary.max_dev);i++)
	{
     /* 
	printf("ip_list_devno i: %d ,amount: %d\n",i,amount);
	printf("ip_list_devno dev_no %02x%02x%02x%02x\t \n",devno[0],devno[1],devno[2],devno[3]);	
	printf("ip_list_devno dest_no  %02x%02x%02x%02x\t \n",com_dev_list ->dev.DevNo[0],com_dev_list ->dev.DevNo[1],com_dev_list ->dev.DevNo[2],com_dev_list ->dev.DevNo[3]);		
*/	
	    if(com_dev_list!=NULL && !memcmp (devno,com_dev_list ->dev.DevNo,2))//comment add by wrm 20151009 just cmp the first three byte for 区域管理机
		//if(com_dev_list!=NULL)
		{

			*ptr=com_dev_list->dev;
			com_dev_list=com_dev_list->next;
			ptr++;
			amount++;
		}
	}

	if((amount==1)&&(is_vlan_enable()))
	{
		uint32_t devip;
		memcpy(&devip,dev_list->DevIp,4);
		if(devip==INADDR_NONE)
		{
			amount=DEVNO_NOT_IN_LIST;
		}
	}	
	pthread_rwlock_unlock(&dev_arrary_list_lock);
	return amount;
}


/**
 *
 * ARP阶段获取回复IP信息的目标设备的IP地址
 * @param [out] com_dev_list 返回查询DevList链表头
 * @return 返回查询DevList链表元素数量，.
*/
 int get_com_ip_list(DevList *dev_list)
{
	int amount=0;
	if(pthread_rwlock_rdlock (&dev_arrary_list_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return -1;
	}

	dev_arrary_t*com_dev_list=com_dev_arrary.arrary_head;
	DevList*ptr=dev_list;
	int i=0;
	for(i=0;(i<TONG_HAOIP_MAX)&&(i<com_dev_arrary.cur_pos)&&(i<com_dev_arrary.max_dev);i++)
	{
		if(com_dev_list!=NULL)
		{
			*ptr=com_dev_list->dev;
			com_dev_list=com_dev_list->next;
			ptr++;
			amount++;
		}
	}
	
	if((amount==1)&&(is_vlan_enable()))
	{
		uint32_t devip;
		memcpy(&devip,dev_list->DevIp,4);
		if(devip==INADDR_NONE)
		{
			amount=DEVNO_NOT_IN_LIST;
		}
	}
	pthread_rwlock_unlock(&dev_arrary_list_lock);
	return amount;
}

/**
 *
 * ARP阶段获取回复IP信息的目标设备的IP地址
 * @param [in] dev_ip 建立通话的设备IP
*/
void set_com_dev_ip(void* dev_ip)
{
	if(pthread_rwlock_wrlock (&dev_arrary_list_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return;
	}

	unsigned int *cur_ptr;
	cur_ptr=(unsigned int*)com_dev_arrary.arrary_head->dev.DevIp;
	*cur_ptr=*(unsigned int *)dev_ip;
	pthread_rwlock_unlock(&dev_arrary_list_lock);
}

/**
 *
 * ARP阶段获取回复IP信息的目标设备的IP地址
 * @param [in] dev_ip 建立通话的设备IP
*/
void free_com_dev_ip(void* dev_ip)
{
	if(pthread_rwlock_wrlock (&dev_arrary_list_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return;
	}

	unsigned int *cur_ptr;
	cur_ptr=(unsigned int*)com_dev_arrary.arrary_head->dev.DevIp;
	*cur_ptr=*(unsigned int *)dev_ip;
	if(com_dev_arrary.max_dev>TONG_HAOIP_MAX)
	{
		int i=0;
		dev_arrary_t *cur=com_dev_arrary.arrary_head;
		for(i=0;i<com_dev_arrary.max_dev;i++)
		{
			if(cur!=NULL)
			{
				if(i>=(TONG_HAOIP_MAX-1))
				{
					dev_arrary_t *prv=cur;
					dev_arrary_t *last=cur;
					cur=cur->next;
					last->next=NULL;
					i++;
					for(;((i<com_dev_arrary.max_dev)&&(cur!=NULL));i++)
					{
						prv=cur;
						cur=cur->next;
						free(prv);
					}
					break;
				}
				cur=cur->next;
			}
		}
	}
	com_dev_arrary.toatl_dev=1;
	com_dev_arrary.cur_pos=1;

	pthread_rwlock_unlock(&dev_arrary_list_lock);
}


//#else
#endif



static DevList com_devlist;
static unsigned int ip_addr=0;

void set_com_devlist(unsigned char *list)
{
	if(pthread_mutex_lock(&ip_addr_mutex)<0){
		app_debug(DBG_FATAL,"lock ip_addr_mutex failure!");
		return ;
	}
	memcpy(&com_devlist,list,sizeof(DevList));
	printf("ip_addr=%x\n",ip_addr);
	pthread_mutex_unlock(&ip_addr_mutex);

}


/**
 *
 * ARP阶段获取回复IP信息的目标设备房号
 * @return 返回房号
*/
unsigned int get_com_devno(void)
{
	if(pthread_mutex_lock(&ip_addr_mutex)<0){
		app_debug(DBG_FATAL,"lock ip_addr_mutex failure!");
		return 0;
	}
	unsigned int devno=0;
	memcpy(&devno,com_devlist.DevNo,4);
	pthread_mutex_unlock(&ip_addr_mutex);
	return devno;

}

/**
 *
 * ARP阶段获取回复IP信息的目标设备的IP地址
 * @return 返回IP地址，失败返回-1.
*/
unsigned int get_ip_addr()
{
	unsigned int ip=0;
	if(pthread_mutex_lock(&ip_addr_mutex)<0){
		app_debug(DBG_FATAL,"lock ip_addr_mutex failure!");
		return -1;
	}
	memcpy(&ip,com_devlist.DevIp,4);
	pthread_mutex_unlock(&ip_addr_mutex);
	return ip;
}


static uint32_t inaddr_none=INADDR_NONE;

uint32_t vlanip_search_by_devno(const unsigned char dev_no[4],const int max_ip,uint32_t*ip_ret)
{
	return arp_search_vlanip_by_devno(dev_no,max_ip,ip_ret);
}

//#include <sys/time.h>


inline void vlan_arp_replay(int socketfd,const DevList *recv_devlist,struct sockaddr_in *client)
{
	unsigned char send_buf[32];
	send_buf[0]=ARP_REPLY;
	uint32_t ip_arrary[TONG_HAOIP_MAX]={INADDR_NONE};
	DevList  replay_list;
	int rett = 0, i = 0;
	vlan_list_t dst_data = {0,}, *tmp = NULL;
	
	memcpy(&replay_list,recv_devlist,sizeof(DevList));
	printf("recv_devlist->DevNo:");
	for(i=0;i<4;i++){
		printf("%02x ",recv_devlist->DevNo[i]);
	}
	printf("\n");
	
	memcpy(&dst_data.data.dev_no,(uint8_t*)recv_devlist->DevNo,4);
	printf("dst->data.dev_no = %08x\n",dst_data.data.dev_no);

	
		rett = query_vlan_list(&dst_data, 0);
		printf("do_load_vlan_list_data--ret=%d\n", rett);
		if(rett > 0) {

			tmp = dst_data.next;
	
			for(i=0; i<rett && tmp; i++) {
				/*printf("\n%08X ", tmp->data.dev_no);
				printf("%08X ", tmp->data.reserved1);
				printf("%08X ", tmp->data.reserved2);
				printf("%08X ", tmp->data.ipaddr);
				printf("%08X \r\n", tmp->data.reserved3);*/

				if(i<8)
					ip_arrary[i] = tmp->data.ipaddr;
					//memcpy(&ip_arrary[i],(uint32_t*)(dptr+16),4);
				
				tmp = tmp->next;
			}
		}
		for(i=0;i<8;i++)
			printf("ip_arrary[%d] = %08X\n",i,ip_arrary[i]);
	

	struct in_addr in_addr;
	in_addr.s_addr=ip_arrary[0];
	printf("###addr[0]:%s\n",inet_ntoa(in_addr));
	printf("###realret:%d\n",rett);
	if(rett>0)
	{
		int i=0;
		for(i=0;i<rett;i++)
		{
			memcpy(replay_list.DevIp,&ip_arrary[i],4);
			memcpy(&send_buf[1],&replay_list,sizeof(replay_list));
			client->sin_port =htons(UDP_PORT);
			send_data_to(socketfd,send_buf,sizeof(DevList)+1,(const struct sockaddr *)(client));
		}
	}
	else
	{
		memcpy(replay_list.DevIp,&inaddr_none,4);
		memcpy(&send_buf[1],&replay_list,sizeof(replay_list));
		client->sin_port =htons(UDP_PORT);
		send_data_to(socketfd,send_buf,sizeof(DevList)+1,(const struct sockaddr *)(client));
	}

//	printf("vlan_arp_replay------------time out-----\n");
}




/**
 *
 * udp 6672端口数据接收与处理函数
 * @param [in] socketfd udp　socket文件描述符
*/

void udp_process_task(int socketfd)
{
    app_debug(DBG_INFO,"udp_process_task starting!\n");
    int rec_len;
    unsigned char recv_buf[1024];

    int arp_rec_run=1;
    while(arp_rec_run)
    {
    	struct sockaddr_in client;
        rec_len=recv_data_from(socketfd,recv_buf,UDP_LEN,(struct sockaddr *)(&client));
    //	app_debug(DBG_INFO, "get rec_len=%d!!!!!!!\n",rec_len);
        if(rec_len>0)
        {
           switch(recv_buf[0])
            {
            unsigned char send_buf[32];
            case ARP_REQUEST:                  //****收到ARP请求
            {
                DevList recv_devlist;
                memcpy(&recv_devlist,&recv_buf[1],sizeof(recv_devlist));
				//printf("++++++++++++++++++++ruan\n");
				
                if(is_vlan_enable())
                {
	               	vlan_arp_replay(socketfd,&recv_devlist,&client);
                }
                else{
                if(is_call_me(recv_devlist.DevNo))          //是本机
                {
                	app_debug(DBG_INFO, "ARP_REQUEST!!!!!!!\n");
//同号呼叫功能添加
#ifdef  HAVE_TONGHAO
                	  int m_second=rand()%15;
                	  usleep(m_second*1000);
#endif

#if DEV_GATE|DEV_CONTROL|DEV_TERMINAL
                    send_buf[0]=ARP_REPLY;
#if DEV_GATE
                    my_devlist.DevNo[0]=gData.DoorNo[0];
                    my_devlist.DevNo[1]=gData.DoorNo[1];
                    my_devlist.DevNo[2]=0x00;
                    my_devlist.DevNo[3]=gData.DoorNo[2];
#elif DEV_CONTROL
                    my_devlist.DevNo[0]=dev_info->dev_No[0];
                    my_devlist.DevNo[1]=dev_info->dev_No[1];
                    my_devlist.DevNo[2]=dev_info->dev_No[2];
                    my_devlist.DevNo[3]=dev_info->dev_No[3];
#else //DEV_TERMINAL
                    my_devlist.DevNo[0]=gData.DoorNo[0];
                    my_devlist.DevNo[1]=gData.DoorNo[1];
                    my_devlist.DevNo[2]=gData.DoorNo[2];
                    my_devlist.DevNo[3]=gData.DoorNo[3];
#endif
                    memcpy(&send_buf[1],&my_devlist,sizeof(my_devlist));
                    client.sin_port =htons(UDP_PORT);
                    send_data_to(socketfd,send_buf,sizeof(DevList)+1,(const struct sockaddr *)(&client));
#else//DEV_GLJKZQ
                    net_addr_msg net2proc_addr;
                    net2proc_addr.flag_addr=GLJ_RQ_ADDR;
                    net2proc_addr.net_cmd=0;
                    net2proc_addr.data_len=0;
                    net2proc_addr.num=0;
                    net_mem *new_head=NULL;
                    int ret;
                    ret = request_list_head_mem(&new_head);
                    if (ret == 0) {
                    	struct sockaddr_in peeraddr;
                    	socklen_t addrlen=sizeof(struct sockaddr_in);
                    	getpeername(socketfd,(struct sockaddr *)&peeraddr,&addrlen);
                    	socket_addr_t ipaddr;
                    	ipaddr.addr=client.sin_addr.s_addr;
                    	ipaddr.port=UDP_PORT;
                    	memcpy(new_head->net_mem,&ipaddr,sizeof(socket_addr_t));
                    	new_head->data_count=sizeof(socket_addr_t);
                    	memcpy(&new_head->net_mem[new_head->data_count],&recv_buf[0],rec_len);
                    	new_head->data_count+=rec_len;
                    	net2proc_addr.data_len=new_head->data_count;
                    }
                    else
                    {
                    	app_debug(DBG_ERROR, "request_list_head_mem error!\n");
                    	break;
                    }
                    proc_msg net2proc_data;
                    net2proc_addr.list_num = new_head->mem_num;
                    net2proc_addr.net_cmd=0x992;
                    net2proc_data.msg_type=MSG_FROM_NET;
                    memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
                    //send_data_to_proc(&net2proc_data, MSG_LEN_PROC);
                    send_udp_data_to_ui(&net2proc_data, MSG_LEN_PROC);
#endif
                } else if(is_call_my_buliding(recv_devlist.DevNo)) {
                    net_addr_msg net2proc_addr;
                    net2proc_addr.flag_addr=GLJ_RQ_ADDR;
                    net2proc_addr.net_cmd=0;
                    net2proc_addr.data_len=0;
                    net2proc_addr.num=0;
                    net_mem *new_head=NULL;
                    int ret;
                    ret = request_list_head_mem(&new_head);
                    if (ret == 0)
                    {
                    	struct sockaddr_in peeraddr;
                    	socklen_t addrlen=sizeof(struct sockaddr_in);
                    	getpeername(socketfd,(struct sockaddr *)&peeraddr,&addrlen);
                    	socket_addr_t ipaddr;
                    	ipaddr.addr=client.sin_addr.s_addr;
                    	ipaddr.port=UDP_PORT;
                    	memcpy(new_head->net_mem,&ipaddr,sizeof(socket_addr_t));
                    	new_head->data_count=sizeof(socket_addr_t);
                    	memcpy(&new_head->net_mem[new_head->data_count],&recv_buf[0],rec_len);
                    	new_head->data_count+=rec_len;
                    	net2proc_addr.data_len=new_head->data_count;
                }
                    else
                    {
                    	fprintf(stderr, "request_list_head_mem error!\n");
                    	break;
                    }
                    proc_msg net2proc_data;
                    net2proc_addr.list_num = new_head->mem_num;
                    net2proc_addr.net_cmd=0x992;
                    net2proc_data.msg_type=MSG_FROM_NET;
                    memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
                  send_udp_data_to_ui(&net2proc_data, MSG_LEN_PROC);
                }
                	}
            }
                break;

            case ARP_REPLY:                 //****收到ARP回复
            {
            	app_debug(DBG_INFO, "get replay!!!!!!!\n");
//#if DEV_GATE|DEV_CONTROL|DEV_TERMINAL

#ifdef HAVE_TONGHAO
#if 0
            	DevList *dev_ptr=(DevList*)&recv_buf[1];
            	if((!is_mydev_ip(*((unsigned int*)dev_ptr->DevIp))&&\
            			(!is_get_ip()))&&(*(unsigned int*)dev_ptr->DevIp!=0))
            	{
            		add_com_devlist(&recv_buf[1]);
            	}
#else
            	//DevList *dev_ptr=(DevList*)&recv_buf[1];
            	DevList dev_ptr;
                memcpy(&dev_ptr, &recv_buf[1], sizeof(dev_ptr));

				if(!is_mydev_ip(*((unsigned int*)dev_ptr.DevIp)))//modify by wrm 20150716	不判断 (!is_get_ip())是为了防止上一个ip没有处理完，下一个ip就来了，导致没有接收到的情况
            	{
            		add_com_devlist(&recv_buf[1]);
            	}
#endif
#else

                if(!is_get_ip())
                {
                    set_com_devlist(&recv_buf[1]);
                    set_ip_flag();
                }
#endif
/*
#else//DEV_GLJKZQ
                net_addr_msg net2proc_addr;
                net2proc_addr.flag_addr=GLJ_RP_ADDR;
                net2proc_addr.net_cmd=0;
                net2proc_addr.data_len=rec_len;
                net2proc_addr.num=0;
                net_mem *new_head=NULL;
                int ret;
                ret = request_list_head_mem(&new_head);
                if (ret == 0) {
                	memcpy(new_head->net_mem,&recv_buf[0],rec_len);
                	new_head->data_count=rec_len;
                } else{
                	app_debug(DBG_ERROR, "request_list_head_mem error!\n");
                	break;
                }
                proc_msg net2proc_data;
                net2proc_addr.list_num = new_head->mem_num;
                net2proc_addr.net_cmd=0x993;
                net2proc_data.msg_type=MSG_FROM_NET;
                memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
 if               send_data_to_proc(&net2proc_data, MSG_LEN_PROC);
#endif
*/
            }
                break;
#if DEV_GLJKZQ|DEV_TSGLJKZQ
            case ARP_INFO_REQUEST:
            {
            	app_debug(DBG_INFO, "ARP_INFO_REQUEST!!!!!!!\n");
            	char DevNo[4],srcNo[4];
            	DevNo[0]=recv_buf[1];
            	DevNo[1]=recv_buf[2];
            	DevNo[2]=recv_buf[3];
            	DevNo[3]=recv_buf[4];
            	srcNo[0]=recv_buf[5];
            	srcNo[1]=recv_buf[6];
            	srcNo[2]=recv_buf[7];
            	srcNo[3]=recv_buf[8];
            	if(is_call_me(DevNo))          //是本机
            	{
            		my_devlist.DevNo[0]=gData.DoorNo[0];
            		my_devlist.DevNo[1]=gData.DoorNo[1];
            		my_devlist.DevNo[2]=0x00;
            		my_devlist.DevNo[3]=gData.DoorNo[2];
            		send_buf[0]=ARP_INFO_REPLY;
            		memcpy(&send_buf[1],&my_devlist,sizeof(my_devlist));
            		printf("my_devlist.DevStatus=%d\n",my_devlist.DevStatus);
            		client.sin_port =htons(UDP_PORT);
            		send_data_to(socketfd,send_buf,sizeof(DevList)+1,(const struct sockaddr *)(&client));
            	}
            }
            break;
            case ARP_INFO_REPLY:                 //ARP回复
            {
            	app_debug(DBG_INFO, "ARP_INFO_REPLY!!!!!!!\n");
            	net_addr_msg net2proc_addr;
            	net2proc_addr.flag_addr=GLJ_RP_ADDR;
            	net2proc_addr.net_cmd=0;
            	net2proc_addr.data_len=rec_len;
            	net2proc_addr.num=0;
            	net_mem *new_head=NULL;
            	int ret;
            	ret = request_list_head_mem(&new_head);
            	if (ret == 0) {
            		memcpy(new_head->net_mem,&recv_buf[0],rec_len);
            		new_head->data_count=rec_len;
            	} else{
            		app_debug(DBG_ERROR, "request_list_head_mem error!\n");
            		break;
            	}
            	proc_msg net2proc_data;
            	net2proc_addr.list_num = new_head->mem_num;
            	net2proc_data.msg_type=MSG_FROM_NET;
            	net2proc_addr.net_cmd=0x993;
            	memcpy(&net2proc_data.msg,&net2proc_addr,sizeof(net2proc_addr));
            	//send_data_to_proc(&net2proc_data, MSG_LEN_PROC);
            	send_udp_data_to_ui(&net2proc_data, MSG_LEN_PROC);
            }
            break;
#endif

            case TIME_ASK_REQUEST:
            {
                app_debug(DBG_INFO, "get TIME_ASK_REQUEST !!!!!!!\n");
#if DEV_GATE|DEV_GLJKZQ|DEV_TSGLJKZQ
                    my_devlist.DevNo[0]=gData.DoorNo[0];
                    my_devlist.DevNo[1]=gData.DoorNo[1];
                    my_devlist.DevNo[2]=0x00;
                    my_devlist.DevNo[3]=gData.DoorNo[2];
#elif DEV_CONTROL
                    my_devlist.DevNo[0]=dev_info->dev_No[0];
                    my_devlist.DevNo[1]=dev_info->dev_No[1];
                    my_devlist.DevNo[2]=dev_info->dev_No[2];
                    my_devlist.DevNo[3]=dev_info->dev_No[3];
#else // DEV_TERMINAL
                    my_devlist.DevNo[0]=gData.DoorNo[0];
                    my_devlist.DevNo[1]=gData.DoorNo[1];
                    my_devlist.DevNo[2]=gData.DoorNo[2];
                    my_devlist.DevNo[3]=gData.DoorNo[3];
#endif

                if(is_call_me(&recv_buf[3])) //add 20140430
                {
                    short Len = sizeof(DevList);
                    send_buf[0]= TIME_ASK_REPLY;
                    memcpy(&send_buf[1],&Len,2);
                    memcpy(&send_buf[3],&my_devlist,sizeof(DevList));
                    client.sin_port=htons(UDP_PORT);
                    send_data_to(socketfd,send_buf,Len+3,(const struct sockaddr *)(&client));
                }
            }
                break;
            case TIME_ASK_REPLY:
            {
                app_debug(DBG_INFO, "get TIME_ASK_REPLY !!!!!!!\n");
            }
                break;
            default:
                break;
            }
        }
    }
    app_debug(DBG_INFO,"udp_process_task closed!\n");
}


/**
 *
 * UDP广播目标房号查询目标IP
 * @param [in] dest_dev_No 四位目标房号
 * @param [in] req_dev_status 请求状态
 * @param [out] ip返回查询到的IP
 * @return 成功返回IP总数量，失败返回0
*/
#ifdef HAVE_TONGHAO
//pthread_mutex_t  arp_lock=PTHREAD_MUTEX_INITIALIZER;
int get_dest_ip_by_arp_vlan(unsigned char *dest_dev_No,DevList *dev, signed long req_dev_status,unsigned int addr_way)
{
	DevList dest_devlist;
	int amount=0,loop=0;;
	unsigned char udp_buf[40];
	memset(udp_buf,0x00,40);
	udp_buf[0]=ARP_REQUEST;
	memcpy(dest_devlist.DevNo,dest_dev_No,4);
#if DEV_GATE|DEV_GLJKZQ|DEV_TSGLJKZQ
	dest_devlist.DevIp[0]=gData.DoorNo[0];
	dest_devlist.DevIp[1]=gData.DoorNo[1];
	dest_devlist.DevIp[2]=0x00;
	dest_devlist.DevIp[3]=gData.DoorNo[2];

#elif DEV_CONTROL
	dest_devlist.DevIp[0]=dev_info->dev_No[0];
	dest_devlist.DevIp[1]=dev_info->dev_No[1];
	dest_devlist.DevIp[2]=dev_info->dev_No[2];
	dest_devlist.DevIp[3]=dev_info->dev_No[3];

#else //DEV_TERMINAL
	dest_devlist.DevIp[0]=gData.DoorNo[0];
	dest_devlist.DevIp[1]=gData.DoorNo[1];
	dest_devlist.DevIp[2]=gData.DoorNo[2];
	dest_devlist.DevIp[3]=gData.DoorNo[3];
#endif
	dest_devlist.DevType=my_devlist.DevType;//0x03;
	dest_devlist.DevStatus=req_dev_status;
	dest_devlist.Default=my_devlist.Default;

    lock_pthread_mutex_lock(&lock_for_ip_search);//增加此锁以避免其他线程调用
	memcpy(&udp_buf[1],&dest_devlist,sizeof(dest_devlist));
	clear_com_dev_arrary();
	clr_ip_flag();

	send_data_to_udp_node(udp_buf,sizeof(dest_devlist)+1,addr_way,UDP_PORT);
	loop=10*ARP_RETYR_TIMES;
	do
	{
		usleep(ARP_DELAY);
		amount=get_com_ip_list_devno(dev,dest_dev_No);
		if((amount>0)||(amount==(DEVNO_NOT_IN_LIST)))
		{
            unlock_pthread_mutex_lock(&lock_for_ip_search);
			return amount;
		}
	}while(loop--);
	amount=get_com_ip_list_devno(dev,dest_dev_No);
	unlock_pthread_mutex_lock(&lock_for_ip_search);
	return amount;
}

int get_dest_ip_by_arp(unsigned char *dest_dev_No,DevList *dev, signed long req_dev_status,unsigned short addr_way)
{
	DevList dest_devlist;
	int amount=0,try=0,loop=0;
	unsigned char udp_buf[40];
	memset(udp_buf,0x00,40);
	udp_buf[0]=ARP_REQUEST;
	memcpy(dest_devlist.DevNo,dest_dev_No,4);
#if DEV_GATE|DEV_GLJKZQ|DEV_TSGLJKZQ
	dest_devlist.DevIp[0]=gData.DoorNo[0];
	dest_devlist.DevIp[1]=gData.DoorNo[1];
	dest_devlist.DevIp[2]=0x00;
	dest_devlist.DevIp[3]=gData.DoorNo[2];

#elif DEV_CONTROL
	dest_devlist.DevIp[0]=dev_info->dev_No[0];
	dest_devlist.DevIp[1]=dev_info->dev_No[1];
	dest_devlist.DevIp[2]=dev_info->dev_No[2];
	dest_devlist.DevIp[3]=dev_info->dev_No[3];

#else //DEV_TERMINAL
	dest_devlist.DevIp[0]=gData.DoorNo[0];
	dest_devlist.DevIp[1]=gData.DoorNo[1];
	dest_devlist.DevIp[2]=gData.DoorNo[2];
	dest_devlist.DevIp[3]=gData.DoorNo[3];
#endif
	dest_devlist.DevType=my_devlist.DevType;//0x03;
	dest_devlist.DevStatus=req_dev_status;
	dest_devlist.Default=my_devlist.Default;
	
    lock_pthread_mutex_lock(&lock_for_ip_search);//增加此锁以避免其他线程调用
	memcpy(&udp_buf[1],&dest_devlist,sizeof(dest_devlist));
	clear_com_dev_arrary();
	clr_ip_flag();


	do
	{
		send_data_to_udp_node(udp_buf,sizeof(dest_devlist)+1,get_boardcast_ip(),UDP_PORT);
		loop=10;
		do{
			usleep(ARP_DELAY);
			if((amount=get_com_ip_list_devno(dev,dest_dev_No))>0)
//get_com_ip_list(dev) modify by wrm 20150518 for 在留言最后时刻与发送刷卡记录冲突，留言的目标ip会变成管理机ip
//get_com_ip_list_devno是wrm新加入的，主要添加针对目标房号的验证，如果不匹配将会一直while循环中
			{
				//break;
				unlock_pthread_mutex_lock(&lock_for_ip_search);
				return amount;
			}
		}while(loop--);
		try++;
	}while(try<ARP_RETYR_TIMES);
	//amount=get_com_ip_list(dev);

	unlock_pthread_mutex_lock(&lock_for_ip_search);
	return amount;
}

#else

int get_dest_ip_by_arp(unsigned char *dest_dev_No,char *ip, signed long req_dev_status,unsigned short addr_way)
{
    DevList dest_devlist;
    int i=10;
    int time=0;
    do
    {
        i=10;
        unsigned char udp_buf[40];
        memset(udp_buf,0x00,40);
        udp_buf[0]=ARP_REQUEST;
        memcpy(dest_devlist.DevNo,dest_dev_No,4);
#if DEV_GATE|DEV_GLJKZQ|DEV_TSGLJKZQ
        dest_devlist.DevIp[0]=gData.DoorNo[0];
        dest_devlist.DevIp[1]=gData.DoorNo[1];
        dest_devlist.DevIp[2]=0x00;
        dest_devlist.DevIp[3]=gData.DoorNo[2];

#elif DEV_CONTROL
        dest_devlist.DevIp[0]=dev_info->dev_No[0];
        dest_devlist.DevIp[1]=dev_info->dev_No[1];
        dest_devlist.DevIp[2]=dev_info->dev_No[2];
        dest_devlist.DevIp[3]=dev_info->dev_No[3];

#else //DEV_TERMINAL
        dest_devlist.DevIp[0]=gData.DoorNo[0];
        dest_devlist.DevIp[1]=gData.DoorNo[1];
        dest_devlist.DevIp[2]=gData.DoorNo[2];
        dest_devlist.DevIp[3]=gData.DoorNo[3];
#endif
        dest_devlist.DevType=my_devlist.DevType;//0x03;
        dest_devlist.DevStatus=req_dev_status;
        dest_devlist.Default=my_devlist.Default;

        memcpy(&udp_buf[1],&dest_devlist,sizeof(dest_devlist));
        clr_ip_flag();
        send_data_to_udp_node(udp_buf,sizeof(dest_devlist)+1,get_boardcast_ip(),UDP_PORT);
        //send_data_to_udp_node(udp_buf,sizeof(dest_devlist)+1,INADDR_BROADCAST,UDP_PORT);
        while(i--)
        {
            if(is_get_ip())
            {
                if(ip!=NULL)
                {
                	unsigned int ip_addr=get_ip_addr();
                	if(ip_addr>0)
                   memcpy(ip,&ip_addr,4);
                	else
                		return -1;
                }
                return 1;
            }
            usleep(ARP_DELAY);
        }
        time++;
    }while(time<ARP_RETYR_TIMES);
    app_debug(DBG_INFO,"boardcast times =%d\n",time);
    return -1;
}

#endif

#ifdef HAVE_TONGHAO
//add for multi_lift_ctl
int send_lift_ctrl_msg_tcp_process(net_data_msg *msg,unsigned int net_cmd,unsigned int ip_addr)
{
	char cmd_pc[4];
	int try = 0,ret = 0;
	tcp_data *tcp_buf = NULL;	
	int items = ITEM_ONE;
		
	app_debug(DBG_INFO,"ip_addr=%08x\n",ip_addr);

    do{
        try = 0;
        do {
        	if (try > 3){
        		if(ret<0){
        			app_debug(DBG_ERROR, "request tcp_buf\n");
        			return -1;
        		}
        		else
        			break;
        	}
        	ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
        	try++;
        }while ((ret < 0) && (NULL == tcp_buf));

        if((tcp_buf==NULL)||(ret<0))
        	break;
        sprintf(cmd_pc,"%x",net_cmd);
        tcp_buf->length =build_net_data(tcp_buf->buf,(char *)msg->buf,msg->data_len,cmd_pc);
        tcp_buf->dest_ip=ip_addr;
        items = ITEM_ONE;
        ret = send_tcp_data(&items,TCP_TML);
        if (ret < 0) {
        	app_debug(DBG_INFO,"tcp_send_data  <918> err");
        	break;
        }

	}while(0);

	return ret;
}


/**
 *
 * 网络处理UI发送过来的数据
 * @param [in] buf UI发送过来的数据
 * @param [in] len 数据长度
*/


void ui_data_pro_thread_cache(char *buf,int len)//处理接收到的UI数据
{
	unsigned char *dest_No=(unsigned char [4]){};
	unsigned char *src_No = (unsigned char[4]){};
	//dev_arrary_t *th_addr;
	DevList th_addr_array[TONG_HAOIP_MAX];
	unsigned int net_cmd;
	unsigned char cmd;
	short allow=0;
	unsigned int oper_type;
	int dev_type=TYPE_JDM365;
	int resstatus;
       int ret = 0;
       int ret_ip=0;
	char flag_addr=MEM_DATA;
	proc_msg_ret msg_ret={};
	msg_ret.msg_type=MSG_FROM_NET;
	oper_type = 0x00;
	resstatus = 0x00;

	ajb_msg *msg=(ajb_msg *)buf;

	if(msg->data.net_cmd>0xfff)
		goto data_err;

	memset(th_addr_array, 0x00, sizeof(th_addr_array));
	dbg_inf("%s:net_cmd=%02x,cmd=%02x,pack_type=0x%02x\n",__func__,msg->data.net_cmd,msg->data.cmd,buf[0]);

	switch(buf[0])
	{
	case MEM_GLJ:
	{
		switch(msg->data.net_cmd)
		{
		case GDATC:
		{
               app_debug(DBG_INFO,"ui__518\n");
               if(is_have_infostore()) {
                   switch(msg->data.cmd) {
                       case SKEY:
                       case SSCS:
                       case SSIC:
                       case SDIS://增加
                       {
                               net_cmd=msg->data.net_cmd;
                               oper_type=msg->data.oper_type;
                               char cmd_pc[4];
                               sprintf(cmd_pc,"%x",msg->data.net_cmd);
                               int path=TCP_TML;
                               memcpy(dest_No,msg->data.dest_No,4);

                               unsigned char *dest_No_2=(unsigned char [4]){};
                               memcpy(dest_No_2,dest_No,4);
                               short int fuhao=get_code_from_container_data((char *)&dest_No_2[2]);
                               if(fuhao>0)
                               {
                                       dest_No_2[2]=0;
                                       dest_No_2[3]=fuhao&0xff;
                               }
                               app_debug(DBG_INFO,"dest_No_2[3]=%d\n",dest_No_2[3]);
#if 0  //modify by wrm 20150511 for 刷卡撤防，一共改了4处，主要作用有两点

                               ret = get_dest_ip_by_arp(dest_No_2,th_addr_array, ARPINFOREQ,0);
                               if (ret < 0) {
                                       app_debug(DBG_INFO,"dest dev is not online!\n");
                                       goto err_get_ip;
                               }
#else    // 1.在有信息存储器且跨网段时加入对跨网段的支持
			if(is_vlan_enable())
			{
				printf("##ssic vlan_enable\n");
				uint32_t gate_ip;
				unsigned char dest_No_2[4];
				memcpy(dest_No_2,dest_No,4);
				memcpy(&gate_ip,gData.addr_gw,4);

				if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
				{
					ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
							       printf("dest_ip is %d\n",th_addr_array[0].DevIp[0]);
				}
				//大门口机 管理机 终端呼叫分机才进行if里面的动作,门口机应直接在if中返回失败
				if (ret == DEVNO_NOT_IN_LIST)
				{
					dest_No_2[2]=0;
					dest_No_2[3]=0;
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
					}
					if (ret<=0)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
				}
				else if(ret == 0)
				{
					get_dns1((unsigned char*)&gate_ip);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
					}
					//设备回复房号不在列表
					if (ret == DEVNO_NOT_IN_LIST)
					{
						/******************大门口机 管理机 终端呼叫分机才进行此段的动作,门口机应直接返回失败*************************/
						dest_No_2[2]=0;
						dest_No_2[3]=0;
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
						}
						/******************大门口机 管理机 终端呼叫分机才进行此段的动作,门口机应直接返回失败*************************/
						if (ret<=0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
					else if(ret ==0)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
				}
			}else{
				ret=get_dest_ip_by_arp(dest_No_2,th_addr_array,ARPCALLREQ,0);
				if (ret <= 0) {
					app_debug(DBG_INFO,"dest dev is not online!\n");
					goto err_get_ip;
					}
				}				
#endif
#if 0
                               char data[10]={};
                               data[0]=msg->data.cmd;
                               int data_len=msg->data.src_No[0];
                               memcpy(&data[1],&msg->data.src_No[1],data_len);
                               int try = 0;
                               tcp_data *tcp_buf = NULL;
                               try = 0;
                               do {
                                       if (try > 3)
                                               break;
                                       ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
                                       try++;
                               } while ((ret < 0) && (NULL == tcp_buf));
                               if((tcp_buf==NULL)||(ret<0))
                                       goto err_get_ip;
                               tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);
                               tcp_buf->dest_ip=*((unsigned int*)th_addr_array[0].DevIp);
			       printf("tcp_buf->dest_ip is %u\n",tcp_buf->dest_ip);							   
                               int items = ITEM_ONE;
                               ret = send_tcp_data(&items,path);
                               if (ret < 0) {
                                       app_debug(DBG_INFO,"tcp_send_data  <%s> err",cmd_pc);
                                       goto err_req;
                               }
                               set_ip_flag();
#else //  2.对获得的ip地址进行循环并发送tcp包
				clear_tonghaoip_from_call_list(dest_No);
				int i=0;
				int loop=0;
				lock_ntelc_cache_lock();
				netlc_arp=1;
				unlock_ntelc_cache_lock();
				int timedelay=40;//接收3秒内回复的数据
				do{

					if((loop=get_com_ip_list(th_addr_array))>0)
					{
						for(i=0;i<loop;i++)
						{
							if((th_addr_array[i].DevNo[0]==dest_No[0])&&(th_addr_array[i].DevNo[1]==dest_No[1]))
							{
								add_call_item(dest_No,*((unsigned int*)th_addr_array[i].DevIp));
							}
						}
					}
					else
					{
						usleep(ARP_DELAY);
						continue;
					}

					ip_tongh_t ip_addr[TONG_HAOIP_MAX];
   				memset(ip_addr, 0x00, sizeof(ip_addr));
					loop=get_iplist_from_call_list(dest_No,SEND_NC,ip_addr);
					for(i=0;(i<loop)&&(netlc_arp>0);i++)
					{
						char data[10]={};
						data[0]=msg->data.cmd;
						int data_len=msg->data.src_No[0];
						memcpy(&data[1],&msg->data.src_No[1],data_len);
						int try = 0;
						tcp_data *tcp_buf = NULL;
						try = 0;
						do {
							if (try > 3)
								break;
							ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
							try++;
						} while ((ret < 0) && (NULL == tcp_buf));
						if((tcp_buf==NULL)||(ret<0))
							goto err_get_ip;
						tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);
						tcp_buf->dest_ip=ip_addr[i].ip_addr;
						int items = ITEM_ONE;
						lock_ntelc_cache_lock();
						if(netlc_arp<=0)
						{
							unlock_ntelc_cache_lock();
							break;
						}
						else
						{
							ret = send_tcp_data(&items,path);
							if (ret < 0) {
								fprintf(stderr,"tcp_send_data  <%s> err",cmd_pc);
								unlock_ntelc_cache_lock();
								goto err_req;
							}
						}
						unlock_ntelc_cache_lock();
						set_dest_dev_status(dest_No,tcp_buf->dest_ip,TONGHAO_CALLON);
					}

				}while((timedelay--)&&(netlc_arp>0));
				set_ip_flag();
				lock_ntelc_cache_lock();
				netlc_arp=0;
				unlock_ntelc_cache_lock();
#endif
                           }
                           break;
                           
                           default:
                           {
                                   net_cmd=msg->data.net_cmd;
                                   oper_type=msg->data.oper_type;
                                   char cmd_pc[4];
                                   sprintf(cmd_pc,"%x",msg->data.net_cmd);
                                   int path=TCP_TML;
                                   memcpy(dest_No,msg->data.dest_No,4);
                                   unsigned char *dest_No_2=(unsigned char [4]){};
                              	   memcpy(dest_No_2,dest_No,4);								   
#if 0								   
                                   ret = get_dest_ip_by_arp(dest_No,th_addr_array, ARPINFOREQ,0);
                                   if (ret < 0) {
                                           app_debug(DBG_INFO,"dest dev is not online!\n");
                                           goto err_get_ip;
                                   }
#else
				if(is_vlan_enable())
				{
					uint32_t gate_ip;
					//unsigned char dest_No_2[4];
					//memcpy(dest_No_2,dest_No,4);
					memcpy(&gate_ip,gData.addr_gw,4);

					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
					}
					//大门口机 管理机 终端呼叫分机才进行if里面的动作,门口机应直接在if中返回失败
					if (ret == DEVNO_NOT_IN_LIST)
					{
						dest_No_2[2]=0;
						dest_No_2[3]=0;
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
						}
						if (ret<=0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
					else if(ret == 0)
					{
						get_dns1((unsigned char*)&gate_ip);
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
						}
						//设备回复房号不在列表
						if (ret == DEVNO_NOT_IN_LIST)
						{
							/******************大门口机 管理机 终端呼叫分机才进行此段的动作,门口机应直接返回失败*************************/
							dest_No_2[2]=0;
							dest_No_2[3]=0;
							if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
							{
								ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
							}
							/******************大门口机 管理机 终端呼叫分机才进行此段的动作,门口机应直接返回失败*************************/
							if (ret<=0)
							{
								fprintf(stderr,"vlan: dest dev is not online!\n");
								unlock_pthread_mutex_lock(&arp_lock);
								goto err_get_ip;
							}
						}
						else if(ret ==0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
				}else{
					ret=get_dest_ip_by_arp(dest_No_2,th_addr_array,ARPCALLREQ,0);
					if (ret <= 0) {
						app_debug(DBG_INFO,"dest dev is not online!\n");
						goto err_get_ip;
						}
					}				
#endif
#if 0
                                   char data[10]={};
                                   data[0]=msg->data.cmd;
                                   int data_len=msg->data.src_No[0];
                                   memcpy(&data[1],&msg->data.src_No[1],data_len);
                                   int try = 0;
                                   tcp_data *tcp_buf = NULL;
                                   try = 0;
                                   do {
                                           if (try > 3)
                                                   break;
                                           ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
                                           try++;
                                   } while ((ret < 0) && (NULL == tcp_buf));
                                   if((tcp_buf==NULL)||(ret<0))
                                           goto err_get_ip;
                                   tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);
                                   tcp_buf->dest_ip=*((unsigned int*)th_addr_array[0].DevIp);
                                   int items = ITEM_ONE;
                                   ret = send_tcp_data(&items,path);
                                   if (ret < 0) {
                                           app_debug(DBG_INFO,"tcp_send_data  <%s> err",cmd_pc);
                                           goto err_req;
                                   }
                                   set_ip_flag();
#else								   
				clear_tonghaoip_from_call_list(dest_No);
				int i=0;
				int loop=0;
				lock_ntelc_cache_lock();
				netlc_arp=1;
				unlock_ntelc_cache_lock();
				int timedelay=40;//接收3秒内回复的数据
				do{

					if((loop=get_com_ip_list(th_addr_array))>0)
					{
						for(i=0;i<loop;i++)
						{
							if((th_addr_array[i].DevNo[0]==dest_No[0])&&(th_addr_array[i].DevNo[1]==dest_No[1]))
							{
								add_call_item(dest_No,*((unsigned int*)th_addr_array[i].DevIp));
							}
						}
					}
					else
					{
						usleep(ARP_DELAY);
						continue;
					}

					ip_tongh_t ip_addr[TONG_HAOIP_MAX];
   				memset(ip_addr, 0x00, sizeof(ip_addr));
					loop=get_iplist_from_call_list(dest_No,SEND_NC,ip_addr);
					for(i=0;(i<loop)&&(netlc_arp>0);i++)
					{
						char data[10]={};
						data[0]=msg->data.cmd;
						int data_len=msg->data.src_No[0];
						memcpy(&data[1],&msg->data.src_No[1],data_len);
						int try = 0;
						tcp_data *tcp_buf = NULL;
						try = 0;
						do {
							if (try > 3)
								break;
							ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
							try++;
						} while ((ret < 0) && (NULL == tcp_buf));
						if((tcp_buf==NULL)||(ret<0))
							goto err_get_ip;
						tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);
						tcp_buf->dest_ip=ip_addr[i].ip_addr;
						int items = ITEM_ONE;
						lock_ntelc_cache_lock();
						if(netlc_arp<=0)
						{
							unlock_ntelc_cache_lock();
							break;
						}
						else
						{
							ret = send_tcp_data(&items,path);
							if (ret < 0) {
								fprintf(stderr,"tcp_send_data  <%s> err",cmd_pc);
								unlock_ntelc_cache_lock();
								goto err_req;
							}
						}
						unlock_ntelc_cache_lock();
						set_dest_dev_status(dest_No,tcp_buf->dest_ip,TONGHAO_CALLON);
					}

				}while((timedelay--)&&(netlc_arp>0));
				set_ip_flag();
				lock_ntelc_cache_lock();
				netlc_arp=0;
				unlock_ntelc_cache_lock();
#endif								   
                           }
                           break;
                        }
                    } else {
			
			switch(msg->data.cmd)
			{
			case SSCS:
                       case SSIC:
			{
				net_cmd=msg->data.net_cmd;
				oper_type=msg->data.oper_type;
				char cmd_pc[4];
				sprintf(cmd_pc,"%x",msg->data.net_cmd);
				int path=TCP_TML;
				memcpy(dest_No,msg->data.dest_No,4);

				
				if(is_vlan_enable())
				{
					uint32_t gate_ip;
					unsigned char dest_No_2[4];
					memcpy(dest_No_2,dest_No,4);
					memcpy(&gate_ip,gData.addr_gw,4);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
					}
					if (ret == DEVNO_NOT_IN_LIST)
					{
						dest_No_2[2]=0;
						dest_No_2[3]=0;
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
						}
						if (ret<=0)
						{
							fprintf(stderr,"vlan: dest fenji is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
					else if(ret == 0)
					{
						get_dns1((unsigned char*)&gate_ip);
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
						}
						if (ret == DEVNO_NOT_IN_LIST)
						{
							dest_No_2[2]=0;
							dest_No_2[3]=0;
							if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
							{
								ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
							}
							if (ret<=0)
							{
								fprintf(stderr,"vlan: dest fenji is not online!\n");
								unlock_pthread_mutex_lock(&arp_lock);
								goto err_get_ip;
							}
						}
						else if(ret ==0)
						{
							fprintf(stderr,"vlan: dest fenji is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
				}
				else
				{
                    dbg_lo("-------------\n");
					ret = get_dest_ip_by_arp(dest_No,th_addr_array, ARPINFOREQ,0);
					if (ret <= 0) {
						dbg_warn("dest fenji is not online!\n");
						goto err_get_ip;
					}
				}
				clear_tonghaoip_from_call_list(dest_No);
				int i=0;
				int loop=0;
                               set_netlc_arp_value(1);
				int timedelay=40;//接收3秒内回复的数据
				do{

					if((loop=get_com_ip_list(th_addr_array))>0)
					{
						for(i=0;i<loop;i++)
						{
							if((th_addr_array[i].DevNo[0]==dest_No[0])&&(th_addr_array[i].DevNo[1]==dest_No[1]))
							{
								add_call_item(dest_No,*((unsigned int*)th_addr_array[i].DevIp));
							}
						}
					}
					else
					{
						usleep(ARP_DELAY);
						continue;
					}

					ip_tongh_t ip_addr[TONG_HAOIP_MAX];
   				       memset(ip_addr, 0x00, sizeof(ip_addr));
					loop=get_iplist_from_call_list(dest_No,SEND_NC,ip_addr);
                                 //printf("loop=======%d\n",loop);
					for(i=0;(i<loop)&&(netlc_arp>0);i++)
					{
						char data[10]={};
						data[0]=msg->data.cmd;
						int data_len=msg->data.src_No[0];
						memcpy(&data[1],&msg->data.src_No[1],data_len);
						int try = 0;
						tcp_data *tcp_buf = NULL;
						try = 0;
						do {
							if (try > 3)
								break;
							ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
							try++;
						} while ((ret < 0) && (NULL == tcp_buf));
						if((tcp_buf==NULL)||(ret<0))
							goto err_get_ip;
						tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);
						tcp_buf->dest_ip=ip_addr[i].ip_addr;
						int items = ITEM_ONE;
						lock_ntelc_cache_lock();
						if(netlc_arp<=0)
						{
							unlock_ntelc_cache_lock();
							break;
						}
						else
						{
							ret = send_tcp_data(&items,path);
							if (ret < 0) {
								fprintf(stderr,"tcp_send_data  <%s> err",cmd_pc);
								unlock_ntelc_cache_lock();
								goto err_req;
							}
						}
						unlock_ntelc_cache_lock();
						set_dest_dev_status(dest_No,tcp_buf->dest_ip,TONGHAO_CALLON);
					}

				}while((timedelay--)&&(netlc_arp>0));
				set_ip_flag();
                               set_netlc_arp_value(0);
			}
			break;
			

			}
		}
		}
		break;
		}
	}
	break;
	case MEM_DATA:
	{
		if (msg->data.cmd == 0x88) { //返回状态
			ajb_msg_ret_split_whole(&net_cmd, &cmd, &allow, &oper_type, &resstatus,&dev_type,&flag_addr,(ajb_msg_ret*)msg);
		} else
		{
			ajb_msg_split_whole(&net_cmd, &cmd, &dest_No, &src_No,&oper_type,&resstatus,&flag_addr,msg);
		}
		flag_addr=MEM_DATA;
		switch (net_cmd) {
		case NTELC:
		{
                int try=0;
                hold_on=0;
                try_call=0;
                int timedelay=30;//接收3秒内回复的数据
                ntel_data call_data;
                unsigned char *dest_No_2=(unsigned char [4]){};
                memcpy(dest_No_2, dest_No, 4);
                if (is_have_infostore()) {
                       short int fuhao=get_code_from_container_data((char *)&dest_No_2[2]);
                       if(fuhao>0)
                       {
                           dest_No_2[2]=0;
                           dest_No_2[3]=fuhao&0xff;
                           printf("^^^^^^is_have_infostore:%d \n", dest_No_2[3]);
                       }
			}

			
			if(is_vlan_enable())
			{
				uint32_t gate_ip;
				unsigned char dest_No_2[4];
				memcpy(dest_No_2,dest_No,4);
				memcpy(&gate_ip,gData.addr_gw,4);

				if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
				{
					ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
				}
				//大门口机 管理机 终端呼叫分机才进行if里面的动作,门口机应直接在if中返回失败
				if (ret == DEVNO_NOT_IN_LIST)
				{
					dest_No_2[2]=0;
					dest_No_2[3]=0;
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
					}
					if (ret<=0)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
				}
				else if(ret == 0)
				{
					get_dns1((unsigned char*)&gate_ip);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
					}
					//设备回复房号不在列表
					if (ret == DEVNO_NOT_IN_LIST)
					{
						/******************大门口机 管理机 终端呼叫分机才进行此段的动作,门口机应直接返回失败*************************/
						dest_No_2[2]=0;
						dest_No_2[3]=0;
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
						}
						/******************大门口机 管理机 终端呼叫分机才进行此段的动作,门口机应直接返回失败*************************/
						if (ret<=0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
					else if(ret ==0)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
				}
			}
			else
			{
				ret=get_dest_ip_by_arp(dest_No_2,th_addr_array,ARPCALLREQ,0);
				printf("get_dest_ip_by_arp 704, ret=%d\n",ret);
#ifdef FRONT_DOOR
				if (ret <= 0)
				{
					unsigned char *dest_No_2=(unsigned char [4]){};
					memcpy(dest_No_2,dest_No,4);
					dest_No_2[2]=0;
					dest_No_2[3]=0;
					ret=get_dest_ip_by_arp(dest_No_2,th_addr_array,ARPCALLREQ,0);
					if (ret <=0)
					{
						fprintf(stderr,"dest dev is not online!\n");
						goto err_get_ip;
					} else {
					    unsigned int real_dest = 0;
					    memcpy(&real_dest, dest_No_2, 4);
		                ajb_msg_ret_build(&msg_ret.msg, net_cmd, 0x87, 0, real_dest,ret,flag_addr);
		                send_data_to_proc(&msg_ret, MSG_LEN_PROC);
					}
				}
#endif
			}
			clear_tonghaoip_from_call_list(dest_No);
			int i=0;
			int loop=0;
			set_netlc_arp_value(1);
			do{
				if((loop=get_com_ip_list(th_addr_array))>0)
				{
					for(i=0;i<loop;i++)
					{
						if((th_addr_array[i].DevNo[0]==dest_No[0])&&(th_addr_array[i].DevNo[1]==dest_No[1]))
						{
							add_call_item(dest_No,*((unsigned int*)th_addr_array[i].DevIp));
						}
					}
				}
				else
				{
					usleep(ARP_DELAY);
					continue;
				}
				ip_tongh_t ip_addr[TONG_HAOIP_MAX];
				memset(ip_addr, 0x00, sizeof(ip_addr));
				loop=get_iplist_from_call_list(dest_No,SEND_NC,ip_addr); //20140122
                           if((loop==1)&&(*(unsigned int*)dest_No==0))
				{
					if((ip_addr[0].ip_addr==0))
					{
						ajb_msg_ret_build(&msg_ret.msg, RTELC, 0x99, -1, oper_type,ret,flag_addr);
						send_data_to_proc(&msg_ret, MSG_LEN_PROC);
						set_netlc_arp_value(0);
						set_ip_flag();
						clear_com_dev_arrary();
						return ;
					}
				}
				for(i=0;(i<loop)&&(netlc_arp>0);i++)
				{
                              if((th_addr_array[i].DevNo[0]==0)&&(th_addr_array[i].DevNo[1]==0))
                                {//若呼通的是管理机，只需要发一次704，防止与518的arp冲突
                                loop = 1;
                                timedelay = 1;
                                }
					tcp_data *buf=NULL;
					try=0;
					do{
						if(try>3)
						{
							break;
						}
						ret=request_send_buf(&buf,NET_BUF_SIZE,TCP_TML);
						try++;
					}while((ret<0)&&(NULL==buf));

					if((buf==NULL)||(ret<0))
					{
						break;
					}
					memcpy(call_data.dest_addr,dest_No,4);
					memcpy(call_data.src_addr,src_No,4);
					call_data.oper_type=oper_type;
					call_data.resstaus=resstatus;
					buf->length=build_call_data_j365(buf->buf,&call_data,(unsigned char*)"704");
					buf->dest_ip=ip_addr[i].ip_addr;
					int items=ITEM_ONE;
					if(netlc_arp>0)
					{
						ret=send_tcp_data(&items,TCP_TML);
						if(ret<0)
						{
							fprintf(stderr,"tcp_send_data <704> err");
							break;
						}
					}
					set_dest_dev_status(dest_No,buf->dest_ip,TONGHAO_CALLON);
				}
				usleep(ARP_DELAY);
			}while((timedelay--)&&(netlc_arp>0));
			set_netlc_arp_value(0);
			set_ip_flag();
			oper_type=*((unsigned int*)dest_No);
			clear_com_dev_arrary();
			printf("net_data_pro 704 return\n");
		}
		break;
#ifdef CALL_3G      //add by wrm 20150305 for cloud
    case NTELC3:
		{
                   if(hold_on==1) break;
                    try_call=1;
                    app_debug(DBG_INFO,"NTELC3__________________804\n");
			int try=0;
			ntel_data call_data;
			ip_addr=get_server_ip();
			memcpy(th_addr_array[0].DevIp,&ip_addr,4);
			memcpy(th_addr_array[0].DevNo,dest_No,4);
			
			netlc_arp=1;
            tcp_data *buf=NULL;
            try=0;
            do{
                if(try>3)
                {
                    break;
                }
                ret=request_send_buf(&buf,NET_BUF_SIZE,TCP_TML);
                try++;
            }while((ret<0)&&(NULL==buf));

            if((buf==NULL)||(ret<0))
            {
                break;
            }
            memcpy(call_data.dest_addr,dest_No,4);
            memcpy(call_data.src_addr,src_No,4);
            call_data.oper_type=oper_type;
            call_data.resstaus=resstatus;
            buf->length=build_call_data(buf->buf,&call_data,(unsigned char*)"804");
            buf->dest_ip=ip_addr;
            int items=ITEM_ONE;

            ret=send_tcp_data(&items,TCP_TML);
            if(ret<0)
            {
                fprintf(stderr,"tcp_send_data <804> err");
                break;
            }


        }
        set_ip_flag();
        oper_type=*((unsigned int*)dest_No);


		break;
      
#endif	
		default:
			break;
		}
	}
	break;
	case MEM_ADDR:
	{
		int try = 0;
		tcp_data *tcp_buf = NULL;
		char cmd_buf[4];
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;

		switch(net_cmd)
		{
		case WXLYC:
		{
                    app_debug(DBG_INFO, "528\n");
			memcpy(dest_No,&msg->num,4);
			if(is_vlan_enable())
			{
				uint32_t gate_ip;
				unsigned char dest_No_2[4];
				memcpy(dest_No_2,dest_No,4);
				memcpy(&gate_ip,gData.addr_gw,4);
				if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
				{
					ret = get_dest_ip_by_arp_vlan(dest_No,th_addr_array, ARPCALLREQ, gate_ip);
				}
				if (ret == DEVNO_NOT_IN_LIST)
				{
					dest_No_2[2]=0;
					dest_No_2[3]=0;
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
					}
					if (ret<=0)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
				}
				else if(ret == 0)
				{
					get_dns1((unsigned char*)&gate_ip);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
					}
					if (ret == DEVNO_NOT_IN_LIST)
					{

						dest_No_2[2]=0;
						dest_No_2[3]=0;
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No_2,th_addr_array,ARPCALLREQ,gate_ip);
						}

						if (ret<=0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
					else if(ret ==0)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
				}
			}
            else
            if (is_have_infostore()) {
                   unsigned char *dest_No_2=(unsigned char [4]){};
                   memcpy(dest_No_2, dest_No, 4);
                   short int fuhao=get_code_from_container_data((char *)&dest_No_2[2]);
                   if(fuhao>0) {
                       dest_No_2[2]=0;
                       dest_No_2[3]=fuhao&0xff;
                   }
                   ret=get_dest_ip_by_arp(dest_No_2,th_addr_array,ARPCALLREQ,0);
                   if (ret < 0) {
                       app_debug(DBG_INFO,"dest dev is not online!\n");
                       goto err_get_ip;
                   }
                   memcpy(&ret_ip, th_addr_array[0].DevIp, sizeof(ret_ip));
                   if(is_mydev_ip(ret_ip))
                       goto err_get_ip;
		    } 
		    else {
			ret = get_dest_ip_by_arp(dest_No,th_addr_array, ARPCALLREQ, 0);
			if (ret <=0)
			{
				fprintf(stderr,"dest dev is not online!\n");
				free_list_data(msg->list_num);
				goto err_get_ip;
			}
               }
			char *data_buf=alloca(msg->data_len);
			if(data_buf==NULL)
			{
				fprintf(stderr,"alloca for 528 failure!\n");
				free_list_data(msg->list_num);
				goto err_get_ip;
			}
			if(get_list_data(msg->list_num,data_buf)<0)
			{
				fprintf(stderr,"get_list_data %x err",msg->net_cmd);
				goto err_req;
			}
			//usleep(ARP_DELAY*10*ARP_RETYR_TIMES);
			//del by wrm 20150519 for  不能再次可能获取，可能会add_com_devlist
		//	ret=get_com_ip_list_devno(th_addr_array,dest_No);//造成tcp无法建立链接是因为ret返回0
		#ifdef FRONT_DOOR		
			if((ret>1)&&(memcmp(th_addr_array[0].DevIp,th_addr_array[1].DevIp,4))==0)	{
				ret=1;
                          // printf("************************\n");
			}
            #endif

			set_ip_flag();
			int i=0;
                    int loop=ret;
			for(i=0;i<loop;i++)
			{
				ret=try=0;
				do
				{
					if (try > 3)
					{
						if(ret<0)
						{
							fprintf(stderr, "request tcp_buf\n");
							goto err_req;
						}
						else
						{
							break;
						}
					}
					ret = request_send_buf(&tcp_buf,msg->data_len,TCP_TML);
					try++;
				} while ((ret < 0) && (NULL == tcp_buf));
				if((tcp_buf==NULL)||(ret<0))
				{
					goto err_get_ip;
				}
				sprintf(cmd_buf,"%x",msg->net_cmd);
				tcp_buf->dest_ip=*((unsigned int *)th_addr_array[i].DevIp);
				tcp_buf->length=msg->data_len;
				build_net_package_head(tcp_buf->cmd_buf,msg->data_len,cmd_buf);
				memcpy(tcp_buf->buf,data_buf,msg->data_len);
				int items = ITEM_TWO;
				ret = send_tcp_data(&items,TCP_TML);
				if (ret < 0)
				{
					fprintf(stderr,"tcp_send_data %x err",msg->net_cmd);
					goto err_req;
				}
			}
		}
		break;
		default:
			break;
		}
	}
	break;
	default:
		break;
	}
data_err:
//#if DEV_GLJKZQ
set_ip_flag();
#if DEV_GLJKZQ|DEV_CONTROL|DEV_TSGLJKZQ
	app_debug(DBG_INFO,"recv ui data len:%d\n",len);
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, oper_type,ret,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
/*
#else
	app_debug(DBG_INFO,"recv ui data len:%d\n",len);
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,ret,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
*/
	return;
err_get_ip://没有获取到IP
set_ip_flag();
#if DEV_GLJKZQ|DEV_CONTROL|DEV_TSGLJKZQ
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,DEV_OFFINE,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
	return;
err_req:	//获取到IP但是发送失败
set_ip_flag();
#if DEV_GLJKZQ|DEV_CONTROL|DEV_TSGLJKZQ
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,SEND_ERR,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
	return;

}

void ui_data_pro(char *buf,int len)//处理接收到的UI数据
{
	int ret=0,loop=0;
	unsigned char *dest_No=(unsigned char [4]){};
	unsigned char *src_No = (unsigned char[4]){};
	unsigned int ip_addr;
	//dev_arrary_t *th_addr;
	DevList th_addr_array[TONG_HAOIP_MAX];
	unsigned int net_cmd;
	unsigned char cmd;
	short allow=0;
	unsigned int oper_type;
	int dev_type=TYPE_JDM365;
	int resstatus;
	char flag_addr=MEM_DATA;
	proc_msg_ret msg_ret={};
	msg_ret.msg_type=MSG_FROM_NET;
	oper_type = 0x00;
	resstatus = 0x00;
	ajb_msg *msg=(ajb_msg *)buf;
	app_debug(DBG_INFO,"new net package to be sent:cmd=%02x,pack_type=0x%02x\n",msg->data.net_cmd,buf[0]);


	if(msg->data.net_cmd>0xfff)
		goto data_err;
	switch(buf[0])
	{
		
	case MEM_GLJ:
	{
		switch(msg->data.net_cmd)
		{
		case GDATC:
		{
			switch(msg->data.cmd)
			{
			case SSSK:
			{
				net_cmd=msg->data.net_cmd;
				oper_type=msg->data.oper_type;
				char cmd_pc[4];
				sprintf(cmd_pc,"%x",msg->data.net_cmd);
				memcpy(dest_No,msg->data.dest_No,4);
				char data[10]={};
				data[0]=msg->data.cmd;
				int data_len=msg->data.src_No[0];
				memcpy(&data[1],&msg->data.src_No[1],data_len);
				connect_node *net_runtime;
				if(find_connect_node(msg->num,&net_runtime)==0)
				{
					int path=TCP_TML;
					int try = 0;
					tcp_data *tcp_buf = NULL;
					try = 0;
					do {
						if (try > 3)
							break;
						ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
						try++;
					} while ((ret < 0) && (NULL == tcp_buf));
					if((tcp_buf==NULL)||(ret<0))
						goto err_get_ip;
					tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);

					tcp_buf->dest_ip=net_runtime->dest_ip;
					int items = ITEM_ONE;
					ret = send_tcp_data(&items,path);
					if (ret < 0) {
						app_debug(DBG_INFO,"tcp_send_data  <%s> err",cmd_pc);
						goto err_req;
					}
				}
				else
					goto err_req;
			}
			break;
			case SSCS:
			{

				send_data_to_uipro_cache(buf,len);
				//usleep(2000000);
				return;
				/*net_cmd=msg->data.net_cmd;
				oper_type=msg->data.oper_type;
				char cmd_pc[4];
				sprintf(cmd_pc,"%x",msg->data.net_cmd);
				int path=TCP_TML;
				memcpy(dest_No,msg->data.dest_No,4);
				ret = get_dest_ip_by_arp(dest_No,th_addr_array, ARPINFOREQ,0);
				set_ip_flag();
				if (ret <= 0) {
					app_debug(DBG_INFO,"dest dev is not online!\n");
					goto err_get_ip;
				}

				int i=0;
				int loop=ret;
				for(i=0;i<loop;i++)
				{
					char data[10]={};
					data[0]=msg->data.cmd;
					int data_len=msg->data.src_No[0];
					memcpy(&data[1],&msg->data.src_No[1],data_len);
					int try = 0;
					tcp_data *tcp_buf = NULL;
					try = 0;
					do {
						if (try > 3)
							break;
						ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
						try++;
					} while ((ret < 0) && (NULL == tcp_buf));
					if((tcp_buf==NULL)||(ret<0))
						goto err_get_ip;
					tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);
					tcp_buf->dest_ip=*((unsigned int*)th_addr_array[i].DevIp);
					int items = ITEM_ONE;
					ret = send_tcp_data(&items,path);
					if (ret < 0) {
						fprintf(stderr,"tcp_send_data  <%s> err",cmd_pc);
						goto err_req;
					}
				}*/
			}
			break;
			/*
			case SDIS:// 增加
			{
				send_data_to_uipro_cache(buf,len);
				return;
			}
			break;
			*/
			default:
			{
                           if ((msg->data.cmd == SSIC) && (msg->data.dest_No[2]!=0) && (msg->data.dest_No[3]!=0)) {
                               send_data_to_uipro_cache(buf,len);
                               return;
                           }
				net_cmd=msg->data.net_cmd;
				oper_type=msg->data.oper_type;
				char cmd_pc[4];
				sprintf(cmd_pc,"%x",msg->data.net_cmd);
				int path=TCP_TML;
				memcpy(dest_No,msg->data.dest_No,4);
				
				if(is_vlan_enable())
				{
					uint32_t gate_ip;
					memcpy(&gate_ip,gData.addr_gw,4);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret = get_dest_ip_by_arp_vlan(dest_No,th_addr_array, ARPINFOREQ,gate_ip);
					}
					printf("ret = %d\n",ret);
					if (ret == DEVNO_NOT_IN_LIST)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
					else if (ret == 0)
					{
						get_dns1((unsigned char*)&gate_ip);
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
						}
						if (ret <= 0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;

						}
					}
				}
				else
				{
					ret = get_dest_ip_by_arp(dest_No,th_addr_array, ARPINFOREQ,0);
					if (ret <= 0) {
						app_debug(DBG_INFO,"dest dev is not online!\n");
						goto err_get_ip;
					}
				}
				char data[10]={};
				data[0]=msg->data.cmd;
				int data_len=msg->data.src_No[0];
				memcpy(&data[1],&msg->data.src_No[1],data_len);
				int try = 0;
				tcp_data *tcp_buf = NULL;
				try = 0;
				do {
					if (try > 3)
						break;
					ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
					try++;
				} while ((ret < 0) && (NULL == tcp_buf));
				if((tcp_buf==NULL)||(ret<0))
					goto err_get_ip;
				tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);
				tcp_buf->dest_ip=*((unsigned int*)th_addr_array[0].DevIp);
				int items = ITEM_ONE;
				ret = send_tcp_data(&items,path);
				if (ret < 0) {
					fprintf(stderr,"tcp_send_data  <%s> err",cmd_pc);
					goto err_req;
				}
				set_ip_flag();
			}
			break;
			}
		}
		break;
		}
	}
	break;
	case MEM_DATA:
	{
		if (msg->data.cmd == 0x88) { //返回状态
			//ajb_msg_ret_split(&net_cmd, &cmd, &allow, &oper_type, &resstatus,&flag_addr,(ajb_msg_ret*)msg);
			ajb_msg_ret_split_whole(&net_cmd, &cmd, &allow, &oper_type, &resstatus,&dev_type,&flag_addr,(ajb_msg_ret*)msg);
		} else
		{
			//ajb_msg_split(&net_cmd, &cmd, &dest_No, &src_No,&flag_addr,msg);
			ajb_msg_split_whole(&net_cmd, &cmd, &dest_No, &src_No,&oper_type,&resstatus,&flag_addr,msg);
			//app_debug(DBG_INFO,"net_cmd:0x%.3x oper_type:0x%.4x resstatus:0x%.4x!\n",net_cmd,oper_type,resstatus);
		}
		flag_addr=MEM_DATA;
		switch (net_cmd) {

#ifdef CALL_3G
		case RTELC3:
		{
			app_debug(DBG_INFO,"ui_805  %d  0x%x dev_type:0x%d\n",msg->num,msg->num,dev_type);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{

				send_len=build_rcall_data(&cmdbuf,&rcall_data,"805");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
		
	 case NENDC3:
		{
			ntel_data call_data;
			netlc_arp=0;
			
            memcpy(call_data.dest_addr,dest_No,4);
            memcpy(call_data.src_addr,src_No,4);
            call_data.oper_type=oper_type;//0x00000004L;
            call_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
            tcp_data *tcp_buf;
            int try=0;
            do{
                if(try>3)
                {
                    break;
                }
                ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
                try++;
            }while((ret<0)&&(NULL==tcp_buf));

            if((tcp_buf==NULL)||(ret<0))
            {
                goto err_get_ip;
            }

            tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"808");
            tcp_buf->dest_ip=get_server_ip();

            int items=ITEM_ONE;
            ret=send_tcp_data(&items,TCP_TML);
            if(ret<0)
            {
                fprintf(stderr,"tcp_send_data <808> err");
                goto err_req;
            }
        }

        break; 

		case RENDC3:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data(&cmdbuf,&rcall_data,"809");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case NANSC3:
		{
			//	app_debug(DBG_INFO,"ui_710  %d  0x%x\n",msg->num,msg->num);
			ntel_data call_data;
			ret=get_ip_from_call_list(dest_No,&ip_addr);
			if(ret<0)
			{
				app_debug(DBG_INFO,"dest dev is not online!\n");
				goto err_get_ip;
			}
			else
			{
				app_debug(DBG_INFO,"get_ip_from_call_list!\n");
			}

			memcpy(call_data.dest_addr,dest_No,4);
			memcpy(call_data.src_addr,src_No,4);
			call_data.oper_type=oper_type;
			call_data.resstaus=resstatus;
			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}

			tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"810");
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data  <710> err");
				goto err_req;
			}

		}
		break;
		case RANSC3:
		{
			app_debug(DBG_INFO,"ui_811  %d  0x%x\n",msg->num,msg->num);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;

			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data(&cmdbuf,&rcall_data,"811");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
		case ULOCKC3:
		{
			app_debug(DBG_INFO,"ui_812\n");
			int path=TCP_TML;
			ip_tongh_t ip_addr_talk[TONG_HAOIP_MAX];
			if(get_talkip_from_call_list(dest_No,ip_addr_talk)<1)
			{
				ip_addr=get_server_ip();
			}
			else
			{
				ip_addr=ip_addr_talk[0].ip_addr;
			}


			int try = 0;
			tcp_data *tcp_buf = NULL;
			try = 0;
			do {
				if (try > 3)
					break;
				ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
				try++;
			} while ((ret < 0) && (NULL == tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}
			int data_len=8+14;
			tcp_buf->buf[0]=0x7;
			tcp_buf->buf[1]=0xb8;
			memcpy((void *)(&tcp_buf->buf[2]),&data_len,4);
			memcpy((void *)(&tcp_buf->buf[6]),"req=",4);
			memcpy((void *)(&tcp_buf->buf[10]),(unsigned char*)"812",3);
			memcpy((void *)(&tcp_buf->buf[13]),"&query=",7);
			memcpy((void *)(&tcp_buf->buf[20]),dest_No,4);
			memcpy((void *)(&tcp_buf->buf[24]),src_No,4);
			tcp_buf->length=28;

			//buf->length=build_call_data(buf->buf,&call_data,(unsigned char*)"804");
			//tcp_buf->length =build_net_data(tcp_buf->buf,&msg->data.cmd,10,(char*) "518");
			tcp_buf->dest_ip=ip_addr;
			int items = ITEM_ONE;
			ret = send_tcp_data(&items,path);
			if (ret < 0) {
				fprintf(stderr,"tcp_send_data  <812> err");
				goto err_req;
			}
		}
		break;
		case RLOCKC3:
		{
			unsigned char *cmdbuf=(unsigned char [40]){};
			char buf[10];
			memcpy(buf,dest_No,4);
			buf[4]=cmd;
			int data_len=1+14;
			cmdbuf[0]=0x7;
			cmdbuf[1]=0xb8;
			memcpy((void *)(&cmdbuf[2]),&data_len,4);
			memcpy((void *)(&cmdbuf[6]),"req=",4);
			memcpy((void *)(&cmdbuf[10]),(unsigned char*)"813",3);
			memcpy((void *)(&cmdbuf[13]),"&query=",7);
			memcpy((void *)(&cmdbuf[20]),&cmd,1);
			int send_len=21;
			//buf->length=build_call_data(buf->buf,&call_data,(unsigned char*)"804");
			//tcp_buf->length =build_net_data(tcp_buf->buf,&msg->data.cmd,10,(char*) "518");
			///tcp_buf->dest_ip=ip_addr;
			//int send_len=build_net_data(cmdbuf,buf,5,RDST);
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}

		}
		break;
#endif		
		case NTELC:
#ifdef CALL_3G//add by wrm 20150305 for cloud
		case NTELC3:
#endif
		{

			printf("ruan_704_20151203\n");
			send_data_to_uipro_cache(buf,len);
			//usleep(2000000);
			return;
#if 0
			int try=0;
			ntel_data call_data;
			ret=get_dest_ip_by_arp(dest_No,th_addr_array,ARPCALLREQ,0);
			if (ret <= 0)
			{
				unsigned char *dest_No_2=(unsigned char [4]){};
				memcpy(dest_No_2,dest_No,4);
				dest_No_2[2]=0;
				dest_No_2[3]=0;
				ret=get_dest_ip_by_arp(dest_No_2,th_addr_array,ARPCALLREQ,0);
				if (ret <=0)
				{
					fprintf(stderr,"dest dev is not online!\n");
					goto err_get_ip;
				}
			}
			int i=0;
			int loop=rtelc_cache.ntelc_count=ret;
			memcpy(rtelc_cache.ntelc_NO,dest_No,4);
			if(ret>0)
			{
				clear_tonghaoip_from_call_list(dest_No);
			}

			for(i=0;i<loop;i++)
			{

				if((th_addr_array[i].DevNo[0]==dest_No[0])&&(th_addr_array[i].DevNo[1]==dest_No[1]))
				{
					add_call_item(dest_No,*((unsigned int*)th_addr_array[i].DevIp));
				}
			}
			for(i=0;i<loop;i++)
			{
				tcp_data *buf=NULL;
				try=0;
				do{
					if(try>3)
					{
						break;
					}
					ret=request_send_buf(&buf,NET_BUF_SIZE,TCP_TML);
					try++;
				}while((ret<0)&&(NULL==buf));

				if((buf==NULL)||(ret<0))
				{
					clear_com_dev_arrary();
					clear_tonghaoip_from_call_list(dest_No);
					goto err_get_ip;
				}

				memcpy(call_data.dest_addr,dest_No,4);
				memcpy(call_data.src_addr,src_No,4);
				call_data.oper_type=oper_type;
				call_data.resstaus=resstatus;
				//buf->length=build_call_data(buf->buf,&call_data,(unsigned char*)"704");
				buf->length=build_call_data_j365(buf->buf,&call_data,(unsigned char*)"704");
				buf->dest_ip=*((unsigned int*)th_addr_array[i].DevIp);
				int items=ITEM_ONE;
				ret=send_tcp_data(&items,TCP_TML);
				if(ret<0)
				{
					fprintf(stderr,"tcp_send_data <704> err");
					clear_com_dev_arrary();
					clear_tonghaoip_from_call_list(dest_No);
					goto err_req;
				}
				//add_call_item(call_data.dest_addr,buf->dest_ip);
			}
			oper_type=*((unsigned int*)dest_No);
//#if DEV_GLJKZQ
/*			oper_type=get_com_devno();
			if(memcmp(call_data.dest_addr,&oper_type,4)!=0)
			{
				add_call_item((unsigned char *)&oper_type,ip_addr);
			}*/
			clear_com_dev_arrary();
//#endif
#endif
		}
		break;

#if 0
		case RTELC:
		{
			app_debug(DBG_INFO,"ui_705  %d  0x%x dev_type:0x%d\n",msg->num,msg->num,dev_type);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
            	switch(dev_type)
            	{
            	case TYPE_JDM365:
            		send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"705");
            		break;
				case TYPE_J926:
            		send_len=build_rcall_data_bytype(&cmdbuf,&rcall_data,RTEL,RES_926);
					break;	            		
            	case TYPE_D2100:
            		send_len=build_rcall_data_bytype(&cmdbuf,&rcall_data,RTEL,RES_2100);
            		break;
            	default:
            		send_len=build_rcall_data(&cmdbuf,&rcall_data,"705");
            		break;
            	}
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;

		case NENDC:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			ntel_data call_data;
			lock_ntelc_cache_lock();
			netlc_arp=0;
			unlock_ntelc_cache_lock();
			ip_tongh_t ip_array[TONG_HAOIP_MAX];
			memset(ip_array, 0x00, sizeof(ip_array));
			unsigned int count_th=get_iplist_from_call_list(dest_No,msg->data.cmd,ip_array);
                   if((*(unsigned int*)dest_No==0) && (ip_array[0].ip_addr==0)) //管理机繁忙应答的IP是0，挂机不发708
                       goto err_get_ip; //20140122
                   else
			if(count_th<=0)
			{
				fprintf(stderr,"dest dev is not online!\n");
				if((msg->data.cmd==SEND_AL)||(msg->data.cmd==SEND_TA))
				{
					clear_tonghaoip_from_call_list(dest_No);
				}
				goto err_get_ip;
			}

			for(loop=0;loop<count_th;loop++)
			{
				memcpy(call_data.dest_addr,dest_No,4);
				memcpy(call_data.src_addr,src_No,4);
				call_data.oper_type=oper_type;//0x00000004L;
				call_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
				tcp_data *tcp_buf;
				int try=0;
				do{
					if(try>3)
					{
						break;
					}
					ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
					try++;
				}while((ret<0)&&(NULL==tcp_buf));

				if((tcp_buf==NULL)||(ret<0))
				{
					goto err_get_ip;
				}

				tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"708");
				tcp_buf->dest_ip=ip_array[loop].ip_addr;

				int items=ITEM_ONE;
				ret=send_tcp_data(&items,TCP_TML);
				if(ret<0)
				{
					fprintf(stderr,"tcp_send_data <708> err");
					goto err_req;
				}
				set_dest_dev_status(dest_No,tcp_buf->dest_ip,TONGHAO_DOWN);
			}
			//已经挂断所有通话，清空同号IP记录
			if((msg->data.cmd==SEND_AL)||(msg->data.cmd==SEND_TA))
			{
				clear_tonghaoip_from_call_list(dest_No);
			}
		}
		break;
		case RENDC:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"709");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case NANSC:
		{
		//	app_debug(DBG_INFO,"ui_710  %d  0x%x\n",msg->num,msg->num);
			ntel_data call_data;
			ret=get_ip_from_call_list(dest_No,&ip_addr);
			if(ret<0)
			{
				app_debug(DBG_INFO,"dest dev is not online!\n");
				goto err_get_ip;
			}
			else
			{
				app_debug(DBG_INFO,"get_ip_from_call_list!\n");
			}

			memcpy(call_data.dest_addr,dest_No,4);
			memcpy(call_data.src_addr,src_No,4);
			call_data.oper_type=oper_type;
			call_data.resstaus=resstatus;
			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}

			tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"710");
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data  <710> err");
				goto err_req;
			}
		}
		break;
		case RANSC:
		{
			app_debug(DBG_INFO,"ui_711  %d  0x%x\n",msg->num,msg->num);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;

			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"711");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
#else		
		case RTELC:
		{
			app_debug(DBG_INFO,"ui_705  %d  0x%x dev_type:0x%d\n",msg->num,msg->num,dev_type);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
            	switch(dev_type)
            	{
            	case TYPE_JDM365:	
            		send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"705");
            		break;
            	case TYPE_D2100:
            		send_len=build_rcall_data_bytype(&cmdbuf,&rcall_data,RTEL,RES_2100);
            		break;
				case TYPE_J926:
            		send_len=build_rcall_data_bytype(&cmdbuf,&rcall_data,RTEL,RES_926);
				break;			
            	default:
            		send_len=build_rcall_data(&cmdbuf,&rcall_data,"705");
            		break;
            	}
				printf("*************     RTELC     **********\n");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
	
		case NENDC:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			ntel_data call_data;
			set_netlc_arp_value(0);
			ip_tongh_t ip_array[TONG_HAOIP_MAX];
			memset(ip_array, 0x00, sizeof(ip_array));
                    printf("msg->data.cmd-----=%d\n",msg->data.cmd);
                   unsigned int count_th=get_iplist_from_call_list(dest_No,msg->data.cmd,ip_array);
                    printf("count_th__________=%d\n",count_th);
                   if((*(unsigned int*)dest_No==0) && (ip_array[0].ip_addr==0)) //管理机繁忙应答的IP是0，挂机不发708
                       goto err_get_ip; //20140122
                   else
			if(count_th<=0)
			{
				fprintf(stderr,"dest dev is not online!\n");
				if((msg->data.cmd==SEND_AL)||(msg->data.cmd==SEND_TA))
				{
					clear_tonghaoip_from_call_list(dest_No);
				}
				goto err_get_ip;
			}

			for(loop=0;loop<count_th;loop++)
			{
				memcpy(call_data.dest_addr,dest_No,4);
				memcpy(call_data.src_addr,src_No,4);
				call_data.oper_type=oper_type;//0x00000004L;
				call_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
				tcp_data *tcp_buf;
				int try=0;
				do{
					if(try>3)
					{
						break;
					}
					ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
					try++;
				}while((ret<0)&&(NULL==tcp_buf));

				if((tcp_buf==NULL)||(ret<0))
				{
					goto err_get_ip;
				}

				tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"708");
				tcp_buf->dest_ip=ip_array[loop].ip_addr;

				
			     struct in_addr addr;
				addr.s_addr=tcp_buf->dest_ip;
				printf("@###end_ip :%s\n",inet_ntoa(addr));

				int items=ITEM_ONE;
				ret=send_tcp_data(&items,TCP_TML);
				if(ret<0)
				{
					fprintf(stderr,"tcp_send_data <708> err");
					goto err_req;
				}
				set_dest_dev_status(dest_No,tcp_buf->dest_ip,TONGHAO_DOWN);
			}
			//已经挂断所有通话，清空同号IP记录
			if((msg->data.cmd==SEND_AL)||(msg->data.cmd==SEND_TA))
			{
				clear_tonghaoip_from_call_list(dest_No);
			}
		}
		break;
		case RENDC:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				printf("*************     709     **********\n");
				send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"709");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case NANSC:
		{
			app_debug(DBG_INFO,"ui_710  %d  0x%x\n",msg->num,msg->num);
			ntel_data call_data;
			ret=get_ip_from_call_list(dest_No,&ip_addr);
			if(ret<0)
			{
				app_debug(DBG_INFO,"dest dev is not online!\n");
				goto err_get_ip;
			}
			else
			{
				app_debug(DBG_INFO,"get_ip_from_call_list!\n");
			}

			memcpy(call_data.dest_addr,dest_No,4);
			memcpy(call_data.src_addr,src_No,4);
			call_data.oper_type=oper_type;
			call_data.resstaus=resstatus;
			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}

			tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"710");
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data  <710> err");
				goto err_req;
			}
		}
		break;
		case RANSC:
		{
			app_debug(DBG_INFO,"ui_711  %d  0x%x\n",msg->num,msg->num);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;

			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"711");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;

#endif	
		case GDATC:
		{
			app_debug(DBG_INFO,"ui_518\n");
			int path=TCP_TML;
			if((flag_addr&DATA_MASK)>0)
			{
				ip_addr=get_server_ip();
				path=TCP_PC;
			}
			else
			{

				
				if(is_vlan_enable())
				{
					unsigned int gate_ip;
					memcpy(&gate_ip,gData.addr_gw,4);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret = get_dest_ip_by_arp_vlan(dest_No,th_addr_array, ARPINFOREQ,gate_ip);
					}
					if (ret == DEVNO_NOT_IN_LIST)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
					else if (ret == 0)
					{
						get_dns1((unsigned char*)&gate_ip);
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
						}
						if (ret <= 0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
				}
				else
				{
					ret = get_dest_ip_by_arp(dest_No,th_addr_array, ARPINFOREQ, 0);
					if (ret < 0)
					{
						fprintf(stderr,"dest dev is not online!\n");
						goto err_get_ip;
					}
				}
				ip_addr=*((unsigned int *)th_addr_array[0].DevIp);
				set_ip_flag();
			}
/*			ntel_data call_data;
			memcpy(call_data.dest_addr, dest_No, 4);
			memcpy(call_data.src_addr, src_No, 4);
			call_data.oper_type = oper_type;
			call_data.resstaus = resstatus;*/
			int try = 0;
			tcp_data *tcp_buf = NULL;
			try = 0;
			do {
				if (try > 3)
					break;
				ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
				try++;
			} while ((ret < 0) && (NULL == tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
				goto err_get_ip;
			tcp_buf->length =build_net_data(tcp_buf->buf,&msg->data.cmd,10,(char*) "518");
			tcp_buf->dest_ip=ip_addr;
			int items = ITEM_ONE;
			ret = send_tcp_data(&items,path);
			if (ret < 0) {
				fprintf(stderr,"tcp_send_data  <518> err");
				goto err_req;
			}
		}
			break;
		case RGDAC:
		{
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow = allow;
			rcall_data.oper_type = 0;
			rcall_data.resstaus = 0;
			int send_len = 0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{

/*				if(get_dest_dev_type_by_ip(net_runtime->dest_ip)==TYPE_JDM365)
					send_len =build_rcall_data_j365(&cmdbuf, &rcall_data,"519");
				else*/
				send_len =build_rcall_data(&cmdbuf, &rcall_data,"519");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
		case SJYDC:
		{
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow = allow;
			rcall_data.oper_type = 0;
			rcall_data.resstaus = 0;
			int send_len = build_rcall_data(&cmdbuf, &rcall_data,"521");
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
		case GDSTC:
		{
			app_debug(DBG_INFO,"ui_536  %d  0x%x\n",msg->num,msg->num);
			
			if(is_vlan_enable())
			{
				unsigned int gate_ip;
				memcpy(&gate_ip,gData.addr_gw,4);
				if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
				{
					ret = get_dest_ip_by_arp_vlan(dest_No,th_addr_array, ARPINFOREQ,gate_ip);
				}
				if (ret == DEVNO_NOT_IN_LIST)
				{
					fprintf(stderr,"vlan: dest dev is not online!\n");
					unlock_pthread_mutex_lock(&arp_lock);
					goto err_get_ip;
				}
				else if (ret ==0)
				{
					get_dns1((unsigned char*)&gate_ip);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
					}
					if (ret <= 0)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
				}
			}
			else
			{
				ret=get_dest_ip_by_arp(dest_No,th_addr_array,ARPINFOREQ,0);
				if (ret < 0) {
					fprintf(stderr,"dest dev is not online!\n");
					goto err_get_ip;
				}
			}
			ip_addr=*((unsigned int *)th_addr_array[0].DevIp);
			set_ip_flag();

			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}

			tcp_buf->length=build_net_data(tcp_buf->buf,&dest_No[0],4,GDST);
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				fprintf(stderr,"tcp_send_data  <536> err");
				goto err_req;
			}
		}
		break;
		case RDSTC:
		{
			unsigned char *cmdbuf=(unsigned char [40]){};
			char buf[10];
			memcpy(buf,dest_No,4);
			buf[4]=cmd;
			int send_len=build_net_data(cmdbuf,buf,5,RDST);
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				app_debug(DBG_INFO,"ui_537_2  %d  0x%x\n",msg->num,msg->num);
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
	/*	case RPZLC://533
		case SPFKC://581
		{
			char cmd_pc[4]={0};
			app_debug(DBG_INFO,"ui_%03x  %d  0x%x\n",net_cmd,msg->num,msg->num);
			ret=get_dest_ip_by_arp(dest_No,(void **) &th_addr,ARPINFOREQ,0);
			if (ret < 0) {
				fprintf(stderr,"dest dev is not online!\n");
				goto err_get_ip;
			}

			ip_addr=*((unsigned int *)th_addr->dev.DevIp);

			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}
			sprintf(cmd_pc,"%x",msg->net_cmd);
			tcp_buf->length=build_net_data(tcp_buf->buf,&dest_No[0],4,GDST);
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				fprintf(stderr,"tcp_send_data  <536> err");
				goto err_req;
			}

		}
		break;*/
#ifdef CALL_3G_CLOUD_01
		case RTELC3:
		{
			app_debug(DBG_INFO,"ui_805  %d  0x%x dev_type:0x%d\n",msg->num,msg->num,dev_type);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{

				send_len=build_rcall_data(&cmdbuf,&rcall_data,"805");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
/*		case NENDC3:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			ntel_data call_data;
			netlc_arp=0;
			ip_tongh_t ip_array[TONG_HAOIP_MAX];
			memset(ip_array, 0x00, sizeof(ip_array));
			unsigned int count_th=get_iplist_from_call_list(dest_No,msg->data.cmd,ip_array);
			if(count_th<=0)
			{
				fprintf(stderr,"dest dev is not online!\n");
				if((msg->data.cmd==SCRE)||(msg->data.cmd==SEND_AL)||(msg->data.cmd==SEND_TA))
				{
					clear_tonghaoip_from_call_list(dest_No);
				}
				goto err_get_ip;
			}

			for(loop=0;loop<count_th;loop++)
			{
				memcpy(call_data.dest_addr,dest_No,4);
				memcpy(call_data.src_addr,src_No,4);
				call_data.oper_type=oper_type;//0x00000004L;
				call_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
				tcp_data *tcp_buf;
				int try=0;
				do{
					if(try>3)
					{
						break;
					}
					ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
					try++;
				}while((ret<0)&&(NULL==tcp_buf));

				if((tcp_buf==NULL)||(ret<0))
				{
					goto err_get_ip;
				}

				tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"808");
				tcp_buf->dest_ip=ip_array[loop].ip_addr;

				int items=ITEM_ONE;
				ret=send_tcp_data(&items,TCP_TML);
				if(ret<0)
				{
					fprintf(stderr,"tcp_send_data <708> err");
					goto err_req;
				}
				set_dest_dev_status(dest_No,tcp_buf->dest_ip,TONGHAO_DOWN);
			}
			//已经挂断所有通话，清空同号IP记录
			if((msg->data.cmd==SEND_AL)||(msg->data.cmd==SEND_TA))
			{
				clear_tonghaoip_from_call_list(dest_No);
			}
		}
		break;
*/
        case NENDC3:
		{
			ntel_data call_data;
			netlc_arp=0;
			
            memcpy(call_data.dest_addr,dest_No,4);
            memcpy(call_data.src_addr,src_No,4);
            call_data.oper_type=oper_type;//0x00000004L;
            call_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
            tcp_data *tcp_buf;
            int try=0;
            do{
                if(try>3)
                {
                    break;
                }
                ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
                try++;
            }while((ret<0)&&(NULL==tcp_buf));

            if((tcp_buf==NULL)||(ret<0))
            {
                goto err_get_ip;
            }

            tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"808");
            tcp_buf->dest_ip=get_server_ip();

            int items=ITEM_ONE;
            ret=send_tcp_data(&items,TCP_TML);
            if(ret<0)
            {
                fprintf(stderr,"tcp_send_data <808> err");
                goto err_req;
            }
        }

        break; 
		
		case RENDC3:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data(&cmdbuf,&rcall_data,"809");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case NANSC3:
		{
				app_debug(DBG_INFO,"ui_710  %d  0x%x\n",msg->num,msg->num);
			ntel_data call_data;
			ret=get_ip_from_call_list(dest_No,&ip_addr);
			if(ret<0)
			{
				app_debug(DBG_INFO,"dest dev is not online!\n");
				goto err_get_ip;
			}
			else
			{
				app_debug(DBG_INFO,"get_ip_from_call_list!\n");
			}

			memcpy(call_data.dest_addr,dest_No,4);
			memcpy(call_data.src_addr,src_No,4);
			call_data.oper_type=oper_type;
			call_data.resstaus=resstatus;
			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}

			tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"810");
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data  <710> err");
				goto err_req;
			}

		}
		break;
		case RANSC3:
		{
			app_debug(DBG_INFO,"ui_711  %d  0x%x\n",msg->num,msg->num);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;

			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data(&cmdbuf,&rcall_data,"811");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
		break;
		case ULOCKC3:
		{
			app_debug(DBG_INFO,"ui_812\n");
			int path=TCP_TML;
			ip_tongh_t ip_addr_talk[TONG_HAOIP_MAX];
			if(get_talkip_from_call_list(dest_No,ip_addr_talk)<1)
			{
				ip_addr=get_server_ip();
			}
			else
			{
				ip_addr=ip_addr_talk[0].ip_addr;
			}


			int try = 0;
			tcp_data *tcp_buf = NULL;
			try = 0;
			do {
				if (try > 3)
					break;
				ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
				try++;
			} while ((ret < 0) && (NULL == tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				goto err_get_ip;
			}
			int data_len=8+14;
			tcp_buf->buf[0]=0x7;
			tcp_buf->buf[1]=0xb8;
			memcpy((void *)(&tcp_buf->buf[2]),&data_len,4);
			memcpy((void *)(&tcp_buf->buf[6]),"req=",4);
			memcpy((void *)(&tcp_buf->buf[10]),(unsigned char*)"812",3);
			memcpy((void *)(&tcp_buf->buf[13]),"&query=",7);
			memcpy((void *)(&tcp_buf->buf[20]),dest_No,4);
			memcpy((void *)(&tcp_buf->buf[24]),src_No,4);
			tcp_buf->length=28;

			//buf->length=build_call_data(buf->buf,&call_data,(unsigned char*)"804");
			//tcp_buf->length =build_net_data(tcp_buf->buf,&msg->data.cmd,10,(char*) "518");
			tcp_buf->dest_ip=ip_addr;
			int items = ITEM_ONE;
			ret = send_tcp_data(&items,path);
			if (ret < 0) {
				fprintf(stderr,"tcp_send_data  <812> err");
				goto err_req;
			}
		}
		break;
		case RLOCKC3:
		{
			unsigned char *cmdbuf=(unsigned char [40]){};
			char buf[10];
			memcpy(buf,dest_No,4);
			buf[4]=cmd;
			int data_len=1+14;
			cmdbuf[0]=0x7;
			cmdbuf[1]=0xb8;
			memcpy((void *)(&cmdbuf[2]),&data_len,4);
			memcpy((void *)(&cmdbuf[6]),"req=",4);
			memcpy((void *)(&cmdbuf[10]),(unsigned char*)"813",3);
			memcpy((void *)(&cmdbuf[13]),"&query=",7);
			memcpy((void *)(&cmdbuf[20]),&cmd,1);
			int send_len=21;
			//buf->length=build_call_data(buf->buf,&call_data,(unsigned char*)"804");
			//tcp_buf->length =build_net_data(tcp_buf->buf,&msg->data.cmd,10,(char*) "518");
			///tcp_buf->dest_ip=ip_addr;
			//int send_len=build_net_data(cmdbuf,buf,5,RDST);
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}

		}
		break;	
		#endif

#if   1
	case GVNUMC:
		{
			ntel_data call_data;
			netlc_arp=0;
			
            memcpy(call_data.dest_addr,dest_No,4);
            memcpy(call_data.src_addr,src_No,4);
            call_data.oper_type=oper_type;//0x00000004L;
            call_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
            tcp_data *tcp_buf;
            int try=0;
            do{
                if(try>3)
                {
                    break;
                }
                ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
                try++;
            }while((ret<0)&&(NULL==tcp_buf));

            if((tcp_buf==NULL)||(ret<0))
            {
                goto err_get_ip;
            }

            tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"814");
            tcp_buf->dest_ip=get_server_ip();

            int items=ITEM_ONE;
            ret=send_tcp_data(&items,TCP_TML);
            if(ret<0)
            {
                fprintf(stderr,"tcp_send_data <808> err");
                goto err_req;
            }
        }
	break;

	case RVNUMC:
		{
			app_debug(DBG_INFO,"ui_815  %d  0x%x\n",msg->num,msg->num);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;

			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data(&cmdbuf,&rcall_data,"815");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
			{
				goto err_req;
			}
		}
	break;

#endif		
		default:
			break;
		}
	}
	break;
	case MEM_ADDR:
	{
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;

		switch(net_cmd)
		{
		case WXLYC:
		{
			send_data_to_uipro_cache(buf,len);
			//usleep(2000000);
			return;
#if 0
			memcpy(dest_No,&msg->num,4);
			ret = get_dest_ip_by_arp(dest_No,th_addr_array, ARPCALLREQ, 0);
			if (ret <=0)
			{
				fprintf(stderr,"dest dev is not online!\n");
				free_list_data(msg->list_num);
				goto err_get_ip;
			}
			char *data_buf=alloca(msg->data_len);
			if(data_buf==NULL)
			{
				fprintf(stderr,"alloca for 528 failure!\n");
				free_list_data(msg->list_num);
				goto err_get_ip;
			}
			if(get_list_data(msg->list_num,data_buf)<0)
			{
				fprintf(stderr,"get_list_data %x err",msg->net_cmd);
				goto err_req;
			}

			int i=0;
			int loop=ret;
			for(i=0;i<loop;i++)
			{
				ret=try=0;
				do
				{
					if (try > 3)
					{
						if(ret<0)
						{
							fprintf(stderr, "request tcp_buf\n");
							goto err_req;
						}
						else
						{
							break;
						}
					}
					ret = request_send_buf(&tcp_buf,msg->data_len,TCP_TML);
					try++;
				} while ((ret < 0) && (NULL == tcp_buf));
				if((tcp_buf==NULL)||(ret<0))
				{
					goto err_get_ip;
				}
				sprintf(cmd_buf,"%x",msg->net_cmd);
				tcp_buf->dest_ip=*((unsigned int *)th_addr_array[i].DevIp);
				tcp_buf->length=msg->data_len;
				build_net_package_head(tcp_buf->cmd_buf,msg->data_len,cmd_buf);
				memcpy(tcp_buf->buf,data_buf,msg->data_len);
				int items = ITEM_TWO;
				ret = send_tcp_data(&items,TCP_TML);
				if (ret < 0)
				{
					fprintf(stderr,"tcp_send_data %x err",msg->net_cmd);
					goto err_req;
				}
			}
#endif
		}
		break;
		default:
			break;
		}
	}
	break;
	case PC_ADDR:
	{	
		int try = 0;
		tcp_data *tcp_buf = NULL;
		char cmd_pc[4];
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;
        app_debug(DBG_INFO,"PC_ADDR pro: %x\n",msg->net_cmd);		
		if(net_cmd==WXLYC){
			if(msg->num!=DEV_NO_NULL)
			{
				
				if(is_vlan_enable())
				{
					unsigned int gate_ip;
					memcpy(&gate_ip,gData.addr_gw,4);
					if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
					{
						ret = get_dest_ip_by_arp_vlan((unsigned char *)&msg->num,th_addr_array, ARPCALLREQ, gate_ip);
					}
					if (ret == DEVNO_NOT_IN_LIST)
					{
						fprintf(stderr,"vlan: dest dev is not online!\n");
						unlock_pthread_mutex_lock(&arp_lock);
						goto err_get_ip;
					}
					else if (ret == 0)
					{
						get_dns1((unsigned char*)&gate_ip);
						if((INADDR_NONE!=gate_ip)&&(INADDR_ANY!=gate_ip))
						{
							ret=get_dest_ip_by_arp_vlan(dest_No,th_addr_array,ARPCALLREQ,gate_ip);
						}
						if (ret <= 0)
						{
							fprintf(stderr,"vlan: dest dev is not online!\n");
							unlock_pthread_mutex_lock(&arp_lock);
							goto err_get_ip;
						}
					}
				}
				else
				{
					ret = get_dest_ip_by_arp((unsigned char *)&msg->num,th_addr_array, ARPCALLREQ, 0);
					if (ret <= 0) {
						fprintf(stderr,"dest dev is not online!\n");
						free_list_data(msg->list_num);
						goto err_get_ip;
					}
				}
				set_ip_flag();
				//ip_addr=*((unsigned int *)th_addr_array[0].DevIp);
				char *data_buf=alloca(msg->data_len);
				if(data_buf==NULL)
				{
					fprintf(stderr,"alloca for 528 failure!\n");
					goto err_get_ip;
				}
				if(get_list_data(msg->list_num,data_buf)<0)
				{
					fprintf(stderr,"get_list_data %x err",msg->net_cmd);
					goto err_req;
				}
				int i=0;
				int loop=ret;
				for(i=0;i<loop;i++)
				{
					do
					{
						if (try > 3)
						{
							if(ret<0)
							{
								fprintf(stderr, "request tcp_buf\n");
								free_list_data(msg->list_num);
								goto err_req;
							}
							else
							{
								break;
							}
						}
						ret = request_send_buf(&tcp_buf,msg->data_len,TCP_TML);
						try++;
					} while ((ret < 0) && (NULL == tcp_buf));
					if((tcp_buf==NULL)||(ret<0))
					{
						free_list_data(msg->list_num);
						goto err_get_ip;
					}
					sprintf(cmd_pc,"%x",msg->net_cmd);
					tcp_buf->dest_ip=*((unsigned int *)th_addr_array[i].DevIp);
					tcp_buf->length=msg->data_len;
					build_net_package_head(tcp_buf->cmd_buf,msg->data_len,cmd_pc);
					memcpy(tcp_buf->buf,data_buf,msg->data_len);
					int items = ITEM_TWO;
					ret = send_tcp_data(&items,TCP_PC);
					if (ret < 0)
					{
						fprintf(stderr,"tcp_send_data %x err",msg->net_cmd);
						goto err_req;
					}
				}
			}
		}
		else
		{
			if((msg->flag_addr&DATA_MASK)>0){
				ip_addr=get_server_ip();
				if(ip_addr==0){
				    free_list_data(msg->list_num);				
					goto err_get_ip;
				}
			}
			else {
				free_list_data(msg->list_num);
				goto err_get_ip;
			}

			do {
					if (try > 3){
						if(ret<0){
							fprintf(stderr, "request tcp_buf\n");
							free_list_data(msg->list_num);
							goto err_req;
						}
						else
							break;
					}
					ret = request_send_buf(&tcp_buf,msg->data_len,TCP_PC);
					try++;
				} while ((ret < 0) && (NULL == tcp_buf));
				if((tcp_buf==NULL)||(ret<0))
				{
					free_list_data(msg->list_num);
					goto err_get_ip;
				}
				sprintf(cmd_pc,"%x",msg->net_cmd);
				tcp_buf->dest_ip=ip_addr;
				tcp_buf->length=msg->data_len;
				build_net_package_head(tcp_buf->cmd_buf,msg->data_len,cmd_pc);

				if(get_list_data(msg->list_num,tcp_buf->buf)>0)
				{
					int items = ITEM_TWO;
					ret = send_tcp_data(&items,TCP_PC);
					if (ret < 0) {
						app_debug(DBG_INFO,"tcp_send_data %x err",msg->net_cmd);
						goto err_req;
					}
				}
		}


	}
		break;
	case PC_DATA:
	{
		net_data_msg *msg=(net_data_msg*)buf;
		net_cmd=msg->net_cmd;
		app_debug(DBG_INFO,"PC_DATA pro: %x\n",msg->net_cmd);
		net_cmd=msg->net_cmd;
		flag_addr=PC_DATA;
		char cmd_pc[4];

		if((msg->flag_addr&DATA_MASK)>0){
			ip_addr=get_server_ip();
			if(ip_addr==0)
				goto err_get_ip;
		}
		else
		{
			goto err_get_ip;
		}


		int try = 0;
		tcp_data *tcp_buf = NULL;
		try = 0;
		do {
			if (try > 3){
				if(ret<0){
					app_debug(DBG_ERROR, "request tcp_buf\n");
					goto err_req;
				}
				else
					break;
			}
			ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_PC);
			try++;
		}while ((ret < 0) && (NULL == tcp_buf));
		if((tcp_buf==NULL)||(ret<0))
			goto err_get_ip;
		sprintf(cmd_pc,"%x",msg->net_cmd);
		tcp_buf->length =build_net_data(tcp_buf->buf,(char *)msg->buf,msg->data_len,cmd_pc);
		tcp_buf->dest_ip=ip_addr;
		int items = ITEM_ONE;
		ret = send_tcp_data(&items,TCP_PC);
		if (ret < 0) {
			app_debug(DBG_INFO,"tcp_send_data  <542> err");
			goto err_req;
		}
	}
		break;
	case PC_ADDR_RET:
	{
		
		char cmd_pc[4];
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;
		app_debug(DBG_INFO,"ui_%x\n",msg->net_cmd);
		sprintf(cmd_pc,"%x",msg->net_cmd);
		connect_node *net_runtime;
		char cmd_buf[21];
		build_net_package_head(cmd_buf,msg->data_len,cmd_pc);
		char *send_buf=alloca(msg->data_len+10);
		if(msg->data_len>0)
		if(get_list_data(msg->list_num,send_buf)<0)
		{
			app_debug(DBG_INFO,"get_list_data %x err",msg->net_cmd);
				goto err_req;
		}
		if(find_connect_node(msg->num,&net_runtime)==0)
		{
			net_runtime->respond_fun(net_runtime->socketfd,cmd_buf,20);
			net_runtime->respond_fun(net_runtime->socketfd,send_buf,msg->data_len);
			net_runtime->connect_on = NET_STOP;
		}
		else
		{
			goto err_req;
		}
	}
	break;
	case PC_DATA_RET:
	{
		char cmd_pc[4];
		net_data_msg *msg=(net_data_msg*)buf;
		net_cmd=msg->net_cmd;
		sprintf(cmd_pc,"%x",msg->net_cmd);
		connect_node *net_runtime;
		char *send_buf=alloca(msg->data_len+21);
		build_net_package_head(send_buf,msg->data_len,cmd_pc);
		memcpy(send_buf+20,msg->buf,msg->data_len);
		if(find_connect_node(msg->num,&net_runtime)==0)
		{
			net_runtime->respond_fun(net_runtime->socketfd,send_buf,msg->data_len+20);
			net_runtime->connect_on = NET_STOP;
		}
		else
		{
			goto err_req;
		}
	}
	break;
	case SERVER_DATA:
	{
	
		net_data_msg *msg=(net_data_msg*)buf;
		net_cmd=msg->net_cmd;
		app_debug(DBG_INFO,"ui_%x\n",msg->net_cmd);
		net_cmd=msg->net_cmd;

		flag_addr=SERVER_DATA;
		char cmd_pc[4];

		if((msg->flag_addr&DATA_MASK)>0){
			ip_addr=get_server_ip();
			if(ip_addr==0)
				goto err_get_ip;
		}
		else
		{
			goto err_get_ip;
		}

		printf("ip_addr=%X\n",ip_addr);
		int try = 0;
		tcp_data *tcp_buf = NULL;
		try = 0;
		do {
			if (try > 3){
				if(ret<0){
					app_debug(DBG_ERROR, "request tcp_buf\n");
					goto err_req;
				}
				else
					break;
			}
			ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
			try++;
		}while ((ret < 0) && (NULL == tcp_buf));
		if((tcp_buf==NULL)||(ret<0))
			goto err_get_ip;
		sprintf(cmd_pc,"%x",msg->net_cmd);
		tcp_buf->length =build_net_data(tcp_buf->buf,(char *)msg->buf,msg->data_len,cmd_pc);
		tcp_buf->dest_ip=ip_addr;
		int items = ITEM_ONE;
		ret = send_tcp_data(&items,TCP_TML);
		if (ret < 0) {
			app_debug(DBG_INFO,"tcp_send_data  <542> err");
			goto err_req;
		}
	
	}
	break;	
//add for multi_lift_ctl
	case KZQ_LIFT_DATA:
    {
	
		net_data_msg *msg=(net_data_msg*)buf;
		net_cmd=msg->net_cmd;
		printf("KZQ_LIFT_DATA:%x\n",net_cmd);

        if((msg->flag_addr&DATA_MASK)>0)
        {
			ip_addr=get_1st_lift_controller_ip();//第一路网络呼梯
        	if(ip_addr != 0){
                send_lift_ctrl_msg_tcp_process(msg,net_cmd,ip_addr);
        	}
        	
        	ip_addr=get_2nd_lift_controller_ip();//第二路网络呼梯
        	if(ip_addr != 0){
                send_lift_ctrl_msg_tcp_process(msg,net_cmd,ip_addr);
        	}

        	ip_addr=get_3rd_lift_controller_ip();
        	if(ip_addr != 0){
                send_lift_ctrl_msg_tcp_process(msg,net_cmd,ip_addr);
        	}

            ip_addr=get_4th_lift_controller_ip();
        	if(ip_addr != 0){
                send_lift_ctrl_msg_tcp_process(msg,net_cmd,ip_addr);
        	}

            ip_addr=get_5th_lift_controller_ip();
        	if(ip_addr != 0){
                send_lift_ctrl_msg_tcp_process(msg,net_cmd,ip_addr);
        	}        	
        }
	
	}
    	break;	
//end    	

	case PC_ADDR_JSON:
	{
	    app_debug(DBG_INFO,"PC_ADDR_JSON pro!\n");
		int try = 0;
		tcp_data *tcp_buf = NULL;
		char cmd_pc[4];
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;	

		if((msg->flag_addr&DATA_MASK)>0){
			ip_addr=get_server_ip();
			if(ip_addr==0){
			    free_list_data(msg->list_num);
				goto err_get_ip;
			}
		}
		else {
			free_list_data(msg->list_num);
			goto err_get_ip;
		}

		do {
				if (try > 3){
					if(ret<0){
						fprintf(stderr, "request tcp_buf\n");
						free_list_data(msg->list_num);
						goto err_req;
					}
					else
						break;
				}
				ret = request_send_buf(&tcp_buf,msg->data_len,TCP_PC_JSON);
				try++;
		} while ((ret < 0) && (NULL == tcp_buf));
		if((tcp_buf==NULL)||(ret<0))
		{
			free_list_data(msg->list_num);
			goto err_get_ip;
		}
		sprintf(cmd_pc,"%x",msg->net_cmd);
		tcp_buf->dest_ip=ip_addr;
		tcp_buf->length=msg->data_len;
		build_json_net_package_head(tcp_buf->cmd_buf,msg->data_len,cmd_pc,1);

		if(get_list_data(msg->list_num,tcp_buf->buf)>0)
		{
			int items = ITEM_TWO;
			ret = send_tcp_data(&items,TCP_PC_JSON);
			if (ret < 0) {
				app_debug(DBG_INFO,"tcp_send_data %x err",msg->net_cmd);
				goto err_req;
			}
		}
		
	}
    break;
    
	default:
		break;
	}
data_err:
set_ip_flag();
#if (!DEV_GATE)
	app_debug(DBG_INFO,"recv ui data len:%d\n",len);
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, oper_type,ret,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
/*
#else
	app_debug(DBG_INFO,"recv ui data len:%d\n",len);
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,ret,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
*/
	return;
err_get_ip://没有获取到IP

#if DEV_GLJKZQ|DEV_CONTROL|DEV_TSGLJKZQ
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,DEV_OFFINE,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
	return;
err_req:	//获取到IP但是发送失败

#if DEV_GLJKZQ|DEV_CONTROL|DEV_TSGLJKZQ
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,SEND_ERR,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
	return;
}

#else

void ui_data_pro(char *buf,int len)//处理接收到的UI数据
{
	app_debug(DBG_INFO,"recv ui data len:%d\n",len);
	int ret=0;
	unsigned char *dest_No=(unsigned char [4]){};
	unsigned char *src_No = (unsigned char[4]){};
	unsigned int ip_addr = 0;
	unsigned int net_cmd;
	unsigned char cmd;
	short allow=0;
	unsigned int oper_type;
	int dev_type=TYPE_JDM365;
	int resstatus;
	char flag_addr=MEM_DATA;
	proc_msg_ret msg_ret={};
	msg_ret.msg_type=MSG_FROM_NET;
	oper_type = 0x00;
	resstatus = 0x00;

	ajb_msg *msg=(ajb_msg *)buf;

	if(msg->data.net_cmd>0xfff)
		goto data_err;
	switch(buf[0])
	{
	case MEM_GLJ:
	{
		switch(msg->data.net_cmd)
		{
		case GDATC:
		{
			switch(msg->data.cmd)
			{
			case SSSK:
			{
				net_cmd=msg->data.net_cmd;
				oper_type=msg->data.oper_type;
				char cmd_pc[4];
				sprintf(cmd_pc,"%x",msg->data.net_cmd);
				memcpy(dest_No,msg->data.dest_No,4);
				char data[10]={};
				data[0]=msg->data.cmd;
				int data_len=msg->data.src_No[0];
				memcpy(&data[1],&msg->data.src_No[1],data_len);
				connect_node *net_runtime;
				if(find_connect_node(msg->num,&net_runtime)==0)
				{
					int path=TCP_TML;
					int try = 0;
					tcp_data *tcp_buf = NULL;
					try = 0;
					do {
						if (try > 3)
							break;
						ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
						try++;
					} while ((ret < 0) && (NULL == tcp_buf));
					if((tcp_buf==NULL)||(ret<0))
						goto err_get_ip;
					tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);

					tcp_buf->dest_ip=net_runtime->dest_ip;
					int items = ITEM_ONE;
					ret = send_tcp_data(&items,path);
					if (ret < 0) {
						app_debug(DBG_INFO,"tcp_send_data  <%s> err",cmd_pc);
						goto err_req;
					}
				}
				else
					goto err_req;
			}
			break;
			/*
			case SDIS:// 增加
			{
				send_data_to_uipro_cache(buf,len);
				return;
			}
			break;
			*/
			default:
			{
				net_cmd=msg->data.net_cmd;
				oper_type=msg->data.oper_type;
				char cmd_pc[4];
				sprintf(cmd_pc,"%x",msg->data.net_cmd);
				int path=TCP_TML;
				memcpy(dest_No,msg->data.dest_No,4);
				ret = get_dest_ip_by_arp(dest_No,(char *) &ip_addr, ARPINFOREQ,0);
				if (ret < 0) {
					app_debug(DBG_INFO,"dest dev is not online!\n");
					goto err_get_ip;
				}
				char data[10]={};
				data[0]=msg->data.cmd;
				int data_len=msg->data.src_No[0];
				memcpy(&data[1],&msg->data.src_No[1],data_len);
				int try = 0;
				tcp_data *tcp_buf = NULL;
				try = 0;
				do {
					if (try > 3)
						break;
					ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
					try++;
				} while ((ret < 0) && (NULL == tcp_buf));
				if((tcp_buf==NULL)||(ret<0))
					goto err_get_ip;
				tcp_buf->length =build_net_data(tcp_buf->buf,data,data_len+1,cmd_pc);

				tcp_buf->dest_ip=ip_addr;
				int items = ITEM_ONE;
				ret = send_tcp_data(&items,path);
				if (ret < 0) {
					app_debug(DBG_INFO,"tcp_send_data  <%s> err",cmd_pc);
					goto err_req;
				}
			}
			break;
			}
		}
		break;
		}
	}
	break;
	case MEM_DATA:
	{
		if (msg->data.cmd == 0x88) { //返回状态
			//ajb_msg_ret_split(&net_cmd, &cmd, &allow, &oper_type, &resstatus,&flag_addr,(ajb_msg_ret*)msg);
			ajb_msg_ret_split_whole(&net_cmd, &cmd, &allow, &oper_type, &resstatus,&dev_type,&flag_addr,(ajb_msg_ret*)msg);
		} else
		{
			//ajb_msg_split(&net_cmd, &cmd, &dest_No, &src_No,&flag_addr,msg);
			ajb_msg_split_whole(&net_cmd, &cmd, &dest_No, &src_No,&oper_type,&resstatus,&flag_addr,msg);
			app_debug(DBG_INFO,"net_cmd:0x%.3x oper_type:0x%.4x resstatus:0x%.4x!\n",net_cmd,oper_type,resstatus);
		}
		flag_addr=MEM_DATA;
		switch (net_cmd) {
		case NTELC:
		{
			app_debug(DBG_INFO,"ui_704\n");
			int try=0;
			ntel_data call_data;
			ret=get_dest_ip_by_arp(dest_No,(char *)&ip_addr,ARPCALLREQ,0);
			if (ret <= 0) {
				unsigned char *dest_No_2=(unsigned char [4]){};
				memcpy(dest_No_2,dest_No,4);
				dest_No_2[2]=0;
				dest_No_2[3]=0;
				ret=get_dest_ip_by_arp(dest_No_2,(char *)&ip_addr,ARPCALLREQ,0);
				if (ret < 0){
					app_debug(DBG_INFO,"dest dev is not online!\n");
					goto err_get_ip;
				}
				if(is_mydev_ip(ip_addr))
				{
					goto err_get_ip;
				}
			}
			tcp_data *buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==buf));
			if((buf==NULL)||(ret<0))
				goto err_get_ip;

			memcpy(call_data.dest_addr,dest_No,4);
			memcpy(call_data.src_addr,src_No,4);
			call_data.oper_type=oper_type;
			call_data.resstaus=resstatus;
			//buf->length=build_call_data(buf->buf,&call_data,(unsigned char*)"704");
			buf->length=build_call_data_j365(buf->buf,&call_data,(unsigned char*)"704");
			buf->dest_ip=ip_addr;
			int items=ITEM_ONE;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data <704> err");
				goto err_req;
			}
			add_call_item(call_data.dest_addr,ip_addr);
//#if DEV_GLJKZQ
			oper_type=get_com_devno();
			if(memcmp(call_data.dest_addr,&oper_type,4)!=0){
				add_call_item((unsigned char *)&oper_type,ip_addr);
			}
//#endif
		}
		break;
		case RTELC:
		{
			app_debug(DBG_INFO,"ui_705  %d  0x%x dev_type:0x%d\n",msg->num,msg->num,dev_type);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
            	switch(dev_type)
            	{
            	case TYPE_JDM365:
				case TYPE_J926:	
            		send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"705");
            		break;
            	case TYPE_D2100:
            		send_len=build_rcall_data_bytype(&cmdbuf,&rcall_data,RTEL,RES_2100);
            		break;
            	default:
            		send_len=build_rcall_data(&cmdbuf,&rcall_data,"705");
            		break;
            	}
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case NENDC:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			app_debug(DBG_INFO,"ui_708\n");
			ntel_data call_data;
			int try=0;
			ret=get_ip_from_call_list(dest_No,&ip_addr);
			if(ret<0)
			{
				ret=get_dest_ip_by_arp(dest_No,(char *)&ip_addr,ARPCALLREQ,0);
				if (ret < 0) {
					app_debug(DBG_INFO,"dest dev is not online!\n");
					goto err_get_ip;
				}
			}
			memcpy(call_data.dest_addr,dest_No,4);
			memcpy(call_data.src_addr,src_No,4);
			call_data.oper_type=oper_type;//0x00000004L;
			call_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			tcp_data *tcp_buf;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
				goto err_get_ip;
			tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"708");
			tcp_buf->dest_ip=ip_addr;
			int items=ITEM_ONE;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data <708> err");
				goto err_req;
			}
		}
		break;
		case RENDC:
		{
#if DEV_GLJKZQ|DEV_TSGLJKZQ
			set_devstatus(DEV_IDLE);
#endif
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;//0x00000004L;
			rcall_data.resstaus=resstatus;//AUDIONRES|VIDEORES;
			int send_len=0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"709");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case NANSC:
		{
			app_debug(DBG_INFO,"ui_710  %d  0x%x\n",msg->num,msg->num);
			ntel_data call_data;

			ret=get_ip_from_call_list(dest_No,&ip_addr);
			if(ret<0)
			{
				ret=get_dest_ip_by_arp(dest_No,(char *)&ip_addr,ARPCALLREQ,0);
				if (ret < 0) {
					app_debug(DBG_INFO,"dest dev is not online!\n");
					goto err_get_ip;
				}
			}else
				app_debug(DBG_INFO,"get_ip_from_call_list!\n");

			memcpy(call_data.dest_addr,dest_No,4);
			memcpy(call_data.src_addr,src_No,4);
			call_data.oper_type=oper_type;
			call_data.resstaus=resstatus;
			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
				goto err_get_ip;

			tcp_buf->length=build_call_data(tcp_buf->buf,&call_data,(unsigned char*)"710");
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data  <710> err");
				goto err_req;
			}
		}
		break;
		case RANSC:
		{
			app_debug(DBG_INFO,"ui_711  %d  0x%x\n",msg->num,msg->num);
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow=allow;
			rcall_data.oper_type=oper_type;
			rcall_data.resstaus=resstatus;
			int send_len=0;

			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				send_len=build_rcall_data_j365(&cmdbuf,&rcall_data,"711");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case GDATC:{
			app_debug(DBG_INFO,"ui_518\n");
			int path=TCP_TML;
			if((flag_addr&DATA_MASK)>0)
			{
				ip_addr=get_server_ip();
				path=TCP_PC;
			}else{
				ret = get_dest_ip_by_arp(dest_No,(char *) &ip_addr, ARPINFOREQ, 0);
				if (ret < 0) {
					app_debug(DBG_INFO,"dest dev is not online!\n");
					goto err_get_ip;
				}
			}
/*			ntel_data call_data;
			memcpy(call_data.dest_addr, dest_No, 4);
			memcpy(call_data.src_addr, src_No, 4);
			call_data.oper_type = oper_type;
			call_data.resstaus = resstatus;*/
			int try = 0;
			tcp_data *tcp_buf = NULL;
			try = 0;
			do {
				if (try > 3)
					break;
				ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,path);
				try++;
			} while ((ret < 0) && (NULL == tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
				goto err_get_ip;
			tcp_buf->length =build_net_data(tcp_buf->buf,&msg->data.cmd,10,(char*) "518");
			tcp_buf->dest_ip=ip_addr;
			int items = ITEM_ONE;
			ret = send_tcp_data(&items,path);
			if (ret < 0) {
				app_debug(DBG_INFO,"tcp_send_data  <518> err");
				goto err_req;
			}
		}
			break;
		case RGDAC:
		{
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow = allow;
			rcall_data.oper_type = 0;
			rcall_data.resstaus = 0;
			int send_len = 0;
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{

/*				if(get_dest_dev_type_by_ip(net_runtime->dest_ip)==TYPE_JDM365)
					send_len =build_rcall_data_j365(&cmdbuf, &rcall_data,"519");
				else*/
				send_len =build_rcall_data(&cmdbuf, &rcall_data,"519");
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case SJYDC:
		{
			unsigned char *cmdbuf=(unsigned char [40]){};
			rtel_data rcall_data;
			rcall_data.allow = allow;
			rcall_data.oper_type = 0;
			rcall_data.resstaus = 0;
			int send_len = build_rcall_data(&cmdbuf, &rcall_data,"521");
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		case GDSTC:
		{
			app_debug(DBG_INFO,"ui_536  %d  0x%x\n",msg->num,msg->num);
			ret=get_dest_ip_by_arp(dest_No,(char *)&ip_addr,ARPINFOREQ,0);
			if (ret < 0) {
				app_debug(DBG_INFO,"dest dev is not online!\n");
				goto err_get_ip;
			}
			int try=0;
			tcp_data *tcp_buf=NULL;
			try=0;
			do{
				if(try>3)
					break;
				ret=request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_TML);
				try++;
			}while((ret<0)&&(NULL==tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
				goto err_get_ip;

			tcp_buf->length=build_net_data(tcp_buf->buf,&dest_No[0],4,GDST);
			int items=ITEM_ONE;
			tcp_buf->dest_ip=ip_addr;
			ret=send_tcp_data(&items,TCP_TML);
			if(ret<0)
			{
				app_debug(DBG_INFO,"tcp_send_data  <536> err");
				goto err_req;
			}
		}
		break;
		case RDSTC:
		{
			app_debug(DBG_INFO,"ui_537  %d  0x%x\n",msg->num,msg->num);
			unsigned char *cmdbuf=(unsigned char [40]){};
			char buf[10];
			memcpy(buf,dest_No,4);
			buf[4]=cmd;
			//int send_len=build_call_data(cmdbuf,buf,(unsigned char*)RDST);
			int send_len=build_net_data(cmdbuf,buf,5,RDST);
			connect_node *net_runtime;
			if(find_connect_node(msg->num,&net_runtime)==0)
			{

				app_debug(DBG_INFO,"ui_537_2  %d  0x%x\n",msg->num,msg->num);
				net_runtime->respond_fun(net_runtime->socketfd,cmdbuf,send_len);
				net_runtime->connect_on = NET_STOP;
			}
			else
				goto err_req;
		}
		break;
		default:
			break;
		}
	}
	break;
	case MEM_ADDR:
	{
		int try = 0;
		tcp_data *tcp_buf = NULL;
		char cmd_buf[4];
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;

		switch(net_cmd)
		{
		case WXLYC:
		{
			memcpy(dest_No,&msg->num,4);
			ret = get_dest_ip_by_arp(dest_No,(char *) &ip_addr, ARPCALLREQ, 0);
			if (ret < 0) {
				app_debug(DBG_INFO,"dest dev is not online!\n");
				free_list_data(msg->list_num);
				goto err_get_ip;
			}
			do {
				if (try > 3){
					if(ret<0){
						app_debug(DBG_ERROR, "request tcp_buf\n");
						free_list_data(msg->list_num);
						goto err_req;
					}
					else
						break;
				}
				ret = request_send_buf(&tcp_buf,msg->data_len,TCP_TML);
				try++;
			} while ((ret < 0) && (NULL == tcp_buf));
			if((tcp_buf==NULL)||(ret<0))
			{
				free_list_data(msg->list_num);
				goto err_get_ip;
			}
			sprintf(cmd_buf,"%x",msg->net_cmd);
			tcp_buf->dest_ip=ip_addr;
			tcp_buf->length=msg->data_len;
			build_net_package_head(tcp_buf->cmd_buf,msg->data_len,cmd_buf);

			if(get_list_data(msg->list_num,tcp_buf->buf)>0)
			{

				int items = ITEM_TWO;
				ret = send_tcp_data(&items,TCP_TML);
				if (ret < 0) {
					app_debug(DBG_INFO,"tcp_send_data %x err",msg->net_cmd);
					goto err_req;
				}
			}
		}
		break;
		default:
			break;
		}

	}
	break;
	case PC_ADDR:
	{
		int try = 0;
		tcp_data *tcp_buf = NULL;
		char cmd_pc[4];
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;
		if(net_cmd==WXLYC){
			if(msg->num!=DEV_NO_NULL)
			{
				ret = get_dest_ip_by_arp((unsigned char *)&msg->num,(char *) &ip_addr, ARPCALLREQ, 0);
				if (ret < 0) {
					app_debug(DBG_INFO,"dest dev is not online!\n");
					free_list_data(msg->list_num);
					goto err_get_ip;
				}
			}
		}
		else
		{
			if((msg->flag_addr&DATA_MASK)>0){
				ip_addr=get_server_ip();
				if(ip_addr<=0)
					goto err_get_ip;
			}
			else {
				free_list_data(msg->list_num);
				goto err_get_ip;
			}
		}

		do {
			if (try > 3){
				if(ret<0){
					app_debug(DBG_ERROR, "request tcp_buf\n");
					free_list_data(msg->list_num);
					goto err_req;
				}
				else
					break;
			}
			ret = request_send_buf(&tcp_buf,msg->data_len,TCP_PC);
			try++;
		} while ((ret < 0) && (NULL == tcp_buf));
		if((tcp_buf==NULL)||(ret<0))
		{
			free_list_data(msg->list_num);
			goto err_get_ip;
		}
		sprintf(cmd_pc,"%x",msg->net_cmd);
		tcp_buf->dest_ip=ip_addr;
		tcp_buf->length=msg->data_len;
		build_net_package_head(tcp_buf->cmd_buf,msg->data_len,cmd_pc);

		if(get_list_data(msg->list_num,tcp_buf->buf)>0)
		{
			int items = ITEM_TWO;
			ret = send_tcp_data(&items,TCP_PC);
			if (ret < 0) {
				app_debug(DBG_INFO,"tcp_send_data %x err",msg->net_cmd);
				goto err_req;
			}
		}
	}
		break;
	case PC_DATA:
	{
		net_data_msg *msg=(net_data_msg*)buf;
		net_cmd=msg->net_cmd;
		app_debug(DBG_INFO,"ui_%x\n",msg->net_cmd);
		net_cmd=msg->net_cmd;
		flag_addr=PC_DATA;
		char cmd_pc[4];

		if((msg->flag_addr&DATA_MASK)>0){
			ip_addr=get_server_ip();
			if(ip_addr<=0)
				goto err_get_ip;
		}
		else {
			goto err_get_ip;
		}


		int try = 0;
		tcp_data *tcp_buf = NULL;
		try = 0;
		do {
			if (try > 3){
				if(ret<0){
					app_debug(DBG_ERROR, "request tcp_buf\n");
					goto err_req;
				}
				else
					break;
			}
			ret = request_send_buf(&tcp_buf,NET_BUF_SIZE,TCP_PC);
			try++;
		}while ((ret < 0) && (NULL == tcp_buf));
		if((tcp_buf==NULL)||(ret<0))
			goto err_get_ip;
		sprintf(cmd_pc,"%x",msg->net_cmd);
		tcp_buf->length =build_net_data(tcp_buf->buf,(char *)msg->buf,msg->data_len,cmd_pc);
		tcp_buf->dest_ip=ip_addr;
		int items = ITEM_ONE;
		ret = send_tcp_data(&items,TCP_PC);
		if (ret < 0) {
			app_debug(DBG_INFO,"tcp_send_data  <542> err");
			goto err_req;
		}
	}
		break;
	case PC_ADDR_RET:
	{
		char cmd_pc[4];
		net_addr_msg *msg=(net_addr_msg*)buf;
		net_cmd=msg->net_cmd;
		app_debug(DBG_INFO,"ui_%x\n",msg->net_cmd);
		sprintf(cmd_pc,"%x",msg->net_cmd);
		connect_node *net_runtime;
		char cmd_buf[21];
		build_net_package_head(cmd_buf,msg->data_len,cmd_pc);
		char *send_buf=alloca(msg->data_len+10);
		if(msg->data_len>0)
		if(get_list_data(msg->list_num,send_buf)<0)
		{
			app_debug(DBG_INFO,"get_list_data %x err",msg->net_cmd);
				goto err_req;
		}
		if(find_connect_node(msg->num,&net_runtime)==0)
		{
			net_runtime->respond_fun(net_runtime->socketfd,cmd_buf,20);
			net_runtime->respond_fun(net_runtime->socketfd,send_buf,msg->data_len);
			net_runtime->connect_on = NET_STOP;
		}
		else
		{
			goto err_req;
		}
	}
	break;
	case PC_DATA_RET:
	{
		char cmd_pc[4];
		net_data_msg *msg=(net_data_msg*)buf;
		net_cmd=msg->net_cmd;
		sprintf(cmd_pc,"%x",msg->net_cmd);
		connect_node *net_runtime;
		char *send_buf=alloca(msg->data_len+21);
		build_net_package_head(send_buf,msg->data_len,cmd_pc);
		memcpy(send_buf+20,msg->buf,msg->data_len);
		if(find_connect_node(msg->num,&net_runtime)==0)
		{
			net_runtime->respond_fun(net_runtime->socketfd,send_buf,msg->data_len+20);
			net_runtime->connect_on = NET_STOP;
		}
		else
		{
			goto err_req;
		}
	}
	break;
	default:
		break;
	}
data_err:
#if (!DEV_GATE)
	app_debug(DBG_INFO,"recv ui data len:%d\n",len);
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, oper_type,ret,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
/*
#else
	app_debug(DBG_INFO,"recv ui data len:%d\n",len);
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,ret,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
*/
	return;
err_get_ip://没有获取到IP
#if DEV_GLJKZQ|DEV_CONTROL|DEV_TSGLJKZQ
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,DEV_OFFINE,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
	return;
err_req:	//获取到IP但是发送失败
#if DEV_GLJKZQ|DEV_CONTROL|DEV_TSGLJKZQ
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, 0,SEND_ERR,flag_addr);
	send_data_to_proc(&msg_ret, MSG_LEN_PROC);
#endif
	return;
}
#endif



#ifdef CFG_SUPPORT_INFO_STORE
void ui_udp_data_pro(char *buf,int len)//处理接收到的UI数据
{
	app_debug(DBG_INFO,"recv ui_udp_data len:%d\n",len);
	int ret=0;
	unsigned int net_cmd=0;
	unsigned int oper_type=0;
	int resstatus;
	char flag_addr=MEM_DATA;
	proc_msg_ret msg_ret={};
	msg_ret.msg_type=MSG_FROM_NET;
	oper_type = 0x00;
	resstatus = 0x00;
	net_addr_msg *msg=(net_addr_msg *)buf;
	switch(buf[0])
	{
	case GLJ_RQ_RET:
	case GLJ_RQ_ADDR:
	{
		//net_addr_msg *msg=(net_addr_msg*)buf;;
		net_cmd=msg->net_cmd;
		char send_buf[msg->data_len+100];
		app_debug(DBG_INFO,"get_list_data %x ",msg->net_cmd);
		if(get_list_data(msg->list_num,send_buf)<0)
		{
			app_debug(DBG_INFO,"get_list_data %x err",msg->net_cmd);
			ret=-1;
			goto data_err;
		}
		char *send_ptr=send_buf;
		socket_addr_t ipaddr;
		memcpy(&ipaddr,send_ptr,sizeof(socket_addr_t));
		send_ptr+=sizeof(socket_addr_t);
		ret=send_data_to_udp_node(send_ptr,sizeof(DevList)+1,ipaddr.addr,ipaddr.port);
	}
	break;
	default:
		free_list_data(msg->list_num);
		break;
	}
	return;
data_err:
	ajb_msg_ret_build(&msg_ret.msg, net_cmd, SCODE, 0, oper_type,ret,flag_addr);
	send_udp_data_to_ui(&msg_ret, MSG_LEN_PROC);//UDP发送数据到 UI
	return;
}
#endif
