/*
 ============================================================================
 Name        : ui.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 15A UI
  ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "public.h"
#include "ajb_bus.h"
#include "global_def.h"
#include "dev_config.h"

#include "ui.h"
#include "osd.h"
#include "img.h"
#include "msg.h"
//#include "spi.h" //del by wrm 20141121
#include "key.h"
#include "tim.h"
#include "text.h"
#include "uart.h"
#include "edit.h"
#include "list.h"
#include "label.h"
#include "radio.h"
#include "button.h"
#include "slider.h"
#include "catalog.h"
#include "cpld.h"
#include "audio.h"
//#include "voice.h"
#include "auto_test.h"
#include "video_send.h"
#include "info_container.h"
#include "sqlite_data.h"
#include "dpgpio.h"
//#include "dpdevice.h"          //by mkq 20170907

#include "face.h"

#include "finger.h"	//by mkq finger
#include "json_msg.h"//add for photo upload


#ifndef IOCTRL_SET_IRQ_MODE
#define IOCTRL_SET_IRQ_MODE                     0x4c434b09//add by mkq 20170906
#endif

static pipe_handle     ui_pipe_ui;
static pipe_handle     ui_pipe_proc;
static pthread_t       ui_tid;
static UI_OP_MODE      ui_oper_mode;
static int lift_ctrl_no; //add for multi_lift_ctl

unsigned char  is_call_kzq = 0;
unsigned char  target[ROOM_NUM];
unsigned char  target_night[ROOM_NUM];  //夜间模式

extern need_opencheck; //by mkq 20170907
extern net_send_to_server(unsigned int, void *, int);    //by mkq for songjian 20170907

#define SET_SEL_NUM     6

static button_handle   bt_bright_inc, bt_bright_dec, bt_volumes_inc, bt_volumes_dec;
static button_handle   bt_set_sel[SET_SEL_NUM];
static edit_handle     edit_norm, edit_pw_old, edit_pw_new, edit_pw_firm, edit_date, edit_time, edit_ip_addr, edit_ip_gate, edit_ip_mask;
static list_handle     list_msg, list_pic,list_unlock_ad;
static catalog_handle contacts;
static radio_handle    radio_min_0, radio_min_1, radio_min_5;
static slider_handle   slider_bright, slider_volumes;
static label_object    label_count_down = {
    .w          = 80,   //COUNT_DOWN_W,
    .h          = 50,   //COUNT_DOWN_H,
    .x          = 177+3,  //COUNT_DOWN_X,
    .y          = 76-4,   //COUNT_DOWN_Y, 
    .txt_w      = 30, 
    .txt_h      = 30,
    .rgb        = NULL,
};

static label_object    label_count_down1 = { //add by mkq 20170907
    .w          = 120,   //COUNT_DOWN_W,
    .h          = 50,   //COUNT_DOWN_H,
    .x          = 166,  //COUNT_DOWN_X,
    .y          = 65,   //COUNT_DOWN_Y, 
    .txt_w      = 46, 
    .txt_h      = 46,
    .rgb        = NULL,
};

static label_object    label_prompt = {
    .w          = 160,
    .h          = 26,
    .x          = 100,
    .y          = 145, 
    .txt_w      = 16, 
    .txt_h      = 16,
    .rgb        = NULL,
};
static label_object    label_unlock = {
    .w          = 180,
    .h          = 30,
    .x          = 100,
     .y          = 114, 
    .txt_w      = 16,
    .txt_h      = 16,
    .rgb        = NULL,
};

static label_object    label_cloud_show = {
    .w          = 180,
    .h          = 30,
    .x          =83,       
     .y          = 114,
    .txt_w      = 16,
    .txt_h      = 16,
    .rgb        = NULL,
};
static label_object    label_calling_promot = {
    .w          = 180,
    .h          = 26,
    .x          = 75,
    .y          = 145, 
    .txt_w      = 16, 
    .txt_h      = 16,
    .rgb        = NULL,
};
static label_object     label_talking_no_unlock = {//by mkq finger
    .w          = 211, 
    .h          = 38,
    .x          = 90+6,
    .y          = 180, 
    .txt_w      = 18, 
    .txt_h      = 18,
    .rgb        = NULL,
};


static label_object    label_noanswer = {
    .w          = 220,
    .h          = 26,
    .x          = 65,
    .y          = 180, 
    .txt_w      = 20, 
    .txt_h      = 20,
    .rgb        = NULL,
};

static label_object     label_talking_unlock = {
    .w          = 211, 
    .h          = 38,
    .x          = 55,
    .y          = 120+2, 
    .txt_w      = 25, 
    .txt_h      = 25,
    .rgb        = NULL,
};

static label_object     label_noanswer_unlock = {
    .w          = 211, 
    .h          = 38,
    .x          = 55,
    .y          = 120+10, 
    .txt_w      = 25, 
    .txt_h      = 25,
    .rgb        = NULL,
};

static label_object     label_leave_unlock = {
    .w          = 211, 
    .h          = 38,
    .x          = 55,
    .y          = 120+50, //45
    .txt_w      = 25, 
    .txt_h      = 25,
    .rgb        = NULL,
};
static label_object     label_noanswer_no_unlock = {//by mkq finger
    .w          = 160,
    .h          = 26,
    .x          = 100,
    .y          = 145, 
    .txt_w      = 16, 
    .txt_h      = 16,
    .rgb        = NULL,
};


static const int scene_set_sel[SET_SEL_NUM] = {
    SCENE_SET_PW,   SCENE_SET_SCREEN, SCENE_SET_VOLUMN, 
    SCENE_SET_TIME, SCENE_SET_IP,     SCENE_SET_ABOUT,
};

static unsigned char  focus_sel_idx = 0;
static unsigned char  focus_screen_idx = 0;

static int      scene_pre;
static int      scene_cur;
static int      tim_count_down;
static int      tim_scene_timeout;
static int      tim_refresh_stbar,tim_face_det_timeout;
//static int      tim_cardlight_timeout;
static int      lcd_on = 0;
static char     key_code;
static char     key_status;
static char code[16];
static int catalog_flag = 0;
static int body_sense_result=0,face_det_flag=0; 
  int sb_pw=0;//add by lmx  用于判断是按*键还是0000进入管理密码界面
char g_usr_pwd[10]={0}; //add for photo upload 

static void ui_proc_input_pw(void);//add by wrm 20141121

static void ui_scene_sw(int scene);

extern int ui_exit_face_det(void);

//add for photo upload
int get_body_sense_result(void)
{
    return body_sense_result;
}

int get_cur_scene(void)
{
    return scene_cur;
}

inline  void ui_lcd_on(void)
{
    if (!lcd_on) {
#if 0    
        cpld_io_clr(IO36);
	cpld_io_set(EIO_LCD_AVDD); //add by wrm 20141124 for lcd on
        cpld_io_clr(EIO_LCD_DVDD);
        usleep(20*1000);
        cpld_io_set(IO36);
        usleep(200*1000);
	cpld_lcd_init(); //add by wrm 20141231 for lcd reverse
	usleep(20*1000);//this sleep for reverse configuration sucess
        cpld_io_set(EIO_LCD_BACKLIGHT);
#else  
        Set_Back_Light(1);      
#endif 
        lcd_on = 1;
    }
}

inline static void ui_lcd_off(void)
{
#if 0
    cpld_io_clr(EIO_LCD_BACKLIGHT);
    usleep(100*1000);
    cpld_io_clr(EIO_LCD_AVDD);//add by wrm 20141124 for lcd off
    cpld_io_set(EIO_LCD_DVDD);
#else    
        Set_Back_Light(0);
#endif  
    lcd_on = 0;
}
global_data gbl_cardlight = GBL_DATA_INIT;

void ui_cardlight(int arg1,int arg2)//add by wrm 20150302 for card light
{
	int card_light_on = gbl_data_get(&gbl_cardlight);
	printf("card light_on is %d\n",card_light_on);
	if(card_light_on){
	    Set_CardLight(0);//cpld_io_clr(EIO_KEYPAD_LIGHT);
	    gbl_data_set(&gbl_cardlight,0);
	}
	else{
	    Set_CardLight(1);//cpld_io_set(EIO_KEYPAD_LIGHT);
	    gbl_data_set(&gbl_cardlight,1);	   
	}
}
void needopen_fun(int ar1,int ar2)   //by mkq for songjian 
{
                printf("enter the callback of needopen_fun  PLAY+++++\n");
                unsigned char buf[10]={0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x9B,0x00,0x00};
                buf[1] = dev_cfg.my_code[0];
                buf[2] = dev_cfg.my_code[1];
                buf[4] = dev_cfg.my_code[3];
                buf[9] = buf[0]+buf[1]+buf[2]+buf[4]+buf[7];             
		if(need_opencheck==1)
		  {
                    audio_play(PMT_ALARM, DEV_VOL_PLAY);
                    net_send_to_server(0x518, buf, 10); 
                    
                   }


}
//电控锁和磁力锁的区别，默认电控锁，常态为低电平，高电平开锁，保持时间2-3秒
//当设置为磁力锁时，常态为高电平，低电平开锁，保持时间等于开锁延迟时间
//act为0时相当于设置锁开关
void ui_unlock(int act)
{
    static int lock_status = 0;

    if (act) {
    	ui_cardlight(0,0);		
        if (lock_status == DOOR_OPEN) return;
		    need_opencheck=1;                                            //by mkq 20170907

        if (dev_cfg.lock_type == 0) {
            //cpld_io_set(EIO_LOCK_SWITCH);
            Set_lock(1);
            tim_set_event(TIME_1S(1), (tim_callback)ui_action, UI_UNLOCK_RELEASE, 0, TIME_DESTROY);
		    //tim_set_event(TIME_1S(5), (tim_callback)needopen_fun, 0, 0, TIME_DESTROY);   //by mkq 20170907

        }
        else {
            //cpld_io_clr(EIO_LOCK_SWITCH); 
            Set_lock(0);
            tim_set_event(TIME_1S(dev_cfg.lock_time), (tim_callback)ui_action, UI_UNLOCK_RELEASE, 0, TIME_DESTROY);
		    //tim_set_event(TIME_1S(5), (tim_callback)needopen_fun, 0, 0, TIME_DESTROY);   //by mkq 20170907

        }
        lock_status = DOOR_OPEN;
    }
    else {
        if (dev_cfg.lock_type == 0){
            //cpld_io_clr(EIO_LOCK_SWITCH);
            Set_lock(0);
        }
        else{  				
            //cpld_io_set(EIO_LOCK_SWITCH);
            Set_lock(1);
        }    
        lock_status = DOOR_CLOSE;
    }
}
/* //del by wrm 20141121 for spi
void ui_unlock(int act)
{
    spi_unlock(dev_cfg.lock_type, act, dev_cfg.lock_time);
}
*/
static void ui_widget_create(void)
{
    bt_set_sel[0]  = BT_CREATE(PB_UL_PW);
    bt_set_sel[1]  = BT_CREATE(SCREEN);
    bt_set_sel[2]  = BT_CREATE(VOLUME);
    bt_set_sel[3]  = BT_CREATE(TIME);
    bt_set_sel[4]  = BT_CREATE(IP);
    bt_set_sel[5]  = BT_CREATE(ABOUT);
    bt_bright_inc  = BT_CREATE(BRIGHT_INC);
    bt_bright_dec  = BT_CREATE(BRIGHT_DEC);
    bt_volumes_inc = BT_CREATE(VOLUMES_INC);
    bt_volumes_dec = BT_CREATE(VOLUMES_DEC);

    edit_norm      = edit_create_ex(text_font, EDIT_IMG_NORM,  EDIT_NORM_X, EDIT_NORM_Y,    ES_NUM|ES_LM_C|ES_FT_L);
    edit_pw_old    = edit_create_ex(text_font, EDIT_IMG_SMALL, EDIT_PW_X,   EDIT_PW_OLD_Y,  ES_CUR|ES_PW|ES_LM_6|ES_FT_N);
    edit_pw_new    = edit_create_ex(text_font, EDIT_IMG_SMALL, EDIT_PW_X,   EDIT_PW_NEW_Y,  ES_CUR|ES_PW|ES_LM_6|ES_FT_N);
    edit_pw_firm   = edit_create_ex(text_font, EDIT_IMG_SMALL, EDIT_PW_X,   EDIT_PW_FIRM_Y, ES_CUR|ES_PW|ES_LM_6|ES_FT_N);
    edit_date      = edit_create_ex(text_font, EDIT_IMG_DATE, EDIT_DATE_X, EDIT_DATE_Y,    ES_DATE|ES_FT_N);
    edit_time      = edit_create_ex(text_font, EDIT_IMG_TIME, EDIT_TIME_X, EDIT_TIME_Y,    ES_TIME|ES_FT_N);
    edit_ip_addr   = edit_create_ex(text_font, EDIT_IMG_IP, EDIT_IP_X,   EDIT_IP_ADDR_Y, ES_IP|ES_FT_N);
    edit_ip_gate   = edit_create_ex(text_font, EDIT_IMG_GATE, EDIT_IP_X,   EDIT_IP_GATE_Y, ES_IP|ES_FT_N);
    edit_ip_mask   = edit_create_ex(text_font, EDIT_IMG_MASK, EDIT_IP_X,   EDIT_IP_MASK_Y, ES_IP|ES_FT_N);
    contacts       = catalog_create_ex(text_font, CATALOG_W, CATALOG_H, CATALOG_X, CATALOG_Y);

    slider_bright  = slider_create_ex(SLIDER_IMG_BG, SLIDER_IMG_FG, SLIDER_BRIGHT_X, SLIDER_BRIGHT_Y, 15, 1);
    slider_volumes = slider_create_ex(SLIDER_IMG_BG, SLIDER_IMG_FG, SLIDER_VOLUMES_X, SLIDER_VOLUMES_Y, 15, 1);

    radio_min_0    = radio_create_ex(RADIO_IMG_T, RADIO_IMG_F, RADIO_IMG_F_T, RADIO_IMG_F_F, RADIO_ALWAY_X, RADIO_ALWAY_Y);
    radio_min_1    = radio_create_ex(RADIO_IMG_T, RADIO_IMG_F, RADIO_IMG_F_T, RADIO_IMG_F_F, RADIO_MIN_1_X, RADIO_MIN_1_Y);
    radio_min_5    = radio_create_ex(RADIO_IMG_T, RADIO_IMG_F, RADIO_IMG_F_T, RADIO_IMG_F_F, RADIO_MIN_5_X, RADIO_MIN_5_Y);
//没用到以下两张图片
    list_msg       = list_create_ex(text_font, IMG_MSG_LIST, IMG_MSG_LIST_X, IMG_MSG_LIST_Y, LS_TXT);
    list_pic       = list_create_ex(text_font, IMG_PIC_LIST, IMG_PIC_LIST_X, IMG_PIC_LIST_Y, LS_PIC);
    list_unlock_ad       = list_create_ex(text_font, IMG_PIC_LIST, IMG_PIC_LIST_X, IMG_PIC_LIST_Y, LS_UNLOCK_AD);
}


static int          count_down_time = 60;
//static int          count_down_time = 3600;// 60; add by mkq 20170907
static int         calling_fp_unlock_faild_time = 0;//by mkq finger
static int          talking_unlock_time = 0;

static int		try_call_time = 0; 
static int		calling_unlock_time = 0; 
static int 		try_call_ser= 0;
static int 		is_calling_unlock = 0;	//add by wrm 20150119 for calling unlock

static unsigned int main_rgb[UI_FULL_W*UI_FULL_H];
static unsigned int comm_top_rgb[UI_FULL_W*UI_TOP_H];
static unsigned int comm_bot_rgb[UI_FULL_W*UI_BOT_H];

int ui_make_background(const char *pic)
{
    //if (scene_cur == SCENE_UNLOCK)  return -1;
    unsigned int background_rgb[UI_FULL_W*UI_FULL_H];
    
    if ( 0 != img_zoom(pic, background_rgb, UI_FULL_W, UI_FULL_H)) {
        pic = "./pic/BG.png";
    }
    if ( 0 != img_zoom(pic, background_rgb, UI_FULL_W, UI_FULL_H)) {
        return -1;
    }
    osd_put_canvas(background_rgb, UI_FULL_W, UI_FULL_H, 0, 0);
   
    //img_fill_canvas_ex(IMG_TOP_BANNER, IMG_TOP_BANNER_X, IMG_TOP_BANNER_Y);//don't need
    //img_fill_canvas_ex(IMG_TOP_WELCOME, IMG_TOP_WELCOME_X, IMG_TOP_WELCOME_Y);//欢迎文字在底部
    img_fill_canvas_ex(IMG_BOT_WELCOME, IMG_BOT_WELCOME_X, IMG_BOT_WELCOME_Y);
	
	
    osd_get_canvas(comm_top_rgb, UI_FULL_W, UI_TOP_H, 0, 0);
    osd_get_canvas(main_rgb, UI_FULL_W, UI_FULL_H, 0, 0);
    osd_get_canvas(comm_bot_rgb, UI_FULL_W, UI_BOT_H, 0, UI_TOP_H);

   // osd_put_canvas(main_rgb, UI_FULL_W, UI_FULL_H, 0, 0);//del by wrm 20150114 for don't needed

#ifdef FRONT_DOOR
img_fill_canvas_ex(IMG_FD_MAIN_TOP, IMG_MAIN_TOP_X, IMG_MAIN_TOP_Y);
    img_fill_canvas_ex(IMG_MAIN_LABEL1, IMG_MAIN_LABEL1_X, IMG_MAIN_LABEL1_Y);
    text_fill_canvas(text_font, ENCODING, RGB_WHITE, 17, 18, 0, 0, 60, 52,  "请输入楼栋加房号呼叫住户分机");
    text_fill_canvas(text_font, ENCODING, RGB_WHITE, 10, 11, 0, 0, 60, 52+19, "Please input floor + flat numbers to call interphone");
#else
   // text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, 72, 62+2,    "呼叫住户分机输入四位房号");
  //  text_fill_canvas(text_font, ENCODING, RGB_WHITE, 10, 11, 0, 0, 72, 62+19+2, "Please input four numbers to call interphone");
   img_fill_canvas_ex(IMG_MAIN_TOP, IMG_MAIN_TOP_X, IMG_MAIN_TOP_Y);
#endif
    img_fill_canvas_ex(IMG_MAIN_MID, IMG_MAIN_MID_X, IMG_MAIN_MID_Y);
    img_fill_canvas_ex(IMG_MAIN_BOT, IMG_MAIN_BOT_X, IMG_MAIN_BOT_Y);
    //img_fill_canvas_ex(IMG_MAIN_LABEL1, IMG_MAIN_LABEL1_X, IMG_MAIN_LABEL1_Y);
   // img_fill_canvas_ex(IMG_MAIN_LABEL2, IMG_MAIN_LABEL2_X, IMG_MAIN_LABEL2_Y);
   // img_fill_canvas_ex(IMG_MAIN_LABEL3, IMG_MAIN_LABEL3_X, IMG_MAIN_LABEL3_Y);
  //  text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, 72, 107+2,    "输入密码开锁请按开锁键");
  //  text_fill_canvas(text_font, ENCODING, RGB_WHITE, 10, 11, 0, 0, 72, 107+19+2, "Please press unlock key to input passwords to unlock ");
  //  text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, 72, 152+2,    "呼叫管理处请按管理处键");
  //  text_fill_canvas(text_font, ENCODING, RGB_WHITE, 10, 11, 0, 0, 72, 152+19+2, "Please press management key to call management "); 
     
    osd_get_canvas(main_rgb, UI_FULL_W, UI_FULL_H, 0, 0);

    strcpy(dev_cfg.bg_pic, pic);
    dev_config_save();
    return 0;
}

static void ui_fill_background(int bg)
{
    if(bg == BG_MAIN) {
        osd_put_canvas(main_rgb, UI_FULL_W, UI_FULL_H, 0, 0);
    }
    else {
        if (bg & BG_TOP) {
            osd_put_canvas(comm_top_rgb, UI_FULL_W, UI_TOP_H, 0, 0);
        }
        if (bg & BG_BOT) {
            osd_put_canvas(comm_bot_rgb, UI_FULL_W, UI_BOT_H, 0, UI_TOP_H);
        }
    }
}

static unsigned int statusbar_bg_rgb[OSD_STBAR_W*OSD_STBAR_H];
static unsigned int statusbar_bg_get_en = 1;
static unsigned int net_link_status = NET_LINK_ON;

