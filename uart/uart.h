#ifndef _UART_H_
#define _UART_H_

#include <termios.h>

#if defined (__cplusplus)
extern "C" {
#endif

/*是否使用串口0，注意使用串口0要在UBOOT中关掉串口0的console*/
#define CFG_USE_UART0

#define UART_0              0
#define UART_1  			1

#define UART0_DATA			0
#define UART1_DATA			1

/*串口帧长度*/
#define UART0_FRAME_LEN		10
#define UART1_FRAME_LEN		8

#define UART1_FRAME_LEN_BLE     21

typedef struct uart_object_struct {
	int		fd;
} uart_object, *uart_handle;

struct uart_attrs {
	speed_t 	speed;		//typedef unsigned int in <termios.h>
	int 		data_bits;
	int 		stop_bits;
	int 		parity;
};

extern int  uart_start(void *arg);
extern void uart_send_data(int u_id, unsigned char *buf);
extern void uart_send_data_fg_ex(unsigned char *buf,int send_len);//by mkq finger
extern void uart0_switch_baud(unsigned int uart_baud);//by mkq finger

#if defined (__cplusplus)
}
#endif

#endif

