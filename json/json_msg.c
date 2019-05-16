/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name               	: to_sip_server.c
* Author               	   : Ritchie Tang
* Version               	   : V1.0.0
* Date               	       : 2018-1-18
* Description               :
* Modify by               	:
* Modify date              :
* Modify description    :
******************************************************************************************/
#include <time.h>
#include <fcntl.h>
#include "public/common.h"
#include "cjson.h"
#include "json_msg.h"
#include "dev_info.h"
#include "dev_config.h"
#include "sqlite_data.h"
#include "base64.h"
#include "net_data_pro.h"
#include "cmd_def.h"

#define STR_COMMAND(NAME)  { #NAME, NAME}

const event_type_t  event_type_list[] =  {   
   STR_COMMAND(0),              //无效位
   STR_COMMAND(VISITOR_PHOTO), //访客留影 1
   STR_COMMAND(MAGNETIC_PHOTO), //门磁拍录 2
   STR_COMMAND(MISSED_VISITOR_PHOTO), //呼叫无人接听拍照 3
   STR_COMMAND(VISITOR_VOICE_MESSAGE), //访客留言 4
   STR_COMMAND(CARD_UNLOCK_RECORD), //刷卡开锁成功 5
   STR_COMMAND(CARD_UNLOCK_FAIL_RECORD), //刷卡开锁失败 6
   STR_COMMAND(BULETOOTH_UNLOCK_RECORD), //蓝牙开锁成功 7
   STR_COMMAND(BULETOOTH_UNLOCK_FAIL_RECORD), //蓝牙开锁失败 8
   STR_COMMAND(FINGERPRINT_UNLOCK_RECORD), //指纹开锁成功 9
   STR_COMMAND(FINGERPRINT_UNLOCK_FAIL_RECORD), //指纹开锁失败 10
   STR_COMMAND(QR_UNLOCK_RECORD), //二维码开锁成功 11
   STR_COMMAND(QR_UNLOCK_FAIL_RECORD), //二维码开锁失败 12
   STR_COMMAND(FACE_UNLOCK_RECORD), //人脸开锁成功 13
   STR_COMMAND(FACE_UNLOCK_FAIL_RECORD), //人脸开锁失败14		
   STR_COMMAND(PASSWORD_UNLOCK_RECORD),  //密码开锁成功15
   STR_COMMAND(PASSWORD_UNLOCK_FAIL_RECORD),  //密码开锁失败16   
   STR_COMMAND(INTERCOM_UNLOCK_RECORD),  //对讲开锁成功17 
   STR_COMMAND(INTERCOM_UNLOCK_FAIL_RECORD),  //对讲开锁失败18   
   STR_COMMAND(PUBLIC_PASSWORD_UNLOCK_RECORD),  //公共密码开锁成功19   
   STR_COMMAND(PUBLIC_PASSWORD_UNLOCK_FAIL_RECORD),  //公共密码开锁失败20  
};	

