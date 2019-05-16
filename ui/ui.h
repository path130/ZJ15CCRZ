#ifndef _UI_H_
#define _UI_H_

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum {
    UI_LCD_TURN_ON,
    UI_LCD_TURN_OFF,
    UI_LCD_DORMANCY,
    UI_SCENE_SWITCH,
    UI_SCENE_REDRAW,
    UI_FORCE_SWITCH,
    UI_CURSOR_RESUME,
    UI_CURSOR_FLASH,
    UI_STBAR_REFRESH,
    UI_TM_COUNT_DOWN,
    UI_PIC_AUTO_SLIDE,
    UI_START_LEAVE_MSG,
    UI_UNLOCK_RELEASE,
    UI_UART0_GOT_DATA,
    UI_UART1_GOT_DATA,
    UI_UART1_GOT_CARD, 
    UI_NET_LINK_STAT,       //NET_LINK_ON NET_LINK_OFF
    UI_CALLING_FAIL,        //TARGET_BUSY TARGET_OFFLINE
    UI_CALLING_GOTASK,
    UI_CARD_UNLOCK,
    UI_PB_PW_UNLOCK,
    UI_SB_PW_UNLOCK,
    UI_USER_PW_UNLOCK,
    UI_TALKING_UNLOCK,
    UI_NIGHT_TRANSMIT,
    UI_CALLING_TIMEOUT,
    UI_TALKING_TIMEOUT,
    UI_WATCHING_TIMEOUT,
    UI_WAIT_MSG_TIMEOUT,
    UI_USER_PW_UL_TIMEOUT,
    UI_WAIT_SB_PW_TIMEOUT,
    UI_WAIT_SB_PW_ERR,
    UI_CALL_SERVER_TIMEOUT, //add by wrm 20150306 for cloud 
    UI_CALL_SHOW,
    UI_WAIT_TC_PW_TIMEOUT,
    //by mkq finger
    UI_NO_FEINGERTP,
    UI_FINGER_NOINPUT,
    UI_FINGER_UNLOCK,
    UI_FINGER_IMGNOTCLEAR,
    UI_FINGER_NOFEATURE,
    UI_FINGER_NOVALIDIMG,
    UI_FINGER_REG_NOFEATURE,
    UI_FINGER_REG_READ,
    UI_FINGER_REG_READAGAIN,
    UI_FINGER_REG_MODEL_FAILD,
    UI_FINGER_REG_ERROR,
    UI_FINGER_HAD_REGD,
    UI_FINGER_NO_EMPTY,
    UI_FINGER_REG_STOREOK,
    UI_FINGER_REG_FAILD,
    UI_FINGER_REG_NOFINGER,
    UI_FINGER_DELECT_FAILD,
    UI_FINGER_FORMAT_OK,
    UI_FINGER_FORMAT_NG,
    UI_FINGER_FORMAT_PW_ERROR,
    UI_FINGER_DELECT_OK,
    UI_FINGER_DOWNLOADING_OK,
    //end
    UI_AUTO_TEST,
    UI_CLOSE_FACE_DET,   
    UI_FACE_IO_CTRL, 
    UI_SW_FR_TO_MAIN,
} UI_MSG_ACTION;            //message send to ui

typedef enum {
    UI_SEND_AJB_DATA,
    UI_SEND_LEAVE_MSG,
    UI_SEND_CALL_RECORDS,
    UI_SEND_CALL_TRANSFER,
    UI_SEND_AUTO_TEST,
} UI_MSG_TRIGGER;           //message from ui

