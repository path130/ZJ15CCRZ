/*
 ============================================================================
 Name        : key.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : keyboard
  ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "public.h"
#include "msg.h"
#include "key.h"
#include "ui.h"
#include "auto_test.h"
#include "tim.h"
#include "cpld.h"
#include <linux/i2c-dev.h>
#include "dpgpio.h" //digital power

#define DEBOUNCE_TIME   4
extern  void SetGpioVal(int , int );
extern int GetGpioVal(int );
extern  int InitGpio(int, int, int);
extern  void DeinitGpio(int hGpio);

typedef struct
{
    int     debounce_h;
    int     debounce_l;
    int     status;
} IOSCAN;

static IOSCAN      io_door, io_ul_key, io_detach,io_body;
static pthread_t   key_tid, buzz_tid;
static pipe_handle buzz_pipe_buzz;
static volatile    int buzzing;
static int         tim_EMC_reset_speech;

//KEY_F1 "C"取消，KEY_F2 "开锁"公共密码开锁，KEY_F3 "OK"确认，KEY_F4 "管理处"
#if 0
static const char key_table[] = {
    KEY_X,  KEY_9,  KEY_Y,  KEY_0,
    KEY_5,  KEY_8,  KEY_7,  KEY_6, 
    KEY_3,  KEY_4,  KEY_2,  KEY_1,
    KEY_F1, KEY_F2, KEY_F4, KEY_F3
};
#else
static const char key_table[] = {//modify by wrm 20150723 使用15B按键程序
   	KEY_4,  KEY_1,   KEY_7,  KEY_X,
    KEY_2,  KEY_0,  KEY_8,   KEY_5, 
    KEY_Y,  KEY_9,  KEY_3,   KEY_6,
    KEY_F2,  KEY_F4,   KEY_F1,  KEY_F3
};
#endif


/*void BEEP(void)//by mkq 20170907
{
  int m_hbeep;
  m_hbeep =InitGpio(71,1,0);
  SetGpioVal(m_hbeep,1);
  usleep(50*1000);
  SetGpioVal(m_hbeep,0);
  
 }*/
/* 优先级高，不会被打断，但会占用调用线程的时间 */
void key_buzz(BUZZ_TYPE type)
{
    
    unsigned char warn_cnt = 3;
    buzzing = 0;
    switch(type) {
        case BUZZ_KEY:
            //cpld_io_set(EIO_BUZZER);
            Set_keybuzz(1);
            usleep(50000);
            //cpld_io_clr(EIO_BUZZER);
            Set_keybuzz(0);
            break;
        case BUZZ_BEEP:
            //cpld_io_set(EIO_BUZZER);
            Set_keybuzz(1);
            usleep(180000);
            //cpld_io_clr(EIO_BUZZER);
            Set_keybuzz(0);
            break;
        case BUZZ_CONFIRM:
            //cpld_io_set(EIO_BUZZER);
            Set_keybuzz(1);
            usleep(650000);
            //cpld_io_clr(EIO_BUZZER);
            Set_keybuzz(0);
            break;
        case BUZZ_WARNING:
            while (warn_cnt--) {
                //cpld_io_set(EIO_BUZZER);
                Set_keybuzz(1);
                usleep(80000);
                //cpld_io_clr(EIO_BUZZER);
                Set_keybuzz(0);
                usleep(60000);
            }
            break;
        default:
            break;
    }
}


/* 优先级低，会被打断，依赖于buzz_thread */
void buzz_play(BUZZ_TYPE type)
{
    unsigned char msg[MSG_LEN_MAX];
    
    msg[0]  = MSG_FROM_BUZZ;
    msg[1]  = type;
    buzzing = 0;
    pipe_put(buzz_pipe_buzz, msg, MSG_LEN_BUZZ);
}

static inline void buzz_wait(int msec)
{
    int m10sec = msec/10;
    while ((m10sec--) && (buzzing))
        usleep(10000);
}

