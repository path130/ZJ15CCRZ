#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>

#include "dpplatform.h"
#include "dpgpio.h"
#include "dpvideo.h"
#include "libfacedetector.h"

#include "preview.h"
#include "sensor.h"
#include "frame.h"
#include "json_data_pro.h"
#include "public.h"
#include "msg.h"
#include "video_send.h"
#include "tim.h"
#include "my_debug.h"
#include "net_com.h"
#include "audio.h"
#include "dev_pro.h"
#include "global_data.h"
#include "face.h"
#include "ui.h"
#include "key.h"
#include "json_msg.h"//add for photo upload

#define ZBAR
//#undef ZBAR
#ifdef ZBAR
#include <zbar.h>
#endif

#define MAX_FACE 5
#define DET_WIDTH 320
#define DET_HEIGH 220
#define MODEL_PATH "model_frontal.bin"
#define JPEG_BUF 200*1024

static int running;
static pthread_t pid_face;
static pthread_t pid_enc;
static volatile int take_photo;
static volatile int big_face;

static int thread_enc_running = 0;

static pipe_handle  face_pipe_proc;
static int algo_sw_tim_id;
static int result_clear_tim_id,FR_wait_tim_id;


static int lg_fail_cnt,lg_show_name_enable;
static  global_data  gbl_FR_stop    = GBL_DATA_INIT;//int recv_stop;//edit by hgj
static long long lg_pre_id=0;

extern void ui_lcd_on(void);
extern void Set_CCD_Light(int Onoff);

//add for photo upload
extern int get_body_sense_result(void);


int ajb_connect_face_server(uint16_t const port)
{
	int ret = 0;
	char buf[1024] = {0};
	int tcp_time_out = 3;
	int socket_fd = -1;

	struct sockaddr_in server = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = inet_addr(setup_json_server_ip),
	};
	socklen_t len = sizeof(server);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd < 0) {
		dbg_perror("socket");
		return -1;
	}


	app_debug(DBG_INFO,"ip %s port %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
	ret = connect_nonb(socket_fd, (struct sockaddr *)&server, len, tcp_time_out);

	//ret = net_connect(socket_fd, (struct sockaddr *)&server);
	if (ret < 0) {
		dbg_perror("connect:");
	} else {
		return socket_fd;
	}

	return ret;
}

