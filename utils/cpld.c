/*
 ============================================================================
 Name        : cpld.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : cpld
  ============================================================================
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "cpld.h"
#include "public.h"
#include "string.h"

#define CPLD_IO_MAGIC       'l'
#if 0
#define DEF_EIO_DIR_OUT     _IO(CPLD_IO_MAGIC, 0x0A)
#define DEF_EIO_DIR_IN 	 _IO(CPLD_IO_MAGIC, 0x0B)
#define DEF_EIO_SET_DATA    _IO(CPLD_IO_MAGIC, 0x0C)
#define DEF_EIO_CLR_DATA    _IO(CPLD_IO_MAGIC, 0x0D)
#define DEF_EIO_GET_DATA    _IO(CPLD_IO_MAGIC, 0x0E)
#define DEF_FLASH_GET_DATA  _IO(CPLD_IO_MAGIC, 0x0F)
#define DEF_PWM_W_SET       _IO(CPLD_IO_MAGIC, 0x10)
#define DEF_PWM_W_GET       _IO(CPLD_IO_MAGIC, 0x11)
//#else
#define DEF_EIO_DIR_OUT     _IOW(CPLD_IO_MAGIC, 0x8A, int)
#define DEF_EIO_DIR_IN      _IOW(CPLD_IO_MAGIC, 0x8B, int)
#define DEF_EIO_SET_DATA    _IOW(CPLD_IO_MAGIC, 0x8C, int)
#define DEF_EIO_CLR_DATA    _IOW(CPLD_IO_MAGIC, 0x8D, int)
#define DEF_EIO_GET_DATA    _IOR(CPLD_IO_MAGIC, 0x8E, int)
#define DEF_FLASH_GET_DATA  _IOR(CPLD_IO_MAGIC, 0x8F, int)
#define DEF_PWM_W_SET       _IOW(CPLD_IO_MAGIC, 0x90, int)
#define DEF_PWM_W_GET       _IOR(CPLD_IO_MAGIC, 0x91, int)
#endif

static int cpld_fd = -1;

static unsigned char cpld_i2c_addr = 0xA2;
static int m_hGpio_ZW_switch;
static int m_hkeybuzz;
static int m_hGpioCCDLight;
static int m_hGpioCardLight;
static int m_hGpioSEL_S_M,m_hGpioBackLight;
static int hgpioLOCK;  
static int m_hGpioPhotoSence;

/////////////////////////////////////////////////////////////////////////////add by mkq 20170916

void SetGpioVal(int hdev, int value)
{
	if(hdev == NULL)
		return;
	if(ioctl((int)hdev, IOCTRL_SET_GPIO_VALUE,&value))
	{
		printf("IOCTRL_SET_GPIO_VALUE  fail error \r\n");
		return;
	}
}

 int GetGpioVal(int hdev)//add by mkq 20170914
{
	int data= 0;
	int bytesreturned;
	/*if(!ioctl((int)hdev, IOCTRL_GET_GPIO_VALUE, &data))
	{
	  printf( "IOCTRL_GET_GPIO_VALUE  fail error %d \n",data);
		return 0;
	}
	return 1;*/
	ioctl((int)hdev, IOCTRL_GET_GPIO_VALUE, &data);
        return data;
}

int InitGpio(int pin, int inout, int value)
{
	int fd;
	GPIO_CFG gpio_cfg;

	fd = open("/dev/gpio", O_RDWR,0);
	if(fd < 0)
	{
		printf("open /dev/gpio  fail!\n");
		return 0;
	}
	
	memset(&gpio_cfg,0,sizeof(GPIO_CFG));
	gpio_cfg.gpio_num= (short)pin;
	gpio_cfg.gpio_cfg = (short)inout;
	gpio_cfg.gpio_pull = (short)value;
	if(ioctl(fd,IOCTRL_GPIO_INIT,&gpio_cfg))
	{
		printf("IOCTRL_GPIO_INIT  fail error is \r\n");
		close(fd);
		return 0;
	}
	return (int)fd;
}

void DeinitGpio(int hGpio)
{
	close((int)hGpio);
}

void Set_ZW_switch(int Onoff){//add by hgj
	printf("change the m_hGpio_ZW_switch to %d\n",Onoff);
	SetGpioVal(m_hGpio_ZW_switch, Onoff);
}

void Set_CardLight(int Onoff){//add by mkq 
	SetGpioVal(m_hGpioCardLight, Onoff);
}

void Set_CCD_Light(int Onoff){//add by mkq
	printf("change the CCD light to %d\n",Onoff);
	SetGpioVal(m_hGpioCCDLight, Onoff);
}