//added by hgj 180529
int get_time_string(char *dst_buff, int max_sz)
{
	int ret = 0;
	
	time_t now;
	struct tm *tm_now;

	if(assert_ptr(dst_buff)) {
		return -1;
	}
	
	time(&now);
	tm_now = localtime(&now);

	snprintf(dst_buff, max_sz, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tm_now->tm_year, 1 + tm_now->tm_mon, \
															tm_now->tm_mday,  tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

	//dbg_lo("dst_buff=%s\n", dst_buff);
	return ret;
}

blob_byte_t make_enter_record_json(enter_record_t *enter_record)
{
	cJSON *root = NULL, *item = NULL;
	char *out = NULL;
	blob_byte_t jdata = {0, NULL};
	
	if (assert_ptr(enter_record)) {
		return jdata;
	}

	root = cJSON_CreateObject();

	if (assert_ptr(root)) {
		return jdata;
	}
    app_debug(DBG_INFO,"device_id = %s,event_type = %s,crendence_id = %s\n",enter_record->device_id,\
        enter_record->event_type,enter_record->crendence_id);
    app_debug(DBG_INFO,"timestamp = %d,user_id = %s,picture size = %d\n",enter_record->timestamp,enter_record->usr_id,\
        strlen(enter_record->picture));
    
	cJSON_AddItemToObject(root, "data", item = cJSON_CreateObject());

	cJSON_AddStringToObject(item, "device_id", 	enter_record->device_id);
	cJSON_AddStringToObject(item, "event_type", 	enter_record->event_type);	
	cJSON_AddStringToObject(item, "crendence_id", 	enter_record->crendence_id);	
	cJSON_AddNumberToObject(item, "timestamp", 	enter_record->timestamp);	
	cJSON_AddStringToObject(item, "user_id", 	enter_record->usr_id);
	cJSON_AddStringToObject(item, "picture", 	enter_record->picture);

	out = cJSON_Print(root);
	cJSON_Delete(root);
	jdata.data = out;

	return jdata;
}

int send_enter_record_to_server(EVENT_TYPE_T event_type,char * crendence_id,char *usr_id,char *pic_data,int  pic_len)
{
#ifndef _PHOTO_UPLOAD
    return -1;
#endif
	int ret = 0;
	blob_byte_t jdata = {0, NULL};
	int jsize = 0;
    enter_record_t	enter_record={};
    
	dbg_lo("%s\n", __func__);
	if (assert_ptr(crendence_id)||assert_ptr(pic_data)) {
		return -1;
	}	

	printf("pic_len=%d\n",pic_len);
    base64_encode_jpg_buffer(pic_data, pic_len, (uint8_t *)&enter_record.picture);	
	printf("base64 size = %d\n",strlen(enter_record.picture));

	snprintf(enter_record.device_id,sizeof(enter_record.device_id),"%02x%02x%02x%02x",\
	    dev_cfg.my_code[0],dev_cfg.my_code[1],dev_cfg.my_code[2],dev_cfg.my_code[3]);
    snprintf(enter_record.crendence_id,sizeof(enter_record.crendence_id),"%s",crendence_id);
    snprintf(enter_record.usr_id,sizeof(enter_record.usr_id),"%s",usr_id);    
	snprintf(enter_record.event_type,sizeof(enter_record.event_type),"%s",event_type_list[event_type].string);	
	//ret = get_time_string(enter_record.timestamp, sizeof(enter_record.timestamp));
    enter_record.timestamp = time(NULL);
    
	jdata = make_enter_record_json(&enter_record);
	jsize = strlen(jdata.data);
	dbg_lo("jsize=%d\n", jsize);

	send_json_addr_to_server(JSON_UPLOAD_C,jdata.data,jsize);
	ret = release_blob_byte(&jdata);
	if(NULL != enter_record.picture) {
		free(enter_record.picture);
		enter_record.picture = NULL;
	}
	free(pic_data);

	return ret;
}


int send_face_enter_record_to_server(EVENT_TYPE_T event_type,char * crendence_id,char * const pic_data,int pic_len)
{
#ifndef _PHOTO_UPLOAD
    return -1;
#endif
	int ret = 0;
	blob_byte_t jdata = {0, NULL};
	int jsize = 0;
    enter_record_t	enter_record={};
    
	dbg_lo("%s\n", __func__);
	if (assert_ptr(crendence_id)||assert_ptr(pic_data)) {
		return -1;
	}	

	printf("pic_len=%d\n",pic_len);
    base64_encode_jpg_buffer(pic_data, pic_len, (uint8_t *)&enter_record.picture);	
	printf("base64 size = %d\n",strlen(enter_record.picture));

	snprintf(enter_record.device_id,sizeof(enter_record.device_id),"%02x%02x%02x%02x",\
	    dev_cfg.my_code[0],dev_cfg.my_code[1],dev_cfg.my_code[2],dev_cfg.my_code[3]);
    snprintf(enter_record.crendence_id,sizeof(enter_record.crendence_id),"%s",crendence_id);
    memcpy(enter_record.usr_id,crendence_id,8);
	snprintf(enter_record.event_type,sizeof(enter_record.event_type),"%s",event_type_list[event_type].string);		
    enter_record.timestamp = time(NULL);
    
	jdata = make_enter_record_json(&enter_record);
	jsize = strlen(jdata.data);
	dbg_lo("jsize=%d\n", jsize);

	send_json_addr_to_server(JSON_UPLOAD_C,jdata.data,jsize);
	ret = release_blob_byte(&jdata);
	if(NULL != enter_record.picture) {
		free(enter_record.picture);
		enter_record.picture = NULL;
	}

	return ret;
}

void upload_record_pc(EVENT_TYPE_T event_type)
{
	int  pic_len = 0;
    char *pic_data = NULL;
	char str[5]={0};
	char usr_id[10]={0};
    if(fkly_backup(&pic_data, &pic_len)){
        printf("[err] shoot_visitor_photo fail!\n");
    }    
	 send_enter_record_to_server(event_type,str,usr_id,pic_data,pic_len);
	printf("------1--------");
}


