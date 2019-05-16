/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: sqlite_data.h
* Author				: Ritchie
* Version				: V1.0.0
* Date					: 2013年10月10日
* Description			: 
* Modify by				: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#ifndef __SQLITE_DATA_H
#define	__SQLITE_DATA_H
#include "sqlite_ui.h"

			/*防区，住户，门磁，边界，布防状态*/
typedef enum{ALARM_FQ, ALARM_ZH=0x100,ALARM_MC,ALARM_FC, ALARM_BJ,ALARM_ONOFF, DEF_ALL=0xFFF}alarm_type_t;
typedef enum{MISSED_CALL, USL_CALLED, USL_CALLING}history_type_t;


#define	insert_alarm_zh(x) 		insert_alarm_info(x, ALARM_ZH, 0)
#define	insert_alarm_mc(x) 		insert_alarm_info(x, ALARM_MC, 0)
#define	insert_alarm_fc(x) 		insert_alarm_info(x, ALARM_FC, 0)
#define	insert_alarm_fq(x, y) 	insert_alarm_info(x, y, 0)
#define	insert_alarm_bj(x) 		insert_alarm_info(x, ALARM_BJ, 0)
/* 布撤防状态*/
#define	insert_alarm_bcf(x, y) 	insert_alarm_info((x), ALARM_ONOFF, (y));
#define clear_card_unlock()     delete_card_unlock(1, 0)
#define clear_card_unlock_ex() 	delete_card_unlock_ex(1,0)

#pragma pack(1)

typedef struct ST_ARLARM_IN_SQL{
	int id;
	int dev_no;
	int alarm_type;
	int status;
	int reserved1;
	int reserved2;
	uint8_t date_time[20];
	struct ST_ARLARM_IN_SQL *next;
	struct ST_ARLARM_IN_SQL *last;
}alarm_data_t;


typedef struct ST_CARD_UNLOCK_SQL{
	int id;
	unsigned int dev_no;
	unsigned int card_no;
	unsigned int validity;
	int reserved2;
	int reserved3;
	uint8_t date_time[20];
	struct ST_CARD_UNLOCK_SQL *next;
	struct ST_CARD_UNLOCK_SQL *last;
}card_unlock_t;


typedef struct ST_CARD_UNLOCK_SQL_EX{
	int id;
	unsigned int dev_no;
	unsigned int card_no;
	unsigned int validity;
	unsigned int blacklist;
	int reserved1;
	uint8_t date_time[20];
	struct ST_CARD_UNLOCK_SQL *next;
	struct ST_CARD_UNLOCK_SQL *last;
}card_unlock_t_ex;

typedef struct ST_CALL_HISTORY_SQL{
	int id;
	int dev_no;
	int call_type;
	int reserved1;
	int reserved2;
	int reserved3;
	uint8_t date_time[20];
	struct ST_CALL_HISTORY_SQL *next;
	struct ST_CALL_HISTORY_SQL *last;
}call_history_t;

#pragma pack(1)
typedef struct ST_PACK_CONTACTS_SQL{
	int id;
	uint16_t uid;
	uint8_t grade;
	uint16_t pid;
	char dev_name[36];
	int dev_no;
	struct ST_CONTACTS_SQL *next;
	struct ST_CONTACTS_SQL *last;
}pack_contact_t;
#pragma pack()

typedef struct ST_CONTACTS_SQL{
	int id;
	int uid;
	int grade;
	int pid;
	char dev_name[36];
	int dev_no;
	struct ST_CONTACTS_SQL *next;
	struct ST_CONTACTS_SQL *last;
}contact_t;

typedef struct ST_MLT_CONTRACTS_SQL{
	int flag;
	int count;
	int idx;
	uint level;
	uint depth;
	contact_t addr_book;
	struct ST_MLT_CONTRACTS_SQL *parent;	
	struct ST_MLT_CONTRACTS_SQL *sub;
}mlt_cntct_t;

typedef struct ST_CONTROLLERS_DATA{	
	uint16_t ext_id;
	uint16_t sub_id;
}controller_data_t;

