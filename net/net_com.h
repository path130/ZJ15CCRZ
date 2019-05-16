/*
 * net_com.h
 *
 *  Created on: 2012-6-18
 *
 */

#ifndef NET_COM_H_
#define NET_COM_H_

#include <stdio.h>          /* These are the usual header files */
#include <strings.h>          /* for bzero() */
#include <unistd.h>         /* for close() */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "global_def.h"
#if defined (__cplusplus)
extern "C" {
#endif

inline int send_data_to(int coon,const void *buf,int len,const struct sockaddr *dest_addr);
inline int recv_data_from(int coon,void *buf,int len,struct sockaddr *src_addr);

inline int send_data(int coon,const unsigned char *buf,int len);
inline int recv_data(int coon,unsigned char *buf,int len);

int net_bind(int coon,struct sockaddr_in * server_t);
int net_listen(int sockfd,int backlog);
int net_connect(int sockfd, struct sockaddr *addr);
int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec);
int net_accept(int sockfd, struct sockaddr *addr);

void net_close(int *fd);

#if defined (__cplusplus)
}
#endif

#endif /* NET_COM_H_ */
