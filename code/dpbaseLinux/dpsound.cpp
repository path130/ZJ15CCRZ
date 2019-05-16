#include "roomlib.h"
#include "AudioCore.h"
#include "MP3dec.h"

static StaticLock g_SoundCS;
#define NCHANS          2           // max 2 channels
#define NBLOCK       1152           // MP3 one frame max 1152 sample
#define OUTBUFSIZE   NBLOCK*NCHANS  // output pcm buffer size,  must >= NBLOCK*NCHANS

#define	BLOCKTIME         20		//每个缓冲数据块的时长（毫秒）
#define	PLAYBLOCKS        4			//播放最大缓冲数据块数
#define	RECORDBLOCKS      4			//录音最大缓冲数据块数

class CAudioPlay
{
public:
	CAudioPlay()
	{
		m_threadid = 0;
		itimes = 0;
		bPlay = false;
		m_Volume= 90;
		m_pdata = NULL;
	}

	~CAudioPlay()
	{
		m_threadid = 0;
		itimes = 0;
		bPlay = false;
	}

	bool PlayMP3(char *filename,int times)
	{
		if(bPlay == false)
		{
			DBGMSG(SND_MOD, "PlayMp3 filename:%s\n",filename);
			itimes = times;
			strcpy(filepath,filename);
			bPlay = true;

			int ret = pthread_create(&m_threadid, NULL, PlayMp3FileStub, this);
		}
		return true;
	}

	bool PlayWav(const char *filename,int times)
	{
		if(bPlay == false)
		{
			DBGMSG(SND_MOD, "PlayWav filename:%s\n",filename);
			itimes = times;
			strcpy(filepath,filename);
			bPlay = true;

			pthread_create(&m_threadid, NULL, PlayWavFileStub, this);
		}
		return true;
	}

	bool PlayData(char *pdata, int len, int channels, int samplebits,int samplerate, int times)
	{
		if(bPlay == false)
		{
			DBGMSG(SND_MOD, "PlayData :%x %d\n", pdata, len);
			bPlay = true;
			m_pdata = pdata;
			m_datalen = len;
			itimes = times;
			m_iChannels = channels;
			m_iSampleBits = samplebits;
			m_iSampleRate = samplerate;
			pthread_create(&m_threadid, NULL, PlayAudioDataStub, this);
		}
		return true;
	}

	bool Stop()
	{
		if(bPlay == true)
		{
			itimes = 0;
			bPlay = false;
			if(m_threadid != 0)
			{
				DBGMSG(SND_MOD, "Sound join thread:%x\n", m_threadid);
				pthread_join(m_threadid, NULL);
				m_threadid = 0;
				DBGMSG(SND_MOD, "Sound join thread ok\n");
			}
			if(m_pdata != NULL)
				free(m_pdata);
			m_pdata = NULL;
		}
		return true;
	}

	void SetVolume(int dVol)
	{
		m_Volume = dVol;
		bool bret = Audio_Player_SetVolume(m_nStreamID, dVol);
	}

	// 播放MP3文件，线程函数
	static void * PlayMp3FileStub(void * param)
	{
		CAudioPlay * pAudioplay = (CAudioPlay *) param;
		pAudioplay->PlayMp3FileThread();
		return NULL;
	}

