#include <roomlib.h>
#include <fcntl.h>
void SocketClose(SOCKET sock)
{
	if (sock != INVALID_SOCKET)
	{
		close(sock);
		sock = INVALID_SOCKET;
	}
}

BOOL InitNetwork(void)
{
	return TRUE;
}

BOOL SocketUnblock(SOCKET sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags|O_NONBLOCK);
	return TRUE;
}

int TcpConnectWait(SOCKET lsock, DWORD timeout)
{
	struct timeval tv_out;
	fd_set fWR; 
	int ret;

	tv_out.tv_sec  = timeout/1000;
	tv_out.tv_usec = (timeout%1000)*1000;
	FD_ZERO(&fWR);
	FD_SET(lsock, &fWR);
	ret = select(lsock + 1, NULL, &fWR, NULL, &tv_out);

	if(ret == 0)
	{
		//time out
		DBGMSG(DPWARNING, "Socket1 select timeout \r\n");
		return -1;
	}
	if(ret == SOCKET_ERROR)
	{
		DBGMSG(DPWARNING, "Socket1 connect select error %d\r\n", DPGetLastError());
		return -1;
	}

	if(FD_ISSET(lsock, &fWR))
	{
		return 0;
	}
	DBGMSG(DPWARNING, "Should not reach hear %d\r\n", DPGetLastError());
	return -1;
}

SOCKET TcpConnectTry(unsigned long ip, short port)
{
	int ret;
	SOCKET m_sock;
	struct sockaddr_in sin;

	// ´´½¨socket
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sock == INVALID_SOCKET)
	{
		DBGMSG(DPWARNING, "Socket create error %d\r\n", DPGetLastError());
		return INVALID_SOCKET;
	}

	// set unblock mode
	SocketUnblock(m_sock);

	memset((char*)&sin, 0, sizeof(struct sockaddr_in));    
	sin.sin_family = AF_INET;	 
	sin.sin_port = htons(port); 
	sin.sin_addr.s_addr = ip;

	// begin connect
	ret = connect(m_sock, (struct sockaddr *)&sin, sizeof(sin));
	if(ret != SOCKET_ERROR)
	{
		DBGMSG(DPWARNING, "Socket connect is OK\r\n");
		return m_sock;
	}

	if(DPGetLastError() == EINPROGRESS)
	{
		DBGMSG(DPWARNING, "Socket connect need select\r\n");
		return m_sock;
	}
	else
	{
		SocketClose(m_sock);
		DBGMSG(DPWARNING, "Socket connect %d\r\n", DPGetLastError());
		return INVALID_SOCKET;
	}
}

SOCKET TcpConnect(char * rip, int rport, int timeout)
{
	int ret;
	struct sockaddr_in sin;
	struct timeval tv_out;
	fd_set fdR;	 
	SOCKET m_sock;

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sock == INVALID_SOCKET)
	{
		DBGMSG(DPWARNING, "Socket create error %d\r\n", DPGetLastError());
		return INVALID_SOCKET;
	}

	SocketUnblock(m_sock);

	memset((char*)&sin, 0, sizeof(struct sockaddr_in));    
	sin.sin_family = AF_INET;    
	sin.sin_port = htons(rport);    
	sin.sin_addr.s_addr = inet_addr(rip);

	ret = connect(m_sock, (struct sockaddr *)&sin, sizeof(sin));
	if(ret == SOCKET_ERROR)
	{
		if(DPGetLastError() == EINPROGRESS)
		{
			tv_out.tv_sec  = timeout/1000;
			tv_out.tv_usec = (timeout%1000)*1000;
			FD_ZERO(&fdR);
			FD_SET(m_sock, &fdR);

			ret = select(m_sock + 1, NULL, &fdR, NULL, &tv_out);
			if( ret == 0)
			{
				//time out
				DBGMSG(DPWARNING, "Socket connect timeout \r\n");
				SocketClose(m_sock);
				return INVALID_SOCKET;
			}
			if(ret == SOCKET_ERROR)
			{
				DBGMSG(DPWARNING, "Socket connect select error %d\r\n", DPGetLastError());
				SocketClose(m_sock);
				return INVALID_SOCKET;
			}
		}
		else
		{
			SocketClose(m_sock);
			DBGMSG(DPWARNING, "Socket connect %s %d errno %d\r\n", rip, rport, DPGetLastError());
			return INVALID_SOCKET;
		}
	}
	return m_sock;
}

