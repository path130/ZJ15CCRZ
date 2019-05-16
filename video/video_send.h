#ifndef _VIDEO_SEND_H_
#define _VIDEO_SEND_H_

#if defined (__cplusplus)
extern "C" {
#endif

// #include <ApproDrvMsg.h>
// #include <Appro_interface.h>
// #include <Msg_Def.h>

#include "ajb_rtpc.h"

#define RTP_MAXDEST 4

#define RTP_VIDEO_MULTIDEST

#define RTP_PACKET_SIZE     1024

#define VID_SND_RTP         0
#define VID_SND_UDP         1
#define VID_SND_TCP         2

#define VID_RTP_PACK_MULTI  0
#define VID_RTP_PACK_ONE    1

#define VID_WSVGA_W         1024
#define VID_WSVGA_H         600
#define VID_PAL_W           704
#define VID_PAL_H           576
#define VID_VGA_W           640
#define VID_VGA_H           480
#define VID_QVGA_W          320
#define VID_QVGA_H          240
#define VID_CIF_W           352
#define VID_CIF_H           288

#define VID_MAIN_LCD        0
#define VID_MAIN_CVBS       1

#define H264_BIT_RATE       2000000
#define ROUNDDOWN(x)        ((x & 0x1f) ?(x & ~(0x1f)) : x)

#define VID_MODE_DAY        0
#define VID_MODE_NIGHT      1

enum vid_type{
    VID_H264,
    VID_JPEG,
    VID_MOBILE,
    VID_H264_926,
    VID_SERVER,
};

enum vid_res {
    VID_PAL,
    VID_VGA,
    VID_QVGA,
    VID_DF2100,
    VID_CIF,
    VID_CIF5,
    VID_WSVGA,
    VID_720P,
};

extern int video_send_init(void);
#ifdef RTP_VIDEO_MULTIDEST
extern int video_send_start(unsigned long lip[], enum vid_type vid_t, enum vid_res vid_r);
extern int video_del_rtpdest(unsigned long lip[]);
extern int video_del_rtpdest_one(unsigned long lip);
extern int video_add_rtpdest_one(unsigned long lip);
extern int video_single_rtpdest(unsigned long lip);
#else
extern int video_send_start(const char *dest_ip, enum vid_type vid_t, enum vid_res vid_r);
#endif
extern int video_send_stop(void);
extern void proc_signal(int signo);
extern int video_day_night_set(void);
extern void video_day_night_clr(void);
extern void video_main_mode_set(int mode);
#if defined (__cplusplus)
}
#endif

#endif

