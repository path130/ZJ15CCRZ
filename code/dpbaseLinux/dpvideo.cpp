#include "roomlib.h"
#include <dpgpio.h>
#include <fcntl.h>

#define DBGMSG(x, ...) printf("DBGMSG");
static int g_hVideoDec = -1;
BOOL JPGDecStart(int left, int top, int width, int height)
{
	Vdec_Info decinfo;
	int ret;

	if(g_hVideoDec < 0)
	{
		memset(&decinfo, 0, sizeof(Vdec_Info));

		decinfo.decode.m_winl = left;
		decinfo.decode.m_wint = top;
		decinfo.decode.m_winw = width;
		decinfo.decode.m_winh = height;
		decinfo.decode.m_decw = 720;
		decinfo.decode.m_dech = 576;
		decinfo.decode.m_dectype = DECODE_JPEG;
		decinfo.decode.m_outtype = DECODE_OUTDIR;

		g_hVideoDec = open("/dev/encdec", O_RDWR, 0);
		if(g_hVideoDec < 0)
		{
			printf("open /dev/encdec fail\n");
			return FALSE;
		}

		ret = ioctl(g_hVideoDec, IOCTL_DECODE_START, &decinfo);
		if(ret < 0)
		{
			printf("IOCTL_PREVIEW_START fail\r\n");
			close(g_hVideoDec);
			return	FALSE;
		}
	}
	return TRUE;
}

BOOL H264DecStart(int left, int top, int width, int height)
{
	Vdec_Info decinfo;
	int ret;

	if(g_hVideoDec < 0)
	{
		memset(&decinfo, 0, sizeof(Vdec_Info));

		decinfo.decode.m_winl = left;
		decinfo.decode.m_wint = top;
		decinfo.decode.m_winw = width;
		decinfo.decode.m_winh = height;
		decinfo.decode.m_decw = 720;
		decinfo.decode.m_dech = 576;
		decinfo.decode.m_dectype = DECODE_H264;
		decinfo.decode.m_outtype = DECODE_OUTDIR;

		g_hVideoDec = open("/dev/encdec", O_RDWR, 0);
		if(g_hVideoDec < 0)
		{
			printf("open /dev/encdec fail\n");
			return FALSE;
		}

		ret = ioctl(g_hVideoDec, IOCTL_DECODE_START, &decinfo);
		if(ret < 0)
		{
			printf("IOCTL_PREVIEW_START fail\r\n");
			close(g_hVideoDec);
			return	FALSE;
		}
	}
	return TRUE;
}

void H264WriteData(char * pdata,DWORD dlen, DWORD ptr)
{
	Vdec_Info video_info;
	if(g_hVideoDec > 0)
	{
		memset(&video_info, 0, sizeof(Vdec_Info));
		video_info.wbuf.dwsize = dlen;
		video_info.wbuf.buf = (BYTE*)pdata;
		video_info.wbuf.dec_pts = ptr;
		ioctl(g_hVideoDec,IOCTL_WRITE_BUF, &video_info);
	}
}

typedef struct
{
	union
	{
		Preview_Info	preview;
		Decode_Info		decode;
		Encode_Info		encode;
		READ_INFO		rbuf;
		WRITE_INFO		wbuf;
		Sensor_Prop		spop;
		Sensor_Set		sset;
	} params;
} Video_Info;

void H264SetDisplayRect(int left, int top, int width, int height)
{
	Video_Info video_info;
	if(g_hVideoDec > 0)
	{
		memset(&video_info, 0, sizeof(Video_Info));
		video_info.params.decode.m_winl = left;
		video_info.params.decode.m_wint = top;
		video_info.params.decode.m_winw = width;
		video_info.params.decode.m_winh = height;
		ioctl(g_hVideoDec,IOCTL_DECODE_SET, &video_info);
	}
}

void H264DecStop(void)
{
	if(g_hVideoDec > 0)
	{
		ioctl(g_hVideoDec, IOCTL_DECODE_STOP, NULL);
		close(g_hVideoDec);
		g_hVideoDec = -1;
	}
}

