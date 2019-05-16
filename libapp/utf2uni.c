/*
 ============================================================================
 Name        : utf2uni.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : Change UTF code to unicode
			   把UTF8转换成对应UNICODE
  ============================================================================
 */

#include "stdio.h"
#include "utf2uni.h"

/*
* U-00000000 - U-0000007F: 0xxxxxxx  
* U-00000080 - U-000007FF: 110xxxxx 10xxxxxx  
* U-00000800 - U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx  
* U-00010000 - U-001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
* U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
* U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
* 开头1的数目为编码连续字节数,实现到U-0000FFFF足以
*/
int get_utf8_bytes(const unsigned char *txt) 
{
	unsigned char mask = 0x80;
  	int bytes = 0;

	while (*txt & mask) {
		bytes++;
		mask>>=1;
	}

	return (bytes==0 ? 1 : bytes);
}

/*
* 返回UTF8对应的unicode码
*/
unsigned short utf8_trans_unicode(const char *txt, int bytes) 
{
	unsigned short uni = 0x0000;
	unsigned short uni_h, uni_l;
	switch (bytes) {
		case 1:
			uni = (unsigned short)*txt;
			break;
		case 3:
			uni_h = ((*txt & 0x0F) << 4) | ((*(txt+1) >> 2) & 0x0F);
            uni_l = ((*(txt+1) & 0x03) << 6) | (*(txt+2) & 0x3F);
			uni   = uni_h << 8 | uni_l;
			break;
		default:
			break;
	}

	return uni;
}