int face_check_by_server(char *jpeg_buf, int len, char usr_id[10])
{
	int ret = 0;
	int count = 0, pack_len = 0;
	short type = 0;

	int face_recognize_score = -1;
	char recv_buf[128] = {0,};
	int expect_len = 21;
	int i = 0;
	struct timeval timeout = {3, 0};

	if (assert_ptr(jpeg_buf) || assert_ptr(usr_id)) {
		return -1;
	}

	int socket_fd = ajb_connect_face_server(AJB_FACE_QUERY_PORT);

	if (socket_fd < 0) {
		dbg_err("\n");
		return -1;
	} else {
		//dbg_lo("connecg success! socket_fd=%d\n", socket_fd);
	}

	jpeg_tcp_head_t pic_header = {
		.head = 0xff53,
		.package_len = len + 10,
		.cmd = 0x3001,
	};
#ifndef FRONT_DOOR
	pic_header.devno[0] = (gData.DoorNo[0] >> 4) + '0';
	pic_header.devno[1] = (gData.DoorNo[0] & 0xf) + '0';
	pic_header.devno[2] = (gData.DoorNo[1] >> 4) + '0';
	pic_header.devno[3] = (gData.DoorNo[1] & 0xf) + '0';
#else
    pic_header.devno[0] = pic_header.devno[1] = pic_header.devno[2] = pic_header.devno[3] = 'a';
#endif

	app_debug(DBG_INFO,"head=%x len=%d cmd=%x %c%c%c%c\n", pic_header.head, pic_header.package_len, pic_header.cmd, \
	       pic_header.devno[0], pic_header.devno[1], pic_header.devno[2], pic_header.devno[3]);

	do {

		if (0 > send_data(socket_fd, &pic_header, sizeof(pic_header))) {
			dbg_perror("send:");
			break;
		}

		if (0 > send_data(socket_fd, jpeg_buf, len)) {
			dbg_perror("send:");
			break;
		}

		app_debug(DBG_INFO,"ready to receive face check!\n");

		ret = setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

		ret = recv_data(socket_fd, recv_buf, expect_len);

		if (ret <= 0) {
			dbg_perror("recv:");
			break;
		} else {
			app_debug(DBG_INFO,"tcp received from ajb server, ret=%d\n", ret);
			//print_n_byte(recv_buf, expect_len);
		}

		if (recv_buf[0] != 0x53) {
			dbg_err("1st 0x%02X\n", recv_buf[0]);
			break;
		}

		if (recv_buf[1] != 0xff) {
			dbg_lo("2nd %x\n", recv_buf[1]);
			break;
		}

		memcpy((char *)&pack_len, &recv_buf[2], sizeof(pack_len));
		app_debug(DBG_INFO,"pack_len=%d\n", pack_len);
		if(pack_len > (expect_len-2)){
		    dbg_inf("continue to recv the rest %d byte\n",pack_len-expect_len+2);
    		ret = recv_data(socket_fd, &recv_buf[21], pack_len-expect_len+2);

    		if (ret <= 0) {
    			dbg_perror("recv:");
    			break;
    		} else {
    			app_debug(DBG_INFO,"tcp received from ajb server, ret=%d\n", ret);
    			//print_n_byte(recv_buf, expect_len);
    		}            
		}

		memcpy((char *)&type, &recv_buf[6], 2);
		app_debug(DBG_INFO,"type=%04X\n", type);
		
		dbg_inf("id=%s\n", &recv_buf[8]);
		memcpy(usr_id, &recv_buf[8], 10);

		face_recognize_score  = 0;

		char *tmp_score_ptr = NULL;
		tmp_score_ptr = strstr(&recv_buf[8], ":");
		app_debug(DBG_INFO,"tmp_score_ptr=%s\n", tmp_score_ptr);
		face_recognize_score = strtol(&tmp_score_ptr[1], NULL, 10);
		dbg_inf("face_recognize_score=%d\n", face_recognize_score);

	} while (0);

	net_close(&socket_fd);

	return face_recognize_score;
}