typedef enum {
    UI_NORMAL,
    UI_INPUT_CODE,
    UI_INPUT_VI_CODE,
    UI_INPUT_SB_PW,
    UI_INPUT_PB_PW,
    UI_INPUT_KB_PW,
    UI_INPUT_KB_PW_OLD,
    UI_INPUT_KB_PW_NEW,
    UI_INPUT_KB_PW_FIRM,
    UI_INPUT_CARD_FMT,
    UI_INPUT_CARD_REG,
    UI_INPUT_CARD_DEL,
    UI_INPUT_CARD_NUM,
    UI_INPUT_NEW_CARD,
    UI_WAIT_NEW_CARD,
    UI_WAIT_CARD_WRITE,
    UI_INPUT_CIF_MODE,
    UI_INPUT_MC_DELAY,
    UI_INPUT_VOLUME_RecallTime,
    UI_INPUT_LOCK_TYPE,
    UI_INPUT_LOCK_TIME,
    UI_INPUT_CAMERA_MODE,
    UI_INPUT_BLE_DBM_TX,
    UI_INPUT_VOLUME_RX,
    UI_INPUT_VOLUME_TX,
    UI_INPUT_CVBS_MODE,
    UI_INPUT_AUTO_TARGET,
    UI_WAIT_USER_UNLOCK,
    UI_WAIT_CHECK_SB_PW,
    UI_WAIT_LEAVE_MSG,
    UI_LEAVING_MSG,
    UI_SET_BACKGRROUND,
    UI_NIGHT_CALLING,
    UI_NIGHT_TALKING,
    UI_PIC_DISPLAY,
    UI_SCREENSAVE,
    UI_LCD_OFF,
    UI_CATALOG,
    //by mkq finger
    UI_INPUT_VOLUME_SPK,
    UI_INPUT_FINGER_REG,
    UI_INPUT_FINGER_FMT,
    UI_INPUT_FINGER_DELE_ID,
    //end
} UI_OP_MODE;

#define CFG_USE_OLD_CARDREADER
#define UI_MEM_SEMISTATIC
//#define TIME_SHOW_SEC

//ui_action : arg
#define TARGET_OFFLINE          0  
#define TARGET_BUSY             1
#define SERVER_OFFLINE          2  
#define SERVER_BUSY             3
#define NET_LINK_OFF            0
#define NET_LINK_ON             1

#define UI_FULL_W               320
#define UI_FULL_H               240
#define UI_TOP_H                182
#define UI_BOT_H                58

#define PATH_TXT                "./../text"
#define PATH_PIC                "./../image"
#define PATH_PIC_UNLOCK         "./ad"

#define FONT_FILE               "microhei.ttc"

#ifdef  FRONT_DOOR
#define ROOM_NUM                8
#define ES_LM_C                 ES_LM_8
#define STR_ROOM_LESS           "请输入八位房号!"
#define DEVICE_NAME             "15C主机"
#else
#define ROOM_NUM                4
#define ES_LM_C                 ES_LM_4
#define STR_ROOM_LESS           "请输入四位房号!" 
#define DEVICE_NAME             "15C主机"
#endif

#define PB_UL_PW_NUM            6
#define STR_PW_SET_OK           "密码修改成功!"
#define STR_PW_ERROR            "密码错误,请重输!"
#define STR_SW_ERROR            "访客密码错误!"
#define STR_PW_LESS             "请输入六位密码!"
#define STR_NO_MANAGE           "管理密令未开启!"
#define STR_PW_FIRM_ERR         "两次新密码不相同!"

#define INPUT_PASSWORD          1
#define INPUT_TEXT              0

#define MSG_UI_KEY_IN           0x01
#define MSG_UI_ACTION           0x02
#define MSG_UI_BODY_SENSE       0x03

#define SCENE_NONE              0x00
#define SCENE_MAIN              0x01
#define SCENE_MESSAGE           0x02
#define SCENE_PICTURE           0x03
#define SCENE_SETTING           0x04
#define SCENE_HELP              0x05
#define SCENE_CALL              0x06
#define SCENE_CALLING           0x07
#define SCENE_NO_ANSWER         0x08
#define SCENE_TALKING           0x09
#define SCENE_SET_SEL           0x0A
#define SCENE_INPUT_PW          0x0B
#define SCENE_UNLOCK            0x0C
#define SCENE_SET_PW            0x0D
#define SCENE_SET_SCREEN        0x0E
#define SCENE_SET_VOLUMN        0x0F
#define SCENE_SET_TIME          0x10
#define SCENE_SET_IP            0x11    
#define SCENE_SET_ABOUT         0x12
#define SCENE_TXT_DETAIL        0x13
#define SCENE_PIC_DETAIL        0x14
#define SCENE_CATALOG           0x15
#define SCENE_LCD_OFF           0x16
#define SCENE_TWO_CODE_ERR      0x17
#define SCENE_FACE_DETECT       0x18
#define SCENE_REG_FINGER             0x19
#define SCENE_FINGER_UNLOCK	 0x1A
#define SCENE_NO_FINGER	        0X1B
#define SCENE_SET_LIFT_IP       0x1C    //add for multi_lift_ctl
#define SCENE_MAX               0x1D
#define SCENE_CURRENT           SCENE_NONE