SOCKET TcpConnect(int rip, int rport, int timeout)
{
	char szMCip[256];
	sprintf(szMCip, "%d.%d.%d.%d",(rip & 0xFF), (rip >> 8) & 0xFF, (rip >> 16) & 0xFF, (rip >> 24) & 0xFF);		
	return TcpConnect(szMCip, rport, timeout);
}

SOCKET TcpListen(char* ipaddr, short port)
{
	struct sockaddr_in sin;
	SOCKET socklisten;

	socklisten = socket(AF_INET, SOCK_STREAM, 0);
	if(socklisten == INVALID_SOCKET)
	{
		DBGMSG(DPWARNING, "socket create error %d\n", DPGetLastError());
		return INVALID_SOCKET;
	}

	SocketUnblock(socklisten);

	memset(&sin,0,sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	if(ipaddr != NULL)
		sin.sin_addr.s_addr = inet_addr(ipaddr);
	else
		sin.sin_addr.s_addr = htonl(INADDR_ANY);


	if(bind(socklisten, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1)
	{
		DBGMSG(DPWARNING, "socket bind error %d\n", DPGetLastError());
		SocketClose(socklisten);
		return INVALID_SOCKET;
	}

	if(listen(socklisten,2) == -1)
	{
		DBGMSG(DPWARNING, "socket listen error %d\n", DPGetLastError());
		SocketClose(socklisten);
		return INVALID_SOCKET;
	}

	return socklisten;
}

SOCKET TcpAccpet(SOCKET socklisten, DWORD timeout, struct sockaddr_in* psin)
{
	SOCKET ret ;
	struct timeval tv_out;    
	fd_set readfds;
	struct sockaddr_in sin;
	socklen_t nnn;

	tv_out.tv_sec  = timeout/1000;
	tv_out.tv_usec = (timeout%1000)*1000;

	FD_ZERO(&readfds); 
	FD_SET(socklisten, &readfds);

	ret = select(socklisten + 1, &readfds, NULL, NULL, &tv_out);   
	if(ret == 0)
	{
		//DBGMSG(DPERROR, "Socket accept timeout %d \r\n",timeout);
		return INVALID_SOCKET;
	}
	if(ret == SOCKET_ERROR)
	{
		DBGMSG(DPWARNING, "Socket accept select error %d\n", DPGetLastError());
		return INVALID_SOCKET;
	}

	nnn = sizeof(sin);

	if(psin == NULL)
		psin = &sin;
	ret = accept(socklisten, (struct sockaddr*)psin, &nnn);
	if(ret == INVALID_SOCKET)
	{
		DBGMSG(DPWARNING, "Socket accept error %d\n", DPGetLastError());
	}
	return ret;
}

int TcpSendData(SOCKET sock, char *pdata, int dlen, int timeout)
{
	int ret = 0;
	int i = 0;
	struct timeval tv_out;    
	fd_set writefds;
	DWORD starttick = DPGetTickCount();
	DWORD curtick;
	DWORD maxout = timeout;
	while(i < dlen)
	{
		tv_out.tv_sec  = timeout/1000;
		tv_out.tv_usec = (timeout%1000)*1000;

		FD_ZERO(&writefds); 
		FD_SET(sock, &writefds);

		ret = select(sock + 1, NULL, &writefds, NULL, &tv_out);   
		if( ret == 0)
		{
			//time out
			return -1;
		}

		if(ret == SOCKET_ERROR)
		{
			return -2;
		}


		ret = send(sock,pdata+i,dlen-i,0);
		if(ret == SOCKET_ERROR)
		{
			if(EALREADY != DPGetLastError())
				return -3;
		}
		else
			i += ret;
		curtick = DPGetTickCount();
		if((curtick - starttick) >= maxout)
			break;
		timeout = maxout - (curtick - starttick);
	}

	return i;
}

int	TcpRecvData(SOCKET sock, char *pdata, int dlen, int timeout)
{
	int ret = 0;
	int i = 0;
	struct timeval tv_out;    
	fd_set readfds;
	DWORD starttick = DPGetTickCount();
	DWORD curtick;
	DWORD maxout = timeout;

	while(i < dlen)
	{
		tv_out.tv_sec  = timeout/1000;
		tv_out.tv_usec = (timeout%1000)*1000;

		FD_ZERO(&readfds); 
		FD_SET(sock, &readfds);

		ret = select(sock + 1, &readfds, NULL, NULL, &tv_out);   
		if(ret == 0)
		{
			//time out
			printf("timeout\r\n");
			return i;
		}

		if(ret == SOCKET_ERROR)
		{
			printf("select SOCKET_ERROR\r\n");
			return i;
		}

		if (FD_ISSET(sock, &readfds))
		{
			ret = recv(sock,pdata+i,dlen-i,0);
			if(ret == SOCKET_ERROR)
			{
				printf("recv SOCKET_ERROR\r\n");
				if(EALREADY != DPGetLastError())
					return i;
			}
			else if(ret == 0)
			{
				printf("recv 0\r\n");
				return i;
			}
			else
				i += ret;
		}
		curtick = DPGetTickCount();
		if((curtick - starttick) >= maxout)
			break;
		timeout = maxout - (curtick - starttick);
	}

	return i;
}

int	TcpRecvDataTry(SOCKET sock, char *pdata, int dlen, int timeout)
{
	int ret = 0;
	struct timeval tv_out;    
	fd_set readfds;

	tv_out.tv_sec  = timeout/1000;
	tv_out.tv_usec = (timeout%1000)*1000;

	FD_ZERO(&readfds); 
	FD_SET(sock, &readfds);

	ret = select(sock + 1, &readfds, NULL, NULL, &tv_out);   
	if(ret == 0)
	{
		//time out
		printf("recv data timeout\r\n");
		return 0;
	}

	if(ret == SOCKET_ERROR)
	{
		printf("select SOCKET_ERROR\r\n");
		return -1;
	}

	if (FD_ISSET(sock, &readfds))
	{
		ret = recv(sock,pdata, dlen,0);
		if(ret == SOCKET_ERROR)
		{
			printf("recv SOCKET_ERROR\r\n");
			if(EALREADY != DPGetLastError())
				return -1;
		}
		else if(ret == 0)
		{
			printf("recv 0\r\n");
			return -1;
		}
		else
			return ret;
	}
	return 0;
}

SOCKET UdpCreate(short port)
{
	SOCKET sockfd;
	struct sockaddr_in servaddr;
	int b = 1;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("socket create error");
		return INVALID_SOCKET;
	}

	SocketUnblock(sockfd);

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&b, sizeof(b)) == -1)
	{
		printf("udp_create: socket SO_REUSEADDR error");
		SocketClose(sockfd);
		return INVALID_SOCKET;
	}

	/* init servaddr */
	memset((char*)&servaddr, 0, sizeof(struct sockaddr_in));    
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //#define INADDR_ANY	((unsigned long int) 0x00000000)
	servaddr.sin_port = htons(port);

	/* bind address and port to socket */
	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		printf("bind error");
		SocketClose(sockfd);
		return INVALID_SOCKET;
	}
	return sockfd;
}