	void PlayMp3FileThread(void)
	{
		MP3File    mp3file;
		AudioInfo *mp3info;
		int    iret, nsamples;
		int   CurrTime = 0;

		//open input file
		iret = MP3File_Open(&mp3file, filepath);
		if (iret != MP3DEC_ERR_NONE)
		{
			DBGMSG(DPERROR, "PlayMp3FileThread open file error:%s\n", filepath);
			return;
		}

		// get audio info
		mp3info = (AudioInfo*) mp3file;
		DBGMSG(SND_MOD, "mp3 format :%d %d %d\n", mp3info->ChannelNumber, mp3info->SampleBit, mp3info->SampleRate);

		int SamplesPerBlock = (mp3info->SampleRate < 32000 ? 576 : 1152);
		m_nStreamID = Audio_Player_Open();
		if (m_nStreamID == 0)	
		{
			DBGMSG(DPERROR, "Audio_Player_Open failed.\n");
			MP3File_Close(mp3file);
			return;
		}

		Audio_Player_SetFormat(m_nStreamID, mp3info->ChannelNumber, mp3info->SampleBit, mp3info->SampleRate);
		Audio_Player_SetCache (m_nStreamID, PLAYBLOCKS, SamplesPerBlock);
		Audio_Player_SetMode  (m_nStreamID, true, NULL, NULL);
		Audio_Player_SetVolume(m_nStreamID, m_Volume);

		if (Audio_Player_Start(m_nStreamID) == false)
		{
			DBGMSG(DPERROR, "Audio_Player_Start failed.\n");
			Audio_Player_Close(m_nStreamID);
			MP3File_Close (mp3file);
			return;
		}

		int BlockSize = SamplesPerBlock * mp3info->ChannelNumber * (mp3info->SampleBit / 8);
		int BlockTime = SamplesPerBlock * 1000 / mp3info->SampleRate;

		short* outbuf = (short *)malloc(sizeof(short) * BlockSize);
		memset(outbuf, 0, BlockSize);
		int nSleepTime = (float)SamplesPerBlock / mp3info->SampleRate * 8 * 1000;
		//main loop to decode MP3 file
		while(bPlay && itimes > 0)
		{
			// try to decode current frame
			iret = MP3File_Decode (mp3file, (short *)outbuf, &nsamples, &CurrTime);
			DBGMSG(SND_MOD, "mp3 dec :%d %d %d\n", iret, nsamples, CurrTime);
			if (iret == MP3DEC_ERR_NOMOREDATA)
			{
				if(itimes > 0)
				{
					itimes --;
					MP3File_Seek (mp3file, 0, &CurrTime);
					continue;
				}
				else
					break;
			}
			else if (iret == MP3DEC_ERR_NONE)
			{
			}
			else
			{
				DBGMSG(DPERROR, "MP3 decoder fatal inner error !\n");
				struct timespec ts;
				ts.tv_sec=nSleepTime/1000;
				ts.tv_nsec=(nSleepTime % 1000)*1000000LL;
				nanosleep(&ts,NULL);
				break;
			}
			//播放
			if (!Audio_Player_Write(m_nStreamID, (char*)outbuf, BlockSize, BlockTime * PLAYBLOCKS))
			{
				DBGMSG(DPERROR, "Audio_Player_Write error!\n");
				if (!Audio_Player_Write(m_nStreamID, (char*)outbuf, BlockSize, BlockTime * PLAYBLOCKS))
				{
					DBGMSG(DPERROR, "Audio_Player_Write error!\n");
					break;
				}
			}
		}
		if(bPlay)
			usleep(BLOCKTIME * PLAYBLOCKS * 1000);

		Audio_Player_Stop(m_nStreamID);
		Audio_Player_Close(m_nStreamID);
		MP3File_Close(mp3file);

		free(outbuf);
		m_nStreamID = 0;
		bPlay = false;
		itimes = 0;
		return;
	}

	// 播放WAV格式的声音线程函数
	static void * PlayWavFileStub(void * param)
	{
		CAudioPlay * pAudioplay = (CAudioPlay *) param;
		pAudioplay->PlayWavFileThread();
		return NULL;
	}

