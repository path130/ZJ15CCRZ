/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name               	: msg_to_sip.h
* Author               	   : Ritchie Tang
* Version               	   : V1.0.0
* Date               	       : 2018-1-18
* Description               : 
* Modify by               	: 
* Modify date              : 
* Modify description    : 
******************************************************************************************/
#ifndef __JSON_MSG_H
#define	__JSON_MSG_H

#include "json_com.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define	MAX_URL_COUNT								(10)
#define	MAX_URL_SIZE								(256)


typedef struct{
    char device_id[10];
    char event_type[40];	
    char crendence_id[50];
    int timestamp;
    char usr_id[10];
    char *picture;
}enter_record_t;

typedef enum{
    VISITOR_PHOTO = 1, //访客留影 1
    MAGNETIC_PHOTO, //门磁拍录 2
    MISSED_VISITOR_PHOTO, //呼叫无人接听拍照 3
    VISITOR_VOICE_MESSAGE, //访客留言 4
    CARD_UNLOCK_RECORD, //刷卡开锁成功 5
    CARD_UNLOCK_FAIL_RECORD, //刷卡开锁失败 6
    BULETOOTH_UNLOCK_RECORD, //蓝牙开锁成功 7
    BULETOOTH_UNLOCK_FAIL_RECORD, //蓝牙开锁失败 8
    FINGERPRINT_UNLOCK_RECORD, //指纹开锁成功 9
    FINGERPRINT_UNLOCK_FAIL_RECORD, //指纹开锁失败 10
    QR_UNLOCK_RECORD, //二维码开锁成功 11
    QR_UNLOCK_FAIL_RECORD, //二维码开锁失败 12
    FACE_UNLOCK_RECORD, //人脸开锁成功 13
    FACE_UNLOCK_FAIL_RECORD, //人脸开锁失败14
    PASSWORD_UNLOCK_RECORD,  //密码开锁成功15
    PASSWORD_UNLOCK_FAIL_RECORD,  //密码开锁失败16
    INTERCOM_UNLOCK_RECORD,			//对讲开锁成功17
    INTERCOM_UNLOCK_FAIL_RECORD,	//对讲开锁失败18
    PUBLIC_PASSWORD_UNLOCK_RECORD,	//公共密码开锁成功19
    PUBLIC_PASSWORD_UNLOCK_FAIL_RECORD	//公共密码开锁失败20    
}EVENT_TYPE_T;

typedef struct{
    char  *string;
    int   id;    
}event_type_t;

extern int send_enter_record_to_server(EVENT_TYPE_T event_type,char * crendence_id,char *usr_id,char *pic_data,int  pic_len);
extern int send_face_enter_record_to_server(EVENT_TYPE_T event_type,char * crendence_id,char * const pic_data,int pic_len);
extern void upload_record_pc(EVENT_TYPE_T event_type);	//add at 190516


#if defined (__cplusplus)
}
#endif

#endif
