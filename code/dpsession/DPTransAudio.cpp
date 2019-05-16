#include "roomlib.h"
#include "AES.h"
#include "dpvideo.h"
#include "dpgpio.h"
#include "dpmsg.h"
#include "DPFilterSilk.h"

#define _PACKET_PCM_LEN		640

static BYTE INIT_KEY[AES_BLOCK_SIZE] =
{
	0xf8,0x0c,0x43,0xa2,0x1b,0x85,0x1a,0xe1,
	0xa7,0x18,0xca,0x2a,0x0c,0x21,0x79,0xe5
};

static BYTE SET_KEY[AES_BLOCK_SIZE] = 
{
	0x8c,0xde,0x02,0x16,0xf9,0x48,0xaf,0x1e,
	0x1a,0x24,0x57,0x2b,0x7c,0xcb,0x4a,0x11
};

static HANDLE g_hRecvThread = NULL;
static AES* g_pAes = NULL;
static BOOL g_bATransRun = FALSE;
static SOCKET g_sockfd = INVALID_SOCKET;
static StaticLock g_AudioCS;

// 放音
static HANDLE g_hAudioPlay = NULL;

// 录音
static char* g_pBuffer = NULL;
static DPTransPacket* g_pPacket = NULL;
static HANDLE g_hAudioRec = NULL;
static WORD g_dwSeq = 0;
static HANDLE g_hSilkEnc = NULL;
static SOCKADDR_IN g_SockAddr;

// 留言
static BOOL g_bLiuyanRun = FALSE;
static BOOL g_bSaveStart = FALSE;		// 发送留言提示完毕，开始保存留言
static HANDLE g_hLiuyanRecv = NULL;
static FILE* g_pLiuyanFile = NULL;
static HANDLE g_hLYWaitEvent = NULL;		// 等待留言文件保存退出

static void AudioSendInit(int ip, short port)
{
	g_SockAddr.sin_family = AF_INET;
	g_SockAddr.sin_port = htons(port);
	g_SockAddr.sin_addr.s_addr = ip;

	g_dwSeq = 0;
	g_pBuffer = (char*)malloc(_PACKET_LEN);
	g_pPacket = (DPTransPacket*)g_pBuffer;
	g_hSilkEnc = SilkEncCreate();
	g_hAudioRec = AudioRecCreate();
}

static void AudioSendUnInit()
{
	free(g_pBuffer);
	SilkEncDestroy(g_hSilkEnc);
	AudioRecStop(g_hAudioRec);
	AudioRecDestroy(g_hAudioRec);
}

static BOOL AudioSend(DWORD userData, char * pdata, int dlen)
{
	if(g_bATransRun)
	{
		g_pPacket->check = DP_CHECK_ID;
		g_pPacket->flag = 1 | 2;
		g_pPacket->seq_no = g_dwSeq++;
		g_pPacket->timestamp = DPGetTickCount();

		int length = SilkEncRun(g_hSilkEnc, pdata, dlen, (char*)g_pPacket->pdata);
		length += _PACKET_HEAD_LEN;

		AES_SetEncInitVec(g_pAes, SET_KEY);
		AES_DataEncrypt(g_pAes, (BYTE*)g_pPacket, (BYTE*)g_pPacket, length);

		sendto(g_sockfd, (char*)g_pPacket, length, 0, (SOCKADDR*)&g_SockAddr, sizeof(SOCKADDR_IN));
	}

	return TRUE;
}

