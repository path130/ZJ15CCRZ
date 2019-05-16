/*
 * data_com.c
 *
 *  Created on: 2012-6-18
 *
 */
#include "data_com.h"
int pipe_creat(int fd[2])
{
	int ret=0;
	ret=pipe(fd);
    if (ret == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    return ret;
}

int pipe_write(int fd,const void *buf,int count)
{
	int ret;
	ret=write(fd,buf,count);
	if(ret!=count)
		perror("write fifo");
	return ret;
}

int pipe_read(int fd,void *buf,int count)
{
	int ret;
	ret=read(fd,buf,count);
	if(ret<0)
		perror("read fifo");
	return ret;
}

void pipe_close(int fd[2])
{
	close(fd[0]);
	close(fd[1]);
}