SOCKET UdpCreate()
{
	SOCKET sockfd;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("socket create error");
		return INVALID_SOCKET;
	}

	SocketUnblock(sockfd);
	return sockfd;
}

int UdpSend(SOCKET socket_fd, const char *rip, short port, char* buf, int len)
{
	struct sockaddr_in my_addr;

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(port);
	my_addr.sin_addr.s_addr=inet_addr(rip);
	memset((char*)&my_addr.sin_zero, 0, 8);    

	if(sendto(socket_fd,buf,len,0,(struct sockaddr *)&my_addr,sizeof(my_addr)) != len)
	{
		printf("udp_send error");
		return -1;
	}
	return 0;
}

int UdpSend(SOCKET socket_fd, int rip, short port, char* buf, int len)
{
	struct sockaddr_in my_addr;

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(port);
	my_addr.sin_addr.s_addr=rip;
	memset((char*)&my_addr.sin_zero, 0, 8);    

	if(sendto(socket_fd,buf,len,0,(struct sockaddr *)&my_addr,sizeof(my_addr)) != len)
	{
		printf("udp_send error");
		return -1;
	}
	return 0;
}

int UdpRecv(SOCKET socketid, char* buf, int len, int itimeout, int* remoteip)
{
	int retlen = 0;
	struct sockaddr_in user_addr;
	socklen_t usize;

	usize=sizeof(user_addr);

	if(itimeout > 0)
	{
		fd_set fdread;
		int rc;
		struct timeval timeout;

		timeout.tv_sec = itimeout/1000;
		timeout.tv_usec = (itimeout%1000) * 1000;
		FD_ZERO(&fdread);
		FD_SET(socketid,&fdread); //Ìí¼ÓÃèÊö·û 

		rc = select(socketid + 1, &fdread, NULL, NULL, &timeout);
		if(rc < 0)
			return -1;
		if(rc == 0)
			return -1;

		retlen = recvfrom(socketid, buf, len, 0, (struct sockaddr *)&user_addr,&usize);
		if(retlen <= 0)
			return -1;
		if(remoteip != NULL)
			*remoteip = user_addr.sin_addr.s_addr;
	}
	else
	{
		retlen = recvfrom(socketid, buf, len, 0, (struct sockaddr *)&user_addr, &usize);
		if(remoteip != NULL)
			*remoteip = user_addr.sin_addr.s_addr;
	}

	return retlen;
}