static void *buzz_thread(void *arg)
{
    unsigned char warn_cnt = 3;
    unsigned char msg[MSG_LEN_MAX];

    while(1) {
        if (pipe_get(buzz_pipe_buzz, msg, MSG_LEN_BUZZ) < 0) {
            usleep(100*1000);
            continue;
        }
        buzzing  = 1;
        warn_cnt = 3;
        switch(msg[1]) {
            case BUZZ_KEY:
                //cpld_io_set(EIO_BUZZER);
                Set_keybuzz(1);
                buzz_wait(50);
                //cpld_io_clr(EIO_BUZZER);
                Set_keybuzz(0);
                break;
            case BUZZ_BEEP:
                //cpld_io_set(EIO_BUZZER);
                Set_keybuzz(1);
                buzz_wait(180);
                //cpld_io_clr(EIO_BUZZER);
                Set_keybuzz(0);
                break;
            case BUZZ_CONFIRM:
                //cpld_io_set(EIO_BUZZER);
                Set_keybuzz(1);
                buzz_wait(650);
                //cpld_io_clr(EIO_BUZZER);
                Set_keybuzz(0);
                break;
            case BUZZ_WARNING:
                while (warn_cnt--) {
                    //cpld_io_set(EIO_BUZZER);
                    Set_keybuzz(1);
                    buzz_wait(80);
                    //cpld_io_clr(EIO_BUZZER);
                    Set_keybuzz(0);
                    buzz_wait(60);
                }
                break;
            default:
                break;
        }
    }
    return THREAD_FAILURE;
}

