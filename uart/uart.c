/*
 ============================================================================
 Name        : uart.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 串口收发
 ============================================================================
 */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "public.h"
#include "uart.h"
#include "msg.h"
#include "finger.h"
#include "cpld.h"

static uart_object 	uart0_obj, uart1_obj;
static uart_handle 	h_uart0,   h_uart1;

static pipe_handle  uart_pipe_uart;
static pipe_handle  uart_pipe_proc;

static global_data	gbl_uart0_revself;
static global_data	gbl_uart1_revself;

static struct uart_attrs uart_default_attrs = {
	.speed 	     = B1200,
	.data_bits   = 8,
	.stop_bits   = 1,
	.parity	     = 'N',
};
//by mkq finger
static struct uart_attrs uart1_default_attrs = {
	.speed 	     = B1200,
	.data_bits   = 8,
	.stop_bits   = 1,
	.parity	     = 'N',
};

static struct uart_attrs uart0_default_attrs = {
	.speed 	     = B115200,
	.data_bits   = 8,
	.stop_bits   = 1,
	.parity	     = 'N',
};
//end

/*默认串口数据处理函数*/
#if 0
int dummy_fun(unsigned char *data) {
	int i;
	for (i = 0; i < 10; i++)
		printf("uart_buf[%d]:0x%02X\n", i, data[i]);
	printf("\n\n");
	return 0;
}
#endif

/*
* 打开一个串口设备
* 串口0 : "/dev/ttyS0"
* 串口1 : "/dev/ttyS1"
*/
inline static int uart_open(const char *dev)
{
	int fd = open(dev, O_RDWR);

	if (fd) {
       return fd;
	}
	printf( "open %s failed!--------------\n", dev);
	return -1;
}

/*关闭一个串口设备*/
inline static int uart_close(int fd)
{
	if (fd) {
       close(fd);
       fd = -1;
       return 0;
	}

	return -1;
}

/*设置串口参数*/
static int uart_set_attrs(uart_handle h_uart, struct uart_attrs *attrs)
{
    struct termios options;

    int fd = h_uart->fd;

    tcflush(fd, TCIOFLUSH);
    if (tcgetattr(fd, &options)  !=  0) {
        app_debug(DBG_ERROR, "uart get attrs error!\n");
        return -1;
    }

    cfsetispeed(&options, attrs->speed);
    cfsetospeed(&options, attrs->speed);

    options.c_cflag &= ~CSIZE;

    switch (attrs->data_bits) {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            app_debug(DBG_ERROR, "unsupported data bits!\n");
            return -1;
    }

    switch (attrs->stop_bits) {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            app_debug(DBG_ERROR, "unsupported stop bits!\n");
            return -1;
    }

    switch (attrs->parity)	{
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            app_debug(DBG_ERROR, "unsupported parity\n");
            return -1;
    }

    /* Set input parity option */
    if (attrs->parity != 'n')
        options.c_iflag |= INPCK;

    options.c_oflag &= ~(ONLCR | OCRNL | OPOST);
    options.c_iflag &= ~(IXON | IXOFF | IXANY | IGNCR | ICRNL | INLCR);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    tcflush(fd, TCIFLUSH);

    options.c_cc[VTIME] = 150; 	/* 15 seconds*/
    options.c_cc[VMIN]  = 0; 	/* Update the options and do it NOW */

    if (tcsetattr(fd, TCSANOW, &options) != 0)  {
        app_debug(DBG_ERROR, "uart set attrs error!\n");
        return -1;
    }

    tcflush(fd,TCIOFLUSH);

    return 0;
}

void uart0_switch_baud(unsigned int uart_baud)//by mkq finger
{
   struct uart_attrs 	uart0_attrs_sw;

   pthread_mutex_init(&gbl_uart0_revself.mutex,  NULL);
   gbl_data_set(&gbl_uart0_revself,  0);
   
   if(uart_baud == 115200) uart0_attrs_sw.speed = B115200;
   else if(uart_baud == 1200) uart0_attrs_sw.speed = B1200;
   
   uart0_attrs_sw.data_bits = 8;
   uart0_attrs_sw.stop_bits = 1;
   uart0_attrs_sw.parity = 'N';

   uart_set_attrs(h_uart0, &uart0_attrs_sw); 

   printf("uart0_switch_baud %d!------------------------------------------------\n",uart_baud);
}

