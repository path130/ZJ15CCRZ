/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: sqlite_data.c
* Author				: Ritchie
* Version				: V1.0.0
* Date				: 2013??10??10??
* Description			: 
* Modify by			: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sqlite3.h>
#include <time.h>

//#define		DEBUG_ERROR
#include "my_types.h"
#include "my_debug.h"
#include "sqlite_ui.h"
#include "ipc_ctrl.h"

#include "sqlite_data.h"
#include "sql_pro.h"
#include "common.h"
#include "dev_config.h"
#include "public.h"

#pragma pack(1)

static const sql_column_t lg_dcard_inf[] = {
	{"ID",				SQLIPKEY},
	{"dev_no",			SQLINT},
	{"cardnumber",	SQLBLOB},
	{"init_date",	   SQLBLOB},
	{"exprirydate",	SQLBLOB},
	{"blacklist",		SQLINT},
	{"reserved0",		SQLINT},
	{"card_type",		SQLINT},
	{"reserved1",		SQLINT},
	{"reserved2",		SQLINT},
	{NULL,	NULL},
};


/*----------------------------------------------------------------*/
/* ??????*/
static const sql_column_t call_history[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"dev_no", 			SQLINT},
	{"call_type",    	SQLINT},
	{"reserved0",      	SQLINT},
	{"reserved1",     	SQLINT},
	{"reserved2",     	SQLINT},
	{"datetime",		SQLDATETIME},
	{NULL,	NULL},
};

/* ???*/
static const sql_column_t contact[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"uid", 			SQLINT},
	{"grade",			SQLINT},
	{"pid",    			SQLINT},
	{"dev_name",		SQLVNCHAR(36)},
	{"dev_no",			SQLINT},
	{NULL,	NULL},
};

static const sql_column_t alarm_info[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"dev_no", 			SQLINT},/* ????????*/
	{"alarm_type",    	SQLINT},/* ???????*/
	{"status",      	SQLINT},/* ????????*/
	{"reserved1",     	SQLINT},
	{"reserved2",     	SQLINT},
	{"datetime",		SQLDATETIME},
	{NULL,	NULL},
};
#if 0
static const sql_column_t card_unlock[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"dev_no", 			SQLINT},/* ????????*/
	{"card_no",    		SQLINT},/* ???????*/
	{"validity",      	SQLINT},// reserved0 modify by wrm 20150520
	{"blacklist",     	SQLINT},
	{"reserved2",     	SQLINT},
	{"datetime",		SQLDATETIME},
	{NULL,	NULL},
};
#endif

static const sql_column_t card_unlock[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"dev_no", 			SQLINT},/* ????????*/
	{"card_no",    		SQLINT},/* ???????*/
	{"validity",      	SQLINT},
	{"reserved1",     	SQLINT},
	{"reserved2",     	SQLINT},
	{"datetime",		SQLDATETIME},
	{NULL,	NULL},
};

static const sql_column_t card_unlock_ex[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"dev_no", 			SQLINT},/* ????????*/
	{"card_no",    		SQLINT},/* ???????*/
	{"validity",      	SQLINT},// reserved0 modify by wrm 20150520
	{"blacklist",     	SQLINT},
	{"reserved1",     	SQLINT},
	{"datetime",		SQLDATETIME},
	{NULL,	NULL},
};


static const sql_column_t lg_sub_controller[] = {
	{"ID", 				SQLIPKEY},
	{"ext_id",			SQLINT},
	{"sub_id",			SQLINT},
	{NULL,	NULL},
};

static sql_column_t const lg_ip_cameras[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"dev_ip",    		SQLINT},
	{"port",      		SQLINT},
	{"channel",     	SQLINT},
	{"name",     		SQLTEXT},
	{NULL,	NULL},
};

static const sql_column_t lg_vlan_list[] = {
		{"ID",				SQLIPKEY},
		{"dev_no",			SQLINT},
		{"reserved1", 		SQLINT},
		{"reserved2", 		SQLINT},
		{"ipaddr",			SQLINT},
		{"reserved3",		SQLINT},
		{NULL,	NULL},
	
};


static const char tbl_alarm[] = {"alarm_info"};
static const char tbl_card_unlock[] = {"card_unlock_info"};
static const char tbl_card_unlock_ex[] = {"card_unlock_info_ex"};
static const char tbl_call_history[] = {"call_history"};
static const char tbl_contact[] = {"contact"};
static const char tbl_sub_controller[] = {"sub_controller"};
static const char tbl_ip_camera[] = {"ip_camera"};
static const char tbl_vlan_list[] = {"vlan_list"};

static sqlite3 *lg_db_handle = NULL;
static alarm_data_t lg_alarm_info = {0,};

#ifdef DOOR_ACCESS_CARD_LIST
static const char lg_tbl_ncard_inf[] = {"door_card_inf"};
static const char tbl_dcard_inf[] = {"card_inf"};
/* ????*/
static const sql_column_t lg_st_ncard_inf[] = {
	{"ID",			SQLIPKEY},
	{"dev_no",		SQLINT},
	{"cardnumber", 	SQLTEXT},
	{"init_date", 	SQLTEXT},
	{"exprirydate", SQLTEXT},
	{"blacklist",	SQLINT},
	{"reserved0",	SQLINT},
	{"card_type",	SQLINT},
	{"reserved1",	SQLINT},
	{"reserved2",	SQLINT},
	{NULL,	NULL},
};
/*
typedef struct DATA_TBL_CARD{
	uint8_t dev_no[4];		//??????
	uint8_t cardnumber[8];	//??,??8??,??4???[0~3]??0
	uint8_t init_date[6];	//????,[0~6]??:???????????
	uint8_t exprirydate[6]; //????,[0~6]??:???????????
	uint8_t blacklist;		//???????,0:???,??:???
	uint8_t reserved0;		//??,??0
	uint8_t card_type;		//??,?????,???2??????
	uint8_t reserved1;		//??
	uint32_t reserved2; 	//??
}card_data_t;

*/

/**********************************************************************************
*Function name	: DO_MALLOC
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void *do_malloc(size_t size)
{
	void *ptr = NULL;
	
	ptr = malloc(size);
//	dbg_inf("++++++++++++++++++++[%08X]\n", (int)ptr);

	return ptr;
}


/**********************************************************************************
*Function name	: CREATE_DOOR_CARD_TBL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int create_door_card_tbl(sqlite3 *db)
{
	if(assert_ptr(db)) {
		return -1;
	}
		
	return db_create_tbl(db, lg_tbl_ncard_inf, lg_st_ncard_inf, FALSE);
}

/**********************************************************************************
*Function name	: INSERT_NDOOR_CARD_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_ndoor_card_data_buf(char *pc_data_buf)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;

	
	if(assert_ptr(pc_data_buf)) {
		return -1;
	}
	sqlite3 *db = lg_db_handle;

	//dbg_inf("insert door cards list\n");
	

	rc = db_build_sql_init(db, &binsert, DINSERT, tbl_ptr, colum_ptr);
	
	rc = db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	if(rc < 0) {
		dbg_err("fill blob");
		return rc;
	}

	
	
	/*
	dbg_blue("[%0X],[%llX]\n", pc_data->dev_no, pc_data->cardnumber);	
	dbg_blue("init_date\n");
	print_n_byte(pc_data->init_date, 6);
	dbg_blue("exprirydate\n");
	print_n_byte(pc_data->exprirydate, 6);
	*/


	printf("&&&&&&&&&&&&&&&&&&&\n");
	print_n_byte(pc_data_buf, 40);
	printf("%d,%d,%d,%d\n",sizeof(card_data_t),sizeof(uint64_t),sizeof(uint32_t),sizeof(uint8_t));
	printf("&&&&&&&&&&&&&&&&&&&\n");
	

#if 1
	 card_data_t *pc_data=alloca(40);    
	 memcpy(&(pc_data->dev_no),   &pc_data_buf[0], 4); 
     memcpy(&(pc_data->cardnumber),  &pc_data_buf[4], 8);
     memcpy(&(pc_data->init_date), &pc_data_buf[12], 6);
     memcpy(&(pc_data->exprirydate), &pc_data_buf[18], 6);
     memcpy(&(pc_data->blacklist), &pc_data_buf[24], 1);
     memcpy(&(pc_data->card_type), &pc_data_buf[26], 1);    
#endif
     

	dbg_blue("dev_no=[%0X],cardnumber=[%llX]=[%llu]\n", pc_data->dev_no, pc_data->cardnumber, pc_data->cardnumber);
	print_n_byte((const uint8_t * const) (&pc_data->dev_no), 4);
	print_n_byte((const uint8_t * const) (&pc_data->cardnumber), 8);
	dbg_blue("blacklist=[%0X],card_type=[%X]=[%c]\n", pc_data->blacklist, pc_data->card_type, pc_data->card_type);
	dbg_blue("init_date:\n");
	print_n_byte(pc_data->init_date, 6);
	dbg_blue("exprirydate:\n");
	print_n_byte(pc_data->exprirydate, 6);

	dbg_blue("init_date=[%0X],exprirydate=[%0X]\n", pc_data->init_date, pc_data->exprirydate);

#if 1

	//C1
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->dev_no, sizeof(pc_data->dev_no));
	rc = db_build_sql_stmt(&binsert, 1, &data, colum_ptr);
	//C2
	//pc_data->cardnumber += random()%100;
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->cardnumber, sizeof(pc_data->cardnumber));
	rc = db_build_sql_stmt(&binsert, 2, &data, colum_ptr);
	//C3
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->init_date, sizeof(pc_data->init_date));
	rc = db_build_sql_stmt(&binsert, 3, &data, colum_ptr);
	//C4
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->exprirydate, sizeof(pc_data->exprirydate));
	rc = db_build_sql_stmt(&binsert, 4, &data, colum_ptr);
	//C5
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->blacklist, sizeof(pc_data->blacklist));
	rc = db_build_sql_stmt(&binsert, 5, &data, colum_ptr);
	//C6
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved0, sizeof(pc_data->reserved0));
	//rc = db_build_sql_stmt(&binsert, 6, &data, colum_ptr);
	//C7
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->card_type, sizeof(pc_data->card_type));
	rc = db_build_sql_stmt(&binsert, 7, &data, colum_ptr);
	//C8
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved1, sizeof(pc_data->reserved1));
	//rc = db_build_sql_stmt(&binsert, 8, &data, colum_ptr);
	//C9
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved2, sizeof(pc_data->reserved2));
	//rc = db_build_sql_stmt(&binsert, 9, &data, colum_ptr);
#else


	db_sql_init(db, &binsert, DINSERT, lg_tbl_ncard_inf);
	
	binsert.columns = (sql_column_t*)lg_st_ncard_inf;
	//rc = gl10_limit_data_store(&binsert, MAX_OF_CARD_UNLK);
	
	data.data = (uint8_t*)&src_data;
	data.len = sizeof(int);

    src_data = pc_data->dev_no;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_st_ncard_inf);

	/* dev_no */
	src_data = pc_data->cardnumber;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_st_ncard_inf);
	
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->cardnumber, sizeof(pc_data->cardnumber));
	//rc = db_build_sql_stmt(&binsert, 2, &data, colum_ptr);
	
	/* card no */
	src_data = pc_data->init_date;	
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_st_ncard_inf);
    /* validity */
	src_data = pc_data->exprirydate;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_st_ncard_inf);
	/*blacklist*/
	src_data = pc_data->blacklist;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_st_ncard_inf);
	src_data = pc_data->card_type;
	col_no++;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_st_ncard_inf);

	
#endif

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}

/**********************************************************************************
*Function name	: INSERT_NDOOR_CARD_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_ndoor_card_data(card_data_t *pc_data)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;


	if(assert_ptr(pc_data)) {
		return -1;
	}
	sqlite3 *db = lg_db_handle;

	//dbg_inf("insert door cards list\n");

	rc = db_build_sql_init(db, &binsert, DINSERT, tbl_ptr, colum_ptr);
	
	rc = db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	if(rc < 0) {
		dbg_err("fill blob");
		return rc;
	}
	
	/*
	dbg_blue("[%0X],[%llX]\n", pc_data->dev_no, pc_data->cardnumber);	
	dbg_blue("init_date\n");
	print_n_byte(pc_data->init_date, 6);
	dbg_blue("exprirydate\n");
	print_n_byte(pc_data->exprirydate, 6);
	*/
#if 0	
	//C1
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->dev_no, sizeof(pc_data->dev_no));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C2
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->cardnumber, sizeof(pc_data->cardnumber));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C3
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->init_date, sizeof(pc_data->init_date));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C4
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->exprirydate, sizeof(pc_data->exprirydate));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C5
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->blacklist, sizeof(pc_data->blacklist));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C6
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved0, sizeof(pc_data->reserved0));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C7
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->card_type, sizeof(pc_data->card_type));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C8
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved1, sizeof(pc_data->reserved1));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C9
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved2, sizeof(pc_data->reserved2));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
#else

	printf("&&&&&&&&&&&&&&&&&&&\n");
	print_n_byte(pc_data, 40);
	printf("%d,%d,%d\n",sizeof(card_data_t),sizeof(uint64_t),sizeof(uint32_t));
	printf("&&&&&&&&&&&&&&&&&&&\n");
	

#if 0
	 card_data_t *pc_data1=alloca(30);    
	 memcpy(&(pc_data1->dev_no),   &pc_data[0], 4); 
     memcpy(&(pc_data1->cardnumber),  &pc_data[4], 8);
     memcpy(&(pc_data1->init_date), &pc_data[12], 6);
     memcpy(&(pc_data1->exprirydate), &pc_data[16], 6);
     memcpy(&(pc_data1->blacklist), &pc_data[22], 1);
     memcpy(&(pc_data1->card_type), &pc_data[24], 1);    
#endif
     

	dbg_blue("dev_no=[%0X],cardnumber=[%llX]=[%llu]\n", pc_data->dev_no, pc_data->cardnumber, pc_data->cardnumber);
	print_n_byte((const uint8_t * const) (&pc_data->dev_no), 4);
	print_n_byte((const uint8_t * const) (&pc_data->cardnumber), 8);
	dbg_blue("blacklist=[%0X],card_type=%X[%c]\n", pc_data->blacklist, pc_data->card_type, pc_data->card_type);
	dbg_blue("init_date:\n");
	print_n_byte(pc_data->init_date, 6);
	dbg_blue("exprirydate:\n");
	print_n_byte(pc_data->exprirydate, 6);

	//C1
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->dev_no, sizeof(pc_data->dev_no));
	rc = db_build_sql_stmt(&binsert, 1, &data, colum_ptr);
	//C2
	//pc_data->cardnumber += random()%100;
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->cardnumber, sizeof(pc_data->cardnumber));
	rc = db_build_sql_stmt(&binsert, 2, &data, colum_ptr);
	//C3
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->init_date, sizeof(pc_data->init_date));
	rc = db_build_sql_stmt(&binsert, 3, &data, colum_ptr);
	//C4
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->exprirydate, sizeof(pc_data->exprirydate));
	rc = db_build_sql_stmt(&binsert, 4, &data, colum_ptr);
	//C5
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->blacklist, sizeof(pc_data->blacklist));
	rc = db_build_sql_stmt(&binsert, 5, &data, colum_ptr);
	//C6
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved0, sizeof(pc_data->reserved0));
	//rc = db_build_sql_stmt(&binsert, 6, &data, colum_ptr);
	//C7
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->card_type, sizeof(pc_data->card_type));
	rc = db_build_sql_stmt(&binsert, 7, &data, colum_ptr);
	//C8
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved1, sizeof(pc_data->reserved1));
	//rc = db_build_sql_stmt(&binsert, 8, &data, colum_ptr);
	//C9
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved2, sizeof(pc_data->reserved2));
	//rc = db_build_sql_stmt(&binsert, 9, &data, colum_ptr);

#endif
	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}

