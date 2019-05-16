/*
 ============================================================================
 Name        : public.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 公共模块
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <ti/sdo/dmai/priv/_Buffer.h>
#include "public.h"

char debug_level   = 0;

#if 0
Fifo_Attrs   Fifo_default  = { 0 };
Pause_Attrs  Pause_default = { 0 };
Buffer_Attrs cmem_default  = {
    Buffer_Memory_Params_DEFAULT_DEFINE,
    Buffer_Type_BASIC,
    1,
    FALSE
};
#endif
inline int gbl_data_get(global_data *gbl)
{
    int ret;

    pthread_mutex_lock(&gbl->mutex);
    ret = gbl->data;
    pthread_mutex_unlock(&gbl->mutex);

    return ret;
}

inline void gbl_data_set(global_data *gbl, int data)
{
    pthread_mutex_lock(&gbl->mutex);
    gbl->data = data;
    pthread_mutex_unlock(&gbl->mutex);
}

void app_debug_init(void)
{
    if (getenv("APP_DEBUG") != NULL) {
        debug_level = getenv("APP_DEBUG")[0] - '0';
        if ((debug_level < 0) || (debug_level > 4))
            debug_level = 0;
    }
}


/******************************************************************************
 * pipe_create
 ******************************************************************************/
pipe_handle pipe_create(void)
{
    pipe_handle h_pipe;

    h_pipe = calloc(1, sizeof(pipe_object));

    if (h_pipe == NULL) {
        app_debug(DBG_FATAL, "Alloc failed: pipe object\n");
        return NULL;
    }

    if (pipe(h_pipe->pipes)) {
        free(h_pipe);
        return NULL;
    }

    return h_pipe;
}

/******************************************************************************
 * Fifo_delete
 ******************************************************************************/
int pipe_delete(pipe_handle h_pipe)
{
    int ret = 0;

    if (h_pipe) {
        if (close(h_pipe->pipes[0])) {
            ret = -1;
        }

        if (close(h_pipe->pipes[1])) {
            ret = -1;
        }
        free(h_pipe);
    }

    return ret;
}

/******************************************************************************
 * Fifo_get
 ******************************************************************************/
int pipe_get(pipe_handle h_pipe, void *data, int num)
{
    if (h_pipe == NULL) return -1;

    if (read(h_pipe->pipes[0], data, num) != num) {  
        return -1;
    }

    return 0;
}


/******************************************************************************
 * Fifo_put
 ******************************************************************************/
int pipe_put(pipe_handle h_pipe, void *data, int num)
{
    if (h_pipe == NULL) return -1;

    if (write(h_pipe->pipes[1], data, num) != num) {
        return -1;
    }

    return 0;
}