static void statusbar_get_canvas(void)
{
    if (scene_cur == SCENE_PIC_DETAIL||scene_cur == SCENE_UNLOCK||scene_cur == SCENE_FINGER_UNLOCK)  return;//by mkq finger
    if (statusbar_bg_get_en == 0)       return;
    osd_get_canvas(statusbar_bg_rgb, OSD_STBAR_W, OSD_STBAR_H, OSD_STBAR_X, OSD_STBAR_Y);
    statusbar_bg_get_en = 0;
}

static int statusbar_fill_canvas(void)
{
    if (scene_cur == SCENE_PIC_DETAIL ||scene_cur == SCENE_UNLOCK||scene_cur == SCENE_FINGER_UNLOCK)  return -1;//by mkq finger
#ifndef TIME_SHOW_SEC
    static int tm_min_prev = -1;
#endif
    static int net_st_prev = -1;
    time_t now;
    struct tm *tnow;
    char time_date[20] = {'\0'};
    char time_s[20] = {'\0'};

    time(&now);
#ifdef TIME_SHOW_SEC
    tnow = localtime(&now);
    sprintf(time_date, "%4d-%02d-%02d", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min);
    sprintf(time_s, "%02d:%02d:%02d",  tnow->tm_hour, tnow->tm_min, tnow->tm_sec);
#else
    tnow = localtime(&now);
    if (scene_cur == scene_pre) {
        if ((tm_min_prev == tnow->tm_min) && (net_link_status == net_st_prev)) return -1;
    }
    tm_min_prev = tnow->tm_min;
    sprintf(time_date, "%4d-%02d-%02d", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday);
    sprintf(time_s, "%02d:%02d",  tnow->tm_hour, tnow->tm_min);
#endif
    osd_put_canvas(statusbar_bg_rgb, OSD_STBAR_W, OSD_STBAR_H, OSD_STBAR_X, OSD_STBAR_Y);
    text_fill_canvas(text_font, ENCODING, RGB_WHITE, 27, 27, 0, 0, OSD_STBAR_TIME_X, OSD_STBAR_TIME_Y, time_s);
    text_fill_canvas(text_font, ENCODING, RGB_WHITE, 16, 16, 0, 0, OSD_STBAR_DATE_X, OSD_STBAR_DATE_Y, time_date);

    if (net_link_status)
        img_fill_canvas_ex(IMG_NET_LINK_ON,  IMG_NET_LINK_X, IMG_NET_LINK_Y);
    else
        img_fill_canvas_ex(IMG_NET_LINK_OFF, IMG_NET_LINK_X, IMG_NET_LINK_Y);
    net_st_prev = net_link_status;
    return 0;
}

static void statusbar_show_canvas(void)
{
    if (0 == statusbar_fill_canvas())
        osd_draw_canvas(OSD_STBAR_W, OSD_STBAR_H, OSD_STBAR_X, OSD_STBAR_Y);
}

static int ui_start_auto_slide(void)
{
    //return list_fill_detail(list_pic);
    int ret = 0;
   #ifdef ADD_UNLOCK_AD
    ret = list_auto_unlock_ad_show(list_unlock_ad);
    #else
    ret = list_auto_slide_show(list_pic);
    #endif

    return ret;
}

void ui_action(UI_MSG_ACTION act, int arg)
{
    unsigned char msg[MSG_LEN_UI];
    msg[0] = MSG_FROM_UI;
    msg[1] = MSG_UI_ACTION;
    msg[2] = act;
    msg[3] = arg;
    pipe_put(ui_pipe_ui, msg, MSG_LEN_UI);
}

void ui_scene_timeout(int arg, int arg2)
{
    switch(scene_cur) {
        case SCENE_MAIN:
            ui_action(UI_LCD_DORMANCY, 0);
            break;
        case SCENE_LCD_OFF:
            ui_action(UI_SCENE_SWITCH, SCENE_LCD_OFF);
            break;
        case SCENE_PIC_DETAIL:
            if (ui_oper_mode == UI_SCREENSAVE) 
                ui_action(UI_SCENE_SWITCH, SCENE_LCD_OFF);
            else
                ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
            break;
        case SCENE_FACE_DETECT:
            ui_exit_face_det();
            ui_action(UI_FORCE_SWITCH, SCENE_MAIN);
        break;
        default:
            ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
            break;  
    }
}

void ui_scene_reset_time(int scene)
{
    switch(scene_cur) {
        case SCENE_MAIN:
            tim_reset_time(tim_scene_timeout, TIME_1S(dev_cfg.delay_screen));
            break;
        case SCENE_CALLING:
        case SCENE_TALKING:
            tim_reset_time(tim_scene_timeout, TIME_1S(100));
            break;
        case SCENE_NO_ANSWER:
            tim_reset_time(tim_scene_timeout, TIME_1S(15));
            break;
        case SCENE_UNLOCK:
        case SCENE_FINGER_UNLOCK://by mkq finger
        case SCENE_NO_FINGER:
#ifndef ADD_UNLOCK_AD
            tim_reset_time(tim_scene_timeout, TIME_1S(3));
#else            
            tim_reset_time(tim_scene_timeout, TIME_1S(10));
#endif
            break;
        case SCENE_PIC_DETAIL:
            if (ui_oper_mode == UI_SCREENSAVE) {
                if (!DEV_TIME_SNS)
                    tim_suspend_event(tim_scene_timeout);
                else
                    tim_reset_time(tim_scene_timeout, DEV_TIME_SNS);
            }
            else
                tim_reset_time(tim_scene_timeout, TIME_1S(dev_cfg.delay_gomain));
            break;
        case SCENE_LCD_OFF:
            tim_suspend_event(tim_scene_timeout);
            break;
        case SCENE_FACE_DETECT: 
#ifdef _FACE_DET           
            tim_suspend_event(tim_scene_timeout);
#else
            tim_reset_time(tim_scene_timeout, TIME_1S(10));
#endif
            break;              
        case SCENE_REG_FINGER:
            tim_suspend_event(tim_scene_timeout);
            break;
        default:
            tim_reset_time(tim_scene_timeout, TIME_1S(dev_cfg.delay_gomain));
            break;  
    }
}

static void ui_msg2proc(unsigned char msg1, unsigned char msg2, unsigned char msg3, unsigned char msg4, unsigned char msg5, unsigned char msg6)
{
    unsigned char msg[MSG_LEN_PROC];
    msg[0] = MSG_FROM_UI;
    msg[1] = msg1;
    msg[2] = msg2;
    msg[3] = msg3;
    msg[4] = msg4;
    msg[5] = msg5;
    msg[6] = msg6;
    pipe_put(ui_pipe_proc, msg, MSG_LEN_PROC);
}

void ui_get_target(unsigned char room[])
{
    printf("ui_get_target ui_oper_mode:%d ,target_night:%02X%02X%02X%02X\n", ui_oper_mode, target_night[0], target_night[1], target_night[2], target_night[3]);
    //if (ui_oper_mode == UI_NIGHT_TALKING)
    //if (dev_cfg.day_night == 0)
    if ((ui_oper_mode == UI_NIGHT_CALLING) || (ui_oper_mode == UI_NIGHT_TALKING))
        memcpy(room, target_night, 4);
    else
        memcpy(room, target, 4);
}
static void ui_send_call_records(unsigned char dest[], unsigned char type)
{
    ui_msg2proc(UI_SEND_CALL_RECORDS, dest[0], dest[1], dest[2], dest[3], type);
}
static void ui_make_call(unsigned char floor, unsigned char unit, unsigned char room1, unsigned char room2)
{
    unsigned char cmd = STEL;
    target[0] = floor;
    target[1] = unit;
    target[2] = room1;
    target[3] = room2;
    ui_get_village();
    try_call_ser=0;
    if (ui_oper_mode == UI_NIGHT_TALKING) {
        cmd = STEL;
    }
    else
    if (dev_cfg.day_night == 0){
        //if ((0x00 != (target[0]|target[1]|target[2]|target[3]))) {
        if (dev_get_type(target) != DEV_ADMIN) {
            ui_oper_mode = UI_NIGHT_CALLING;
            memcpy(target_night, target, 4);
            memset(target, 0x00, 4);
            cmd = SONE;
        }
    }
    if ((scene_cur == SCENE_NO_ANSWER) && (dev_get_type(target) != DEV_ADMIN)) 
        cmd = 0x7C;
    ui_msg2proc(UI_SEND_AJB_DATA, cmd, target[0], target[1], target[2], target[3]);
    app_debug(DBG_INFO, "ui_make_call: %02x%02x%02x%02x\n", floor, unit, room1, room2);
}

static void ui_make_end(unsigned char floor, unsigned char unit, unsigned char room1, unsigned char room2)
{
    target[0] = floor;
    target[1] = unit;
    target[2] = room1;
    target[3] = room2;
    ui_msg2proc(UI_SEND_AJB_DATA, SEND, target[0], target[1], target[2], target[3]);
    //ui_msg2proc(UI_SEND_AJB_DATA, SSINT, target[0], target[1], target[2], target[3]);
}


static int manage = 0;

static int ui_check_edit_pw(edit_handle h_edit, const char *password)
{
    if (0 == strcmp(edit_get_text(h_edit), password)) {
        return PW_OK;
    }
    edit_show_prompt(h_edit, RGB_RED, STR_PW_ERROR);

    return PW_ERROR;
}

static int ui_check_edit_cmd(edit_handle h_edit, const char *command)
{
    if (0 == strcmp(edit_get_text(h_edit), command)) {
        if (!manage) {
            edit_show_prompt(h_edit, RGB_RED, STR_NO_MANAGE);
            return CMD_LIMIT;
        }
        return CMD_OK;
    }

    return CMD_UNMATCH;
}

static int ui_check_edit_ip(edit_handle h_edit, int ip_type)
{
    unsigned char ucip[4];

    int result = dev_check_ip(edit_get_text(h_edit), ucip, ip_type);
 
    if (result == IP_ERR_ILLEGAL)
        edit_show_prompt(h_edit, RGB_RED, "输入不合法,请重输!");
    else
    if (result == IP_ERR_PRIV)
        edit_show_prompt(h_edit, RGB_RED, "非私有地址,请重输!");
    else
    if (result == IP_ERR_MASK)
        edit_show_prompt(h_edit, RGB_RED, "掩码无效,请重输!");
    else 
        edit_show_text(h_edit, WG_DRAW, "%hhu.%hhu.%hhu.%hhu", ucip[0], ucip[1], ucip[2], ucip[3]);
    
    return result;
}

#define DT_DATE     0
#define DT_TIME     1
static int ui_check_edit_dt(edit_handle h_edit, int dt_type)
{
    unsigned int  tm_year = 0, tm_mon = 0, tm_day = 0;
    unsigned int  tm_hour = 0, tm_min = 0, tm_sec = 0;
    unsigned char mon_days = 0;
    
    if (dt_type == DT_DATE) {
        if (sscanf(edit_get_text(h_edit), "%u-%u-%u", &tm_year, &tm_mon, &tm_day) != 3)
            goto dt_err;
        if (tm_year == 0)
            goto dt_err;
        if ((tm_mon < 1) || (tm_mon > 12))
            goto dt_err;
        if (tm_mon == 2) {
            if (((tm_year%4 ==0) && (tm_year%100 != 0)) || (tm_year%400 == 0))
                mon_days = 29;
            else
                mon_days = 28;
        }
        else
        if ((tm_mon == 4) || (tm_mon == 6) || (tm_mon == 9) || (tm_mon == 11))
            mon_days = 30;
        else
            mon_days = 31;
        if ((tm_day < 1) || (tm_day > mon_days)) 
            goto dt_err; 
    }
    else
    if (dt_type == DT_TIME) {
        int item = sscanf(edit_get_text(h_edit), "%u:%u:%u", &tm_hour, &tm_min, &tm_sec);
        if ((item != 2) && (item != 3))
            goto dt_err;
        if ((tm_hour > 24) || (tm_min > 59) || (tm_sec > 59))
            goto dt_err;
    }
    return 0;
    
dt_err:
    if (dt_type == DT_DATE) {
        edit_show_prompt(h_edit, RGB_RED, "日期有误,请重输!");
    }
    else
    if (dt_type == DT_TIME) {
        edit_show_prompt(h_edit, RGB_RED, "时间有误,请重输!");
    }
    return -1;
}

/* F1-F4触发的scene变化在ui_button_release中实现
 * 要更改F1-F4触发的scene须用bt_Fn[scene_cur][3].scene = SCENE_SET_SEL;
 */
static inline void ui_scene_sw(int scene)
{
    if ((scene < SCENE_MAX) && (scene > SCENE_NONE)) {
        key_status = 0;
        scene_cur  = scene;
        //app_debug(DBG_INFO, "scene_cur = %d\n", scene_cur);
        printf("scene_cur = 0x%02x\n", scene_cur);
    }
}

static int  if_scene_fill_advance = 0;
static void ui_scene_fill_advance(int scene_next)
{
    if_scene_fill_advance = 1;//printf("----------scene_next:%x\n", scene_next);
    switch(scene_next) {
        case SCENE_SETTING:
            ui_fill_background(BG_COMM);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, 71, 66, "请输入工程密码:");
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 11, 11, 0, 0, 72, 86, "Please input the setup passwords");
            edit_fill_canvas_ex(edit_norm, ES_PW|ES_LM_4|ES_FT_L);
            break;

        case SCENE_HELP:
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_HELP_HEAD, IMG_HELP_HEAD_X, IMG_HELP_HEAD_Y);
            break;

        case SCENE_SET_PW:
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_PW_CHANGE, IMG_PW_CHANGE_X, IMG_PW_CHANGE_Y);
            img_fill_canvas_ex(IMG_TEXT_PW_OLD, IMG_TEXT_PW_X, IMG_TEXT_PW_OLD_Y);
            img_fill_canvas_ex(IMG_TEXT_PW_NEW, IMG_TEXT_PW_X, IMG_TEXT_PW_NEW_Y);
            img_fill_canvas_ex(IMG_TEXT_PW_FIRM, IMG_TEXT_PW_X, IMG_TEXT_PW_FIRM_Y);
            edit_fill_canvas(edit_pw_old);
            edit_fill_canvas(edit_pw_new);
            edit_fill_canvas(edit_pw_firm);
            text_fill_canvas(text_font, ENCODING, RGB_YELLOW, 12, 10, 0, 0, 72, 200-2, "*:删除/取消   #:下一项	OK:确定");
            break;

        case SCENE_SET_SCREEN:
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_BRIGHTNESS, IMG_BRIGHTNESS_X, IMG_BRIGHTNESS_Y);
            img_fill_canvas_ex(IMG_SNSAVERS_TIME, IMG_SNSAVERS_TIME_X, IMG_SNSAVERS_TIME_Y);
            img_fill_canvas_ex(IMG_ALWAY, IMG_ALWAY_X, IMG_ALWAY_Y);
            img_fill_canvas_ex(IMG_MIN_1, IMG_MIN_1_X, IMG_MIN_1_Y);
            img_fill_canvas_ex(IMG_MIN_5, IMG_MIN_5_X, IMG_MIN_5_Y);
	    //button_fill_canvas(bt_bright_dec, BT_PRESS);
            //button_fill_canvas(bt_bright_inc, BT_SHOW);		
            text_fill_canvas(text_font, ENCODING, RGB_YELLOW, 13, 13, 0, 0, 15+10, 200-12, "4:亮度- 6:亮度+ 2:屏保- 8:屏保+ *:取消 OK:确定");
            slider_fill_canvas(slider_bright, dev_cfg.level_bright);
            { 
            unsigned char radio_selected[3] = {0};
            if (dev_cfg.level_ss_tim > 2) 
                dev_cfg.level_ss_tim = 2; 

            radio_selected[dev_cfg.level_ss_tim] = 1;
 
            radio_fill_canvas(radio_min_0, 0, radio_selected[0]);
            radio_fill_canvas(radio_min_1, 0, radio_selected[1]);
            radio_fill_canvas(radio_min_5, 0, radio_selected[2]);
            }
            focus_screen_idx = 0;
            break;
        case SCENE_SET_VOLUMN:
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_VOLUMES, IMG_VOLUMES_X, IMG_VOLUMES_Y);
	    //button_fill_canvas(bt_volumes_dec, BT_PRESS);
           // button_fill_canvas(bt_volumes_inc, BT_SHOW);		
            slider_fill_canvas(slider_volumes, dev_cfg.level_volume);
            text_fill_canvas(text_font, ENCODING, RGB_YELLOW, 14, 14, 0, 0, 60, 200-12, "4:音量-  6:音量+  *:取消	OK:确定");
            break;
        case SCENE_SET_TIME:
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_DATE, IMG_DATE_X, IMG_DATE_Y);
            img_fill_canvas_ex(IMG_TIME, IMG_TIME_X, IMG_TIME_Y);
            edit_fill_canvas(edit_date);
            edit_fill_canvas(edit_time);
            text_fill_canvas(text_font, ENCODING, RGB_YELLOW, 14, 14, 0, 0, 56-15, 200-12, "*: 删除/取消   #: - / : /下一项	OK:确定");
            break;
        case SCENE_SET_IP:
            dev_config_get();
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_IP_ADDR, IMG_IP_ADDR_X, IMG_IP_ADDR_Y);
            img_fill_canvas_ex(IMG_IP_GATE, IMG_IP_GATE_X, IMG_IP_GATE_Y);
            img_fill_canvas_ex(IMG_IP_MASK, IMG_IP_MASK_X, IMG_IP_MASK_Y);
            edit_fill_canvas(edit_ip_addr); 
            edit_fill_canvas(edit_ip_gate);
            edit_fill_canvas(edit_ip_mask);
            edit_show_text(edit_ip_addr, WG_FILL, "%d.%d.%d.%d", \
            dev_cfg.ip_addr[0], dev_cfg.ip_addr[1], dev_cfg.ip_addr[2], dev_cfg.ip_addr[3]);
            edit_show_text(edit_ip_gate, WG_FILL, "%d.%d.%d.%d", \
            dev_cfg.ip_gate[0], dev_cfg.ip_gate[1], dev_cfg.ip_gate[2], dev_cfg.ip_gate[3]);
            edit_show_text(edit_ip_mask, WG_FILL, "%d.%d.%d.%d", \
            dev_cfg.ip_mask[0], dev_cfg.ip_mask[1], dev_cfg.ip_mask[2], dev_cfg.ip_mask[3]); 
            text_fill_canvas(text_font, ENCODING, RGB_YELLOW, 14, 14, 0, 0, 70-20, 200-12, "*:删除/取消   #: . /下一项	OK:确定"); 
            break;
        case SCENE_SET_ABOUT: {
            char string[20]={'\0'};
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_ABOUT_MODEL, IMG_ABOUT_MODEL_X, IMG_ABOUT_MODEL_Y);
            img_fill_canvas_ex(IMG_ABOUT_IP, IMG_ABOUT_IP_X, IMG_ABOUT_IP_Y);
            img_fill_canvas_ex(IMG_ABOUT_MAC, IMG_ABOUT_MAC_X, IMG_ABOUT_MAC_Y);
            img_fill_canvas_ex(IMG_ABOUT_GATE, IMG_ABOUT_GATE_X, IMG_ABOUT_GATE_Y);
            img_fill_canvas_ex(IMG_ABOUT_S_VER, IMG_ABOUT_S_VER_X, IMG_ABOUT_S_VER_Y);
            img_fill_canvas_ex(IMG_ABOUT_H_VER, IMG_ABOUT_H_VER_X, IMG_ABOUT_H_VER_Y);
			
            img_fill_canvas_ex(EDIT_IMG_ABOUT_MODEL, EDIT_ABOUT_MODEL_X, EDIT_ABOUT_MODEL_Y);
            img_fill_canvas_ex(EDIT_IMG_ABOUT_IP, EDIT_ABOUT_MODEL_X, EDIT_ABOUT_IP_Y);
            img_fill_canvas_ex(EDIT_IMG_ABOUT_MAC, EDIT_ABOUT_MODEL_X, EDIT_ABOUT_MAC_Y);
            img_fill_canvas_ex(EDIT_IMG_ABOUT_GATE, EDIT_ABOUT_GATE_X, EDIT_ABOUT_GATE_Y);
            img_fill_canvas_ex(EDIT_IMG_ABOUT_SV, EDIT_ABOUT_GATE_X, EDIT_ABOUT_SV_Y);
            img_fill_canvas_ex(EDIT_IMG_ABOUT_HV, EDIT_ABOUT_GATE_X, EDIT_ABOUT_HV_Y);

            dev_config_get();
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 16, 16, 0, 0, EDIT_ABOUT_MODEL_X+2, EDIT_ABOUT_MODEL_Y+3, DEVICE_NAME);
            sprintf(string, "%02X%02X%02X", dev_cfg.my_code[0], dev_cfg.my_code[1], dev_cfg.my_code[3]);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 13, 13, 0, 0, EDIT_ABOUT_MODEL_X+1+94, EDIT_ABOUT_MODEL_Y+7, string);
            sprintf(string, "%d.%d.%d.%d", dev_cfg.ip_addr[0], dev_cfg.ip_addr[1], dev_cfg.ip_addr[2], dev_cfg.ip_addr[3]);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 16, 16, 0, 0, EDIT_ABOUT_MODEL_X+4, EDIT_ABOUT_IP_Y+2+6, string);
            sprintf(string, "%02X:%02X:%02X:%02X:%02X:%02X", \
                    dev_cfg.ip_mac[0], dev_cfg.ip_mac[1], dev_cfg.ip_mac[2], dev_cfg.ip_mac[3], dev_cfg.ip_mac[4], dev_cfg.ip_mac[5]);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 16, 16, 0, 0, EDIT_ABOUT_MODEL_X+3, EDIT_ABOUT_MAC_Y+10, string);
            sprintf(string, "%d.%d.%d.%d", dev_cfg.ip_gate[0], dev_cfg.ip_gate[1], dev_cfg.ip_gate[2], dev_cfg.ip_gate[3]);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 16, 16, 0, 0, EDIT_ABOUT_GATE_X+3, EDIT_ABOUT_GATE_Y+6, string);
            //sprintf(string, "%s-%s", "11E", __DATE__);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 16, 16, 0, 0, EDIT_ABOUT_GATE_X+3, EDIT_ABOUT_SV_Y+2+6, dev_software_ver());
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 16, 16, 0, 0, EDIT_ABOUT_GATE_X+3, EDIT_ABOUT_HV_Y+10, "X3-A07840825");
            }
            break;
        case SCENE_TXT_DETAIL:
            ui_fill_background(BG_COMM);
            list_fill_detail(list_msg);
            break;
        case SCENE_CATALOG:
            ui_fill_background(BG_COMM);
            catalog_fill_canvas(contacts);
           // text_fill_canvas(text_font, ENCODING, RGB_WHITE, 14, 14, 0, 0, 16, 218, "1:PgUp  7:PgDn  3:Prev  9:Next  *:Exit  #:Call");
            //text_fill_canvas(text_font, ENCODING, RGB_WHITE, 14, 14, 0, 0, 5, 218, "1:上页  7:下页  3:上一项  9:下一项  *:退出  #:确定");
            break;
        default:
            if_scene_fill_advance = 0;
            break;
    }
}