/*static inline void keyboard_reset(void)
{
    printf("resetting stm8!\n");
    cpld_io_set(IO31);
    usleep(50000);
    cpld_io_clr(IO31);
}*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static int fd_key_int;
static int fd_key_i2c;
static pipe_handle my_pipe_ui; 
static int m_hGpioFC;//by mkq 20170914
static int m_hGpioMC;
static int m_hGpioLockSw;

void my_signal_fun(int signum)
{
    unsigned char msg[MSG_LEN_MAX];
        int i;
        int reciveint[32] = {0};//??ó|INT0~INT31

        read(fd_key_int, &reciveint, sizeof(reciveint)); //?áè??μμ?reciveint?o′???￡??áè?3¤?èsizeof(reciveint)
                        //printf("recive int\n");
// for (i = 0; i < 32; i++) {
//         printf("%d  ", reciveint[i]);
// }
// printf("\n");
        if (reciveint[16]) //GPIO_B2---->EINT16
        {
                        //printf("recive int2\n");
                        //readspidata();
                char data = 0x00;
                int ret;
                //ret = write(ifd, &data, 1);
                //printf("write:%d\n", ret);
                ret = read(fd_key_i2c, &data, 1);
//                 printf("read:%d\n", ret);
//                 printf("data:%d\n", data);


        if (data < 16) {
                    msg[0] = MSG_FROM_KEY;
                    msg[1] = MSG_UI_KEY_IN;
                    msg[2] = key_table[(int)data];
                    msg[3] = 1;
					
//                     printf("###key_thread key_code %d  key_table:%c\n",ie.code,key_table[ie.code]);
// for (i = 0; i < MSG_LEN_UI; i++) {
// 	printf("0x%02x  ", msg[i]);
// }
// printf("\n");
                    pipe_put(my_pipe_ui, msg, MSG_LEN_UI);
                    //printf("-------------------%s_%d\n", __func__, __LINE__);//by hgj 
                        
                    msg[3] = 0;
                    pipe_put(my_pipe_ui, msg, MSG_LEN_UI); 
                    //printf("-------------------%s_%d\n", __func__, __LINE__);//by hgj                      
                    key_buzz(BUZZ_KEY);//by mkq
        }


        }

}

int reset_ir_detect_status()
{
    memset(&io_body,0,sizeof(io_body));
}

static void *key_thread(void *arg)
{
    struct msg_t *p_msg = (struct msg_t *)arg;
    unsigned char msg[MSG_LEN_PROC];
    unsigned char key_code   = 0xFA;
    unsigned char norm_delay = 1;
    unsigned char time_25ms  = 0;
    //unsigned int time_25ms_1  = 0;    
    unsigned char key_press  = 0;

    pipe_handle pipe_proc = p_msg->pipe_proc; 
    pipe_handle pipe_ui   = p_msg->pipe_ui;

	my_pipe_ui =  pipe_ui;

//     cpld_i2c_init();
    sleep(2);
    atest_check_resume();
   
//test key
        char stat;
        int ret;
        GPIO_CFG gpio_cfg;

///////////power the touch keyboard
        int fd_key_pwr = open("/dev/gpio", O_RDWR);
        if (fd_key_pwr < 0) perror("open gpio");
        memset(&gpio_cfg, 0, sizeof(gpio_cfg));
        gpio_cfg.gpio_num = GPIO_B14;
        gpio_cfg.gpio_cfg = 1;
        ioctl(fd_key_pwr, IOCTRL_GPIO_INIT, &gpio_cfg);
        stat = 0;
        ioctl(fd_key_pwr, IOCTRL_SET_GPIO_VALUE, &stat);

#if 1 
////////i2c touch keyboard init
        int Oflags;

        signal(SIGIO, my_signal_fun);

        unsigned char setmode = NEGATIVE_EDGE;
        //GPIO_CFG gpio_cfg;

        memset(&gpio_cfg,0,sizeof(gpio_cfg));
        gpio_cfg.gpio_num = GPIO_B2;
        gpio_cfg.gpio_cfg = 6;

        fd_key_int = open("/dev/gpio", O_RDWR,0);
        if (fd_key_int < 0) {
                printf("open /dev/gpio  fail!\n");
                return -1 ;
        }
        if(ioctl(fd_key_int,IOCTRL_GPIO_INIT,&gpio_cfg))
        {
                printf("IOCTRL_GPIO_INIT  fail error is %d\r\n");
        }

        if(ioctl(fd_key_int,IOCTRL_SET_IRQ_MODE,&setmode))
        {
                printf("IOCTRL_SET_IRQ_MODE  fail error is %d\r\n");
        }

        fcntl(fd_key_int, F_SETOWN, getpid());
        Oflags = fcntl(fd_key_int, F_GETFL); 
        fcntl(fd_key_int, F_SETFL, Oflags | FASYNC);


        fd_key_i2c = open("/dev/i2c-0", O_RDWR);
        if (fd_key_i2c < 0) perror("open i2c");

        ret = ioctl(fd_key_i2c, I2C_SLAVE, 0xa2 >> 1);
        printf("ioctl:%d\n", stat);

#endif
        m_hGpioFC = InitGpio(GPIO_G7,0,1);
        m_hGpioMC = InitGpio(GPIO_C14,0,1);
        //m_hGpioUnlock = InitGpio(77,1,dev_cfg.lock_type);
        m_hGpioLockSw = InitGpio(GPIO_C12, 0, 1);        

        int m_hGpioBS = InitGpio(GPIO_C15,0,1);
        //int m_hGpioUL = InitGpio(GPIO_C12,0,1);
        while (1)
        { 
            atest_key_put(pipe_ui);
            if (norm_delay)
                usleep(25000);
            else
                norm_delay = 1;
#if 1                                
        if ((++time_25ms & 0x7f) == 0) {//每次延时25ms*128    //del by mkq photosence has wrong
            if (get_photo_sence()){
        		gbl_data_set(&gbl_cardlight,1);//add by wrm 20150309	
        		//cpld_io_set(EIO_KEYPAD_LIGHT);//add by wrm 20150309
        		Set_CardLight(1);
        	}
            else{
		   //printf("photo sence 0!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                gbl_data_set(&gbl_cardlight,0);//add by wrm 20150309				
                //cpld_io_clr(EIO_KEYPAD_LIGHT);//add by wrm 20150309
                Set_CardLight(0);
        	}
        }
//由原理图可知IO22为门磁检测端，默认情况是处于短接状态(门已关状态)，即IO22为低电平
        if (GetGpioVal(m_hGpioMC)) {
            io_door.debounce_h = 0;
            if (++io_door.debounce_l >= DEBOUNCE_TIME) {
                if (io_door.status != DOOR_OPEN) {
                    msg[0] = MSG_FROM_KEY;
                    msg[1] = KEY_MC;
                    msg[2] = 1;
                    pipe_put(pipe_proc, msg, MSG_LEN_PROC);
                }
                io_door.status = DOOR_OPEN;
            }
        }
        else {
            io_door.debounce_l = 0;
            if (++io_door.debounce_h >= DEBOUNCE_TIME) {
                if (io_door.status != DOOR_CLOSE) {
                    msg[0] = MSG_FROM_KEY;
                    msg[1] = KEY_MC;
                    msg[2] = 0;
                    pipe_put(pipe_proc, msg, MSG_LEN_PROC);
                }
                io_door.status = DOOR_CLOSE;
            }
        }
        if (/*!cpld_io_voltage(EIO_UNLOCK_KEY)*/!GetGpioVal(m_hGpioLockSw)) {
            io_ul_key.debounce_h = 0;
            if (++io_ul_key.debounce_l >= DEBOUNCE_TIME) {
                if (io_ul_key.status)
                    ui_unlock(1);
                io_ul_key.status = 0;
            }
        }
        else {
            io_ul_key.status = 1;
            io_ul_key.debounce_l = 0;
        }
        if (!GetGpioVal(m_hGpioFC)) {//mod by wrm 20150310 防拆触发方式变化
     //   if (cpld_io_voltage(EIO_DETACH_STATE)) {
            io_detach.debounce_l = 0;
            if (++io_detach.debounce_h >= DEBOUNCE_TIME) {
                if (!io_detach.status) {
                    msg[0] = MSG_FROM_KEY;
                    msg[1] = KEY_FC;
                    pipe_put(pipe_proc, msg, MSG_LEN_PROC);
                }
                io_detach.status = 1;
            }
        }
        else {
            io_detach.status = 0;
            io_detach.debounce_h = 0;
        }