	void PlayWavFileThread(void)
	{
		FILE *hfile;
		WaveSound	head;
		char *BlockBuf;
		int SamplesPerBlock;	// 每次播放的声音数据采样数，BLOCKTIME时间长度的数据
		unsigned int BlockSize;	// 每次播放的声音数据字节数，BLOCKTIME时间长度的数据

		hfile = fopen(filepath, "rb");
		if(!hfile)
		{
			DBGMSG(DPERROR, "Open file %s FAILED !!!\n", filepath);
			return;
		}

		fread(&head, 1, sizeof(WaveSound),hfile);
		DBGMSG(SND_MOD, "Wave format: %d Channal, %d Rate %d bits\n", head.wNChannels, head.dwSamplingRate,head.wNBitsPerSam);

		m_nStreamID = Audio_Player_Open();
		if (m_nStreamID == 0)	
		{
			DBGMSG(DPERROR, "Audio_Player_Open failed!\n");
			fclose(hfile);
			return;
		}
		SamplesPerBlock = BLOCKTIME * head.dwSamplingRate/1000;
		BlockSize = SamplesPerBlock * head.wNChannels * head.wNBitsPerSam / 8;

		Audio_Player_SetFormat(m_nStreamID, head.wNChannels, head.wNBitsPerSam, head.dwSamplingRate);
		Audio_Player_SetCache (m_nStreamID, PLAYBLOCKS, SamplesPerBlock);
		// 同步模式，直接调用写函数，每次写入一块数据
		Audio_Player_SetMode  (m_nStreamID, true, NULL, NULL);
		Audio_Player_SetVolume(m_nStreamID, m_Volume);

		if(Audio_Player_Start(m_nStreamID) == false)
		{
			DBGMSG(DPERROR, "Audio_Player_Start error\n");
			fclose(hfile);
			Audio_Player_Close(m_nStreamID);
			return;
		}

		BlockBuf = (char *)malloc(BlockSize);
		memset(BlockBuf, 0, BlockSize);

		DBGMSG(SND_MOD, "Lens: %d %d \n", SamplesPerBlock, BlockSize);

		while(bPlay && itimes > 0)
		{
			unsigned int iread;
			iread = fread(BlockBuf, 1, BlockSize, hfile);
			if(iread < BlockSize)
			{
				if(itimes > 1)
				{
					unsigned int itemlen;
					fseek(hfile,sizeof(WaveSound),SEEK_SET);
					itemlen = fread(&BlockBuf[iread], 1, BlockSize - iread, hfile);
					if(itemlen < BlockSize-iread)
					{
						DBGMSG(DPERROR, "fread error break loop \n");
						break;
					}
				}
				else
					memset(&BlockBuf[iread], 0, BlockSize - iread);
				itimes --;
			}
			if(Audio_Player_Write(m_nStreamID, BlockBuf, BlockSize, BLOCKTIME * PLAYBLOCKS) == false)
			{
				DBGMSG(DPERROR, "Audio_Player_Write false break loop \n");
				break;			
			}
		}
		if(bPlay)
			usleep(BLOCKTIME * PLAYBLOCKS * 1000);
		Audio_Player_Stop(m_nStreamID);
		Audio_Player_Close(m_nStreamID);
		fclose(hfile);
		DBGMSG(SND_MOD, "playwave end %d %d\n", bPlay, itimes);
		hfile = NULL;
		free(BlockBuf);
	}

	// 播放声音数据线程函数
	static void * PlayAudioDataStub(void * param)
	{
		CAudioPlay* pAudioplay = (CAudioPlay *)param;
		pAudioplay->PlayAudioDataThread();
		return NULL;
	}

