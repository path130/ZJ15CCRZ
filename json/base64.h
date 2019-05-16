/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name            	: base64.h
* Author               	: Ritchie Tang
* Version              	: V1.0.0
* Date                 	: 2018-1-31
* Description          	: 
* Modify by            	: 
* Modify date          	: 
* Modify description   	: 
******************************************************************************************/
#ifndef __BASE64_H
#define	__BASE64_H
#if defined (__cplusplus)
extern "C" {
#endif

int base64_encode_jpg_file(char *file_name, uint8_t **base64);
int base64_decode(const char *base64, unsigned char *bindata);
int open_file_with_fd(char *file_name, int *file_sz);
int base64_encode_jpg_file(char *file_name, uint8_t **base64);
int base64_encode_jpg_buffer(char *src_data, int src_len, uint8_t **base64);

#if defined (__cplusplus)
}
#endif

#endif
