#ifndef __FINGERWEL_H__
#define __FINGERWEL_H__


struct person{
	int id;
	char name[10];
};

struct person finger[1024];
int finger_num;
int finger_ID;
int finger_regID;
typedef struct _Finger_buff_Struct_
{
	struct  
	{
		unsigned char head[2];		//包头
		unsigned char address[4];	//芯片地址
		unsigned char sign;			//包标识
		unsigned char length[2];		//包长度
		unsigned char command;		//指令
		unsigned char fdata[64];		//数据
	} new_trans;
	struct  
	{
	       unsigned char head[2];		//包头
		unsigned char address[4];	//芯片地址
		unsigned char sign;			//包标识
		unsigned char length[2];		//包长度
		unsigned char respond;		//返回码
		unsigned char fdata[64];		//数据
	} new_rev;
	unsigned char  buf[74];
}Finger_buff,*P_Finger_buff;


Finger_buff comm_buff;	                           //接发收数据缓存

//半导体与光学协议
/*****************协议包定义***********************/
#define C_COMMAND               0x01//命令包

/*****************缓冲区定义***********************/
#define CharBufferA             0x01
#define CharBufferB             0x02

/*****************指令码定义***********************/
#define C_GetImage              0x01//从传感器上读入图像
#define C_GenTemplet            0x02//根据原始图像生成指纹特征
#define C_Search                0x04//以CharBufferA或 CharBufferB中的特征文件搜索整个或部分指纹库 
#define C_MergeTwoTemplet       0x05//将CharBufferA与CharBufferB中的特征文件合并生成模板存于ModelBuffer 
#define C_StoreTemplet          0x06//将ModelBuffer中的文件储存到flash指纹库中
#define C_EraseAllTemplet       0x0d//清空flash指纹库
#define C_TempleteNum						0x1d//读取有效指纹个数
#define C_DeletChar							0x0c//删除某个指纹

/*****************60系列专用(不兼容半导体模块，暂不用)***********************/
#define C_OpenLED					0x50//打开模块灯
#define C_CloseLED				0x51//关闭模块灯
#define C_GetImageFree		0x52//无灯控采集指纹  配合以上命令使用
#define C_GetEcho					0x53//握手
#define C_AotoSave				0x54//自动搜索登记
#define C_AotoSearch			0x55//自动搜索自纹

/*****************函数反馈结果**********************/
#ifndef TRUE
#define TRUE					0x01
#endif
#ifndef FALSE
#define FALSE				0X00
#endif
#define TIMEOUT				0x03			//超时
#define NO_FINGER                   0x04			//没有手指
#define PIC_LONG			0x05			//图像太乱
#define LOW_FEATURE		0x06			//特征点太少
#define RESIDUAL_FINGER       0x07		//残留指纹
#define NO_SEARCH_IN_INTERVAL	0x08	//在指定区间没有找到

#define GET_IMAGEOK_FIRST      0x56
#define GET_IMAGEOK_SECOND   0x57
#define ERR_DUP_FINGER              0x24