static void ui_scene_show(void)
{
    int i;
    //unsigned char string[30] = {'\0'};//by mkq finger
    if (scene_cur == scene_pre) return;//判断是否前后是同一个界面

    key_status      = 0;
    count_down_time = 0;
#if 0 //#ifndef UI_MEM_SEMISTATIC
/* 释放上一scene内存 */

#endif
    edit_flash_suspend(0);

    if (if_scene_fill_advance == 0)
        ui_scene_fill_advance(scene_cur);

    ui_scene_reset_time(scene_cur);
    tim_suspend_event(tim_count_down);
    label_reset(&label_leave_unlock);	
    label_reset(&label_noanswer_unlock);
    label_reset(&label_talking_unlock);
    label_reset(&label_prompt);
    label_reset(&label_noanswer);
    label_reset(&label_count_down);
    /* 显示当前画面基本元素 */
    switch(scene_cur) {
        case SCENE_MAIN:
            list_reset_index(list_msg);
            list_reset_index(list_pic);
            ui_oper_mode = UI_NORMAL; 
            talking_unlock_time = 0;
            calling_unlock_time=0;
            is_calling_unlock = 0;//add by wrm 20150119 for in calling unlock
            sb_pw=0;
            ui_fill_background(BG_MAIN);

            if (scene_pre == SCENE_LCD_OFF || scene_pre == SCENE_FACE_DETECT){
                audio_play(PMT_WELCOME, DEV_VOL_PLAY);
				//dev_code_cardreader_test(); //test
			}
            if(fp_status_get() != status_getimg ){//by mkq finger
                if(scene_pre == SCENE_CALL || scene_pre == SCENE_CALLING || scene_pre == SCENE_TALKING){//added by hgj 180711
                    break;//呼叫或对讲结束需要呼梯，先不开指纹轮询
                }
                else{
                    fp_status_set(status_getimg);
                }
            }
            
			break;
        case SCENE_SETTING:
            edit_set_current(edit_norm);
            break;
        case SCENE_CALL:
            audio_play(PMT_CALL_USER, DEV_VOL_PLAY);
            ui_fill_background(BG_COMM);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, 48, 68, "请输入房号:");
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 11, 11, 0, 0, 48+1, 88, "Please enter the room");			
            img_fill_canvas_ex(IMG_CALL_BACK, CALL_BACK_X, CALL_BACK_Y);
     
            img_fill_canvas_ex(IMG_CALL_MESS, CALL_MESS_X, CALL_MESS_Y);		
            edit_fill_canvas_ex(edit_norm, ES_NUM|ES_LM_C|ES_FT_L);
            edit_set_current(edit_norm);
            //img_fill_canvas_ex(IMG_CALL_ICON, CALL_ICON_X, CALL_ICON_Y);						
	
            break;

        case SCENE_CALLING:
            ui_fill_background(BG_COMM);
            //img_fill_canvas_ex(IMG_CALLING, IMG_CALLING_X, IMG_CALLING_Y);//正在呼叫业主	
            if (dev_get_type(target) == DEV_ADMIN) {
		img_fill_canvas_ex(IMG_CALLING_ADMIN, IMG_CALLING_UP_X, IMG_CALLING_UP_Y);		
                //text_fill_canvas(text_font, ENCODING, RGB_WHITE, 23, 23, 0, 0, IMG_CALLING_X+2, IMG_CALLING_Y, "正在呼叫管理处...");
                //text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, IMG_CALLING_X, IMG_CALLING_Y+26, "Calling is in progress...");
            }
            else {
		img_fill_canvas_ex(IMG_CALLING_UP, IMG_CALLING_UP_X, IMG_CALLING_UP_Y);		
                img_fill_canvas_ex(IMG_CALLING, IMG_CALLING_X, IMG_CALLING_Y);//正在呼叫业主
            }
	    img_fill_canvas_ex(IMG_CALLING_DOWN, IMG_CALLING_DOWN_X, IMG_CALLING_DOWN_Y);
	    //count_down_time = 30;//add by wrm 20150429	
            label_show_text(&label_prompt, WG_FILL, RGB_YELLOW, "连接中，请稍候...");		
                //img_fill_canvas_ex(IMG_CALLING_LINK, IMG_CALLING_LINK_X, IMG_CALLING_LINK_Y);          
            break;

         case SCENE_NO_ANSWER:
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_NO_ANSWER, IMG_NO_ANSWER_X, IMG_NO_ANSWER_Y);
            label_show_text(&label_noanswer, WG_FILL, RGB_YELLOW, "1:留言  2:重拨  *:退出");
            ui_oper_mode = UI_NORMAL; 
            count_down_time = 15;   //for talking_unlock_time - count_down_time == 3
            is_calling_unlock = 0; //add by wrm 20150119 for in calling unlock
            tim_reset_time(tim_count_down, TIME_1S(1));
            break;
        
        case SCENE_INPUT_PW:
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_INPUT_PWD_BACK, IMG_INPUT_PWD_X, IMG_INPUT_PWD_Y);
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, 48, 68, "请输入密码:");
            text_fill_canvas(text_font, ENCODING, RGB_WHITE, 11, 11, 0, 0, 49, 88, "Please input the passwords");
            edit_fill_canvas_ex(edit_norm, ES_PW|ES_FT_L);
            edit_set_current(edit_norm);
            if (key_code == KEY_F2 && scene_pre == SCENE_MAIN) {
                ui_oper_mode = UI_INPUT_PB_PW;
                edit_set_style(edit_norm, ES_PW|ES_LM_6|ES_FT_L);
            }
            else if (key_code == KEY_Y && scene_pre == SCENE_MAIN) {
                ui_oper_mode = UI_INPUT_SB_PW;
                edit_set_style(edit_norm, ES_NUM|ES_LM_6|ES_FT_L);
            }
    	    else {
                ui_oper_mode = UI_NORMAL;
                edit_set_style(edit_norm, ES_PW|ES_LM_8|ES_FT_L);
            }
            
    	    if(scene_pre == SCENE_CATALOG){//add by wrm 20141119
        		ui_oper_mode = UI_CATALOG;
        		edit_norm->index = edit_norm->limit;
        		ui_proc_input_pw();
        		//(*ui_proc_list[scene_cur])(); 
    	    }
	//    printf("pre = %02x ,%d \n",scene_pre,ui_oper_mode);
            break;

        case SCENE_REG_FINGER://by mkq finger
	        edit_set_current(edit_norm);
	        ui_oper_mode = UI_NORMAL;
	        edit_set_style(edit_norm, ES_PW|ES_LM_14|ES_FT_S);
            break;

        case SCENE_UNLOCK:
	    case SCENE_FINGER_UNLOCK://by mkq finger
            audio_play(PMT_UNLOCK, DEV_VOL_PLAY);
		 printf("ruan_UI_CARD_UNLOCK SCENE_UNLOCK\n");// by mkq
		ui_fill_background(BG_COMM);//by mkq
            img_fill_canvas_ex(IMG_UL_HEAD, IMG_UL_HEAD_X, IMG_UL_HEAD_Y);
            img_fill_canvas_ex(IMG_UL_MSG, IMG_UL_MSG_X, IMG_UL_MSG_Y);//by mkq
#ifndef ADD_UNLOCK_AD
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_UL_HEAD, IMG_UL_HEAD_X, IMG_UL_HEAD_Y);
            img_fill_canvas_ex(IMG_UL_MSG, IMG_UL_MSG_X, IMG_UL_MSG_Y);
#else
	     list_auto_unlock_ad_show(list_unlock_ad);
		//list_auto_slide_show(list_pic);
#endif
            break;
        case SCENE_NO_FINGER://by mkq finger
             buzz_play(BUZZ_WARNING);
             ui_fill_background(BG_COMM);
                //img_fill_canvas_ex(IMG_UL_HEAD, IMG_UL_HEAD_X, IMG_UL_HEAD_Y);
                text_fill_canvas(text_font, ENCODING, RGB_RED, 24, 24, 0, 0, 70, 120, "未发现匹配指纹");
             break;
        case SCENE_SET_SEL:
            ui_fill_background(BG_COMM);
            button_fill_canvas(bt_set_sel[focus_sel_idx], BT_PRESS);
            for (i = 0; i < focus_sel_idx; i++)
                button_fill_canvas(bt_set_sel[i], BT_SHOW);
            for (i = focus_sel_idx + 1; i < SET_SEL_NUM; i++)
                button_fill_canvas(bt_set_sel[i], BT_SHOW);
            text_fill_canvas(text_font, ENCODING, RGB_YELLOW, 14, 12, 0, 0, 75-28, 200-5, "4:左移 6:右移 2:上移 8:下移	OK:确定 ");
            break;
        
        case SCENE_SET_PW:
            edit_set_current(edit_pw_old);
            break;

        case SCENE_SET_TIME:
            edit_set_current(edit_date);
            break;
        
        case SCENE_SET_IP:
            edit_set_current(edit_ip_addr);
            break;

        case SCENE_TALKING:
            atest_cnt_inc(AT_CNT_TALK);
            if (ui_oper_mode == UI_NIGHT_CALLING)
                ui_oper_mode =  UI_NIGHT_TALKING;
            ui_fill_background(BG_COMM);
            img_fill_canvas_ex(IMG_REMAIN_TIME, IMG_REMAIN_TIME_X, IMG_REMAIN_TIME_Y);
		 //img_fill_canvas_ex(IMG_REMAIN_TIME, IMG_REMAIN_TIME_X-40, IMG_REMAIN_TIME_Y);// add by mkq 20170907
		   // text_fill_canvas(text_font, ENCODING, RGB_WHITE, 23, 23, 0, 0, 63, 70, "通话剩余时间");
            //text_fill_canvas(text_font, ENCODING, RGB_WHITE, 18, 18, 0, 0, 64, 96, "Remaining time");
            count_down_time = 60;
            //count_down_time = 3600;//60; add by mkq 20170907
	    is_calling_unlock = 0;	//add by wrm 20150119 for in calling unlock
            label_show_text(&label_count_down, WG_FILL, RGB_YELLOW, "%02d", count_down_time);
             //label_show_text(&label_count_down1, WG_FILL, RGB_YELLOW, "%04d", count_down_time);//add by mkq 20170907
            tim_reset_time(tim_count_down, TIME_1S(1));
            break;
        case SCENE_PIC_DETAIL:
            if (ui_oper_mode == UI_SCREENSAVE) {
                tim_reset_time(tim_count_down, TIME_1S(5));
            }
            break;
        case SCENE_LCD_OFF:
#ifdef _FACE_DET 
            ui_lcd_off();
    #ifdef _KEEP_OPEN
            printf("face_det_start [%s_%d]\n", __func__, __LINE__);//by hgj                   
            ui_scene_sw(SCENE_FACE_DETECT);   
            reset_ir_detect_status();
            /*开始人脸检测，发送图片给服务器*/
            face_det_start();
    #endif

#else
            ui_lcd_off();
#endif

            break;
//add for multi_lift_ctl
        case SCENE_SET_LIFT_IP:
            edit_set_current(edit_norm);
            edit_set_style(edit_norm, ES_IP|ES_FT_N);
            edit_show_text(edit_norm, WG_DRAW,"%d.%d.%d.%d", dev_cfg.lift_ip[lift_ctrl_no][0], dev_cfg.lift_ip[lift_ctrl_no][1],\
                    dev_cfg.lift_ip[lift_ctrl_no][2], dev_cfg.lift_ip[lift_ctrl_no][3]); 
            text_fill_canvas(text_font, ENCODING, RGB_YELLOW, 14, 14, 0, 0, 70-20, 200-12, "*:删除/取消  C:清空  #: .  OK:确定");                     
            break;            
            
        case SCENE_MAX:
            ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
        default:
            break;
    }

    statusbar_get_canvas();
    statusbar_fill_canvas();
    statusbar_bg_get_en = 1;    //SCENE更新后，使能获取statusbar背景

    scene_pre = scene_cur;

    atest_info_show();
    osd_draw_canvas(osd0_rect.width, osd0_rect.height, 0, 0);
    edit_flash_resume();
    if(scene_cur == SCENE_MAIN){
        usleep(500*1000);//避免用户回到主页面后立即做其他操作导致冲突
    }    
}

static int ui_button_press(char key_code, char key_down)
{
    int scene_next = SCENE_NONE;
    if (scene_cur >= SCENE_MAX) return -1;
#if 1
    if (!key_down) return -1;
    key_status = 1;
    switch (key_code) {
        case KEY_X:
        case KEY_Y:
        case KEY_F3:
            break;
        default:
            if (scene_cur == SCENE_MAIN) //主界面按下数字键触发CALL,否则只有抬起按键才处理
                return -1;
            break;
    }
#endif
    /*button F1-F4触发且确认按下刷屏内容固定才可在按下时刷画板,否则在ui_scene_show中刷*/
    ui_scene_fill_advance(scene_next);
    //按下预先填充完scene后时获取statusbar背景,scene可能被刷时间的消息刷上时间，故切换界面时不应再获取statusbar背景
    if (if_scene_fill_advance)
        statusbar_get_canvas(); 

    return 0;
}

static int ui_button_release(char key_code, char key_down)
{
    if (scene_cur >= SCENE_MAX) return -1;
    if (key_down)    return -1;
    if (!key_status) return -1;
    key_status = 0;

    if (scene_cur != scene_pre) return -1;

    return 0;
}

inline void ui_delay_return_set_sel(int time)
{
    dev_config_save();
    usleep(500*1000);
    ui_action(UI_SCENE_SWITCH, SCENE_SET_SEL);
}


static void ui_proc_main(void)
{
    switch(key_code) {
      // case KEY_Y:
     //  		ui_scene_sw(SCENE_INPUT_PW);
     //       break;
    
       case KEY_X: 
#ifdef APP_AUTO_TEST
        if (ATEST_RUN == atest_get_status()) { 
            ui_scene_sw(SCENE_INPUT_PW);
            break;
        }
#endif	  
        sb_pw=1;
        //edit_text_clr(edit_norm);
        ui_scene_sw(SCENE_INPUT_PW); 
break;

       // if (!dev_cfg.en_user_unlock) break;//屏蔽用户密码开锁		
	case KEY_F2://add by wrm 20140104 公共密码开锁按键		
            ui_scene_sw(SCENE_INPUT_PW);
            break;
        case KEY_F4://add by wrm 20140104 管理处按键
            ui_make_call(0x00, 0x00, 0x00, 0x00);
            ui_scene_sw(SCENE_CALLING);
            break;
         
        case KEY_F3:
            printf("face_det_start [%s_%d]\n", __func__, __LINE__);//by hgj             
            Set_CCD_Light(1); 
            face_det_start();
            face_det_flag = 1;//应对红外感应失效的场景
            ui_scene_sw(SCENE_FACE_DETECT); 
        break;
       
        default:
            if ((key_code>='0') && (key_code<='9')) {
                ui_scene_sw(SCENE_CALL);
            }
            break;
    }
}


void ui_get_unlock_pw(unsigned char passwd0, unsigned char passwd1,unsigned char passwd2){
    unsigned char data[4] = {0};
    data[0] = passwd0;
    data[1] = passwd1;
    data[2] = passwd2;	
    data[3] = 'G';  
    ui_msg2proc(UI_SEND_AJB_DATA, GETUK, data[0], data[1], data[2], data[3]);
}


 void ui_get_village(){
    ui_msg2proc(UI_SEND_AJB_DATA, GETVI, 0x00,0x00,0x00,0x00);
}
static void ui_proc_message(void)
{

}

static void ui_proc_picture(void)
{

}

static void ui_proc_setting(viod)
{
    switch(key_code) {
        case KEY_X:
            if (!edit_is_empty(edit_norm))
                edit_text_clr(edit_norm);
            else
                ui_scene_sw(SCENE_MAIN);
            break;
        default:
            if (edit_is_full(edit_norm)) {
                int cmd_ret = ui_check_edit_cmd(edit_norm, "8550");
                if (cmd_ret == CMD_UNMATCH) {
                    edit_show_prompt(edit_norm, RGB_RED, STR_PW_ERROR);
                }
                else
                if (cmd_ret == CMD_OK) {
                    ui_scene_sw(SCENE_SET_SEL);
                }
            }
            break;
    }
}

static void ui_proc_help(void)
{

}
static void ui_proc_call(void)
{
    unsigned char room[8];
    switch(key_code) {
        case KEY_F1:
#ifndef FRONT_DOOR
            tim_suspend_event(tim_count_down);
#endif
            if (!edit_is_empty(edit_norm))
                edit_text_clr(edit_norm);
            else
                ui_scene_sw(SCENE_MAIN);
            break;
	case KEY_X:
            if (edit_is_empty(edit_norm)){
 		 ui_scene_sw(SCENE_MAIN);	
		}
	    break;
        default:
#ifndef FRONT_DOOR
            if (edit_get_length(edit_norm) == (ROOM_NUM-1)) {
                edit_get_bcd(edit_norm, room);
                if (room[0] & 0xF0) {
                    count_down_time = 4;
                    tim_reset_time(tim_count_down, TIME_1S(1));
                }
                break;
            }
#endif
            if (edit_get_length(edit_norm) >= ROOM_NUM) {
                
                edit_get_bcd(edit_norm, room);
#ifdef FRONT_DOOR
                if (!(room[0]|room[1]|room[2]|room[3])) {
                    ui_scene_sw(SCENE_INPUT_PW);
                    break;
                }
#else
                count_down_time = 0;
                tim_suspend_event(tim_count_down);
                memcpy(&room[2], room, 2); 
                memcpy(room, dev_cfg.my_code, 2); 
                if (!(room[2]|room[3])) { 
                    ui_scene_sw(SCENE_INPUT_PW);
                    break;
                }
#endif
                if (DEV_INDOOR == dev_get_type(room)) {
                    ui_make_call(room[0], room[1], room[2], room[3]);
                    usleep(100*1000);
                    ui_scene_sw(SCENE_CALLING);
                } else {
                    edit_show_prompt(edit_norm, RGB_RED, "房号有误,请重输!");
                }
            }
            break;
    }
}

