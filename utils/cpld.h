#ifndef _CPLD_H_
#define _CPLD_H_

#if defined (__cplusplus)
extern "C" {
#endif      
#include "dpgpio.h"

#define CPLD_DEVICE     "/dev/gpio"

typedef enum {
	IO1 = 0, ///<    EIO - 1
	IO2,     ///<    EIO - 2
	IO3,     ///<    EIO - 3
	IO4,     ///<    EIO - 4
	IO5,     ///<    EIO - 5
	IO6,     ///<    EIO - 6
	IO7,     ///<    EIO - 7
	IO8,     ///<    EIO - 8
	IO9,     ///<    EIO - 9
	IO10,    ///<    EIO - 10
	IO11,    ///<    EIO - 11
	IO12,    ///<    EIO - 12
	IO13,    ///<    EIO - 13
	IO14,    ///<    EIO - 14
	IO15,    ///<    EIO - 15
	IO16,    ///<    EIO - 16
	IO17,    ///<    EIO - 17
	IO18,    ///<    EIO - 18
	IO19,    ///<    EIO - 19
	IO20,    ///<    EIO - 10
	IO21,    ///<    EIO - 21
	IO22,    ///<    EIO - 22
	IO23,    ///<    EIO - 23
	IO24,    ///<    EIO - 24
	IO25,    ///<    EIO - 25
	IO26,    ///<    EIO - 26
	IO27,    ///<    EIO - 27
	IO28,    ///<    EIO - 28
	IO29,    ///<    EIO - 29
	IO30,    ///<    EIO - 30
	IO31,    ///<    EIO - 31
	IO32,    ///<    EIO - 32
	IO33,    ///<    EIO - 33
	IO34,    ///<    EIO - 34
	IO35,    ///<    EIO - 35
	IO36,    ///<    EIO - 36
	IO37,    ///<    EIO - 37
	IO38,    ///<    EIO - 38
	IO39,    ///<    EIO - 39
	IO45,    ///<    EIO - 45
	IODM9Rst,        ///<  EIO - IODM9Rst
	IOTVP5150Rst,    ///<  EIO - TVP5150
	IOCmosRst,       ///<  EIO - IOCmosRst
	IOAicRst,        ///<  EIO - IOAicRst
	IOLed,           ///<  EIO - IOLed
	IOReboot,        ///<  EIO - IOReboot
	IO_NULL,
	IOUsbVbus,       ///<  EIO - IOUsbVbus
	EIO_MAX
} EIO_ID;

#define EIO_I2C_SDA             IO34
#define EIO_I2C_SCL             IO32
#define EIO_I2C_ITR             IO7

#define EIO_BUZZER              GPIO_C7//IO28
#define EIO_DOOR_STATE          GPIO_C14//IO22
#define EIO_UNLOCK_KEY          GPIO_C12//IO18
//#define EIO_LOCK_SWITCH         IO20
#define EIO_CARD_CODE_EN        GPIO_G5//IO6
#define EIO_DETACH_STATE        GPIO_G7//IO2
//#define EIO_VIDEO_IN            IO4
#define EIO_KEYPAD_LIGHT        GPIO_C8//IO8
#define EIO_CAMERA_LIGHT        GPIO_C5//IO24
//#define EIO_LCD_AVDD            IO10 //H active
//#define EIO_LCD_DVDD            IO12 //L active
#define EIO_LCD_BACKLIGHT       GPIO_C11 //H active
#define EIO_PHOTOSENSITIVE      GPIO_C6//IO26
//#define EIO_FLASH_PROTECT       IO33

#define IR_CUT_DAY              0x00
#define IR_CUT_NIGHT            0x01
#define IR_CUT_UNKNOWN          0xFF

extern void cpld_init(void);
extern void cpld_close(void);
extern void cpld_io_output(int id);
extern void cpld_io_input(int id);
extern void cpld_io_set(int id);
extern void cpld_io_clr(int id);
extern int  cpld_io_voltage(int id);
extern void cpld_i2c_init(void);
extern void cpld_ircut_sw(int sw);
extern unsigned char cpld_i2c_fetch_data(void);
extern void cpld_lcd_init(void);

void Set_ZW_switch(int Onoff);
void Set_CardLight(int Onoff);//by mkq
void Set_CCD_Light(int Onoff);//by mkq
void Set_Back_Light(int Onoff);//add by hgj
void Set_SEL_S_M(int Onoff);//add by hgj
void Set_keybuzz(int Onoff);//add by hgj
void Set_lock(int Onoff);//add by hgj
int get_photo_sence(void);

#if defined (__cplusplus)
}
#endif
 
#endif
