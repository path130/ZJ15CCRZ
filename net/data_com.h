/*
 * data_com.h
 *
 *  Created on: 2012-6-18
 *
 */

#ifndef DATA_COM_H_
#define DATA_COM_H_
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int pipe_creat(int fd[2]);
int pipe_write(int fd, const void *buf,int count);
int pipe_read(int fd, void *buf,int count);
void pipe_close(int fd[2]);

#endif /* DATA_COM_H_ */
