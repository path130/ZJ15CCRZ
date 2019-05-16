/*
 ============================================================================
 Name        : finger.c
 Author      : zalebool
 Version     : V0.01
 Copyright   : 
 Description :  finger proc
 Created Time: Wed 01 Jun 2017 01:54:59 PM HKT
 ============================================================================
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "public.h"
#include "cpld.h"
#include "msg.h"
#include "ui.h"
#include "auto_test.h"
#include "tim.h"
#include "uart.h"

#include "finger.h"
#include "cpld.h"
#include "json_msg.h"


static pthread_t   finger_tid;
static pipe_handle finger_pipe_finger;


static int          tim_fg_timeout;
static int   tim_uart0_timeout;

void proc_fp_timeout(int arg1,int arg2);
extern void uart_send_lift_data(unsigned char *data);

global_data  gbl_finger_status = GBL_DATA_INIT;
unsigned char *p_Packet = NULL;

void pause_fp_polling(void)
{
    fp_status_set(status_non);
    usleep(10*1000);//确保前一刻外发的指纹轮询包发送完毕
    tim_reset_time(tim_uart0_timeout, TIME_250MS(3));
}

void proc_uart0_timeout()//by mkq finger
{
printf("proc_uarto_timeout!!!!!!!!!!!!!!!!\n");
    uart0_switch_baud(115200);  
    Set_ZW_switch(1);
    fp_status_set(status_getimg);
}

void SendCmdPacket(unsigned int DeviceAddress,
	unsigned char Packet_pid,
	unsigned int length,
	unsigned char CmdCode,
	unsigned char *p_data)
	{
	int i ;
	unsigned int data_len;
	unsigned int CheckSum;
	unsigned int p_Packet_len = 0;
	memset(&comm_buff,0,sizeof(comm_buff));
	p_Packet = (unsigned char	 *)&comm_buff;


	comm_buff.new_trans.head[0] = 0xEF;
	comm_buff.new_trans.head[1] = 0x01;

	comm_buff.new_trans.address[0] = DeviceAddress&0xFF;
	comm_buff.new_trans.address[1] = (DeviceAddress>>8)&0xFF; 
	comm_buff.new_trans.address[2] = (DeviceAddress >>16)&0xFF;
	comm_buff.new_trans.address[3] = (DeviceAddress >>24)&0xFF;

	comm_buff.new_trans.sign = Packet_pid;
       
	comm_buff.new_trans.length[1] = length&0xFF;
	comm_buff.new_trans.length[0] = (length>>8)&0xFF;

	comm_buff.new_trans.command = CmdCode;

	data_len = length;
       //printf("the ...%02X %02X\n",comm_buff.new_trans.length[0],comm_buff.new_trans.length[1]);
	if(data_len-3>0)
		memcpy(comm_buff.new_trans.fdata,p_data,data_len);
	CheckSum = 0;

	p_Packet = (unsigned char	 *)&comm_buff.new_trans.sign;

	for (i=0; i<=length+2-2; i++)
	{
	       //printf("%02X\n",*p_Packet);
		CheckSum += *p_Packet++;
	}
	comm_buff.new_trans.fdata[data_len-2] =   CheckSum&0xFF;
	comm_buff.new_trans.fdata[data_len-3] =  (CheckSum>>8)&0xFF;

      //debug ......
     // if(CmdCode != C_GetImage){
       //printf("sum :%02x %02x %04X %d\n",comm_buff.new_trans.fdata[data_len+7] ,comm_buff.new_trans.fdata[data_len+8],CheckSum,data_len );

     /* app_debug(DBG_INFO,"the Packet:%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",comm_buff.new_trans.head[0],comm_buff.new_trans.head[1],\
	   	comm_buff.new_trans.address[0],comm_buff.new_trans.address[1],comm_buff.new_trans.address[2],comm_buff.new_trans.address[3],\
	   	comm_buff.new_trans.sign,comm_buff.new_trans.length[0],comm_buff.new_trans.length[1],comm_buff.new_trans.command,\
	   	comm_buff.new_trans.fdata[0],comm_buff.new_trans.fdata[1]);
     */
 #if 0
      app_debug(DBG_INFO,"Send the Packet:");
	p_Packet = (unsigned char	 *)&comm_buff;
	for(i = 0;i<length+9;i++)
		 app_debug(DBG_INFO,"%02X ",*p_Packet++);
	app_debug(DBG_INFO,"\n");