typedef enum enum_FingerErr
{
       FINGER_OK = 0,				/*!< 成功*/
	ERR_PACKET = 0x01,			/*!< 包错误*/
	ERR_NO_FINGER = 0x02,		/*!< 传感器上无手指*/
	ERR_GET_IMG = 0x03,		/*!< 图像录入不成功*/

	//////////////////////////////////////////
	ERR_IMG_NOT_CLEAN=0x06,  /*!<采集的图像不清楚 */
	ERR_IMG_NOT_ENOUGH=0x07,  /*!<采集的图像的特征点不足 */
	ERR_NOT_MATCH=0x08,		/*!<指纹不匹配 */
	ERR_SEARCH=0x09,			/*!<没有搜索到指纹 */

	//////////////////////////////////////////////
	ERR_GEN_CHAR_FAIL=0x0a,  /*!<生成特征失败,将char1和char2中的特征文件合成模板失败 */
	ERR_PAGE_RANGE=0x0b,			/*!<表示PageID 超出指纹库范围 */
	ERR_RD_TEMPLATE=0x0c,	/*!<读出的模板数据有错或者无效 */
	ERR_UP_FEATURE=0x0d, /*!<上传特征失败 */
	ERR_CONTINUE_RECV=0x0e,  /*!<不能继续接收后续的包 */
	ERR_UP_IMG=0x0f,			/*!<上传图像失败 */
	ERR_DEL_TEMPLATE=0x10,	/*!<删除模板失败 */
	ERR_CLEAR_FINGER_DB = 0x11, /*!<清空指纹库失败 */

	//////////////////////////////////////////
	ERR_PASSWORD=0x13,		/*!<口令不正确 */
	///////////////////////////////////////////
	ERR_NO_IMG=0x15,			/*!<图像缓冲区中没有图像 */
	///////////////////////////////////////////
	ERR_RW_FLASH=0x18,		/*!<读写FLASH 出错 */
	///////////////////////////////////////////////////
	ERR_REG_INDEX=0x1a,		/*!<寄存器序号错误 */
	//////////////////////////////////////////////////
	ERR_ADDRESS=0x20, /*!<地址码错误 */
	ERR_AUTH = 0x21,		/*!<必须验证口令 */
	////////////////////////////////////////////////////
	ERR_TIMEOUT,		/*!<接收到的数据超时获取长度不对 */
	ERR_RS232,			/*!<接口通信故障 */
	ERR_GEN_TEMP_FAIL,  /*!<生成模板失败 */
       ERR_INVALID_VAL	/*!<参数错误 */

}ERR_FINGER;



#define  fp_wait_time2s                            0x1f
#define  fp_wait_time_2_5s                      0x26
#define  fp_wait_time3s                            0x2e
#define  fp_wait_time_3_5s                      0x36
#define  fp_wait_time4s                            0x3e
#define  fp_wait_time_4_5s                      0x45
#define  fp_wait_time_5s                          0x4d
#define  fp_wait_time_5_5s                      0x55

#define  fp_same_save_en                  0x01
#define  fp_same_save_unen              0x00

#define  fp_collect_2time                   0x02
#define  fp_collect_3time                      0x03

#define  C_ReadConList                           0x1F
#define  DEVICE_ADDRESS                       0xFFFFFFFF
#define  PACKET_PID                               0x01
#define  fp_device_capacity                    1000
#define  fp_device_size                   1000


#define  status_non      			 0x00
#define  status_getimg 			 0x01
#define  status_img2tz  			 0x02
#define  status_serarch 		        0x03
#define  status_index				 0x04
#define  status_reg                           0x05
#define  status_reg_first                   0x06
#define  status_reg_second              0x07
#define  status_reg_ok                     0x08
#define  status_format			 0x09
#define  status_delect			 0x0A
#define  status_download			 0x0B
#define  status_downloading	        0x0C
#define  status_reg_timeout 			 0x0D

#define  CMD_PACKET                       0x01
#define  DATA_PACKET                      0x02
#define  ACK_PACKET                        0x07
#define  ENDDATA_PACKET               0x08

void SendCmdPacket(unsigned int DeviceAddress,
	unsigned char Packet_pid,
	unsigned int length,
	unsigned char CmdCode,
	unsigned char *p_data);

void fp_get_image(void);
void fp_generate_template(unsigned char  ramBuffer);
void fp_readcolist(unsigned char page_nu);
void fp_merge_template(void);
void fp_empty_all(void);
int fp_delete_char(unsigned int template_id,unsigned char templet_count);
void fp_auto_reg_templet(unsigned char wait_time,unsigned char wait_count,
	unsigned char STemplate_h,unsigned char STemplate_l,unsigned char repeat_flag);

void fp_auto_search_templet(unsigned char wait_time,unsigned char start_nu_h,
	unsigned char start_nu_l,unsigned char search_nu_h,unsigned search_nu_l);

void fp_search_template(unsigned char rambuff,unsigned int start_Page,unsigned int Page_nu);
void fp_status_set(int status);
int fp_status_get();
int Read_conlist(unsigned char *index,int page);
extern global_data  gbl_finger_status;
void proc_fpdata_uart(unsigned char *data);
void fp_time_int(void);
extern void   finger_start(void *arg);
extern void pause_fp_polling(void);
//extern void Downloadtemplet(void);
#endif
