#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include "dev_info.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define DEV_VERSION                 "15CH"
#define DEV_FUNTYPE                 "P006"   //P:IPNC kernel 006:MT9P006(MT9P031)

#define CFG_SUPPORT_INFO_STORE 
#define CFG_CARD_IN_FLASH 

#define DEV_CONFIG_PATH             "./param.cfg"
#define DEV_SV_PATH                      "./vs.cfg"
#define DEV_TIME_SNS                 dev_ss_time[dev_cfg.level_ss_tim]
#define DEV_VOL_TEST(x)              (100-(dev_vol_play[x]))//modified by hgj
#define DEV_VOL_PLAY                 (100-(dev_vol_play[dev_cfg.level_volume]))  //modified by hgj
#if 0
#define DEV_VOL_RX                   (118-(dev_vol_rx[dev_cfg.level_vol_rx] & 0x7F))    //dev_vol_vol_rx[dev_cfg.level_vol_rx]
#define DEV_VOL_RX_2                 (118-(dev_vol_rx_2[dev_cfg.level_vol_rx] & 0x7F))
#define DEV_VOL_TX                   dev_vol_tx[dev_cfg.level_vol_tx]
#define DEV_VOL_TX_2                 dev_vol_tx_2[dev_cfg.level_vol_tx]
#else
//#define DEV_VOL_RX                  (((dev_cfg.level_vol_rx + 10) > 116) ? 116 : (dev_cfg.level_vol_rx + 10))
#define DEV_VOL_RX                  (((dev_cfg.level_vol_rx + 10) > 100) ? 100 : (dev_cfg.level_vol_rx + 10))
#define DEV_VOL_RX_2                (((dev_cfg.level_vol_rx + 30) > 116) ? 116 : (dev_cfg.level_vol_rx + 30))
#define DEV_VOL_TX_2                ((dev_cfg.level_vol_tx > 100) ? 100 : dev_cfg.level_vol_tx)
#define DEV_VOL_TX                  (DEV_VOL_TX_2 + 6)
#define MSG_VOL(x)                  ((x+16) >= 116 ? 116 : x+16)
#endif
#define IPSET_ERR_ADDR              (-1)
#define IPSET_ERR_HOST              (-2)
#define IPSET_ERR_OPEN              (-3)
#define IPSET_ERR_WRITE             (-4)

#define IP_TYPE_NORM                0
#define IP_TYPE_PRIV                1
#define IP_TYPE_MASK                2
#define IP_TYPE_DEST                3

#define IP_ERR_NEVER                0
#define IP_ERR_ILLEGAL              (-1)
#define IP_ERR_PRIV                 (-2)
#define IP_ERR_MASK                 (-3)

#define ROOM_BYTES                  4

#define IS_ADMIN(x)                 ((dev_get_type(x) == DEV_ADMIN)?1:0)
#define IS_KZQ(x)                   ((dev_get_type(x) == DEV_DOOR)?1:0)

#define DEV_VER_NORMAL              0
#define DEV_VER_INFOSTORE           1
#define DEV_VER_IM15A               2

typedef enum
{
    DEV_ADMIN,
    DEV_DOOR,
    DEV_INDOOR,
    DEV_ILLEGAL,
} DEV_T;
struct dev_config_t
{
   char            bg_pic[64];
    char            pw_manage[8];
    char            pw_public[8];
    char            pw_reserve[16];
    unsigned char   ip_server[4];
    unsigned char   ip_addr[4];
    unsigned char   ip_mask[4];
    unsigned char   ip_bcast[4];
    unsigned char   ip_gate[4];
    unsigned char   ip_mac[8];
    unsigned char   my_code[4];
    unsigned char   level_volume;
    unsigned char   level_vol_rx;
    unsigned char   recall_time;
    unsigned char   level_vol_tx;
    unsigned char   level_bright;
    unsigned char   level_ss_tim;
    unsigned char   level_reserve[4];
    unsigned char   en_user_unlock;
    unsigned char   en_mc_check;
    unsigned char   en_fc_alarm;
    unsigned char   en_reserve[4];
    unsigned char   day_night;
    unsigned char   lock_type;
    unsigned char   lock_time;
    unsigned short  delay_screen;
    unsigned short  delay_gomain;
    unsigned short  delay_doorala;
    unsigned short  camera_night;
    unsigned short  cvbs_mode;
    unsigned short  default_ver;
    unsigned short cif_mode;
    unsigned char   info_store;
    unsigned char   char_dummy;
    unsigned char   my_village_code[4]; //// 保留但不使用
    unsigned short  reserve[5];

    unsigned char   card_in_flash;
    unsigned char dns1[4];
    unsigned char dns2[4];
    unsigned char  addr_gw[6];
    unsigned char  level_ble_tx_dbm;
    unsigned char lift_ip[4][4];  //add for multi_lift_ctl
};

struct dev_sv_t
{
    unsigned char   my_village_code[4];
    unsigned char   has_set;
    unsigned char   reserve[11];
    unsigned char   dev_seed_data[16];
};
#define DEV_VER_NORMAL              0
#define DEV_VER_INFOSTORE           1
#define DEV_VER_VLAN                2
#define DEV_VER_EXPORT              3
#define DEV_VER_BLE				4

extern const unsigned char dev_vol_rx[], dev_vol_rx_2[];
extern const unsigned char dev_vol_tx[], dev_vol_tx_2[];
extern const unsigned char dev_vol_play[];
extern const unsigned int  dev_ss_time[];
extern struct dev_config_t dev_cfg;
extern struct dev_sv_t      vs_cfg;

extern DEV_T dev_get_type(unsigned char room[]);
extern void dev_sys_reboot(void);
extern void dev_app_restart(void);
extern void dev_flash_protect(int wp);
//extern void my_village_code_to_Ble(void);
extern void my_new_village_code_to_Ble(void);
extern void dev_code_cardreader(void);
extern char *dev_software_ver(void);
extern int  dev_check_ip(const char *ip_str, unsigned char *ip_out, int ip_type);
extern int  dev_set_ip(char *ip_addr, char *ip_mask, char *ip_gate);
extern int  dev_set_time(int tm_year, int tm_mon, int tm_mday, int tm_hour, int tm_min, int tm_sec);
extern void dev_config_get(void);
extern void dev_config_save(void);
extern int  global_data_load(void);
extern int  global_data_save(void);
extern int  eth_para_save(eth_para_t *para);
extern int  is_have_infostore(void);
extern void dev_sv_get(void);
extern void dev_set_village(unsigned char village1,unsigned char village2,unsigned char village3);
extern void dev_code_cardreader_test(void);
#if defined (__cplusplus)
}
#endif
 
#endif
