#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <fcntl.h>

#define    FAILURE   -1
#define    SUCCESS    0

unsigned char ip[4];
unsigned char src_mac[6] = {0};
unsigned char dst_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int ARPGetTickCount(void);
int send_arp(int checkip, int sockfd, struct sockaddr_ll *peer_addr);
int recv_arp(int checkip, int sockfd, struct sockaddr_ll *peer_addr);

//ARP封装包
typedef struct _tagARP_PACKET{
	struct ether_header  eh;
	struct ether_arp arp;
}ARP_PACKET_OBJ, *ARP_PACKET_HANDLE;

// 检测IP是否冲突，冲突返回true, 否则返回false
bool CheckIPConflict(int checkip)
{
	int sockfd;
	int rtval = -1;
	struct sockaddr_ll peer_addr;
	//创建socket
	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (sockfd < 0)
	{
		printf("CheckIPConflict socket failed:%s\r\n", strerror(errno));
		return false;
	}

	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sll_family = AF_PACKET;
	struct ifreq req;
	bzero(&req, sizeof(struct ifreq));
	strcpy(req.ifr_name, "eth0");
	if(ioctl(sockfd, SIOCGIFINDEX, &req) != 0)
		perror("ioctl()");
	peer_addr.sll_ifindex = req.ifr_ifindex;
	peer_addr.sll_protocol = htons(ETH_P_ARP);
	//peer_addr.sll_family = AF_PACKET;

	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);

	rtval = send_arp(checkip, sockfd, &peer_addr);
	if (FAILURE == rtval)
	{
		printf("CheckIPConflict send failed: %s\n", strerror(errno));
		return false;
	}
	rtval = recv_arp(checkip, sockfd, &peer_addr);
	if (rtval == SUCCESS)
	{
		printf ("%x conflict\r\n", checkip);
		return true;
	}
	else if (rtval == FAILURE)
	{
		printf ("%x not conflict\r\n", checkip);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// 函数名: send_arp
// 描述 : 填充ARP数据包报文并发送出去。
// 参数:
//    [in] sockfd -- 创建的socket描述符;
//    [in] peer_addr -- 对端的IP信息
// 返回值:
//    成功: SUCCESS, 失败: FAILURE;
// 说明:
//////////////////////////////////////////////////////////////////////////
int send_arp(int checkip, int sockfd, struct sockaddr_ll *peer_addr)
{
	int rtval;
	ARP_PACKET_OBJ frame;
	memset(&frame, 0x00, sizeof(ARP_PACKET_OBJ));

	//填充以太网头部
	memcpy(frame.eh.ether_dhost, dst_mac, 6);    //目的MAC地址
	memcpy(frame.eh.ether_shost, src_mac, 6);    //源MAC地址
	frame.eh.ether_type = htons(ETH_P_ARP);      //协议

	//填充ARP报文头部
	frame.arp.ea_hdr.ar_hrd = htons(ARPHRD_ETHER);    //硬件类型
	frame.arp.ea_hdr.ar_pro = htons(ETHERTYPE_IP);    //协议类型 ETHERTYPE_IP | ETH_P_IP
	frame.arp.ea_hdr.ar_hln = 6;                //硬件地址长度
	frame.arp.ea_hdr.ar_pln = 4;                //协议地址长度
	frame.arp.ea_hdr.ar_op = htons(ARPOP_REQUEST);    //ARP请求操作

	memcpy(frame.arp.arp_sha, src_mac, 6);		//源MAC地址
	memcpy(frame.arp.arp_spa, &checkip, 4);     //源IP地址
	memcpy(frame.arp.arp_tha, dst_mac, 6);		//目的MAC地址
	memcpy(frame.arp.arp_tpa, &checkip, 4);     //目的IP地址

	rtval = sendto(sockfd, &frame, sizeof(ARP_PACKET_OBJ), 0,
		(struct sockaddr*)peer_addr, sizeof(struct sockaddr_ll));
	if (rtval < 0)
	{
		return FAILURE;
	}
	return SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// 函数名: recv_arp
// 描述 : 接收ARP回复数据报文并判断是不是对免费ARP的回复。
// 参数:
//    [in] sockfd -- 创建的socket描述符;
//    [in] peer_addr -- 对端的IP信息
// 返回值:
//    成功: SUCCESS, 失败: FAILURE;
// 说明:
//    若是对免费arp请求的回复则返回:SUCCESS.
//////////////////////////////////////////////////////////////////////////


int ARPUdpRecv(int socketid, char* buf, int len, int itimeout, int* remoteip)
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
		FD_SET(socketid,&fdread); //添加描述符 

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

int recv_arp(int checkip, int sockfd, struct sockaddr_ll *peer_addr)
{
	int rtval;
	ARP_PACKET_OBJ frame;

	memset(&frame, 0, sizeof(ARP_PACKET_OBJ));
	rtval = ARPUdpRecv(sockfd, (char*)&frame, sizeof(frame), 1000, NULL);
	//判断是否接收到数据并且是否为回应包
	if (htons(ARPOP_REPLY) == frame.arp.ea_hdr.ar_op && rtval > 0)
	{
		//判断源地址是否为冲突的IP地址
		if (memcmp(frame.arp.arp_spa, &checkip, 4) == 0)
		{
			//printf("IP address is common~\n");
			return SUCCESS;
		}
	}
	if (rtval < 0)
	{
		return FAILURE;
	}
	return FAILURE;
}
