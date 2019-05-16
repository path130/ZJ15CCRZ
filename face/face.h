#ifndef _FACE_H_
#define _FACE_H_

#define	AJB_FACE_QUERY_PORT									(10888)
#define	AJB_FACE_REGISTER_PORT							(10882)

#pragma pack(1)
typedef struct{
        unsigned short head;//0xff53 fix
        unsigned int package_len;  //len+10
	unsigned short cmd;//3001(Dec) fix
	unsigned char devno[4];//global_data.h
}jpeg_tcp_head_t;
#pragma pack()

void face_det_start();
void face_det_stop();
int face_start(void *arg);
void restart_algo_auto_sw(void);//added by hgj 180522

#endif