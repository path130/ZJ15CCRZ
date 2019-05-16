#ifndef _KEY_H_
#define _KEY_H_

#if defined (__cplusplus)
extern "C" {
#endif
/*F1 -- 取消, F2 -- 开锁键, F3 -- OK, F4 -- 管理处*/
#define KEY_F1      'A'
#define KEY_F2      'B'
#define KEY_F3      'C'
#define KEY_F4      'D'
#define KEY_1       '1'
#define KEY_2       '2'
#define KEY_3       '3'
#define KEY_4       '4'
#define KEY_5       '5'
#define KEY_6       '6'
#define KEY_7       '7'
#define KEY_8       '8'
#define KEY_9       '9'
#define KEY_0       '0'
#define KEY_X       '*'
#define KEY_Y       '#'

#define KEY_MC      'E'
#define KEY_FC      'F'
#define KEY_UL      'G'

typedef enum {
    BUZZ_KEY,
    BUZZ_BEEP,
    BUZZ_CONFIRM,
    BUZZ_WARNING,
} BUZZ_TYPE;

extern void buzz_play(BUZZ_TYPE type);
extern void key_buzz(BUZZ_TYPE type);
extern int  key_start(void *arg);

extern int get_photo_sence(void);//by mkq


#if defined (__cplusplus)
}
#endif
 
#endif