/**********************************************************************************
*Function name	: QUERY_NCARD_LIST_CB
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_ncard_list_cb(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;
	
	//dbg_inf("query door cards list call back\n");
	if(assert_ptr(dest) || assert_ptr(state)) {
		return -1;
	}
#ifdef QUERY_DATA_TYPE
#undef QUERY_DATA_TYPE
#endif
#define	QUERY_DATA_TYPE			card_list_sql_t
	sql_cmd_t *qdata = (sql_cmd_t *)dest;
	QUERY_DATA_TYPE *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	int *pdata = NULL;

	int i = 0;
	int ctype = 0, clm_cnt = 0;

	head = list = (QUERY_DATA_TYPE*)do_malloc(sizeof(QUERY_DATA_TYPE));
	
	if(assert_ptr(list)) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		bzero(list, sizeof(list));
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
		priv = list;
		list = (QUERY_DATA_TYPE*)do_malloc(sizeof(QUERY_DATA_TYPE));
		if(assert_ptr(list)) {
			count = -1;
			break;
		} else {
			bzero(list, sizeof(list));
			list->next = NULL;
			list->last = priv;
			priv->next = list;
		}
		clm_cnt = sqlite3_column_count(state);//用来获取查询结果集中的信息
		for(i=0,pdata=(int *)list; pdata && i<clm_cnt; i++) {
			ctype = sqlite3_column_type(state, i);
			//dbg_lo("ctype=%d, i=%d", ctype, i);
			if(2 == ctype) {
				list->data.cardnumber =	sqlite3_column_int64(state, i);
				//dbg_lo("list->number=%llu \t[%llX]\n", list->number,	list->number);
			} else if (SQLITE_TEXT == ctype) {
				uint8_t *text = NULL;
				
				text = (uint8_t*)sqlite3_column_text(state, i);
				rc = sqlite3_column_bytes(state, i);
				
				switch(i) {
				case 2:
					memcpy((uint8_t*)&list->data.cardnumber, text, rc);
					break;
					
				case 3:
					memcpy((uint8_t*)&list->data.init_date, text, rc);
					break;
				
				case 4:
					memcpy((uint8_t*)&list->data.exprirydate, text, rc);
					pdata = (int*)&list->data.blacklist;
					break;
					
				default:
					break;
				}
				//dbg_lo("rc=%d, number=%llu\n", rc, list->data.cardnumber);
			} else {
				switch(i) {
					case 0:
						pdata = (int *)&list->mkey;
						rc = db_column_data(state, i, (int*)pdata); 
						break;
						
					case 1:
						pdata = (int *)&list->data;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 5:
						pdata = (int *)&list->data.blacklist;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 6:
						pdata = (int *)&list->data.reserved0;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 7:
						pdata = (int *)&list->data.card_type;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 8:
						pdata = (int *)&list->data.reserved1;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 9:
						pdata = (int *)&list->data.reserved2;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
				}
			}
			if(rc < SQLITE_OK) {
				debug_err("db column data\n");
				free_query_list(head, sizeof(QUERY_DATA_TYPE));
				return -1;
			}
		}
		count ++;
		rc = db_do_step(state);
	}while(SQLITE_ROW == rc);


	if(count > 0) {
		qdata->dest = (uint8_t*)head->next;
		list = head->next;		
	} else{
		dest = NULL;
	}
	free(head);
	
	return count;
}



/**********************************************************************************
*Function name	: QUERY_NDOOR_CARD_LIST
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_ndoor_card_list(card_list_sql_t *dst, int opt)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	sqlite3 *db = lg_db_handle;
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;

	if(assert_ptr(dst)) {
		return -1;
	}
	
	rc = db_build_sql_init(db, &bquery, DQUERY, tbl_ptr, colum_ptr);

	bquery.deal_fun = query_ncard_list_cb;
	bquery.condition = EQUALTO;

	if(0 == opt) {
		uint64_t reverse_number = 0;
		ntohll(dst->data.cardnumber, &reverse_number);
		dbg_lo("dst->number=%llu [%llX], reverse_number=%llu\n", dst->data.cardnumber, dst->data.cardnumber, reverse_number);
		db_fill_blob(&data, (uint8_t*)&dst->data.cardnumber, sizeof(dst->data.cardnumber));
		col_no = 2;
	} else if(1 == opt) {
		db_fill_blob(&data, (uint8_t*)&dst->data.dev_no, sizeof(dst->data.dev_no));
	} else {
		db_fill_blob(&data, (uint8_t*)&dst->data.dev_no, sizeof(dst->data.dev_no));
		bquery.condition = CNDTN_NULL;
	}

	rc = db_build_sql_stmt(&bquery, col_no, &data, colum_ptr);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	if(rc > 0) {
		dst->next = (card_list_sql_t *)bquery.dest;;
	} else {
		debug_warn("query, no data\n");
		dst->next = NULL;
	}
	
	return rc;
	
}


/**********************************************************************************
*Function name	: MANAGE_DOOR_CARD_INDEX
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int rebuild_ndoor_card_index(int opt)
{
	int rc = 0;
	sqlite3 * db = lg_db_handle;
	sql_cmd_t ccmd = {0,};
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;
	
	rc = db_build_sql_init(db, &ccmd, DQUERY, tbl_ptr, colum_ptr);
	
	{
		rc = db_drop_index(&ccmd);
		ccmd.fd_set = Bit2;
		rc = db_create_index(&ccmd);
	}

	return rc;
}

/**********************************************************************************
*Function name	: DELETE_NDOOR_UNIT_TABLE
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_ndoor_unit_table(void)
{
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;

	return delete_tbl_data(tbl_ptr, colum_ptr);	
}
int delete_sfz8or4_cardreader_list(void)
{
       printf("delete the card list!!!\n");
	return	delete_tbl_data_card(tbl_dcard_inf, lg_dcard_inf);
}


/**********************************************************************************
*Function name	: MAN_DOOR_UNIT_TBL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int man_ndoor_unit_tbl(int opt)
{
	sqlite3 *db = lg_db_handle;
	int rc = 0;	
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;

	if(FALSE == opt) {
		rc = db_drop_table(db, tbl_ptr);
	}
	rc = db_create_tbl(db, tbl_ptr, colum_ptr, FALSE);

	return rc;
}

#endif



#ifndef DOOR_ACCESS_CARD_LIST
static const char lg_tbl_ncard_inf[] = {"door_card_inf"};
/* ????*/
static const sql_column_t lg_st_ncard_inf[] = {
	{"ID",			SQLIPKEY},
	{"dev_no",		SQLINT},
	{"cardnumber", 	SQLTEXT},
	{"init_date", 	SQLTEXT},
	{"exprirydate", SQLTEXT},
	{"blacklist",	SQLINT},
	{"reserved0",	SQLINT},
	{"card_type",	SQLINT},
	{"reserved1",	SQLINT},
	{"reserved2",	SQLINT},
	{NULL,	NULL},
};
/*
typedef struct DATA_TBL_CARD{
	uint8_t dev_no[4];		//??????
	uint8_t cardnumber[8];	//??,??8??,??4???[0~3]??0
	uint8_t init_date[6];	//????,[0~6]??:???????????
	uint8_t exprirydate[6]; //????,[0~6]??:???????????
	uint8_t blacklist;		//???????,0:???,??:???
	uint8_t reserved0;		//??,??0
	uint8_t card_type;		//??,?????,???2??????
	uint8_t reserved1;		//??
	uint32_t reserved2; 	//??
}card_data_t;

*/


/**********************************************************************************
*Function name	: DO_MALLOC
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void *do_malloc(size_t size)
{
	void *ptr = NULL;
	
	ptr = malloc(size);
//	dbg_inf("++++++++++++++++++++[%08X]\n", (int)ptr);

	return ptr;
}


/**********************************************************************************
*Function name	: CREATE_DOOR_CARD_TBL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int create_door_card_tbl(sqlite3 *db)
{
	if(assert_ptr(db)) {
		return -1;
	}
		
	return db_create_tbl(db, lg_tbl_ncard_inf, lg_st_ncard_inf, FALSE);
}

/**********************************************************************************
*Function name	: INSERT_NDOOR_CARD_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_ndoor_card_data(card_data_t *pc_data)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;
	
	if(assert_ptr(pc_data)) {
		return -1;
	}
	sqlite3 *db = lg_db_handle;

	dbg_inf("insert door cards list\n");

	rc = db_build_sql_init(db, &binsert, DINSERT, tbl_ptr, colum_ptr);
	
	rc = db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	if(rc < 0) {
		dbg_err("fill blob");
		return rc;
	}

	/*
	dbg_blue("[%0X],[%llX]\n", pc_data->dev_no, pc_data->cardnumber);	
	dbg_blue("init_date\n");
	print_n_byte(pc_data->init_date, 6);
	dbg_blue("exprirydate\n");
	print_n_byte(pc_data->exprirydate, 6);
	*/
#if 0	
	//C1
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->dev_no, sizeof(pc_data->dev_no));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C2
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->cardnumber, sizeof(pc_data->cardnumber));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C3
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->init_date, sizeof(pc_data->init_date));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C4
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->exprirydate, sizeof(pc_data->exprirydate));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C5
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->blacklist, sizeof(pc_data->blacklist));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C6
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved0, sizeof(pc_data->reserved0));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C7
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->card_type, sizeof(pc_data->card_type));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C8
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved1, sizeof(pc_data->reserved1));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
	//C9
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved2, sizeof(pc_data->reserved2));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, colum_ptr);
#else


	printf("&&&&&&&&&&&&&&&&&&&\n");
	print_n_byte(pc_data, 40);
	printf("%d,%d,%d,%d\n",sizeof(card_data_t),sizeof(uint64_t),sizeof(uint32_t),sizeof(uint8_t));
	printf("&&&&&&&&&&&&&&&&&&&\n");
		
	dbg_blue("dev_no=[%0X],cardnumber=[%llX]=[%llu]\n", pc_data->dev_no, pc_data->cardnumber, pc_data->cardnumber);
	print_n_byte((const uint8_t * const) (&pc_data->dev_no), 4);
	print_n_byte((const uint8_t * const) (&pc_data->cardnumber), 8);
	dbg_blue("blacklist=[%0X],card_type=[%X]=[%c]\n", pc_data->blacklist, pc_data->card_type, pc_data->card_type);
	dbg_blue("init_date:\n");
	print_n_byte(pc_data->init_date, 6);
	dbg_blue("exprirydate:\n");
	print_n_byte(pc_data->exprirydate, 6);

	dbg_blue("init_date=[%0X],exprirydate=[%0X]\n", pc_data->init_date, pc_data->exprirydate);


	//C1
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->dev_no, sizeof(pc_data->dev_no));
	rc = db_build_sql_stmt(&binsert, 1, &data, colum_ptr);
	//C2
	//pc_data->cardnumber += random()%100;
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->cardnumber, sizeof(pc_data->cardnumber));
	rc = db_build_sql_stmt(&binsert, 2, &data, colum_ptr);
	//C3
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->init_date, sizeof(pc_data->init_date));
	rc = db_build_sql_stmt(&binsert, 3, &data, colum_ptr);
	//C4
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->exprirydate, sizeof(pc_data->exprirydate));
	rc = db_build_sql_stmt(&binsert, 4, &data, colum_ptr);
	//C5
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->blacklist, sizeof(pc_data->blacklist));
	rc = db_build_sql_stmt(&binsert, 5, &data, colum_ptr);
	//C6
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved0, sizeof(pc_data->reserved0));
	//rc = db_build_sql_stmt(&binsert, 6, &data, colum_ptr);
	//C7
	rc = db_fill_blob(&data, (uint8_t*)&pc_data->card_type, sizeof(pc_data->card_type));
	rc = db_build_sql_stmt(&binsert, 7, &data, colum_ptr);
	//C8
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved1, sizeof(pc_data->reserved1));
	//rc = db_build_sql_stmt(&binsert, 8, &data, colum_ptr);
	//C9
	//rc = db_fill_blob(&data, (uint8_t*)&pc_data->reserved2, sizeof(pc_data->reserved2));
	//rc = db_build_sql_stmt(&binsert, 9, &data, colum_ptr);

#endif
	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}

/**********************************************************************************
*Function name	: QUERY_NCARD_LIST_CB
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_ncard_list_cb(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;
	
	//dbg_inf("query door cards list call back\n");
	if(assert_ptr(dest) || assert_ptr(state)) {
		return -1;
	}
#ifdef QUERY_DATA_TYPE
#undef QUERY_DATA_TYPE
#endif
#define	QUERY_DATA_TYPE card_list_sql_t
	sql_cmd_t *qdata = (sql_cmd_t *)dest;
	QUERY_DATA_TYPE *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	int *pdata = NULL;

	int i = 0;
	int ctype = 0, clm_cnt = 0;

	head = list = (QUERY_DATA_TYPE*)do_malloc(sizeof(QUERY_DATA_TYPE));
	
	if(assert_ptr(list)) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		bzero(list, sizeof(list));
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
		priv = list;
		list = (QUERY_DATA_TYPE*)do_malloc(sizeof(QUERY_DATA_TYPE));
		if(assert_ptr(list)) {
			count = -1;
			break;
		} else {
			bzero(list, sizeof(list));
			list->next = NULL;
			list->last = priv;
			priv->next = list;
		}
		clm_cnt = sqlite3_column_count(state);
		for(i=0,pdata=(int *)list; pdata && i<clm_cnt; i++) {
			ctype = sqlite3_column_type(state, i);
			//dbg_lo("ctype=%d, i=%d", ctype, i);
			if(2 == ctype) {
				list->data.cardnumber =	sqlite3_column_int64(state, i);
				//dbg_lo("list->number=%llu \t[%llX]\n", list->number,	list->number);
			} else if (SQLITE_TEXT == ctype) {
				uint8_t *text = NULL;
				
				text = (uint8_t*)sqlite3_column_text(state, i);
				rc = sqlite3_column_bytes(state, i);
				
				switch(i) {
				case 2:
					memcpy((uint8_t*)&list->data.cardnumber, text, rc);
					break;
					
				case 3:
					memcpy((uint8_t*)&list->data.init_date, text, rc);
					break;
				
				case 4:
					memcpy((uint8_t*)&list->data.exprirydate, text, rc);
					pdata = (int*)&list->data.blacklist;
					break;
					
				default:
					break;
				}
				//dbg_lo("rc=%d, number=%llu\n", rc, list->data.cardnumber);
			} else {
				switch(i) {
					case 0:
						pdata = (int *)&list->mkey;
						rc = db_column_data(state, i, (int*)pdata); 
						break;
						
					case 1:
						pdata = (int *)&list->data;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 5:
						pdata = (int *)&list->data.blacklist;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 6:
						pdata = (int *)&list->data.reserved0;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 7:
						pdata = (int *)&list->data.card_type;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 8:
						pdata = (int *)&list->data.reserved1;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
					case 9:
						pdata = (int *)&list->data.reserved2;
						rc = db_column_data(state, i, (int*)pdata); 
					break;
				}
			}
			if(rc < SQLITE_OK) {
				debug_err("db column data\n");
				free_query_list(head, sizeof(QUERY_DATA_TYPE));
				return -1;
			}
		}
		count ++;
		rc = db_do_step(state);
	}while(SQLITE_ROW == rc);


	if(count > 0) {
		qdata->dest = (uint8_t*)head->next;
		list = head->next;		
	} else{
		dest = NULL;
	}
	free(head);
	
	return count;
}



/**********************************************************************************
*Function name	: QUERY_NDOOR_CARD_LIST
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_ndoor_card_list(card_list_sql_t *dst, int opt)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	sqlite3 *db = lg_db_handle;
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;

	if(assert_ptr(dst)) {
		return -1;
	}
	
	rc = db_build_sql_init(db, &bquery, DQUERY, tbl_ptr, colum_ptr);

	bquery.deal_fun = query_ncard_list_cb;
	bquery.condition = EQUALTO;

	if(0 == opt) {
		uint64_t reverse_number = 0;
		ntohll(dst->data.cardnumber, &reverse_number);
		dbg_lo("dst->number=%llu [%llX], reverse_number=%llu\n", dst->data.cardnumber, dst->data.cardnumber, reverse_number);
		db_fill_blob(&data, (uint8_t*)&dst->data.cardnumber, sizeof(dst->data.cardnumber));
		col_no = 2;
	} else if(1 == opt) {
		db_fill_blob(&data, (uint8_t*)&dst->data.dev_no, sizeof(dst->data.dev_no));
	} else {
		db_fill_blob(&data, (uint8_t*)&dst->data.dev_no, sizeof(dst->data.dev_no));
		bquery.condition = CNDTN_NULL;
	}

	rc = db_build_sql_stmt(&bquery, col_no, &data, colum_ptr);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	if(rc > 0) {
		dst->next = (card_list_sql_t *)bquery.dest;;
	} else {
		debug_warn("query, no data\n");
		dst->next = NULL;
	}
	
	return rc;
	
}


/**********************************************************************************
*Function name	: MANAGE_DOOR_CARD_INDEX
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int rebuild_ndoor_card_index(int opt)
{
	int rc = 0;
	sqlite3 * db = lg_db_handle;
	sql_cmd_t ccmd = {0,};
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;
	
	rc = db_build_sql_init(db, &ccmd, DQUERY, tbl_ptr, colum_ptr);
	
	{
		rc = db_drop_index(&ccmd);
		ccmd.fd_set = Bit2;
		rc = db_create_index(&ccmd);
	}

	return rc;
}

/**********************************************************************************
*Function name	: DELETE_NDOOR_UNIT_TABLE
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_ndoor_unit_table(void)
{
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;

	return delete_tbl_data(tbl_ptr, colum_ptr);	
}

/**********************************************************************************
*Function name	: MAN_DOOR_UNIT_TBL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int man_ndoor_unit_tbl(int opt)
{
	sqlite3 *db = lg_db_handle;
	int rc = 0;	
	const sql_column_t *colum_ptr = lg_st_ncard_inf;
	const char *tbl_ptr = lg_tbl_ncard_inf;

	if(FALSE == opt) {
		rc = db_drop_table(db, tbl_ptr);
	}
	rc = db_create_tbl(db, tbl_ptr, colum_ptr, FALSE);

	return rc;
}

#endif


/**********************************************************************************
*Function name	: INSERT_ALARM_INFO
*Description		: 
*Input para		: dev_no: device no
				: type :refer to enum alarm_type_t
				: status: valid while (type = ALARM_ONOFF) only
*Output para		: DB_OK if execute ok, or less than 0.
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_alarm_info(const int dev_no, alarm_type_t type, int status)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;


	sqlite3 *db = lg_db_handle;

	//debug_info("insert alarm info\n");

	db_sql_init(db, &binsert, DINSERT, tbl_alarm);
	binsert.columns = (sql_column_t*)alarm_info;
	//rc = gl10_limit_data_store(&binsert, MAX_OF_ALARM_INFO);
	
	data.data = (uint8_t*)&src_data;
	data.len = sizeof(int);

	if(ALARM_ONOFF == type) {
		//delete_alarm_info((dev_no),ALARM_ONOFF);
	}
	/* dev_no */
	src_data = dev_no;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, alarm_info);
	/* type */
	src_data = type;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, alarm_info);
	/* status */
	src_data = status;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, alarm_info);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}

	return rc;
}
/**********************************************************************************
*Function name	: INSERT_CARD_UNLOCK_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_card_unlock_info(int id, int dev_no, int card_no, int validity)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 0;
	int src_data = 0;


	sqlite3 *db = lg_db_handle;

	//debug_info("insert card unlock_ info\n");

	db_sql_init(db, &binsert, DINSERT, tbl_card_unlock);
	
	binsert.columns = (sql_column_t*)card_unlock;
	//rc = gl10_limit_data_store(&binsert, MAX_OF_CARD_UNLK);
	
	data.data = (uint8_t*)&src_data;
	data.len = sizeof(int);

    src_data = id;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock);

	/* dev_no */
	src_data = dev_no;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock);
	/* card no */
	src_data = card_no;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock);
    /* validity */
	src_data = validity;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}