#endif


/*if(++time_25ms_1 > 240)
{
    time_25ms_1 = 0;
    if (io_body.status != 1) {
        io_body.status = 1;
        printf("GpioBS high\n");
        msg[0] = MSG_FROM_KEY;
        msg[1] = MSG_UI_BODY_SENSE;
        msg[2] = 1;
        pipe_put(pipe_ui, msg, MSG_LEN_UI);                 
    }
    else{
        io_body.status = 0;
        printf("GpioBS low\n");
        msg[0] = MSG_FROM_KEY;
        msg[1] = MSG_UI_BODY_SENSE;
        msg[2] = 0;
        pipe_put(pipe_ui, msg, MSG_LEN_UI);    
    }
}*/

            if(GetGpioVal(m_hGpioBS))
            {                
                io_body.debounce_l = 0;
                if (++io_body.debounce_h >= 12) {
                    if (io_body.status != 0) {                    
                        msg[0] = MSG_FROM_KEY;
                        msg[1] = MSG_UI_BODY_SENSE;
                        msg[2] = 0;
                        pipe_put(pipe_ui, msg, MSG_LEN_UI);                                         
                    }
                    io_body.status = 0;
                }	    
                            
            }
            else
            {                
                io_body.debounce_h = 0;
                if (++io_body.debounce_l >= 2) {
                    if (io_body.status != 1) {
                        app_debug(DBG_INFO,"GpioBS low\n");
                        msg[0] = MSG_FROM_KEY;
                        msg[1] = MSG_UI_BODY_SENSE;
                        msg[2] = 1;
                        pipe_put(pipe_ui, msg, MSG_LEN_UI);                      
                    }
                    io_body.status = 1;
                }	    
                    
            }
                             
            //sleep(1);//wait signal
        }

	close(fd_key_pwr);
	close(fd_key_int);
	close(fd_key_i2c);
    DeinitGpio(m_hGpioBS);

    exit(-1);
    return THREAD_FAILURE;
}

int key_start(void *arg)
{
    struct msg_t *p_msg = (struct msg_t *)arg;
    
    buzz_pipe_buzz = p_msg->pipe_buzz;
   /*struct sched_param      	schedParam;
   schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO)+2;	
    if (pthread_attr_setschedparam(&speech_attr, &schedParam)) {
		perror("speech Failed to set scheduler parameters\n");
	}	*/
    pthread_create(&key_tid,  NULL, key_thread,  arg);
    pthread_create(&buzz_tid, NULL, buzz_thread, arg); 

    return 0;
}