int check_face_jpeg(uint8_t *jpg_buf, int jpg_len)
{
	int ret = 0;
	char face_id[12] = {0,};
	int face_recognize_score = 0;
	int font_c = 0;
	char face_info[128] = {0,};
	long long cur_id = 0;
    char *errptr=NULL;	
    char lift_data[10]={};	    
    int diff_ms=0;
    struct timeval tv_pre, tv_now;
	if (assert_ptr(jpg_buf)) {
		return -1;
	}

    gettimeofday(&tv_pre, NULL);
	printf("##picture send time:  %d.%d s\n",tv_pre.tv_sec,tv_pre.tv_usec / 100000);

	ret = face_recognize_score = face_check_by_server(jpg_buf, jpg_len, face_id);
	if(face_recognize_score < 0){
	    frame_draw_text("连接服务器超时!", 1, COL_RED, DEF_FONT_SIZE, DEF_FONT_SIZE, 0, DET_HEIGH);//add by hgj
        tim_reset_event(result_clear_tim_id);
        gbl_data_set(&gbl_FR_stop,1);
        tim_reset_time(FR_wait_tim_id,TIME_500MS(setup_json_stop_period-1));        
        return -1;
	}
	gettimeofday(&tv_now, NULL);
    diff_ms = (tv_now.tv_sec - tv_pre.tv_sec) * 1000 + (tv_now.tv_usec - tv_pre.tv_usec) / 1000;
    printf("##face recognize diff_ms:%d\n",diff_ms);
    
    cur_id = strtoll(face_id,&errptr,10);
    if(cur_id == -1)
    {
        printf("server answer: no face detected!\n");
        return -1;
    }
    if(errptr == &face_id[0]){
        printf("err char: %s\n",errptr);
        return -1;
    }     
    char *name = get_name_by_id(face_id);  
	if (face_recognize_score >= setup_json_face_threhold)
	{
		gbl_data_set(&gbl_FR_stop,1);
		tim_reset_time(FR_wait_tim_id,TIME_500MS(setup_json_stop_period-1));
		lg_fail_cnt = 0;
		if(lg_show_name_enable){
            //sprintf(face_info, "%s(%s) %d|%d  门锁已开", name, face_id, setup_json_face_threhold, face_recognize_score);
            sprintf(face_info, "%s  门锁已开, 请进", name); 
            font_c = COL_BLU;
            frame_draw_text(face_info, 1, font_c, DEF_FONT_SIZE, DEF_FONT_SIZE, 0, DET_HEIGH);//add by hgj			
        }
        ui_unlock(1);     
        printf("cur_id= %lld,pre_id= %lld\n",cur_id,lg_pre_id);
		if(cur_id != lg_pre_id){
            audio_play(PMT_UNLOCK, DEV_VOL_PLAY);		
		    lg_pre_id = cur_id; 

/**add for photo upload**/
            //send_enter_record_to_server(FACE_UNLOCK_RECORD,face_id);
            send_face_enter_record_to_server(FACE_UNLOCK_RECORD,face_id,jpg_buf,jpg_len);
/**end**/		    
			lift_data[0] = 0x1B;
			lift_data[2] = ((face_id[4]-'0') << 4)|(face_id[5]-'0');
			lift_data[3] = ((face_id[6]-'0') << 4)|(face_id[7]-'0');
			uart_send_lift_data(lift_data);//呼梯   		    
            send_face_unlock_to_server(cur_id);
        }

	}
	else
	{
		if (++lg_fail_cnt >= 3) {
			lg_fail_cnt = 0;

			if (lg_show_name_enable) {
				font_c = COL_RED;
    			//sprintf(face_info, "%s(%s) %d|%d  未注册", name, face_id, setup_json_face_threhold, face_recognize_score);
    			sprintf(face_info, "未注册");
				frame_draw_text(face_info, 1, font_c, DEF_FONT_SIZE, DEF_FONT_SIZE, 0, DET_HEIGH);

				dbg_err("not match! face_recognize_score=%d\n", face_recognize_score);
				//add for photo upload
				memset(face_id,0,sizeof(face_id));
				send_face_enter_record_to_server(FACE_UNLOCK_FAIL_RECORD,face_id,jpg_buf,jpg_len);
			}

			ae_rule_algo_auto_sw();
		}
	}
	tim_reset_event(result_clear_tim_id);

	return ret;
}

//切换镜头处理算法
void sw_algo(int a,int b)
{
#ifdef _KEEP_OPEN
    if(!get_body_sense_result()){return;}//访客离开，挂起算法切换定时器
#endif    
    ae_rule_algo_auto_sw();
    tim_reset_event(algo_sw_tim_id);
    //frame_draw_text("未检测到人脸, 请注视摄像头重试", 1, COL_RED, 15, 15, 0, 220);    
}

//add for photo upload
void restart_algo_auto_sw(void)
{
    if(tim_get_time(algo_sw_tim_id) == -1){//考虑到人体感应返回值不稳定的问题
        tim_reset_event(algo_sw_tim_id);
    }
}

void server_recv_pause()
{
	gbl_data_set(&gbl_FR_stop,1);//recv_stop = 1;
	tim_suspend_event(result_clear_tim_id);
	tim_suspend_event(FR_wait_tim_id);
}

void server_recv_resume()
{
	gbl_data_set(&gbl_FR_stop,0);//recv_stop = 0;
	tim_reset_event(result_clear_tim_id);

	lg_pre_id=0;
    lg_fail_cnt=0;
}

