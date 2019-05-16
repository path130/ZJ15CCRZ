/*
 * speech_pro.h
 *
 *  Created on: 2017年7月4日
 *      Author: chli
 */

#ifndef SPEECH_SPEECH_PRO_H_
#define SPEECH_SPEECH_PRO_H_
#include "volume_set.h"


/**
 *
 * 启动语音对讲
 * @param [in] ip 音频数据发送的目标ＩＰ
 * @param [in] capture_volume 对讲的录音音量，不设置填-1，其他大于-1的值设置音量
 * @param [in] paly_volume 对讲的放音音量，不设置填-1，其他大于-1的值设置音量
 * @param [in] mode 对讲模式，默认填０
 * @return 成功返回0，否则返回-1
*/
int  speech_start(char *ip,int capture_volume,int paly_volume,int mode);
int  cloud_speech_start(char *ip,int capture_volume,int paly_volume,int mode);
/**
 *
 * 停止语音对讲
 *
 */
int  speech_stop(void);

int speech_volume_set(int volume);

#endif /* SPEECH_SPEECH_PRO_H_ */