#define IMG_COMMON              "./pic/common.bmp"
#define IMG_COMMON_X            0
#define IMG_COMMON_Y            0
#define IMG_NET_LINK_ON         "./pic/net_link.png"
#define IMG_NET_LINK_OFF        "./pic/net_link_off.png"
#define IMG_NET_LINK_X          277
#define IMG_NET_LINK_Y          4

/* SCENE_MAIN */
#define IMG_MAIN                "./pic/BG.png"
#define IMG_MAIN_X              0
#define IMG_MAIN_Y              0
#define BT_F1_MAIN_X            12
#define BT_F2_MAIN_X            88
#define BT_F3_MAIN_X            166
#define BT_F4_MAIN_X            244
#define BT_F1_4_MAIN_Y          149
#define BT_F1_MAIN_IMG_U        "./pic/F1_msg.png"
#define BT_F1_MAIN_IMG_D        "./pic/F1_msg_down.png"
#define BT_F2_MAIN_IMG_U        "./pic/F2_pic.png"
#define BT_F2_MAIN_IMG_D        "./pic/F2_pic_down.png"
#define BT_F3_MAIN_IMG_U        "./pic/F3_set.png"
#define BT_F3_MAIN_IMG_D        "./pic/F3_set_down.png"
#define BT_F4_MAIN_IMG_U        "./pic/F4_help.png"
#define BT_F4_MAIN_IMG_D        "./pic/F4_help_down.png"
#define IMG_MAIN_MSG            "./pic/main_msg.png"
#define IMG_MAIN_MSG_X          0 
#define IMG_MAIN_MSG_Y          139
#define IMG_TOP_BANNER          "./pic/top_banner.png"
#define IMG_TOP_BANNER_X        0
#define IMG_TOP_BANNER_Y        2
#define IMG_TOP_WELCOME         "./pic/top_welcome.png"
#define IMG_TOP_WELCOME_X       4
#define IMG_TOP_WELCOME_Y       10

#define IMG_BOT_WELCOME         "./pic/bot_welcome.png"
#define IMG_BOT_WELCOME_X       85
#define IMG_BOT_WELCOME_Y       211

#define IMG_BOT_TRAY            "./pic/bot_tray.png"
#define IMG_BOT_TRAY_X          1
#define IMG_BOT_TRAY_Y          197
#define IMG_BOT_BANNER          "./pic/bot_banner.png"
#define IMG_BOT_BANNER_X        0
#define IMG_BOT_BANNER_Y        192

#define IMG_MAIN_TOP         "./pic/main_top_picword.png"
#define IMG_FD_MAIN_TOP         "./pic/main_top.png"
#define IMG_MAIN_TOP_X       16
#define IMG_MAIN_TOP_Y       48

#define IMG_MAIN_LABEL1         "./pic/main_call_user.png"
#define IMG_MAIN_LABEL1_X       27
#define IMG_MAIN_LABEL1_Y       52

#define IMG_MAIN_MID         "./pic/main_mid_picword.png"
#define IMG_MAIN_MID_X       16
#define IMG_MAIN_MID_Y       100

#define IMG_MAIN_LABEL2         "./pic/main_password.png"
#define IMG_MAIN_LABEL2_X       27
#define IMG_MAIN_LABEL2_Y       110

#define IMG_MAIN_BOT         "./pic/main_bot_picword.png"
#define IMG_MAIN_BOT_X       16
#define IMG_MAIN_BOT_Y       152

#define IMG_MAIN_LABEL3         "./pic/main_call_man.png"
#define IMG_MAIN_LABEL3_X       27
#define IMG_MAIN_LABEL3_Y       157

