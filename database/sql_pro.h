/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: sql_pro.h
* Author				: Ritchie
* Version				: V1.0.0
* Date				: 2012年9月24日
* Description			: 
* Modify by			: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#ifndef __SQL_PRO_H
#define	__SQL_PRO_H

#include <sqlite3.h>

#ifndef CARD_NET_VERSION
	#define	CARD_NET_VERSION		1
#endif


typedef struct SQL_TBL_ST{
	unsigned int mkey;
	int sid;
	int val;
	int date;
	int time;
	unsigned char note[20];	
	struct SQL_TBL_ST *next;
	struct SQL_TBL_ST *last;
}sql_data_t, para_data_t;

typedef struct SQL_VALUES{
    char *primary_key;
    int id;
    int val;
    int date;
	int time;
    char *parameter;
}sql_val_t;


typedef struct SQL_QUERY_INFO{
    char *db_name;
    char **output;
    int nrow;
    int ncolumn;
}sql_query_info_t;

//世界时间
#define	SQLDATA			"CUR_DATA"
#define	SQLTIME			"CUR_TIME"
#define	SQLTIMESTAP		"CUR_TIMESTAP"

//本地时间
#define	SQLDATA_L			"CUR_DATA_L"
#define	SQLTIME_L			"CUR_TIME_L"
#define	SQLTIMESTAP_L		"CUR_TIMESTAP_L"

//排序
typedef enum
{
	ORDER_DESC, //降序（大到小）
	ORDER_ASC,  //升序（小到大，默认使用）
}order_dir_t;


typedef enum
{
		SI_INTEGER=SQLITE_INTEGER,
		SI_FLOAT=SQLITE_FLOAT,
		SI_TEXT=SQLITE_TEXT,
		SI_BLOB=SQLITE_BLOB,//二进制
		SI_NULL=SQLITE_NULL,
		SI_REAL,
		SI_NEW_ITEM,                //新一行数据
}s_type_index;


#define DB_NOT_FULL 0
#define DB_FULL 1


typedef struct{
	sqlite3 *db;
	pthread_rwlock_t db_rw_lock;//加锁
}db_handle_t;



typedef struct {
	char *column_id;
	char *data_type;
	char *id_more;
}column_struct_t;


/*
 * 插入的结构
 */

typedef struct {
	void *data;
	int len;
	int row_pos;
}column_pack_i;

//blob
typedef struct BLOB_STRUCT
{
	char *data;
	int len;
	int pos;
	s_type_index type_index;
	struct BLOB_STRUCT *next;
}blob_struct;


typedef struct {
	char *cmd;
	blob_struct*blob_ret;
}insert_cmd_i;


/*
 * 查找的结构
 */

//行条件
typedef struct ROW_STRUCT_G{
	char *name;                   //列名字
	struct ROW_STRUCT_G *last;
	struct ROW_STRUCT_G *next;
}row_struct_g;

//排序
typedef struct ORDER_STRUCT_T{
	//char *name;                    //  列名字
	int row_pos;
	int dir;                   //  正序或则倒序   DESC
	struct ORDER_STRUCT_T *last;   //
	struct ORDER_STRUCT_T *next;   //  多个条件时向后添加
}order_struct_g;

/*typedef struct COLUMN_STRUCT_G{
	void *data;                    //条件数据内容
	int len;                       //条件数据长度
	int row_pos;
	char operator[10];                //操作符   ==,>等
	char relationship[10];            //和前一个条件之间的关系， AND、OR、LIKE、GLOB、NOT LIKE
}column_item_g;*/

//列条件
typedef struct COLUMN_STRUCT_G{
	//column_item_g *item;
	void *data;                    //条件数据内容
	int len;                       //条件数据长度
	int row_pos;
	char operator[10];                //操作符   ==,>等
	char relationship[10];            //和前一个条件之间的关系， AND、OR、LIKE、GLOB、NOT LIKE
	struct COLUMN_STRUCT_G *last;    //非空需要添加relationship，
	struct COLUMN_STRUCT_G *next;    //多个条件时向后添加
}column_struct_g;

