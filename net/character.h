#ifndef CHARACTER_H_
#define CHARACTER_H_
#include "global_def.h"
#if DEV_TERMINAL
#define dhcp_ebl_s 0
#define equ_description_s "安居宝AJB-ZD10E数字终端"
#define equ_name_s "AJB-ZD10E数字终端"
#define prod_name_s "广东安居宝数码科技股份有限公司"
#define equ_type_s 2
#define type_name_s "分机"
#define hardware_version_s "DM365"
#define position_s "室内"
#define description_s "可以升级"
#endif

#if DEV_GATE
#define dhcp_ebl_s 0
#define equ_description_s "安居宝AJB-ZJ11BC主机"
#define equ_name_s "11B门口主机"
#define prod_name_s "广东安居宝数码科技股份有限公司"
#define equ_type_s 2
#define type_name_s "主机"
#define hardware_version_s "dm365"
#define position_s "单元门口"
#define description_s "可以升级"
#define AnimaWord_s "安居宝楼宇对讲系统"

#endif

#if DEV_CONTROL
#define dhcp_ebl_s 0
#define equ_description_s "安居宝HY-20020406B主机控制器"
#define equ_name_s "大门口主机控制器"
#define prod_name_s "广东安居宝数码科技股份有限公司"
#define equ_type_s 2
#define type_name_s "控制器"
#define hardware_version_s "dm365"
#define position_s "用户家"
#define description_s "可以升级"
#endif

#if DEV_GLJKZQ
#define dhcp_ebl_s 0
#define equ_description_s "安居宝HY-20020406C管理机控制器"
#define equ_name_s "AJB管理机控制器"
#define prod_name_s "广东安居宝数码科技股份有限公司"
#define equ_type_s 2
#define type_name_s "控制器"
#define hardware_version_s "dm365"
#define position_s "读卡机箱"
#define description_s "可以升级"
#endif


#endif/*CHARACTER_H_*/