//人脸检测线程
void *thread_face(void *arg)
{

	int i;
	int len;
	int pos[MAX_FACE*4];
	int ret = 0;
	char *buf = NULL;
	char qr_info[50]={0};
	char qr_buf[MSG_LEN_PROC]={0};
	//char is_night=0;
	buf = malloc(320*240);
	if(buf == NULL) {
			printf("buf alloc fail\n");
		return -1;
	}


#ifdef ZBAR
	/* create a reader */
	zbar_image_scanner_t *scanner = NULL;
	scanner = zbar_image_scanner_create();
	
	/* configure the reader */
	zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
	
	/* wrap image data */
	zbar_image_t *image = zbar_image_create();
	//zbar_image_set_format(image, *(int*)"GREY");
	zbar_image_set_format(image, *(int*)"Y800");
	zbar_image_set_size(image, 320, 240);
	zbar_image_set_data(image, buf, 320*240, zbar_image_free_data);

	int qr_count = 0, qr_interval = 0;

#endif

	//is_night=video_day_night_set();	
	//Set_CCD_Light(1);
#ifdef _FACE_DET	
    #ifndef _KEEP_OPEN	
    ae_rule_algo_init(setup_json_cam_algo,0,0);
    #endif 
    
    tim_reset_event(algo_sw_tim_id);
#endif

	while(running)
	{        
		if (0 > SensorRead(buf))
			continue;
#ifdef _FACE_DET        


    do{
    #ifdef _KEEP_OPEN      
        if(get_face_det_flag() == 0)
        {
            big_face = 0;
            qr_interval = 0;//访客离开后清0，保证下次能立即触发二维码识别
            usleep(100*1000);
            break;            
        }
        else 
    #endif        
        {
    		take_photo = len = face_detect(DET_HEIGH, DET_WIDTH, buf, pos, MAX_FACE*4);//pos:col, row, width, height

    		_frame_draw_block(0, 0, DET_WIDTH, DET_HEIGH, 0x00);
    		for (i = 0; i < len; i++)
    		{
    			frame_draw_sqar(pos[i*4], pos[i*4+1], pos[i*4+2]-1, pos[i*4+3]-1);//minus 1 for safe
    			if (pos[i*4+2]-1 > setup_json_face_size && pos[i*4+3] > setup_json_face_size){
    				big_face = 1;
    				tim_reset_event(algo_sw_tim_id);
    			}
    			else{
                    big_face = 0;
    			}
    		}
        }
    }while(0);
#else
        usleep(200*1000);
#endif

#ifdef ZBAR
        if (qr_count++ >= qr_interval){
            qr_count = 0;
        }
        else{
            continue;                        
        }
    /* scan the image for barcodes */
    	int n = zbar_scan_image(scanner, image);//The image format must be "Y800" or "GRAY".
    	// printf("zbar_scan_image ret:%d\n", n);
    	if (n <= 0) continue;
    	
    	/* extract results */
    	const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    	for(; symbol; symbol = zbar_symbol_next(symbol)) {
        	/* do something useful with results */
        	//zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        	const char *data = zbar_symbol_get_data(symbol);
        	printf("%s\n", data);
            //frame_draw_sqar(zbar_symbol_get_loc_x(symbol, 0), zbar_symbol_get_loc_y(symbol, 0), zbar_symbol_get_loc_size(symbol), zbar_symbol_get_loc_size(symbol));	
        	qr_buf[0] = MSG_FROM_UART;
        	qr_buf[1] = 1;//UART_1;
        	qr_buf[2] = 0xAF;
        	qr_buf[3] = 0;
            memcpy(&qr_buf[4],data,6);
            pipe_put(face_pipe_proc, qr_buf, MSG_LEN_PROC);
            buzz_play(BUZZ_BEEP);
            qr_interval = 5;//二维码识别成功后间隔1s左右再进行第二次
        	//frame_draw_text("                  ", 1, COL_RED, 15, 15, 0, 220);    
            //sprintf(qr_info, "%s",data);	
            //frame_draw_text(qr_info, 1, COL_BLU, 15, 15, 0, 220);
            //tim_reset_event(result_clear_tim_id);//add for photo upload
        }
#endif	
    
		usleep(1000);

	}
printf("--------------%s 3\n", __func__);
    big_face = 0;
	take_photo = 0;
    tim_suspend_event(algo_sw_tim_id);
	//SensorStop();
	frame_draw_block(0, 0, DET_WIDTH, DET_HEIGH, 0x00);
    //video_day_night_clr();
#ifndef ZBAR
	free(buf);
#endif

	
#ifdef ZBAR
	/* clean up */
	zbar_image_destroy(image);
	zbar_image_scanner_destroy(scanner);
#endif

printf("--------------%s 5\n", __func__);
return 0;
}

