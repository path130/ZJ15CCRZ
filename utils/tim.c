/*
 ============================================================================
 Name        : tim.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : timer
  ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "tim.h"
#include "public.h"

static pthread_t    tim_tid;
static struct tim_t_struct {
    volatile int    amount;
    volatile int    count;
    volatile int    used;
    int             period;
    int             p_arg1;
    int             p_arg2;
    tim_callback    p_fun;
    pthread_mutex_t mutex;
} tim_t[TIME_ID_MAX];

void *tim_thread(void *arg)
{
    void *status = THREAD_SUCCESS;
    int  id, fun_run;
    while(1) {
        usleep(250*1000 - 10);
        for (id = 1; id < TIME_ID_MAX; id++) {
            fun_run = 0;
            pthread_mutex_lock(&tim_t[id].mutex);
            if ((tim_t[id].used > 0) && (tim_t[id].count > 0)) {
                if (--tim_t[id].count == 0) {
                    fun_run = 1;
                    if (tim_t[id].period == TIME_DESTROY) {
                        tim_t[id].used   =  0;
                        tim_t[id].amount = -1;
                        tim_t[id].count  = -1;
                    }
                    else
                    if (tim_t[id].period == TIME_PERIODIC)
                        tim_t[id].count  = tim_t[id].amount;
                    
                    else
                        tim_t[id].count  = -1;
                }
            }
            pthread_mutex_unlock(&tim_t[id].mutex);

            if (fun_run) 
                tim_t[id].p_fun(tim_t[id].p_arg1, tim_t[id].p_arg2);
        }
    }
    exit(1);
    return status;
}

int tim_start(void *arg)
{
    int id;
    memset(tim_t, 0, sizeof(tim_t));
    for (id = 0; id < TIME_ID_MAX; id++) {
        pthread_mutex_init(&tim_t[id].mutex, NULL);
    }
    pthread_create(&tim_tid, NULL, tim_thread, NULL);
    return 0;
}

int tim_set_event(int time_count, tim_callback time_fun, int fun_arg1, int fun_arg2, int period)
{
    int id;
    #if 0
    for (id = 1; id < TIME_ID_MAX; id++) {
        pthread_mutex_lock(&tim_t[id].mutex);
        if (tim_t[id].used != 0)  {
            pthread_mutex_unlock(&tim_t[id].mutex);
            continue;
        } else {
            tim_t[id].used   = 1;
            tim_t[id].amount = time_count;
            tim_t[id].count  = time_count;
            tim_t[id].period = period;
            tim_t[id].p_fun  = time_fun;
            tim_t[id].p_arg1 = fun_arg1;
            tim_t[id].p_arg2 = fun_arg2;
        }
        pthread_mutex_unlock(&tim_t[id].mutex);
        printf("time_id:%d\n", id);
        return id;
    }
    #endif
    for (id = 1; id < TIME_ID_MAX; id++) {
        pthread_mutex_lock(&tim_t[id].mutex);
        if (tim_t[id].used == 0)  {
            tim_t[id].used   = 1;
            tim_t[id].amount = time_count;
            tim_t[id].count  = time_count;
            tim_t[id].period = period;
            tim_t[id].p_fun  = time_fun;
            tim_t[id].p_arg1 = fun_arg1;
            tim_t[id].p_arg2 = fun_arg2;
            pthread_mutex_unlock(&tim_t[id].mutex);
            //printf("time_id:%d\n", id);
            return id;
        }
        pthread_mutex_unlock(&tim_t[id].mutex);
    }

    app_debug(DBG_FATAL, "no free tim_id, the TIME_ID_MAX is %d\n", TIME_ID_MAX);
    return 0;
}

void tim_reset_event(int id)
{
    if ((id < TIME_ID_MAX) && (tim_t[id].used)) {
        pthread_mutex_lock(&tim_t[id].mutex);
        tim_t[id].count = tim_t[id].amount;
        pthread_mutex_unlock(&tim_t[id].mutex);
    }
}

int tim_get_time(int id)
{
    int ret = -1;
    if ((id < TIME_ID_MAX) && (tim_t[id].used)) {
        pthread_mutex_lock(&tim_t[id].mutex);
        ret = tim_t[id].count;
        pthread_mutex_unlock(&tim_t[id].mutex);
    }
    return ret;
}

void tim_suspend_event(int id)
{
    if ((id < TIME_ID_MAX) && (tim_t[id].used)) {
        pthread_mutex_lock(&tim_t[id].mutex);
        tim_t[id].count  = -1;
        pthread_mutex_unlock(&tim_t[id].mutex);
    }
}

void tim_reset_time(int id, int time)
{
    if ((id < TIME_ID_MAX) && (tim_t[id].used)) {
        pthread_mutex_lock(&tim_t[id].mutex);
        tim_t[id].amount = time;
        tim_t[id].count  = time;
        pthread_mutex_unlock(&tim_t[id].mutex);
    }
}

void tim_kill_event(int id)
{
    if ((id < TIME_ID_MAX) && (tim_t[id].used)) {
	//printf("kill event time_id:%d\n", id);
        pthread_mutex_lock(&tim_t[id].mutex);
        tim_t[id].used   =  0;
        tim_t[id].amount = -1;
        tim_t[id].count  = -1;
        tim_t[id].p_fun  = NULL; 
        pthread_mutex_unlock(&tim_t[id].mutex);
    }
}

