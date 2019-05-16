/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name            	: base64.c
* Author               	: Ritchie Tang
* Version              	: V1.0.0
* Date                 	: 2018-1-31
* Description          	:
* Modify by            	:
* Modify date          	:
* Modify description   	:
******************************************************************************************/
#include <fcntl.h>
#define	DEBUG_ERROR
#include "public/common.h"
#include "base64.h"
#include "json_com.h"

extern int fkly_backup(char **ptr, int *len);

static const char *base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(const unsigned char *bindata, char *base64, int binlength)
{
	int i, j;
	unsigned char current;

	for (i = 0, j = 0 ; i < binlength ; i += 3) {
		current = (bindata[i] >> 2) ;
		current &= (unsigned char)0x3F;
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)(bindata[i] << 4)) & ((unsigned char)0x30) ;

		if (i + 1 >= binlength) {
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}

		current |= ((unsigned char)(bindata[i + 1] >> 4)) & ((unsigned char) 0x0F);
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)(bindata[i + 1] << 2)) & ((unsigned char)0x3C) ;

		if (i + 2 >= binlength) {
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			break;
		}

		current |= ((unsigned char)(bindata[i + 2] >> 6)) & ((unsigned char) 0x03);
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)bindata[i + 2]) & ((unsigned char)0x3F) ;
		base64[j++] = base64char[(int)current];
	}

	base64[j] = '\0';
	return base64;
}

int base64_decode(const char *base64, unsigned char *bindata)
{
	int i, j;
	unsigned char k;
	unsigned char temp[4];

	for (i = 0, j = 0; base64[i] != '\0' ; i += 4) {
		memset(temp, 0xFF, sizeof(temp));

		for (k = 0 ; k < 64 ; k ++) {
			if (base64char[k] == base64[i])
			{ temp[0] = k; }
		}

		for (k = 0 ; k < 64 ; k ++) {
			if (base64char[k] == base64[i + 1])
			{ temp[1] = k; }
		}

		for (k = 0 ; k < 64 ; k ++) {
			if (base64char[k] == base64[i + 2])
			{ temp[2] = k; }
		}

		for (k = 0 ; k < 64 ; k ++) {
			if (base64char[k] == base64[i + 3])
			{ temp[3] = k; }
		}

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC)) |
		               ((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03));

		if (base64[i + 2] == '=')
		{ break; }

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
		               ((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F));

		if (base64[i + 3] == '=')
		{ break; }

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
		               ((unsigned char)(temp[3] & 0x3F));
	}

	return j;
}

/**********************************************************************************
*Function name	: OPEN_FILE_WITH_FD
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int open_file_with_fd(char *file_name, int *file_sz)
{
	int ret = 0;
	int fd = -1;
	
	if(assert_ptr(file_name)) {
		return -1;
	}

	fd = open(file_name, O_RDONLY, 0);
	if(fd < 0) {
		dbg_perror("open:");
		dbg_err("open %s failed\n", file_name);
		return -1;
	}

	do{
		ret = lseek(fd, 0, SEEK_END);
		if(ret < 0) {
			dbg_perror("lseek:");
			break;
		}
		if(NULL != file_sz) {
			*file_sz = ret;
		}
		
		ret = lseek(fd, 0, SEEK_SET);
		if(ret < 0) {
			dbg_perror("lseek:");
			break;
		}
	}while(0);

	if(ret < 0) {
		close(fd);
		fd = -1;
	}
	
	return fd;
}

/**********************************************************************************
*Function name	: READ_JPG_FILE
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int read_jpg_file(char *file_name, uint8_t **mem_buff)
{
	int ret = 0;
	int fd = -1, file_sz = 0;
	uint8_t *buffer = NULL;

	if(assert_ptr(file_name) || assert_ptr(mem_buff)) {
		return -1;
	}
	
	fd = open_file_with_fd(file_name, &file_sz);
	if(ret < 0 || file_sz < 0) {
		dbg_err("\n");
		return fd;
	}

	buffer = (uint8_t*)malloc(file_sz);
	if(assert_ptr(buffer)) {
		dbg_perror("malloc:");
		close(fd);
		fd = -1;
		return -1;
	}

	ret = read(fd, buffer, file_sz);
	if(ret < 0) {
		dbg_perror("read:");
		free(buffer);
		buffer = NULL;
		return ret;
	}

	*mem_buff = buffer;
	
	return ret;
}

/**********************************************************************************
*Function name	: BASE64_ENCODE_JPG_FILE
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int base64_encode_jpg_file(char *file_name, uint8_t **base64)
{
	int ret = 0;
	int fd = -1, raw_sz = 0, base64_sz = 0;
	uint8_t *raw_ptr = NULL, *base64_ptr = NULL;

	
	if(assert_ptr(file_name) || assert_ptr(base64)) {
		return -1;
	}
	*base64 = NULL;
	
	raw_sz = read_jpg_file(file_name, &raw_ptr);
	if(raw_sz < 0) {
		return -1;
	}

	base64_sz = raw_sz * 2;

	base64_ptr = (uint8_t *)malloc(base64_sz);
	if(assert_ptr(base64_ptr)) {
		dbg_perror("malloc:");
		free(raw_ptr);
		return -1;
	}

	base64_encode(raw_ptr, base64_ptr, raw_sz);

	if(raw_ptr != NULL) {
		free(raw_ptr);
	}

	*base64 = base64_ptr;
	
	return ret;
}


/**********************************************************************************
*Function name	: BASE64_ENCODE_JPG_BUFFER
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int base64_encode_jpg_buffer(char *src_data, int src_len, uint8_t **base64)
{
	int ret = 0;
	int fd = -1,  base64_sz = 0;
	uint8_t *raw_ptr = NULL, *base64_ptr = NULL;

	
	if(assert_ptr(src_data) || assert_ptr(base64)) {
		return -1;
	}
	*base64 = NULL;
	raw_ptr = src_data;

	base64_sz = src_len * 2;

	base64_ptr = (uint8_t *)malloc(base64_sz);
	if(assert_ptr(base64_ptr)) {
		dbg_perror("malloc:");
		return -1;
	}

	base64_encode(raw_ptr, base64_ptr, src_len);

	*base64 = base64_ptr;
	
	return ret;
}

int test_base64_encode(void)
{
	char *file_name = "black.jpg";
	int ret = 0;
	uint8_t *base64 = NULL;

	ret = base64_encode_jpg_file(file_name, &base64);

	if(ret < 0 || assert_ptr(base64)) {
		return ret;
	}

	int i = 0;
	
	dbg_lo("base64 is:\n");
	for(i=0; i<20; i++) {
		dbg_print("%c", base64[i]);
	}
	dbg_print("\n");

	if(NULL != base64) {
		free(base64);
	}
	
	return ret;
}