#endif	
//}
	p_Packet = (unsigned char	 *)&comm_buff;
       p_Packet_len = length +9;

	uart_send_data_fg_ex(p_Packet,p_Packet_len);
	
}
void fp_get_image(void){
	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x03, C_GetImage, NULL);
}

void fp_generate_template(unsigned char  ramBuffer){
       unsigned char buf[1] = {0};
	buf[0] = ramBuffer;
	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x04, C_GenTemplet, buf);
}

void fp_search_template(unsigned char rambuff,unsigned int start_Page,unsigned int Page_nu){
	unsigned char start_pagec[2] = {0},page_nuc[2] ={0};
	unsigned char buf[5] = {0};
	
	start_pagec[1] = start_Page&0xFF;
	start_pagec[0] = (start_Page>>8)&0xFF;

	page_nuc[1] = Page_nu&0xFF;
	page_nuc[0] = (Page_nu>>8)&0xFF;

	buf[0] = rambuff;
	buf[1] = start_pagec[0] ;
	buf[2] = start_pagec[1] ;

	buf[3] = page_nuc[0] ;
	buf[4] = page_nuc[1]; 

	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x08, C_Search, buf);
}

void fp_merge_template(void){

	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x03, C_MergeTwoTemplet, NULL);
}

void fp_store_lib(unsigned char rambuff, unsigned char Template_h,unsigned char Template_l){
	unsigned char buf[3] = {0};
	
	buf[0] = rambuff;
	buf[1] = Template_h;
	buf[2] = Template_l;
	
	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x06, C_StoreTemplet, buf);
}
void fp_empty_all(void){
	
	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x03, C_EraseAllTemplet, NULL);
}

void fp_readcolist(unsigned char page_nu){
       unsigned char buf[1] = {0};
	buf[0] = page_nu;
	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x04, C_ReadConList, buf);
}
int  fp_delete_char(unsigned int template_id,unsigned char templet_count){
	unsigned char template_idc[2] = {0},templet_countc[2] = {0};
	unsigned char buf[4] ={ 0};

	if(template_id  >= fp_device_capacity) return -1;
	
	template_idc[1] = template_id&0xFF;
	template_idc[0] = (template_id/0xFF)&0XFF;

	templet_countc[1] = templet_count&0xFF;
	templet_countc[0] =  (templet_count/0xFF)&0XFF;

	buf[0] = template_idc[0];
	buf[1] = template_idc[1];
	buf[2] = templet_countc[0];
	buf[3] = templet_countc[1];

       printf("eht %04x %02x %02x %02x %02x\n",template_id,buf[0],buf[1],buf[2],buf[3]);
	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x07, C_DeletChar, buf);

	return 0;
}

void fp_auto_reg_templet(unsigned char wait_time,unsigned char wait_count,
	unsigned char STemplate_h,unsigned char STemplate_l,unsigned char repeat_flag){

	unsigned char buf[5] ={ 0};
	buf[0] = wait_time;
	buf[1] = wait_count;
	buf[2] = STemplate_h,
	buf[3] = STemplate_l,
	buf[4] = repeat_flag,

	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x08, C_AotoSave, buf);

}
void fp_auto_search_templet(unsigned char wait_time,unsigned char start_nu_h,
	unsigned char start_nu_l,unsigned char search_nu_h,unsigned search_nu_l){
	unsigned char buf[5] ={ 0};
	buf[0] = wait_time;
	buf[1] = start_nu_h;
	buf[2] = start_nu_l,
	buf[3] = search_nu_h,
	buf[4] = search_nu_l,

	SendCmdPacket(DEVICE_ADDRESS,PACKET_PID, 0x08, C_AotoSearch, buf);
}