static DWORD AudioRecv(HANDLE pParam)
{
	WORD seq_no = 0;
	BOOL bFindHead = TRUE;		// 丢包，先找到包头

	int nRecvLen;
	char buf[_PACKET_LEN];
	DPTransPacket *pPacket = (DPTransPacket*)buf;

	DWORD dwFrameLen = 0;
	char* pFrameData = (char*)malloc(0x1000);

	DWORD dwCount = 0;
	BOOL bStartPlay = FALSE;
	g_hAudioPlay = AudioPlayCreate();

	HANDLE hSilk = SilkDecCreate();
	char* pcmBuffer = (char*)malloc(2048);
	int pcmLen = 0;

	while(g_bATransRun)
	{
		nRecvLen = UdpRecv(g_sockfd, (char*)pPacket, _PACKET_LEN, 200, NULL);
		if(nRecvLen > _PACKET_HEAD_LEN)
		{
			AES_SetDecInitVec(g_pAes, SET_KEY);
			AES_DataDecrypt(g_pAes, (BYTE*)pPacket, (BYTE*)pPacket, nRecvLen);

			if(pPacket->check != DP_CHECK_ID)
			{
				continue;
			}

			if(pPacket->seq_no != seq_no)
			{
				//printf("audio seq_no:%d, timesamp:%d, len:%d\r\n", pPacket->seq_no, pPacket->timestamp, nRecvLen - 12);
				bFindHead = TRUE;
			}
			seq_no = pPacket->seq_no + 1;

			if(bFindHead)
			{
				if(pPacket->flag & 1)
				{
					bFindHead = FALSE;
					dwFrameLen = 0;
				}
				else
				{
					continue;
				}
			}

			if(dwFrameLen + nRecvLen > 0xFFF4)
			{
				DBGMSG(DPINFO, "packet is too big\r\n");
				bFindHead = TRUE;
				continue;
			}

			memcpy(pFrameData + dwFrameLen, pPacket->pdata, nRecvLen - _PACKET_HEAD_LEN);
			dwFrameLen += nRecvLen - _PACKET_HEAD_LEN;

			if(pPacket->flag & 2)
			{
				if(!bStartPlay)
				{
					AudioPlayStart(g_hAudioPlay, 8000, 1);
					AudioPlaySetVolume(g_hAudioPlay, 0x11111111);
					bStartPlay = TRUE;
				}

				pcmLen = SilkDecRun(hSilk, pFrameData, dwFrameLen, pcmBuffer);

				AudioPlayAddPlay(g_hAudioPlay, pcmBuffer, pcmLen * 2);
				dwFrameLen = 0;
			}
		}
		else
		{
			if(bStartPlay)
			{
				dwCount++;
				if(dwCount == 10)
				{
					AudioPlayStoped(g_hAudioPlay);
					bStartPlay = FALSE;
				}
			}
		}
	}

	AudioPlayDestroy(g_hAudioPlay);
	SilkDecDestroy(hSilk);
	free(pFrameData);
	free(pcmBuffer);
	return 0;
}

BOOL DPTransAudioStart(int ip, int port, int localport)
{
	if(ip == 0)
		return TRUE;

	DBGMSG(DPINFO, "DPTransAudioStart start ip:%x, port:%d, localport:%d\r\n", ip, port, localport);
	g_AudioCS.lockon();

	if(!g_bATransRun && !g_bLiuyanRun)
	{
		g_bATransRun = TRUE;
		if(g_pAes == NULL)
		{
			g_pAes = AES_New();
			AES_SetMode(g_pAes, AES_MODE_SIC);
			AES_SetKey(g_pAes, INIT_KEY, 128);
		}

		g_sockfd = UdpCreate(localport);
		if(!UdpSetRecvBuf(g_sockfd, 60000))
		{
			DBGMSG(DPERROR, "DPTrans UdpSetRecvBuf\r\n");
			return 0;
		}

		AudioSendInit(ip, port);
		AudioRecStart(g_hAudioRec, 8000, 1, AudioSend, NULL);
		g_hRecvThread = DPThreadCreate(0x40000, AudioRecv, NULL, TRUE, 1);
	}

	g_AudioCS.lockoff();
	return TRUE;
}

BOOL DPTransAudioStop()
{
	g_AudioCS.lockon();
	if(g_bATransRun)
	{
		DWORD tick = DPGetTickCount();
		g_bATransRun = FALSE;
		DPThreadJoin(g_hRecvThread);
		SocketClose(g_sockfd);
		g_sockfd = INVALID_SOCKET;
		AudioSendUnInit();
		DBGMSG(DPINFO, "DPTransAudioStop end cost %dms\r\n", DPGetTickCount() - tick);
	}
	g_AudioCS.lockoff();
	return TRUE;
}

void DPTransAudioSetVol(DWORD dwVol)
{
	g_AudioCS.lockon();
	if(g_hAudioPlay)
	{
		AudioPlaySetVolume(g_hAudioPlay, dwVol);
	}
	g_AudioCS.lockoff();
}

/***************************************************************
					留言
***************************************************************/
static BOOL LiuyanSend(DWORD userData, char * pdata, int dlen)
{
	if(g_bLiuyanRun)
	{
		if(g_pLiuyanFile)
		{
			g_pPacket->check = DP_CHECK_ID;
			g_pPacket->flag = 1 | 2;
			g_pPacket->seq_no = g_dwSeq++;
			g_pPacket->timestamp = DPGetTickCount();

			char buf[1280];
			if(dlen != fread(buf, 1, dlen, g_pLiuyanFile))
			{
				// 读取文件完毕
				fclose(g_pLiuyanFile);
				g_pLiuyanFile = NULL;
				g_bSaveStart = TRUE;
				return TRUE;
			}
			int length = SilkEncRun(g_hSilkEnc, buf, dlen, (char*)g_pPacket->pdata);
			length += _PACKET_HEAD_LEN;

			AES_SetEncInitVec(g_pAes, SET_KEY);
			AES_DataEncrypt(g_pAes, (BYTE*)g_pPacket, (BYTE*)g_pPacket, length);

			sendto(g_sockfd, (char*)g_pPacket, length, 0, (SOCKADDR*)&g_SockAddr, sizeof(SOCKADDR_IN));
		}
	}

	return TRUE;
}

