/*
 * speech_def.h
 *
 *  Created on: 2012-11-14 下午8:06:10
 *  
 */

#ifndef SPEECH_DEF_H_
#define SPEECH_DEF_H_
#include "public.h"
#include "global_def.h"


//音量

//#if DEV_GLJKZQ

/*		 Item #0 'Mic Bias_dowm'
		 Item #1 'Mic Bias 2V'
		 Item #2 'Mic Bias 2.5V'
		 Item #3 'Mic Bias AVDD'
*/

#define HAVE_UDP_SPEECH 1

#define RTP_SPEECH 0x01
#define UDP_SPEECH 0x02
#define LOCAL_SPEECH 0x03

#define REG_43_VAL (0xff&0x7f)
#if DEV_CONTROL|DEV_GLJKZQ
#define REG_16_VAL (0xaa&0x7f)
#define REG_17_VAL (0x0)
#define REG_18_VAL (0x1)
#define REG_19_VAL (0x1)
#define REG_25_VAL (0x00)
#define REG_73_VAL (0)
#if DEV_CONTROL
#define REG_86_VAL (0x99)
#endif
#else
#define REG_16_VAL (0xac&0x7f)
#define REG_17_VAL (0x1)
#define REG_18_VAL (0x1)
#define REG_19_VAL (0x0)
#define REG_25_VAL (0x2)
#define REG_73_VAL (2)
#define REG_86_VAL (0x69)
#endif

// SND
#define SND_LEN (160)// 320+160// 160
#define UDP_SND_LEN (256)
#define SND_WAITE (1000)
//#define AIC_3007
#define SND_PCM_LEN (320)   //for cloud
#define UDP_SND_PCM_LEN (480)

/*
 * 音频初始设置信息
 */
#define SPEECH_MAX_LEN (300*1024)

#define SPEECH_RATE 8000									//采样频率
//#define SPEECH_FORMAT SND_PCM_FORMAT_S16_LE
#define SPEECH_BIT_WIDE 16
//#define FORMAT 	SND_PCM_FORMAT_U8   		//量化位数
#define SPEECH_CHANNELS 1 								//声道数目

/*
 * Ring_buf
 */
#define RING_BUF_N 5



#endif /* SPEECH_DEF_H_ */

