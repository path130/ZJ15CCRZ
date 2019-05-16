#pragma once
#include "dpplatform.h"

const DWORD DP_CHECK_ID		= 0xabcd1234;

typedef struct
{
	DWORD check;			// У���ֶ� 0xabcd1234
	DWORD msgid;			// ��ϢID
}DPPacketHead;

typedef struct
{
	DPPacketHead head;			
	DWORD length;			// length + strlen(pContent)
	char pContent[];		// ��Ϣ����
}DPPacket;

typedef	struct
{
	DWORD check;		// 0xabcd1234
	WORD seq_no;		// �ܵİ����
	WORD flag;			// ��һλΪ1 ��ʾ��ͷ  �ڶ�λΪ1 ��ʾΪ��β
	DWORD timestamp;	// ֡��ʱ���,�������ж��Ƿ�Ϊͬһ֡����һ����֤
	BYTE pdata[];		// ����
}DPTransPacket;

typedef struct
{
	int ip;
	int port;
	int cport;
}DPAddr;

typedef struct
{
	DPAddr addr;
	int enc_type;		// ��Ƶ���� H264
	int	trans_type;		// �������� ��λBit��ʾ ���շ���	TRANS_TYPE_RECV		TRANS_TYPE_SEND
	int width;
	int height;
	int bitrate;
}DPMediaVideo;

typedef struct
{
	DPAddr addr;
	int enc_type;		// ��Ƶ���� silk
	int	trans_type;		// �������� ��λBit��ʾ ���շ���	TRANS_TYPE_RECV		TRANS_TYPE_SEND
}DPMediaAudio;

typedef struct
{
	DPMediaVideo video;
	DPMediaAudio audio;
}DPMedia;

typedef struct
{
	int		remoteip;			// �Է�ip
	char	dst_number[16];		// �Է�����
	int		lid;				// ����ͨ��ID
	int		rid;				// �Է�ͨ��ID
	DWORD	status;				// ��ǰ״̬
	DWORD	starttick;			// ��ǰ״̬����ʼʱ��
	DWORD	heartSend;			// ��һ�η���������ʱ��
	DWORD	heartRecv;			// ��һ�ν���������ʱ��
	BOOL	bCallOut;			// �Ƿ����
	DWORD	calltype;			// ͨ������
	DPMedia rmedia;				// �Է�ý����Ϣ
}DPSession;

#define TRANS_TYPE_SEND		1
#define TRANS_TYPE_RECV		2	

#define _PACKET_LEN				2800	
#define _PACKET_HEAD_LEN		12		// sizeof(DPTransPacket)

#define	DPMSG_NEWCALLIN			0x1001		
#define	DPMSG_BUSY				0x1002
#define DPMSG_IDLE				0x1003
#define DPMSG_ACCEPT			0x1004
#define DPMSG_HANGUP			0x1005
#define DPMSG_INFO				0x1006		// ����ָ����Ҫ�Ļ��������������Ϣ���ͽ��մ���

#define	DPMSG_HEARTBEAT			0x2001		// ������

#define DPMSG_OFFLINE			0x6001		// �Է�����
#define DPMSG_START_VIDEO		0x6002		// �ſڻ����н�����������Ƶ��ʾ
#define DPMSG_CALL				0x6003		// ��������
#define DPMSG_MONITOR			0x6004		// ����
#define DPMSG_LIUYAN			0x6005		// ��������״̬
#define DPMSG_IP_CHANGE			0x6006		// IP����

#define DPMSG_NONE				0xFFFE		// δָ֪��
#define DPMSG_QUIT				0xFFFF		// �˳�

#define DPCALL_IDLE		0
#define DPCALL_WAIT		1
#define DPCALL_RING		2		
#define DPCALL_TALK		3		