//发送图片给服务器
void *thread_enc(void *arg)
{
    if (thread_enc_running == 1) return NULL;
    HANDLE h;
    DWORD property = ENCODE_REALTIME;
    DWORD ret,jpg_len;
    thread_enc_running = 1;
    char *buf = malloc(JPEG_BUF);
	if (buf == NULL) {
		perror("malloc");
		goto end;
	}
printf("--------------%s 1\n", __func__);
    //pthread_mutex_lock(&face_mutex);
    h = VideoEncStart(ENCODE_JPEG, 640, 480, 10);
    if(h == INVALID_HANDLE_VALUE) {
        printf("INVALID_HANDLE_VALUE\n");
    	free(buf);
        goto end;
    }
printf("--------------%s 2\n", __func__);
    //pthread_mutex_unlock(&face_mutex);
    struct timeval tv_now;  
	gettimeofday(&tv_now, NULL);
	printf("[%s]tv_sec= %d,tv_usec= %d\n",__func__,(int)tv_now.tv_sec,(int)tv_now.tv_usec);
	
    char save_cnt=0;
    usleep(50000);

    while (running) 
    {        
        if (0 >= (jpg_len = VideoEncRead(h, buf, JPEG_BUF, &property))) 
        {                 
            usleep(30000);
            continue;//TODO.
        }
                      
		if (take_photo && big_face) //检测到人脸个数大于0，人脸大小大于指定值
		{            
            if (5120 > jpg_len) 
            {
                printf("VideoEncRead produce wrong picture: jpg_len=%d\n",jpg_len);                

                VideoEncStop(h);
                h = VideoEncStart(ENCODE_JPEG, 640, 480, 10);
                usleep(50000);
                continue;
            } 	
               
            if(gbl_data_get(&gbl_FR_stop)){
                //app_debug(DBG_INFO,"send stop!\n"); 
                usleep(100000);  
                continue;
            }
			//server_send(buf, ret);
          
			ret = check_face_jpeg(buf, jpg_len);
/*
            save_cnt++;
            if(save_cnt >5){
                save_cnt = 1;
            }
            char *string=(char [4]){};
            sprintf(string,"%d.jpg",save_cnt);
            int fd = open(string, O_WRONLY|O_TRUNC|O_CREAT, 0664);
            if (fd < 0) {
                printf("Cannot open file:%s for write\n", string);
                return;
            }
            write(fd, buf,jpg_len);
            fdatasync(fd);
            close(fd);
            dbg_inf("save picture in %d.jpg\n",save_cnt);*/
		}
    }
printf("--------------%s 3\n", __func__);


	free(buf);

    printf("VideoEncStop\n");
    VideoEncStop(h);
    
end:
    printf("VideoEncStop end\n");

thread_enc_running = 0;

}

