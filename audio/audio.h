#ifndef _AUDIO_H_
#define _AUDIO_H_

#if defined (__cplusplus)
extern "C" {
#endif      

#define VOL_DEVCFG  0xFF

typedef enum {
    PMT_NONE = 0x00,
    PMT_TEST,
    PMT_WELCOME,
    PMT_ECHORING,
    PMT_UNLOCK,
    PMT_COME_IN,
    PMT_CALL_USER,
    PMT_NO_ANSWER,
    PMT_ALARM                               //by mkq 20170907

} prompt_e;

#define WAV_WELCOME     "./wav/welcome.wav"
#define WAV_ECHORING    "./wav/echoring.wav"
#define WAV_UNLOCK      "./wav/unlock.wav"
#define WAV_COME_IN     "./wav/comein.wav"
#define WAV_CALL_USER   "./wav/calluser.wav"
#define WAV_NO_ANSWER   "./wav/noanswer.wav"
#define WAV_ALARM       "./wav/alarm.wav"   //by mkq 20170907


extern void audio_start(void *arg);
extern void audio_play(prompt_e pmt, int vol);
extern void audio_stop(void);

#if defined (__cplusplus)
}
#endif
 
#endif