/*
* 串口发送数据，
* 计算校验和并放到帧最后一个字节
*/
void uart_send_data(int u_id, unsigned char *buf)
{
    unsigned char msg[MSG_LEN_UART];
    int           i, len, sum = 0;

    msg[0] = MSG_FROM_PROC;
    if (u_id == UART_0) {
        msg[1] = UART_0;
        len    = UART0_FRAME_LEN;
	 uart0_switch_baud(1200);
	 Set_ZW_switch(0);
	 usleep(10*1000);
    }
    else {
        msg[1] = UART_1;
        len    = UART1_FRAME_LEN;
    }
    for (i = 0; i < len - 1; i++) {
		msg[i+2] = *(buf + i);
		sum     += *(buf + i);
    }
    msg[2+len-1] = sum;
    pipe_put(uart_pipe_uart, msg, MSG_LEN_UART);
    printf("uart%d_send_data:  %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", \
            msg[1],msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11]);
}

void uart_send_data_fg_ex(unsigned char *buf,int send_len){//by mkq finger
	unsigned char msg[MSG_LEN_UART];
	int           i; 
	long sum = 0;
	msg[0] = MSG_FROM_PROC;
	msg[1] = UART_0;
	for( i = 0;i< send_len;i++){
		msg[2+i] =*(buf+i);
		sum += *(buf+i);
	}
	
	//printf("msg[26] = %02x msg[27] = %02x \n",msg[26],msg[27]);
	pipe_put(uart_pipe_uart, msg, MSG_LEN_UART);
	/*if(msg[0]!=0){
	printf("finger uart%d msg:",msg[1]);
	      for (i = 0; i < send_len; i++)
        		printf("%02X ", msg[i+2]);
   		printf("\n");	
	}*/
}

inline int uart_verify(unsigned char *buf, unsigned char len)
{
    unsigned char sum = 0x00;
    unsigned char i;

    if (len < 1) return 0;

    for (i = 0; i < (len-1); i++)
        sum += buf[i];
    if (sum == buf[len-1])
        return 1;
    else
        return 0;
}

inline int uart_verify_fg(unsigned char *buf, unsigned char len,unsigned char uart_num)//by mkq finger
{
    unsigned char sum = 0x00;
    unsigned char i;
	
    if (len < 1) return 0;
    if(uart_num == UART_0){
    for (i = 0; i < (len-7); i++){
        sum += buf[i+6];
	// printf("sum=%02X\n",sum);
	} 
	if(buf[len-2]==0){
		if (sum == buf[len-1])
        		return 1;
		else
        		return 0;
	}
	else{
		sum=sum -buf[len-2];
		if (sum == buf[len-1])
        		return 1;
		else
       		return 0;
		}
    	}
	else
	return 0;
}

/*
 * 串口发送线程, 两个串口发送都在此处
 */
