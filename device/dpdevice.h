


#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED			0
#endif
#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS			0
#endif
#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif

#define IOCTRL_GPIO_INIT 				0x4c434b06
#define IOCTRL_SET_GPIO_VALUE			0x4c434b07
#define IOCTRL_GET_GPIO_VALUE			0x4c434b08
#ifndef IOCTRL_SET_IRQ_MODE
#define IOCTRL_SET_IRQ_MODE                     0x4c434b09//add by mkq 20170906
#endif
#define IOCTL_START_WATCHDOG		0x12345678
#define IOCTL_REFRESH_WATCHDOG		0x12345679
#define IOCTL_STOP_WATCHDOG			0x1234567a

typedef struct
{
	unsigned short gpio_num;
	unsigned char gpio_cfg;		//0 input 1 output (0~7)
	unsigned char gpio_pull;	//PULL_DISABLE,PULL_UP,PULL_DOWN	
	unsigned char gpio_driver;	//0~3
} GPIO_CFG;

 
#define	GPIO_H2							226  

/*
int InitGpio(int pin, int inout, int value);
void SetGpioVal(int hDev, int value);
int GetGpioVal(int hdev);
void DeinitGpio(int hDev);
void Reset_Net(void);
void FCcheck(void);
*/
