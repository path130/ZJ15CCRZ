/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name               	: json_com.h
* Author               	   : Ritchie Tang
* Version               	   : V1.0.0
* Date               	       : 2017-12-13
* Description               : 
* Modify by               	: 
* Modify date              : 
* Modify description    : 
******************************************************************************************/
#ifndef __JSON_COM_H
#define	__JSON_COM_H

#include "cjson.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct cJSON cjson;

typedef struct{
	int length;
	unsigned char *data;
}blob_byte_t;

#define	json_get_obj_item(x,y) 		cJSON_GetObjectItem(x,y)
#define	json_get_array_size(x) 		cJSON_GetArraySize(x)
#define	json_get_array_item(x,y) 	cJSON_GetArrayItem(x,y)

int json_get_int(cjson *object,const char *string, int *value);
int json_get_string(cjson *object, const char *string, char **value);
int release_blob_byte(blob_byte_t *src);


#if defined (__cplusplus)
}
#endif

#endif
