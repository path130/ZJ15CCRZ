/*
 * net_com.c
 *
 *  Created on: 2012-6-18
 *
 */
#include <errno.h>
#include <fcntl.h>
#include "net_com.h"
#include "public.h"

int net_bind(int sockfd,struct sockaddr_in * server_t)
{
	int ret;
	if ((ret=bind(sockfd,(struct sockaddr *)server_t, sizeof(struct sockaddr))) == -1)
	{
		perror("Bind error.");
		exit(EXIT_FAILURE);
	}
	return ret;
}

int net_listen(int sockfd,int backlog)
{
	int ret;
	if ((ret=listen(sockfd,backlog)) == -1)
	{
		perror("Lisen error.");
		exit(EXIT_FAILURE);
	}
	return ret;
}

int net_connect(int sockfd, struct sockaddr *addr)
{
	int ret;
	socklen_t len=sizeof(struct sockaddr);
	ret=connect(sockfd,addr,len);
	if(ret<0)
	{
		switch(errno)
		{
		case EADDRINUSE:
			app_debug(DBG_INFO, "The socket address of the given addr is already in use.\n");
			break;
		case EISCONN:
			app_debug(DBG_INFO, "The socket socket is already connected.\n");
			break;
		case ETIMEDOUT:
			app_debug(DBG_INFO, "The attempt to establish the connection timed out.\n");
			break;
		case ECONNREFUSED:
			app_debug(DBG_INFO, "The server has actively refused to establish the connection.\n");
			break;
		case ENETUNREACH:
			app_debug(DBG_INFO, "The network of the given addr isn’t reachable from this host.\n");
			break;
		default:
			//app_debug(DBG_INFO, "\n connecting err\n");
			perror("connect : ");
			break;
		}
	}
	return ret;
}

int net_accept(int sockfd, struct sockaddr *addr)
{
	int ret;
	socklen_t client_len=sizeof(struct sockaddr);;
	ret=accept(sockfd,addr,&client_len);
	if(ret<0)
	{
		perror("accept error!");
	//	exit(EXIT_FAILURE);
	}
	return ret;
}

int send_data_to(int sockfd,const void *buf,int len,const struct sockaddr *dest_addr)
{
	int ret;
	ret=sendto(sockfd,buf,len,0,dest_addr,sizeof(struct sockaddr));
	if(ret<0)
			perror("Send data error!");
	return ret;
}

int recv_data_from(int sockfd,void *buf,int len,struct sockaddr *src_addr)
{
	int ret;
	socklen_t length=sizeof(struct sockaddr_in);
	ret=recvfrom(sockfd,buf,len,0,src_addr,&length);
	if(ret<0)
		perror("Receive data error!");
	return ret;
}

int send_data(int sockfd,const unsigned char *buf,int len)
{
	int ret;
	ret=send(sockfd,buf,len,0);
	if(ret<=0)
		perror("send error!");
	return ret;
}

int recv_data(int sockfd,unsigned char *buf,int len)
{
	int ret;
	ret=recv(sockfd,buf,len,0);
	if(ret<0)
	{
		if ((errno != EWOULDBLOCK)&&(errno != ECONNRESET))
		{
			perror("receive error!");
		}

	}

	return ret;
}

void net_close(int * fd)
{
	if(*fd>0){
		close(*fd);
		*fd=-1;
		app_debug(DBG_INFO, "close\n");
	}
	return;
}

int Fcntl(int fd, int cmd, int arg)
{
	int	n;
	if ( (n = fcntl(fd, cmd, arg)) == -1)
		app_debug(DBG_INFO,"fcntl error");
	return(n);
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
	int		n;
	if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
		app_debug(DBG_INFO,"select error");
	return(n);		/* can return 0 on timeout */
}


int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)
{
	int				flags, n, error;
	socklen_t		len;
	fd_set			rset, wset;
	struct timeval	tval;

	flags = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	error = 0;
	if ( (n = connect(sockfd, saptr, salen)) < 0)
		if (errno != EINPROGRESS)
			return(-1);

	/* Do whatever we want while the connect is taking place. */

	if (n == 0)
		goto done;	/* connect completed immediately */

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ( (n = Select(sockfd+1, &rset, &wset, NULL,
					 nsec ? &tval : NULL)) == 0) {
		close(sockfd);		/* timeout */
		errno = ETIMEDOUT;
		return(-1);
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			return(-1);			/* Solaris pending error */
	} else
		return(-1);

done:
	Fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */

	if (error) {
		close(sockfd);		/* just in case */
		errno = error;
		return(-1);
	}
	return(0);
}