void Set_Back_Light(int Onoff){//add by hgj
	printf("change the BackLight to %d\n",Onoff);
	SetGpioVal(m_hGpioBackLight, Onoff);
}
void Set_SEL_S_M(int Onoff){//add by hgj
	printf("change the SEL_S_M to %d\n",Onoff);
	SetGpioVal(m_hGpioSEL_S_M, Onoff);
}

void Set_keybuzz(int Onoff){//add by hgj
	//printf("change the keybuzz to %d\n",Onoff);
	SetGpioVal(m_hkeybuzz, Onoff);
}

void Set_lock(int Onoff){//add by hgj
	printf("change the lock to %d\n",Onoff);
	SetGpioVal(hgpioLOCK, Onoff);
}

int get_photo_sence(void){
	//printf("get the photo sence\n");
	return GetGpioVal(m_hGpioPhotoSence);
}
void cpld_init(void)
{
#if 1
//printf("%s not define\n", __func__);return ;
    m_hGpio_ZW_switch = InitGpio(GPIO_C10, 1, 0);
    m_hkeybuzz = InitGpio(GPIO_C7, 1, 0);//buzz
    m_hGpioCCDLight = InitGpio(GPIO_C5, 1, 0);
    m_hGpioCardLight = InitGpio(GPIO_C8, 1, 0);
    m_hGpioSEL_S_M = InitGpio(GPIO_G5, 1, 0);
    m_hGpioBackLight = InitGpio(GPIO_C11,1,0); 
    hgpioLOCK = InitGpio(GPIO_C13,1,0);
    m_hGpioPhotoSence = InitGpio(GPIO_C9, 0, 0);    
    
    Set_ZW_switch(1);
    Set_CCD_Light(0);
    Set_lock(0);
    Set_CardLight(0);
    Set_keybuzz(0);
    Set_Back_Light(0);
    
#else
    cpld_fd = open(CPLD_DEVICE, O_RDWR);
    if (cpld_fd < 0){
        perror("can't open cpld.\n");
        exit(0);
    }
    //cpld_io_output(IO9);
    //cpld_io_clr(IO9);
    //cpld_io_output(IOAicRst);
    //cpld_io_clr(IOAicRst);
    //cpld_io_output(EIO_FLASH_PROTECT);
    //cpld_io_clr(EIO_FLASH_PROTECT);
    /*cpld_io_output(EIO_VIDEO_IN);
    cpld_io_clr(EIO_VIDEO_IN);
    
    cpld_io_output(EIO_LCD_AVDD);
    cpld_io_clr(EIO_LCD_AVDD);
    cpld_io_output(EIO_LCD_DVDD);
    cpld_io_set(EIO_LCD_DVDD);*/
    //cpld_io_output(EIO_LCD_BACKLIGHT);
    //cpld_io_clr(EIO_LCD_BACKLIGHT);
    /*cpld_io_output(IOTVP5150Rst);
    cpld_io_clr(IOTVP5150Rst);*/
    cpld_io_output(EIO_CAMERA_LIGHT);
    cpld_io_clr(EIO_CAMERA_LIGHT);
    cpld_io_output(EIO_KEYPAD_LIGHT);
    cpld_io_clr(EIO_KEYPAD_LIGHT);
    //cpld_io_output(EIO_BUZZER);
    //cpld_io_clr(EIO_BUZZER);
    cpld_io_output(EIO_CARD_CODE_EN);
    cpld_io_clr(EIO_CARD_CODE_EN);
    /*cpld_io_output(EIO_LOCK_SWITCH);
    cpld_io_clr(EIO_LOCK_SWITCH);
    cpld_io_output(IOLed);
    cpld_io_clr(IOLed);
    cpld_io_output(IO5);
    cpld_io_clr(IO5);
    cpld_io_output(IO36);
    cpld_io_set(IO36);*/
    cpld_io_input(EIO_UNLOCK_KEY);
    //cpld_io_input(EIO_DOOR_STATE);
    //cpld_io_input(EIO_DETACH_STATE);
    cpld_io_input(EIO_PHOTOSENSITIVE);

    /* IRCUT */
    /*cpld_io_output(GPIO_G5);//IO6
    cpld_io_output(GPIO_C15);//IO14
    cpld_io_output(GPIO_C10);//IO16
    cpld_ircut_sw(IR_CUT_DAY);*/

    //cpld_lcd_init(); //add by wrm 20141229 for lcd reverse,this sentence need to move other places
#endif
}