/* SCENE_MESSAGE */
#define IMG_MSG_HEAD            NULL//"./pic/msg_head.png"
#define IMG_MSG_LIST            "./pic/msg_list.png"
#define IMG_MSG_HEAD_X          7
#define IMG_MSG_HEAD_Y          55
#define IMG_MSG_LIST_X          7
#define IMG_MSG_LIST_Y          55//79

/* SCENE_PICTURE */
#define IMG_PIC_HEAD            "./pic/pic_head.png"
#define IMG_PIC_LIST            "./pic/pic_list1.png"//"./pic/pic_list.png"             
#define IMG_PIC_HEAD_X          7
#define IMG_PIC_HEAD_Y          53
#define IMG_PIC_LIST_X          7
#define IMG_PIC_LIST_Y          53//74
#define IMG_FRAME_X             86
#define IMG_FRAME_Y             53
#define IMG_FRAME_W             218
#define IMG_FRAME_H             99

/* SCENE_SETTING */
#define IMG_SETUP_PW            "./pic/setup_pw.png"
#define IMG_SETUP_PW_X          71
#define IMG_SETUP_PW_Y          70
#define IMG_INPUT_PWD_BACK "./pic/input_pwd_back.png"
#define IMG_INPUT_PWD_X	48
#define IMG_INPUT_PWD_Y	96+10
/* SCENE_HELP */
#ifdef FRONT_DOOR
#define IMG_HELP_TEXT           "./pic/help_text_front_door.png"
#else
#define IMG_HELP_TEXT           "./pic/help_text.png"
#endif
#define IMG_HELP_HEAD           "./pic/help_head.png"
#define IMG_HELP_HEAD_X         8
#define IMG_HELP_HEAD_Y         66
#define IMG_HELP_TEXT_X         36
#define IMG_HELP_TEXT_Y         35

/* SCENE_CALL */
#define EDIT_IMG_NORM           "./pic/txt_label.png"
#define EDIT_NORM_X             87
#define EDIT_NORM_Y             102+10
#define IMG_CALL_BACK           "./pic/call_background.png"
#define CALL_BACK_X             48
#define CALL_BACK_Y             96+10
#define IMG_CALL_MESS           "./pic/call_message.png"
#define CALL_MESS_X             59
#define CALL_MESS_Y             102+10
#define IMG_CALL_ICON         "./pic/call_icon.png"
#define CALL_ICON_X             251
#define CALL_ICON_Y             102+10

/* SCENE_CALLING */
#define IMG_CALLING             "./pic/calling.png"
#define IMG_CALLING_X           90
#define IMG_CALLING_Y           73
#define IMG_CALLING_ADMIN             "./pic/calling_admin.png"
#define IMG_CALLING_UP      "./pic/calling_up.png"
#define IMG_CALLING_UP_X           65
#define IMG_CALLING_UP_Y           68
#define IMG_CALLING_DOWN      "./pic/calling_down.png"
#define IMG_CALLING_DOWN_X           65
#define IMG_CALLING_DOWN_Y           137      //145
#define IMG_CALLING_LINK             "./pic/calling_link.png"
#define IMG_CALLING_LINK_X           90
#define IMG_CALLING_LINK_Y           141

/* SCENE_NO_ANSWER */
#define IMG_NO_ANSWER           "./pic/no_answer.png"
#define IMG_NO_ANSWER_X         65
#define IMG_NO_ANSWER_Y         80

/* SCENE_TALKING*/
#define IMG_REMAIN_TIME         "./pic/remaining_time.png"
#define IMG_REMAIN_TIME_X       62
#define IMG_REMAIN_TIME_Y       76
#define IMG_TALKING_UNLOCK  "./pic/talking_unlock.png"
#define IMG_TALKING_UNLOCK_X	55
#define IMG_TALKING_UNLOCK_Y	130

