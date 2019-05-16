#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include <pthread.h>
//#include <ti/sdo/dmai/Dmai.h>
//#include <ti/sdo/dmai/Fifo.h>
//#include <ti/sdo/dmai/Pause.h>
//#include <ti/sdo/dmai/Buffer.h>

#if defined (__cplusplus)
extern "C" {
#endif

/* Comment next line if we don't need any debug message or it's a release version.*/
#define APP_DEBUG          

#define APP_AUTO_TEST
#define RecallTime
#define ADD_UNLOCK_AD
//#define Test_Cloud
//#define FRONT_DOOR
#define _FACE_DET   //不用刷脸功能时请屏蔽掉
#define _KEEP_OPEN  //不用刷脸功能时请屏蔽掉
#define _PHOTO_UPLOAD
#define _FP_MODULE
/* app version */
#define APP_VERSION         "1.0.1"

/* .c .h file's encoding */
#define T_ASCII             0
#define T_UTF8              1
#define ENCODING            T_UTF8

//#define EMC_TEST		1

/* Function error codes */
#ifndef SUCCESS
#define SUCCESS             0
#endif

#ifndef FAILURE
#define FAILURE             -1
#endif

/* Thread error codes */
#ifndef THREAD_SUCCESS
#define THREAD_SUCCESS      (void *) 0
#endif

#ifndef THREAD_FAILURE
#define THREAD_FAILURE      (void *) -1
#endif

/* Common marcro defineition */
#define MIN(a,b)            (((a)<(b)) ? (a) : (b))
#define MAX(a,b)            (((a)>(b)) ? (a) : (b))
#define CLEAR(x)	         memset (&(x), 0, sizeof(x))
#define ERR(fmt, args...)   fprintf(stderr, "Error: " fmt, ## args)


/* Cleans up cleanly after a failure */
#define cleanup(x)          \
    status = (x);           \
    goto cleanup

#if 0
/* dmai init */
#define dmai_init           Dmai_init
#define ce_runtime_init     CERuntime_init

/*Cmem buffer Multithreads synchronization & communicate*/
extern  Buffer_Attrs        cmem_default;
extern  Fifo_Attrs          Fifo_default;
extern  Pause_Attrs         Pause_default; 

#define cmem_handle         Buffer_Handle
#define cmem_create(x)      Buffer_create(x, &cmem_default);
#define cmem_delete(x)      Buffer_delete(x)
#define cmem_userptr(x)     Buffer_getUserPtr(x)

#define fifo_handle         Fifo_Handle
#define fifo_create()       Fifo_create(&Fifo_default)
#define fifo_delete         Fifo_delete
#define fifo_put            Fifo_put
#define fifo_get            Fifo_get
#define fifo_entries        Fifo_getNumEntries
#define fifo_flush          Fifo_flush

#define pause_handle        Pause_Handle
#define pause_create()      Pause_create(&Pause_default)
#define pause_delete        Pause_delete	
#define pause_test          Pause_test
#define pause_on            Pause_on
#define pause_off           Pause_off
#endif
typedef struct pipe_object_struct {
    pthread_mutex_t mutex;
    int             pipes[2];
} pipe_object, *pipe_handle;

extern pipe_handle pipe_create(void);
extern int  pipe_delete(pipe_handle h_pipe);
extern int  pipe_get(pipe_handle h_pipe, void *data, int num);
extern int  pipe_put(pipe_handle h_pipe, void *data, int num);


/* App debug marcro & init function */
#define DBG_FATAL           1
#define DBG_ERROR           2
#define DBG_WARNING         3
#define DBG_INFO            4

#ifdef  APP_DEBUG 
#define app_debug(level, fmt, args...)  \
    if(debug_level >= level) {          \
        if (level == DBG_INFO)          \
            fprintf(stderr, ""fmt"", ## args); \
        else                            \
        fprintf(stderr, "%s@[%s_%d]: "fmt"", ""#level"", __FILE__, __LINE__, ## args); \
    }
#else
#define app_debug(level, fmt, args...)
#endif

    
extern char debug_level;
extern void app_debug_init(void);

/* Global data in multithreads */
#define GBL_DATA_INIT { 0, PTHREAD_MUTEX_INITIALIZER }
typedef struct global_data_struct {
    volatile int    data;
    pthread_mutex_t mutex;
} global_data;

extern inline int   gbl_data_get(global_data *gbl);
extern inline void  gbl_data_set(global_data *gbl, int data);

#if defined (__cplusplus)
}
#endif
 
#endif