void Downloadtemplet(void){
	unsigned char buf[10];
	buf[0] = 0x01;
	buf[1] = 0x00;
	buf[2] = 0x04;
	buf[3] = 0x09;
	buf[4] = 0x01;
	buf[5] = 0x00;
	buf[6] = 0x00;
	//uart_send_data_fg(UART_0, buf, 13);
	printf("Downloadtemplet .....\n");
}
/*******读取表格数据 *************/

int Read_conlist(unsigned char *index,int page){
	int i=0,j=0,temp=0;		
	int free_id = -1;
	for(i=0;i<32;i++){
		for(j=0;j<8;j++){
			//printf("index[%d] = %d\n",i,index[i]);
			temp=(index[i]>>j)&0x01;
			if(temp==0){ 
				free_id = i*8+j+page*256;
				return free_id;
				}
			}
		}
	return free_id;
}		

long get_file_size(char *path)
{
   unsigned long filesiz = 0;
   FILE *fp = fopen(path, "r");
   if (fp == NULL) 
   	return filesiz;
    fseek(fp, 0, SEEK_END);
    filesiz = ftell(fp);
    fclose(fp);
   return filesiz;
}

void fp_status_set(int status)
{
    gbl_data_set(&gbl_finger_status, status);
   app_debug(DBG_INFO,"set finger_status:%02X\n",status);
    
}
int fp_status_get()
{
    return gbl_data_get(&gbl_finger_status);
}

void fp_time_int(void)
{
	
    tim_fg_timeout   = tim_set_event(TIME_1S(0), (tim_callback)proc_fp_timeout, 0, 0, TIME_ONESHOT);
}
/*
void proc_fp_timeout(int arg1,int arg2){

    int status = fp_status_get();
    switch(status){
	   case  status_getimg:
		ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
		break;
	   case  status_reg_second:
	   case  status_reg_first:
	   case  status_reg_ok:
	   	fp_status_set(status_index);
		fp_readcolist(0x00);
		break;
        default:
            break;
	}
}
*/

void finger_ok_pro(unsigned char *buf)
{
	switch(fp_status_get()){
		case status_getimg:
			{
			  printf("Input fingerprint successful!\n");
			  fp_generate_template(CharBufferA);
			  fp_status_set(status_img2tz);
			  printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
			  break;
			  }
		case status_img2tz:
			{
			      printf("Image generation feature successful!\n");
                         	fp_status_set(status_non);
				usleep(50*1000);
			    	fp_search_template(CharBufferA, 0x00, fp_device_capacity);
			    	fp_status_set(status_serarch);
			    	printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
				break;
			        }
		case status_format:
			{
				 printf("empty the finger templet OK!\n");
				ui_action(UI_FINGER_FORMAT_OK,0);
				break;
			}
		case status_delect:
			{
				  	printf("delect OK!\n");
					ui_action(UI_FINGER_DELECT_OK,0);
					break;
				  }

		case status_download:
			{
				  	printf("Ready to download!-----------\n");
					ui_action(UI_FINGER_DOWNLOADING_OK,0);
					fp_status_set(status_downloading);
					break;
				  }
		case status_downloading:
			{
				  	printf("downloading......\n");
					/*
					if(sizedb => 256){
						uart_send_data_fg(UART_0, fgtemplet_pointer, 256+9);
						sizedb = sizedb - 256;
						fgtemplet_pointer += 256;
					}
					uart_send_data_fg(UART_0, fgtemplet_pointer, sizedb);
					*/
					break;
				  }
		case status_reg_second:
			{
				  	printf("OK ! REG printfinger ...............\n");
					ui_action(UI_FINGER_REG_STOREOK,0);
					fp_status_set(status_reg_ok);
					tim_reset_time(tim_fg_timeout, TIME_1S(3));
					break;
				  }
		case status_serarch:

		{
				  	printf("ERROR......status_serarch......again.......\n");
					fp_search_template(CharBufferA, 0x00, fp_device_capacity);
					//ui_action(UI_FINGER_REG_ERROR,0);
					break;
				  }
		case status_index:
			{
				  	printf("ERROR......status_serarch......again.......\n");

					fp_readcolist(0x00);
			              //fp_status_set(status_index);
			              ui_action(UI_FINGER_REG_READ,0);
			              //ui_scene_sw(SCENE_REG_FINGER);
					//ui_action(UI_FINGER_REG_ERROR,0);
					break;
				  }
		default:
			break;
			
	}
}

