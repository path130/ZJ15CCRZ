/*
 * ring_buf.h
 *
 *  Created on: 2012-5-23
 *      Author: gyt
 */

#ifndef RING_BUF_H_
#define RING_BUF_H_

#include <stdio.h>
#include <pthread.h>
#include "public.h"

typedef struct{
	void *buf;     				//环形缓冲区数据存储空间
	int buf_size;  				//环形缓冲区每一个节点的容量
	int buf_nums;  				//环形缓冲区每一个节点的数量
	int ring_put;  				//环形缓冲区写指针位置
	int ring_get;  				//环形缓冲区读指针位置
	int ring_n;    				//环形缓冲区有效数据节点数量
	int ring_mutex_en;   		//环形缓冲区线程锁使能
	pthread_mutex_t ring_mutex; //环形缓冲区线程锁
}ring_buf_s;


int ring_buf_creat(ring_buf_s *buf_handle,int per_buf_size,int buf_count);
void ring_buf_delete(ring_buf_s *buf_handle);
#ifdef CFG_APP_USE_CMEM
cmem_handle ring_cmem_creat(ring_buf_s *buf_handle,int per_buf_size,int buf_count);
void ring_cmem_delete(cmem_handle h_cmem, ring_buf_s *buf_handle);
#endif
unsigned char *get_write_pos(ring_buf_s *buf_handle);// 获取向环形缓冲区中插入元素的地址
unsigned char * get_read_pos(ring_buf_s *buf_handle);// 获取从环形缓冲区中读取元素的地址
#endif /* RING_BUF_H_ */