// linux 编码要设置摄像头分辨率
static void bicubic(DWORD *cubic_coeff, DWORD ratio, int tapes)
{
	int i, j;
	float step;
	float s[4];
	float c[4];
	float sum;

	if(ratio < 256)
		ratio = 256;

	for(i=0; i<tapes; i++)
	{
		step = (float)i;
		step /= tapes;
		s[0] = (step + 1)*256/ratio;
		s[1] = (step + 0)*256/ratio;
		s[2] = (1 - step)*256/ratio;
		s[3] = (2 - step)*256/ratio;

		sum = 0;
		for(j=0; j<4; j++)
		{
			if(s[j] <= 1)
			{
				c[j] = (float)(1.5*s[j]*s[j]*s[j] - 2.5*s[j]*s[j] + 1);
			}
			else if(s[j] <= 2)
			{
				c[j] = (float)(-0.5*s[j]*s[j]*s[j] + 2.5*s[j]*s[j] - 4*s[j] + 2);
			}
			else
			{
				c[j] = 0;
			}
			sum += c[j];
		}
		if(sum==1)
		{
			cubic_coeff[2*i+0] = (((int)(c[0]*256+0.5))&0xffff) + (((int)(c[1]*256+0.5))<<16);
			cubic_coeff[2*i+1] = (((int)(c[2]*256+0.5))&0xffff) + (((int)(c[3]*256+0.5))<<16);
		}
		else
		{
			cubic_coeff[2*i+0] = (((int)(c[0]/sum*256+0.5))&0xffff) + (((int)(c[1]/sum*256+0.5))<<16);
			cubic_coeff[2*i+1] = (((int)(c[2]/sum*256+0.5))&0xffff) + (((int)(c[3]/sum*256+0.5))<<16);
		}
	}
}

HANDLE VideoEncStart(DWORD format, DWORD width, DWORD height, DWORD qu)
{
	HANDLE hHandle;
	Vdec_Info   videoinfo;
	Encode_Info encinfo;

	hHandle = (HANDLE)open("/dev/encdec", O_RDWR, 0);
	if(hHandle < 0)
	{
		DBGMSG(DPERROR, "%s(%d): $$$$$$$$$ open /dev/encdec fail $$$$$$$$$\n", __FILE__, __LINE__);
		return INVALID_HANDLE_VALUE;
	}
	memset(&videoinfo, 0, sizeof(Vdec_Info));

	encinfo.m_enctype = ENCODE_MP4;
	encinfo.m_encw = width;
	encinfo.m_ench = height;
		
	//if(size == 5/*V_CIF_P*/)
	//{
	//	encinfo.m_encw = 352;
	//	encinfo.m_ench = 288;
	//}
	//else
	//{
	//	encinfo.m_encw = 720;
	//	encinfo.m_ench = 576;
	//}

	encinfo.m_quality   = qu;
	encinfo.property    = ENCODE_QUALITY;
	encinfo.m_maxkey    = 25;

	// 获取摄像头分辨率
	if(ioctl((int)hHandle, IOCTL_SIF_GETCAP, &videoinfo) < 0)
	{
		DBGMSG(DPERROR, "%s(%d): $$$$$$$$$ encdec ioctl_sif_getcap $$$$$$$$$\n", __FILE__, __LINE__);
		close((int)hHandle);
		return  INVALID_HANDLE_VALUE;
	}

	DBGMSG(DPINFO, "%s(%d): $$$$$$$$$ enc H: %d enc W: %d $$$$$$$$$\n", __FILE__, __LINE__, encinfo.m_ench, encinfo.m_encw);

	SIF_SIZE sifSize;
	sifSize.Width   = encinfo.m_encw;
	sifSize.Height  = encinfo.m_ench;
	int nIndex = 0;
	for(nIndex = 0; nIndex < 6; nIndex++)
	{
		if(encinfo.m_encw == videoinfo.spop.OutSize[nIndex].Width && encinfo.m_ench == videoinfo.spop.OutSize[nIndex].Height)
			break;
	}
	if(nIndex == 6)
	{
		sifSize.Width   = videoinfo.spop.OutSize[0].Width;
		sifSize.Height  = videoinfo.spop.OutSize[0].Height;
	}

	//设置摄像头分辨率
	memset(&videoinfo, 0, sizeof(videoinfo));
	videoinfo.sset.SifProperty = SIF_SETOUTSIZE;
	videoinfo.sset.val = (sifSize.Width << 16) | sifSize.Height;
	if(ioctl((int)hHandle, IOCTL_SIF_CONTROL, &videoinfo) < 0)
	{
		DBGMSG(DPERROR, "%s(%d): $$$$$$$$$ encdec ioctl_sif_control $$$$$$$$$\n", __FILE__, __LINE__);
		close((int)hHandle);
		return  INVALID_HANDLE_VALUE;
	}

	if(ioctl((int)hHandle, IOCTL_ENCODE_START, &encinfo) < 0)
	{
		DBGMSG(DPERROR, "%s(%d): $$$$$$$$$ ioctl_encode_start fail $$$$$$$$$\n", __FILE__, __LINE__);
		close((int)hHandle);
		return	INVALID_HANDLE_VALUE;
	}

	//如果编码和摄像头大小不一样，设置编码缩放参数
	if(encinfo.m_encw != sifSize.Width)
	{
		DWORD horRatio = (sifSize.Width * 256 + encinfo.m_encw / 2) / encinfo.m_encw;
		DWORD scaler_coeff[64] = {0};
		bicubic(scaler_coeff, horRatio, 16);
		encinfo.property = ENCODE_SET_SCALER;
		encinfo.m_scalerinfo = (DWORD)scaler_coeff;

		if(ioctl((int)hHandle, IOCTL_ENCODE_SET, &encinfo) < 0)
		{
			DBGMSG(DPERROR, "%s(%d): $$$$$$$$$ encdec ioctl_encode_set fail $$$$$$$$$\n", __FILE__, __LINE__);
			close((int)hHandle);
			return INVALID_HANDLE_VALUE;
		}
	}

	DBGMSG(DPINFO, "VideoEncStart success\r\n");
	return hHandle;
}