static void *uart_tx_thread(void *arg)
{
    unsigned char msg[MSG_LEN_UART];
    int fp_send_cnt=0;

    while (1) {
        if (pipe_get(uart_pipe_uart, msg, MSG_LEN_UART) < 0) {
            usleep(100*1000);
            continue;
        }
        //msg[0]   MSG_FROM_
        if (msg[1] == UART_0) {
#ifdef CFG_USE_UART0
	//delete by mkq finger
	   /*
            gbl_data_set(&gbl_uart0_revself, 1);
            write(h_uart0->fd, &msg[2], UART0_FRAME_LEN);
            tcdrain(h_uart0->fd);               //等待发送完成
            usleep(30*1000);                    //等待回读结束
            gbl_data_set(&gbl_uart0_revself, 0);
            */
            //by mkq finger
             if(msg[2] == 0xEF&&msg[3] == 0x01){
    		   	write(h_uart0->fd, &msg[2], 0x09+msg[10]);
    		   	tcdrain(h_uart0->fd);               //等待发送完成
    		   	if(++fp_send_cnt >= 10){
    		   	    fp_send_cnt = 0;
       	            printf("info:fingerprint polling task keepalive!\n");
       	        }
    	     } 
    	     else{
    		 	write(h_uart0->fd, &msg[2], UART0_FRAME_LEN);
    			tcdrain(h_uart0->fd);               //等待发送完成
    			usleep(30*1000);                    //等待回读结束
    			gbl_data_set(&gbl_uart0_revself, 0);
    			printf("write uart0 ok\n");
    	    }
            //end
            /*
		     发送时间 = FRAME_LEN * (1/波特率) *10(数据位1+起始位8+停止位1) * 1.2 (冗余1.2倍)
		     波特率1200  发送时间 = FRAME_LEN * 10
	       	*/
#endif
        }
        else {
		/*
            gbl_data_set(&gbl_uart1_revself, 1);
            write(h_uart1->fd, &msg[2], UART1_FRAME_LEN);
            tcdrain(h_uart1->fd);               //等待发送完成
            usleep(30*1000);                    //等待回读结束
            gbl_data_set(&gbl_uart1_revself, 0);
            */
            //by mkq finger
            gbl_data_set(&gbl_uart1_revself, 1);
            if(msg[2] == 0x16 && msg[3] == 0x4d && msg[4] == 0x0d)// QR DATA
		       write(h_uart1->fd, &msg[2], 11);
	     	else
	     		write(h_uart1->fd, &msg[2], UART1_FRAME_LEN);
            tcdrain(h_uart1->fd);               //等待发送完成
            usleep(30*1000);                    //等待回读结束
            gbl_data_set(&gbl_uart1_revself, 0);
		//end
            
        }
        usleep(50*1000);					    //发送下一帧至少间隔50ms
	}

	return THREAD_SUCCESS;
}

/*
 * 串口0接收线程
 */
