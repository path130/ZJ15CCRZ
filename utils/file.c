/*
 ============================================================================
 Name        : file.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : ÎÄ¼þË÷Òý
  ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "public.h"
#include "file.h"

void file_get_name(const char *fpath, char *fname, char *fexpand)
{
    int  i = 0, pos_dot = -1, pos_slash = -1;
    int  fpath_len  = strlen(fpath);
    char *fpath_ptr = (char *)fpath;

    for (i = 0; i < fpath_len; i++) {
        if (*(fpath_ptr+i) == '/') {
            pos_slash = i;
            pos_dot   = -1;
        }
        else
        if (*(fpath_ptr+i) == '.')
            pos_dot   = i;
    }

    if (pos_slash > -1) {
        if (pos_dot > (pos_slash+1)) {
            memcpy(fname, fpath+pos_slash+1, pos_dot-pos_slash-1);
        }
    }
    if (pos_dot > -1) {
        if (pos_dot+1 < fpath_len) {
            memcpy(fexpand, fpath+pos_dot+1, fpath_len-pos_dot-1);
        }
    }
}

void file_list_in_dir(FILE_TPYE type, const char *dpath, FILE_ATTR fa[], int *p_cnt)
{  
    char fpath[FILE_PATH_LEN_MAX]   = {'\0'};
    char fname[FILE_PATH_LEN_MAX]   = {'\0'};
    char fexpand[FILE_PATH_LEN_MAX] = {'\0'};
    int  cnt = 0; 

    DIR           *dir = NULL;
	 struct dirent *ent = NULL;
    struct stat    st; 

    if ((dir=opendir(dpath)) == NULL)
        return;

    while ((ent=readdir(dir)) != NULL) {
        snprintf(fpath, FILE_PATH_LEN_MAX, "%s/%s", dpath, ent->d_name);
        if (lstat(fpath, &st) < 0)
		    return;
	     if (S_ISDIR(st.st_mode)) {
		    continue;
        }
        memset(fname,   '\0', FILE_PATH_LEN_MAX);
        memset(fexpand, '\0', FILE_PATH_LEN_MAX);
        file_get_name(fpath, fname, fexpand);
        //printf("fexpand:%s mtime:%ld \n", fexpand, (unsigned long)st.st_mtime);
        if (type == FILE_PIC) {
            if ((0==strcmp(fexpand, "png")) || (0==strcmp(fexpand, "PNG")) || \
                (0==strcmp(fexpand, "jpg")) || (0==strcmp(fexpand, "JPG")) || \
                (0==strcmp(fexpand, "bmp")) || (0==strcmp(fexpand, "BMP")) || \
                (0==strcmp(fexpand, "jpeg")) || (0==strcmp(fexpand, "JPEG"))) {
                    strcpy(fa[cnt].fpath, fpath);
                    strcpy(fa[cnt].fname, fname);
                    strcpy(fa[cnt].fexpand, fexpand);
                    fa[cnt].ftime = st.st_mtime;
                if (++cnt == FILE_LIST_CNT_MAX) break;
            }
        }
        else
        if (type == FILE_TXT) {
            if ((0==strcmp(fexpand, "txt")) || (0==strcmp(fexpand, "TXT")) || \
                (0==strcmp(fexpand, "text")) || (0==strcmp(fexpand, "TEXT"))) {
                    strcpy(fa[cnt].fpath, fpath);
                    strcpy(fa[cnt].fname, fname);
                    strcpy(fa[cnt].fexpand, fexpand);
                    fa[cnt].ftime = (unsigned long)st.st_mtime;
                if (++cnt == FILE_LIST_CNT_MAX) break;
            }
        }
#if 1
        int  i, j;
        FILE_ATTR fa_tmp;

        for (i = 0; i < cnt-1; i++) {
            if (fa[i].ftime < fa[i+1].ftime) {
                j = i + 1;
                while(j > 0 && fa[j].ftime > fa[j-1].ftime) {
                    //strcpy(fa_tmp.fpath,   fa[j].fpath);
                    //strcpy(fa_tmp.fname,   fa[j].fname);
                    //strcpy(fa_tmp.fexpand, fa[j].fexpand);
                    memcpy(&fa_tmp,  &fa[j],   sizeof(FILE_ATTR));
                    memcpy(&fa[j],   &fa[j-1], sizeof(FILE_ATTR));
                    memcpy(&fa[j-1], &fa_tmp,  sizeof(FILE_ATTR));
                    j--;
                }
            }
        }
#endif
    }
    closedir(dir);
    *p_cnt = cnt;
}



