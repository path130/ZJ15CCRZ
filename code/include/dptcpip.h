#pragma once

#ifdef DPCE
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

BOOL InitNetwork(void);
void SocketClose(SOCKET sock);
BOOL SocketUnblock(SOCKET sock);
int TcpConnectWait(SOCKET lsock, DWORD timeout);
SOCKET TcpConnectTry(unsigned long ip, short port);
SOCKET TcpConnect(char * rip, int rport, int timeout);
//SOCKET TcpConnect(int rip, int rport, int timeout);
SOCKET TcpListen(char* ipaddr, short port);
SOCKET TcpAccpet(SOCKET socklisten, DWORD timeout, struct sockaddr_in* psin);
int TcpSendData(SOCKET sock, char *pdata, int dlen, int timeout);
int	TcpRecvData(SOCKET sock, char *pdata, int dlen, int timeout);
//int	TcpRecvDataTry(SOCKET sock, char *pdata, int dlen, int timeout);
SOCKET UdpCreate(short port);
//SOCKET UdpCreate();
int UdpSend(SOCKET socket_fd, const char *rip, short port, char* buf, int len);
//int UdpSend(SOCKET socket_fd, int rip, short port, char* buf, int len);
int UdpRecv(SOCKET socketid, char* buf, int len, int itimeout, int* remoteip); 
BOOL UdpGroupTTL(SOCKET sockfd, int mcastTTL);
BOOL UdpJoinGroup(SOCKET sockfd, int ip);
BOOL UdpLeaveGroup(SOCKET sockfd, int ip);
int SocketSelect(SOCKET* pSock, int nSock, int timeout);
BOOL UdpSetRecvBuf(SOCKET sockfd, int length);

//class CTcpClientSock
//{
//public:
//	CTcpClientSock(void);
//	~CTcpClientSock(void);
//
//	void SetTimeout(int ms);
//	BOOL Connect(char * rip,int rport);
//	BOOL Connect(int rip,int rport);
//	void Disconnect();
//	BOOL Send(char * pdata,int dlen);
//	BOOL Recv(char * pdata,int dlen);
//	int RecvOne(char * pdata,int dlen);
//private:
//	SOCKET	m_sock;
//	int m_time;
//};