/**********************************************************************************
*Function name	: INSERT_CARD_UNLOCK_INFO_EX
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_card_unlock_info_ex(int id, int dev_no, int card_no, int validity,int blacklist)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 0;
	int src_data = 0;


	sqlite3 *db = lg_db_handle;

	//debug_info("insert card unlock_ info\n");

	db_sql_init(db, &binsert, DINSERT, tbl_card_unlock_ex);
	
	binsert.columns = (sql_column_t*)card_unlock_ex;
	//rc = gl10_limit_data_store(&binsert, MAX_OF_CARD_UNLK);
	
	data.data = (uint8_t*)&src_data;
	data.len = sizeof(int);

    src_data = id;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock_ex);

	/* dev_no */
	src_data = dev_no;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock_ex);
	/* card no */
	src_data = card_no;	
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock_ex);
    /* validity */
	src_data = validity;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock_ex);
	/*blacklist*/
	src_data = blacklist;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock_ex);
	src_data = 0;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, card_unlock_ex);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}

/**********************************************************************************
*Function name	: INSERT_CALL_HISTORY
*Description		: 
*Input para		: dev_no:call dest device no;
				: call_type: 	0-missed call
				:			1-called in
				:			2-calling out
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_call_history(int dev_no, history_type_t call_type)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;


	sqlite3 *db = lg_db_handle;

	//debug_info("insert call history\n");

	db_sql_init(db, &binsert, DINSERT, tbl_call_history);
	
	binsert.columns = (sql_column_t*)call_history;
	//rc = gl10_limit_data_store(&binsert, MAX_OF_CALL_HSTRY);

	data.data = (uint8_t*)&src_data;
	data.len = sizeof(int);

	/* dev_no */
	src_data = dev_no;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, call_history);
	/* call_type */
	src_data = call_type;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, call_history);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}

/**********************************************************************************
*Function name	: INSERT_CONTACTS
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_contacts(pack_contact_t *pc_data)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;

	if(NULL == pc_data) {
		debug_err("null input\n");
		return -1;
	}
	sqlite3 *db = lg_db_handle;

	//debug_info("insert contacts\n");

	db_sql_init(db, &binsert, DINSERT, tbl_contact);
	
	data.data = (uint8_t*)&src_data;
	data.len = sizeof(int);
	//db_fill_blob(&data, (uint8_t*)&src_data, sizeof(int));
	/* uid */
	src_data = pc_data->uid;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, contact);
	/* grade */	
	src_data = pc_data->grade;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, contact);
	/* pid */
	src_data = pc_data->pid;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, contact);
	/* dev name */
	db_fill_blob(&data, (uint8_t*)pc_data->dev_name, sizeof(pc_data->dev_name));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, contact);
	/* dev no */
	src_data = pc_data->dev_no;
	data.data = (uint8_t *)&src_data;
	data.len = sizeof(pc_data->dev_no);
	rc = db_build_sql_stmt(&binsert, col_no++, &data, contact);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}



/**********************************************************************************
*Function name	: insert_ip_camers
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_ip_camers(ip_camera_inf_t *pc_data)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;

	if(NULL == pc_data) {
		debug_err("null input\n");
		return -1;
	}
	sqlite3 *db = lg_db_handle;

	debug_info("insert ip camers\n");

	db_sql_init(db, &binsert, DINSERT, tbl_ip_camera);
	
	data.data = (uint8_t*)&src_data; 
	data.len = sizeof(int);

	/* u32 */
	src_data = pc_data->dev_ip;
	//col_no = 0;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_ip_cameras);
	/* u16 */	
	src_data = pc_data->port;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_ip_cameras);
	/* u16 */
	src_data = pc_data->channel;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_ip_cameras);
	/* dev name */
	db_fill_blob(&data, (uint8_t*)pc_data->dev_name, sizeof(pc_data->dev_name));
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_ip_cameras);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}




/**********************************************************************************
*Function name	: INSERT_VLAN_LIST_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_vlan_list_data(pkd_vlan_list_t *pc_data)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;
	
	if(assert_ptr(pc_data)) {
		return -1;
	}
	sqlite3 *db = lg_db_handle;

	//dbg_inf("insert vlan list\n");

	rc = db_build_sql_init(db, &binsert, DINSERT, tbl_vlan_list, lg_vlan_list);
	
	/* c1 dev no */
	src_data = pc_data->dev_no;
	rc = db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	if(rc < 0) {
		dbg_err("fill blob");
		return rc;
	}
	
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_vlan_list);
	
	src_data = pc_data->reserved1;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_vlan_list);
	
	src_data = pc_data->reserved2;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_vlan_list);
	
	src_data = pc_data->ipaddr;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_vlan_list);
	
	src_data = pc_data->reserved3;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_vlan_list);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
	
}


/**********************************************************************************
*Function name	: FREE_ALARM_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int free_alarm_data(alarm_data_t *const fptr)
{
	if(NULL == fptr) {
		debug_err("null input\n");
		return -1;
	}

	alarm_data_t *cur = fptr;

	for(; cur!=NULL; ) {
		//debug_info("free,cur=%08X\n", (int)cur);
		free(cur);
		cur=cur->next;
	}

	return 0;
}

/**********************************************************************************
*Function name	: FREE_CONTACTS_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int free_contacts_data(contact_t *const fptr)
{
	contact_t *cur = fptr;
	
	if(NULL == fptr) {
		debug_err("null input\n");
		return -1;
	}

	for(; cur!=NULL; ) {
		free(cur);
		cur=cur->next;
	}

	return 0;
}

/**********************************************************************************
*Function name	: FREE_QUERY_LIST
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int free_query_list(void * fptr, const int size)
{	
	uint8_t *tmp = NULL;
	
	if(assert_ptr(fptr)) {
		return -1;
	}

	for(; fptr!=NULL; ) {
		tmp = (uint8_t*)fptr + size - 8;
		/*debug_info("free,fptr=%08X,*tmp=%08X\n", (int)fptr, *(int*)tmp);*/
		free(fptr);
		fptr = (int*)(*(int*)tmp);
	}

	return 0;
}

