/*
 * ring_buf.c
 *
 *  Created on: 2012-5-23
 *      Author: gyt
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ring_buf.h"
#include "public.h"


int ring_buf_creat(ring_buf_s *buf_handle,int per_buf_size,int buf_count)
{
    buf_handle->buf_nums = buf_count;
    buf_handle->buf_size = per_buf_size;
    buf_handle->buf=(unsigned char *)malloc(per_buf_size*buf_count);

    if(buf_handle->buf==NULL) {
        printf("malloc mem for ring_buf failure !");
        return -1;
    }
    if (pthread_mutex_init(&buf_handle->ring_mutex, NULL) != 0) {
        perror("pthread_mutex_init!");
        return -1;
    }

    buf_handle->ring_mutex_en=1;
    memset(buf_handle->buf,0x00,per_buf_size*buf_count);

    return 0;
}

void ring_buf_delete(ring_buf_s *buf_handle)
{
    if(buf_handle->buf != NULL){
        free(buf_handle->buf);
        buf_handle->buf = NULL;
    }
    if(buf_handle->ring_mutex_en != 0)
        pthread_mutex_destroy(&buf_handle->ring_mutex);
}


#ifdef CFG_APP_USE_CMEM
cmem_handle ring_cmem_creat(ring_buf_s *buf_handle,int per_buf_size,int buf_count)
{
    cmem_handle h_cmem = cmem_create(per_buf_size*buf_count);

    if (h_cmem == NULL) {
        printf("malloc cmem for ring_buf fail !");
        return NULL;
    }

    buf_handle->buf_nums = buf_count;
    buf_handle->buf_size = per_buf_size;
    buf_handle->buf = cmem_userptr(h_cmem);

    if(buf_handle->buf == NULL) {
        printf("malloc cmem for ring_buf fail !");
        return NULL;
    }
    if (pthread_mutex_init(&buf_handle->ring_mutex, NULL) != 0) {
        perror("pthread_mutex_init!");
        return NULL;
    }

    buf_handle->ring_mutex_en = 1;
    memset(buf_handle->buf, 0x00, per_buf_size*buf_count);

    return h_cmem;
}

void ring_cmem_delete(cmem_handle h_cmem, ring_buf_s *buf_handle)
{
    if (h_cmem != NULL) {
        cmem_delete(h_cmem);
    }
    if(buf_handle->ring_mutex_en != 0)
        pthread_mutex_destroy(&buf_handle->ring_mutex);
}
#endif

inline int addring(ring_buf_s *buf_handle, int i)
{
    return (i+1)==buf_handle->buf_nums?0:i+1;
}

unsigned char * get_read_pos(ring_buf_s *buf_handle)// 获取从环形缓冲区中读取元素的地址
{
    int pos,ret;
    ret = pthread_mutex_lock(&buf_handle->ring_mutex);
    if (ret < 0){
        perror("Pthread_lock failed");
        return NULL;
    }
    //printf("buf_handle->ring_n=%d, buf_handle->buf_nums=%d\n",buf_handle->ring_n,buf_handle->buf_nums);
    if (buf_handle->ring_n>0)
    {
        pos = buf_handle->ring_get;
        buf_handle->ring_get = addring(buf_handle,buf_handle->ring_get);
        buf_handle->ring_n--;
        pthread_mutex_unlock(&buf_handle->ring_mutex);
        return buf_handle->buf+(pos*buf_handle->buf_size);
    }
    else
    {
        pthread_mutex_unlock(&buf_handle->ring_mutex);
        usleep(1000);
        return NULL;
    }
}

unsigned char *get_write_pos(ring_buf_s *buf_handle)// 获取向环形缓冲区中插入元素的地址
{
    int pos,ret;
    ret = pthread_mutex_lock(&buf_handle->ring_mutex);
    if (ret < 0){
        perror("Pthread_lock failed");
        return NULL;
    }
    //printf("buf_handle->ring_n=%d, buf_handle->buf_nums=%d \n",buf_handle->ring_n,buf_handle->buf_nums);
    if (buf_handle->ring_n < buf_handle->buf_nums)
    {
        pos=buf_handle->ring_put;
        buf_handle->ring_put=addring(buf_handle,buf_handle->ring_put);
        buf_handle->ring_n++;
        pthread_mutex_unlock(&buf_handle->ring_mutex);
        return buf_handle->buf+(pos*buf_handle->buf_size);
    }
    else
    {
        pthread_mutex_unlock(&buf_handle->ring_mutex);
        usleep(1000);
        return NULL;
    }
}


