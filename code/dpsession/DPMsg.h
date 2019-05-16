#pragma once
#include "dpplatform.h"

const DWORD DP_CHECK_ID		= 0xabcd1234;

typedef struct
{
	DWORD check;			// 校验字段 0xabcd1234
	DWORD msgid;			// 消息ID
}DPPacketHead;

typedef struct
{
	DPPacketHead head;			
	DWORD length;			// length + strlen(pContent)
	char pContent[];		// 消息内容
}DPPacket;

typedef	struct
{
	DWORD check;		// 0xabcd1234
	WORD seq_no;		// 总的包序号
	WORD flag;			// 第一位为1 表示包头  第二位为1 表示为包尾
	DWORD timestamp;	// 帧的时间戳,用来做判断是否为同一帧的另一个保证
	BYTE pdata[];		// 数据
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
	int enc_type;		// 视频编码 H264
	int	trans_type;		// 传输类型 两位Bit表示 接收发送	TRANS_TYPE_RECV		TRANS_TYPE_SEND
	int width;
	int height;
	int bitrate;
}DPMediaVideo;

typedef struct
{
	DPAddr addr;
	int enc_type;		// 音频编码 silk
	int	trans_type;		// 传输类型 两位Bit表示 接收发送	TRANS_TYPE_RECV		TRANS_TYPE_SEND
}DPMediaAudio;

typedef struct
{
	DPMediaVideo video;
	DPMediaAudio audio;
}DPMedia;

typedef struct
{
	int		remoteip;			// 对方ip
	char	dst_number[16];		// 对方房号
	int		lid;				// 本地通话ID
	int		rid;				// 对方通话ID
	DWORD	status;				// 当前状态
	DWORD	starttick;			// 当前状态的起始时间
	DWORD	heartSend;			// 上一次发送心跳包时间
	DWORD	heartRecv;			// 上一次接收心跳包时间
	BOOL	bCallOut;			// 是否呼出
	DWORD	calltype;			// 通话类型
	DPMedia rmedia;				// 对方媒体信息
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
#define DPMSG_INFO				0x1006		// 其它指令需要的话，可以用这个消息发送接收处理

#define	DPMSG_HEARTBEAT			0x2001		// 心跳包

#define DPMSG_OFFLINE			0x6001		// 对方掉线
#define DPMSG_START_VIDEO		0x6002		// 门口机呼叫进来，开启视频显示
#define DPMSG_CALL				0x6003		// 主动呼叫
#define DPMSG_MONITOR			0x6004		// 监视
#define DPMSG_LIUYAN			0x6005		// 进入留言状态
#define DPMSG_IP_CHANGE			0x6006		// IP更换

#define DPMSG_NONE				0xFFFE		// 未知指令
#define DPMSG_QUIT				0xFFFF		// 退出

#define DPCALL_IDLE		0
#define DPCALL_WAIT		1
#define DPCALL_RING		2		
#define DPCALL_TALK		3		