static void ui_proc_calling(void)
{
    switch(key_code) {
        case KEY_X:
        case KEY_F1:    //add by wrm 20140104 C按键
            ui_make_end(target[0], target[1], target[2], target[3]);
            ui_scene_sw(SCENE_MAIN);
            break;    
        //case KEY_F4: //del by wrm 20141113
           // ui_make_end(target[0], target[1], target[2], target[3]);
            break;
    }
}

static void ui_proc_no_answer(void)
{
    switch(key_code) {
        case KEY_1:
            if (ui_oper_mode == UI_NORMAL) {
                ui_oper_mode =  UI_WAIT_LEAVE_MSG;
                label_clear(&label_noanswer);
        		label_clear(&label_noanswer_unlock);
        		label_clear(&label_noanswer_no_unlock);
                label_show_text(&label_prompt, WG_DRAW, RGB_WHITE, "  正在准备留言...");
                tim_reset_time(tim_scene_timeout, TIME_1S(20));
                ui_msg2proc(UI_SEND_LEAVE_MSG, 1, target[0], target[1], target[2], target[3]);
            }
            break;
        case KEY_2:
            if ((ui_oper_mode == UI_WAIT_LEAVE_MSG) || (ui_oper_mode == UI_LEAVING_MSG))
                ui_msg2proc(UI_SEND_LEAVE_MSG, 0, target[0], target[1], target[2], target[3]);
            label_clear(&label_prompt);
            ui_make_call(target[0], target[1], target[2], target[3]);
            ui_scene_sw(SCENE_CALLING);
            break;
       // case KEY_F1://add by wrm 20140104 C按键
        case KEY_X:
            if ((ui_oper_mode == UI_WAIT_LEAVE_MSG) || (ui_oper_mode == UI_LEAVING_MSG))
                ui_msg2proc(UI_SEND_LEAVE_MSG, 0, target[0], target[1], target[2], target[3]);
            ui_scene_sw(SCENE_MAIN);
            break;
    }
}

static void ui_proc_talking(void)
{
    switch(key_code) {
        case KEY_X:
        case KEY_F1: //add by wrm 20140104 C按键
            ui_make_end(target[0], target[1], target[2], target[3]);
            ui_scene_sw(SCENE_MAIN);
            break;
        //case KEY_F4://del by wrm 20141114
           // ui_make_end(target[0], target[1], target[2], target[3]);
           // break;
    }
}

static void ui_proc_set_sel(void)
{
    switch(key_code) {
        case KEY_2:
            button_release(bt_set_sel[focus_sel_idx]);
            if (focus_sel_idx == 0) 
                focus_sel_idx = SET_SEL_NUM-1;
            else
            if (focus_sel_idx >= 3) 
                focus_sel_idx-= 3;
            else 
                focus_sel_idx+= 2;
            button_press(bt_set_sel[focus_sel_idx]);
            break;
        case KEY_4:
            button_release(bt_set_sel[focus_sel_idx]);
            if (focus_sel_idx == 0) 
                focus_sel_idx = SET_SEL_NUM-1;
            else
                focus_sel_idx-= 1;
            button_press(bt_set_sel[focus_sel_idx]);
            break;
        case KEY_8:
            button_release(bt_set_sel[focus_sel_idx]);
            if (focus_sel_idx == SET_SEL_NUM-1) 
                focus_sel_idx = 0;
            else
            if (focus_sel_idx >= 3) 
                focus_sel_idx-= 2;
            else 
                focus_sel_idx+= 3;
            button_press(bt_set_sel[focus_sel_idx]);
            break;
        case KEY_6:
            button_release(bt_set_sel[focus_sel_idx]);
            if (focus_sel_idx == SET_SEL_NUM-1) 
                focus_sel_idx = 0;
            else
                focus_sel_idx+= 1;
            button_press(bt_set_sel[focus_sel_idx]);
            break;
        case KEY_F1: //add by wrm 20140104 C按键
        case KEY_X:
            ui_scene_sw(SCENE_MAIN);
            break;
        case KEY_F3: //add by wrm 20140104 OK按键
            ui_scene_sw(scene_set_sel[focus_sel_idx]);
            break;
        default:
            break;
    }
}

global_data     gbl_alarm_delay = GBL_DATA_INIT;
global_data     gbl_user_unlock= GBL_DATA_INIT;


static void ui_proc_catalog(void)
{
    switch(key_code) {
        case KEY_F1://add by wrm 20140104 C按键
            ui_scene_sw(SCENE_MAIN);
            break;
        case KEY_F3://add by wrm 20140104 OK按键
#if 0 //del and add by wrm 20141112
            if (0 == catalog_get_bcd(contacts, room)) {
                ui_make_call(dev_cfg.my_code[0], dev_cfg.my_code[1], room[0], room[1]);
                ui_scene_sw(SCENE_CALLING);
            }
#else
            if (0 == catalog_get_code(contacts, code)) {
               // ui_make_call(dev_cfg.my_code[0], dev_cfg.my_code[1], room[0], room[1]);
		printf("Catalog code is %s \n",code);
		catalog_flag = 1;                //确认在input_pw界面可确定是由此界面跳转
		//ui_oper_mode = UI_CATALOG;
		//edit_norm->index = edit_norm->limit;
		ui_scene_sw(SCENE_INPUT_PW);
            }            
#endif
            break;
        case KEY_1:
       // case KEY_L_1:
            catalog_page_up(contacts);
            break;
        case KEY_7:
       // case KEY_L_7:
            catalog_page_down(contacts);
            break;
        case KEY_3:
       // case KEY_L_3:
            catalog_move_up(contacts);
            break;
        case KEY_9:
       // case KEY_L_9:
            catalog_move_down(contacts);
            break;
#ifdef CFG_CATALOG_LONGPRESS
        case KEY_RELS:
            tim_suspend_event(tim_key_successive);
            return;
#endif
        default:
            break;
    }
#ifdef CFG_CATALOG_LONGPRESS
    if (IsLongPress(key_code)) {
        if ((key_code==KEY_L_1) || (key_code==KEY_L_7)) {
            tim_reset_time_arg(tim_key_successive, TIME_250MS(3), (tim_callback)key_action, LongPress2Key(key_code), 0);
        } else if ((key_code==KEY_L_3) || (key_code==KEY_L_9) ) {
            tim_reset_time_arg(tim_key_successive, TIME_500MS(1), (tim_callback)key_action, LongPress2Key(key_code), 0);
        }
    }
#endif
}

static void ui_proc_input_pw(void)
{
    int  ret = CMD_UNMATCH;
    char *text_input;
    char string[30] = {'\0'};
    unsigned char room[8] = {0x00};
    unsigned char village[8] ={0x00};
    unsigned char buf[10];
    static char pw_kb_new[16];
    unsigned char recall_time;
    unsigned char buffer[8] = {0x00};//by mkq finger
    if (key_code == KEY_X)
    {
#ifdef APP_AUTO_TEST
        if (ATEST_RUN == atest_get_status()) {
            atest_pause();
            edit_show_prompt(edit_norm, RGB_GREEN, "自动呼叫测试已暂停!");
            return;
        }
#endif
        // ui_oper_mode = UI_NORMAL;
        // edit_set_style(edit_norm, ES_PW|ES_LM_8|ES_FT_L);
        // mod by wrm 20150330 for * del one char the original is * del all so not need	
        printf("edit_is_empty0=%d\n",edit_is_empty(edit_norm));
        if (edit_is_empty(edit_norm) && ui_oper_mode !=UI_NORMAL) {
           printf("oper_mode != UI_NORMAL\n");
            ui_oper_mode = UI_NORMAL;
            edit_set_style(edit_norm, ES_PW|ES_LM_8|ES_FT_L); 
        // ui_scene_sw(SCENE_MAIN);  		
        }
        else if (edit_is_empty(edit_norm) && ui_oper_mode ==UI_NORMAL)
        {
            printf("oper_mode == UI_NORMAL\n");		
            ui_scene_sw(SCENE_MAIN);        
        }		
    }
    else if (key_code == KEY_Y) {
#ifdef APP_AUTO_TEST
        if (ATEST_RUN == atest_get_status()) {
            atest_stop();
            ui_msg2proc(UI_SEND_AUTO_TEST, 0, target[0], target[1], target[2], target[3]);
            edit_show_prompt(edit_norm, RGB_GREEN, "自动呼叫测试已取消!");
        }
#endif
     //if ((scene_cur == SCENE_INPUT_PW)&&(sb_pw)&& (edit_is_empty(edit_norm))){
       if ((scene_cur == SCENE_INPUT_PW)&&(sb_pw)&& (ui_oper_mode ==  UI_NORMAL))
       {
            sb_pw=0;
            ui_oper_mode = UI_INPUT_SB_PW;
            edit_show_prompt(edit_norm, RGB_WHITE, "请输入访客密码");
            edit_set_style(edit_norm, ES_NUM|ES_LM_6|ES_FT_L);
        } 
    }
    else if (key_code == KEY_F1) { //add by wrm 20140104 C按键
       // if (!edit_is_empty(edit_norm)) {
            edit_text_clr(edit_norm);
        //} else {
            ui_scene_sw(SCENE_MAIN);
     //   }
    }
/*    else
     if((key_code == KEY_F4) ? (manage) : 0){//查询键，且已开管理模式
         
         ui_scene_sw(SCENE_CATALOG);
     }
     if((key_code == KEY_F4) ? (!manage) : 0){
         
          edit_show_prompt(edit_norm, RGB_RED, STR_NO_MANAGE);
     }
  */      
	else if (edit_is_full(edit_norm)) {
        dev_config_get();
        memset(buf, 0x00, 7);
        text_input = edit_get_text(edit_norm);
        switch (ui_oper_mode) {
            case UI_NORMAL:
			{
                if (strcmp(text_input, "00008558") == 0) {      //8558不受限于管理密令
                    ret = CMD_OK;   
                    ui_oper_mode = UI_INPUT_KB_PW;
                    edit_set_style(edit_norm, ES_PW|ES_LM_4|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入管理密码");
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008556")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_KB_PW_OLD;
                    edit_set_style(edit_norm, ES_PW|ES_LM_4|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入管理密码(旧)");
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008550")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_CODE;
                    edit_set_style(edit_norm, ES_NUM|ES_LM_6|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入编码");
                }

		  else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009551")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_VI_CODE;
                    edit_set_style(edit_norm, ES_NUM|ES_LM_6|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入小区编码");
                }

                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008031")) == CMD_OK) {
			if (dev_cfg.card_in_flash) {
                        edit_show_prompt(edit_norm, RGB_RED, "已开启主机存卡功能");
                        break;
                    }
                    if (dev_cfg.my_code[3]) {
                        edit_show_prompt(edit_norm, RGB_RED, "副机不能进行此操作!");
                        break;
                    }
#ifdef FRONT_DOOR
                    memset(buf, 0x00, 10);
                    buf[0] = SFOR;
                    buf[1] = dev_cfg.my_code[3];
                    uart_send_data(UART_1, buf);
                    edit_show_prompt(edit_norm, RGB_GREEN, "门禁格式化中,哔声后完成");
                    ui_oper_mode = UI_NORMAL;
#else
                    ui_oper_mode = UI_INPUT_CARD_FMT;
                    edit_set_style(edit_norm, ES_NUM|ES_LM_6|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入门禁格式化规则");
#endif
                }

#ifdef _FP_MODULE 
        		else if((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008064")) == CMD_OK) {//by mkq finger register
                     /*   if (dev_cfg.my_code[3]) {
                            edit_show_prompt(edit_norm, RGB_RED, "副机不能进行此操作!");
                            break;
                        }*/
                    ui_oper_mode = UI_INPUT_FINGER_REG;
    		      fp_status_set(status_non);
    		       printf("wating !!\n");
        			usleep(300*1000);
        			ui_action(UI_FINGER_REG_READ,0);
        			ui_scene_sw(SCENE_REG_FINGER);
        			printf("start!!!!!!\n");
        			fp_readcolist(0x00);
        			fp_status_set(status_index);
    			
    			//tim_reset_time(tim_scene_timeout, MIN(TIME_1S(60), TIME_1S(dev_cfg.delay_gomain)));
                     ui_oper_mode = UI_NORMAL;
                }

         		else if((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008062")) == CMD_OK) {//by mkq finger delete fingerprint
                  /*  if (dev_cfg.my_code[3]) {
                        edit_show_prompt(edit_norm, RGB_RED, "副机不能进行此操作!");
                        break;
                    }*/
                    printf("!!!!gbl_finger_status set non!!\n");
        		     fp_status_set(status_non);
                    ui_oper_mode = UI_INPUT_FINGER_DELE_ID;
                    edit_set_style(edit_norm, ES_NUM|ES_LM_4|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "输入删除的ID(4位,不足补0)");

                }

        		else if((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008061")) == CMD_OK) {//by mkq finger format fingerprint
                  /*  if (dev_cfg.my_code[3]) {
                        edit_show_prompt(edit_norm, RGB_RED, "副机不能进行此操作!");
                        break;
                    }*/
                    printf("!!!!gbl_finger_status set non!!\n");
                    fp_status_set(status_non);
		    
                    ui_oper_mode = UI_INPUT_FINGER_FMT;
                    edit_set_style(edit_norm, ES_PW|ES_LM_6|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "输入六位密码, 清空指纹");
                    usleep(100*1000);

                }
#endif

				
                
#ifdef CFG_USE_OLD_CARDREADER
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008051")) == CMD_OK) {
			if (dev_cfg.card_in_flash) {
                        edit_show_prompt(edit_norm, RGB_RED, "已开启主机存卡功能");
                        break;
                    }
                    if (dev_cfg.my_code[3]) {
                        edit_show_prompt(edit_norm, RGB_RED, "副机不能进行此操作!");
                        break;
                    }
                    ui_oper_mode = UI_INPUT_CARD_NUM;
#ifdef FRONT_DOOR
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入8位卡号");
                    edit_set_style(edit_norm, ES_NUM|ES_LM_8|ES_FT_L);
#else
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入6位序号+8位卡号");
                    edit_set_style(edit_norm, ES_NUM|ES_LM_14|ES_FT_L);
#endif
                }
#endif
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008054")) == CMD_OK) {
			if (dev_cfg.card_in_flash) {
                        edit_show_prompt(edit_norm, RGB_RED, "已开启主机存卡功能");
                        break;
                    }
                    if (dev_cfg.my_code[3]) {
                        edit_show_prompt(edit_norm, RGB_RED, "副机不能进行此操作!");
                        break;
                    }
                    ui_oper_mode = UI_INPUT_CARD_REG;
                    buf[0] = SWRI;
                    uart_send_data(UART_1, buf);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请刷卡注册");
                    tim_reset_time(tim_scene_timeout, MIN(TIME_1S(45), TIME_1S(dev_cfg.delay_gomain)));
                    ui_oper_mode = UI_NORMAL;
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008053")) == CMD_OK) {
			if (dev_cfg.card_in_flash) {
                        edit_show_prompt(edit_norm, RGB_RED, "已开启主机存卡功能");
                        break;
                    }
                    if (dev_cfg.my_code[3]) {
                        edit_show_prompt(edit_norm, RGB_RED, "副机不能进行此操作!");
                        break;
                    }
                    ui_oper_mode = UI_INPUT_CARD_DEL;
                    edit_set_style(edit_norm, ES_NUM|ES_LM_10|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "输入要删除的10位卡号");
                }
               else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008087")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_BLE_DBM_TX;
                    sprintf(string, "输入蓝牙信号强度(1~3)");
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_LM_1|ES_FT_L);
                }
#ifdef RecallTime   
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008085")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_VOLUME_RecallTime;
                    sprintf(string, "应答转呼时间(05~25)[%d]", dev_cfg.recall_time );
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_LM_2|ES_FT_L);
                }
#endif               
               else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009008")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_CIF_MODE;
                    sprintf(string, "云对讲视频为:1帧,5帧[%d]", (dev_cfg.cif_mode==0)?0:5);
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_BOOL|ES_LM_1|ES_FT_L);
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009182")) == CMD_OK) {
                    dev_cfg.en_mc_check = !dev_cfg.en_mc_check;
                    dev_config_save();
                    if (dev_cfg.en_mc_check)
                        edit_show_prompt(edit_norm, RGB_GREEN, "门磁状态检测开启");
                    else
                        edit_show_prompt(edit_norm, RGB_GREEN, "门磁状态检测关闭");
                }
                
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008055")) == CMD_OK) {//ADD_CARD_IN_FLADH
		//modify by wrm 20150515 开启主机存卡后不需要再重新编码					
                    dev_cfg.card_in_flash = !dev_cfg.card_in_flash;
                   // dev_config_save();
                    if (dev_cfg.card_in_flash)
                        edit_show_prompt(edit_norm, RGB_GREEN, "主机存卡开启");
                    else
                        edit_show_prompt(edit_norm, RGB_GREEN, "主机存卡关闭");
		usleep(500*1000);
                dev_code_cardreader();
                dev_config_save();
                edit_show_prompt(edit_norm, RGB_GREEN, "正在重启...");
                dev_app_restart();					
                }           
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008056")) == CMD_OK) {//ADD_SECTION_CODE
				//add by wrm 20150619								
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入扇区号及密码");
			ui_oper_mode = UI_INPUT_NEW_CARD;
			edit_set_style(edit_norm, ES_NUM|ES_LM_14|ES_FT_L);
                	}
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008553")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_MC_DELAY;
                    edit_set_style(edit_norm, ES_NUM|ES_LM_2|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入门磁报警延时");
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008990")) == CMD_OK) {
                    dev_cfg.en_fc_alarm = !dev_cfg.en_fc_alarm;
                    dev_config_save();
                    if (dev_cfg.en_fc_alarm)
                        edit_show_prompt(edit_norm, RGB_GREEN, "防拆报警开启");
                    else
                        edit_show_prompt(edit_norm, RGB_GREEN, "防拆报警关闭");
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009009")) == CMD_OK) {
                    ui_scene_sw(SCENE_SET_SEL);
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009003")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_LOCK_TYPE;
                    sprintf(string, "选择:0电控锁,1磁力锁 [%d]", (dev_cfg.lock_type==0)?0:1);
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_BOOL|ES_LM_1|ES_FT_L);
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009005")) == CMD_OK) {
                        edit_show_prompt(edit_norm, RGB_RED, "不支持彩转黑");
                        break;                    
                    /*ui_oper_mode = UI_INPUT_CAMERA_MODE;
                    sprintf(string, "彩转黑:0关闭,1开启 [%d]", (dev_cfg.camera_night==0)?0:1);
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_BOOL|ES_LM_1|ES_FT_L);*/
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009007")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_CVBS_MODE;
                    sprintf(string, "模拟视频:0自动,1优化 [%d]", (dev_cfg.cvbs_mode==0)?0:1);
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_BOOL|ES_LM_1|ES_FT_L);
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009009")) == CMD_OK) {
                    ui_scene_sw(SCENE_SET_ABOUT);
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00002012")) == CMD_OK) {
                    dev_cfg.lock_type = 0;  //防止恢复默认时重启时磁锁灯一直亮
                    //spi_set_locktype(LOCK_ELECTRIC)//del by wrm 20141121;
                    ui_unlock(0);
                    edit_show_prompt(edit_norm, RGB_GREEN, "正在恢复出厂设置...");
                    dev_flash_protect(0);
                    remove(DEV_CONFIG_PATH);
		      system("sync");
		      remove(DEV_SV_PATH);
                    system("sync");
                    dev_flash_protect(1);
                    dev_set_ip("192.168.14.252", "255.255.0.0", "192.168.14.254");
		      
                    //	system("/etc/init.d/S40network restart");
                    //edit_show_prompt(edit_norm, RGB_GREEN, "已恢复出厂设置,重启中...");
                    //dev_app_restart();

                    delete_card_unlock_ex(1,0);//****add
                    delete_info_controller();
                    delete_vlan_list();

                    delete_sfz8or4_cardreader_list();
		      
                    dev_sv_get();
                    dev_config_get();
                    sleep(1);
                    edit_show_prompt(edit_norm, RGB_GREEN, "已恢复出厂设置,重启中...");
                    usleep(800000);
                    key_buzz(BUZZ_CONFIRM);
                    dev_sys_reboot();
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008084")) == CMD_OK) {
                    ui_oper_mode = UI_INPUT_VOLUME_RX;
                    sprintf(string, "输入受话音量(00~99) [%d]", dev_cfg.level_vol_rx);
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_LM_2|ES_FT_L);
                }
                
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008083")) == CMD_OK) {
#if 0                
                    ui_oper_mode = UI_INPUT_VOLUME_TX;
                    sprintf(string, "输入送话音量(00~99) [%d]", dev_cfg.level_vol_tx);
                    edit_show_prompt(edit_norm, RGB_WHITE, string);
                    edit_set_style(edit_norm, ES_NUM|ES_LM_2|ES_FT_L);
