/*
 ============================================================================
 Name        : dev_config.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 设备参数配置
  ============================================================================
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "dev_config.h"
#include "global_data.h"
#include "ansi_char.h"
#include "public.h"
#include "cpld.h"
#include "uart.h"
//#include "spi.h" //del by wrm 20141121
#include "key.h"

#include "msg.h"
#include "tim.h"
#include "video_send.h"
#include "aes.h"
#include "rtc.h"
//aic3007 reg value

const unsigned char dev_vol_play[16] = {70, 62, 54, 46, 40, 36, 32, 28, 25, 22, 18, 14, 11, 7, 4, 0};  
//const unsigned char dev_vol_rx[10]   = {0xA5, 0xA1, 0x9D, 0x9A, 0x98, 0x96, 0x93, 0x8F, 0x8A, 0x85};  //one speaker
const unsigned char dev_vol_rx[10]   = {0xA9, 0xA5, 0xA1, 0x9D, 0x9A, 0x98, 0x96, 0x93, 0x8F, 0x8A};  //df2013
const unsigned char dev_vol_rx_2[10] = {0x9A, 0x94, 0x90, 0x8D, 0x8B, 0x89, 0x87, 0x85, 0x83, 0x81};  //df2100
//const unsigned char dev_vol_tx[10]   = {36, 39, 42, 44, 46, 48, 50, 53, 56, 60};  //one speaker
const unsigned char dev_vol_tx[10]   = {23, 28, 32, 36, 39, 42, 45, 48, 52, 56};  //df2013
const unsigned char dev_vol_tx_2[10] = {22, 26, 29, 32, 34, 36, 39, 43, 47, 51};  //df2100
const unsigned int  dev_ss_time[3] = {0, TIME_1S(60), TIME_1S(5*60)};


struct dev_config_t dev_cfg;
struct dev_sv_t      vs_cfg;
DEV_T dev_get_type(unsigned char room[])
{
    if ((room[0]|room[1]|room[2]) == 0x00)
        return DEV_ADMIN;
    else
    if (room[2] == 0x00)
        return DEV_DOOR;
    else
    if (room[0]|room[1])
        return DEV_INDOOR;
    else
        return DEV_ILLEGAL;
}

void dev_sys_reboot(void)
{
    //printf("SYS confirm before\n");
    key_buzz(BUZZ_CONFIRM);
   // printf("SYS confirm after\n");
    usleep(100000);
    Set_Back_Light(0);//cpld_io_clr(EIO_LCD_BACKLIGHT);
    /*cpld_io_clr(EIO_LCD_AVDD);
    cpld_io_set(EIO_LCD_DVDD);*/
    usleep(100000);
    cpld_close();
    system( "if test -e /opt/app/res_update.tar.gz\n"
            "then\n"
            "tar xzf /opt/app/res_update.tar.gz -C /opt/\n"
            "rm /opt/app/res_update.tar.gz\n"
            "fi\n"
            "sync\n");
    usleep(300000);
    system("reboot");
}

void dev_app_restart(void)
{
    usleep(80000);
    //printf("confirm before\n");
    key_buzz(BUZZ_CONFIRM);
    //printf("confirm after\n");
    usleep(200000);
    cpld_close();
    usleep(300000);
    proc_signal(0);
    exit(0);
}