void finger_data_error(void){
	
	
}

void no_finger_on_serson(void){
	switch(fp_status_get()){
		case status_getimg:
			//fp_status_set(status_getimg);
			break;
		case status_format:
			printf("empty the finger templet failed!\n");
			ui_action(UI_FINGER_FORMAT_NG,0);
			break;
		case status_delect:
			printf("empty the finger templet failed!\n");
			ui_action(UI_FINGER_DELECT_FAILD,0);
			break;
		case status_reg_first:
		case status_reg_second:
			printf("Not found finger!!EXIT REG!\n");
			ui_action(UI_FINGER_REG_NOFINGER,0);
			fp_status_set(status_reg_timeout);
			//fp_get_image();
			tim_reset_time(tim_fg_timeout, TIME_1S(1));
			break;
		case status_img2tz:
			printf("ERROR status_img2tz!--------\n");
			fp_status_set(status_getimg);
			break;
		case status_index:
			printf("ERROR status_index!---%02x-----\n",fp_status_get());
			fp_readcolist(0x00);
			break;
		case status_serarch:
			printf("ERROR......status_serarch....%02x.........\n",fp_status_get());
			fp_search_template(CharBufferA, 0x00, fp_device_capacity);
			break;
		default:
			break;

	}
}

void fp_not_clean(void){
	switch(fp_status_get()){
		case status_img2tz:
			printf("the img is not clear \n");
			ui_action(UI_FINGER_IMGNOTCLEAR,0);
			fp_status_set(status_getimg);
			break;
		case status_reg:
			printf("the img is not clear \n");			
			ui_action(UI_FINGER_IMGNOTCLEAR,0);
			fp_status_set(status_getimg);
			printf("Error 06 gbl_finger_stastus :%d\n",gbl_finger_status.data);
			break;
		case status_reg_first:
		case status_reg_second:
			printf("the img is not clear \n");			
			ui_action(UI_FINGER_REG_FAILD,0);
			//tim_reset_time(tim_fg_timeout, TIME_1S(2));
			   printf("Error 11 gbl_finger_stastus :%d\n",gbl_finger_status.data);
			break;
		default:
			break;

	}
}