/* SCENE_SET_SEL */
#define BT_IMG_PB_UL_PW_U       "./pic/pb_ul_pw.png"
#define BT_IMG_PB_UL_PW_D       "./pic/pb_ul_pw_down.png"
#define BT_IMG_SCREEN_U         "./pic/screen.png"
#define BT_IMG_SCREEN_D         "./pic/screen_down.png"
#define BT_IMG_VOLUME_U         "./pic/volumn.png"
#define BT_IMG_VOLUME_D         "./pic/volumn_down.png"
#define BT_IMG_TIME_U           "./pic/time.png"
#define BT_IMG_TIME_D           "./pic/time_down.png"
#define BT_IMG_IP_U             "./pic/ip.png"
#define BT_IMG_IP_D             "./pic/ip_down.png"
#define BT_IMG_ABOUT_U          "./pic/about.png"
#define BT_IMG_ABOUT_D          "./pic/about_down.png"
#define BT_PB_UL_PW_X           16
#define BT_PB_UL_PW_Y           47
#define BT_SCREEN_X             113
#define BT_SCREEN_Y             47
#define BT_VOLUME_X             210
#define BT_VOLUME_Y             47
#define BT_TIME_X               16
#define BT_TIME_Y               118
#define BT_IP_X                 113
#define BT_IP_Y                 118
#define BT_ABOUT_X              210
#define BT_ABOUT_Y              118

/* SCENE_INPUT_PW */
#define IMG_INPUT_PW            "./pic/input_pw.png"
#define IMG_INPUT_PW_X          71
#define IMG_INPUT_PW_Y          103

/* SCENE_UNLOCK */
#define IMG_UL_HEAD             "./pic/unlock_head.png"
#define IMG_UL_MSG              "./pic/unlock_msg.png"
#define IMG_UL_HEAD_X           16
#define IMG_UL_HEAD_Y           46
#define IMG_UL_MSG_X            41
#define IMG_UL_MSG_Y            98

/* SCENE_SET_PW */

#define IMG_PW_CHANGE           "./pic/pw_change.png"
#define IMG_TEXT_PW_OLD         "./pic/pw_old.png"
#define IMG_TEXT_PW_NEW         "./pic/pw_new.png"
#define IMG_TEXT_PW_FIRM        "./pic/pw_firm.png"
#define IMG_PW_CHANGE_X         21
#define IMG_PW_CHANGE_Y         52-6
#define IMG_TEXT_PW_X           27
#define IMG_TEXT_PW_OLD_Y       82
#define IMG_TEXT_PW_NEW_Y       121
#define IMG_TEXT_PW_FIRM_Y      160
#define EDIT_IMG_SMALL          "./pic/txt_label_s.png"
#define EDIT_PW_X               112
#define EDIT_PW_OLD_Y           87
#define EDIT_PW_NEW_Y           126
#define EDIT_PW_FIRM_Y          165


/* SCENE_SET_SCREEN */
#define IMG_BRIGHTNESS          "./pic/brightness.png"
#define IMG_BRIGHTNESS_X        131
#define IMG_BRIGHTNESS_Y        49
#define BT_IMG_BRIGHT_INC_U     "./pic/bt_inc.png"
#define BT_IMG_BRIGHT_INC_D     "./pic/bt_inc_down.png"
#define BT_IMG_BRIGHT_DEC_U     "./pic/bt_dec.png"
#define BT_IMG_BRIGHT_DEC_D     "./pic/bt_dec_down.png"
#define BT_BRIGHT_INC_X         277
#define BT_BRIGHT_INC_Y         83
#define BT_BRIGHT_DEC_X         21
#define BT_BRIGHT_DEC_Y         83
#define IMG_SNSAVERS_TIME       "./pic/snsavers_time.png"
#define IMG_SNSAVERS_TIME_X     131
#define IMG_SNSAVERS_TIME_Y     120
#define IMG_ALWAY               "./pic/always_on.png"
#define IMG_ALWAY_X             54
#define IMG_ALWAY_Y             159+3
#define IMG_MIN_1               "./pic/minute_1.png"
#define IMG_MIN_1_X             155
#define IMG_MIN_1_Y             159+3
#define IMG_MIN_5               "./pic/minute_5.png"
#define IMG_MIN_5_X             252
#define IMG_MIN_5_Y             159+3
#define RADIO_IMG_T             "./pic/radio_t.png"
#define RADIO_IMG_F             "./pic/radio_f.png"
#define RADIO_IMG_F_T           "./pic/radio_f_t.png"
#define RADIO_IMG_F_F           "./pic/radio_f_f.png"
#define RADIO_ALWAY_X           31
#define RADIO_ALWAY_Y           159
#define RADIO_MIN_1_X           131
#define RADIO_MIN_1_Y           159
#define RADIO_MIN_5_X           228
#define RADIO_MIN_5_Y           159
#define SLIDER_IMG_BG           "./pic/slider_bg.png"
#define SLIDER_IMG_FG           "./pic/slider_fg.png"
#define SLIDER_BRIGHT_X         49
#define SLIDER_BRIGHT_Y         83