/**********************************************************************************
*Function name	: query_alarm_callback
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_alarm_callback(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;
	
	if(assert_ptr(dest) || assert_ptr(state)) {
		return -1;
	} else {
		//debug_info("test query alarm call back\n");
	}

	sql_cmd_t *alarm_dest = (sql_cmd_t *)dest;
	alarm_data_t *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	int *pdata = NULL;

	int i = 0, j = 0;

	head = list = (alarm_data_t*)malloc(sizeof(alarm_data_t));
	
	if(NULL == list) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		//debug_info("head=list=%08X\n", (int)head);
		bzero(list->date_time, sizeof(list->date_time));
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
		priv = list;
		list = (alarm_data_t*)malloc(sizeof(alarm_data_t));
		if(NULL == list) {
			count = -1;
		} else {
			bzero(list->date_time, sizeof(list->date_time));
			list->next = NULL;
			list->last = priv;
			priv->next = list;
			//debug_info("list=%08X,list->last=%08X,priv->next=%08X\n", (int)list,(int)list->last,(int)priv->next);
		}

		for(i=0,j=-1,pdata=(int*)list; i<7; i++,pdata++) {
			rc = db_column_data(state, i, pdata);
			if(rc < SQLITE_OK) {
				debug_err("db column data\n");
				return -1;
			}
		}
		
		count ++;
		rc = db_do_step(state);
	}while(SQLITE_ROW == rc);


	if(count > 0) {
		alarm_dest->dest = (uint8_t*)head->next;
		list = head->next;		
	} else{
		dest = NULL;
		free_query_list(head->next, sizeof(alarm_data_t));
	}
	free(head);
	
	return count;
}

/**********************************************************************************
*Function name	: QUERY_CONTACTS_CALLBACK
*Description		: ?????????
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_contacts_callback(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;
	debug_info("test query contacts call back\n");
	if(assert_ptr(dest) || assert_ptr(state)) {
		return -1;
	}

	sql_cmd_t *address_book = (sql_cmd_t *)dest;
	contact_t *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	int *pdata = NULL;

	int i = 0, j = 0;

	head = list = (contact_t*)malloc(sizeof(contact_t));
	
	if(NULL == list) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		bzero(list, sizeof(list));
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
		priv = list;
		list = (contact_t*)malloc(sizeof(contact_t));
		if(NULL == list) {
			count = -1;
		} else {
			bzero(list, sizeof(list));
			list->next = NULL;
			list->last = priv;
			priv->next = list;
		}
		int ctype = 0;
		int clm_cnt = sqlite3_column_count(state);
		for(i=0,j=-1,pdata=(int*)list; pdata && i<clm_cnt; i++) {
			ctype = sqlite3_column_type(state, i);
			switch(ctype) 
			{				
				case SQLITE_TEXT:	
				case SQLITE_BLOB:
					pdata = (int*)list->dev_name;
					break;
				default:
					break;
			}
			rc = db_column_data(state, i, pdata);
			if(rc < SQLITE_OK) {
				debug_err("db column data\n");
				return -1;
			}
			switch(ctype) 
			{				
				case SQLITE_TEXT:
				case SQLITE_BLOB:
					j = sizeof(list->dev_name);
					list->dev_name[(rc<j)?rc:(j-1)] = '\0';
					//dbg_inf("pdata:%s\n", pdata);
					pdata +=  (rc/4);
					pdata += (rc%2);
					break;

				case SQLITE_INTEGER:
				default:
					//dbg_inf("pdata=%d, *pdata=%d\n", pdata, *pdata);
					pdata += sizeof(int)/4;
					break;
			}
		}
		
		count ++;
		rc = db_do_step(state);
	}while(SQLITE_ROW == rc);


	if(count > 0) {
		address_book->dest = (uint8_t*)head->next;
		list = head->next;		
	} else{
		dest = NULL;
	}
	free(head);
	
	return count;
}


/**********************************************************************************
*Function name	: QUERY_CONTACTS_CB
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_info_container_cb(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;
	int ctype = 0, clm_cnt = 0;
	debug_info("test query info container call back\n");
	if(assert_ptr(dest) || assert_ptr(state)) {
		return -1;
	}
#ifdef QUERY_DATA_TYPE
#undef QUERY_DATA_TYPE
#endif
#define	QUERY_DATA_TYPE			controoler_sql_t
	sql_cmd_t *address_book = (sql_cmd_t *)dest;
	QUERY_DATA_TYPE *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	uint16_t *pdata = NULL;

	int i = 0, j = 0;

	head = list = (QUERY_DATA_TYPE*)malloc(sizeof(QUERY_DATA_TYPE));
	
	if(NULL == list) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		bzero(list, sizeof(list));
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
		priv = list;
		list = (QUERY_DATA_TYPE*)malloc(sizeof(QUERY_DATA_TYPE));
		if(NULL == list) {
			count = -1;
		} else {
			bzero(list, sizeof(list));
			list->next = NULL;
			list->last = priv;
			priv->next = list;
		}
		clm_cnt = sqlite3_column_count(state);
		for(i=0,j=-1,pdata=(uint16_t *)list; pdata && i<clm_cnt; i++) {
			rc = db_column_data(state, i, (int*)pdata);
			if(rc < SQLITE_OK) {
				debug_err("db column data\n");
				return -1;
			}
			ctype = sqlite3_column_type(state, i);
			switch(ctype) {
				case SQLITE_FLOAT: 
					pdata += sizeof(double)/4;
					break;
					
				case SQLITE_TEXT:	
				case SQLITE_BLOB:
					pdata +=  (rc/4);
					pdata += (rc%2);
					break;

				case SQLITE_INTEGER:
				default:
					pdata += sizeof(uint16_t)/(2+(i?0:-1));
					break;
			}
		}
		count ++;
		rc = db_do_step(state);
	}while(SQLITE_ROW == rc);


	if(count > 0) {
		address_book->dest = (uint8_t*)head->next;
		list = head->next;		
	} else{
		dest = NULL;
	}
	free(head);
	
	return count;
}


/**********************************************************************************
*Function name	: QUERY_CAMERA_INF_CB
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_camera_inf_cb(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;
	debug_info("test query info container call back\n");
	if(assert_ptr(dest) || assert_ptr(state)) {
		return -1;
	}
#ifdef QUERY_DATA_TYPE
#undef QUERY_DATA_TYPE
#endif
#define	QUERY_DATA_TYPE			ip_camera_inf_t
	sql_cmd_t *address_book = (sql_cmd_t *)dest;
	QUERY_DATA_TYPE *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	uint16_t *pdata = NULL;

	int i = 0, j = 0;

	head = list = (QUERY_DATA_TYPE*)malloc(sizeof(QUERY_DATA_TYPE));
	
	if(NULL == list) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		bzero(list, sizeof(list));
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
		priv = list;
		list = (QUERY_DATA_TYPE*)malloc(sizeof(QUERY_DATA_TYPE));
		if(NULL == list) {
			count = -1;
		} else {
			bzero(list, sizeof(list));
			list->next = NULL;
			list->last = priv;
			priv->next = list;
		}
		int ctype = 0;
		int clm_cnt = 0;
		clm_cnt = sqlite3_column_count(state);
		for(i=0,j=-1,pdata=(uint16_t *)list; pdata && i<clm_cnt; i++) {
			ctype = sqlite3_column_type(state, i);
			switch(ctype) {
				case SQLITE_FLOAT: 
					pdata += sizeof(double)/4;
					break;
					
				case SQLITE_TEXT:	
				case SQLITE_BLOB:
					pdata = (uint16_t *)list + 6;
					break;

				case SQLITE_INTEGER:
				default:
					if(0 == i) {
						pdata = (uint16_t *)list;
					} else {
						pdata += sizeof(uint16_t)/(2+(i>2?0:-1));
					}
					break;
			}
			//debug_info("ctype=%d,i=%d,pdata=%08X\n", ctype, i,(int)pdata);
			rc = db_column_data(state, i, (int*)pdata);
			if(rc < SQLITE_OK) {
				debug_err("db column data\n");
				return -1;
			}
		}
		count ++;
		rc = db_do_step(state);
	}while(SQLITE_ROW == rc);


	if(count > 0) {
		address_book->dest = (uint8_t*)head->next;
		list = head->next;		
	} else{
		dest = NULL;
	}
	free(head);
	
	return count;
}


/**********************************************************************************
*Function name	: QUERY_VLAN_LIST_CB
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_vlan_list_cb(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;
	
	dbg_inf("query vlan list call back\n");
	if(assert_ptr(dest) || assert_ptr(state)) {
		printf("query_vlan_list_cb_quit_-1");
		return -1;
	}
#ifdef QUERY_DATA_TYPE
#undef QUERY_DATA_TYPE
#endif
#define	QUERY_DATA_TYPE			vlan_list_t
	sql_cmd_t *address_book = (sql_cmd_t *)dest;
	QUERY_DATA_TYPE *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	int *pdata = NULL;

	int i = 0;
	int ctype = 0, clm_cnt = 0;

	head = list = (QUERY_DATA_TYPE*)malloc(sizeof(QUERY_DATA_TYPE));
	
	if(assert_ptr(list)) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		bzero(list, sizeof(list));
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
	//	dbg_inf("query vlan list call back----do\n");
		priv = list;
	//	dbg_inf("query vlan list call back----do-----1\n");
		list = (QUERY_DATA_TYPE*)malloc(sizeof(QUERY_DATA_TYPE));
	//	dbg_inf("query vlan list call back----do-----2\n");
		if(assert_ptr(list)) {
			perror("malloc");
			dbg_err("malloc\n");
			count = -1;
			break;			
		} else {
			//bzero(list, sizeof(list));
			list->next = NULL;
			list->last = priv;
			priv->next = list;
		}
		clm_cnt = sqlite3_column_count(state);
		//dbg_inf("clm=%d\n", clm_cnt);
		for(i=0,pdata=(int *)list; pdata && i<(clm_cnt-1); i++) {
			ctype = sqlite3_column_type(state, i);

			//pdata ++;
			
			rc = db_column_data(state, i, (int*)pdata);
			if(rc < SQLITE_OK) {
				debug_err("db column data\n");
				free_query_list(head, sizeof(QUERY_DATA_TYPE));
				return -1;
			}
			//dbg_inf("*pdata = %08X\n", *pdata);
			pdata ++;
		}
		count ++;
	//	dbg_inf("query vlan list call back---under counter++\n");
		rc = db_do_step(state);
	}while(SQLITE_ROW == rc);

	dbg_inf("query vlan list call back---end do");

	if(count > 0) {
		address_book->dest = head->next;		
	} else{
		dest = NULL;
	}
	//free(head);
	
	return count;
}



/**********************************************************************************
*Function name	: DO_QUERY_SQL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int do_query_sql(sqlite3 *db, sql_cmd_t *qcmd)
{
	return db_state_query(db, qcmd);
}


#define	Q_LIST_PAGE_SZ						(9)
/**********************************************************************************
*Function name	: QUERY_ALARM_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_alarm_info(alarm_data_t *const dest, const alarm_type_t type, int offset)
{
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	int src_data[2] = {0, 0};

	sqlite3 *db = lg_db_handle;
	
	debug_info("query alarm info\n");
	bzero(&bquery, sizeof(bquery));
	bquery.cmd = DQUERY;
	bquery.db = db;
	bquery.name = tbl_alarm;
	bquery.sql = NULL;
	bquery.state = state;
	bquery.dest = (uint8_t *)&lg_alarm_info;
	bquery.deal_fun = query_alarm_callback;
	
	bquery.fd_set = Bit1 | Bit2;
	bquery.query.page.size = Q_LIST_PAGE_SZ;
	bquery.query.page.offset = offset;
	bquery.query.qord.col = 0;
	bquery.query.qord.ord = ORDDSC;
		
	src_data[0] = type;
	data.data = (uint8_t*)&src_data[0];
	data.len = sizeof(int);
	col_no = 2;
	switch(type) {
		case ALARM_FQ:
			bquery.condition = BETWEENOF;
			src_data[1] = 0xFF;
			rc = db_build_sql_stmt(&bquery, col_no, &data, alarm_info);
			if(DB_OK != rc) {
				debug_err("query,build sql statement failed\n");
				return -1;
			}
			break;
			
		case ALARM_ZH:
		case ALARM_MC:
		case ALARM_BJ:
		case ALARM_FC:
		case ALARM_ONOFF:
		{
			bquery.condition = EQUALTO;
			rc = db_build_sql_stmt(&bquery, col_no, &data, alarm_info);
			if(DB_OK != rc) {
				debug_err("query,build sql statement failed\n");
				return -1;
			}
		}
		break;

		
		case DEF_ALL:
		default:
			bquery.condition = LESSTHAN;
			src_data[0] = ALARM_BJ;
			rc = db_build_sql_stmt(&bquery, col_no, &data, alarm_info);
			if(DB_OK != rc) {
				debug_err("query,build sql statement failed\n");
				return -1;
			}
			break;
	}
	rc = do_query_sql(db, &bquery);
	
	if(rc > 0) {
		dest->next = (alarm_data_t *)bquery.dest;
	} else {
		dest->next = NULL;
	}

	return rc;
}
/**********************************************************************************
*Function name	: QUERY_CARD_UNLOCK_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_card_unlock_info(card_unlock_t *const dest, int offset)
{
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	card_unlock_t tmp_data = {0, };

	sqlite3 *db = lg_db_handle;
	if(assert_ptr(dest)) {
		return -1;
	}

	
	debug_info("query card unlock info\n");
	db_sql_init(db, &bquery, DQUERY, tbl_card_unlock);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_alarm_callback;

	bquery.condition = CNDTN_NULL;
	bquery.query.page.offset = offset;
	bquery.query.page.size = 0;
	bquery.query.qord.col = 0;
	//bquery.query.qord.ord = ORDDSC;

	rc = db_build_sql_stmt(&bquery, col_no, &data, card_unlock);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	
	if(rc > 0) {
		dest->next = (card_unlock_t *)bquery.dest;
	} else {
		dest->next = NULL;
	}

	return rc;
}

/**********************************************************************************
*Function name	: QUERY_CARD_UNLOCK_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_card_unlock_info_ex(card_unlock_t_ex *const dest, int offset)
{
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	card_unlock_t_ex tmp_data = {0, };

	sqlite3 *db = lg_db_handle;
	if(assert_ptr(dest)) {
		return -1;
	}

	
	debug_info("query card unlock info\n");
	db_sql_init(db, &bquery, DQUERY, tbl_card_unlock_ex);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_alarm_callback;

	bquery.condition = CNDTN_NULL;
	bquery.query.page.offset = offset;
	bquery.query.page.size = 0;
	bquery.query.qord.col = 0;
	//bquery.query.qord.ord = ORDDSC;

	rc = db_build_sql_stmt(&bquery, col_no, &data, card_unlock_ex);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	
	if(rc > 0) {
		dest->next = (card_unlock_t_ex *)bquery.dest;
	} else {
		dest->next = NULL;
	}

	return rc;
}

int is_card_unlock(unsigned int card_no, unsigned char room[])
{
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	card_unlock_t tmp_data = {0, };

	sqlite3 *db = lg_db_handle;
	
	debug_info("query card unlock info:%u\n", card_no);
	db_sql_init(db, &bquery, DQUERY, tbl_card_unlock);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_alarm_callback;

    unsigned int src_data = card_no;
	data.data = (uint8_t*)&src_data;
    data.len  = sizeof(unsigned int);

	bquery.query.condition = EQUALTO;
	col_no = 2;

	rc = db_build_sql_stmt(&bquery, col_no, &data, card_unlock);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	
	if ((rc>0) && (room!=NULL)) {
		unsigned char i, hex[4];
		card_unlock_t *dest = (card_unlock_t *)bquery.dest;
		if (dest->validity != 0xFFFFFFFF) {
			unsigned int now_sec = time(NULL);
			if (now_sec > dest->validity+8*3600)
				return 0;
		}
		memcpy(hex, &dest->dev_no, 4);
		for (i = 0; i < 4; i++) {
			room[i] = ((hex[i]/10)*16) + (hex[i]%10);
		}
		printf("is_card_unlock room:%02X%02X%02X%02X \n", room[0], room[1], room[2], room[3]);
	}
	return rc;
}


int is_real_room_exist(unsigned int card_no,unsigned char room[])
{
	sqlite3_stmt *state = NULL;
	int rc = 1;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	int room_num;
	card_unlock_t_ex tmp_data = {0, };
	room_num = room[0];

	sqlite3 *db = lg_db_handle;	
	
	debug_info("query card unlock info:%u\n", card_no);
	db_sql_init(db, &bquery, DQUERY, tbl_card_unlock_ex);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_alarm_callback;

    unsigned int src_data = card_no;
	data.data = (uint8_t*)&src_data;
    data.len  = sizeof(unsigned int);

	bquery.query.condition = EQUALTO;
	col_no = 1;//添加要查找的选项 1代表第一个参数 2代表查找第二个参数

	rc = db_build_sql_stmt(&bquery, col_no, &data, card_unlock_ex);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -5;
	}
	rc = do_query_sql(db, &bquery);	
	if(rc<0) //找不到卡号
		return -2;
	if ((rc>0) && (room!=NULL)) {
		unsigned char i, hex[4];
		card_unlock_t_ex *dest = (card_unlock_t_ex *)bquery.dest;
		if (dest->validity != 0xFFFFFFFF) {
			unsigned int now_sec = time(NULL);
			if (now_sec > dest->validity+8*3600)
				return -1;
		}
		if(dest->blacklist)
			return -3;

		printf("dest->card_no=%d;dest->card_no=%d\n",dest->card_no,dest->card_no);

		room[2]=dest->card_no/100;
		room[3]=dest->card_no%100;
		//memcpy(hex, &dest->card_no, 4);
		//for (i = 0; i < 4; i++) {
			//room[i] = ((hex[i]/10)*16) + (hex[i]%10);
		//}
		printf("is_card_unlock_ex room:%02X%02X%02X%02X blacklist %d\n", room[0], room[1], room[2], room[3],dest->card_no);
		return 0 ;//可开锁
	}
	return 1;
	
}

int is_room_exist(unsigned int card_no, unsigned char room[])
{
	sqlite3_stmt *state = NULL;
	int rc = 1;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	card_unlock_t_ex tmp_data = {0, };

	sqlite3 *db = lg_db_handle;
	
	printf("query card unlock info:%u\n", card_no);
	debug_info("query card unlock info:%u\n", card_no);
	db_sql_init(db, &bquery, DQUERY, tbl_card_unlock_ex);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_alarm_callback;

    unsigned int src_data = card_no;
	data.data = (uint8_t*)&src_data;
    data.len  = sizeof(unsigned int);

	bquery.query.condition = EQUALTO;
	col_no = 2;

	rc = db_build_sql_stmt(&bquery, col_no, &data, card_unlock_ex);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -5;
	}
	rc = do_query_sql(db, &bquery);	
	if(rc<0) //找不到卡号
		return -2;
	if ((rc>0) && (room!=NULL)) {
		unsigned char i, hex[4];
		card_unlock_t_ex *dest = (card_unlock_t_ex *)bquery.dest;
		if (dest->validity != 0xFFFFFFFF) {
			unsigned int now_sec = time(NULL);
			if (now_sec > dest->validity+8*3600)
				return -1;
		}
		if(dest->blacklist)
			return -3;
		memcpy(hex, &dest->dev_no, 4);
		for (i = 0; i < 4; i++) {
			room[i] = ((hex[i]/10)*16) + (hex[i]%10);
		}
		printf("is_card_unlock_ex room:%02X%02X%02X%02X blacklist %d\n", room[0], room[1], room[2], room[3],dest->blacklist);
		return 0 ;//可开锁
	}
	return 1;
	
}

int is_card_unlock_ex(unsigned int card_no, unsigned char room[])
{
	sqlite3_stmt *state = NULL;
	int rc = 1;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	card_unlock_t_ex tmp_data = {0, };

	sqlite3 *db = lg_db_handle;
	
	debug_info("query card unlock info:%u\n", card_no);
	db_sql_init(db, &bquery, DQUERY, tbl_card_unlock_ex);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_alarm_callback;

    unsigned int src_data = card_no;
	data.data = (uint8_t*)&src_data;
    data.len  = sizeof(unsigned int);

	bquery.query.condition = EQUALTO;
	col_no = 2;

	rc = db_build_sql_stmt(&bquery, col_no, &data, card_unlock_ex);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -5;
	}
	rc = do_query_sql(db, &bquery);	
	if(rc<0) //找不到卡号
		return -2;
	if ((rc>0) && (room!=NULL)) {
		unsigned char i, hex[4];
		card_unlock_t_ex *dest = (card_unlock_t_ex *)bquery.dest;
		if (dest->validity != 0xFFFFFFFF) {
			unsigned int now_sec = time(NULL);
			if (now_sec > dest->validity+8*3600)
				return -1;
		}
		if(dest->blacklist)
			return -3;
		memcpy(hex, &dest->dev_no, 4);
		for (i = 0; i < 4; i++) {
			room[i] = ((hex[i]/10)*16) + (hex[i]%10);
		}
		printf("is_card_unlock_ex room:%02X%02X%02X%02X blacklist %d\n", room[0], room[1], room[2], room[3],dest->blacklist);
		return 0 ;//可开锁
	}
	return 1;
	
}
int delete_card_unlock_ex(int clear, unsigned int card_no)
{
    int rc = 0;
	sqlite3 *db = lg_db_handle;
	sql_cmd_t bdelete = {0, };
	db_blob_t data = {0, };

	debug_info("delete card unlock\n");

	//db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	
	db_sql_init(db, &bdelete, DDELETE, tbl_card_unlock_ex);
	debug_info("bdelete.condition=%X\n", bdelete.condition);

    unsigned int src_data = card_no;
	data.data = (uint8_t*)&src_data;
    data.len  = sizeof(unsigned int);

	bdelete.condition = clear?CNDTN_NULL:EQUALTO;
	rc = db_build_sql_stmt(&bdelete, 0, &data, contact);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}
	
	debug_info("delete,sql=%s\n", bdelete.sql);
	rc = db_exec_state(db, bdelete.state, TRUE);
	if(DB_OK != rc && SQLITE_DONE != rc) {
		debug_err("db exec sql,rc=%d,%s\n", rc, DBERR_MSG(db));
		return rc;
	} else {
		rc = DB_OK;
	}

	return rc;
}
int delete_card_unlock(int clear, unsigned int card_no)
{
    int rc = 0;
	sqlite3 *db = lg_db_handle;
	sql_cmd_t bdelete = {0, };
	db_blob_t data = {0, };

	debug_info("delete card unlock\n");

	//db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	
	db_sql_init(db, &bdelete, DDELETE, tbl_card_unlock);
	debug_info("bdelete.condition=%X\n", bdelete.condition);

    unsigned int src_data = card_no;
	data.data = (uint8_t*)&src_data;
    data.len  = sizeof(unsigned int);

	bdelete.condition = clear?CNDTN_NULL:EQUALTO;
	rc = db_build_sql_stmt(&bdelete, 0, &data, contact);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}
	
	debug_info("delete,sql=%s\n", bdelete.sql);
	rc = db_exec_state(db, bdelete.state, TRUE);
	if(DB_OK != rc && SQLITE_DONE != rc) {
		debug_err("db exec sql,rc=%d,%s\n", rc, DBERR_MSG(db));
		return rc;
	} else {
		rc = DB_OK;
	}

	return rc;
}

/**********************************************************************************
*Function name	: QUERY_CALL_HISTORY_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_call_history_info(call_history_t *const dest, history_type_t type, int offset)
{
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	call_history_t tmp_data = {0, };
	int src_data[2] = {0, 0};

	sqlite3 *db = lg_db_handle;

	if(assert_ptr(dest)) {
		return -1;
	}
	debug_info("query call history info\n");
	db_sql_init(db, &bquery, DQUERY, tbl_call_history);

	data.data = (uint8_t*)&src_data[0];
	data.len = sizeof(int);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_alarm_callback;
	bquery.condition = EQUALTO;
	bquery.fd_set = Bit1;
	src_data[0] = type;
	col_no = 2;
	bquery.query.page.offset = offset;
	bquery.query.page.size = Q_LIST_PAGE_SZ;
	bquery.query.qord.col = 0;
	bquery.query.qord.ord = ORDDSC;

	rc = db_build_sql_stmt(&bquery, col_no, &data, call_history);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	
	if(rc > 0) {
		dest->next = (call_history_t *)bquery.dest;
	} else {
		dest->next = NULL;
	}

	return rc;
}

/**********************************************************************************
*Function name	: QUERY_ADDRESS_BOOK
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_address_book(contact_t *dest, sql_cndtn_t *conditions, const int offset)
{
	if(assert_ptr(dest)) {
		return -1;
	}
	
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	call_history_t tmp_data = {0, };
	sql_cndtn_t cndt = 0;
	int grade = 0, pid = 0;
	sqlite3 *db = lg_db_handle;

	debug_info("query address book\n");
	db_sql_init(db, &bquery, DQUERY, tbl_contact);
	
	bquery.query.page.offset = offset;
	bquery.query.page.size = (Q_LIST_PAGE_SZ + 2);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_contacts_callback;

	if(NULL != conditions) {
		cndt = *(conditions);
		grade = dest->grade;
		pid = dest->pid;
		dbg_inf("pid=%d\n", pid);
		switch(cndt) {
			case BIGGERTHAN:
				col_no = 1;
				bquery.condition = EQUALTO;
				data.data = (uint8_t*)&pid;
				data.len = sizeof(pid);
				break;
				
			case LESSTHAN:
			case EQUALTO:
				bquery.condition = cndt;
				col_no = 3;
				data.data = (uint8_t*)&pid;
				data.len = sizeof(pid);
				break;
				
			case LIKE:
				bquery.query.qord.ord = ORDNULL;
				bquery.query.qord.col = 4;
				bquery.condition = LIKE;
				col_no = 4;
				data.data = (uint8_t*)dest->dev_name;
				data.len = strlen(dest->dev_name);
				break;
				
			default:
				bquery.query.qord.ord = ORDDIS;
				bquery.query.qord.col = 4;
				bquery.condition = EQUALTO;
				col_no = 2;
				data.data = (uint8_t*)&grade;
				data.len = sizeof(grade);
				
				break;
		}
	} else {
		bquery.condition = CNDTN_NULL;
        bquery.query.page.offset = 0;
        bquery.query.page.size   = 0;
	}
	rc = db_build_sql_stmt(&bquery, col_no, &data, contact);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	
	if(rc > 0) {
		dest->next = (contact_t *)bquery.dest;
	} else {
		debug_warn("query no data\n");
		dest->next = NULL;
	}

	return rc;
	
}

/**********************************************************************************
*Function name	: DELETE_ALARM_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_adress_book(const int dev_no)
{
    int rc = 0;
	sqlite3 *db = lg_db_handle;
	sql_cmd_t bdelete = {0, };
	db_blob_t data = {0, };
	int src_data = dev_no;

	debug_info("delete adress book\n");

	db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	
	db_sql_init(db, &bdelete, DDELETE, tbl_contact);
	debug_info("bdelete.condition=%X\n", bdelete.condition);
	bdelete.condition = CNDTN_NULL;
	rc = db_build_sql_stmt(&bdelete, 0, &data, contact);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}
	
	debug_info("delete,sql=%s\n", bdelete.sql);
	rc = db_exec_state(db, bdelete.state, TRUE);
	if(DB_OK != rc && SQLITE_DONE != rc) {
		debug_err("db exec sql,rc=%d,%s\n", rc, DBERR_MSG(db));
		return rc;
	} else {
		rc = DB_OK;
	}

	return rc;
}

/**********************************************************************************
*Function name	: QUERY_INFO_CONTROLLER
*Description		: 
*Input para		: query option, opt=1, by ext_id=dest->data.ext_id
				: opt=0, by id=dest->id
				: opt=2, by id>dest->id
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_info_controller(int opt, controoler_sql_t *dest)
{
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 0;
	int src_data = 0;
	
	sqlite3 *db = lg_db_handle;

	if(assert_ptr(dest) || assert_ptr(db)) {
		return -1;
	}
	debug_info("query info controller\n");
	db_sql_init(db, &bquery, DQUERY, tbl_sub_controller);

	bquery.state = state;
	bquery.dest = (uint8_t *)&src_data;
	bquery.deal_fun = query_info_container_cb;
	data.data = (uint8_t *)&src_data;
	if(0xFF == opt) {
		bquery.condition = CNDTN_NULL;
	} else if(0 == opt){
		src_data = dest->id;
		bquery.condition = EQUALTO;
	}else if (1 == opt) {
		src_data = dest->data.ext_id;
		bquery.condition = EQUALTO;
		col_no = 1;
	} else if (2 == opt) {
		src_data = dest->id;
		bquery.condition = BIGGERTHAN;
	} else {
	}

	rc = db_build_sql_stmt(&bquery, col_no, &data, lg_sub_controller);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	if(rc > 0) {
		dest->next = (controoler_sql_t *)bquery.dest;
	} else {
		debug_warn("query no data\n");
		dest->next = NULL;
		rc = -1;
	}

	return rc;
}


/**********************************************************************************
*Function name	: DISTINGUISH_IP_CAMERAS
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int distinguish_ip_cameras(sqlite3 *db, int col_no, char *sql)
{
	char tmp_sql[100] = {0, }; 
	int rc = 0;
	sql_cmd_t qcmd = {0,};
	
	db_sql_init(db, &qcmd, DQUERY, tbl_ip_camera);
	sprintf(tmp_sql, "%s %s %s", "SELECT", lg_ip_cameras[col_no].name,DISTINCT);
	qcmd.sql = tmp_sql;

	return rc;
}

/**********************************************************************************
*Function name	: QUERY_IP_CAMERA_INF
*Description		: query option:
				: opt=0, by id=dest->id
				: opt=1, by dev_ip=dest->dev_ip
				: opt=2, by dev_name=dest->dev_name
				: opt=0xF0, get how many different "name" in ip camera,and fetch them
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_ip_camera_inf(int opt, ip_camera_inf_t *dest)
{
	if(assert_ptr(dest)) {
		return -1;
	}
	
	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	ip_camera_inf_t tmp_data = {0, };
	int src_data = 0;

	sqlite3 *db = lg_db_handle;

	debug_info("query ip camera infomation\n");
	db_sql_init(db, &bquery, DQUERY, tbl_ip_camera);

	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_camera_inf_cb;
	bquery.condition = CNDTN_NULL;

	db_fill_blob(&data, (uint8_t *)&src_data, sizeof(src_data));
	if(0xFF == opt) {
		bquery.condition = CNDTN_NULL;
	} else if(0 == opt){
		src_data = dest->id;
		bquery.condition = EQUALTO;
	}else if (1 == opt) {
		src_data = dest->dev_ip;
		bquery.condition = EQUALTO;
		col_no = 1;
	} else if (2 == opt) {
		db_fill_blob(&data, (uint8_t*)dest->dev_name, sizeof(dest->dev_name));
		bquery.condition = EQUALTO;
		col_no = 4;
	} else if(0xF0 == opt) {
		bquery.condition = CNDTN_NULL;
		bquery.order = ORDDIS;
		col_no = 4;
	} else {
		debug_err("invalid option\n");
		return -1;
	}

	rc = db_build_sql_stmt(&bquery, col_no, &data, lg_ip_cameras);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	
	if(rc > 0) {		 
		dest->next = (ip_camera_inf_t *)bquery.dest;;
	} else {
		debug_warn("query, no data\n");
		dest->next = NULL;
	}

	return rc;
	
}

/**********************************************************************************
*Function name	: DELETE_ALARM_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_alarm_info(const int dev_no, const alarm_type_t type)
{
    int rc = 0;
	sqlite3 *db = lg_db_handle;
	sql_cmd_t bdelete = {0, };
	db_blob_t data = {0, };
	int src_data = dev_no;


	db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));
	
	db_sql_init(db, &bdelete, DDELETE, tbl_alarm);
	
	rc = db_build_sql_stmt(&bdelete, 1, &data, alarm_info);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}

	src_data = type;
	rc = db_build_sql_stmt(&bdelete, 2, &data, alarm_info);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}

	
	rc = db_exec_state(db, bdelete.state, TRUE);
	if(DB_OK != rc && SQLITE_DONE != rc) {
		debug_err("db exec sql,rc=%d,%s\n", rc, DBERR_MSG(db));
		return rc;
	} else {
		rc = DB_OK;
	}

	return rc;
}



/**********************************************************************************
*Function name	: QUERY_VLAN_LIST
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_vlan_list(vlan_list_t *dst, int opt)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;
	sqlite3 *db = lg_db_handle;

	if(assert_ptr(dst)) {
		return -1;
	}
	
	rc = db_build_sql_init(db, &bquery, DQUERY, tbl_vlan_list, lg_vlan_list);

	bquery.deal_fun = query_vlan_list_cb;
	bquery.condition = EQUALTO;

	if(0 == opt) {
		//db_fill_blob(&data, (uint8_t*)&dst->data.dev_no, sizeof(dst->data.dev_no));
		db_fill_blob(&data, (uint8_t*)&dst->data.dev_no, sizeof(dst->data.dev_no));
	} else if(1 == opt) {
		db_fill_blob(&data, (uint8_t*)&dst->data.ipaddr, sizeof(dst->data.ipaddr));
		col_no = 4;
	} else if(2 == opt) {
		db_fill_blob(&data, (uint8_t*)&dst->data.dev_no, sizeof(dst->data.dev_no));
		bquery.condition = CNDTN_NULL;
	}

	rc = db_build_sql_stmt(&bquery, col_no, &data, lg_vlan_list);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		printf("query,build sql statement failed\n");
		return -1;
	}
	rc = do_query_sql(db, &bquery);
	if(rc > 0) {		 
		dst->next = (vlan_list_t *)bquery.dest;;
	} else {
		debug_warn("query, no data\n");
		printf("query,no data\n");
		dst->next = NULL;
	}
	
	return rc;
	
}

/**********************************************************************************
*Function name	: MANAGE_VLAN_LIST_INDEX
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void manage_vlan_list_index(const int mcmd)
{
	int rc = 0;
	sqlite3 * db = lg_db_handle;
	sql_cmd_t ccmd = {0,};

	rc = db_build_sql_init(db, &ccmd, DQUERY, tbl_vlan_list, lg_vlan_list);

	if(mcmd == 0) {
		rc = db_drop_index(&ccmd);
	} else {
		ccmd.fd_set = Bit1;
		rc = db_create_index(&ccmd);
	}
}



/**********************************************************************************
*Function name	: DO_GET_COUNT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: ?????????豸????
************************************************************************************/
static int do_get_count(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt * state = *ptr_state;
	if(NULL == dest || NULL == ptr_state) {
		debug_err("null input\n");
		return -1;
	} else {
	}

	int rc = 0, count = 0;
	int *dest_no = (int*)dest;
	
	if(NULL == state) {
		debug_err("null input\n");
		return -1;
	} else {
		debug_info("step, rc=%d, fetching data...\n", rc);
		do {
			rc = db_column_data(state, 0, dest_no+count);
			count ++;
			debug_info("data[%d]=%08X\n", count, dest_no[count-1]);
			rc = db_do_step(state);			
		}while(SQLITE_ROW == rc && count <= 0xFF);
	}

	return count;
}