#ifdef CFG_USE_UART0
static void *uart0_rx_thread(void *arg)
{
	void *status = THREAD_SUCCESS;
     unsigned char rx_buf[128],  uart_buf[64];
    unsigned char self_bytes = 0;
    unsigned char idx        = 2;
    struct timeval tv_pre, tv_now;
    int read_bytes, diff_ms = 0, begin = 0;

    memset(&tv_pre, 0, sizeof(tv_pre));

    while (1) {
	
        read_bytes = read(h_uart0->fd, rx_buf, UART0_FRAME_LEN);
        gettimeofday(&tv_now, NULL);
        if (begin == 0) {
            tv_pre.tv_sec  = tv_now.tv_sec;
            tv_pre.tv_usec = tv_now.tv_usec;
            begin = 1;
        }
        if (read_bytes <= 0) { continue;}
        //printf("uart0 get data!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        diff_ms = (tv_now.tv_sec - tv_pre.tv_sec) * 1000 + (tv_now.tv_usec - tv_pre.tv_usec) / 1000;
	// printf("read_bytes:%d diff_ms:%d\n", read_bytes, diff_ms);
        tv_pre.tv_sec  = tv_now.tv_sec;
        tv_pre.tv_usec = tv_now.tv_usec;
        /* 字节之间间隔时间超过80ms的丢掉 */
        if (diff_ms > 280) { 
            idx = 2;
        }
        /* 自己发的数据LOOP回来的丢掉 */
       // if (!gbl_data_get(&gbl_uart0_revself)) {
            self_bytes = 0;	
	     memcpy(&uart_buf[idx], rx_buf, read_bytes);
            idx += read_bytes;
			
            if (idx > uart_buf[10]+10){
                idx = 2;
                uart_buf[0] = MSG_FROM_UART;
                uart_buf[1] = UART_0;
		  /*printf("rx uart_buf :%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X   %02X \n",\
		  	uart_buf[2],uart_buf[3],uart_buf[4],\
		  	uart_buf[5],uart_buf[6],uart_buf[7],uart_buf[8],uart_buf[9],uart_buf[10],uart_buf[11],\
		  	uart_buf[12],uart_buf[13],uart_buf[14],uart_buf[15],uart_buf[16],uart_buf[17]);*/
              // if(uart_buf[2] == 0xEF&&uart_buf[3] == 0x01){
		 if (uart_verify_fg(&uart_buf[2], uart_buf[10]+9,UART_0))
                    pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
                else
                    printf("UART0 verify error!\n");
		//	}
		  //else {
                //if (uart_verify(&uart_buf[2], UART0_FRAME_LEN))
              //      pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
               // else
               //     printf("UART0 verify error!\n");
		 // }
            	}
        }
    return status;
	
#if 0//by mkq finger
    void *status = THREAD_SUCCESS;
    unsigned char rx_buf[50],  uart_buf[31];
    unsigned char self_bytes = 0;
    unsigned char idx        = 2;
    struct timeval tv_pre, tv_now;
    int read_bytes, diff_ms = 0, begin = 0;

    memset(&tv_pre, 0, sizeof(tv_pre));

    while (1) {
        read_bytes = read(h_uart0->fd, rx_buf, UART0_FRAME_LEN);
        gettimeofday(&tv_now, NULL);
        if (begin == 0) {
            tv_pre.tv_sec  = tv_now.tv_sec;
            tv_pre.tv_usec = tv_now.tv_usec;
            begin = 1;
        }
        if (read_bytes <= 0) continue;

        diff_ms = (tv_now.tv_sec - tv_pre.tv_sec) * 1000 + (tv_now.tv_usec - tv_pre.tv_usec) / 1000;
 	//printf("read_bytes:%d diff_ms:%d\n", read_bytes, diff_ms);
        tv_pre.tv_sec  = tv_now.tv_sec;
        tv_pre.tv_usec = tv_now.tv_usec;
        /* 字节之间间隔时间超过80ms的丢掉 */
        if (diff_ms > 80) { 
            idx = 2;
        }
        /* 自己发的数据LOOP回来的丢掉 */
        if (!gbl_data_get(&gbl_uart0_revself)) {
            self_bytes = 0;		
            memcpy(&uart_buf[idx], rx_buf, read_bytes);
            idx += read_bytes;
            if (idx > UART0_FRAME_LEN+1) {
                idx = 2;
                uart_buf[0] = MSG_FROM_UART;
                uart_buf[1] = UART_0;
                if (uart_verify(&uart_buf[2], UART0_FRAME_LEN))
                    pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
                else
                    printf("UART0 verify error!\n");
            }
        }
        else {
            self_bytes += read_bytes;
            if (self_bytes >= UART0_FRAME_LEN) {
                gbl_data_set(&gbl_uart0_revself, 0);
                self_bytes = 0;
            }
        }
    }
    return status;
#endif
}
#endif

/*
 * 串口1接收线程
 */