/* SCENE_SET_VOLUMN */
#define IMG_VOLUMES             "./pic/volumes.png"
#define IMG_VOLUMES_X           131
#define IMG_VOLUMES_Y           78
#define BT_IMG_VOLUMES_INC_U    "./pic/bt_inc.png"
#define BT_IMG_VOLUMES_INC_D    "./pic/bt_inc_down.png"
#define BT_IMG_VOLUMES_DEC_U    "./pic/bt_dec.png"
#define BT_IMG_VOLUMES_DEC_D    "./pic/bt_dec_down.png"
#define BT_VOLUMES_INC_X        277
#define BT_VOLUMES_INC_Y        113
#define BT_VOLUMES_DEC_X        21
#define BT_VOLUMES_DEC_Y        113
#define SLIDER_VOLUMES_X        49
#define SLIDER_VOLUMES_Y        113

/* SCENE_SET_TIME */
#define IMG_DATE                "./pic/time_ymd.png"
#define IMG_TIME                "./pic/time_hms.png"
#define EDIT_IMG_TIME		"./pic/hms_edit.png"
#define EDIT_IMG_DATE		"./pic/ymd_edit.png"
#define IMG_DATE_X              32
#define IMG_DATE_Y              82
#define IMG_TIME_X              32
#define IMG_TIME_Y              124
#define EDIT_DATE_X             111
#define EDIT_DATE_Y             82
#define EDIT_TIME_X             111
#define EDIT_TIME_Y             124

/* SCENE_SET_IP */
#define IMG_IP_ADDR             "./pic/ip_addr.png"
#define IMG_IP_GATE             "./pic/ip_gate.png"
#define IMG_IP_MASK             "./pic/ip_mask.png"
#define EDIT_IMG_IP    "./pic/ip_edit.png"
#define EDIT_IMG_GATE    "./pic/gate_edit.png"
#define EDIT_IMG_MASK    "./pic/mask_edit.png"
#define IMG_IP_ADDR_X           37
#define IMG_IP_ADDR_Y           68
#define IMG_IP_GATE_X           37
#define IMG_IP_GATE_Y           110
#define IMG_IP_MASK_X           37
#define IMG_IP_MASK_Y           152
#define EDIT_IP_X               108
#define EDIT_IP_ADDR_Y          68
#define EDIT_IP_GATE_Y          110
#define EDIT_IP_MASK_Y          152

/* SCENE_SET_ABOUT */
#define EDIT_IMG_ABOUT_MODEL          "./pic/about_label_model.png"
#define EDIT_IMG_ABOUT_MAC          "./pic/about_label_mac.png"
#define EDIT_IMG_ABOUT_IP          "./pic/about_label_ip.png"
#define EDIT_IMG_ABOUT_GATE        "./pic/about_label_gate.png"
#define EDIT_IMG_ABOUT_SV          "./pic/about_label_sv.png"
#define EDIT_IMG_ABOUT_HV          "./pic/about_label_hv.png"

#define EDIT_ABOUT_MODEL_X     14
#define EDIT_ABOUT_MODEL_Y     65
#define EDIT_ABOUT_MAC_Y     116
#define EDIT_ABOUT_IP_Y     169
#define EDIT_ABOUT_GATE_X     166
#define EDIT_ABOUT_GATE_Y     65
#define EDIT_ABOUT_SV_Y     116
#define EDIT_ABOUT_HV_Y     169

