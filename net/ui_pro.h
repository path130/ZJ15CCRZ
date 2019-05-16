/*
 * ui_pro.h
 *
 *  Created on: 2012-9-20 上午9:26:59
 *  
 */

#ifndef UI_PRO_H_
#define UI_PRO_H_

#define		VISITINFO		0	//查询访客信息
#define		ENTRAINFO		1	//查询进出记录
#define		CARDINFO		3	//查询刷卡记录
#define		PRIVATINFO		4  	//分机查询个人信息
#define		PUBLICINFO		5	//分机查询共公信息
#define		FKLYINFO		6	//访客留言

typedef struct
{
	unsigned char FJNo[2];//拜访那家
	unsigned char FKVideo;// 1.访客信息 或当前第几个访客 其它
	unsigned char Date[3];//年月日
	unsigned char Time[3];
	unsigned char Card[4];
}video_info;

void net_data_process(void *buf,int len,pipe_handle);
void make_call(unsigned char *devdest);
void call_speech(unsigned char *devdest);
void call_end(unsigned char *devdest);
void send_cmd_518(char cmd,unsigned char *devdest);
void send_cmd_548();
void call_speech(unsigned char *devdest);

#endif /* UI_PRO_H_ */
