#ifndef _AUTO_TEST_H_
#define _AUTO_TEST_H_

#include "public.h"

#if defined (__cplusplus)
extern "C" {
#endif


#define ATEST_RUN           1
#define ATEST_STOP          0
#define ATEST_PAUSE         2

#define AT_CNT_CALL         0
#define AT_CNT_ASK          1
#define AT_CNT_TALK         2
#define AT_CNT_APPRESTART   3
#define AT_CNT_SYSREBOOT    4

extern void atest_start(unsigned char *target);
extern void atest_pause(void);
extern void atest_stop(void);
extern int  atest_get_status(void);
extern void atest_cnt_inc(unsigned char at_id);
extern void atest_check_resume(void);
extern void atest_info_show(void);
extern void atest_key_put(pipe_handle pipe_ui);

#if defined (__cplusplus)
}
#endif
 
#endif