#define IMG_ABOUT_MODEL         "./pic/about_model.png"
#define IMG_ABOUT_MODEL_X       14
#define IMG_ABOUT_MODEL_Y       49
#define IMG_ABOUT_IP            "./pic/about_ip.png"
#define IMG_ABOUT_IP_X          14
#define IMG_ABOUT_IP_Y          153
#define IMG_ABOUT_MAC           "./pic/about_mac.png"
#define IMG_ABOUT_MAC_X         14
#define IMG_ABOUT_MAC_Y         101
#define IMG_ABOUT_GATE          "./pic/about_gateway.png"
#define IMG_ABOUT_GATE_X        166
#define IMG_ABOUT_GATE_Y        49
#define IMG_ABOUT_S_VER         "./pic/about_soft_ver.png"
#define IMG_ABOUT_S_VER_X       166
#define IMG_ABOUT_S_VER_Y       101
#define IMG_ABOUT_H_VER         "./pic/about_hard_ver.png"
#define IMG_ABOUT_H_VER_X       166
#define IMG_ABOUT_H_VER_Y       153

/* BUTTON POS & FILES */
#define BT_F1_X                 8
#define BT_F2_X                 85
#define BT_F3_X                 162
#define BT_F4_X                 239
#define BT_F1_4_Y               197

#define BT_F1_IMG_NUM_U         "./pic/F1_num.png"
#define BT_F1_IMG_NUM_D         "./pic/F1_num_down.png"
#define BT_F1_IMG_UP_U          "./pic/F1_up.png"
#define BT_F1_IMG_UP_D          "./pic/F1_up_down.png"
#define BT_F1_IMG_U_D_U         "./pic/F1_u_d.png"
#define BT_F1_IMG_U_D_D         "./pic/F1_u_d_down.png"
#define BT_F1_IMG_LAV_U         "./pic/F1_leave.png"
#define BT_F1_IMG_LAV_D         "./pic/F1_leave_down.png"
#define BT_F1_IMG_MOVE_U        "./pic/F1_move.png"
#define BT_F1_IMG_MOVE_D        "./pic/F1_move_down.png"
#define BT_F1_IMG_DEC_U         "./pic/F1_dec.png"
#define BT_F1_IMG_DEC_D         "./pic/F1_dec_down.png"

#define BT_F2_IMG_DEL_U         "./pic/F2_del.png"
#define BT_F2_IMG_DEL_D         "./pic/F2_del_down.png"
#define BT_F2_IMG_DOWN_U        "./pic/F2_down.png"
#define BT_F2_IMG_DOWN_D        "./pic/F2_down_down.png"
#define BT_F2_IMG_L_R_U         "./pic/F2_l_r.png"
#define BT_F2_IMG_L_R_D         "./pic/F2_l_r_down.png"
#define BT_F2_IMG_TIME_U        "./pic/F2_time.png"
#define BT_F2_IMG_TIME_D        "./pic/F2_time_down.png"
#define BT_F2_IMG_FOCUS_U       "./pic/F2_focus.png"
#define BT_F2_IMG_FOCUS_D       "./pic/F2_focus_down.png"
#define BT_F2_IMG_INC_U         "./pic/F2_inc.png"
#define BT_F2_IMG_INC_D         "./pic/F2_inc_down.png"

#define BT_F3_IMG_EXIT_U        "./pic/F3_exit.png"
#define BT_F3_IMG_EXIT_D        "./pic/F3_exit_down.png"
#define BT_F3_IMG_DETAIL_U      "./pic/F3_detail.png"
#define BT_F3_IMG_DETAIL_D      "./pic/F3_detail_down.png"
#define BT_F3_IMG_CANCEL_U      "./pic/F3_cancel.png"
#define BT_F3_IMG_CANCEL_D      "./pic/F3_cancel_down.png"
#define BT_F3_IMG_DEL_U         "./pic/F3_del.png"
#define BT_F3_IMG_DEL_D         "./pic/F3_del_down.png"
#define BT_F3_IMG_FULL_U        "./pic/F3_full.png"
#define BT_F3_IMG_FULL_D        "./pic/F3_full_down.png"
#define BT_F3_IMG_REDIAL_U      "./pic/F3_redial.png"
#define BT_F3_IMG_REDIAL_D      "./pic/F3_redial_down.png"