#else
                    edit_show_prompt(edit_norm, RGB_RED, "当前版本不支持此操作");
                    break;   
#endif
                }
			/*else
		       if((ret == CMD_LIMIT)?0:(ret = ui_check_edit_cmd(edit_norm,"00002018"))== CMD_OK){ //add by mkq 20170907
		    Reset_Net();
                     printf("This Reset_Net \n");
		       }*/
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008869")) == CMD_OK) {
                    if (ATEST_PAUSE == atest_get_status()) {
                        atest_start(NULL);
                        edit_show_prompt(edit_norm, RGB_GREEN, "已启动自动呼叫测试!");
                        ui_oper_mode = UI_NORMAL;
                        sleep(1);
                        ui_scene_sw(SCENE_MAIN);
                    } else {
                        ui_oper_mode = UI_INPUT_AUTO_TARGET;
                        edit_show_prompt(edit_norm, RGB_WHITE, "自动呼叫:输入房号");
                        edit_set_style(edit_norm, ES_NUM|ES_LM_C|ES_FT_L);
                    }
                }
#ifndef FRONT_DOOR
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00009185")) == CMD_OK) {
                    dev_cfg.en_user_unlock = !dev_cfg.en_user_unlock;
                    dev_config_save();
                    if (dev_cfg.en_user_unlock)
                        edit_show_prompt(edit_norm, RGB_GREEN, "用户密码开锁开启");
                    else
                        edit_show_prompt(edit_norm, RGB_GREEN, "用户密码开锁关闭");
                }
                //add for multi_lift_ctl
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008072")) == CMD_OK) {
                    printf("set 2nd lift controller ip\n");
                    lift_ctrl_no = 0;
                    ui_scene_sw(SCENE_SET_LIFT_IP);
                }
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008073")) == CMD_OK) {
                    printf("set 3rd lift controller ip\n");
                    lift_ctrl_no = 1;
                    ui_scene_sw(SCENE_SET_LIFT_IP);
                }
                /*else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008074")) == CMD_OK) {
                    printf("set 4th lift controller ip\n");
                    lift_ctrl_no = 2;
                    ui_scene_sw(SCENE_SET_LIFT_IP);
                } 
                else
                if ((ret == CMD_LIMIT) ? 0 : (ret = ui_check_edit_cmd(edit_norm, "00008075")) == CMD_OK) {
                    printf("set 5th lift controller ip\n");
                    lift_ctrl_no = 3;
                    ui_scene_sw(SCENE_SET_LIFT_IP);
                } */
#endif

				if (ret == CMD_UNMATCH)
				{
					strcpy(string, dev_cfg.pw_public);
#ifndef FRONT_DOOR

					if (edit_get_length(edit_norm) == 8)
					{
						unsigned char dat[4];   //dat[0]层 [1]房 [2]密码高 [3]密码低

						if (dev_cfg.en_user_unlock) {
							edit_get_bcd(edit_norm, dat);
							ui_msg2proc(UI_SEND_AJB_DATA, SSCS, dat[0], dat[1], dat[2], dat[3]);
							gbl_data_set(&gbl_user_unlock, 1);
							snprintf(g_usr_pwd,sizeof(g_usr_pwd),"%02x%02x%02x%02x",dat[0], dat[1], dat[2], dat[3]);//add for photo upload

							if (ui_oper_mode != UI_WAIT_USER_UNLOCK)
							{ tim_set_event(TIME_1S(3), (tim_callback)ui_action, UI_USER_PW_UL_TIMEOUT, 0, TIME_DESTROY); }

							ui_oper_mode = UI_WAIT_USER_UNLOCK;
							edit_show_prompt(edit_norm, RGB_WHITE, "连接中，请稍候...");
						}
						else
						{ 
						    edit_show_prompt(edit_norm, RGB_RED, "用户密码开锁未启用!"); 
						}
					}
					else 
#endif					
					if (0 == ui_check_edit_pw(edit_norm, string))
					{ 
					    ui_action(UI_PB_PW_UNLOCK, 0);
                            
					}
				}

				break;
            }
            case UI_INPUT_PB_PW:            
                if (0 == ui_check_edit_pw(edit_norm, dev_cfg.pw_public))
                    ui_action(UI_PB_PW_UNLOCK, 0);
					upload_record_pc(PUBLIC_PASSWORD_UNLOCK_RECORD);
                break;
             case UI_INPUT_CIF_MODE:

                dev_cfg.cif_mode = atoi(text_input);
                dev_config_save();
                if (dev_cfg.cif_mode == 1)
                {
                    sprintf(string, "云对讲视频为5帧");                   
                   
                }
                else                {
                    sprintf(string, "云对讲视频为1帧");
                     
                    
                }
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                //ui_oper_mode = UI_NORMAL;

                    sleep(1);
                    buzz_play(BUZZ_CONFIRM); 
                    ui_scene_sw(SCENE_MAIN);                
               
                break; 
            case UI_INPUT_SB_PW:
                edit_get_bcd(edit_norm, village);

                ui_get_unlock_pw(village[0],village[1],village[2]);

                printf("This Device:%02X %02X %02X \n",village[0],village[1] ,village[2]);
		if(ui_oper_mode != UI_WAIT_CHECK_SB_PW)
		tim_set_event(TIME_1S(4), (tim_callback)ui_action, UI_WAIT_SB_PW_TIMEOUT, 0, TIME_DESTROY);
		ui_oper_mode = UI_WAIT_CHECK_SB_PW;
		edit_show_prompt(edit_norm, RGB_WHITE, "连接中，请稍候...");

            
            	 //if (0 == ui_check_edit_pw(edit_norm, dev_cfg.pw_public))
                    //ui_action(UI_PB_PW_UNLOCK, 0);
                break;

            
            case UI_INPUT_KB_PW:
                strcpy(string, dev_cfg.pw_manage);
                if (0 == ui_check_edit_pw(edit_norm, string)) {
                    manage = !manage;
                    if (manage)
                        edit_show_prompt(edit_norm, RGB_GREEN, "管理密令已开启");
                    else
                        edit_show_prompt(edit_norm, RGB_GREEN, "管理密令已关闭");
                    ui_oper_mode = UI_NORMAL;
                }
                break;
            case UI_INPUT_CODE:
                if (edit_get_length(edit_norm) < 6) {
                    edit_show_prompt(edit_norm, RGB_RED, "编码须为六位,请重输!");
                    break;
                }
                edit_get_bcd(edit_norm, room);
                if (room[0] == 0x00) {
                    edit_show_prompt(edit_norm, RGB_RED, "楼号不能为0,请重输!");
                    break;
                }
                if (!((dev_cfg.my_code[0] == room[0])&&(dev_cfg.my_code[1] == room[1])&&(dev_cfg.my_code[3] == room[2]))){
                    delete_sfz8or4_cardreader_list();
                 }
                dev_cfg.my_code[0] = room[0];
                dev_cfg.my_code[1] = room[1];
                dev_cfg.my_code[2] = 0x00;
                dev_cfg.my_code[3] = room[2];
#ifdef FRONT_DOOR
                if (dev_cfg.my_code[3] != 0x00) {  //大门口机无副门口
                    dev_cfg.my_code[3]  = 0x00;
                    edit_show_text(edit_norm, WG_DRAW, "%02X%02X%02X%02X", \
                    dev_cfg.my_code[0], dev_cfg.my_code[1], dev_cfg.my_code[2], dev_cfg.my_code[3]);
                    usleep(500*1000);
                }
#endif
                dev_code_cardreader();
                dev_config_save();
                edit_show_prompt(edit_norm, RGB_GREEN, "编码成功,正在重启...");
                dev_app_restart();
                //ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
                break;
            case UI_INPUT_KB_PW_OLD:
                strcpy(string, dev_cfg.pw_manage);
                if (PW_OK == ui_check_edit_pw(edit_norm, string)) {
                    ui_oper_mode = UI_INPUT_KB_PW_NEW;
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入管理密码(新)");
                }
                break;

            case UI_INPUT_VI_CODE: //delete
                if (edit_get_length(edit_norm) < 6) {
                    edit_show_prompt(edit_norm, RGB_RED, "编码须为六 位,请重输!");
                    break;
                }
                edit_get_bcd(edit_norm, village);
                if (village[0] == 0x00&&village[1] == 0x00&&village[2] == 0x00) {
                    edit_show_prompt(edit_norm, RGB_RED, "小区号不能全为0,请重输!");
                    break;
                	}
		 

#ifdef FRONT_DOOR
                if (dev_cfg.my_code[3] != 0x00) {  //大门口机无副门口
                    dev_cfg.my_code[3]  = 0x00;
                    edit_show_text(edit_norm, WG_DRAW, "%02X%02X%02X%02X", \
                    dev_cfg.my_code[0], dev_cfg.my_code[1], dev_cfg.my_code[2], dev_cfg.my_code[3]);
                    usleep(500*1000);
                }
#endif
               if(vs_cfg.has_set == 0){
               //memcpy(dev_cfg.my_village_code,village,4);
               memcpy(vs_cfg.my_village_code,village,4);
                //my_village_code_to_Ble();
                my_new_village_code_to_Ble();
                //dev_code_cardreader();
                dev_config_save();
                usleep(200);
		  //dev_config_get();
		  dev_sv_get();
                edit_show_prompt(edit_norm, RGB_GREEN, "编码成功");
                ui_get_village();	
               }
		 else
		 	edit_show_prompt(edit_norm, RGB_RED, "编码失败");
                ui_oper_mode = UI_NORMAL; 
                break;

            case UI_INPUT_KB_PW_NEW:
                if (edit_get_length(edit_norm) < 4) {
                    edit_show_prompt(edit_norm, RGB_RED, "密码须为四位,请重输!");
                    break;
                }
                strcpy(pw_kb_new, edit_get_text(edit_norm));
                ui_oper_mode = UI_INPUT_KB_PW_FIRM;
                edit_show_prompt(edit_norm, RGB_WHITE, "请确认管理密码(新)");
                break;
            case UI_INPUT_KB_PW_FIRM:
                if (edit_get_length(edit_norm) < 4) {
                    edit_show_prompt(edit_norm, RGB_RED, "密码须为四位,请重输!");
                    break;
                }
                if (0 == strcmp(pw_kb_new, edit_get_text(edit_norm))) {
                    strcpy(dev_cfg.pw_manage, pw_kb_new);
                    dev_config_save();
                    edit_show_prompt(edit_norm, RGB_GREEN, "管理密码已更改!");
                }
                else {
                    edit_show_prompt(edit_norm, RGB_RED, "两次新密码不相同!");
                }
                ui_oper_mode = UI_NORMAL;
                break;
            case UI_INPUT_CARD_FMT:
                if (edit_get_length(edit_norm) < 6) {
                    edit_show_prompt(edit_norm, RGB_RED, "规则为六位,请重输!");
                    break;
                }
                edit_get_bcd(edit_norm, &buf[4]);
                buf[0] = SFOR;
                buf[1] = dev_cfg.my_code[3];
                buf[2] = 0x00;
                buf[3] = 0x00;
                uart_send_data(UART_1, buf);
                usleep(500*1000);
                edit_show_prompt(edit_norm, RGB_GREEN, "门禁格式化中,哔声后完成");
                ui_oper_mode = UI_NORMAL;
                break;
            case UI_INPUT_CARD_DEL:
                if (edit_get_length(edit_norm) < 10) {
                    edit_show_prompt(edit_norm, RGB_RED, "卡号10位,不足前补0!");
                    break;
                }
                edit_get_bcd(edit_norm, &buf[2]);
                buf[0] = 0xDB;
                buf[1] = dev_cfg.my_code[0];
                uart_send_data(UART_1, buf);
                usleep(500*1000);
                edit_show_prompt(edit_norm, RGB_GREEN, "卡号已删除!");
                ui_oper_mode = UI_NORMAL;
                break;
            case UI_INPUT_BLE_DBM_TX:
                dev_cfg.level_ble_tx_dbm = atoi(text_input);
                if ((dev_cfg.level_ble_tx_dbm> 3)||(dev_cfg.level_ble_tx_dbm<1)){
			edit_show_prompt(edit_norm, RGB_RED, "输入1~3,请重新输入");
			break;
		  }
              dev_config_save();
		Set_SEL_S_M(1);//cpld_io_set(EIO_CARD_CODE_EN);
		printf("--------------set ble db\n");
		memset(buf, 0x00, sizeof(buf));
		buf[0] = 0xA1;
		buf[1] = dev_cfg.level_ble_tx_dbm;

		uart_send_data(UART_1, buf);		
              sprintf(string, "蓝牙信号强度等级%d", dev_cfg.level_ble_tx_dbm);
              edit_show_prompt(edit_norm, RGB_GREEN, string);
              ui_oper_mode = UI_NORMAL;
              break;
#ifdef RecallTime   
             case UI_INPUT_VOLUME_RecallTime:
                recall_time = atoi(text_input);
                if ((recall_time> 25) || (recall_time < 5)) {
                    edit_show_prompt(edit_norm, RGB_RED, "转呼时间须在05-25秒!");
                    key_buzz(BUZZ_WARNING); 
                    break;
                } 
                dev_cfg.recall_time = recall_time;
                dev_config_save();
                sprintf(string, "应答转呼时间为%d秒", dev_cfg.recall_time);
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                ui_oper_mode = UI_NORMAL;
                break;
#endif                
            case UI_INPUT_MC_DELAY://门磁报警第一次设置有效，时间到后会重新变为3min
                dev_cfg.delay_doorala = atoi(text_input);
                dev_config_save();
                gbl_data_set(&gbl_alarm_delay, TIME_1MIN(dev_cfg.delay_doorala));
                if (dev_cfg.delay_doorala)
                    sprintf(string, "门磁报警延时:%d分", dev_cfg.delay_doorala);
                else
                    sprintf(string, "门磁报警已关闭!");
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                ui_oper_mode = UI_NORMAL;
                break;
#ifdef CFG_USE_OLD_CARDREADER
            case UI_INPUT_CARD_NUM:
#ifdef FRONT_DOOR
                if (edit_get_length(edit_norm) < 8) {
                    edit_show_prompt(edit_norm, RGB_RED, "卡号为8位,请重新输入");
                    break;
                }

                buf[0] = SWRI;
                buf[1] = dev_cfg.my_code[0];
                buf[2] = 0x01;
                buf[3] = 0x01;
                buf[4] = 0x01;
                buf[5] = 0;
                buf[6] = dev_cfg.my_code[3];
#else
                if (edit_get_length(edit_norm) < 14) {
                    edit_show_prompt(edit_norm, RGB_RED, "格式有误,请重新输入");
                    break;
                }
                edit_get_bcd(edit_norm, room);
                if ((room[0] < 0x01) || (room[1] < 0x01) || (room[2] < 0x01)) {
                    edit_show_prompt(edit_norm, RGB_RED, "格式有误,请重新输入");
                    break;
                }

                buf[0] = SWRI;
                buf[1] = dev_cfg.my_code[0];
                buf[2] = room[0];
                buf[3] = room[1];
                buf[4] = room[2];
                buf[5] = 0;
                buf[6] = dev_cfg.my_code[3];
#endif
                ui_oper_mode = UI_WAIT_CARD_WRITE;
                uart_send_data(UART_1, buf);
                break;
#endif
		case UI_INPUT_NEW_CARD: //add by wrm 20150619
                if (edit_get_length(edit_norm) < 14) {
                    edit_show_prompt(edit_norm, RGB_RED, "格式有误,请重新输入");
                    break;
                }
		edit_get_bcd(edit_norm, room);
		 if(room[0] > 0x15){//****modify 22
                    edit_show_prompt(edit_norm, RGB_RED, "扇区号输入有误,请重新输入");
                    break;
                }
                buf[0] = SCCS;
                buf[1] = dev_cfg.my_code[0];
                buf[2] = dev_cfg.my_code[1];
                buf[3] = 0;
                buf[4] = dev_cfg.my_code[3];
                 buf[5] = (room[0]/16)*10+room[0]%16;          //扇区号
                buf[6] = 0;
		buf[7]=buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
		ui_oper_mode = UI_WAIT_NEW_CARD;
		uart_send_data(UART_1, buf);//设置卡头扇区
		usleep(1000);
	     	buf[0] = SCCP;
                buf[1] = room[1];
                buf[2] = room[2];
                buf[3] = room[3];
                buf[4] = room[4];
                buf[5] = room[5];;
                buf[6] = room[6];
		buf[7]=buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
		uart_send_data(UART_1, buf);//设置卡头密码
		ui_oper_mode = UI_WAIT_NEW_CARD;		
		ui_action(UI_UART1_GOT_CARD, SCCP);

		break;

	//format .....
	 case UI_INPUT_FINGER_FMT://by mkq finger
		     if (edit_get_length(edit_norm) < 6) {
                    edit_show_prompt(edit_norm, RGB_RED, "密码为六位,请重输!");
                    break;
                }
		  if( 0== strcmp(edit_get_text(edit_norm),"300155")){
		  	printf("------formt......gbl_finger_status = %d\n",gbl_finger_status.data);
		  	fp_status_set(status_non);
			usleep(50*1000);
			fp_empty_all();
			printf("send empty templet cmd\n");
			//usleep(50*1000); //commented out by hgj 立即设置标志位，以解决有时指纹清空失败的bug
		  	fp_status_set(status_format);
			ui_oper_mode = UI_NORMAL;
                	break;
		  }else{

		  	printf("formt password ERROR!\n");
			ui_action(UI_FINGER_FORMAT_PW_ERROR,0);
		  }
		  ui_oper_mode = UI_NORMAL;
		  break;

	 //end

	//delete id  .....
	    case UI_INPUT_FINGER_DELE_ID://by mkq finger
		     if (edit_get_length(edit_norm) < 4) {
                    edit_show_prompt(edit_norm, RGB_RED, "格式不正确");
                    break;
                }
		  edit_get_bcd(edit_norm,buffer);
                int test[3] = {0};
		  test[0] = (buffer[0] /16)*10+buffer[0]%16;
		  test[1] = (buffer[1] /16)*10+buffer[1]%16;
		  test[2] = test[0]*100+test[1];
		  printf("buffer :%02x %02x %02x %02x %d\n",buffer[0],buffer[1],test[0],test[1],test[2]);
		  if(fp_delete_char(test[0]*100+test[1],0x01) == -1 ){
		     printf("Out of range!\n");
		     fp_status_set(status_non);
                   ui_oper_mode = UI_INPUT_FINGER_DELE_ID;                 
		     edit_show_prompt(edit_norm, RGB_RED, "超出ID范围,请输入0~999");
		     break;
		  }
		  
		  fp_status_set(status_delect);
		  ui_oper_mode = UI_NORMAL;
		  break;

	//end

	 
            case UI_INPUT_LOCK_TYPE:
                dev_cfg.lock_type = atoi(text_input);
                dev_config_save();
                //spi_set_locktype(dev_cfg.lock_type); //del by wrm 20141121 for i2c
                if (dev_cfg.lock_type == 0) {
                    ui_oper_mode = UI_NORMAL;
                    edit_show_prompt(edit_norm, RGB_GREEN, "已选择电控锁");
                }
                else {
                    ui_oper_mode = UI_INPUT_LOCK_TIME;
                    edit_set_style(edit_norm, ES_NUM|ES_LM_2|ES_FT_L);
                    edit_show_prompt(edit_norm, RGB_WHITE, "请输入磁力锁延时(01~30)");
                }
                ui_unlock(0);
                break;
            case UI_INPUT_LOCK_TIME: {
                unsigned char lock_time = atoi(text_input);
                
                if ((lock_time > 30) || (lock_time < 1)) {
                    edit_show_prompt(edit_norm, RGB_RED, "延时须在1-30秒!");
                    break;
                } 
                dev_cfg.lock_time = lock_time;
                dev_config_save();
                sprintf(string, "已选择磁力锁(%d秒)", dev_cfg.lock_time);
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                ui_oper_mode = UI_NORMAL;
                }
                break;
            case UI_INPUT_CVBS_MODE:
                dev_cfg.cvbs_mode = atoi(text_input);
                dev_config_save();
                if (dev_cfg.cvbs_mode == VID_MAIN_LCD)
                    sprintf(string, "模拟视频效果为自动模式");
                else
                    sprintf(string, "模拟视频效果为优化模式");
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                ui_oper_mode = UI_NORMAL;
                video_main_mode_set(VID_MAIN_LCD);
                break;
            case UI_INPUT_CAMERA_MODE:
                dev_cfg.camera_night = atoi(text_input);
                dev_config_save();
                if (dev_cfg.camera_night == VID_MODE_DAY)
                    sprintf(string, "彩转黑功能已关闭");
                else
                    sprintf(string, "彩转黑功能已开启");
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                ui_oper_mode = UI_NORMAL;
                break;
            case UI_INPUT_VOLUME_RX:
                dev_cfg.level_vol_rx = atoi(text_input);
                if (dev_cfg.level_vol_rx > 99)
                    dev_cfg.level_vol_rx = 99;
                dev_config_save();
                sprintf(string, "受话音量已设为%d", dev_cfg.level_vol_rx);
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                ui_oper_mode = UI_NORMAL;
                break;
            case UI_INPUT_VOLUME_TX:
                dev_cfg.level_vol_tx = atoi(text_input);
                if (dev_cfg.level_vol_tx > 99)
                    dev_cfg.level_vol_tx = 99;
                dev_config_save();
                sprintf(string, "送话音量已设为%d", dev_cfg.level_vol_tx);
                edit_show_prompt(edit_norm, RGB_GREEN, string);
                ui_oper_mode = UI_NORMAL;
                break;
