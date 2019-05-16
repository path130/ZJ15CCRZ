#ifndef _MSG_H_
#define _MSG_H_

#include "public.h"

#define MSG_LEN_MAX        32//28
#define MSG_LEN_UI          4
#define MSG_LEN_KEY         4
#define MSG_LEN_NET         28
#define MSG_LEN_SPI         16
#define MSG_LEN_BUZZ        4
//#define MSG_LEN_UART        16
#define MSG_LEN_UART        64
#define MSG_LEN_PROC        50
#define MSG_LEN_AUDIO       4

#define MSG_FROM_UI         0x01
#define MSG_FROM_TIM        0x02
#define MSG_FROM_KEY        0x03
#define MSG_FROM_NET        0x04
#define MSG_FROM_SPI        0x05
#define MSG_FROM_BUZZ       0x06
#define MSG_FROM_UART       0x07
#define MSG_FROM_PROC       0x08
#define MSG_FROM_AUDIO      0x09
#define MSG_FROM_IGNORE     0xFF

struct msg_t {
    pipe_handle pipe_ui;
    pipe_handle pipe_net;
    pipe_handle pipe_key;
    pipe_handle pipe_spi;
    pipe_handle pipe_buzz;
    pipe_handle pipe_uart;
    pipe_handle pipe_proc;
    pipe_handle pipe_audio;
    pipe_handle pipe_finger;//by mkq finger
};

#endif