	void PlayAudioDataThread(void)
	{
		char *BlockBuf;
		int SamplesPerBlock;	// 每次播放的声音数据采样数，BLOCKTIME时间长度的数据
		unsigned int BlockSize;	// 每次播放的声音数据字节数，BLOCKTIME时间长度的数据
		int ptr;

		m_nStreamID = Audio_Player_Open();
		if (m_nStreamID == 0)	
		{
			DBGMSG(DPERROR, "Audio_Player_Open failed!\n");
			return;
		}
		DBGMSG(SND_MOD, "PlayAudioDataThread: %d %d %d\n", m_iChannels, m_iSampleBits, m_iSampleRate);

		SamplesPerBlock = BLOCKTIME * m_iSampleRate/1000;
		BlockSize = SamplesPerBlock * m_iChannels * m_iSampleBits / 8;

		DBGMSG(SND_MOD, "Lens: %d %d \n", SamplesPerBlock, BlockSize);

		Audio_Player_SetFormat(m_nStreamID, m_iChannels, m_iSampleBits, m_iSampleRate);
		Audio_Player_SetCache (m_nStreamID, PLAYBLOCKS, SamplesPerBlock);
		// 同步模式，直接调用写函数，每次写入一块数据
		Audio_Player_SetMode  (m_nStreamID, true, NULL, NULL);
		Audio_Player_SetVolume(m_nStreamID, m_Volume);

		if(Audio_Player_Start(m_nStreamID) == false)
		{
			DBGMSG(DPERROR, "Audio_Player_Start error\n");
			Audio_Player_Close(m_nStreamID);
			return;
		}

		BlockBuf = (char *)malloc(BlockSize);
		memset(BlockBuf, 0, BlockSize);

		ptr = 0;
		while(bPlay && itimes > 0)
		{
			if((ptr + BlockSize) <= m_datalen)
			{
				memcpy(BlockBuf, m_pdata + ptr, BlockSize);
				ptr += BlockSize;
			}
			else
			{
				int cplen;
				cplen = m_datalen - ptr;
				memcpy(BlockBuf, m_pdata + ptr, cplen);
				if(itimes > 1)
				{
					ptr = 0;
					memcpy(BlockBuf + cplen, m_pdata + ptr, BlockSize - cplen);
					ptr += BlockSize - cplen;
					itimes--;
				}
				else
					memset(BlockBuf + cplen, 0, BlockSize - cplen);
			}
			if(Audio_Player_Write(m_nStreamID, BlockBuf, BlockSize, BLOCKTIME * PLAYBLOCKS) == false)
			{
				DBGMSG(DPERROR, "Audio_Player_Write false break loop \n");
				break;			
			}
		}
		if(bPlay)
			usleep(BLOCKTIME * PLAYBLOCKS * 1000);
		Audio_Player_Stop(m_nStreamID);
		Audio_Player_Close(m_nStreamID);
		free(BlockBuf);
		DBGMSG(SND_MOD, "playbuf end %d %d\n", bPlay, itimes);
	}

	bool bPlay;
	int m_nStreamID;
	int itimes;
	pthread_t m_threadid;
	int m_Volume;
	char filepath[512];
	char* m_pdata;
	int m_datalen;
	int m_iChannels;
	int m_iSampleBits;
	int m_iSampleRate;
protected:
};

HANDLE PlayMp3(char *filename,int times)
{
	if(filename == NULL)
	{
		return NULL;
	}
	int iLen = strlen(filename);
	if (iLen < 5)
	{
		return NULL;
	}
	int ftype;
	if(strncmp(".mp3", &filename[iLen - 4], 4) == 0)
		ftype = 1;
	else if(strncmp(".wav", &filename[iLen - 4], 4) == 0)
		ftype = 2;
	else
		return NULL;

	CAudioPlay * pplay = new CAudioPlay;
	if(pplay == NULL)
		return NULL;

	bool bRet = false;
	if(ftype == 1)
	{
		bRet = pplay->PlayMP3(filename,times);
	}
	else
		bRet = pplay->PlayWav(filename,times);
	if(bRet)
		return (HANDLE)pplay;
	delete pplay;
	pplay = NULL;
	return NULL;
}

void StopMp3(HANDLE hMp3)
{
	CAudioPlay * pAudio = (CAudioPlay*)hMp3;
	if(pAudio)
	{
		pAudio->Stop();
		delete pAudio;
		pAudio = NULL;
	}
}

void SetMp3Volume(HANDLE hMp3,DWORD dVol)
{
	CAudioPlay * pAudio = (CAudioPlay*)hMp3;
	dVol = ((dVol >> 16) * 100) / 0xffff;
	if(pAudio)
		pAudio->SetVolume((int)dVol);
}

int GetPlayState(HANDLE hPlay)
{
	CAudioPlay * pplay = (CAudioPlay*)hPlay;
	if (pplay)
	{
		return pplay->bPlay;
	}
	return -1;
}

