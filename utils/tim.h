#ifndef _TIM_H_
#define _TIM_H_

#if defined (__cplusplus)
extern "C" {
#endif      

#define TIME_ID_MAX         32

#define TIME_ONESHOT        0   //执行一次
#define TIME_DESTROY        1   //执行一次,结束时销毁TIME_ID
#define TIME_PERIODIC       2   //循环执行

#define TIME_250MS(x)       (x)
#define TIME_500MS(x)       ((x)*2)
#define TIME_1S(x)          ((x)*4)
#define TIME_1MIN(x)        ((x)*4*60)

typedef void (*tim_callback)(int arg1, int arg2);
extern int  tim_start(void *arg);
extern int  tim_set_event(int time_count, tim_callback time_fun, int fun_arg1, int fun_arg2, int period);
extern int  tim_get_time(int id);
extern void tim_reset_event(int id);
extern void tim_suspend_event(int id);
extern void tim_reset_time(int id, int time);
extern void tim_kill_event(int id);

#if defined (__cplusplus)
}
#endif
 
#endif