/**********************************************************************************
*Function name	: SELECT_DISTINCT_TEST
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int select_distinct_test(void)
{
	char *sql = NULL;
	sqlite3 *db = lg_db_handle;
	int rc = 0, amount = 0;
	int count = 0;
	sqlite3_stmt *state;
	sql_cmd_t bquery = {0, };
	sql = sqlite3_mprintf("select distinct dev_no from (select * from %q where id > 1)", tbl_alarm);

	debug_info("select distinct test \n");
	debug_info("sql=%s\n", sql);
	db_sql_init(db, &bquery, DQUERY, tbl_alarm);
	bquery.deal_fun = do_get_count;
	bquery.dest = (uint8_t*)amount;

	rc = sqlite3_prepare_v2(db, sql, -1, &state, 0);
	if(SQLITE_OK != rc) {
		debug_err("prepare,%s\n", DBERR_MSG(db));
		debug_err("sql=%s\n", sql);
		rc = -1;
	}
	bquery.state = state;

	//count = db_count_column(db, state);
	count = do_query_sql(db, &bquery);
	return count;
}

/**********************************************************************************
*Function name	: DB_SQL_MPRINTF
*Description		: simulating the function "sqlite3_mprintf ,but free 
				: the memory allocted before by sqlite3_mprintf or this function with the 
				: pointer "sql",so you shoul use like db_sql_mprintf("", sql,), the parameter
				: "sql" is the pointer to be free.
*Input para		: format:the first args should be given;"..." the variabel arguements
				: must follow as old "sql" firstly which allocted memory.
*Output para		: char *,the pointer points  new sql statement.
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static char* db_sql_mprintf_card(char *format, ...)
{
	char *sql = NULL, *fsql = NULL;
	va_list argp;		/* ???屣?溯??????? */	

	/* argp?????????????????
		msg???????????????? */    
	va_start(argp, format);
#if 0
	char *para = NULL; 	/* ?????????????? */	  
	int argno = 0;		/* ?????????? */	  
	while (1)   
    {
		/*	????????????????char *.   */	
		para = va_arg( argp, char *); 
		if(NULL == para) {
			/* ??????????????????? */	
			break;
		}
		dbg_inf("Parameter #%d is: %s\n", argno, para);	  
		argno ++;
		fsql = para;
		break;
	}
#else
	fsql = va_arg(argp, char *);
#endif
	va_start(argp, format);
	sql = sqlite3_vmprintf(format, argp);
	va_end( argp ); /* ??argp???NULL */ 

	if(NULL != fsql) {
		//dbg_inf("db sql mprintf,free, para=%08X,new sql=%08X\n", (int)fsql, (int)sql);
		db_free_sql(fsql);
		fsql = NULL;
	}

	return sql;
}

static const char col_types_card[][10] = { "", SQLINT, SQLFLOAT, SQLTEXT, SQLBLOB, "NULL", SQLDATETIME};

/**********************************************************************************
*Function name	: DB_CREATE_TBL
*Description		: 
*Input para		: time_dflt: ??????????????????
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int db_create_tbl_card(sqlite3 * db,const char *const tabname, const sql_column_t * const column, int time_dflt)
{
	char *sql = NULL;
	int rc = -1;
	

	if(assert_ptr(db) || assert_ptr(tabname) || assert_ptr(column)) {
		return -1;
	}
	dbg_inf("create table %s\n", tabname);

	
	sql = sqlite3_mprintf("create table %q(", tabname);
	int i = 0, semf = 0;
	sql_column_t *tmpcol = (sql_column_t *)column;
	
	for(i=Bit0; tmpcol->name&&tmpcol->type; i<<=1,tmpcol++) {
		if(!memcmp(col_types_card[SQLITE_NULL+1], tmpcol->type, 4)) {
			continue;
		}
		sql = db_sql_mprintf_card("%q%q%q %q", sql,semf?",":"", tmpcol->name, tmpcol->type);
		semf = TRUE;
	}
	if(FALSE != time_dflt) {
		sql = db_sql_mprintf_card("%q,%q%Q,%Q)))", sql, "datetime TimeStamp NOT NULL DEFAULT (datetime(", "now","localtime");
	} else {
		sql = db_sql_mprintf_card("%q)", sql);
		dbg_inf("sql=%s\n", sql);
	}

	rc = db_exec_sql(db, sql, 0);
	switch(rc) {
		case SQLITE_DONE:
			rc = DB_OK;
		case SQLITE_ERROR:
		case SQLITE_OK:
			break;
			
			default:
				break;
	}

	return rc;
}

/**********************************************************************************
*Function name	: SQLITE_DATA_INIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sqlite_data_init(sqlite3 *db)
{
	int rc = 0;
	lg_db_handle = db;
	/*
	rc = db_create_tbl(db, tbl_alarm, alarm_info, TRUE);
	rc = db_create_tbl(db, tbl_card_unlock, card_unlock, TRUE);
	rc = db_create_tbl(db, tbl_call_history, call_history, TRUE);
	rc = db_create_tbl(db, tbl_ip_camera, lg_ip_cameras, FALSE);
	*/
    rc = db_create_tbl(db, tbl_card_unlock, card_unlock, FALSE);
	rc = db_create_tbl(db, tbl_card_unlock_ex, card_unlock_ex, FALSE);
	rc = db_create_tbl(db, tbl_contact, contact, FALSE);		
	rc = db_create_tbl(db, tbl_sub_controller, lg_sub_controller, FALSE);
	
	rc = db_create_tbl(db, tbl_vlan_list, lg_vlan_list, FALSE);

#ifdef DOOR_ACCESS_CARD_LIST
	rc = create_door_card_tbl(db);	
#endif

	rc = db_create_tbl_card(db, tbl_dcard_inf, lg_dcard_inf, FALSE);
	if(1 == rc) {
		manage_door_card_index(1);
	}

	return rc;
}


/**********************************************************************************
*Function name	: SQL_EXIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void sql_data_exit(void)
{	
	db_close_file(lg_db_handle);
}



/**********************************************************************************
*Function name	: DELETE_TBL_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_tbl_data(const char *const tbl_name,  const sql_column_t *const column)
{
	int rc = 0;
	sqlite3 *db = lg_db_handle;
	sql_cmd_t bdelete = {0, };
	db_blob_t data = {0, };
	int src_data = 0;

	if(assert_ptr(db) || assert_ptr(tbl_name) || assert_ptr(column)) {
		return -1;
	}

	dbg_inf("delete table data\n");
	rc = db_fill_blob(&data, (uint8_t*)&src_data, sizeof(src_data));	

	rc = db_sql_init(db, &bdelete, DDELETE, tbl_name);
	bdelete.condition = CNDTN_NULL;

	rc = db_build_sql_stmt(&bdelete, 0, &data, column);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}
	
	dbg_inf("delete,sql=%s\n", bdelete.sql);
	rc = db_exec_state(db, bdelete.state, TRUE);
	if(DB_OK != rc && SQLITE_DONE != rc) {
		debug_err("db exec sql,rc=%d,%s\n", rc, DBERR_MSG(db));
		return rc;
	} else {
		rc = DB_OK;
	}

	return rc;
}

/**********************************************************************************
*Function name	: FREE_CONTROLLERS_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int free_controllers_data(controoler_sql_t *const fptr)
{
	controoler_sql_t *cur = fptr;
	
	if(assert_ptr(fptr)) {
		return -1;
	}

	for(; cur!=NULL; ) {
		free(cur);
		cur=cur->next;
	}

	return 0;
}


/**********************************************************************************
*Function name	: DELETE_INFO_CONTROLLER
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_info_controller(void)
{
	int rc = 0;
	sqlite3 *db = lg_db_handle;
	sql_cmd_t bdelete = {0, };

	debug_info("delete info controller\n");
	db_sql_init(db, &bdelete, DDELETE, tbl_sub_controller);
	bdelete.condition = CNDTN_NULL;
	
	rc = db_build_sql_stmt(&bdelete, 1, NULL, lg_sub_controller);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}
	
	rc = db_exec_state(db, bdelete.state, TRUE);
	if(DB_OK != rc && SQLITE_DONE != rc) {
		debug_err("db exec sql,rc=%d,%s\n", rc, DBERR_MSG(db));
		return rc;
	} else {
		rc = DB_OK;
	}

	return rc;
}


/**********************************************************************************
*Function name	: INSERT_INFO_CONTROLLER
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_info_controller(controller_data_t *res)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 1;

	if(assert_ptr(res)) {
		return -1;
	}

	sqlite3 *db = lg_db_handle;

	/* debug_info("insert info controller\n"); */

	db_sql_init(db, &binsert, DINSERT, tbl_sub_controller);
	
	data.data = (uint8_t*)&src_data;
	data.len = sizeof(uint16_t);


	src_data = res->ext_id;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_sub_controller);
	src_data = res->sub_id;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, lg_sub_controller);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}	


