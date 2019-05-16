/*
 ============================================================================
 Name        : auto_test.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 自动呼叫测试模块
  ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include "auto_test.h" // public.h
#include "text.h"
#include "osd.h"
#include "ui.h"
//#include "spi.h"  //del by wrm 20141121
#include "key.h"
#include "msg.h"

#define ATEST_FILE      "./atest.cfg"

#ifdef APP_AUTO_TEST

static struct atest_config_t {
    unsigned int  status;
    unsigned int  flag;
    unsigned int  count[5]; //0 呼叫次数 1 应答次数 2 通话次数 3 程序重启次数 4系统重启次数
    unsigned char target[ROOM_NUM];
} atest_cfg;

void atest_cfg_save(void)
{
    int fd = open(ATEST_FILE, O_WRONLY|O_TRUNC|O_CREAT, 0664);
    if (fd < 0) {
        app_debug(DBG_FATAL, "Cannot opn config file:%s for write\n", ATEST_FILE);
        return;
    }
    write(fd, &atest_cfg, sizeof(atest_cfg)); 
    fdatasync(fd);
    close(fd);
}

void atest_cfg_get(void)
{
    FILE *fp = fopen(ATEST_FILE, "r");
    if (fp == NULL) {
        atest_cfg.count[0] = 0;
        atest_cfg.count[1] = 0;
        atest_cfg.count[2] = 0;
        atest_cfg.count[3] = 0;
        atest_cfg.count[4] = 0;
        atest_cfg.status   = ATEST_STOP;
        atest_cfg.flag     = 0;
        atest_cfg_save();
    }
    else {
        fread(&atest_cfg, sizeof(atest_cfg), 1, fp); 
        fclose(fp);
    }
}

void atest_start(unsigned char *target)
{
    atest_cfg_get();
    if (target != NULL) {
        memcpy(atest_cfg.target, target, ROOM_NUM/2);
    }
    atest_cfg.status = ATEST_RUN;
    atest_cfg.flag   = 1;
    atest_cfg_save();
}

void atest_pause(void)
{
    atest_cfg.status = ATEST_PAUSE;
    atest_cfg.flag   = 0;
    atest_cfg_save();
}

void atest_stop(void)
{
    atest_cfg.status   = ATEST_STOP;
    atest_cfg.flag     = 0;
    atest_cfg.count[0] = 0;
    atest_cfg.count[1] = 0;
    atest_cfg.count[2] = 0;
    atest_cfg.count[3] = 0;
    atest_cfg.count[4] = 0;
    atest_cfg_save();
    remove(ATEST_FILE);
}

int atest_get_status(void)
{
    return atest_cfg.status;
}

void atest_cnt_inc(unsigned char at_id)
{
    if (atest_cfg.status != ATEST_RUN) return;
    if (at_id < 5)
        atest_cfg.count[at_id]++;
}

static int atest_resume = 0;
void atest_check_resume(void) 
{
    FILE *fp = fopen(ATEST_FILE, "r");
    if (fp != NULL) {
        fread(&atest_cfg, sizeof(atest_cfg), 1, fp); 
        fclose(fp);
        #if 0
        if (atest_cfg.status) {
            if (atest_cfg.flag) 
                atest_cfg.count[AT_CNT_APPRESTART]++;
            else
                atest_cfg.count[AT_CNT_SYSREBOOT]++;
            atest_resume = 1;
        }
        #else
        if (atest_cfg.status == ATEST_RUN) {
            atest_resume = 1;
        }
        if(atest_cfg.flag == 1){
            atest_cfg.count[AT_CNT_APPRESTART]++;
        }
        #endif
    }
    //atest_cfg.flag = 0;
}

void atest_info_show(void)
{
    if (atest_cfg.status != ATEST_RUN) return;
    char string[40] = {'\0'};

    sprintf(string, "呼叫次数\n%d", atest_cfg.count[AT_CNT_CALL]);
    text_fill_canvas(text_font, ENCODING, RGB_GREEN, 12, 12, 0, 0, 5, 35,  string);
    sprintf(string, "收到应答\n%d", atest_cfg.count[AT_CNT_ASK]);
    text_fill_canvas(text_font, ENCODING, RGB_GREEN, 12, 12, 0, 0, 5, 70,  string);
    sprintf(string, "通话次数\n%d", atest_cfg.count[AT_CNT_TALK]);
    text_fill_canvas(text_font, ENCODING, RGB_GREEN, 12, 12, 0, 0, 5, 105, string);
    sprintf(string, "应用重启\n%d", atest_cfg.count[AT_CNT_APPRESTART]);
    text_fill_canvas(text_font, ENCODING, RGB_GREEN, 12, 12, 0, 0, 5, 140, string);
    sprintf(string, "系统重启\n%d", atest_cfg.count[AT_CNT_SYSREBOOT]);
    text_fill_canvas(text_font, ENCODING, RGB_GREEN, 12, 12, 0, 0, 5, 175, string);
    atest_cfg_save();
}

void atest_key_put(pipe_handle pipe_ui)
{
    unsigned char msg[MSG_LEN_MAX];
    
    static unsigned int  test_delay = 0;
    static unsigned int  test_wait  = 0;
    
    if (atest_resume) {
        sleep(2);
        msg[0] = MSG_FROM_KEY;
        msg[1] = MSG_UI_KEY_IN;
        msg[2] = KEY_F1;
        msg[3] = 1; 
        pipe_put(pipe_ui, msg, MSG_LEN_UI);
        usleep(10000);
        msg[3] = 0; 
        pipe_put(pipe_ui, msg, MSG_LEN_UI);
        usleep(10000);
        atest_resume = 0;
    }
    if (atest_cfg.status != ATEST_RUN) test_wait = 40;
    else
    if (++test_wait > 150) { //df2100 220 jianhuaban 100
        int i;
        test_wait  = 0;
        test_delay = 0;
        ui_action(UI_SCENE_SWITCH, SCENE_CALL);
        msg[0] = MSG_FROM_KEY;
        msg[1] = MSG_UI_KEY_IN;

        for(i = 0; i < ROOM_NUM/2; i++) {
            msg[2] = '0' + ((atest_cfg.target[i] >> 4)&0x0f);
            msg[3] = 1;
            pipe_put(pipe_ui, msg, MSG_LEN_UI);
            usleep(10000);
            msg[3] = 0;
            pipe_put(pipe_ui, msg, MSG_LEN_UI);
            usleep(100000);
            msg[2] = '0' + (atest_cfg.target[i]&0x0f);
            msg[3] = 1;
            pipe_put(pipe_ui, msg, MSG_LEN_UI);
            usleep(10000);
            msg[3] = 0;
            pipe_put(pipe_ui, msg, MSG_LEN_UI);
            usleep(100000);
        }
#ifndef FRONT_DOOR
        usleep(150000);
#endif
#ifdef Test_Cloud        
        while (++test_delay < 55) //df2100 15 jianhuaban 8
            sleep(1);
#else

        while (++test_delay < 10) //df2100 15 jianhuaban 8 // 
            sleep(1);
#endif

        msg[2] = KEY_F1;
        msg[3] = 1; 
        pipe_put(pipe_ui, msg, MSG_LEN_UI);
        usleep(30000);
        msg[3] = 0; 
        pipe_put(pipe_ui, msg, MSG_LEN_UI);
        usleep(30000);
    }
}
#else
void signal_bind(void)
{

}

void atest_start(unsigned char *target)
{

}

void atest_pause(void)
{
}

void atest_stop(void)
{
}

int atest_get_status(void)
{
    return 0;
}

void atest_cnt_inc(unsigned char at_id)
{
}

void atest_check_resume(void)
{
}

void atest_info_show(void)
{
}

void atest_key_put(pipe_handle pipe_ui)
{
}

#endif

