/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name               	: json_com.c
* Author               	   : Ritchie Tang
* Version               	   : V1.0.0
* Date               	       : 2017-12-13
* Description               : 
* Modify by               	: 
* Modify date              : 
* Modify description    : 
******************************************************************************************/

#define	DEBUG_ERROR
#include "public/common.h"
#include "json_com.h"


/**********************************************************************************
*Function name	: JSON_GET_INT
*Description		: 
*Input para		: 
*Output para		: 
*Return			   : 
*Modify by		   :
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int json_get_int(cjson *object,const char *string, int *value)
{
	int intval = 0;
	cJSON *target = NULL;

	if(assert_ptr(object) || assert_ptr(string) || assert_ptr(value)) {
		return -1;
	}

	target = cJSON_GetObjectItem(object, string);
	if(assert_ptr(target)) {
		dbg_err("%s not found\n", string);
		return -1;
	}

	*value = target->valueint;

	return 0;
}

/**********************************************************************************
*Function name	: JSON_GET_STRING
*Description		: 
*Input para		: 
*Output para		: 
*Return			   : 
*Modify by		   :
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int json_get_string(cjson *object, const char *string, char **value)
{
	int intval = 0;
	cJSON *target = NULL;

	if(assert_ptr(object) || assert_ptr(string) || assert_ptr(value)) {
		return -1;
	}

	target = cJSON_GetObjectItem(object, string);
	if(assert_ptr(target)) {
		dbg_err("%s not found\n", string);
		return -1;
	}

	*value = target->valuestring;

	return 0;
}


/**********************************************************************************
*Function name	: RELEASE_BLOB_BYTE
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int release_blob_byte(blob_byte_t *src)
{
	if(assert_ptr(src) || assert_ptr(src->data)) {
		return -1;
	}

	free(src->data);
	src->data = NULL;

	return 0;
}