void face_det_start()
{
	if (running == 1) return;
	running = 1;
	quit_monitor_mode();
	preview_start();    
    frame_start();
	usleep(100000);
#ifndef _KEEP_OPEN	
    Set_Back_Light(1);
#endif  

#ifdef _FACE_DET    
    server_recv_resume();
	pthread_create(&pid_enc, NULL, thread_enc, NULL);//encode init could fail because of face detect high cpu usage, make it way before detect start
	sleep(1);
    struct timeval tv_now;  
	gettimeofday(&tv_now, NULL);
	printf("[%s]tv_sec= %d,tv_usec= %d\n",__func__,(int)tv_now.tv_sec,(int)tv_now.tv_usec);
#endif    
	pthread_create(&pid_face, NULL, thread_face, NULL);	
}

void face_det_stop()
{
	if (running == 0) return;
	running = 0;

	pthread_join(pid_face, NULL);
	printf("thread_face exit ok\n");
#ifdef _FACE_DET     
	server_recv_pause();    
	pthread_join(pid_enc, NULL);
    printf("thread_enc exit ok\n");
#endif        
	frame_stop();
	preview_stop();
}


void clear_FR_result(int a, int b)
{
// 	frame_draw_block(0, 220, 320, 240-220, 0xff000000);//TODO dead code
	//frame_draw_text("                  系统正在运行", 1, COL_RED, DEF_FONT_SIZE, DEF_FONT_SIZE, 0, DET_HEIGH);
	frame_draw_text("                  ", 1, COL_RED, DEF_FONT_SIZE, DEF_FONT_SIZE, 0, DET_HEIGH);
	lg_pre_id=0;
    lg_fail_cnt=0;
}

void restart_send_recv(int a, int b)
{
    gbl_data_set(&gbl_FR_stop,0);//recv_stop= 0; 
    printf("%s: set recv_stop to 0\n",__func__);
}

/**add for photo upload**/

//#define _SAVE_IN_FILE
int fkly_backup(char **ptr, int *len)
{
#ifndef _PHOTO_UPLOAD
    return 0;
#endif
	HANDLE h;
	DWORD property = ENCODE_REALTIME;
	DWORD size;
	int i, ret = 0, retry = 0;
	int buf_size = 50*1024;
    dbg_inf("%s start!\n",__func__);	
    
	char *buf = malloc(buf_size);
	if (buf == NULL) {
		perror("malloc");
		ret = -1;
		goto end;
	}
	memset(buf,0,buf_size);

	h = VideoEncStart(ENCODE_JPEG, 320, 240, 10);

	if (h == INVALID_HANDLE_VALUE) {
		dbg_inf("INVALID_HANDLE_VALUE\n");
		free(buf);
		ret = -1;
		goto end;
	}

	usleep(1000);

	while (retry < 5) {
		if (5120 >= (size = VideoEncRead(h, buf, buf_size, &property))) {
			retry++;
			dbg_inf("%s:get invalid picture\n", __func__);
		}
        else{
#ifdef _SAVE_IN_FILE
			int fd = open("fkly.jpg", O_WRONLY | O_TRUNC | O_CREAT, 0664);

			if (fd < 0) {
				dbg_inf("Cannot open file:%s for write\n", "fkly.jpg");
				ret = -1;
				break;
			}

			write(fd, buf, size);
			fdatasync(fd);
			close(fd);
#else
			*ptr = buf;
			*len = size;
#endif
			break;
		}
	}

#ifdef _SAVE_IN_FILE
	free(buf);
#endif
	VideoEncStop(h);

end:
	dbg_inf("%s end\n",__func__);

	return 	ret;

}
/**end**/