/**********************************************************************************
*Function name	: DELETE_VLAN_LIST
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_vlan_list(void)
{
	return delete_tbl_data(tbl_vlan_list, lg_vlan_list);
}


/**********************************************************************************
*Function name	: DO_SAVE_INFO
*Description		: 
in database, ID	| ext_id    				| sub_id
			1	| len	 				| val
			2	| sub-controllers amount	| extensions amount
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int do_save_inf_container(const void *const buf, const int len)
{
	int rc = 0, i = 0;
	controller_data_t res = {0, }, * ptres = &res;
	uint8_t *ptr = (uint8_t *)buf;
	sqlite3 *db = lg_db_handle;
	debug_info("do save info container data,sizeof(buf)=%d\n", len);
#if 1	

	print_n_byte(ptr, 6);
	debug_print("\n");
	for(i=0,ptr+=6; i<len/4; i++) {
		print_n_byte(ptr+4*i, 4);
	}
#endif
	if(assert_ptr(buf)) {
		return -1;
	}
	delete_info_controller();

	db_routine_begin(db, 0);
#if 0
	ptr = (uint8_t *)buf;
	ptres->ext_id = len;
	ptres->sub_id = *(short int *)ptr;
	rc = insert_info_controller(ptres);
	if(rc < 0) {
		db_routine_submit(db, -1);
		return rc;
	}
	
	ptr += 2;
	ptres->ext_id = *(uint16_t*)ptr;/* sub-controllers amount */
	ptr += 2;
	ptres->sub_id = *(uint16_t*)ptr;/* extensions amount */
	rc = insert_info_controller(ptres);
	if(rc < 0) {
		db_routine_submit(db, -1);
		return rc;
	}
	ptr += 2;

#else
	ptr = (uint8_t *)buf;
	ptres->ext_id = 0;
	ptres->sub_id = 0;
	rc = insert_info_controller(ptres);
	if(rc < 0) {
		db_routine_submit(db, -1);
		return rc;
	}
	
	ptr += 2;
	ptres->ext_id = 0;/* sub-controllers amount */
	ptr += 2;
	ptres->sub_id = 0;/* extensions amount */
	rc = insert_info_controller(ptres);
	if(rc < 0) {
		db_routine_submit(db, -1);
		return rc;
	}
	ptr += 2;

	
#endif
	

	for(i=0; ptres && i<(len-6)/4; i++) {
		ptres = (controller_data_t *)(ptr);
		rc = insert_info_controller(ptres);
		if(rc < 0) {
			break;
		}
		ptr += 4;
	}
	db_routine_submit(db, rc);

	return rc;
}

/**********************************************************************************
*Function name	: DO_LOAD_INFO_CONTAINER
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int do_load_info_container(void *buf,int len)
{
	int ret = 0;
	uint8_t *ptr = (uint8_t *)buf;

	int code_len = 0;
	int i = 0;
	controoler_sql_t inf_cntnr = {0,};
	inf_cntnr.id = 1;

	if(assert_ptr(buf)) {
		return -1;
	}
	
	ret = query_info_controller(0, &inf_cntnr);
	if(ret < 0) {
		debug_err("query no data, failed\n");
		return ret;
	} else if(assert_ptr(inf_cntnr.next)) {
		return -1;
	}

	code_len = inf_cntnr.next->data.ext_id;
	memcpy(ptr,&inf_cntnr.next->data.sub_id, 2);
	free_query_list(inf_cntnr.next, sizeof(inf_cntnr));
	//free_controllers_data(inf_cntnr.next);
	ptr += 2;


	inf_cntnr.id = 2;
	ret = query_info_controller(0, &inf_cntnr);
	if(ret < 0) {
		debug_err("query failed\n");
		return ret;
	}

	memcpy(ptr,&inf_cntnr.next->data.ext_id, 4);
	free_query_list(inf_cntnr.next, sizeof(inf_cntnr));
	//free_controllers_data(inf_cntnr.next);
	ptr += 4;

	inf_cntnr.id = 2;
	
	//dbg_blue("query info controller 3\n");
	ret = query_info_controller(2, &inf_cntnr);
	if(ret < 0) {
		debug_err("query failed\n");
		return ret;
	} else if(assert_ptr(inf_cntnr.next)) {
		return -1;
	}
	//dbg_blue("finished query\n");
	controoler_sql_t *tmp_inf = inf_cntnr.next;
	
	for(i=0; tmp_inf && i<ret && ptr; i++) {
		memcpy(ptr, &tmp_inf->data, 4);
		tmp_inf = tmp_inf->next;
		ptr += 4;
	}
	
	if(i != ret) {
		debug_err("memcpy ptr\n");
		return -1;
	}

	free_query_list(inf_cntnr.next, sizeof(inf_cntnr));
	//free_controllers_data(inf_cntnr.next);

	return (ret*4 + 6);
}


int do_load_catalog(char *buf, int len)
{
	contact_t add_book = {0, };
	int rc = 0;
	
	rc = query_address_book(&add_book, NULL, 0);
	if(rc > 0) {
        int i = 0;
    	contact_t *list = (contact_t *)&add_book;
    	if(NULL == list) {
    		debug_err("list is null\n");
    		return -1;
    	}

        //debug_print("id     uid     grade    pid     dev_no ");
        //debug_print("\t dev_name \n");
        list = list->next;
    	for(i = 0; list && (i*40<len); i++) {
            //debug_print("%04d   %04d    %04d    %04d    ", list->id,list->uid,list->grade,list->pid);
            //debug_print("%08X \t", list->dev_no);
            //debug_print("%s\n", list->dev_name);
            memcpy(&buf[i*40],   &(list->dev_no), 4);
            strncpy(&buf[i*40+4], list->dev_name, 36);
    		list = list->next;
    	}
		free_query_list(add_book.next, sizeof(contact_t));
	}

	return rc;
}

int do_save_catalog(char *buf, int len)
{
    sqlite3 * db = lg_db_handle;
    pack_contact_t address_book = {0,};
	int ret = delete_adress_book(0);
	int count = (len-6) / 40;
    int i;
    printf("count:%d  len:%d\n", count, len);
	debug_info("insert contacts\n");
	db_routine_begin(db, 0);
	for(i=0; i<count; i++) {
        address_book.id  = i;
        address_book.uid = i;
        address_book.pid = i;
        memcpy(&(address_book.dev_no), &buf[i*40+6], 4);
        strncpy(address_book.dev_name, &buf[i*40+10], 36);
		ret = insert_contacts(&address_book);
		if(ret != 0) {
			debug_err("insert contacts failed,ret=%d\n", ret);
			ret = -1;
			break;
		}
	}
	db_routine_submit(db, ret);

	return ret;
}
int do_load_card_unlock(char *buf, int len)
{
	card_unlock_t card_ulk = {0, };
	int rc = 0;
	
	rc = query_card_unlock_info(&card_ulk, 0);
	if(rc > 0) {
        int i = 0;
    	card_unlock_t *list = (card_unlock_t *)&card_ulk;
    	if(NULL == list) {
    		debug_err("list is null\n");
    		return -1;
    	} 

        list = list->next;
    	for(i = 0; list && (i*12<len); i++) {
            memcpy(&buf[i*12],   &(list->dev_no), 4);
            memcpy(&buf[i*12+4], &(list->card_no), 4);
            memcpy(&buf[i*12+8], &(list->validity), 4);
            /*
            printf("load card_unlock dev_no: %d \n", list->dev_no);
            printf("load card_unlock card_no: %02X%02X%02X%02X \n", buf[i*12+4] ,buf[i*12+5], buf[i*12+6], buf[i*12+7]);
            printf("load card_unlock validity: %02X%02X%02X%02X \n", buf[i*12+8] ,buf[i*12+9], buf[i*12+10], buf[i*12+11]);
            */
    		list = list->next;
    	}
		free_query_list(card_ulk.next, sizeof(card_unlock_t));
	}

	return rc;
}

int do_save_card_unlock(char *buf, int len)
{
    sqlite3 * db = lg_db_handle;
    card_unlock_t_ex card_ulk = {0,};
	int ret = clear_card_unlock();
	int count = (len-6) / 12;
    int i;
    char *data = buf + 6;  //有效数据从第6Bytes开始
    printf("insert card_unlock count:%d  len:%d\n", count, len);
	db_routine_begin(db, 0);
	for(i=0; i<count; i++) {
        card_ulk.id = i;
        memcpy(&(card_ulk.dev_no),   &data[i*12], 4); 
        memcpy(&(card_ulk.card_no),  &data[i*12+4], 4);
        memcpy(&(card_ulk.validity), &data[i*12+8], 4);
        /*
        printf("insert card_unlock room:%02X%02X%02X%02X \n", data[i*12] ,data[i*12+1], data[i*12+2], data[i*12+3]);
        printf("insert card_unlock card_no:%02X%02X%02X%02X %d \n", data[i*12+4] ,data[i*12+5], data[i*12+6], data[i*12+7], *((unsigned int*)&data[i*12+4]));
        printf("insert card_unlock validity:%02X%02X%02X%02X \n", data[i*12+8] ,data[i*12+9],data[i*12+10], data[i*12+11]);
        */
		ret = insert_card_unlock_info(card_ulk.id, card_ulk.dev_no, card_ulk.card_no, card_ulk.validity);
		if(ret != 0) {
			debug_err("insert card_unlock failed,ret=%d\n", ret);
			ret = -1;
			break;
		}
	}
	db_routine_submit(db, ret);

	return ret;
}

int do_load_card_unlock_ex(char *buf, int len)
{
	card_unlock_t_ex card_ulk = {0, };
	int rc = 0;
	
	rc = query_card_unlock_info_ex(&card_ulk, 0);
	if(rc > 0) {
        int i = 0;
    	card_unlock_t_ex *list = (card_unlock_t_ex *)&card_ulk;
    	if(NULL == list) {
    		debug_err("list is null\n");
    		return -1;
    	} 	
        list = list->next;

    	for(i = 0; list && (i*28<len); i++) {//*12
            memcpy(&buf[i*28], &(list->dev_no), 4);
            memcpy(&buf[i*28+4], &(list->card_no), 4);
            memcpy(&buf[i*28+8], &(list->validity), 4);
	   		memcpy(&buf[i*28+12], &(list->blacklist), 4);		


			printf("---------------------------------------------------\n");	
            printf("load card_unlock dev_no:  %02X%02X%02X%02X  %d \n", buf[i*28] ,buf[i*28+1], buf[i*28+2], buf[i*28+3],list->dev_no);
            printf("load card_unlock card_no: %02X%02X%02X%02X  %d \n", buf[i*28+4] ,buf[i*28+5], buf[i*28+6], buf[i*28+7],list->card_no);
            printf("load card_unlock validity: %02X%02X%02X%02X \n",    buf[i*28+8] ,buf[i*28+9], buf[i*28+10], buf[i*28+11]);
	    	printf("load card_unlock blacklist: %d \n", list->blacklist);			
   			printf("---------------------------------------------------\n"); 
		       
    		list = list->next;
    	}    	
		free_query_list(card_ulk.next, sizeof(card_unlock_t_ex));
	}

	return rc;
}

int do_save_card_unlock_ex(char *buf, int len)
{
    sqlite3 * db = lg_db_handle;
    card_unlock_t_ex card_ulk = {0,};
	int ret = clear_card_unlock_ex();
	int count = (len-6) / 28;
    int i;
    char *data = buf + 6;  //有效数据从第6Bytes开??
    printf("insert card_unlock count:%d  len:%d\n", count, len);

	for(i=0;i<len;i++)
		printf("buf[%d]=%02x\t",i,buf[i]);
   // printf("do_save_card_unlock %s\n",buf);
	db_routine_begin(db, 0);
	for(i=0; i<count; i++) {
        card_ulk.id = i;
        memcpy(&(card_ulk.dev_no),   &data[i*28], 4); 
        memcpy(&(card_ulk.card_no),  &data[i*28+4], 4);
        memcpy(&(card_ulk.validity), &data[i*28+8], 4);
        memcpy(&(card_ulk.blacklist), &data[i*28+12], 4);


#if 0  

			printf("---------------------------------------------------\n");	
            printf("load card_unlock dev_no:  %02X%02X%02X%02X  %d \n", data[i*28] ,data[i*28+1], data[i*28+2], data[i*28+3],card_ulk->dev_no);
            printf("load card_unlock card_no: %02X%02X%02X%02X  %d \n", data[i*28+4] ,data[i*28+5], data[i*28+6], data[i*28+7],card_ulk->card_no);
            printf("load card_unlock validity: %02X%02X%02X%02X \n",    data[i*28+8] ,data[i*28+9],data[i*28+10], data[i*28+11]);
	    	printf("load card_unlock blacklist: %d \n", list->blacklist);			
   			printf("---------------------------------------------------\n"); 
#endif  
     
		printf("---------------------------------------------------\n"); 
        printf("insert card_unlock room:%02X%02X%02X%02X %d\n", data[i*28] ,data[i*28+1], data[i*28+2], data[i*28+3], card_ulk.dev_no);
        printf("insert card_unlock card_no:%02X%02X%02X%02X %d \n", data[i*28+4] ,data[i*28+5], data[i*28+6], data[i*28+7], *((unsigned int*)&data[i*28+4]));
        printf("insert card_unlock validity:%02X%02X%02X%02X \n", data[i*28+8] ,data[i*28+9],data[i*28+10], data[i*28+11]);
		printf("insert card_unlock blacklist:%d \n", *((unsigned int*)&data[i*28+12]));
        printf("---------------------------------------------------\n"); 
    
		ret = insert_card_unlock_info_ex(card_ulk.id, card_ulk.dev_no, card_ulk.card_no, card_ulk.validity,card_ulk.blacklist);
		if(ret != 0) {
			debug_err("insert card_unlock failed,ret=%d\n", ret);
			ret = -1;
			break;
		}
	}
	db_routine_submit(db, ret);

	return ret;
}

#ifndef VLDATA_SIZE
	#define	VLDATA_SIZE			(18)
#endif

/**********************************************************************************
*Function name	: STORE_VLAN_LIST_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int store_vlan_list_data(uint8_t *buf, int len)
{
	int ret = 0;
	int count = 0, i = 0;
	sqlite3 *db = lg_db_handle;
	uint8_t *dptr = buf;
	
	if(assert_ptr(buf)) {
		return -1;
	}
	ret = delete_vlan_list();

	memcpy(&count, dptr, sizeof(count));
	printf("count = %d\n",count);
	dptr += 8;
	ret = db_routine_begin(db, 0);
	for(i=0; i<count && dptr; i++) {
		ret = insert_vlan_list_data((pkd_vlan_list_t*)dptr);
		dptr += VLDATA_SIZE;
	}
	ret = db_routine_submit(db, ret);
	
	if(ret >= 0) {
		manage_vlan_list_index(0);
		manage_vlan_list_index(1);
	} else {
		dbg_err("store vlan list data\n");
	}

	return ret;
}

/**********************************************************************************
*Function name	: DO_LOAD_VLAN_LIST_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int do_load_vlan_list_data(uint8_t *dbuf, int len)
{
	uint8_t *dptr = (uint8_t *)dbuf;

	int ret = 0, i = 0,sum;
	vlan_list_t dst_data = {0,}, *tmp = NULL;
	
	if(assert_ptr(dbuf)) {
		return -1;
	}
	
	ret = query_vlan_list(&dst_data, 2);
	printf("do_load_vlan_list_data--ret=%d\n", ret);
	if(ret > 0) {
		bzero(dptr, 8);
		memcpy(dptr, (uint8_t *)&ret, sizeof(ret));
		dptr += 8;
		tmp = dst_data.next;

		for(i=0; i<ret && tmp; i++) {
			/*printf("\n%08X ", tmp->data.dev_no);
			printf("%08X ", tmp->data.reserved1);
			printf("%08X ", tmp->data.reserved2);
			printf("%08X ", tmp->data.ipaddr);
			printf("%08X \r\n", tmp->data.reserved3);*/
			memcpy(dptr, (uint8_t*)&tmp->data.dev_no, VLDATA_SIZE);
			tmp = tmp->next;
			dptr += VLDATA_SIZE;

			
		}
	}
	sum = ret * VLDATA_SIZE + 8;
	printf("do_load_vlan_list_data------sum=%d\n",sum);
	return (ret * VLDATA_SIZE + 8);
}