typedef struct ST_SUB_CONTROLLERS_SQL{
	int id;
	controller_data_t data;
	struct ST_SUB_CONTROLLERS_SQL *next;
	struct ST_SUB_CONTROLLERS_SQL *last;
}controoler_sql_t;

typedef struct SQL_IP_CAMERA_INFO{
	int id;
	int dev_ip;
	uint16_t port;
	uint16_t channel;
	char dev_name[22];
	struct SQL_IP_CAMERA_INFO *next;
	struct SQL_IP_CAMERA_INFO *last;
}ip_camera_inf_t;

typedef struct ST_PACKED_VLAN_LIST_SQL{	
	uint32_t dev_no;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t ipaddr;
	uint32_t reserved3;
}pkd_vlan_list_t;

typedef struct ST_VLAN_LIST_SQL{
	int id;
	pkd_vlan_list_t data;
	struct ST_VLAN_LIST_SQL *next;
	struct ST_VLAN_LIST_SQL *last;
}vlan_list_t;




#define	free_card_ulck_data 		free_alarm_data
#define	free_call_hstry_data 		free_alarm_data


#define DOOR_ACCESS_CARD_LIST

#ifdef DOOR_ACCESS_CARD_LIST
#define	release_query_list(x)	free_query_list((x).next,  sizeof(x))

/*
0：清空再插入
1：增加
2：按房号删除
3：按卡号删除
4：删除黑名单
5：删除过期卡

*/
enum{DROPB4INSERT=0, ADDCARDS, DLT_BY_DEVNO, DLT_BY_CARD, DLT_BY_BLACK, DLT_BY_EXPIRY};
typedef struct CARD_NET_CMD_ST{
	int rsp_number;
	uint8_t *head;
	uint8_t version;
	uint8_t cmd;
	uint16_t dev_no;
	int32_t count;
	//uint8_t	*data;
}card_net_cmd_t;

typedef struct DATA_TBL_CARD{
	uint32_t dev_no;		//1楼单元层房号
	//unsigned char dev_no;		//1楼单元层房号
	uint64_t cardnumber;	//2卡号，最大8字节，只有4字节时[0~3]填充0		
	uint8_t init_date[6];	//3启用日期，[0~6]依次：年、月、日、时、分、秒
	uint8_t exprirydate[6];	//4失效日期，[0~6]依次：年、月、日、时、分、秒
	uint8_t blacklist;		//5是否列入黑名单,0：白名单，其它：黑名单
	uint8_t reserved0;		//6预留，填充0
	uint8_t card_type;		//7预留，卡类型定义，参见表2“卡类型定义”	
	uint8_t reserved1;		//8预留
	uint32_t reserved2; 	//9预留
}card_data_t;

typedef struct SQL_CARD_TBL_ST{
	int mkey;
	card_data_t data;
	struct SQL_CARD_TBL_ST *next;
	struct SQL_CARD_TBL_ST *last;
}card_list_sql_t;
#endif


#pragma pack(1)
typedef struct ST_CARD_INFO_SQL{
	int id;
	uint8_t dev_no[4];		//楼单元层房号
	uint8_t cardnumber[8];	//卡号，最大8字节，只有4字节时[0~3]填充0
	uint8_t init_date[6];	//启用日期，[0~6]依次：年、月、日、时、分、秒
	uint8_t exprirydate[6];	//失效日期，[0~6]依次：年、月、日、时、分、秒
	uint8_t blacklist;		//是否列入黑名单,0：白名单，其它：黑名单
	uint8_t reserved0;		//预留，填充0
	uint8_t card_type;		//预留，卡类型定义，参见表2“卡类型定义”
	uint8_t reserved1;		//预留
	uint32_t reserved2;		//预留
	struct ST_CARD_INFO_SQL *next;
	struct ST_CARD_INFO_SQL *last;
}dcard_list_t;