static void *uart1_rx_thread(void *arg)
#if 0
{
    void *status = THREAD_SUCCESS;
    unsigned char rx_buf[50],  uart_buf[31];
    unsigned char self_bytes = 0;
    unsigned char idx        = 2;
    struct timeval tv_pre, tv_now;
    int read_bytes, diff_ms = 0, begin = 0;
    unsigned char recv_length = 0;
    memset(&tv_pre, 0, sizeof(tv_pre));

    while (1) {
        read_bytes = read(h_uart1->fd, rx_buf, UART1_FRAME_LEN-recv_length);
        gettimeofday(&tv_now, NULL);
        if (begin == 0) {
            tv_pre.tv_sec  = tv_now.tv_sec;
            tv_pre.tv_usec = tv_now.tv_usec;
            begin = 1;
        }
        if (read_bytes <= 0) continue;

        diff_ms = (tv_now.tv_sec - tv_pre.tv_sec) * 1000 + (tv_now.tv_usec - tv_pre.tv_usec) / 1000;

        //printf("read_bytes:%d diff_ms:%d\n", read_bytes, diff_ms);
        tv_pre.tv_sec  = tv_now.tv_sec;
        tv_pre.tv_usec = tv_now.tv_usec;

        /* 字节之间间隔时间超过80ms的丢掉 */
        if (diff_ms > 80) {
            idx = 2;
        }

        /* 自己发的数据LOOP回来的丢掉 */
        if (!gbl_data_get(&gbl_uart1_revself)) {
            self_bytes = 0;
            memcpy(&uart_buf[idx], rx_buf, read_bytes);
            idx += read_bytes;
	     if(uart_buf[2] == 0X7F){
		 	if( idx > 22){
		 		idx = 2;
				uart_buf[0] = MSG_FROM_UART;
				uart_buf[1] = UART_1;	
				if (uart_verify(&uart_buf[2], 21))
					pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
				else
					printf("UART1 verify error!\n");
				}
			}
		 else{
		 	if (idx > UART1_FRAME_LEN+1) {
				idx = 2;
				uart_buf[0] = MSG_FROM_UART;
				uart_buf[1] = UART_1;
				if (uart_verify(&uart_buf[2], UART1_FRAME_LEN))
					pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
				else
					{
					printf("UART1 verify error!\n");

				}
				}
			}
		}
        else {
            self_bytes += read_bytes;
            if (self_bytes >= UART1_FRAME_LEN) {
                gbl_data_set(&gbl_uart1_revself, 0);
                self_bytes = 0;
            }
        }
    }
    return status;
}
#else

{
    void *status = THREAD_SUCCESS;
    unsigned char rx_buf[50],  uart_buf[50];
    unsigned char self_bytes = 0;
    unsigned char idx        = 2;
    struct timeval tv_pre, tv_now;
    int read_bytes = 0, diff_ms = 0, begin = 0; 
    unsigned char recv_length = 0;
    int i =0;
    memset(&tv_pre, 0, sizeof(tv_pre));
    int read_length = UART1_FRAME_LEN_BLE;
    while (1) {
        read_bytes = read(h_uart1->fd, rx_buf, read_length);
        //read_bytes = read(h_uart1->fd, rx_buf, UART1_FRAME_LEN_BLE);
        gettimeofday(&tv_now, NULL);
        if (begin == 0) {
            tv_pre.tv_sec  = tv_now.tv_sec;
            tv_pre.tv_usec = tv_now.tv_usec;
            begin = 1;
        }
        if (read_bytes <= 0) continue;
	 
        diff_ms = (tv_now.tv_sec - tv_pre.tv_sec) * 1000 + (tv_now.tv_usec - tv_pre.tv_usec) / 1000;

        //printf("read_bytes:%d diff_ms:%d\n", read_bytes, diff_ms);
        tv_pre.tv_sec  = tv_now.tv_sec;
        tv_pre.tv_usec = tv_now.tv_usec;

        /* 字节之间间隔时间超过80ms的丢掉 */
        if (diff_ms > 100) {
            idx = 2;
            recv_length = 0;
            printf("uart1 get new package:cmd=%02x diff_ms= %d\n", rx_buf[0], diff_ms);
        }
        else
	     recv_length += read_bytes;
	 //printf("--------the recv_length : %d read_bytes :%d read_length: %d\n",recv_length,read_bytes,read_length);
	 
	 if(recv_length == UART1_FRAME_LEN_BLE-2)
	 	read_length = 1;  // need add...
	 else
	 	read_length = UART1_FRAME_LEN_BLE;
	 
        /* 自己发的数据LOOP回来的丢掉 */
        if (!gbl_data_get(&gbl_uart1_revself)) {
            self_bytes = 0;	
            memcpy(&uart_buf[idx], rx_buf, read_bytes);
            idx += read_bytes;
	     //printf("--------idx = %d\n",idx);
	     if(uart_buf[2] == 0x59){
		 	if (idx > UART1_FRAME_LEN+3) {
		 		idx = 2;
				
				uart_buf[0] = MSG_FROM_UART;
				uart_buf[1] = UART_1;

				printf("uart1 :");  
				for(i = 0;i<20;i++)
					printf("%02X ",uart_buf[i]);
				printf("\n");
				//if (uart_verify(&uart_buf[2], 21))				
					pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
				//else
					//printf("UART1 verify error!\n");
				}
			}
           //判断帧头 0X7F 区分蓝牙数据      
           else
	     if(uart_buf[2] == 0X7F){
		 	if( idx > 22 ){
		 		idx = 2;
				recv_length = 0;
				read_bytes = 0;
				uart_buf[0] = MSG_FROM_UART;
				uart_buf[1] = UART_1;	
				if (uart_verify(&uart_buf[2], 21))
					pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
				else
					printf("UART1 verify error!\n");
				}
			}
		 else{
		 	if (idx > UART1_FRAME_LEN+1) {
				idx = 2;
				recv_length = 0;
				uart_buf[0] = MSG_FROM_UART;
				uart_buf[1] = UART_1;
				printf("uart1 :");
				for(i = 0;i<20;i++)
					printf("%02X ",uart_buf[i]);
				printf("\n");
				if(uart_buf[2] == 0xAF)
					pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
				else{
				if (uart_verify(&uart_buf[2], UART1_FRAME_LEN))
					pipe_put(uart_pipe_proc, uart_buf, MSG_LEN_PROC);
				else
					printf("UART1 verify error!\n");
					
				}
				}
			}
		}
        else {
            self_bytes += read_bytes;
            if (self_bytes >= UART1_FRAME_LEN) {
                gbl_data_set(&gbl_uart1_revself, 0);
                self_bytes = 0;
		  		recv_length = 0;
            }
        }
    }
    return status;
}
#endif