DWORD VideoEncSetQuality(HANDLE hHandle, DWORD qu)
{
	Encode_Info encinfo;
	encinfo.m_quality = qu;
	encinfo.property = ENCODE_QUALITY;
	if(ioctl((int)hHandle, IOCTL_ENCODE_SET, &encinfo) < 0)
	{
		DBGMSG(DPERROR, "%s(%d): $$$$$$$$$ ioctl_encode_set fail $$$$$$$$$\n", __FILE__, __LINE__);
	}
	return 0;
}

DWORD VideoEncEnable(HANDLE hHandle, BOOL bOn)
{
	Encode_Info encinfo = {0};
	if(bOn)
		encinfo.property = ENCODE_BLANK_OFF;
	else
		encinfo.property = ENCODE_BLANK_ON;
	encinfo.m_bcolor = 0x808080;
	if(ioctl((int)hHandle, IOCTL_ENCODE_SET, &encinfo) < 0)
	{
		DBGMSG(DPERROR, "%s(%d): $$$$$$$$$ ioctl_encode_set fail $$$$$$$$$\n", __FILE__, __LINE__);
	}
	return 0;
}

DWORD VideoEncRead(HANDLE hHandle, BYTE* pData, DWORD len, DWORD* property)
{
	Vdec_Info vinfo;

	memset(&vinfo, 0, sizeof(Vdec_Info));
	vinfo.rbuf.dwsize = len;
	vinfo.rbuf.buf = pData;
	vinfo.rbuf.timeout = 1000;
	vinfo.rbuf.property = *property;
	if(ioctl((int)hHandle, IOCTL_READ_BUF, &vinfo) < 0)
	{
		//DBGMSG(DPERROR, "VideoEncRead ioctl_read_buf fail\r\n");
		return 0;
	}
	//DBGMSG(DPINFO, "%s(%d): $$$$$$$$$ enc size %d  property: %d $$$$$$$$$\n", __FILE__, __LINE__, vinfo.rbuf.dwsize, vinfo.rbuf.property);
	*property = vinfo.rbuf.property;
	return vinfo.rbuf.dwsize;
}


void VideoEncStop(HANDLE hHandle)
{
	if(hHandle != INVALID_HANDLE_VALUE)
	{
		ioctl((int)hHandle, IOCTL_ENCODE_STOP, NULL);
		close((int)hHandle);
	}
}