void cpld_close(void)
{
#if 1
printf("%s not define\n", __func__);return ;
#else
    if (cpld_fd) {
        close(cpld_fd);
        cpld_fd = -1;
    }
#endif
}

void cpld_io_output(int id)
{
	//printf("%s not define\n", __func__);
#if 1
	return ;
	//ioctl(cpld_fd, DEF_EIO_DIR_OUT, id);
#else
	GPIO_CFG gpio_cfg;
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = id;
	gpio_cfg.gpio_cfg = 1;
	ioctl(cpld_fd, IOCTRL_GPIO_INIT, &gpio_cfg);
#endif
}

void cpld_io_input(int id)
{
	//printf("%s not define\n", __func__);
#if 1
	return ;
	//ioctl(cpld_fd, DEF_EIO_DIR_IN, id);
#else
	GPIO_CFG gpio_cfg;
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = id;
	gpio_cfg.gpio_cfg = 0;
	ioctl(cpld_fd, IOCTRL_GPIO_INIT, &gpio_cfg);
#endif
}

void cpld_io_set(int id)
{
	//printf("%s not define\n", __func__);
#if 1	
	return ;

	
	//ioctl(cpld_fd, DEF_EIO_SET_DATA, id);
#else
	GPIO_CFG gpio_cfg;
	char stat=1;
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = id;
	gpio_cfg.gpio_cfg = 1;
	ioctl(cpld_fd, IOCTRL_GPIO_INIT, &gpio_cfg);
    ioctl(cpld_fd, IOCTRL_SET_GPIO_VALUE, &stat);
#endif
}

void cpld_io_clr(int id)
{
//	printf("%s not define\n", __func__);
#if 1	
	return ;

	//ioctl(cpld_fd, DEF_EIO_CLR_DATA, id);
#else
	GPIO_CFG gpio_cfg;
	char stat=0;
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = id;
	gpio_cfg.gpio_cfg = 1;
	ioctl(cpld_fd, IOCTRL_GPIO_INIT, &gpio_cfg);
    ioctl(cpld_fd, IOCTRL_SET_GPIO_VALUE, &stat);
#endif
}

int cpld_io_voltage(int id)
{
#if 1
printf("%s not define\n", __func__);return 0;
	//return ioctl(cpld_fd, DEF_EIO_GET_DATA, id);
#else
    char stat;
    GPIO_CFG gpio_cfg;
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = id;
	gpio_cfg.gpio_cfg = 0;
	ioctl(cpld_fd, IOCTRL_GPIO_INIT, &gpio_cfg);    
    ioctl(cpld_fd, IOCTRL_GET_GPIO_VALUE, &stat);
    return stat;
#endif
}
/*i2c??д???:??????????д(???????slave address???????????)???????SDA????????????
???????????????????:
1.???SDA???ж?????豣?SCL??????????SDA???????
2.???SDA????д????豣?SCL??????????SDA???????д??
*/
unsigned char cpld_i2c_fetch_data(void)
{printf("%s not define\n", __func__);return 0;
    int i, wait_ack = 10;
    unsigned char data = 0;
    
    // read bit
    cpld_i2c_addr |= 0x01;
    cpld_io_output(EIO_I2C_SDA);

    // start bit
    cpld_io_set(EIO_I2C_SCL);
    usleep(20);
    cpld_io_set(EIO_I2C_SDA);
    usleep(20);
    cpld_io_clr(EIO_I2C_SDA);

    // send slave addr
    for (i = 0; i < 8; i++) {
        usleep(10);
        cpld_io_clr(EIO_I2C_SCL);
        usleep(10);
        (cpld_i2c_addr & (0x80>>i)) ? cpld_io_set(EIO_I2C_SDA) : cpld_io_clr(EIO_I2C_SDA);
        cpld_io_set(EIO_I2C_SCL);
    }

    // wait slave ack
    cpld_io_clr(EIO_I2C_SCL);

    cpld_io_set(EIO_I2C_SDA);

    cpld_io_input(EIO_I2C_SDA);

    cpld_io_set(EIO_I2C_SCL);

    usleep(200);
    while (cpld_io_voltage(EIO_I2C_SDA) && (wait_ack--)) {
        usleep(1000);
    }
    cpld_io_clr(EIO_I2C_SCL);

    // read slave data
    for (i = 0; i < 8; i++) {
        cpld_io_clr(EIO_I2C_SCL);
        usleep(10);
        cpld_io_set(EIO_I2C_SCL);
        usleep(10);
        data |= (cpld_io_voltage(EIO_I2C_SDA) ? (0x80>>i) : 0x00);
    }

    cpld_io_output(EIO_I2C_SDA);

    cpld_io_clr(EIO_I2C_SCL);
    usleep(20);
    cpld_io_set(EIO_I2C_SCL);

    cpld_io_clr(EIO_I2C_SDA);
    usleep(20);
    cpld_io_set(EIO_I2C_SDA);

    return data;
}