/*
* 串口模块初始化
* 配置参数并创建收发线程
*/
int uart_init(void)
{
    struct uart_attrs 	uart0_attrs,  uart1_attrs;
    pthread_t 		uart1_rx_tid, uart_tx_tid;

    pthread_mutex_init(&gbl_uart0_revself.mutex,  NULL);
    pthread_mutex_init(&gbl_uart1_revself.mutex,  NULL);
    
    gbl_data_set(&gbl_uart0_revself,  0);
    gbl_data_set(&gbl_uart1_revself,  0);
    
    memset(&uart0_obj, 0, sizeof(uart_object));
    memset(&uart1_obj, 0, sizeof(uart_object));
   // memcpy(&uart0_attrs, &uart_default_attrs, sizeof(uart0_attrs));
   // memcpy(&uart1_attrs, &uart_default_attrs, sizeof(uart1_attrs));
    memcpy(&uart0_attrs, &uart0_default_attrs, sizeof(uart0_attrs));//by mkq finger
    memcpy(&uart1_attrs, &uart1_default_attrs, sizeof(uart1_attrs));//by mkq finger

    h_uart0 = &uart0_obj;
    h_uart1 = &uart1_obj;
    // h_uart1->fd = uart_open("/dev/ttyS1");
    h_uart1->fd = uart_open("/dev/ttyS2"); //by mkq 20170916
    uart_set_attrs(h_uart1, &uart1_attrs);
    pthread_create(&uart1_rx_tid,  NULL, uart1_rx_thread,  NULL);
	//pthread_detach(uart1_rx_tid);//by mkq
    pthread_create(&uart_tx_tid,   NULL, uart_tx_thread,   NULL);

#ifdef CFG_USE_UART0
    pthread_t               uart0_rx_tid;
    //h_uart0->fd = uart_open("/dev/ttyS0");
    h_uart0->fd = uart_open("/dev/ttyS1");//by mkq 20170916
    uart_set_attrs(h_uart0, &uart0_attrs);
    //printf("COM1 init OK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    pthread_create(&uart0_rx_tid, NULL, uart0_rx_thread, NULL);
	//pthread_detach(uart0_rx_tid);
#endif

	return 0;
}

int uart_start(void *arg)
{
    struct msg_t *p_msg = (struct msg_t *)arg;

    uart_pipe_proc = p_msg->pipe_proc; 
    uart_pipe_uart = p_msg->pipe_uart; 

    uart_init();
    return 0;
}