int SocketSelect(SOCKET* pSock, int nSock, int timeout)
{
	fd_set fd;
	FD_ZERO(&fd);

	SOCKET maxSock = pSock[0];
	for(int i = 0; i < nSock; i++)
	{
		FD_SET(pSock[i], &fd);
		if(maxSock < pSock[i])
			maxSock = pSock[i];
	}

	timeval tv_out;
	tv_out.tv_sec = timeout / 1000;
	tv_out.tv_usec = (timeout % 1000) * 1000;
	int ret = select(maxSock + 1, &fd, NULL, NULL, &tv_out);
	if(ret < 0)
		return -1;
	else if(ret == 0)
		return 0;		// timeOut
	else 
	{
		for(int i = 0; i < nSock; i++)
		{
			if(FD_ISSET(pSock[i], &fd))
				return i + 1;
		}
	}

	return -2;
}

BOOL UdpSetRecvBuf(SOCKET sockfd, int length)
{
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char *)&length, sizeof(length)) < 0)
	{
		printf("ERROR: setsockopt(SO_RCVBUF): %d\n", DPGetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL UdpGroupTTL(SOCKET sockfd, int mcastTTL)
{
	if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&mcastTTL, sizeof(mcastTTL)) < 0)
	{
		printf("ERROR: setsockopt(IP_MULTICAST_TTL): %d\n", DPGetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL UdpJoinGroup(SOCKET sockfd, int ip)
{
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = ip;
	mreq.imr_interface.s_addr= INADDR_ANY;
	if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) < 0)
	{
		printf("ERROR: setsockopt(IP_ADD_MEMBERSHIP): %d\n", DPGetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL UdpLeaveGroup(SOCKET sockfd, int ip)
{
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = ip;
	mreq.imr_interface.s_addr= INADDR_ANY;
	if (setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) < 0)
	{
		printf("ERROR: setsockopt(IP_DROP_MEMBERSHIP): %d\n", DPGetLastError());
		return FALSE;
	}
	return TRUE;
}

CTcpClientSock::CTcpClientSock(void)
{
	m_sock = INVALID_SOCKET;
	m_time = 1000;
}

CTcpClientSock::~CTcpClientSock(void)
{
	if(m_sock != INVALID_SOCKET)
	{
		SocketClose(m_sock);
		m_sock = INVALID_SOCKET;
	}
}

void CTcpClientSock::SetTimeout(int ms)
{
	m_time = ms;
}

BOOL CTcpClientSock::Connect(char * rip,int rport)
{
	m_sock = TcpConnect(rip, rport, m_time);
	if(m_sock == INVALID_SOCKET)
		return FALSE;
	return TRUE;
}

BOOL CTcpClientSock::Connect(int rip, int rport)
{
	m_sock = TcpConnect(rip, rport, m_time);
	if(m_sock == INVALID_SOCKET)
		return FALSE;
	return TRUE;
}
void CTcpClientSock::Disconnect()
{
	if(m_sock != INVALID_SOCKET)
		SocketClose(m_sock);
	m_sock = INVALID_SOCKET;
}
BOOL CTcpClientSock::Send(char * pdata,int dlen)
{
	if(m_sock != INVALID_SOCKET)
	{
		if(TcpSendData(m_sock, pdata, dlen, m_time) == dlen)
			return TRUE;
	}
	return FALSE;
}

BOOL CTcpClientSock::Recv(char * pdata,int dlen)
{
	if(m_sock != INVALID_SOCKET)
	{
		if(TcpRecvData(m_sock, pdata, dlen, m_time) == dlen)
			return TRUE;
	}
	return FALSE;
}

int CTcpClientSock::RecvOne(char * pdata,int dlen)
{
	return TcpRecvDataTry(m_sock, pdata, dlen, m_time);
}