typedef struct  PACKED_DOOR_CARD_SQL2{
/*	uint32_t dev_no;
	uint32_t number;
	uint32_t exdate;
	uint32_t black;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	*/
	uint8_t dev_no[4];		//楼单元层房号
	uint8_t cardnumber[8];	//卡号，最大8字节，只有4字节时[0~3]填充0
	uint8_t init_date[6];	//启用日期，[0~6]依次：年、月、日、时、分、秒
	uint8_t exprirydate[6];	//失效日期，[0~6]依次：年、月、日、时、分、秒
	uint8_t blacklist;		//是否列入黑名单,0：白名单，其它：黑名单
	uint8_t reserved0;		//预留，填充0
	uint8_t card_type;		//预留，卡类型定义，参见表2“卡类型定义”
	uint8_t reserved1;		//预留
	uint32_t reserved2;		//预留
}pdoor_card2_t;


#pragma pack()


int insert_alarm_info(const int dev_no, alarm_type_t type, int status);
int insert_card_unlock_info(int id, int dev_no, int card_no, int validity);
int insert_card_unlock_info_ex(int id, int dev_no, int card_no, int validity,int blacklist);
int free_alarm_data(alarm_data_t *const fptr);
int insert_contacts(pack_contact_t *pc_data);
int insert_call_history(int dev_no, history_type_t call_type);
int do_query_sql(sqlite3 *db, sql_cmd_t *qcmd);
int query_alarm_info(alarm_data_t *const dest, const alarm_type_t type, int status);
int delete_alarm_info(const int dev_no, const alarm_type_t type);
int query_address_book(contact_t *dest, sql_cndtn_t *conditions, const int offset);
int free_contacts_data(contact_t *const fptr);
int query_call_history_info(call_history_t *const dest, history_type_t type, int offset);


int do_save_inf_container(const void *const buf, const int len);
int do_load_info_container(void *buf,int len);
int query_info_controller(int opt, controoler_sql_t *dest);
int free_controllers_data(controoler_sql_t *const fptr);

int insert_ip_camers(ip_camera_inf_t *pc_data);
int query_ip_camera_inf(int opt, ip_camera_inf_t *dest);
int query_card_unlock_info(card_unlock_t *const dest, int offset);
int query_card_unlock_info_ex(card_unlock_t_ex *const dest, int offset);


int delete_adress_book(const int dev_no);
int sqlite_data_init(sqlite3 *db);
void sql_data_exit(void);
int free_query_list(void * fptr, const int size);

int delete_card_unlock(int clear, unsigned int card_no);
int delete_card_unlock_ex(int clear, unsigned int card_no);
int do_load_catalog(char *buf, int len);
int do_save_catalog(char *buf, int len);
int do_load_card_unlock(char *buf, int len);
int do_save_card_unlock(char *buf, int len);
int do_load_card_unlock_ex(char *buf, int len);
int do_save_card_unlock_ex(char *buf, int len);
int is_card_unlock(unsigned int card_no, unsigned char room[]);
int is_card_unlock_ex(unsigned int card_no, unsigned char room[]);
int is_room_exist(unsigned int card_no, unsigned char room[]);


int insert_vlan_list_data(pkd_vlan_list_t *pc_data);
int query_vlan_list(vlan_list_t *dst, int opt);
int delete_vlan_list(void);
int store_vlan_list_data(uint8_t *buf, int len);
int do_load_vlan_list_data(uint8_t *dbuf, int len);


int sqldata_routine_begin(const int opt);
int sqldata_routine_submit(const int res);


int store_door_card_number_8byte_info(const int num, const int len);
int query_door_card_list_number_8byte(dcard_list_t *dst, int opt);
void * do_load_door_card_number_8byte_inf(uint32_t *bytes);

/*
 * dst:输入参数，待删除操作的条件信息
 * index：表示dcard_list_t的第几个元素,同数据库中的表的列相同,从0开始
 */

int delete_door_card_list_number_8byte(pdoor_card2_t *dst, int index);
int do_store_door_card_number_8byte_info(const void *const *args);
int delete_sfz8or4_cardreader_list(void);


#endif