#ifdef APP_AUTO_TEST
            case UI_INPUT_AUTO_TARGET:
                if (edit_get_length(edit_norm) < ROOM_NUM) {
                    edit_show_prompt(edit_norm, RGB_RED, STR_ROOM_LESS);
                    break;
                }
                edit_get_bcd(edit_norm, room);
                atest_start(room);
#ifndef FRONT_DOOR
                room[2] = room[0];
                room[3] = room[1];
                room[0] = dev_cfg.my_code[0];
                room[1] = dev_cfg.my_code[1];
#endif
                ui_msg2proc(UI_SEND_AUTO_TEST, 1, room[0], room[1], room[2], room[3]);
                edit_show_prompt(edit_norm, RGB_GREEN, "已启动自动呼叫测试!");
                ui_oper_mode = UI_NORMAL;
                sleep(1);
                ui_scene_sw(SCENE_MAIN);
                break;
#endif
            case UI_WAIT_USER_UNLOCK:
                break;
	     case UI_WAIT_CHECK_SB_PW://add by wrm 20160105 must need
		  break;
            default:
                ui_oper_mode = UI_NORMAL;
                break;
        }
        if (ui_oper_mode == UI_NORMAL)
            edit_set_style(edit_norm, ES_PW|ES_LM_8|ES_FT_L);
        printf("ui_oper_mode:%d\n", ui_oper_mode);
    }
}

static void ui_proc_unlock(void)
{
    switch(key_code) {
    //    case KEY_F1: //add by wrm 20140104 C按键
    //        ui_scene_sw(SCENE_MAIN);
    //        break;
        default:
        ui_scene_sw(SCENE_MAIN);
            break;
    }
}

static void ui_proc_set_pw(void)
{
    char pw_str[7] = {'\0'};
    switch(key_code) {
        case 0xCA:
            edit_text_clr(edit_get_focus());
            break;
	case KEY_F1:
            if (!edit_is_empty(edit_get_focus()))
		edit_text_clr(edit_get_focus());
	    	
            else if (edit_is_empty(edit_get_focus())) {
                if (edit_get_focus() == edit_pw_firm) {
                    edit_set_focus(edit_pw_new);
                } 
                else
                if (edit_get_focus() == edit_pw_new) {
                    edit_set_focus(edit_pw_old);
                } else {
                    ui_scene_sw(SCENE_SET_SEL);
                }
            }
            break;		
        case KEY_X:
            if (edit_is_empty(edit_get_focus())) {
                if (edit_get_focus() == edit_pw_firm) {
                    edit_set_focus(edit_pw_new);
                } 
                else
                if (edit_get_focus() == edit_pw_new) {
                    edit_set_focus(edit_pw_old);
                } else {
                    ui_scene_sw(SCENE_SET_SEL);
                }
            }
            break;
        case KEY_Y:
            if (edit_get_focus() == edit_pw_old) {
                strcpy(pw_str, dev_cfg.pw_public);
                if (PW_OK == ui_check_edit_pw(edit_pw_old, pw_str))
                    edit_set_focus(edit_pw_new);
            }
            else
            if (edit_get_focus() == edit_pw_new) {
                edit_set_focus(edit_pw_firm);
            }
           /* else {//del by wrm  20150302 确定用OK键代替
                strcpy(pw_str, dev_cfg.pw_public);
                if (PW_OK != ui_check_edit_pw(edit_pw_old, pw_str))
                    break;
                if (edit_get_length(edit_pw_new) < PB_UL_PW_NUM) {
                    edit_show_prompt(edit_pw_new, RGB_RED, STR_PW_LESS);
                }
                else
                if (edit_get_length(edit_pw_firm) < PB_UL_PW_NUM) {
                    edit_show_prompt(edit_pw_firm, RGB_RED, STR_PW_LESS);
                }
                else
                if (0 != strcmp(edit_get_text(edit_pw_new), edit_get_text(edit_pw_firm))) {
                    edit_show_prompt(edit_pw_firm, RGB_RED, STR_PW_FIRM_ERR);
                }
                else {
                    strcpy(dev_cfg.pw_public, edit_get_text(edit_pw_new));
                    dev_config_save();
                    edit_show_prompt(edit_pw_old,  RGB_GREEN, STR_PW_SET_OK);
                    ui_delay_return_set_sel(TIME_500MS(1));
                }
            }*/
            break;
	case KEY_F3://add by wrm 20150302 OK按键
            if (edit_get_focus() == edit_pw_firm) {
                strcpy(pw_str, dev_cfg.pw_public);
                if (PW_OK != ui_check_edit_pw(edit_pw_old, pw_str))
                    break;
                if (edit_get_length(edit_pw_new) < PB_UL_PW_NUM) {
                    edit_show_prompt(edit_pw_new, RGB_RED, STR_PW_LESS);
                }
                else
                if (edit_get_length(edit_pw_firm) < PB_UL_PW_NUM) {
                    edit_show_prompt(edit_pw_firm, RGB_RED, STR_PW_LESS);
                }
                else
                if (0 != strcmp(edit_get_text(edit_pw_new), edit_get_text(edit_pw_firm))) {
                    edit_show_prompt(edit_pw_firm, RGB_RED, STR_PW_FIRM_ERR);
                }
                else {
                    strcpy(dev_cfg.pw_public, edit_get_text(edit_pw_new));
                    dev_config_save();
                    edit_show_prompt(edit_pw_old,  RGB_GREEN, STR_PW_SET_OK);
                    ui_delay_return_set_sel(TIME_500MS(1));
                    }		
		}
	    break;
        //case KEY_F1:
            //ui_scene_sw(SCENE_SET_SEL);
           // ui_scene_sw(SCENE_MAIN); //add by wrm 20140104 C按键
           // break;
        default:
            break;
    }
}

static void ui_proc_set_screen(void)
{
    switch(key_code) {
        case KEY_4:
	   // button_release(bt_bright_inc);
	   // button_press(bt_bright_dec);
            slider_dec(slider_bright);
            break;
        case KEY_6:
	 //   button_release(bt_bright_dec);
	 //   button_press(bt_bright_inc);			
            slider_inc(slider_bright);
            break;
        case KEY_2:
            if (radio_min_5 == radio_get_select())  
                radio_set_select(radio_min_1);
            else
            if (radio_min_1 == radio_get_select())
                radio_set_select(radio_min_0);
            else
                radio_set_select(radio_min_5);
            break;
        case KEY_8:
            if (radio_min_0 == radio_get_select())  
                radio_set_select(radio_min_1);
            else
            if (radio_min_1 == radio_get_select())
                radio_set_select(radio_min_5);
            else
                radio_set_select(radio_min_0);
            break;
       case KEY_F1:			
        case KEY_X:
            ui_scene_sw(SCENE_SET_SEL);
            break;
        case KEY_F3://add by wrm 20150302 OK按键
            dev_cfg.level_bright = slider_get_tic(slider_bright);
            if (radio_min_0 == radio_get_select())  dev_cfg.level_ss_tim = 0;
            else
            if (radio_min_1 == radio_get_select())  dev_cfg.level_ss_tim = 1;
            else                                    dev_cfg.level_ss_tim = 2;
            dev_config_save();
            //text_show(text_font, ENCODING, RGB_GREEN, 28, 28, 0, 0, 470, 78, "设置成功!");
            text_show(text_font, ENCODING, RGB_GREEN, 20, 20, 0, 0, 200, 120, "设置成功!");
            ui_delay_return_set_sel(TIME_500MS(1));
            break;
       //case KEY_F1:
            //ui_scene_sw(SCENE_SET_SEL);
           // ui_scene_sw(SCENE_MAIN); //add by wrm 20140104 C按键
          //  break;
    }
}

static void ui_proc_set_volumn(void)
{
    unsigned char vol_level = 0;
    switch(key_code) {
        case KEY_4:
	    //button_release(bt_volumes_inc);
	    //button_press(bt_volumes_dec);
            slider_dec(slider_volumes);
            vol_level = slider_get_tic(slider_volumes);
            if (vol_level < 16)
                audio_play(PMT_TEST, DEV_VOL_TEST(vol_level));
            break;
        case KEY_6:
	    //button_release(bt_volumes_dec);
	   // button_press(bt_volumes_inc);			
            slider_inc(slider_volumes);
            vol_level = slider_get_tic(slider_volumes);
            if (vol_level < 16)
                audio_play(PMT_TEST, DEV_VOL_TEST(vol_level));
            break;
        case KEY_5:
            audio_stop();
            break;
        case KEY_F1:			
        case KEY_X:
            audio_stop();
            ui_scene_sw(SCENE_SET_SEL);
            break;
        case KEY_F3://add by wrm 20150302 OK按键
            text_show(text_font, ENCODING, RGB_GREEN, 20, 20, 0, 0, 120, 160-10, "设置成功!");
            audio_stop();
            dev_cfg.level_volume = slider_get_tic(slider_volumes);
            if (dev_cfg.level_volume > 15) 
                dev_cfg.level_volume = 15;
            
            //text_show(text_font, ENCODING, RGB_GREEN, 20, 20, 0, 0, 100, 160, "设置成功!");
            ui_delay_return_set_sel(TIME_500MS(1));
            return;
        //case KEY_F1:
            //ui_scene_sw(SCENE_SET_SEL);
            //ui_scene_sw(SCENE_MAIN); //add by wrm 20140104 C按键
            break;
    }
}

static void ui_proc_set_time(void)
{
    time_t now;
    struct tm *tnow;
    char now_date[20] = {'\0'};
    char now_time[20] = {'\0'};
    char str_cmd[40]  = {'\0'};
    int  cmd_status   = 0;
    int  set_status   = 0;

    switch(key_code) {
        case 0xCA:
            edit_text_clr(edit_get_focus());
            break;
	case KEY_F1:		
            if (!edit_is_empty(edit_get_focus()))
		edit_text_clr(edit_get_focus());			
            else if (edit_is_empty(edit_get_focus())) {
                if (edit_get_focus() == edit_time) {
                    edit_set_focus(edit_date);
                }
                else {
                    ui_scene_sw(SCENE_SET_SEL);
                }
            }
            break;
	case KEY_X:
            if (edit_is_empty(edit_get_focus())) {
                if (edit_get_focus() == edit_time) {
                    edit_set_focus(edit_date);
                } 
	    else {
                    ui_scene_sw(SCENE_SET_SEL);
                }
            }
	break;
        case KEY_Y:
            if (edit_want_return(edit_get_focus())) {
                if (edit_get_focus() == edit_date) {
                    edit_set_focus(edit_time);
                } /*else {//del by wrm  20150302 确定用OK键代替
                    time(&now);
                    tnow = localtime(&now);
                    sprintf(now_date, "%4d-%02d-%02d", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday);
                    sprintf(now_time, "%02d:%02d:%02d", tnow->tm_hour, tnow->tm_min, tnow->tm_sec);
                    strcpy(str_cmd, "date -s '");
                    if (edit_get_length(edit_date) > 0) {
                        if (ui_check_edit_dt(edit_date, DT_DATE)) break;
                        strcat(str_cmd, edit_get_text(edit_date));
                        strcat(str_cmd, " ");
                        strcat(str_cmd, now_time);
                        strcat(str_cmd, "'");
                        cmd_status = system(str_cmd);
                        //printf("cmd_status :%d \n", WEXITSTATUS(cmd_status));
                        if (WEXITSTATUS(cmd_status) != 0) {
                            edit_show_prompt(edit_date, RGB_RED, "日期有误,请重输!");
                            set_status = -1;
                        }
                    }
                    strcpy(str_cmd, "date -s '");
                    if (set_status == 0) {
                        if (edit_get_length(edit_time) > 0) {
                            if (ui_check_edit_dt(edit_time, DT_TIME)) break;
                            if (edit_get_length(edit_date) > 0)
                                strcat(str_cmd, edit_get_text(edit_date));
                            else
                                strcat(str_cmd, now_date);
                            strcat(str_cmd, " ");
                            strcat(str_cmd, edit_get_text(edit_time));
                            strcat(str_cmd, "'");
                            cmd_status = system(str_cmd);
                            if (WEXITSTATUS(cmd_status) != 0) {
                                edit_show_prompt(edit_time, RGB_RED, "时间有误,请重输!");
                                set_status = -1;
                            }
                        }
                    }
                    system("hwclock -w");
                    if (set_status == 0) {
                        if ((edit_get_length(edit_date) == 0) && (edit_get_length(edit_time) == 0)) {
                            text_show(text_font, ENCODING, RGB_RED, 20, 20, 0, 0, 112, 160, "设置未更改!");
                            ui_delay_return_set_sel(TIME_500MS(1));
                        }
                        else {
                            text_show(text_font, ENCODING, RGB_GREEN, 20, 20, 0, 0, 116, 160, "设置成功!");
                            ui_delay_return_set_sel(TIME_500MS(1));
                            ui_action(UI_STBAR_REFRESH, 0);
                        }
                    }
                }*/
            }
            break;
        case KEY_F3://add by wrm 20140104 OK按键
	    if (edit_get_focus() == edit_time) {
		    time(&now);
                    tnow = localtime(&now);
                    sprintf(now_date, "%4d-%02d-%02d", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday);
                    sprintf(now_time, "%02d:%02d:%02d", tnow->tm_hour, tnow->tm_min, tnow->tm_sec);
                    strcpy(str_cmd, "date -s '");
                    if (edit_get_length(edit_date) > 0) {
                        if (ui_check_edit_dt(edit_date, DT_DATE)) break;
                        strcat(str_cmd, edit_get_text(edit_date));
                        strcat(str_cmd, " ");
                        strcat(str_cmd, now_time);
                        strcat(str_cmd, "'");
                        cmd_status = system(str_cmd);
                        //printf("cmd_status :%d \n", WEXITSTATUS(cmd_status));
                        if (WEXITSTATUS(cmd_status) != 0) {
                            edit_show_prompt(edit_date, RGB_RED, "日期有误,请重输!");
                            set_status = -1;
                        }
                    }
                    strcpy(str_cmd, "date -s '");
                    if (set_status == 0) {
                        if (edit_get_length(edit_time) > 0) {
                            if (ui_check_edit_dt(edit_time, DT_TIME)) break;
                            if (edit_get_length(edit_date) > 0)
                                strcat(str_cmd, edit_get_text(edit_date));
                            else
                                strcat(str_cmd, now_date);
                            strcat(str_cmd, " ");
                            strcat(str_cmd, edit_get_text(edit_time));
                            strcat(str_cmd, "'");
                            cmd_status = system(str_cmd);
                            if (WEXITSTATUS(cmd_status) != 0) {
                                edit_show_prompt(edit_time, RGB_RED, "时间有误,请重输!");
                                set_status = -1;
                            }
                        }
                    }

                    if (set_status == 0) {
                        if ((edit_get_length(edit_date) == 0) && (edit_get_length(edit_time) == 0)) {
                            text_show(text_font, ENCODING, RGB_RED, 20, 20, 0, 0, 112, 160, "设置未更改!");
                            ui_delay_return_set_sel(TIME_500MS(1));
                        }
                        else {
                            text_show(text_font, ENCODING, RGB_GREEN, 20, 20, 0, 0, 116, 160, "设置成功!");
                            ui_delay_return_set_sel(TIME_500MS(1));
                            ui_action(UI_STBAR_REFRESH, 0);
                            time(&now);
                            //save_local_time(now);
                            tnow = localtime(&now);
                            set_rtc_time(*tnow);                            
                        }
                    }

		}
	    break;		
        //case KEY_F1:
            //ui_scene_sw(SCENE_SET_SEL);
           // ui_scene_sw(SCENE_MAIN); //add by wrm 20140104 C按键
          //  break;
    }
}

static void ui_proc_set_ip(void)
{
    int ret;
    switch(key_code) {
        case 0xCA:
            edit_text_clr(edit_get_focus());
            break;
	case KEY_F1:
            if (!edit_is_empty(edit_get_focus()))
		edit_text_clr(edit_get_focus());
	    	
            else if (edit_is_empty(edit_get_focus())) {
                if (edit_get_focus() == edit_ip_gate) {
                    edit_set_focus(edit_ip_addr);
                } 
                else
                if (edit_get_focus() == edit_ip_mask) {
                    edit_set_focus(edit_ip_gate);
                } else {
                    ui_scene_sw(SCENE_SET_SEL);
                }
            }
            break;			
        case KEY_X:
            if (edit_is_empty(edit_get_focus())) {
                if (edit_get_focus() == edit_ip_gate) {
                    edit_set_focus(edit_ip_addr);
                } 
                else
                if (edit_get_focus() == edit_ip_mask) {
                    edit_set_focus(edit_ip_gate);
                } else {
                    ui_scene_sw(SCENE_SET_SEL);
                }
            }
            break;
        case KEY_Y:
            if (edit_want_return(edit_get_focus())) {
                if (edit_get_focus() == edit_ip_addr) {
                    edit_set_focus(edit_ip_gate);
                }
                else
                if (edit_get_focus() == edit_ip_gate) {
                    if (0 == edit_get_length(edit_ip_mask)) {
                        edit_text_fill(edit_ip_mask, "255.255.0.0");
                        edit_text_show(edit_ip_mask);
                    }
                    edit_set_focus(edit_ip_mask);
                }
                /*else { //del by wrm  20150302 确定用OK键代替
                    if (ui_check_edit_ip(edit_ip_addr, IP_TYPE_PRIV))
                        break;
                    if (ui_check_edit_ip(edit_ip_mask, IP_TYPE_MASK) || ui_check_edit_ip(edit_ip_gate, IP_TYPE_NORM))
                        break;
                    ret = dev_set_ip(edit_get_text(edit_ip_addr), edit_get_text(edit_ip_mask), edit_get_text(edit_ip_gate));
                    if (ret == 0) {
                        text_show(text_font, ENCODING, RGB_GREEN, 20, 20, 0, 0, 80, 180, "设置成功, 重启中...");
                       	system("/etc/init.d/S40network restart");
                        dev_app_restart();
                        //dev_sys_reboot();
                    }
                    else
                    if (ret == IPSET_ERR_HOST) {
                        edit_show_prompt(edit_ip_addr, RGB_RED, "IP与掩码组合无效!");
                        break;
                    }
                    else {
                        text_show(text_font, ENCODING, RGB_RED, 20, 20, 0, 0, 90, 200, "配置文件写入失败!");
                    }
                    ui_delay_return_set_sel(TIME_500MS(1));
                }*/
            }
            break;
	case KEY_F3://add by wrm 20140104 OK按键
            if (edit_get_focus() == edit_ip_mask) {
		    if (ui_check_edit_ip(edit_ip_addr, IP_TYPE_PRIV))
                        break;
                    if (ui_check_edit_ip(edit_ip_mask, IP_TYPE_MASK) || ui_check_edit_ip(edit_ip_gate, IP_TYPE_NORM))
                        break;
                    ret = dev_set_ip(edit_get_text(edit_ip_addr), edit_get_text(edit_ip_mask), edit_get_text(edit_ip_gate));
                    if (ret == 0) {
                        text_show(text_font, ENCODING, RGB_GREEN, 15, 15, 0, 0, 100, 50, "设置成功, 重启中...");
                    	system("/etc/init.d/S40network restart");
                        dev_app_restart();
                        //dev_sys_reboot();
                    }
                    else
                    if (ret == IPSET_ERR_HOST) {
                        edit_show_prompt(edit_ip_addr, RGB_RED, "IP与掩码组合无效!");
                        break;
                    }
                    else {
                        text_show(text_font, ENCODING, RGB_RED, 20, 20, 0, 0, 90, 200, "配置文件写入失败!");
                    }
                    ui_delay_return_set_sel(TIME_500MS(1));
		}	
	    break;
        //case KEY_F1:
            //ui_scene_sw(SCENE_SET_SEL);
           // ui_scene_sw(SCENE_MAIN); //add by wrm 20140104 C按键
           // break;
        default:
            //ui_scene_sw(SCENE_INPUT_PW);
            break;
    }
}