//static sqlite3 *lg_db_handle = NULL;
/**********************************************************************************
*Function name	: COUNT_RES_CB
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int count_res_cb(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	int ctype = 0;
	int clm_cnt = 0;
	int i = 0;
	
	int count = 0, rc = -1;
	int *pdata = &count;
	sqlite3_stmt *state = *ptr_state;

	if(assert_ptr(dest) || assert_ptr(state)) {
		return -1;
	}
	
	clm_cnt = sqlite3_column_count(state);
	for(i=0; i<clm_cnt; i++) {
		ctype = sqlite3_column_type(state, i);
		switch(ctype) {
			case SQLITE_FLOAT: 
				break;
				
			case SQLITE_TEXT:	
			case SQLITE_BLOB:
				rc = db_column_data(state, i, pdata);
				if(SQLITE_OK != rc) {
					return -1;
				}
				break;
				
			case SQLITE_INTEGER:
			default:
				rc = db_column_data(state, i, &count);
				if(SQLITE_OK != rc) {
					return -1;
				}
				break;
		}		
	}

	return count;
}







/**********************************************************************************
*Function name	: COUNT_BY_ROWS
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int count_by_rows(const sql_cmd_t *const cmd)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	sqlite3_stmt *state = NULL;
	sqlite3 *db = lg_db_handle;
	int src_data[2] = {0,0};
	int col_no = 0;
	
	dbg_inf("count by rows\n");
	if(assert_ptr(cmd) || assert_ptr(cmd->name) || assert_ptr(cmd->columns)) {
		return -1;
	}
	
	db_sql_init(db, &bquery, DCOUNT, cmd->name);
	bquery.condition = CNDTN_NULL;

	bquery.query.condition = EQUALTO;
	bquery.query.qord.ord = ORCUNT;
	bquery.query.qord.col = 0;
	bquery.deal_fun = count_res_cb;
	bquery.state = state;
	data.data = (uint8_t*)&src_data[0];
	data.len = sizeof(int);
	
	rc = db_build_sql_stmt(&bquery, col_no, &data, cmd->columns);
	if(DB_OK != rc) {
		dbg_err("build sql\n");
		return rc;
	}


	rc = do_query_sql(db, &bquery);
	dbg_inf("counted rc=%d rows\n", rc);

	return rc;
	
}

/**********************************************************************************
*Function name	: DELETE_BY_ROW
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_by_row(const sql_cmd_t *const cmd, const int count)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bdel = {0, };
	sqlite3 *db = lg_db_handle;
	int col_no = 0;
	int i = 0;
	
	dbg_inf("delete by row\n");
	if(assert_ptr(cmd) || assert_ptr(cmd->name) || assert_ptr(cmd->columns)) {
		return -1;
	}
	
	db_sql_init(db, &bdel, DDELETE, cmd->name);
	bdel.condition = SMIN;
		
	rc = db_build_sql_stmt(&bdel, col_no, &data, cmd->columns);
	if(DB_OK != rc) {
		dbg_err("build sql\n");
		return rc;
	}

	db_routine_begin(db, 0);
	for(i=0; i<(count-1); i++) {
		rc = db_exec_state(db, bdel.state, FALSE);
		if(DB_OK != rc) {
			db_routine_submit(db, -1);
			dbg_err("");
			return rc;
		}
	}
	
	db_routine_submit(db, rc);
	if(count>0) {
		rc = db_exec_state(db, bdel.state, TRUE);
	}

	return rc;
}

/**********************************************************************************
*Function name	: ALTER_TABLE_NAME
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int alter_table_name(const char *const ori_tab, const char *const new_tab)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bdel = {0, };
	sqlite3 *db = lg_db_handle;
	int col_no = 0;
	sql_column_t not_use_clo = {0,};


	if(assert_ptr(ori_tab) || assert_ptr(new_tab)) {
		return -1;
	}
	if(assert_ptr(db)) {
		return -1;
	}
	dbg_inf("alter table name\n");

	db_sql_init(db, &bdel, DALTER, ori_tab);
	bdel.order = 1;

	data.data = (uint8_t*)new_tab;
	rc = db_build_sql_stmt(&bdel, col_no, &data, &not_use_clo);
	if(DB_OK != rc) {
		dbg_err("build sql\n");
		return rc;
	}
	
	rc = db_exec_state(db, bdel.state, TRUE);

	return 0;
}



/**********************************************************************************
*Function name	: SQLDATA_ROUTINE_BEGIN
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sqldata_routine_begin(const int opt)
{
	sqlite3 *db = lg_db_handle;
	return db_routine_begin(db, opt);
}

/**********************************************************************************
*Function name	: SQLDATA_ROUTINE_SUBMIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sqldata_routine_submit(const int res)
{
	sqlite3 *db = lg_db_handle;
	return db_routine_submit(db, res);
}

column_struct_t lg_dcard_column[] = {
	{"ID","INTEGER PRIMARY KEY","ID"},
	{"dev_no",SQLINT,"dev_no"},
	{"cardnumber",SQLTEXT,"cardnumber"},
	{"init_date",SQLTEXT,"init_date"},
	{"exprirydate",SQLTEXT,"exprirydate"},
	{"blacklist",SQLINT,"blacklist"},
	{"reserved0",SQLINT,"reserved0"},
	{"card_type",SQLINT,"card_type"},
	{"reserved1",SQLINT,"reserved1"},
	{"reserved2",SQLINT,"reserved2"},
	{NULL,	NULL,	NULL},
};
column_pack_i lg_dcard_data[]=
{
		{NULL,0,0},
		{NULL,0,1},
		{NULL,0,2},
		{NULL,0,3},
		{NULL,0,4},
		{NULL,0,5},
		{NULL,0,6},
		{NULL,0,7},
		{NULL,0,8},
		{NULL,0,9},
		{NULL,0,10},
		{NULL,0,11},
};

/**********************************************************************************
*Function name	: MANAGE_DOOR_CARD_INDEX
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int manage_door_card_index(int opt)
{
	int rc = 0;
	sqlite3 * db = lg_db_handle;
	sql_cmd_t ccmd = {0,};
	
	rc = db_build_sql_init(db, &ccmd, DQUERY, tbl_dcard_inf, lg_dcard_inf);
	
	if(opt == 0) {
		rc = db_drop_index(&ccmd);
	} else {
		ccmd.fd_set = Bit2;
		rc = db_create_index(&ccmd);
	}

	return rc;
}

static int lg_routine_lock = 0;
static pthread_mutex_t lg_db_busy = PTHREAD_MUTEX_INITIALIZER;

/**********************************************************************************
*Function name	: DB_ROUTINE_BEGIN
*Description		: ?????
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int db_routine_begin_card(sqlite3 *const db, int opt)
{
/*	if(pthread_mutex_lock(&lg_db_busy) < 0) {
		debug_err("lock call_item_lock failure !\n");
		return -1;
	}*/
	if(opt) {
			if(pthread_mutex_trylock(&lg_db_busy)) {
				debug_err("trylock mutex failure !\n");
				return -1;
			}
		} else {
			if(pthread_mutex_lock(&lg_db_busy) < 0) {
				debug_err("lock mutex failure !\n");
				return -1;
			}
		}

	if(NULL == db)
		return -1;

	lg_routine_lock = 1;
	sqlite3_exec(db, "begin;", 0, 0, 0);  /*开始事物*/

	return 0;
}



/*typedef struct ST_CARD_INFO_SQL{
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
}dcard_list_t;*/

void card_number_8byte_call_back_fun(data_ret_g *ret,void **cur_item,void **list_head)
{
	int type=ret->data_type;
	switch(type)
	{
	case SI_INTEGER:
	{
		switch(ret->icol)
		{
		case 0:
			((dcard_list_t *)(*cur_item))->id=ret->int_ret;
			break;
		case 1:
			memcpy(((dcard_list_t *)(*cur_item))->dev_no,&ret->int_ret,4);
			break;
		case 5:
			((dcard_list_t *)(*cur_item))->blacklist=ret->int_ret;
			break;
		case 6:
			((dcard_list_t *)(*cur_item))->reserved0=ret->int_ret;
			break;
		case 7:
			((dcard_list_t *)(*cur_item))->card_type=ret->int_ret;
			break;
		case 8:
			((dcard_list_t *)(*cur_item))->reserved1=ret->int_ret;
			break;
		case 9:
			((dcard_list_t *)(*cur_item))->reserved2=ret->int_ret;
			break;
		}
	}
	break;
	case SI_FLOAT:
		break;
	case SI_BLOB:
	{
		switch(ret->icol)
		{
		case 2:
		{
			if(ret->data_len>0)
			{
				memcpy(((dcard_list_t *)(*cur_item))->cardnumber,ret->ptr_ret,sizeof(((dcard_list_t *)(*cur_item))->cardnumber));
			}
		}
		break;
		case 3:
		{
			if(ret->data_len>0)
			{
				memcpy(((dcard_list_t *)(*cur_item))->init_date,ret->ptr_ret,sizeof(((dcard_list_t *)(*cur_item))->init_date));
			}
		}
		break;
		case 4:
		{
			if(ret->data_len>0)
			{
				memcpy(((dcard_list_t *)(*cur_item))->exprirydate,ret->ptr_ret,sizeof(((dcard_list_t *)(*cur_item))->exprirydate));
			}
		}
		break;
		}
	}
	break;
	case SI_NULL:
		break;
	case SI_TEXT:
		break;
	case SI_NEW_ITEM:
	{
		if(*list_head==NULL)
		{
			*list_head=(dcard_list_t *)calloc(sizeof(dcard_list_t),1);
			if(list_head==NULL)
			{
				fprintf(stderr, "%s:err ",__FUNCTION__);
				perror("calloc for list_head:");
				*list_head=NULL;
				return ;
			}
			(*cur_item)=(*list_head);
			((dcard_list_t *)(*list_head))->next=NULL;
		}
		else
		{
			((dcard_list_t *)(*cur_item))->next=calloc(sizeof(dcard_list_t),1);
			if(((dcard_list_t *)(*cur_item))->next==NULL)
			{
				fprintf(stderr, "%s:err ",__FUNCTION__);
				perror("calloc for list_head:");
				return ;
			}
			((dcard_list_t *)(*cur_item))->next->last=((dcard_list_t *)(*cur_item));
			*cur_item=((dcard_list_t *)(*cur_item))->next;
			((dcard_list_t *)(*cur_item))->next=NULL;

		}
	}
	break;
	default:
		break;
	}
}

void free_card_number_8byte_list(dcard_list_t *list)
{
	if(list==NULL)
	{
		printf("dcard_list_t list is NULL\n");
		return;
	}
	dcard_list_t *cur=list;
	dcard_list_t *prv;
	while(cur!=NULL)
	{
		prv=cur;
		cur=cur->next;
		free(prv);
	}
}



/*
 * dst:输入参数，待删除操作的条件信息
 * index：表示dcard_list_t的第几个元素,同数据库中的表的列相同,从0开始
 */

int delete_door_card_list_number_8byte(pdoor_card2_t *dst, int index)
{
	sqlite3_stmt *stat;
	int rc = 0;
	db_handle_t db_h;
	db_h.db= lg_db_handle;
	char * sql =NULL;
	switch(index)
	{
	case 0:
	{
		sql = "delete from card_inf where dev_no=?";
	}
	break;
	case 1:
	{
		sql = "delete from card_inf where cardnumber=?";
	}
	break;
	case 2:
	{
		sql = "delete from card_inf where init_date<?";
	}
	break;
	case 3:
	{
		sql = "delete from card_inf where exprirydate<?";
	}
	break;
	case 4:
	{
		sql = "delete from card_inf where blacklist!=0";
	}
	break;
	}

	rc = sqlite3_prepare_v2(db_h.db, sql, strlen(sql)+1, &stat,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr, "%s_ sqlite3_prepare err:%s !\n",__FUNCTION__,sqlite3_errmsg(db_h.db));
	}
	switch(index)
	{

	case 0:
	{
		uint32_t devno;
		memcpy(&devno,dst->dev_no,sizeof(uint32_t));
		rc=sqlite3_bind_int(stat,1,devno);
		if(rc!=SQLITE_OK)
		{
			printf("%s_qlite3_bind_int err:%s\n",__FUNCTION__,sqlite3_errmsg(db_h.db));
		}
	}
	break;
	case 1:
	{
		rc=sqlite3_bind_blob(stat,1,dst->cardnumber,sizeof(dst->cardnumber), NULL);
		if(rc!=SQLITE_OK)
		{
			printf("%s_sqlite3_bind_blob err:%s\n",__FUNCTION__,sqlite3_errmsg(db_h.db));
		}
	}
	break;
	case 2:
	{
		rc=sqlite3_bind_blob(stat,1,dst->init_date,sizeof(dst->init_date), NULL);
		if(rc!=SQLITE_OK)
		{
			printf("%s_sqlite3_bind_blob err:%s\n",__FUNCTION__,sqlite3_errmsg(db_h.db));
		}
	}
	break;
	case 3:
	{
		rc=sqlite3_bind_blob(stat,1,dst->exprirydate,sizeof(dst->exprirydate), NULL);
		if(rc!=SQLITE_OK)
		{
			printf("%s_sqlite3_bind_blob err:%s\n",__FUNCTION__,sqlite3_errmsg(db_h.db));
		}
	}
	break;
	}

	if(sqlite3_step(stat)!=SQLITE_DONE)
	{
		fprintf(stderr, "%s_ delete sqlite3_step err:%s !\n",__FUNCTION__,sqlite3_errmsg(db_h.db));
	}
	sqlite3_reset(stat);
	sqlite3_finalize(stat);
	return rc;
}



int query_door_card_list_number_8byte(dcard_list_t *dst, int opt)
{
	sqlite3_stmt *stat;
	int rc = 0/*,size = 0*/;
	int column_count=0;
	void *head=NULL,*cur=NULL;
	int row_count=0;
	db_handle_t db_h;
	db_h.db= lg_db_handle;
	char * sql = "select * from card_inf where cardnumber=?";
	rc = sqlite3_prepare_v2(db_h.db, sql, strlen(sql)+1, &stat,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr, "%s_ sqlite3_prepare err:%s !\n",__FUNCTION__,sqlite3_errmsg(db_h.db));
	}
	rc=sqlite3_bind_blob(stat,1,dst->cardnumber,sizeof(dst->cardnumber), NULL);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_bind_text err:%s\n",sqlite3_errmsg(db_h.db));
	}
	column_count=sqlite3_column_count(stat);
	int i=0;
	data_ret_g ret;
	while(sqlite3_step(stat)==SQLITE_ROW)
	{
		row_count++;
		ret.data_type=SI_NEW_ITEM;
		card_number_8byte_call_back_fun(&ret,&cur,&head);
		for(i=0;i<column_count;i++)
		{
			ret.data_type=sqlite3_column_type(stat,i);
			switch(ret.data_type)
			{
			case SI_INTEGER:
			{
				ret.int_ret=sqlite3_column_int(stat,i);
				ret.icol=i;

				ret.data_len=sizeof(int);
				card_number_8byte_call_back_fun(&ret,&cur,&head);
			}
			break;
			case SI_FLOAT:
			{
				ret.dou_ret=sqlite3_column_double(stat,i);
				ret.icol=i;
				ret.data_len=sizeof(double);
				card_number_8byte_call_back_fun(&ret,&cur,&head);
			}
			break;
			case SI_BLOB:
			{
				ret.ptr_ret=(void *)sqlite3_column_blob(stat,i);
				ret.icol=i;
				ret.data_len=sqlite3_column_bytes(stat,i);
				card_number_8byte_call_back_fun(&ret,&cur,&head);
			}
			break;
			case SI_NULL:
				//fun(NULL,i,sizeof(int));
				break;
			case SI_TEXT:
			{
				ret.ptr_ret=(void *)sqlite3_column_text(stat,i);
				ret.icol=i;
				ret.data_len=sqlite3_column_bytes(stat,i);
				card_number_8byte_call_back_fun(&ret,&cur,&head);
			}
			break;
			default:
				break;
			}
		}
	}
	sqlite3_reset(stat);
	sqlite3_finalize(stat);
	if(head!=NULL)
	{
		dcard_list_t*ptr=(dcard_list_t*)head;
		memcpy(dst,head,sizeof(dcard_list_t)-sizeof(ptr)*2);
		free_card_number_8byte_list(head);
	}
	head=NULL;
	add_insert_comut();
	return row_count;

#if 0


	int rc = 0;
	query_struct_g *query_msg_card=alloc_query_struct();

	add_order_pri(query_msg_card,0,ORDER_DESC);//需添加order条件和列条件匹配情况
	add_query_column_val(query_msg_card,dst->cardnumber,sizeof(dst->cardnumber),2,"==",NULL);

	query_cmd_g resault_card;
	build_query_cmd(&lg_dcard_column[0],"card_inf",query_msg_card,NULL,&resault_card);
	if(resault_card.query_cmd!=NULL)
	{
		printf("resault.query_cmd:%s\n",resault_card.query_cmd);
	}
	db_handle_t db_h;
	db_h.db=lg_db_handle;
	dcard_list_t *card_ret=query_data_from_table(&db_h,&resault_card,&rc,(void *)&card_number_8byte_call_back_fun);
	if(card_ret!=NULL)
	{
		memcpy(dst,card_ret,sizeof(dcard_list_t));
		/*dcard_list_t *q_cur=card_ret;
			while(q_cur!=NULL)
			{
				printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				int i=0;
				printf("q_cur->id       :%d\n",q_cur->id);
				printf("q_cur->dev_no   :%g\n",q_cur->dev_no);
				for(i=0;i<4;i++)
				{
					printf(" %02x ",q_cur->dev_no[i]);
				}
				printf("\nq_cur->cardnumber   :\n");

				for(i=0;i<8;i++)
				{
					printf(" %02x ",q_cur->cardnumber[i]);
				}
				printf("\nq_cur->init_date   :\n");

				for(i=0;i<6;i++)
				{
					printf(" %02x ",q_cur->init_date[i]);
				}
				printf("\nq_cur->exprirydate   :\n");

				for(i=0;i<6;i++)
				{
					printf(" %02x ",q_cur->exprirydate[i]);
				}
				printf("\nq_cur->blacklist       :%d\n",q_cur->blacklist);
				printf("q_cur->blacklist       :%d\n",q_cur->reserved0);
				printf("q_cur->blacklist       :%d\n",q_cur->card_type);
				printf("q_cur->blacklist       :%d\n",q_cur->reserved1);
				printf("q_cur->blacklist       :%d\n",q_cur->reserved2);
				printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				q_cur=q_cur->next;
			}
		}
		else
		{
			printf("*******************************\n");
		}
		 */
	}
	free_query_struct(query_msg_card);
	free_card_number_8byte_list(card_ret);
	return rc;
#endif
}


