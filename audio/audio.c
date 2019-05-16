
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "wav_play.h"
#include "public.h"
#include "audio.h"
#include "msg.h"

static pipe_handle      audio_pipe_audio;
static pthread_t        audio_tid;

void *audio_thread(void *arg)
{
    unsigned char  msg[MSG_LEN_MAX];
   
    while(1) {
        if (pipe_get(audio_pipe_audio, msg, MSG_LEN_AUDIO) < 0) {
            usleep(100*1000);
            continue;
        }
        app_debug(DBG_INFO, "audio get msg:\t%d %d\n", msg[0], msg[1]);
        switch(msg[1]) {
            case PMT_TEST:
                wav_play(WAV_ECHORING, 1, msg[2]);
                break;
            case PMT_WELCOME:
                wav_play(WAV_WELCOME, 1, msg[2]);
                break;
            case PMT_ECHORING:
                wav_play(WAV_ECHORING, 10, msg[2]);
                break;
            case PMT_UNLOCK:
                wav_play(WAV_UNLOCK, 1, msg[2]);
                break;
            case PMT_COME_IN:
                wav_play(WAV_COME_IN, 1, msg[2]);
                break;
            case PMT_CALL_USER:
                wav_play(WAV_CALL_USER, 1, msg[2]);
                break;
            case PMT_NO_ANSWER:
                wav_play(WAV_NO_ANSWER, 1, msg[2]+10);
                break;
	     case PMT_ALARM:               //by mkq 20170907
	       //wav_play(WAV_ALARM, 4, msg[2]);//1 change by mkq
                break;	
            default:
                wav_play(NULL, 1, msg[2]);
                break;
        }
        //wav_wait_stop();
    }
    exit(-1);
    return THREAD_SUCCESS;
}

void audio_play(prompt_e pmt, int vol)
{
    unsigned char  msg[MSG_LEN_AUDIO];
    switch(wav_status_get()) {
        case WAV_PLAYING:
            wav_stop();
            break;

        case WAV_STOPED:
        case WAV_STOPING:
            break;
        case WAV_DEVICE_BUSY:
            printf("WAV_DEVICE_BUSY !\n");
            return;
       /* case WAV_PLAY_HI:
            printf("WAV_PLAY_HI !\n");
            return;*/
        case WAV_PLAY_START:
            printf("WAV_PLAY_STARTED !\n");
            return;
    }

    //app_debug(DBG_INFO, "audio play vol:\t%d\n", dev_vol_play[dev_cfg.level_volume]);

    wav_status_set(WAV_PLAY_START);

    msg[0] = MSG_FROM_AUDIO;
    msg[1] = pmt;
    msg[2] = vol;
    pipe_put(audio_pipe_audio, msg, MSG_LEN_AUDIO);
}

void audio_stop(void)
{
    wav_stop();
}

void audio_start(void *arg)
{
    struct msg_t *p_msg = (struct msg_t *)arg;
    audio_pipe_audio = p_msg->pipe_audio; 
    pthread_create(&audio_tid, NULL, audio_thread, arg);
	pthread_detach(audio_tid);
}