static void ui_proc_set_about(void)
{
    ui_scene_sw(SCENE_SET_SEL);
}

static void ui_proc_txt_detail(void)
{

}

static void ui_proc_pic_display(void)
{
    if (ui_oper_mode == UI_SCREENSAVE) {
        ui_scene_sw(SCENE_MAIN);
    }
        
    else {
        ui_scene_sw(SCENE_PICTURE);
    }

    ui_oper_mode = UI_NORMAL;
}

//add for multi_lift_ctl
static void ui_proc_set_lift_ip(void)
{
    char string[50]={0};
    //printf("%s running\n",__func__);
    switch(key_code)
    {
    case KEY_F1:
        if (!edit_is_empty(edit_get_focus())){
            edit_text_clr(edit_get_focus());
        }
        else if(edit_is_empty(edit_get_focus())){
            ui_scene_sw(SCENE_MAIN);
        }
        break;
        
    case KEY_X:
        if (edit_is_empty(edit_get_focus())) {
            ui_scene_sw(SCENE_MAIN);
        }
        break;
        
    case KEY_F3:
        if (edit_want_return(edit_get_focus())) {
            char *ip_str = edit_get_text(edit_norm);
            printf("ip_str= %s\n",ip_str);
    	    if (ui_check_edit_ip(edit_norm, IP_TYPE_DEST))
            {break;} 
            struct in_addr  in_addr_ip;
            inet_aton(ip_str,&in_addr_ip);
            printf("set lift_controller ip:%08x\n",in_addr_ip.s_addr); 
            memcpy(&dev_cfg.lift_ip[lift_ctrl_no][0],&in_addr_ip.s_addr,4);

            dev_config_save();
            sprintf(string, "%d号梯控IP设置成功!", lift_ctrl_no+2);
            edit_show_prompt(edit_norm, RGB_GREEN, string);
            sleep(1);
            ui_scene_sw(SCENE_MAIN);
        } 
        break;
    }
}
static void ui_proc_lcd_off(void)
{
    ui_scene_sw(SCENE_MAIN);
    ui_action(UI_LCD_TURN_ON,  0);
}

static void ui_proc_set_fingre(void)
{
	switch(key_code)
	{
        case KEY_X:
        case KEY_F1:
            printf("ui_proc_set_fingre:%c (%d)\n",key_code,key_code);
            ui_scene_sw(SCENE_MAIN);
            break;

        default:
            break;
	}	
}
static void ui_proc_fingre_unlock(void){//by mkq finger
	switch(key_code) {
		default:
		   ui_scene_sw(SCENE_MAIN);
		break;
		}

}
static void ui_proc_fingre_no_unlock(void){//by mkq finger
	switch(key_code) {
		default:
		   ui_scene_sw(SCENE_MAIN);
		break;
		}
}


/*按下任意键就切换到SCENE_MAIN*/
static void ui_proc_face_detect(void)
{ 

    ui_scene_sw(SCENE_MAIN);
    ui_exit_face_det();
  
}


void (*const ui_proc_list[])() = {
    NULL,
    ui_proc_main,
    ui_proc_message,
    ui_proc_picture,
    ui_proc_setting,
    ui_proc_help,
    ui_proc_call,
    ui_proc_calling,
    ui_proc_no_answer,
    ui_proc_talking,
    ui_proc_set_sel,
    ui_proc_input_pw,
    ui_proc_unlock,
    ui_proc_set_pw,
    ui_proc_set_screen,
    ui_proc_set_volumn,
    ui_proc_set_time,
    ui_proc_set_ip,
    ui_proc_set_about,
    ui_proc_txt_detail,
    ui_proc_pic_display,
    ui_proc_catalog,
    ui_proc_lcd_off,
    NULL,
    ui_proc_face_detect, 
    ui_proc_set_fingre,//by mkq finger
    ui_proc_fingre_unlock,
    ui_proc_fingre_no_unlock,//end     
    ui_proc_set_lift_ip,//add for multi_lift_ctl
    NULL,
};

int get_face_det_flag(void)
{
    return face_det_flag;
}

int close_face_det(int a,int b)
{
    app_debug(DBG_INFO,"%s running!\n",__func__);
    if(scene_cur == SCENE_FACE_DETECT)
    {    
    #ifdef _KEEP_OPEN 
        printf("[IR] time up,confirm visitor's leave\n");
        face_det_flag = 0;
        ui_action(UI_FACE_IO_CTRL,0);
    #else
        ui_action(UI_CLOSE_FACE_DET,0);
    #endif
    }
}

int ui_exit_face_det(void)
{

    tim_suspend_event(tim_face_det_timeout);//仅在页面切换时才用此项
    face_det_stop();
    Set_CCD_Light(0);
#ifdef _KEEP_OPEN
    ui_lcd_on();
    body_sense_result = 0;
    face_det_flag = 0;
#endif

}