static CAudioPlay m_PlayWav;
static CAudioPlay m_PlayAlarm;
static CAudioPlay m_PlayLiuyan;

void PlayWav(DWORD wavindex, DWORD vol)
{
	char filename[64];

	switch(wavindex)
	{
	case KEYPAD_INDEX:
		strcpy(filename, "/FlashDev/sound/keyvol.wav");
		break;
	case DUDU_INDEX:
		strcpy(filename, "/FlashDev/sound/timeover15.wav");
		break;
	case STARTUP_INDEX:
		strcpy(filename, "/FlashDev/sound/startup.wav");
		break;
	case LONGPRESS_INDEX:
		strcpy(filename, "/FlashDev/sound/timeover20.wav");
		break;
	case CALL_BUSY:
		strcpy(filename, "/FlashDev/sound/busy.wav");
		break;
	case MESSAGE_INDEX:
		strcpy(filename, "/FlashDev/sound/msgring2.wav");
		break;
	case OK_INDEX:
		strcpy(filename, "/FlashDev/sound/timeover10.wav");
		break;
	}
	g_SoundCS.lockon();
	m_PlayWav.Stop();
	m_PlayWav.PlayWav(filename, 1);
	vol = vol* 100 / 0xf;
	m_PlayWav.SetVolume(vol);
	g_SoundCS.lockoff();
}

static DWORD m_AlarmVol = 0xffffffff;
HANDLE StartAlarmWav(DWORD index)
{
	if(index == 0)
		m_PlayAlarm.PlayWav("/FlashDev/sound/timeover20.wav", 0x7fffffff);
	else
		m_PlayAlarm.PlayWav("/FlashDev/sound/alarms.wav", 0x7fffffff);
	return &m_PlayAlarm;
}

void StopAlarmWav(HANDLE hdev)
{
	m_PlayAlarm.Stop();
}

void SetAlarmVolume(DWORD dVol)
{
	dVol = ((dVol >> 16) * 100) / 0xffff;
	m_PlayAlarm.SetVolume(dVol);
	m_AlarmVol = dVol;
}

static HANDLE m_hMp3Play;		// 铃声播放句柄
static DWORD lastring = 0;

void StartPlayMp3(DWORD index, DWORD vol, DWORD times)
{
	g_SoundCS.lockon();
	if(m_hMp3Play != NULL)
	{
		StopMp3(m_hMp3Play);
		m_hMp3Play = NULL;
	}
	if(m_hMp3Play == NULL)
	{
		char mp3File[64];
		switch(index)
		{
		case PHOTO_MP3:
			strcpy(mp3File, "/FlashDev/sound/photoshutter.mp3");
			break;
		default:
			mp3File[0] = 0;
			break;
		}
		m_hMp3Play = PlayMp3((char*)mp3File, times);
		if(m_hMp3Play)
		{
			SetMp3Volume(m_hMp3Play, vol * 0x11111111);
		}
	}
	g_SoundCS.lockoff();
}

void ChangeVolume(DWORD vol)
{
	g_SoundCS.lockon();
	if(m_hMp3Play != NULL)
	{
		SetMp3Volume(m_hMp3Play, (DWORD)(vol * 0x11111111));
	}
	g_SoundCS.lockoff();
}

void StopPlayMp3(DWORD ring)
{
	g_SoundCS.lockon();
	if(m_hMp3Play != NULL)
	{
		StopMp3(m_hMp3Play);
		m_hMp3Play = NULL;
	}
	g_SoundCS.lockoff();
}

void StartLiuYan(void* pbuf, DWORD len)
{
	m_PlayLiuyan.Stop();
	m_PlayLiuyan.PlayData((char*)pbuf, len, 1, 16, 8000, 1);
	DBGMSG(DPINFO, "StartLiuYan\r\n");
}

void StopLiuYan(void)
{
	m_PlayLiuyan.Stop();
	DBGMSG(DPINFO, "StopLiuYan\r\n");
}