static DWORD LiuYanRecv(HANDLE pParam)
{
	DBGMSG(DPINFO, "LiuYanRecv start\r\n");
	WORD seq_no = 0;
	BOOL bFindHead = TRUE;		// 丢包，先找到包头

	int nRecvLen;
	char buf[_PACKET_LEN];
	DPTransPacket *pPacket = (DPTransPacket*)buf;

	char pPcmBuf[640];
	DWORD dwPcmLen = 0;

	HANDLE hSilk = SilkDecCreate();

	int datalen = 0;
	int nCount = 0;

	DPGetSemaphore(g_hLYWaitEvent, 0);
	FILE* pFile = fopen(LIUYAN_FILE_PATH, "wb");
	fwrite(&nCount, 1, 4, pFile);

	while(g_bLiuyanRun)
	{
		nRecvLen = UdpRecv(g_sockfd, (char*)pPacket, _PACKET_LEN, 200, NULL);
		if(!g_bSaveStart)
			continue;

		if(nRecvLen > _PACKET_HEAD_LEN)
		{
			AES_SetDecInitVec(g_pAes, SET_KEY);
			AES_DataDecrypt(g_pAes, (BYTE*)pPacket, (BYTE*)pPacket, nRecvLen);

			if(pPacket->check != DP_CHECK_ID)
				continue;

			if(pPacket->flag != 3)
				continue;

			// 保存留言
			datalen = nRecvLen - _PACKET_HEAD_LEN;
			if(datalen > 0)
			{
				fwrite(&datalen, 1, 2, pFile);
				fwrite(pPacket->pdata, 1, datalen, pFile);
				nCount++;
			}

#if 0
			dwPcmLen = SilkDecRun(hSilk, (char*)pPacket->pdata, nRecvLen - _PACKET_HEAD_LEN, pPcmBuf);
			if(dwPcmLen == 320)
			{
				fwrite(pPacket->pdata, 1, _PACKET_PCM_LEN, pFile);
			}
#endif
		}
	}

	// 保存次数
	fseek(pFile, 0, SEEK_SET);
	fwrite(&nCount, 1, 4, pFile);
	fclose(pFile);

	// 文件保存完毕， 发送信号
	DPSetSemaphore(g_hLYWaitEvent);

	SilkDecDestroy(hSilk);

	DBGMSG(DPINFO, "LiuYanRecv end\r\n");
	return 0;
}

BOOL DPTransLiuyanStart(int ip, int port, int localport)
{
	if(ip == 0)
		return TRUE;

	DBGMSG(DPINFO, "DPTransLiuyanStart start ip:%x, port:%d, localport:%d\r\n", ip, port, localport);
	g_AudioCS.lockon();

	if(!g_bLiuyanRun && !g_bATransRun)
	{
		g_bLiuyanRun = TRUE;
		if(g_pAes == NULL)
		{
			g_pAes = AES_New();
			AES_SetMode(g_pAes, AES_MODE_SIC);
			AES_SetKey(g_pAes, INIT_KEY, 128);
		}

		if(g_hLYWaitEvent == NULL)
			g_hLYWaitEvent = DPCreateSemaphore(0, 1);

		g_sockfd = UdpCreate(localport);
		if(!UdpSetRecvBuf(g_sockfd, 60000))
		{
			DBGMSG(DPERROR, "DPTrans UdpSetRecvBuf\r\n");
			return 0;
		}

		g_bSaveStart = FALSE;

		g_pLiuyanFile = fopen("/FlashDev/sound/leaveword_tip.wav", "rb");
		AudioSendInit(ip, port);
		AudioRecStart(g_hAudioRec, 8000, 1, LiuyanSend, NULL);
		g_hLiuyanRecv = DPThreadCreate(0x4000, LiuYanRecv, NULL, TRUE, 1);
	}

	g_AudioCS.lockoff();
	return TRUE;
}

BOOL DPTransLiuyanStop()
{
	g_AudioCS.lockon();
	if(g_bLiuyanRun)
	{
		DWORD tick = DPGetTickCount();
		g_bLiuyanRun = FALSE;
		SocketClose(g_sockfd);
		g_sockfd = INVALID_SOCKET;
		AudioSendUnInit();
		DPThreadJoin(g_hLiuyanRecv);
		g_hLiuyanRecv = NULL;
		if(g_pLiuyanFile)
		{
			fclose(g_pLiuyanFile);
			g_pLiuyanFile = NULL;
		}
		DBGMSG(DPINFO, "DPTransLiuyanStop cost %dms\r\n", DPGetTickCount() - tick);
	}
	g_AudioCS.lockoff();
	return TRUE;
}

void DPTransLiuyanWait()
{
	if(g_hLiuyanRecv == NULL)
		return;

	DPGetSemaphore(g_hLYWaitEvent, 3000);
}