inline int insert_card_number_8byte_2_sqlite(db_handle_t *db_h,pdoor_card2_t *card_data,int data_items)
{
	int rc = 0;
	insert_cmd_i ret_cmd;
	ret_cmd.cmd=NULL;
	ret_cmd.blob_ret=NULL;
	ret_cmd.cmd=sqlite3_mprintf("insert into %q (","card_inf");
	char *sql_val=sqlite3_mprintf("values(");
	int i=0;
	int first=1;
	for(i=0;i<data_items;i++)
	{
		switch(i)
		{
		case 0:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","dev_no");
			sql_val=sqlite3_mprintf("%q%q%d",sql_val,first?"":",",*(int *)card_data->dev_no);
			break;
		case 1:
		{
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","cardnumber");
			sql_val=sqlite3_mprintf("%q%q?",sql_val,first?"":",");
		}
			break;
		case 2:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","init_date");
			sql_val=sqlite3_mprintf("%q%q?",sql_val,first?"":",");
			break;
		case 3:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","exprirydate");
			sql_val=sqlite3_mprintf("%q%q?",sql_val,first?"":",");
			break;
		case 4:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","blacklist");
			sql_val=sqlite3_mprintf("%q%q%d",sql_val,first?"":",",card_data->blacklist);
			break;
		case 5:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","reserved0");
			sql_val=sqlite3_mprintf("%q%q%d",sql_val,first?"":",",card_data->reserved0);
			break;
		case 6:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","card_type");
			sql_val=sqlite3_mprintf("%q%q%d",sql_val,first?"":",",card_data->card_type);
			break;
		case 7:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","reserved1");
			sql_val=sqlite3_mprintf("%q%q%d",sql_val,first?"":",",card_data->reserved1);
			break;
		case 8:
			ret_cmd.cmd=sqlite3_mprintf("%q%q%q",ret_cmd.cmd,first?"":",","reserved2");
			sql_val=sqlite3_mprintf("%q%q%d",sql_val,first?"":",",card_data->reserved2);
			break;
		}
		first=0;
	}
	sql_val=sqlite3_mprintf("%q)",sql_val);
	ret_cmd.cmd=sqlite3_mprintf("%q %q %q",ret_cmd.cmd,")",sql_val);
	if(sql_val!=NULL)
	{
		sqlite3_free(sql_val);
	}
	sqlite3_stmt *stat;
    //sql语句解析(编译成字符码)
	rc = sqlite3_prepare_v2(db_h->db, ret_cmd.cmd, -1, &stat,NULL);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_prepare err:%s\n",sqlite3_errmsg(db_h->db));
	}
    //插入二进制数据
	rc=sqlite3_bind_blob(stat,1,card_data->cardnumber,sizeof(card_data->cardnumber), NULL);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_bind_text2 err:%s\n",sqlite3_errmsg(db_h->db));
	}
	rc=sqlite3_bind_blob(stat,2,(char*)card_data->init_date,sizeof(card_data->init_date), NULL);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_bind_text3 err:%s\n",sqlite3_errmsg(db_h->db));
	}
	rc=sqlite3_bind_blob(stat,3,(char*)card_data->exprirydate,sizeof(card_data->exprirydate), NULL);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_bind_text4 err:%s\n",sqlite3_errmsg(db_h->db));
	}
    //将sql语句写入数据库
	rc = sqlite3_step(stat);
	if(rc!=SQLITE_DONE)
	{
		printf("sqlite3_step err:%s\n",sqlite3_errmsg(db_h->db));
	}
    //重复使用 sqlite3_stmt 结构
	sqlite3_reset(stat);
    //释放分配的sqlite3_stmt结构
	rc = sqlite3_finalize(stat);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_finalize err:%s\n",sqlite3_errmsg(db_h->db));
	}
	if(ret_cmd.cmd!=NULL)
	{
		sqlite3_free(ret_cmd.cmd);
	}

	int x=0;
	printf("\n#########cardnumber:");
	for(x=0;x<8;x++)
	{
		printf("0x%02x ",card_data->cardnumber[x]);
	}
	printf("\n#########exprirydate:");

	printf("\n#########init_date:");
	for(x=0;x<6;x++)
	{
		printf("0x%02x ",card_data->init_date[x]);
	}
	printf("\n#########exprirydate:");
	for(x=0;x<6;x++)
	{
		printf("0x%02x ",card_data->exprirydate[x]);


	}
	printf("\n");


	return rc;
}


void * do_load_door_card_number_8byte_inf(uint32_t *bytes)
{
	char *ret_buf=NULL;
	int rc = 0;
	query_struct_g *query_msg_card=alloc_query_struct();
	add_order_pri(query_msg_card,0,ORDER_DESC);//需添加order条件和列条件匹配情况
	query_cmd_g resault_card;
	build_query_cmd(&lg_dcard_column[0],"card_inf",NULL,NULL,&resault_card);
/*	if(resault_card.query_cmd!=NULL)
	{
		printf("resault.query_cmd:%s\n",resault_card.query_cmd);
	}*/
	db_handle_t db_h;
	db_h.db=lg_db_handle;
	dcard_list_t *card_ret=query_data_from_table(&db_h,&resault_card,&rc,(void *)&card_number_8byte_call_back_fun);
	if(card_ret!=NULL)
	{
		ret_buf=(char *)malloc(rc*sizeof(pdoor_card2_t)+200);
		if(ret_buf!=NULL)
		{
			char *ptr=ret_buf;
			ptr=ptr+4;
			memcpy(ptr,(uint8_t *)&rc, sizeof(rc));
			ptr=ptr+4;
			dcard_list_t *q_cur=card_ret;
			while(q_cur!=NULL)
			{
				memcpy(ptr,q_cur->dev_no, sizeof(pdoor_card2_t));
				ptr=ptr+sizeof(pdoor_card2_t);
				q_cur=q_cur->next;
			}
			*bytes=rc*sizeof(pdoor_card2_t)+4+4;
		}
		else
		{
			*bytes=0;
		}
		free_card_number_8byte_list(card_ret);
	}
	else
	{

		ret_buf=(char *)malloc(200);
		if(ret_buf!=NULL)
		{
			char *ptr=ret_buf;
			ptr=ptr+4;
			int count=0;
			memcpy(ptr,(uint8_t *)&count, sizeof(count));
			ptr=ptr+4;
			*bytes=4+4;
		}
		else
		{
			*bytes=0;
		}
	}
	free_query_struct(query_msg_card);
	return ret_buf;
}


int do_store_door_card_number_8byte_info(const void *const *args)
{
	int ret = -1;
	int count = 0, i = 0;
	db_handle_t db_h;
	db_h.db= lg_db_handle;
#if 0
	uint8_t *buf = (uint8_t *)args;
	uint8_t *dptr = (uint8_t *)buf;

	if(assert_ptr(buf)) {
		ret_data->len = -1;
		return -1;
	}
#else
	db_blob_t *ret_data = (db_blob_t*)args;
	if(assert_ptr(args)) {
		ret_data->len = -1;
		return -1;
	}
	uint8_t *buf = (uint8_t *)ret_data->data;
	uint8_t *dptr = ret_data->data;
#endif
	uint8_t cmd=buf[1];
	switch(cmd)
	{
	case 0:
	{
		//ret = db_routine_begin_card(db_h.db, IPC_MUTEX_UNBLOCK);
		ret = db_routine_begin(db_h.db, IPC_MUTEX_UNBLOCK);
		
		if(ret < 0) {
			ret_data->len = -1;
			free((uint8_t*)buf);
			return ret;
		}

		dptr += 4;
		ret = delete_tbl_data_card(tbl_dcard_inf, lg_dcard_inf);
		dbg_inf("store door's cards infomation\n");
		memcpy(&count, dptr, sizeof(count));
		dptr += 4;
		for(i=0; i<count && dptr; i++) {
			ret= insert_card_number_8byte_2_sqlite(&db_h,(pdoor_card2_t*)dptr,9);
			dptr += sizeof(pdoor_card2_t);
		}
		ret = db_routine_submit(db_h.db, ret);
		//pthread_mutex_unlock(&lg_db_busy);
		free((uint8_t*)buf);

		if(ret >= 0) {
			manage_door_card_index(0);
			manage_door_card_index(1);
			dbg_inf("store door card info success\n");
		} else {
			dbg_err("store door card info\n");
		}
	}
		break;
	case 1:
	{

		ret = db_routine_begin(db_h.db, IPC_MUTEX_UNBLOCK);
		if(ret < 0) {
			ret_data->len = -1;
			free((uint8_t*)buf);
			return ret;
		}
		dptr += 4;
		dbg_inf("insert door's cards infomation\n");
		memcpy(&count, dptr, sizeof(count));
		dptr += 4;
		for(i=0; i<count && dptr; i++) {
			ret= insert_card_number_8byte_2_sqlite(&db_h,(pdoor_card2_t*)dptr,9);
			dptr += sizeof(pdoor_card2_t);
		}
		ret = db_routine_submit(db_h.db, ret);
		free((uint8_t*)buf);
		if(ret >= 0)
		{
			manage_door_card_index(0);
			manage_door_card_index(1);
			dbg_inf("insert door card info success\n");
		}
		else
		{
			dbg_err("store door card info\n");
		}
	}
		break;
	case 2:
	{
		ret = db_routine_begin(db_h.db, IPC_MUTEX_UNBLOCK);
		if(ret < 0) {
			ret_data->len = -1;
			free((uint8_t*)buf);
			return ret;
		}
		dptr += 4;
		dbg_inf("delete door's cards by dev_no \n");
		memcpy(&count, dptr, sizeof(count));
		dptr += 4;
		for(i=0; i<count && dptr; i++)
		{
			delete_door_card_list_number_8byte((pdoor_card2_t*)dptr, 0);
			dptr += sizeof(pdoor_card2_t);
		}
		ret = db_routine_submit(db_h.db, ret);
		free((uint8_t*)buf);
		if(ret >= 0)
		{
			manage_door_card_index(0);
			manage_door_card_index(1);
			dbg_inf("delete door card info success\n");
		}
		else
		{
			dbg_err("delete door card info\n");
		}
	}
		break;
	case 3:
	{
		ret = db_routine_begin(db_h.db, IPC_MUTEX_UNBLOCK);
		if(ret < 0) {
			ret_data->len = -1;
			free((uint8_t*)buf);
			return ret;
		}
		dptr += 4;
		dbg_inf("delete door's cards by cardnumber\n");
		memcpy(&count, dptr, sizeof(count));
		dptr += 4;
		for(i=0; i<count && dptr; i++)
		{
			delete_door_card_list_number_8byte((pdoor_card2_t*)dptr, 1);
			dptr += sizeof(pdoor_card2_t);
		}
		ret = db_routine_submit(db_h.db, ret);
		free((uint8_t*)buf);
		if(ret >= 0)
		{
			manage_door_card_index(0);
			manage_door_card_index(1);
			dbg_inf("delete door's cards by cardnumber success\n");
		}
		else
		{
			dbg_err("delete door's cards by cardnumber\n");
		}
	}
		break;
	case 4:
	{
		ret = db_routine_begin(db_h.db, IPC_MUTEX_UNBLOCK);
		if(ret < 0) {
			ret_data->len = -1;
			free((uint8_t*)buf);
			return ret;
		}
		dptr += 4;
		dbg_inf("delete door's cards by blacklist \n");
		memcpy(&count, dptr, sizeof(count));
		dptr += 4;
		for(i=0; i<count && dptr; i++)
		{
			delete_door_card_list_number_8byte((pdoor_card2_t*)dptr, 4);
			dptr += sizeof(pdoor_card2_t);
		}
		ret = db_routine_submit(db_h.db, ret);
		free((uint8_t*)buf);
		if(ret >= 0)
		{
			manage_door_card_index(0);
			manage_door_card_index(1);
			dbg_inf("delete door's cards by blacklist success\n");
		}
		else
		{
			dbg_err("delete door's cards by blacklist\n");
		}
	}
		break;
	case 5:
	{
		ret = db_routine_begin(db_h.db, IPC_MUTEX_UNBLOCK);
		if(ret < 0) {
			ret_data->len = -1;
			free((uint8_t*)buf);
			return ret;
		}
		dptr += 4;
		dbg_inf("delete door's cards by exprirydate\n");
		memcpy(&count, dptr, sizeof(count));
		dptr += 4;
		for(i=0; i<count && dptr; i++)
		{
			delete_door_card_list_number_8byte((pdoor_card2_t*)dptr, 3);
			dptr += sizeof(pdoor_card2_t);
		}
		ret = db_routine_submit(db_h.db, ret);
		free((uint8_t*)buf);
		if(ret >= 0)
		{
			manage_door_card_index(0);
			manage_door_card_index(1);
			dbg_inf("delete door's cards by exprirydate success \n");
		}
		else
		{
			dbg_err("delete door's cards by exprirydate \n");
		}
	}
		break;
	default:
		break;
	}

	return ret;
}


/*

int do_store_door_card_number_8byte_info(const void *const *args)
{
	int ret = 0;
	int count = 0, i = 0;
	sqlite3 *db = lg_db_handle;
#if 0
	uint8_t *buf = (uint8_t *)args;
	uint8_t *dptr = (uint8_t *)buf;

	if(assert_ptr(buf)) {
		ret_data->len = -1;
		return -1;
	}
#else
	db_blob_t *ret_data = (db_blob_t*)args;
	if(assert_ptr(args)) {
		ret_data->len = -1;
		return -1;
	}
	uint8_t *buf = (uint8_t *)ret_data->data;
	uint8_t *dptr = ret_data->data;
#endif
	ret = db_routine_begin_card(db, IPC_MUTEX_UNBLOCK);
	if(ret < 0) {
		ret_data->len = -1;
		free((uint8_t*)buf);
		return ret;
	}
	printf("################ret=%d\n",ret);
	dptr += 2;
	ret = delete_tbl_data_card(tbl_dcard_inf, lg_dcard_inf);
	dbg_inf("store door's cards infomation\n");
	memcpy(&count, dptr, sizeof(count));
	dptr += 4;

	for(i=0; i<count && dptr; i++) {
		ret = insert_door_card2_data((pdoor_card2_t*)dptr);
		dptr += sizeof(pdoor_card2_t);
	}
	ret = db_routine_submit(db, ret);
	free((uint8_t*)buf);

	if(ret >= 0) {
		manage_door_card_index(0);
		manage_door_card_index(1);
		dbg_inf("store door card info success\n");
	} else {
		dbg_err("store door card info\n");
	}

	return ret;
}
*/

int store_door_card_number_8byte_info(const int num, const int len)
{
	int bytes = 0;
	pthread_t dsdci_td;
	db_blob_t ret_data = {NULL, 0};
	uint8_t *tmp_buf = malloc(len+1024);
	if(assert_ptr(tmp_buf)) {
		perror("malloc");
		return -1;
	}

	bytes = get_list_data(num, tmp_buf);
	if(bytes <= 0) {
		dbg_err("get list data failed, num=%d\n", num);
		free(tmp_buf);
		return -1;
	}
	printf("g_sys_cfg.dev_no:%02x %02x| %02x  %02x \n",dev_cfg.my_code[0],dev_cfg.my_code[1],*(tmp_buf+2),*(tmp_buf+3));
	if(memcmp(tmp_buf+2, dev_cfg.my_code, 2)) {
		dbg_err("not for me\n");
		return -1;
	}
	ret_data.data = tmp_buf;
	ret_data.len = 0;

	
	print_n_byte(tmp_buf, MIN(80, bytes));
	pthread_create(&dsdci_td, NULL, (void*)&do_store_door_card_number_8byte_info, (void*)&ret_data);
	pthread_detach(dsdci_td);
	usleep(100*1000);
	dbg_inf("ret_data.len = %d\n", ret_data.len);
	return ret_data.len;
}

/**********************************************************************************
*Function name	: DB_FILL_BLOB
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int db_fill_blob_card(sql_blob_t *const src, uint8_t *src_data, int src_len)
{
	if(assert_ptr(src) || assert_ptr(src_data)) {
		return -1;
	}

	src->data = src_data;
	src->len = src_len;
	
	return 0;
}

/**********************************************************************************
*Function name	: DELETE_TBL_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int delete_tbl_data_card(const char *const tbl_name,  const sql_column_t *const column)
{
	int rc = 0;
	sqlite3 *db = lg_db_handle;
	sql_cmd_t bdelete = {0, };
	db_blob_t data = {0, };
	int src_data = 0;

	if(assert_ptr(db) || assert_ptr(tbl_name) || assert_ptr(column)) {
		return -1;
	}

	//dbg_inf("delete table data\n");
	rc = db_fill_blob_card((sql_blob_t *)&data, (uint8_t*)&src_data, sizeof(src_data));

	rc = db_sql_init(db, &bdelete, DDELETE, tbl_name);
	bdelete.condition = CNDTN_NULL;

	rc = db_build_sql_stmt(&bdelete, 0, &data, column);
	if(DB_OK != rc) {
		debug_err("db build sql\n");
		return rc;
	}
	
	//dbg_inf("delete,sql=%s\n", bdelete.sql);
	rc = db_exec_state(db, bdelete.state, TRUE);
	if(DB_OK != rc && SQLITE_DONE != rc) {
		debug_err("db exec sql,rc=%d,%s\n", rc, sqlite3_errmsg(db));
		return rc;
	} else {
		rc = DB_OK;
	}

	return rc;
}


int sqlite_door_card_init()
{
	int rc = db_open_file("door_card.db", &lg_db_handle);
	if(rc < 0) {
		printf("open file %s\n", "door_card.db");
		return rc;
	}
	sqlite_data_init(lg_db_handle);
	return 0;
}

void sqlite_door_card_exit(void)
{
	sql_data_exit();
}