#define BT_F4_IMG_OK_U          "./pic/F4_ok.png"
#define BT_F4_IMG_OK_D          "./pic/F4_ok_down.png"
#define BT_F4_IMG_EXIT_U        "./pic/F4_exit.png"
#define BT_F4_IMG_EXIT_D        "./pic/F4_exit_down.png"
#define BT_F4_IMG_HANG_U        "./pic/F4_hang.png"
#define BT_F4_IMG_HANG_D        "./pic/F4_hang_down.png"


#define BT_MF_CREATE(key, ctl) \
bt_##key##_##ctl = button_create_ex(BT_##key##_MAIN_X, BT_F1_4_MAIN_Y, BT_##key##_MAIN_IMG_U, BT_##key##_MAIN_IMG_D)
#define BT_F_CREATE(key, ctl)  \
bt_##key##_##ctl = button_create_ex(BT_##key##_X, BT_F1_4_Y, BT_##key##_IMG_##ctl##_U, BT_##key##_IMG_##ctl##_D)
#define BT_CREATE(ctl)  \
button_create_ex(BT_##ctl##_X, BT_##ctl##_Y, BT_IMG_##ctl##_U, BT_IMG_##ctl##_D)


#define COUNT_DOWN_W            40
#define COUNT_DOWN_H            40
#define COUNT_DOWN_X            150
#define COUNT_DOWN_Y            62

/* ui_fill_background arg*/
#define BG_MAIN                 0x00
#define BG_TOP                  0x01
#define BG_BOT                  0x02
#define BG_COMM                 0x03

/* statusbar */
#ifdef TIME_SHOW_SEC
#define OSD_STBAR_W             303+1
#define OSD_STBAR_X             9
#else
#define OSD_STBAR_W             303
#define OSD_STBAR_X             9
#endif
#define OSD_STBAR_TIME_X   128
#define OSD_STBAR_TIME_Y   2

#define OSD_STBAR_DATE_X   18
#define OSD_STBAR_DATE_Y   12 //UI原坐标为4

#define OSD_STBAR_H             31
#define OSD_STBAR_Y             4

#define ST_TIME                 0x01
#define ST_NET_LINK             0x02
#define ST_NET_NOLINK           0x04

/* ui_check_password return */
#define PW_ERROR            (-1)
#define PW_OK               0

/* ui_check_command return */
#define CMD_LIMIT           (-2)
#define CMD_UNMATCH         (-1)
#define CMD_OK              0

#define WG_FILL             0
#define WG_DRAW             1

#define DOOR_UNKNOWN        0
#define DOOR_OPEN           1
#define DOOR_CLOSE          2
#define RGB_YELLOW	    0xFFC422 //add by wrm 20150119 for ZJ15CCHIP font color already add in osd.h buf for compile so...

#define CALL_RECORD_CALL        0x01
#define CALL_RECORD_HANGON      0x02
#define CALL_RECORD_UNLOCK      0x03
#define CALL_RECORD_ENDCALL     0x05
#define CALL_RECORD_TIMEOUT     0x06

#define BLE_RECORD_OPENDOOR     'V'
#define FKPW_RECORD_OPENDOOR    'G'
#define USER_RECORD_OPENDOOR   'U'

extern unsigned char  is_call_kzq;
extern unsigned char  target[];
extern global_data    gbl_alarm_delay;
extern global_data    gbl_cardlight;
extern global_data  gbl_cloud_show;
extern global_data  gbl_user_unlock;

extern int  ui_start(void *arg);
extern void ui_unlock(int act);
extern void ui_get_target(unsigned char room[]);
extern void ui_action(UI_MSG_ACTION act, int arg);
extern void ui_get_village();
extern void ui_get_unlock_pw(unsigned char passwd0, unsigned char passwd1,unsigned char passwd2);

#if defined (__cplusplus)
}
#endif
 
#endif