void fp_finger_found(unsigned char *data){
	int finger_quality = -1;
	char a[8]={},str[5]={0};
	char usr_id[10]={0};
	printf("the fingerprint ID found!\n");
	finger_quality = data[12]*256+data[13];
	finger_ID=data[10]*256+data[11];
			
	printf("the Quality : %d\n",data[12]*255+data[13]);
	printf("the finger_id is %d\n",finger_ID);
			
	a[0] = 0x1B;
	a[2] = 0x01;
	a[3] = 0x01;
	//buzz_play(BUZZ_BEEP);
	uart_send_lift_data(a);//呼梯                   		
	ui_action(UI_FINGER_UNLOCK, 0);	
	upload_record_pc(FINGERPRINT_UNLOCK_RECORD);
    //snprintf(str,5,"%d",finger_ID);
    //send_enter_record_to_server(FINGERPRINT_UNLOCK_RECORD,str,usr_id); 	
	printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
		
}
void start_store_fp(unsigned char *data){
	int StoreID32H = 0x00,StoreID32L = 0x00;
	static int PageNum = 0x00;
	int  StoreID = -1; 
	printf("index OK!!\n");
	if(PageNum <= 0X03)
		StoreID = Read_conlist(&data[10], PageNum);
	if(StoreID < fp_device_capacity){
		if(StoreID >= 0){
			printf("StoreID :%d\n",StoreID);
			PageNum = 0x00;
			if(StoreID > 0xFF){
				StoreID32H = StoreID/256;
				StoreID32L = StoreID%256;
			}
			else{
				StoreID32H  = 0X00;
				StoreID32L  = StoreID;
			}
				finger_regID = StoreID;
				printf("StoreID32H:%02X StoreID32L:%02X\n",StoreID32H,StoreID32L);
				//Storetemplet(StoreID32H,StoreID32L);
				//fp_store_lib(fp_store_lib,StoreID32H,StoreID32L);
				fp_auto_reg_templet(fp_wait_time_5s,fp_collect_2time,StoreID32H,StoreID32L,fp_same_save_unen);
				//fp_status_set(status_store);
				fp_status_set(status_reg_first);
				ui_action(UI_FINGER_REG_READ,0);
				printf("REG...........Please input you fringer!!\n");
				printf("~~~~~~~~bl_finger_stastus :%d\n",gbl_finger_status.data);
			}
			else{
				PageNum++;
				printf("Num = %d",PageNum);
				if(PageNum <= 3){
					fp_readcolist(PageNum);
					fp_status_set(status_index);
					printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
					return;
						}
						else{
							printf("omg!@!!!!\n");
							PageNum = 0x00;
							ui_action(UI_FINGER_NO_EMPTY,0);
						}
					}
					}
					else{
						printf("Sorry,there no emtpy !!!fp_device_size %d \n",fp_device_capacity);
						ui_action(UI_FINGER_NO_EMPTY,0);
						PageNum = 0x00;
					}
}

void fp_ack_ok_proc(void){
				if(fp_status_get()== status_reg_first){
					printf("OK!Please input again \n");
					//ui_action(UI_FINGER_DELECT_FAILD,0);
					//fp_status_set(status_regetimg);
					ui_action(UI_FINGER_REG_READAGAIN,0);
					fp_status_set(status_reg_second);
			    		printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
					
				}
				else if(fp_status_get()== status_index){
					printf("OK!Please input again \n");
					ui_action(UI_FINGER_REG_FAILD,1);
					tim_reset_time(tim_fg_timeout, TIME_1S(2));
					fp_status_set(status_reg_first);
					//fp_status_set(status_regetimg);
					printf("Error 59 gbl_finger_stastus :%d\n",gbl_finger_status.data);
				}
				else if(fp_status_get()== status_getimg){
					printf("ERROR \n");
					//fp_get_image();
					printf("Error 56 gbl_finger_stastus :%d\n",gbl_finger_status.data);
				}
				
}