typedef struct
{
	row_struct_g    *row_id;      // 列条件
	column_struct_g *column_id;   // 行条件
	order_struct_g  *order_pri;   // 排序优先级
}query_struct_g;

struct ROW_SELECT_T
{
	int row_pos;
}row_select_t;

typedef struct QUERY_CMD_T
{
	char *query_cmd;
}query_cmd_g;


typedef struct QUERY_RET_T
{
	union             //返回的数据
	{
		void 	*ptr_ret;
		double dou_ret;
		int 	int_ret;
	};
	int data_len;      //数据长度
	int icol;          // 列号，从左到右 0开始
	int data_type;     // 数据类型
}data_ret_g;


typedef struct
{
	column_pack_i *update_array;   //set 更改的值列表
	int list_count;               //列表中元素数目
	column_struct_g *column_id;   //列条件
}update_struct_i;

int sql_init(void);
int query_id(int qid, unsigned char *su);
int modify_id(int mid, const unsigned char *const guid);
void sql_exit(void);
int query_by_mid(para_data_t *qbm);

int request_cameras_inf(const uint8_t *const dev_no);
int load_ip_camera_data(void);

#ifdef DOOR_ACCESS_CARD_LIST
	int match_du_card_no(const uint64_t cardnumber, unsigned char room[]);
#endif

int datebase_open(const char *filename,db_handle_t **ppDb);
//void start_db_mem_release(void);
void datebase_close(db_handle_t *ppDb);

int databsae_create_tbl(db_handle_t * db_h, char * tabname,const column_struct_t * column,int time_dflt);
insert_cmd_i  build_insert_cmd(const column_struct_t * column,const char *tabname,column_pack_i *data,int data_items);
int insert_data_to_table(db_handle_t *db,insert_cmd_i sql_cmd);
void free_insert_cmd(insert_cmd_i sql_cmd);


void free_insert_cmd(insert_cmd_i sql_cmd);

query_struct_g *alloc_query_struct(void);
void free_query_struct(query_struct_g *qurey_cmd);

int add_column_id(query_struct_g *r_row_id,char *id);
int add_order_pri(query_struct_g *r_order_id,int row_pos,order_dir_t dir);
int add_query_column_val(query_struct_g*r_column_id,void *data,int len,int row_pos,char operator[10],char relationship[10]);
//int build_query_cmd(const column_struct_t * column, char *tabname,query_struct_g*qurey_cmd,query_cmd_g *resault);
int build_query_cmd(const column_struct_t * column_array, char *tabname,\
		query_struct_g*qurey_cmd,char *add,query_cmd_g *resault);
void * query_data_from_table(db_handle_t *db_h, query_cmd_g *query_cmd,int *count,\
		void*call_back_fun(data_ret_g *ret,void **cur_item,void **list_head));

int build_delete_cmd(const column_struct_t * column_array,const char *tabname,query_struct_g*qurey_cmd,\
		query_cmd_g *resault);
int delete_data_form_table(db_handle_t *db_h, query_cmd_g *query_cmd);



update_struct_i *alloc_update_struct(void);
void free_update_struct(update_struct_i *update_list);



void add_update_array(update_struct_i*update_list,column_pack_i *update_array,int list_count);
int add_update_column_val(update_struct_i*update_list,void *data,int len,int row_pos,\
		char operator[10],char relationship[10]);
int build_update_cmd(const column_struct_t * column_array, char *tabname,update_struct_i *update_list,query_cmd_g *resault);
int update_data_to_table(db_handle_t *db_h, query_cmd_g *query_cmd);

int build_query_cmd_vspf(query_cmd_g *resault,char *fmt, ...);
int databsae_create_table_vspf(db_handle_t * db_h, char *fmt, ...);

void free_query_cmd(query_cmd_g *resault);


inline void add_insert_comut(void);

int info_container_init(void);
int ack_net_to_server(int net_cmd, void *buf,int len, int num);

#endif