static void *ui_thread(void *arg)
{
    unsigned char  msg[MSG_LEN_MAX];
    char key_down;
    char stringid[20]={'\0'};//by mkq finger
    char usr_id[10]={0}; //add for photo upload   

    scene_cur = SCENE_LCD_OFF;
    scene_pre = SCENE_NONE;
  //  spi_set_locktype(dev_cfg.lock_type); //del by wrm 20141121 for i2c
    ui_unlock(0);

    text_font = text_font_load(FONT_FILE);
    ui_make_background(dev_cfg.bg_pic);
    ui_widget_create();
    //system("echo 10240 > /proc/sys/vm/min_free_kbytes");
    //system("echo 1 > /proc/sys/vm/drop_caches");
    target[0] = dev_cfg.my_code[0];
    target[1] = dev_cfg.my_code[1];
    edit_flash_init((tim_callback)ui_action, UI_CURSOR_FLASH, 0);
    tim_count_down    = tim_set_event(TIME_1S(0), (tim_callback)ui_action, UI_TM_COUNT_DOWN,  0, TIME_PERIODIC);  
    tim_scene_timeout = tim_set_event(TIME_1S(0), (tim_callback)ui_scene_timeout, 0,          0, TIME_ONESHOT);
#ifdef TIME_SHOW_SEC
    tim_refresh_stbar = tim_set_event(TIME_1S(1), (tim_callback)ui_action, UI_STBAR_REFRESH,  0, TIME_PERIODIC);
#else
    tim_refresh_stbar = tim_set_event(TIME_1S(5), (tim_callback)ui_action, UI_STBAR_REFRESH,  0, TIME_PERIODIC);
#endif
    tim_face_det_timeout = tim_set_event(TIME_1S(5), (tim_callback)close_face_det, 0,  0, TIME_ONESHOT);
    tim_suspend_event(tim_face_det_timeout);
    
    key_buzz(BUZZ_CONFIRM); //add by wrm 20141121 for running beep
    
    while(1) {
        ui_scene_show(); 
        if (pipe_get(ui_pipe_ui, msg, MSG_LEN_UI) < 0) {
            app_debug(DBG_WARNING, "ui pipe get error\n");
            usleep(100*1000);
            continue;
        }
        
        //app_debug(DBG_INFO, "ui get msg:\t%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
        if (msg[1] == MSG_UI_ACTION) {
            switch (msg[2]) {
		case UI_CALL_SHOW:
            		printf("---------------UI_CALL_SHOW----------------\n"); 	
                    try_call_ser=1;
                    count_down_time=61;   
                    label_clear(&label_unlock);
                    label_clear(&label_talking_no_unlock);//by mkq finger
                    label_show_text(&label_cloud_show, WG_DRAW,RGB_YELLOW, "正在为您转接业主电话");			
			break;

                case UI_SCENE_REDRAW: //强制刷新SCENE
#ifndef UI_MEM_SEMISTATIC
                    /* 释放上一scene内存 */
#endif  
                    if (msg[3] == scene_cur) {
                        scene_pre = SCENE_NONE;
                        ui_scene_sw(scene_cur);
                    }
                    break;
                case UI_FORCE_SWITCH:
                    scene_pre = SCENE_NONE;
                    ui_scene_sw(msg[3]);
                    break;
                case UI_SCENE_SWITCH:
                    //可能在等待返回设置选择页面时收到刷卡开锁
                    if ((msg[3] == SCENE_SET_SEL) && (scene_cur == SCENE_UNLOCK))
                        break;
                    //对讲界面须按序
                    else
                    if ((msg[3] == SCENE_TALKING) && (scene_cur != SCENE_CALLING))
                        break;
                    ui_scene_sw(msg[3]);
                    break;
                case UI_CURSOR_RESUME:
                    edit_flash_resume();
                    break;
                case UI_CURSOR_FLASH:
                    if (0 == edit_cursor_status())
                        edit_flash_resume();
                    edit_flash_cursor();
                    break;
#ifdef CFG_USE_OLD_CARDREADER
                case UI_UART1_GOT_DATA:
                    if ((ui_oper_mode == UI_WAIT_CARD_WRITE) && (msg[3] == SSOK)) {
                        unsigned char data[10];
                        unsigned char buf[10];
                        ui_oper_mode =  UI_NORMAL;
#ifdef FRONT_DOOR
                        edit_get_bcd(edit_norm, &data[3]);
#else
                        edit_get_bcd(edit_norm, data);
#endif
                        buf[0] = SIC;
                        buf[1] = dev_cfg.my_code[0];
                        buf[2] = 0x00;
                        buf[3] = data[3];
                        buf[4] = data[4];
                        buf[5] = data[5];
                        buf[6] = data[6];
                        uart_send_data(UART_1, buf);
                        usleep(100*1000);
                        edit_show_prompt(edit_norm, RGB_GREEN, "注册完成!"); 
                        edit_set_style(edit_norm, ES_PW|ES_LM_8|ES_FT_L);
                    }
                    break;
#endif
		   case UI_UART1_GOT_CARD://****新卡扇区、密码注册
                    if ((ui_oper_mode == UI_WAIT_NEW_CARD) && (msg[3] == SCCP)) {
			   key_buzz(BUZZ_CONFIRM);
                        edit_show_prompt(edit_norm, RGB_GREEN, "扇区及密码注册完成!");
                        ui_oper_mode =  UI_NORMAL;	
			    edit_set_style(edit_norm, ES_PW|ES_LM_8|ES_FT_L);      
                    }
			break;
                case UI_NET_LINK_STAT:
                    net_link_status = msg[3];
                case UI_STBAR_REFRESH:
                    statusbar_show_canvas();
                    break;
                case UI_TM_COUNT_DOWN: 
		    //printf("calling_unlock:%d,cout_down:%d,is_calling_unlock:%d\n",calling_unlock_time,count_down_time,is_calling_unlock);
                    if (scene_cur == SCENE_PIC_DETAIL) {
                        if (ui_oper_mode == UI_SCREENSAVE)
                           #ifdef ADD_UNLOCK_AD
                            list_auto_unlock_ad_show(list_unlock_ad);
                            #else
                            list_auto_slide_show(list_pic);
                            #endif
                    }

                    if (count_down_time <= 0) break;
                    if (--count_down_time == 0) 
                        tim_suspend_event(tim_count_down);

                    if (talking_unlock_time - count_down_time >= 3) {
                        label_clear(&label_talking_unlock);
                        label_clear(&label_noanswer_unlock);
			label_clear(&label_leave_unlock);
			//by mkq finger
			label_clear(&label_noanswer_no_unlock);
			if(scene_cur ==SCENE_TALKING||(scene_cur == SCENE_NO_ANSWER && ui_oper_mode == UI_LEAVING_MSG))
			       label_clear(&label_talking_no_unlock);
			//end
                    }

                    if (scene_cur == SCENE_TALKING) {
				 //label_show_text(&label_count_down1, WG_DRAW, RGB_YELLOW, "%04d", count_down_time);//add by mkq 20170907
				       label_show_text(&label_count_down, WG_DRAW, RGB_YELLOW, "%02d", count_down_time);
                    }
                    else
                    if (scene_cur == SCENE_CALLING) {   
                       if(--calling_unlock_time > 0){
			     if(calling_fp_unlock_faild_time == 0x00){
                        label_clear(&label_cloud_show);
			     label_clear(&label_talking_no_unlock);//by mkq finger
                        label_show_text(&label_unlock, WG_DRAW, RGB_YELLOW, "%s", "门锁已开，请进!");
                    }
			else{//by mkq finger
				label_clear(&label_unlock);
				label_show_text(&label_talking_no_unlock, WG_DRAW, RGB_RED, "%s", "未发现匹配指纹");//zh20150604
			}
                       	}
                       if(calling_unlock_time==0){
                            label_clear(&label_unlock);
				  label_clear(&label_talking_no_unlock);// by mkq finger
                            label_show_text(&label_prompt, WG_DRAW, RGB_YELLOW, "呼叫剩余时间:%02d", count_down_time); 
                            if(try_call_ser==1)
                                label_show_text(&label_cloud_show, WG_DRAW, RGB_YELLOW, "%s", "正在为您转呼业主电话");
                       }
			else{
				is_calling_unlock = 0;
				//usleep(300*1000);
                        	label_show_text(&label_prompt, WG_DRAW, RGB_YELLOW, "呼叫剩余时间:%02d", count_down_time); 
				}								
                    }
                    else                    
                    if (scene_cur == SCENE_NO_ANSWER) {
                        if (ui_oper_mode != UI_LEAVING_MSG) break;
                        if (count_down_time == 0) {
                            ui_msg2proc(UI_SEND_LEAVE_MSG, 0, target[0], target[1], target[2], target[3]);
                            ui_scene_sw(SCENE_MAIN);//ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
                        }
                        label_clear(&label_noanswer_unlock);			
                        label_show_text(&label_prompt, WG_DRAW, RGB_YELLOW, "留言剩余时间:%02d", count_down_time);
                    }
                    else
                    if (scene_cur == SCENE_CALL) {
                        if (count_down_time == 0) {
#ifndef FRONT_DOOR
                            if (edit_get_length(edit_norm) == (ROOM_NUM-1)) {
                                unsigned char room[8];
                                char text[16]={'\0'};
                                strcpy(text, "0");
                                strcat(text, edit_get_text(edit_norm));
                                edit_show_text(edit_norm, WG_DRAW, "%s", text);
                                usleep(800*1000);
                                edit_get_bcd(edit_norm, room);
                                memcpy(&room[2], room, 2); 
                                memcpy(room, dev_cfg.my_code, 2); 
                                ui_make_call(room[0], room[1], room[2], room[3]);
                                ui_scene_sw(SCENE_CALLING);
                            }
#endif
                        }
                    }
                  
                    break;
               case UI_CALLING_GOTASK:
                    if (scene_cur == SCENE_CALLING) {
                        if(try_call_ser==1){             //终端不在线，转呼
                        //printf("3333333333333333-------------\n");
                        count_down_time = 60;
                        }else{                                 //终端在线
                        //printf("555555555555555555------------\n");
                        count_down_time = 30;
                        }
                        label_show_text(&label_prompt, WG_DRAW, RGB_YELLOW, "呼叫剩余时间:%02d", count_down_time);
                        tim_reset_time(tim_count_down, TIME_1S(1));
                    }
                    break;
                case UI_CALLING_FAIL:
                    if (scene_cur != SCENE_CALLING) break;
                    label_clear(&label_prompt);
			label_clear(&label_cloud_show);
                    if (msg[3] == TARGET_BUSY)
                        //text_show(text_font, ENCODING, RGB_RED, 16, 16, 0, 0, 100-25, 140, "对方繁忙，请稍后再呼！");
			label_show_text(&label_calling_promot, WG_DRAW, RGB_RED, "%s", "对方繁忙，请稍后再呼！");		
                    else
                        //text_show(text_font, ENCODING, RGB_RED, 16, 16, 0, 0, 100-25, 140, "设备不在线，呼叫失败！");
                        label_show_text(&label_calling_promot, WG_DRAW, RGB_RED, "%s", "设备不在线，呼叫失败！");
                    tim_reset_time(tim_scene_timeout, TIME_1S(3));
                    break;
                case UI_CALLING_TIMEOUT:
                case UI_CALL_SERVER_TIMEOUT:		//add by wrm 20150306 for cloud  
                    ui_make_end(target[0], target[1], target[2], target[3]);
                    usleep(600*1000);
                    if (dev_get_type(target) == DEV_INDOOR) {
                        if (is_call_kzq){
                            ui_scene_sw(SCENE_MAIN);
                        }
                        else{ ui_scene_sw(SCENE_NO_ANSWER);}
                    }
                    else{
                        ui_scene_sw(SCENE_MAIN);
                    }
                    break;
                case UI_TALKING_TIMEOUT:
                    ui_scene_sw(SCENE_MAIN);
                    ui_make_end(target[0], target[1], target[2], target[3]);
                    break;
                case UI_WATCHING_TIMEOUT:
                    //ui_make_end(target[0], target[1], target[2], target[3]);
                    break;
                case UI_WAIT_MSG_TIMEOUT:
                    if (ui_oper_mode == UI_WAIT_LEAVE_MSG) {
                        ui_oper_mode =  UI_NORMAL;
                        label_show_text(&label_prompt, WG_DRAW, RGB_RED, "对方繁忙,留言失败!");
                        tim_reset_time(tim_scene_timeout, TIME_1S(2));
                    }
                    break;
                case UI_USER_PW_UL_TIMEOUT:                    
                    if (ui_oper_mode == UI_WAIT_USER_UNLOCK) {
                        ui_oper_mode =  UI_NORMAL;
                        edit_show_prompt(edit_norm, RGB_RED, "密码错误或终端不在线");
                        //add for photo upload
                        int  pic_len = 0;
                        char *pic_data = NULL;
                        if(fkly_backup(&pic_data, &pic_len)){
                            printf("[err] shoot_visitor_photo fail!\n");
                        }                            
                        sprintf(usr_id,"%02x%02x",dev_cfg.my_code[0],dev_cfg.my_code[1]);
                        memcpy(&usr_id[4],g_usr_pwd,4);
                        send_enter_record_to_server(PASSWORD_UNLOCK_FAIL_RECORD,g_usr_pwd,usr_id,pic_data,pic_len);
                        ///end
                    }
                    break;
                    case UI_WAIT_SB_PW_ERR:
			if(ui_oper_mode == UI_WAIT_CHECK_SB_PW){
                    printf("UI_WAIT_SB_PW_TIMEOUT\n");
			ui_oper_mode =  UI_INPUT_SB_PW;
                    edit_show_prompt(edit_norm, RGB_RED, "连接失败");
			}
		    break;
		  case UI_WAIT_SB_PW_TIMEOUT:
			if(ui_oper_mode == UI_WAIT_CHECK_SB_PW){
                        printf("UI_WAIT_SB_PW_TIMEOUT\n");
			    ui_oper_mode =  UI_INPUT_SB_PW;
                        edit_show_prompt(edit_norm, RGB_RED, "密码错误，请重新操作");
			}
                        break;	
                case UI_WAIT_TC_PW_TIMEOUT:
                    printf("UI_WAIT_TC_PW_TIMEOUT----\n");
                   // if(ui_oper_mode == UI_WAIT_CHECK_SB_PW){
                        usleep(500*1000);
                        buzz_play(BUZZ_WARNING);
                        //ui_oper_mode=UI_NORMAL;
                       // label_clear(&label_cloud_show);
                        //label_show_text(&label_cloud_show, WG_FILL, RGB_RED, "连接失败，请重新操作!");
                   // }
                    break;
                case UI_START_LEAVE_MSG:
                    if (ui_oper_mode == UI_WAIT_LEAVE_MSG) {
                        ui_oper_mode =  UI_LEAVING_MSG;		
                        count_down_time = 15;
                        label_show_text(&label_prompt, WG_DRAW, RGB_YELLOW, "留言剩余时间:%02d", count_down_time);
                        tim_reset_time(tim_count_down, TIME_1S(1));
                    }
                    break;
		case UI_SB_PW_UNLOCK:
			if (ui_oper_mode == UI_WAIT_CHECK_SB_PW) {
                        ui_oper_mode =  UI_NORMAL;
                        if (msg[3]) {
                            ui_unlock(1);
                           // ui_scene_sw(SCENE_UNLOCK);
				ui_action(UI_FORCE_SWITCH, SCENE_UNLOCK);//强制刷新:解决添加广告图片后，主机不重启就扫不到锁添加图片的问题
                        }
                        else
                            edit_show_prompt(edit_norm, RGB_WHITE, STR_SW_ERROR);
                    }
                    break;
                case UI_USER_PW_UNLOCK:
                    if (ui_oper_mode == UI_WAIT_USER_UNLOCK) {
                        ui_oper_mode =  UI_NORMAL;
                        if (msg[3]) {
                            ui_unlock(1);
                           // ui_scene_sw(SCENE_UNLOCK);

                           //add for photo upload
                            int  pic_len = 0;
                            char *pic_data = NULL;
                            if(fkly_backup(&pic_data, &pic_len)){
                                printf("[err] shoot_visitor_photo fail!\n");
                            }  
                            
                           ui_action(UI_FORCE_SWITCH, SCENE_UNLOCK);
                           
                            sprintf(usr_id,"%02x%02x",dev_cfg.my_code[0],dev_cfg.my_code[1]);
                            memcpy(&usr_id[4],g_usr_pwd,4);                           
                            send_enter_record_to_server(PASSWORD_UNLOCK_RECORD,g_usr_pwd,usr_id,pic_data,pic_len);
                            ///end
                        }
                        else
                            edit_show_prompt(edit_norm, RGB_WHITE, STR_PW_ERROR);
                    }
                    break;
                case UI_TALKING_UNLOCK:
                    if (scene_cur != SCENE_TALKING) break;
                    ui_unlock(1);
                    audio_play(PMT_COME_IN, DEV_VOL_PLAY);
                    label_clear(&label_talking_unlock);
                    usleep(150*1000);
		    label_show_pic(&label_talking_unlock, WG_DRAW, IMG_TALKING_UNLOCK);
                    //label_show_text(&label_unlock, WG_DRAW, RGB_GREEN, "%s", "门锁已开，请进！");
                    talking_unlock_time = count_down_time;
                    break;
                case UI_CARD_UNLOCK:
                case UI_PB_PW_UNLOCK:
		        case UI_FINGER_UNLOCK://by mkq finger
                    ui_unlock(1);

                  //  if ((scene_cur == SCENE_CALLING) || \//新UI在呼叫界面的开锁做另外处理 add by wrm 20150119
                    if  (scene_cur == SCENE_TALKING){
                        if (msg[2] != UI_CARD_UNLOCK){ key_buzz(BUZZ_BEEP);}
                        label_clear(&label_talking_no_unlock);//by mkq finger
                        label_clear(&label_noanswer_unlock);
                        usleep(150*1000);
                        label_show_pic(&label_talking_unlock, WG_DRAW, IMG_TALKING_UNLOCK);
                        talking_unlock_time = count_down_time;                       
                    }
                    else  if(scene_cur == SCENE_NO_ANSWER && ui_oper_mode != UI_LEAVING_MSG){
                        if (msg[2] != UI_CARD_UNLOCK){ key_buzz(BUZZ_BEEP);}
                        label_clear(&label_noanswer_unlock);
                        usleep(150*1000);
                        label_show_pic(&label_noanswer_unlock, WG_DRAW, IMG_TALKING_UNLOCK);
                        talking_unlock_time = count_down_time;
                    }
                    else  if(scene_cur == SCENE_NO_ANSWER && ui_oper_mode == UI_LEAVING_MSG){
                        if (msg[2] != UI_CARD_UNLOCK){ key_buzz(BUZZ_BEEP);}
                        //by mkq finger
                        label_clear(&label_talking_no_unlock);
                        label_clear(&label_noanswer_unlock);
                        label_clear(&label_noanswer_no_unlock);
                        //end
                        label_clear(&label_leave_unlock);
                        usleep(150*1000);
                        label_show_pic(&label_leave_unlock, WG_DRAW, IMG_TALKING_UNLOCK);
                        talking_unlock_time = count_down_time;
                    }			   
                    else  if(scene_cur == SCENE_CALLING){
                        if (msg[2] != UI_CARD_UNLOCK){ key_buzz(BUZZ_BEEP);}
                        is_calling_unlock = 1;
                        label_clear(&label_cloud_show);
                        label_clear(&label_calling_promot);
                        usleep(150*1000);
                        printf("#####UNLOCK\tcalling_unlock:%d,cout_down:%d\n",calling_unlock_time,count_down_time);
                        calling_fp_unlock_faild_time = 0;//by mkq finger
                        if(count_down_time==0){//添加此处判断，是为了解决呼叫不在线终端时，转呼至手机的等待期间刷卡无提示问题
                            label_show_text(&label_unlock, WG_DRAW, RGB_YELLOW, "%s", "门锁已开，请进!");
                        }else{
                            //calling_unlock_time = count_down_time;
                            calling_unlock_time = 3;//替掉上行是为了解决转呼的前2s刷卡开锁出现的bug
                        }
                    }
                    else {
                        ui_oper_mode = UI_NORMAL;
                       // ui_scene_sw(SCENE_UNLOCK);
                         printf("msg[2] : %d\n",msg[2]);//by mkq
                        if(scene_cur == SCENE_LCD_OFF){
                            ui_action(UI_LCD_TURN_ON, 0);
                        }                            
                      
                        if(scene_cur == SCENE_FACE_DETECT)
                        {             
                            ui_exit_face_det();
                        }	                         
              
        			   if(msg[2] == UI_FINGER_UNLOCK){//by mkq finger
        			        scene_pre = SCENE_NONE;
        			   	    ui_scene_sw(SCENE_FINGER_UNLOCK);			   	    
        			   	    //ui_action(UI_FORCE_SWITCH, SCENE_FINGER_UNLOCK);
        			   }	
        			   else{
        			        scene_pre = SCENE_NONE;
    			            ui_scene_sw(SCENE_UNLOCK);
                            //ui_action(UI_FORCE_SWITCH, SCENE_UNLOCK);
                       }        
                                              
                    }
                    break;
/*
                case UI_PIC_AUTO_SLIDE:
                    if (ui_oper_mode == UI_SCREENSAVE)
                        list_auto_slide_show(list_pic);
                    break;
*/

//////////////////////////////////////////////////////////////////////////////////////////
//by mkq ginger
		case UI_NO_FEINGERTP:
		case UI_FINGER_IMGNOTCLEAR:
		case UI_FINGER_NOVALIDIMG:
		case UI_FINGER_NOFEATURE:
	       case UI_FINGER_NOINPUT:
			 if  (scene_cur == SCENE_TALKING){
			   label_clear(&label_talking_no_unlock);
			   label_clear(&label_talking_unlock);
                        usleep(150*1000);
			   label_show_text(&label_talking_no_unlock, WG_DRAW, RGB_RED, "%s", "未发现匹配指纹");//zh20150604
                        talking_unlock_time = count_down_time;                       
                    }
		   else  if(scene_cur == SCENE_NO_ANSWER && ui_oper_mode != UI_LEAVING_MSG){
		   	   label_clear(&label_noanswer_no_unlock);
                        label_clear(&label_noanswer_unlock);
                        usleep(150*1000);
		          label_show_text(&label_noanswer_no_unlock, WG_DRAW, RGB_RED, "%s", "未发现匹配指纹");//zh20150604
                        talking_unlock_time = count_down_time;
                    }
		   else  if(scene_cur == SCENE_NO_ANSWER && ui_oper_mode == UI_LEAVING_MSG){

			   label_clear(&label_talking_no_unlock);
			   label_clear(&label_leave_unlock);
                         usleep(150*1000);
			    label_show_text(&label_talking_no_unlock, WG_DRAW, RGB_RED, "%s", "未发现匹配指纹");//zh20150604
                        talking_unlock_time = count_down_time;
                    }			   
	           else  if(scene_cur == SCENE_CALLING){
			
			is_calling_unlock = 0;
			//label_clear(&label_prompt);
			label_clear(&label_calling_promot);
			label_clear(&label_talking_no_unlock);
			label_clear(&label_unlock);
			///label_show_text(&label_talking_no_unlock, WG_DRAW, RGB_RED, "%s", "未发现匹配指纹");//zh20150604
			usleep(150*1000);
			//calling_fp_unlock_faild_time = 3;
			printf("#####UNLOCK calling_fp_unlock_faild_time:%d,cout_down:%d\n",calling_fp_unlock_faild_time,count_down_time);
			
			calling_fp_unlock_faild_time = 1;
                    if(count_down_time==0){//添加此处判断，是为了解决呼叫不在线终端时，转呼至手机的等待期间刷卡无提示问题
                       // label_show_text(&label_unlock, WG_DRAW, RGB_YELLOW, "%s", "门锁已开，请进!");
			   label_show_text(&label_talking_no_unlock, WG_DRAW, RGB_RED, "%s", "未发现匹配指纹");//zh20150604
                    }else{
			//calling_unlock_time = count_down_time;
			calling_unlock_time = 3;//替掉上行是为了解决转呼的前2s刷卡开锁出现的bug
                    }
					
		    }
 		    else {
			   //printf("the ----------scene_cur = %02x---------\n",scene_cur);
			    int scene_cur_back = 0;
			    scene_cur_back = scene_cur;
			    if(scene_cur_back == SCENE_LCD_OFF ){
    			   	ui_action(UI_LCD_TURN_ON, 0);
			   	}			    
                ui_oper_mode = UI_NORMAL;
			    //ui_action(UI_FORCE_SWITCH, SCENE_NO_FINGER);

                if(scene_cur == SCENE_FACE_DETECT)
                {
                    ui_exit_face_det();
                }			   
          
                scene_pre = SCENE_NONE;
    		    ui_scene_sw(SCENE_NO_FINGER);
            }
            break;
		case UI_FINGER_REG_NOFEATURE:
		       if(scene_cur == SCENE_INPUT_PW){
			edit_text_clr(edit_norm);
			//edit_Err_show_led(); //
			edit_show_prompt(edit_norm, RGB_RED, "图像特征太少失败 ");
			}
			break;

		case UI_FINGER_REG_READAGAIN:
			if(scene_cur == SCENE_REG_FINGER){
             		edit_text_clr(edit_norm);
			
			edit_show_prompt(edit_norm, RGB_WHITE, "请再次确认手指");
			}
			 break;
		case UI_FINGER_REG_ERROR:
			if(scene_cur == SCENE_REG_FINGER||scene_cur == SCENE_INPUT_PW){
             		edit_text_clr(edit_norm);
			//edit_Err_show_led(); //
			edit_show_prompt(edit_norm, RGB_RED, "数据错误,请重试");
			buzz_play(BUZZ_CONFIRM);
			}
			else{
				fp_status_set(status_getimg); // 防止数据出错，、、/This is a bug
				//fp_get_image();
				printf("BUG................Data error!!\n");
			} 
			 break; 
			 
		case UI_FINGER_NO_EMPTY:
			if(scene_cur == SCENE_REG_FINGER||scene_cur == SCENE_INPUT_PW){
			edit_text_clr(edit_norm);
			//edit_Err_show_led(); 
			edit_show_prompt(edit_norm, RGB_RED, "指纹容量已满,请删除");
			buzz_play(BUZZ_CONFIRM);
		       fp_status_set(status_getimg);
		       //fp_get_image();
			sleep(1);
			ui_scene_sw(SCENE_MAIN);
			}
			break;
		case UI_FINGER_REG_READ:
			 if(scene_cur == SCENE_REG_FINGER||scene_cur == SCENE_INPUT_PW){
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm, RGB_WHITE, "注册! 请按手指");	 
			 }
			 break;
		case UI_FINGER_REG_STOREOK:
			 if(scene_cur == SCENE_REG_FINGER||scene_cur == SCENE_INPUT_PW){
			 sprintf(stringid, "注册成功ID:%d ,继续请重按",finger_regID);
             		 edit_text_clr(edit_norm);
			 edit_show_prompt(edit_norm, RGB_WHITE, stringid);
			 buzz_play(BUZZ_CONFIRM);		
			 }
			 break;
		case UI_FINGER_HAD_REGD:
			if(scene_cur == SCENE_REG_FINGER||scene_cur == SCENE_INPUT_PW){
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm, RGB_RED, "该指纹已注册,若继续请重按");
			buzz_play(BUZZ_WARNING);
			}
			 break;
		case UI_FINGER_REG_FAILD:
		case UI_FINGER_REG_MODEL_FAILD:
			if(scene_cur == SCENE_REG_FINGER||scene_cur == SCENE_INPUT_PW){
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm, RGB_RED, "注册失败,若继续请重按");
			buzz_play(BUZZ_WARNING);
				}
			
			 break;

		case UI_FINGER_REG_NOFINGER:
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm, RGB_RED, "传感器未发现手指,退出....");
			 break;
		case UI_FINGER_DELECT_FAILD:
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm, RGB_RED, "删除失败");
			buzz_play(BUZZ_WARNING);
		       fp_status_set(status_getimg);
		       //fp_get_image();
			sleep(1);
			ui_scene_sw(SCENE_MAIN);
			break;
		case UI_FINGER_FORMAT_OK:
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm,RGB_GREEN,"清空指纹成功");
			buzz_play(BUZZ_CONFIRM);
		       fp_status_set(status_getimg);
		       //fp_get_image();
			sleep(1);
			ui_scene_sw(SCENE_MAIN);
			break;
		case UI_FINGER_FORMAT_NG:
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm,RGB_RED,"清空指纹失败请重试");
			buzz_play(BUZZ_WARNING);
		       fp_status_set(status_getimg);
		       //fp_get_image();
			sleep(1);
			ui_scene_sw(SCENE_MAIN);
			break;
		case UI_FINGER_FORMAT_PW_ERROR:
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm,RGB_RED,"密码错误");
			buzz_play(BUZZ_WARNING);
		       fp_status_set(status_getimg);
		       //fp_get_image();
			sleep(1);
			ui_scene_sw(SCENE_MAIN);
			break;
		case UI_FINGER_DELECT_OK:

			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm,RGB_GREEN,"删除成功");
			buzz_play(BUZZ_CONFIRM);
		       fp_status_set(status_getimg);
		       //fp_get_image();
			
			sleep(1);
			ui_scene_sw(SCENE_MAIN);
			break;
		case UI_FINGER_DOWNLOADING_OK:
			edit_text_clr(edit_norm);
			edit_show_prompt(edit_norm,RGB_GREEN,"指纹更新中......");
			break;

//end

                case UI_UNLOCK_RELEASE:
                    ui_unlock(0);
                    break;
                case UI_NIGHT_TRANSMIT:
                    ui_make_end(target[0], target[1], target[2], target[3]);
                    if (ui_oper_mode == UI_NIGHT_TALKING) {
                        memcpy(target, target_night, 4);
                        sleep(1);
                        ui_make_call(target[0], target[1], target[2], target[3]);
                        ui_scene_sw(SCENE_CALLING);
                    }
                    else {
                        ui_scene_sw(SCENE_MAIN);
                    }
                    ui_oper_mode = UI_NORMAL;
                    break;
                case UI_LCD_DORMANCY:
                    //cpld_io_clr(EIO_KEYPAD_LIGHT); //modify by wrm 20150302 for may something wrong with cardread led
#if 1
                    if (0 != ui_start_auto_slide()) {   //没有图片可播放
                        if (0 != DEV_TIME_SNS)          //不是常亮
                            msg[3] = SCENE_LCD_OFF;
                    }
                    else {
                        ui_oper_mode = UI_SCREENSAVE;
                        msg[3] = SCENE_PIC_DETAIL;
                    }
#else
                    msg[3] = SCENE_LCD_OFF;
#endif
                    ui_scene_sw(msg[3]);
                    break;
                case UI_LCD_TURN_OFF:
                    ui_scene_sw(SCENE_LCD_OFF);
                    break;
                case UI_LCD_TURN_ON:
                    ui_lcd_on();
                    break;
                   
                case UI_FACE_IO_CTRL:
                {
                    if(scene_cur == SCENE_FACE_DETECT){
                        ui_lcd_off();
                        Set_CCD_Light(0); 
                    }
                    break;
                }
                                
                case UI_CLOSE_FACE_DET:
                {
   
                    if(scene_cur == SCENE_FACE_DETECT){
                        ui_lcd_off();
                        Set_CCD_Light(0);
                        ui_scene_sw(SCENE_LCD_OFF);
                        face_det_stop();
                        usleep(300*1000);
                    }                      
                    break;
                    
                } 

                case UI_SW_FR_TO_MAIN:
                {
                    ui_exit_face_det();
                    scene_pre = SCENE_NONE;
                    ui_scene_sw(SCENE_MAIN);
                    break;
                }
                
            }
        }
        else
        if (msg[1] == MSG_UI_KEY_IN) {
            key_code = msg[2];
            key_down = msg[3];
            //printf("---key_down=%d\n",key_down);
            if (0 == ui_button_press(key_code, key_down)) 
                continue;
            app_debug(DBG_INFO,"--------scene_cur=0x%02x\n",scene_cur);

            switch(scene_cur) {
                case SCENE_UNLOCK:
                case SCENE_CALLING:
                case SCENE_NO_ANSWER:
		   case SCENE_FINGER_UNLOCK://by mkq finger
		  case SCENE_NO_FINGER://by mkq finger
                    break;
                default:
                    tim_reset_event(tim_scene_timeout);
                    break;
            }

            edit_text_edit(key_code);

            if (ui_proc_list[scene_cur] != NULL)
                (*ui_proc_list[scene_cur])();
            
            ui_button_release(key_code, key_down);
        }
#ifdef _FACE_DET
        else if(msg[1] == MSG_UI_BODY_SENSE)
        {
            switch(scene_cur) 
            {    
            case SCENE_LCD_OFF: 
// _KEEP_OPEN使能后人脸识别后台常驻，页面切换时才关闭       
    #ifdef _KEEP_OPEN    
                body_sense_result = msg[2];
    #else             
                if(msg[2] == 1)
                {
                    printf("[IR] some one nearby!\n");        
                    ui_scene_sw(SCENE_FACE_DETECT);                     
                    /*开始人脸检测，发送图片给服务器*/ 
                    Set_CCD_Light(1);                      
                    face_det_start();                                     
                } 
    #endif                
                break;
            
            case SCENE_FACE_DETECT:              
                if(msg[2] == 0)
                {
                    printf("[IR] visitor is leaving\n");
                    tim_reset_event(tim_face_det_timeout);  
    #ifdef _KEEP_OPEN 
                    body_sense_result = 0;
    #endif
                }
                else if(msg[2] == 1)
                {
                    printf("[IR] visitor approach\n");
                    tim_suspend_event(tim_face_det_timeout);
    #ifdef _KEEP_OPEN 
                    face_det_flag = 1;//有人来访，启动人脸识别
                    Set_CCD_Light(1);                    
                    ui_lcd_on();

                    /*启动算法切换定时器*/
                    body_sense_result = 1;
                    restart_algo_auto_sw();
    #endif                    
                }               
                break;
            }        
        }   
#endif
    }
    exit(1);
    return THREAD_SUCCESS;
}

int ui_start(void *arg)
{
    struct msg_t *p_msg = (struct msg_t *)arg;

    ui_pipe_proc = p_msg->pipe_proc; 
    ui_pipe_ui   = p_msg->pipe_ui; 
    
    pthread_create(&ui_tid, NULL, ui_thread, NULL);

    return 0;
}