void  fp_duplicate_proc(void){
{
				if(fp_status_get()== status_reg_second){
					printf("Sorry ! your finger had reg!!! \n");
					ui_action(UI_FINGER_HAD_REGD,0);
					tim_reset_time(tim_fg_timeout, TIME_1S(2));
			    		printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
					
				}
				if(fp_status_get()== status_getimg){
					printf("exit ERROR!!! \n");
					//fp_get_image();
			    		printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
				}
			}
}
/************20170615****/
void proc_fpdata_uart(unsigned char *data)
{
	int RECV_ADDRESS = 0,data_length = 0,CheckSum = 0,recvsum = 0;
	int  i = 0; 
	unsigned char *p_Rpacket = NULL;

       memset(&comm_buff,0,sizeof(comm_buff));
	p_Rpacket = (unsigned char	 *)&comm_buff.new_rev.head[0];
	
	RECV_ADDRESS = (data[2]<<24)+(data[3]<<16)+(data[4]<<8)+data[5];

	//app_debug(DBG_INFO, "DEVICE_ADDRESS OK!!\n");
       if(RECV_ADDRESS != DEVICE_ADDRESS)   return;

	comm_buff.new_rev.head [0] =  data[0];
	comm_buff.new_rev.head [1] =  data[1];
	
	comm_buff.new_rev.address [0] =  data[2];
	comm_buff.new_rev.address [1] =  data[3];
	comm_buff.new_rev.address [2] =  data[4];
	comm_buff.new_rev.address [3] =  data[5];

	 
	comm_buff.new_rev.sign = data[6];
	comm_buff.new_rev.length[0] = data[7];
	comm_buff.new_rev.length[1] = data[8];
	
	data_length = comm_buff.new_rev.length[0]*0xFF + comm_buff.new_rev.length[1] ;

       comm_buff.new_rev.respond = data[9];

	//printf( "finger data_length :0x%02X \n",data_length);
	
	if(data_length-3>=0)
		memcpy(comm_buff.new_rev.fdata,&data[10],data_length);

	p_Rpacket = (unsigned char	 *)&comm_buff.new_rev.sign;

	for (i=0; i<=data_length; i++)
	{
	       //printf("%02X\n",*p_Rpacket);
		CheckSum += *p_Rpacket++;
	}
       
       recvsum = data[data_length + 7]*0x100 + data[data_length +8];
	   
	p_Rpacket = (unsigned char	 *)&comm_buff.new_rev.head[0];
#if 0
	app_debug(DBG_INFO,"Recv The Packet:");
	for(i = 0;i<data_length+9;i++)
		app_debug(DBG_INFO,"%02X ",*p_Rpacket++);
	app_debug(DBG_INFO,"\n");
#endif
	
	//printf("recvsum OK!!%02X  %02X  %02X  %02X\n",data[data_length +7],\
	//	data[data_length +8],recvsum,CheckSum);
	
	if(recvsum !=CheckSum ){ 
	    printf("CheckSum err,recvsum=0x%08x,CheckSum=0x%08x !\n",recvsum,CheckSum);
	    return ;
	}


	//app_debug(DBG_INFO,"recvsum == CheckSum  OK!!\n");
	
	switch(comm_buff.new_rev.sign)
	{
	case ACK_PACKET:
		//printf( "ACK_PACKET !\n");
		if(data_length == 0x03){
			//printf("data_length = %d,comm_buff.new_rev.respond:%02X\n",\
			//	data_length,comm_buff.new_rev.respond);
			switch(comm_buff.new_rev.respond){
				case FINGER_OK:
					finger_ok_pro(data);
					break;
				case GET_IMAGEOK_FIRST:
				case GET_IMAGEOK_SECOND:
					fp_ack_ok_proc();
					break;
				case ERR_DUP_FINGER:
					fp_duplicate_proc();
					break;
					
				case ERR_PACKET:
					fp_status_set(status_getimg);
					
					printf( "error packet!\n");
					break;
			       case  ERR_NO_FINGER:
				   	no_finger_on_serson();
					//app_debug(DBG_INFO, "no finger on the chip!\n");
					break;
				case ERR_GET_IMG:
				  	if(fp_status_get()== status_getimg){
						printf("Unsuccessful fingerprint input!\n");		
						ui_action(UI_FINGER_NOINPUT,0);	
						fp_status_set(status_getimg);
						//fp_get_image();
						printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);		
						
				  	}
					fp_status_set(status_getimg);
					printf("fingerprint input failed!\n");
					break;
				case ERR_IMG_NOT_CLEAN:
				case ERR_IMG_NOT_ENOUGH:
					printf("the img is not clear or little feature ERR:%d!\n",comm_buff.new_rev.respond);
					fp_not_clean();
					break;
				case ERR_NOT_MATCH:
					break;
				case ERR_GEN_CHAR_FAIL:
			       case ERR_PAGE_RANGE:
					{
					if(fp_status_get()== status_reg_second){
					printf("Sorry ! !!!合成失败! \n");
					ui_action(UI_FINGER_REG_MODEL_FAILD,0);
					tim_reset_time(tim_fg_timeout, TIME_1S(2));
			    		printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
					
				}
				else if(fp_status_get()== status_getimg){
					printf("0x0A ERROR \n");
					//fp_get_image();
					fp_status_set(status_getimg);
					printf("Error 0A gbl_finger_stastus :%d\n",gbl_finger_status.data);
				}
			}
					break;
				case ERR_NO_IMG:
					{
				  	if(fp_status_get()== status_img2tz){
				  	printf("the img buf is no valid!\n");
					ui_action(UI_FINGER_NOVALIDIMG,0);
					fp_status_set(status_getimg);
					//fp_get_image();
			    		printf("gbl_finger_stastus :%d\n",gbl_finger_status.data);
				  	}
				      }
					break;
				default:
					fp_status_set(status_getimg);
					break;
				}
		}
		else if(data_length == 0x07){
			app_debug(DBG_INFO,"data_length = %d,comm_buff.new_rev.respond:%02X\n",\
				data_length,comm_buff.new_rev.respond);
			switch(comm_buff.new_rev.respond){
				case FINGER_OK:
					fp_finger_found(data);//找到匹配的指纹
                    sleep(1);//等待串口呼梯包发送完毕
					fp_status_set(status_getimg);
					break;
				case ERR_PACKET:
					fp_status_set(status_getimg);
					printf("error packet!\n");
					break;
				case ERR_SEARCH://未找到匹配的指纹
					ui_action(UI_NO_FEINGERTP, 1);
					printf("ERR_SEARCH ,the fingerprint ID not found!!\n");
					fp_status_set(status_getimg);

					break;
				default:
					fp_status_set(status_getimg);
					break;
			}
		}
		else if(data_length == 0x23){
			app_debug(DBG_INFO,"data_length = %d,comm_buff.new_rev.respond:%02X\n",\
				data_length,comm_buff.new_rev.respond);

			switch(comm_buff.new_rev.respond){
				case FINGER_OK:
					start_store_fp(data);
					printf("Read Conlist ok!\n");
					break;
				case ERR_PACKET:
					fp_status_set(status_getimg);
					printf("error packet!\n");
					break;
				default:
					fp_status_set(status_getimg);
					break;
			}
		}
		break;
	case CMD_PACKET:
		printf("CMD_PACKET !\n");
		break;
	case DATA_PACKET:
		printf("DATA_PACKET !\n");
		break;
	case ENDDATA_PACKET:
		printf("ENDDATA_PACKET !\n");
		break;
	default:
		break;
	}
}



static void * finger_thread(void *arg)
{
	unsigned char norm_delay = 1;
	unsigned char io_status = 0,io_l = 0;
	int ret = 0;
	while(1){
	 if (norm_delay)
            usleep(600*1000);
        else
            norm_delay = 1;
		if(fp_status_get() == status_getimg){
			fp_get_image();
			}
		
		}		
	return THREAD_FAILURE;
}

void finger_start(void *arg){
    struct msg_t *p_msg = (struct msg_t *)arg;
    finger_pipe_finger = p_msg->pipe_audio; 
#ifdef _FP_MODULE     
    pthread_create(&finger_tid, NULL, finger_thread, arg);
#endif    
    fp_status_set(status_getimg);
    tim_uart0_timeout   = tim_set_event(TIME_1S(0), (tim_callback)proc_uart0_timeout, 0, 0, TIME_ONESHOT);//by mkq finger    
}