char *dev_software_ver(void)
{
    static char software_ver[32], got_flag = 0;
    const  char *mon_tbl[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char mon_str[6];
    int  mon, day, year;

    if (got_flag == 1)
        return software_ver;

    sscanf(__DATE__, "%s %d %d", mon_str, &day, &year);
    for (mon = 0; mon < 12; mon++) {
        if (0 == strcmp(mon_str, mon_tbl[mon])) 
            break;
    }

#ifdef FRONT_DOOR
    sprintf(software_ver, "%04d%02d%02d-15CB", year, mon+1, day);
#else
    sprintf(software_ver, "%04d%02d%02d-15C",  year, mon+1, day);
#endif
    got_flag = 1;
    return software_ver;
}

void dev_code_cardreader(void)
{
    unsigned char buf[10];

    Set_SEL_S_M(1);
    memset(buf, 0x00, sizeof(buf));
    buf[0] = 0x21;
    buf[1] = dev_cfg.my_code[0];
    buf[2] = dev_cfg.my_code[1];
    buf[5] = dev_cfg.my_code[3];
/*
    if(dev_cfg.card_in_flash){
	    buf[5] = dev_cfg.my_code[3]+1;      //卡头不能为主，不然不发卡号出来判断了
	}*/
#ifdef   FRONT_DOOR
    buf[6] = 0xBB;
#endif     
    uart_send_data(UART_1, buf);
    //usleep(150*1000);
}

void dev_code_cardreader_test(void)
{
    unsigned char buf[10];

    Set_SEL_S_M(1);//cpld_io_set(EIO_CARD_CODE_EN);
    memset(buf, 0x00, sizeof(buf));
    buf[0] = 0x21;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[5] = 0x00;

    if(dev_cfg.card_in_flash){
	    buf[5] = dev_cfg.my_code[3]+1;      //卡头不能为主，不然不发卡号出来判断了
	}
#ifdef   FRONT_DOOR
    buf[6] = 0xBB;
#endif     
    uart_send_data(UART_1, buf);
    //usleep(150*1000);
}

/*
void my_village_code_to_Ble(void)
{
    unsigned char buf[10];
    cpld_io_set(EIO_CARD_CODE_EN);
    memset(buf, 0x00, sizeof(buf));
 //   buf[0] = 0x22;
  //  buf[1] = dev_cfg.my_village_code[0];
   // buf[2] = dev_cfg.my_village_code[1];
   // buf[3] = dev_cfg.my_village_code[2];
   // buf[4] = dev_cfg.my_village_code[3];

    buf[0] = 0x22;
    buf[1] = vs_cfg.my_village_code[0];
    buf[2] = vs_cfg.my_village_code[1];
    buf[3] = vs_cfg.my_village_code[2];
    buf[4] = vs_cfg.my_village_code[3];
	
    uart_send_data(UART_1, buf);
    //usleep(150*1000);
}
*/
void my_new_village_code_to_Ble(void)
{
    unsigned char buf[10];
    Set_SEL_S_M(1);//cpld_io_set(EIO_CARD_CODE_EN);
    memset(buf, 0x00, sizeof(buf));
    buf[0] = 0x22;
    buf[1] = vs_cfg.my_village_code[0];
    buf[2] = vs_cfg.my_village_code[1];
    buf[3] = vs_cfg.my_village_code[2];
    buf[4] = vs_cfg.my_village_code[3];
    uart_send_data(UART_1, buf);
    //usleep(150*1000);
}

void dev_flash_protect(int wp)
{
/*
    if (wp) {
        usleep(100000);
        cpld_io_clr(EIO_FLASH_PROTECT);
    }
    else {
        cpld_io_set(EIO_FLASH_PROTECT);
    }
    */
}

int dev_check_ip(const char *ip_str, unsigned char *ip_out, int ip_type)
{
    int i;
    int result = IP_ERR_NEVER;
    
    unsigned int  len = strlen(ip_str);
    unsigned int  uiip[4];
    unsigned char ucip[4];
    
    if (ip_out != NULL)
        memset(ip_out, 0, 4);
    
    if ((len > 15) || (len < 5))
        return IP_ERR_ILLEGAL;
    if (sscanf(ip_str, "%u.%u.%u.%u", &uiip[0], &uiip[1], &uiip[2], &uiip[3]) != 4)
        return IP_ERR_ILLEGAL;
    if ((uiip[0] > 255) || (uiip[1] > 255) || (uiip[2] > 255) || (uiip[3] > 255))
        return IP_ERR_ILLEGAL;
     
    for (i = 0; i < 4; i++)
        ucip[i] = uiip[i];  
        
    if (ip_type == IP_TYPE_PRIV || ip_type == IP_TYPE_DEST) {
        result = IP_ERR_PRIV;
        switch(uiip[0]) {
            case 10:
                result = IP_ERR_NEVER;
                break;
            case 172:
                if ((uiip[1] >= 16) && (uiip[1] <= 31))
                    result = IP_ERR_NEVER;
                break;
            case 192:
                if (uiip[1] == 168)
                    result = IP_ERR_NEVER;
                break;
            case 0:
                if(ip_type == IP_TYPE_DEST){//不往外发包时可设置成0.0.0.0
                    if(!(uiip[1] || uiip[2] || uiip[3])){
                        result = IP_ERR_NEVER;
                    }
                }
                break;
            default:
                break;
        }
    }
    else
    if (ip_type == IP_TYPE_MASK) {
        unsigned long lip  = ucip[0] << 24 | ucip[1] << 16 | ucip[2] << 8 | ucip[3];
        unsigned char pos0 = 0;
        for (i = 31; i >= 0; i--) {
            if ((lip&(unsigned long)(1<<i)) == 0UL)
                pos0 = i;
            else
            if (pos0 > i)
                return IP_ERR_MASK;
        }
    }
    
    if (result == IP_ERR_NEVER) {
        if (ip_out != NULL) {
            memcpy(ip_out, ucip, 4);
        }
    }

    return result;
}

int dev_set_ip(char *ip_addr, char *ip_mask, char *ip_gate)
{
    char ip_text[300];
    char ip_network[16];
    unsigned char ucip[4];
    unsigned char mask[4];

    if (sscanf(ip_addr, "%hhu.%hhu.%hhu.%hhu", &ucip[0], &ucip[1], &ucip[2], &ucip[3]) != 4)
        return IPSET_ERR_ADDR;
    if (sscanf(ip_mask, "%hhu.%hhu.%hhu.%hhu", &mask[0], &mask[1], &mask[2], &mask[3]) != 4)
        return IPSET_ERR_ADDR;

    unsigned long lip   = ucip[0] << 24 | ucip[1] << 16 | ucip[2] << 8 | ucip[3];
    unsigned long lmask = mask[0] << 24 | mask[1] << 16 | mask[2] << 8 | mask[3];
    unsigned long lhost = (lip & (~lmask)) & (~lmask);
    unsigned long lnetwork = lip & lmask;
    if ((lhost == 0) || (lhost == (~lmask))) 
        return IPSET_ERR_HOST;

    memcpy(ucip, &lnetwork, 4);
    sprintf(ip_network, "%hhu.%hhu.%hhu.%hhu", ucip[3], ucip[2], ucip[1], ucip[0]);
    sprintf(ip_text, "# The loopback interface \nauto lo\niface lo inet loopback\n\
    # Wired or wireless interfaces\nauto eth0\niface eth0 inet static\n\taddress %s\n\tnetmask %s\n\tnetwork %s\n\tgateway %s\n\n", \
    ip_addr, ip_mask, ip_network, ip_gate);

    dev_flash_protect(0);
    int fd = open("/etc/network/interfaces", O_WRONLY|O_TRUNC|O_CREAT, 0777);
    if (fd < 0)
        return IPSET_ERR_OPEN;
    if (write(fd, ip_text, strlen(ip_text)) < strlen(ip_text)){
        close(fd);
        return IPSET_ERR_WRITE;
    }
    fdatasync(fd);
    close(fd);
    dev_flash_protect(1);
    return 0;
}

int dev_set_time(int tm_year, int tm_mon, int tm_mday, int tm_hour, int tm_min, int tm_sec)
{
    time_t time;
    struct tm tnow;

    tnow.tm_year = tm_year - 1900;
    tnow.tm_mon  = tm_mon  - 1;
    tnow.tm_mday = tm_mday;
    tnow.tm_hour = tm_hour;
    tnow.tm_min  = tm_min;
    tnow.tm_sec  = tm_sec;

    time = mktime(&tnow);
    if(stime(&time) != 0) {
       perror("stime error:");
       return -1;
    }
    //save_local_time(time);
    set_rtc_time(tnow);
    /*system("hwclock -w");
    system("/etc/rc6.d/S25save-rtc.sh");*/
    return 0;
}

int dev_get_ip(unsigned char *addr, unsigned char *mask, unsigned char *bcast, unsigned char *gate, unsigned char *mac)
{
    int    ret  = 0;
    int    sfd  = -1;
    char   rbuf[128];
    char   iface[16];
    FILE   *fp_route;
    unsigned long s_dest = INADDR_NONE;
    unsigned long s_gate = INADDR_NONE;   
 
    struct ifreq    ifr;
    struct in_addr  in_addr_ip, in_addr_mask, in_addr_bcast;

    in_addr_ip.s_addr   = INADDR_NONE; 
    in_addr_mask.s_addr = INADDR_NONE; 

    strcpy(ifr.ifr_name, "eth0");
    if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        ret |= (0x01|0x02|0x04);
        goto GET_GATEWAY;
    }   

    if (ioctl(sfd, SIOCGIFADDR, &ifr)) {
        ret |= 0x01;
        goto GET_MASK;
    }
    in_addr_ip.s_addr = (((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr).s_addr;
GET_MASK:
    if (ioctl(sfd, SIOCGIFNETMASK, &ifr)) {
        ret |= 0x02;
        goto GET_BCAST;
    }
    in_addr_mask.s_addr = (((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr).s_addr;
GET_BCAST:
    if (ioctl(sfd, SIOCGIFBRDADDR, &ifr)) {
        ret |= 0x04;
        goto GET_MAC;
    }
    in_addr_bcast.s_addr = (((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr).s_addr;
GET_MAC:
    if (ioctl(sfd, SIOCGIFHWADDR, &ifr)) {
        ret |= 0x08;
        goto GET_GATEWAY;
    }
GET_GATEWAY:
    if (sfd > 0)  close(sfd);
    
    fp_route = fopen("/proc/net/route", "r");
    if (fp_route == NULL) {
        ret |= 0x10;
    }

    fgets(rbuf, sizeof(rbuf), fp_route);
    while(fgets(rbuf, sizeof(rbuf), fp_route)) {
        if ((sscanf(rbuf, "%s\%lX\t%lX", iface, &s_dest, &s_gate)!=3) || (s_dest != 0))
            continue;
        break;
    }
    if (fp_route != NULL)   fclose(fp_route);

    if (ret & 0x01)
        memset(addr, 0, 4);
    else 
        memcpy(addr, &in_addr_ip.s_addr, 4);

    if (ret & 0x02)
        memset(mask, 0, 4);
    else 
        memcpy(mask, &in_addr_mask.s_addr, 4);

    if (ret & 0x04)
        memset(bcast, 0, 6);
    else 
        memcpy(bcast, &in_addr_bcast.s_addr, 4);

    if (ret & 0x08)
        memset(mac, 0, 6);
    else 
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6); 

    if (ret & 0x10)
        memset(gate, 0, 4);
    else
        memcpy(gate, &s_gate, 4);

    return (0-ret);
}


///**for new config***********************/
unsigned char key_dev[16] = {0};
unsigned char enbuf[32] = {0};
unsigned char file_key[16]="anjubao30015zj15";
static void encrypt_dev_data(void)
{
    int i = 0;
    unsigned char buf[32] = {0};
    memcpy(buf,&vs_cfg,sizeof(vs_cfg));

    printf("vs_cfg :%02X %02X %02X %02X %02X %02X \n",vs_cfg.my_village_code[0],vs_cfg.my_village_code[1],\
		vs_cfg.my_village_code[2],vs_cfg.reserve[3],vs_cfg.reserve[4],vs_cfg.reserve[5]);
    printf("sv buf :****");
    for(i = 16;i<25;i++){
		printf("%c",buf[i]);
	}
    printf("*****\n");
    AES128_ECB_encrypt(buf,file_key,enbuf);
    AES128_ECB_encrypt(&buf[16],file_key,&enbuf[16]);
    printf("en buf :");
    for(i = 0;i<32;i++){
		//printf("%02x ",enbuf[i]);
		printf("*");
	}
    printf("\n");

    
}

static void decrypt_dev_data(unsigned char *buf)
{
    int i = 0;
    unsigned char debuf[32] = {0};
    printf("read buf :");
    for(i = 0;i<32;i++){
		printf("%02x ",buf[i]);
	}
    printf("\n");
    AES128_ECB_decrypt(buf,file_key,debuf);
    AES128_ECB_decrypt(&buf[16],file_key,&debuf[16]);
    printf("decrypt buf :");
    for(i = 0;i<32;i++){
		printf("%02x ",debuf[i]);
	}
    printf("\n");
    //loading vs_cfg_data
    vs_cfg.my_village_code[0] = debuf[0];
    vs_cfg.my_village_code[1] = debuf[1];  
    vs_cfg.my_village_code[2] = debuf[2];
    vs_cfg.my_village_code[3] = debuf[3];
    vs_cfg.has_set = debuf[4];
    memcpy(vs_cfg.reserve,&debuf[5],11);
    memcpy(vs_cfg.dev_seed_data,&debuf[16],16);
}


static void dev_newcfgfile_save(void)
{
    dev_flash_protect(0);
    int fd = open(DEV_SV_PATH, O_WRONLY|O_TRUNC|O_CREAT, 0664);
    if (fd < 0) {
        app_debug(DBG_FATAL, "Cannot opn config file:%s for write\n", DEV_SV_PATH);
        return;
    }
    encrypt_dev_data();
    //write(fd, &vs_cfg, sizeof(vs_cfg)); 
    write(fd, &enbuf, sizeof(enbuf)); 
    fdatasync(fd);
    close(fd);
    dev_flash_protect(1);
}

void dev_sv_get(void){
	unsigned char  default_seed_data[16] ="guangdonganjubao";
	unsigned char  readbuf[32] = {0};
	FILE *fp = fopen(DEV_SV_PATH,"r");
	if(fp == NULL){
		app_debug(DBG_FATAL, "Cannot opn config file:%s for read\n", DEV_SV_PATH);
		vs_cfg.my_village_code[0] = 0xff;
		vs_cfg.my_village_code[1] = 0xff;
		vs_cfg.my_village_code[2] = 0xff;
		vs_cfg.my_village_code[3] = 0xff;
		vs_cfg.has_set = 0;
		memset(vs_cfg.reserve,0xFF,11);
		memcpy(vs_cfg.dev_seed_data,default_seed_data,16);
              dev_newcfgfile_save();
	}
	else {
        fread(readbuf, sizeof(vs_cfg), 1, fp); 
        decrypt_dev_data(readbuf);
        fclose(fp);
    }
    printf("my_village_code:%02X %02X %02X \n", vs_cfg.my_village_code[0], vs_cfg.my_village_code[1], vs_cfg.my_village_code[2]);
}

void dev_set_village(unsigned char village1,unsigned char village2,unsigned char village3){
    vs_cfg.my_village_code[0] = village1;
    vs_cfg.my_village_code[1] = village2;  
    vs_cfg.my_village_code[2] = village3;
    vs_cfg.my_village_code[3] = 0xFF;
}

///**for new config**************END*******/
static void dev_cfgfile_save(void)
{
    dev_flash_protect(0);
    int fd = open(DEV_CONFIG_PATH, O_WRONLY|O_TRUNC|O_CREAT, 0664);
    if (fd < 0) {
        app_debug(DBG_FATAL, "Cannot opn config file:%s for write\n", DEV_CONFIG_PATH);
        return;
    }
    write(fd, &dev_cfg, sizeof(dev_cfg)); 
    fdatasync(fd);
    close(fd);
    dev_flash_protect(1);
}

void dev_config_save(void)
{
    dev_newcfgfile_save();
    dev_cfgfile_save();
    global_data_load();
}


void dev_config_get(void)
{
    const unsigned char ip_server_default[4] = {192,168,14,21};
    const unsigned char ip_addr_gw_default[6] = {0,0,0,0,0,0};
    FILE *fp = fopen(DEV_CONFIG_PATH, "r");
    if (fp == NULL) {
        app_debug(DBG_FATAL, "Cannot opn config file:%s for read\n", DEV_CONFIG_PATH);
        dev_cfg.my_code[0] = 0x11;
        dev_cfg.my_code[1] = 0x01;
        dev_cfg.my_code[2] = 0x00;
        dev_cfg.my_code[3] = 0x00; 
	/* dev_cfg.my_village_code[0] = 0xff;
	 dev_cfg.my_village_code[1] = 0xff;
	 dev_cfg.my_village_code[2] = 0xff;
	 dev_cfg.my_village_code[3] = 0xff;
        */dev_cfg.en_fc_alarm     = 0;
        dev_cfg.en_mc_check     = 0;
        dev_cfg.en_user_unlock  = 0;
        dev_cfg.level_volume    = 8;
        dev_cfg.level_vol_rx    = 80;//4
        dev_cfg.level_vol_tx    = 25;//4
        dev_cfg.level_bright    = 8;
        dev_cfg.level_ss_tim    = 1;
        dev_cfg.lock_type       = 0;
        dev_cfg.lock_time       = 5;
        dev_cfg.day_night       = 0xFF;
        dev_cfg.delay_screen    = 60;
        dev_cfg.delay_gomain    = 60;
        dev_cfg.delay_doorala   = 0;
        dev_cfg.camera_night    = 1;
	 dev_cfg.cif_mode        =1;
        dev_cfg.cvbs_mode       = 0;
#ifdef RecallTime      
        dev_cfg.recall_time     =20;   //云对讲无应答转呼时间
#endif        
        //dev_cfg.default_ver     = DEV_VER_NORMAL;//****del
	 dev_cfg.default_ver     = DEV_VER_VLAN;
        dev_cfg.level_ble_tx_dbm=3;
        dev_cfg.info_store      = 0;
	dev_cfg.card_in_flash   = 0;

        strcpy(dev_cfg.pw_manage, "8888");
        strcpy(dev_cfg.pw_public, "111111");
        strcpy(dev_cfg.bg_pic, "./pic/BG.png");
        memcpy(dev_cfg.ip_server, ip_server_default, 4);

        memcpy(dev_cfg.addr_gw, ip_addr_gw_default,6);
    	 memcpy(dev_cfg.dns1, ip_addr_gw_default, 4);
        memcpy(dev_cfg.dns2, ip_addr_gw_default, 4);
        
        dev_code_cardreader();
        dev_cfgfile_save();
    }
    else {
        int len = fread(&dev_cfg, 1,sizeof(dev_cfg), fp);
        if(len != sizeof(dev_cfg)){
            printf("warning: parm.cfg's length isn't equal to dev_cfg,read length = %d!\n",len);
        }
        fclose(fp);
    }
    printf("my_code:%02X %02X %02X %02X\n", dev_cfg.my_code[0], dev_cfg.my_code[1], dev_cfg.my_code[2], dev_cfg.my_code[3]);
    	printf("my_village_code:");
		int i = 0;
		for(i=0;i<4;i++)
			printf("%02x ",vs_cfg.my_village_code[i]);
		printf("\n");
	if (dev_cfg.my_code[0] == 0x00) {
        dev_cfg.my_code[0] = 0x11;
        dev_cfg.my_code[1] = 0x01;
        dev_cfg.my_code[2] = 0x00;
        dev_cfg.my_code[3] = 0x00; 
    }
    dev_cfg.my_code[2] = 0x00;
#ifdef FRONT_DOOR
    dev_cfg.my_code[3] = 0x00;
#endif
    if (dev_cfg.level_volume > 15) 
        dev_cfg.level_volume = 8;
    if (dev_cfg.level_bright > 15) 
        dev_cfg.level_bright = 8;
    if (dev_cfg.level_ss_tim > 2)
        dev_cfg.level_ss_tim = 2;
    if (strlen(dev_cfg.pw_manage) < 4)
        strcpy(dev_cfg.pw_manage, "8888");
    if (strlen(dev_cfg.pw_public) < 6)
        strcpy(dev_cfg.pw_public, "111111");
    if (dev_cfg.delay_doorala > 99)
        dev_cfg.delay_doorala = 99;
    if (dev_cfg.lock_time > 30)
        dev_cfg.lock_time = 30;
    else
    if (dev_cfg.lock_time < 1)
        dev_cfg.lock_time = 1;
    if (dev_cfg.delay_gomain < 10)
        dev_cfg.delay_gomain = 10;
     dev_get_ip(dev_cfg.ip_addr, dev_cfg.ip_mask, dev_cfg.ip_bcast, dev_cfg.ip_gate, dev_cfg.ip_mac);

     if (dev_cfg.default_ver != DEV_VER_BLE) {
        dev_cfg.default_ver  = DEV_VER_BLE;
	 /*dev_cfg.my_village_code[0] = 0xff;
	 dev_cfg.my_village_code[1] = 0xff;
	 dev_cfg.my_village_code[2] = 0xff;
	 dev_cfg.my_village_code[3] = 0xff;*/
        dev_code_cardreader();
        dev_cfgfile_save();
    }
}

int global_data_load(void)
{
    P_GateData pData = &gData;

    pData->DoorNo[0] = dev_cfg.my_code[0];
    pData->DoorNo[1] = dev_cfg.my_code[1];
    pData->DoorNo[2] = dev_cfg.my_code[3];
    
/*
    if (!(dev_cfg.ip_server[0]|dev_cfg.ip_server[1]|dev_cfg.ip_server[2]|dev_cfg.ip_server[3])) {
        dev_cfg.ip_server[0] = 192;
        dev_cfg.ip_server[1] = 168;
    }
*/
    //memcpy(pData->My_Village_No,dev_cfg.my_village_code,4);
    //for new ble ..........
    memcpy(pData->My_Village_No,vs_cfg.my_village_code,4);
    memcpy(pData->MyIP,     dev_cfg.ip_addr,   4);
    memcpy(pData->MyMac,    dev_cfg.ip_mac,    6);
    memcpy(pData->mdv_gw,   dev_cfg.ip_gate,   4);
    memcpy(pData->mdv_mask, dev_cfg.ip_mask,   4);
    memcpy(pData->ServerIP, dev_cfg.ip_server, 4);

    memcpy(pData->Dns1, dev_cfg.dns1,   4);
    memcpy(pData->Dns2, dev_cfg.dns2, 4);
    
    strcpy((char *)pData->PubPassword[0], dev_cfg.pw_public);
    strcpy((char *)pData->PubPassword[1], "666666");
    strcpy((char *)pData->PubPassword[2], dev_cfg.pw_manage);
    strcpy((char *)pData->PubPassword[3], "999999");
    strcpy((char *)pData->MAC_PassWord1,  "AJB888");
    strcpy((char *)pData->MAC_PassWord2,  "336226");
    strcpy((char *)pData->IP_PassWord1,   "AJB888");
    strcpy((char *)pData->IP_PassWord2,   "622633");
    strcpy((char *)pData->AnimaWord,      PDU_ANIMAWORD);
    pData->MAC_PassWordTime     = 0;
    pData->IP_PassWordTime      = 0;
    pData->DayOrNight           = dev_cfg.day_night;
    pData->Codeflag             = CON_4;
    pData->ScreenDelay          = dev_cfg.delay_screen;//DEV_TIME_SNS/TIME_1S(1);
    pData->GoMainScreenDelay    = dev_cfg.delay_gomain;

     #ifdef CFG_SUPPORT_INFO_STORE
    pData->InfoStor             = dev_cfg.info_store;
#endif
    memcpy(pData->addr_gw, dev_cfg.addr_gw,6);
    return 0;
}

int global_data_save(void)
{
    char ip_server[32];

    P_GateData pData = &gData;
    pData->DoorNo[2] = (pData->DoorNo[2]/10)*16+(pData->DoorNo[2]%10);
    if (pData->DoorNo[0] != 0x00) {
        if(!((dev_cfg.my_code[0]== pData->DoorNo[0])&&(dev_cfg.my_code[1] ==pData->DoorNo[1])&&(dev_cfg.my_code[3] == pData->DoorNo[2])))
        {
             delete_sfz8or4_cardreader_list();
        }
        dev_cfg.my_code[0] = pData->DoorNo[0];
        dev_cfg.my_code[1] = pData->DoorNo[1];
        dev_cfg.my_code[2] = 0x00;
        dev_cfg.my_code[3] = pData->DoorNo[2];
    }
	memcpy(dev_cfg.my_village_code,pData->My_Village_No,4);
	printf("dev_cfg.my_village_code:");
	int i = 0;
	for(i=0;i<4;i++)
		printf("%02x ",dev_cfg.my_village_code[i]);
	printf("\n");
#ifdef FRONT_DOOR
    dev_cfg.my_code[3] = pData->DoorNo[2] = 0x00;
#endif
    sprintf(ip_server, "%hhu.%hhu.%hhu.%hhu", \
        pData->ServerIP[0], pData->ServerIP[1], pData->ServerIP[2], pData->ServerIP[3]);

    if (dev_check_ip(ip_server, NULL, IP_TYPE_NORM))
        printf("ip_server check error!\n");
    else {
        memcpy(dev_cfg.ip_server, pData->ServerIP, 4);
        printf("dev_cfg.ip_server:%hhu.%hhu.%hhu.%hhu\n", \
            dev_cfg.ip_server[0], dev_cfg.ip_server[1], dev_cfg.ip_server[2], dev_cfg.ip_server[3]);
    } 

    strcpy(dev_cfg.pw_public, (char *)pData->PubPassword[0]);
    strcpy(dev_cfg.pw_manage, (char *)pData->PubPassword[2]);
    dev_cfg.day_night    = pData->DayOrNight;
    dev_cfg.delay_screen = pData->ScreenDelay;
    dev_cfg.delay_gomain = pData->GoMainScreenDelay;
    //pData->Codeflag         = CON_4;
    
    dev_cfg.info_store   = pData->InfoStor;	
    memcpy(dev_cfg.addr_gw,pData->addr_gw, 6);
    
    dev_code_cardreader();
    dev_cfgfile_save();
   // key_buzz(BUZZ_CONFIRM);//wrm 20150106 ipinstall setting configuration sucess beep

    return 0;
}

int eth_para_save(eth_para_t *para)
{
    char ip_addr[32], ip_gate[32], ip_mask[32];

    sprintf(ip_addr, "%hhu.%hhu.%hhu.%hhu", \
        para->ip_addr[0], para->ip_addr[1], para->ip_addr[2], para->ip_addr[3]);
    sprintf(ip_gate, "%hhu.%hhu.%hhu.%hhu", \
        para->gateway_addr[0], para->gateway_addr[1], para->gateway_addr[2], para->gateway_addr[3]);
    sprintf(ip_mask, "%hhu.%hhu.%hhu.%hhu", \
        para->subnet_mask[0], para->subnet_mask[1], para->subnet_mask[2], para->subnet_mask[3]);

    if (dev_check_ip(ip_addr, para->ip_addr, IP_TYPE_NORM) || dev_check_ip(ip_gate, para->gateway_addr, IP_TYPE_NORM) || dev_check_ip(ip_mask, para->subnet_mask, IP_TYPE_MASK))
        return -1;    
    
    memcpy(dev_cfg.ip_addr, para->ip_addr,      4);
    memcpy(dev_cfg.ip_gate, para->gateway_addr, 4);
    memcpy(dev_cfg.ip_mask, para->subnet_mask,  4);

    if (dev_set_ip(ip_addr, ip_mask, ip_gate)) {
        printf("dev_set_ip error\n");    
        return -1;    
    }

    usleep(80000);
    memcpy(dev_cfg.dns1, para->dns1,  4);
    memcpy(dev_cfg.dns2, para->dns2,  4);

    dev_cfgfile_save();
	system("/etc/init.d/S40network restart");
    key_buzz(BUZZ_CONFIRM);
    sleep(1);
    cpld_close();
    usleep(300000);
    exit(0);
     return 0;
     #if 0
 	system("/etc/init.d/S40network restart");
    sleep(1);
    key_buzz(BUZZ_CONFIRM);
    sleep(1);
    exit(0);
    
    return 0;
    #endif
}
int is_have_infostore(void)
{

#ifdef CFG_SUPPORT_INFO_STORE
	return (dev_cfg.info_store != 0) ? 1 : 0;
#else
    return 0;
#endif
}