void cpld_i2c_init(void)
{printf("%s not define\n", __func__);return ;
    cpld_io_input(EIO_I2C_ITR);
    cpld_io_output(IO31);
    cpld_io_clr(IO31);

    cpld_io_output(EIO_I2C_SDA);
    cpld_io_output(EIO_I2C_SCL);
    cpld_io_set(EIO_I2C_SDA);
    cpld_io_set(EIO_I2C_SCL);
}

void cpld_ircut_sw(int sw)
{
#if 1
printf("%s not define\n", __func__);return ;
#else

#if 1
    static int ircut_stat = IR_CUT_UNKNOWN;
    if (sw == IR_CUT_DAY) {
        if (ircut_stat != IR_CUT_DAY) {
            cpld_io_set(GPIO_G5);//IO6
            cpld_io_set(GPIO_C10);//IO16
            ircut_stat = IR_CUT_DAY;
            app_debug(DBG_INFO, "cpld_ircut_sw:DAY \n");
        }
    } else {
        if (ircut_stat != IR_CUT_NIGHT) {
            cpld_io_clr(GPIO_G5);//IO6
            cpld_io_clr(GPIO_C10);//IO16
            ircut_stat = IR_CUT_NIGHT;
            app_debug(DBG_INFO, "cpld_ircut_sw:NIGHT \n");
        }
    }
#else
    app_debug(DBG_INFO, "cpld_ircut_sw:%d \n", sw);
    if (sw == IR_CUT_DAY) {
        cpld_io_set(IO6);
        cpld_io_set(IO16);
    } else {
        cpld_io_clr(IO6);
        cpld_io_clr(IO16);
    }
#endif
/*
    cpld_io_set(GPIO_C15);//IO14
    usleep(160*1000);
    cpld_io_clr(GPIO_C15);//IO14
*/

#endif
}

#define EIO_SPDA  IO37
#define EIO_SPENA IO38
#define EIO_SPCK  IO39

void cpld_ili9322_init(void)
{printf("%s not define\n", __func__);return ;
    cpld_io_output(EIO_SPDA);
    cpld_io_output(EIO_SPCK);
    cpld_io_output(EIO_SPENA);
}

void cpld_ili9322_write(unsigned char addr, unsigned char data)
{ printf("%s not define\n", __func__);return ;
    unsigned char mask = 0;   
    cpld_io_set(EIO_SPENA);         //SPENA = 1;  
    usleep(1000);
    cpld_io_clr(EIO_SPENA);         //SPENA = 0;  
    usleep(100);
    addr &=~ 0x80; //write mode
    for (mask = 0x80; mask; mask >>= 1)
    {
       cpld_io_clr(EIO_SPCK);       //SPCK = 0;
       usleep(3);                   //LO_PULSE;
       if(addr & mask)
         cpld_io_set(EIO_SPDA);     //SPDA = 1;
       else
         cpld_io_clr(EIO_SPDA);     //SPDA = 0;
       usleep(1);                   //LO_PULSE;
       cpld_io_set(EIO_SPCK);       //SPCK = 1;
       usleep(4);                   //HI_PULSE;
    }

    for (mask = 0x80; mask; mask >>= 1)
    {
       cpld_io_clr(EIO_SPCK);       //SPCK = 0;
       usleep(3);                   //LO_PULSE;
       if(data & mask)
         cpld_io_set(EIO_SPDA);     //SPDA = 1;
       else
         cpld_io_clr(EIO_SPDA);     //SPDA = 0;
       usleep(1);                   //LO_PULSE;
       cpld_io_set(EIO_SPCK);       //SPCK = 1;
       usleep(4);                   //HI_PULSE;
    }
    
    cpld_io_set(EIO_SPENA);         //SPENA = 1;    

    usleep(10);        
}

void cpld_lcd_init(void)
{printf("%s not define\n", __func__);return ;
#if 1
    cpld_ili9322_init();

   // cpld_ili9322_write(0x07,0x00);
    usleep(100*1000);
   // cpld_ili9322_write(0x07,0xFF);
    cpld_ili9322_write(0x06,0x6c);//wrm 20141224 for lcd configuration
#endif
}




