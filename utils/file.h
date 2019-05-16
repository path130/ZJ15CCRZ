#ifndef _FILE_H_
#define _FILE_H_

#include <time.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define FILE_PATH_LEN_MAX       80
#define FILE_LIST_CNT_MAX       64

typedef enum {
    FILE_PIC,
    FILE_TXT,
} FILE_TPYE;

typedef struct {
    char    fpath[FILE_PATH_LEN_MAX];
    char    fname[FILE_PATH_LEN_MAX];
    char    fexpand[FILE_PATH_LEN_MAX];
    time_t  ftime;
}FILE_ATTR;

extern void file_get_name(const char *fpath, char *fname, char *fexpand);
extern void file_list_in_dir(FILE_TPYE type, const char *dpath, FILE_ATTR fa[], int *p_cnt);

#if defined (__cplusplus)
}
#endif
 
#endif