void get_settings_from_json()
{
    char *p_set="192.168.22.106";

	int fd = open("id_name.json",O_RDONLY);
	if(fd<0)
	{
		printf("open %s error!\n","id_name.json");
		lg_show_name_enable = 0;
	}
	else{
        lg_show_name_enable = 1;
	    init_id_match_json_data("id_name.json");        
	}
	close(fd);      

	fd	= open("setup.json",O_RDONLY);
	if(fd<0)
	{
		printf("open %s error!\n","setup.json");
		goto no_json;
	}	
	close(fd); 
	
	if((setup_json_face_threhold = get_threshold_json_data("setup.json", "face_threhold"))== -1){
        setup_json_face_threhold = 70;
	}
    p_set=get_string_json_data("setup.json", "server_ip");
    if(p_set!=-1 && p_set!=NULL){
    	strcpy(setup_json_server_ip,p_set);
	}
    else{
        p_set="192.168.22.106";
        strcpy(setup_json_server_ip,p_set);
    }
	//setup_json_elec_0_magn_1 = get_threshold_json_data("setup.json", "elec_0_magn_1");
	//setup_json_magn_unlock_time = get_threshold_json_data("setup.json", "magn_unlock_time");
	if((setup_json_face_size = get_threshold_json_data("setup.json", "face_size"))== -1){
        setup_json_face_size = 40;
	}
	if((setup_json_server_port= get_threshold_json_data("setup.json", "server_port"))== -1){//by hgj
        setup_json_server_port=10888;
	}
	if((setup_json_stop_period = get_threshold_json_data("setup.json","stop_period"))== -1){//by hgj
        setup_json_stop_period=2;
	}
	if((setup_json_cam_algo = get_threshold_json_data("setup.json","cam_algo"))== -1){//by hgj
        setup_json_cam_algo=1;
	}
	if((setup_json_algo_sw_wait= get_threshold_json_data("setup.json","algo_sw_wait"))== -1){//by hgj
        setup_json_algo_sw_wait=6;
	}	

no_json:
    strcpy(setup_json_server_ip,p_set);//保证不存在setup.json可以使用初始值
	printf("face threhold:%d\n", setup_json_face_threhold);
	printf("server ip:%s\n", setup_json_server_ip);
	printf("face size:%d\n", setup_json_face_size);
	printf("server port:%d\n", setup_json_server_port);//by hgj
	printf("stop period:%d\n", setup_json_stop_period);//by hgj
	printf("cam_algo:%d\n", setup_json_cam_algo);//by hgj
	printf("algo_sw_wait:%d\n", setup_json_algo_sw_wait);//by hgj
	
    if(setup_json_stop_period<2){
        setup_json_stop_period = 2;
    }
    else if(setup_json_stop_period>6){
        setup_json_stop_period = 6;
    }
    printf("actual stop period:%d\n", setup_json_stop_period);//by hgj	

    if(setup_json_algo_sw_wait<2){
        setup_json_algo_sw_wait = 2;
    }
    else if(setup_json_algo_sw_wait>10){
        setup_json_algo_sw_wait = 10;
    }
    printf("actual algo_sw_wait:%d\n", setup_json_algo_sw_wait);//by hgj
}

int face_start(void *arg)
{
    struct msg_t *p_msg = (struct msg_t *)arg;
    int ret;
    face_pipe_proc = p_msg->pipe_proc; 
#ifdef _FACE_DET
	face_load(MODEL_PATH);//corresponding release func not given, only call at init 
#endif	
	get_settings_from_json();

	result_clear_tim_id = tim_set_event(TIME_1S(4), clear_FR_result, 0, 0, TIME_ONESHOT);
	FR_wait_tim_id = tim_set_event(TIME_500MS(setup_json_stop_period-1), restart_send_recv, 0, 0, TIME_ONESHOT);//by hgj
    algo_sw_tim_id = tim_set_event(TIME_500MS(setup_json_algo_sw_wait), sw_algo, 0, 0, TIME_ONESHOT);
	tim_suspend_event(result_clear_tim_id);
	tim_suspend_event(FR_wait_tim_id);//by hgj    
    tim_suspend_event(algo_sw_tim_id);
    
	ret = SensorStart(setup_json_cam_algo);
	if(ret != 0) {
		printf("SensorStart fail\n");
	}    

    return 0;
}
