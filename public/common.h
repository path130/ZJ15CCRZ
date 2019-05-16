/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: common.h
* Author				: Ritchie
* Version				: V1.0.0
* Date					: 2012??7??06??
* Description			: 
* Modify by				: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>   

#include "my_types.h"
#include "my_debug.h"
#include "./public.h"


//#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
//#endif

#ifndef READONLY
#define	READONLY			const
	//#define		READONLY		
#endif
void print_n_byte(const uint8_t * const str, const int len);
uint8_t summation(const uint8_t *const src, const int lenth);
int check_sum(const uint8_t * const src, const int length);
int find_str_in_buf(const uint8_t *const str, int slen, const uint8_t * const key, int klen);
#if 0
int hex_to_string(uint8_t hex, uint8_t *dest);

/***-----------------------------------------------------------------***/
int thread_attr_init(pthread_attr_t *attr);
int get_thread_policy(pthread_attr_t *attr);
int show_thread_priority(pthread_attr_t *attr, int policy);
int get_thread_priority(pthread_attr_t *attr);
int get_thread_max_priority(int policy);
int get_thread_min_priority(int policy);
int set_thread_policy(pthread_attr_t *attr, int policy);
int set_thread_priority(pthread_attr_t *attr, int priority);
int thread_attr_destroy(pthread_attr_t *attr);
/***-----------------------------------------------------------------***/
#endif
void ntohll(const uint64_t src_dword, uint64_t *const dst_dword);

#if 1


/* Error message */
#define ERR(fmt, args...)   fprintf(stderr, "Error: " fmt, ## args)

/* Function error codes */
#define SUCCESS              0
#define FAILURE             -1


/* Align buffers to this cache line size (in bytes)*/
#define BUFSIZEALIGN        128
/* The input buffer height alignment restriction imposed by codecs */
#define CODECHEIGHTALIGN    16

#define CODEC_FREE          0x1
#define DISPLAY_FREE        0x2


#endif

#endif
