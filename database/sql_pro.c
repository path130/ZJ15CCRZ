/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: sql_pro.c
* Author				: Ritchie
* Version				: V1.0.0
* Date				: 2012年9月24日
* Description			: 
* Modify by			: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>
#include <time.h>

#include "my_types.h"
//#define	DEBUG_NONE	
#include "my_debug.h"
	
#include "common.h"
#include "sqlite_ui.h"
#include "sql_pro.h"
#include "data_com_ui.h"
#include "sqlite_data.h"
#include "cmd_def.h"

#include "sqlite_data.h"
#include "ipc_ctrl.h"
#include "global_data.h"


#define	SQL_DB_DM365				"sql_ok.db"
#define	SQL_TBL_PARA				"tbl_para"
#define	SQLITE_DB					"mysql_admin.db"

static sqlite3 *lg_db_handle = NULL;
static sql_column_t const sys_para_db[] = {
	{"ID", 		"INTEGER PRIMARY KEY"},
	{"dev_no", 		SQLINT},
	{"dev_ip",    	SQLINT},
	{"level",      	SQLINT},
	{"fun",     	SQLINT},
	{"note",     	SQLTEXT},
	{NULL,	NULL},
};


static const char tbl_para[] = {SQL_TBL_PARA};

/**********************************************************************************
*Function name	: FREE_PARA_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int free_para_data(para_data_t *const fptr)
{
	if(NULL == fptr) {
		debug_err("null input\n");
		return -1;
	}

	para_data_t *cur = fptr;

	for(; cur!=NULL; ) {
		//debug_info("free,cur=%08X\n", (int)cur);
		free(cur);
		cur=cur->next;
	}

	return 0;
}
/**********************************************************************************
*Function name	: QUERY_PARA_CALLBACK
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int query_para_callback(uint8_t * dest ,sqlite3_stmt **ptr_state)
{
	sqlite3_stmt *state = *ptr_state;

	if(NULL == dest || NULL == state) {
		debug_err("query para callback\n");
		return -1;
	}

	sql_cmd_t *alarm_dest = (sql_cmd_t *)dest;
	para_data_t *head = NULL, *priv = NULL, *list = NULL;
	int count = 0, rc = -1;
	int *pdata = NULL;

	int i = 0, j = 0;

	head = list = (para_data_t*)malloc(sizeof(para_data_t));
	
	if(NULL == list) {
		debug_err("can not get memory\n");
		return -1;
	} else {
		head->last = NULL;
		priv = head;
		head->next = NULL;
	}
	
	do {
		priv = list;
		list = (para_data_t*)malloc(sizeof(para_data_t));
		if(NULL == list) {
			count = -1;
		} else {
			list->next = NULL;
			list->last = priv;
			priv->next = list;
		}

		for(i=0,j=-1,pdata=(int*)list; i<6; i++,pdata++) {
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
		free_para_data(head->next);
	}
	free(head);
	
	return count;
}


/**********************************************************************************
*Function name	: MODIFY_ID
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int modify_id(int mid, const unsigned char *const guid)
{	
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 1;

	sqlite3 *db = lg_db_handle;
	
	db_sql_init(db, &bquery, DUPDATE, tbl_para);
	rc = db_build_sql_stmt(&bquery, col_no, &data, sys_para_db);
	return rc;
}

/**********************************************************************************
*Function name	: QUERY_BY_MID
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int query_by_mid(para_data_t *qbm)
{
	int mid = 0;

	if(assert_ptr(qbm)) {
		return -1;
	}
	mid = qbm->mkey;

	sqlite3_stmt *state = NULL;
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t bquery = {0, };
	int col_no = 0;
	para_data_t tmp_data = {0, };

	sqlite3 *db = lg_db_handle;
	data.data = (uint8_t *)&mid;
	data.len = sizeof(mid);

	
	db_sql_init(db, &bquery, DQUERY, tbl_para);
	
	bquery.state = state;
	bquery.dest = (uint8_t *)&tmp_data;
	bquery.deal_fun = query_para_callback;
	bquery.condition = EQUALTO;

	rc = db_build_sql_stmt(&bquery, col_no, &data, sys_para_db);
	if(DB_OK != rc) {
		debug_err("query,build sql statement failed\n");
		return -1;
	}

	rc = db_state_query(db, &bquery);
	if(rc > 0) {
		memcpy(qbm, bquery.dest, sizeof(para_data_t));
	} else {
		debug_err("query data failed,rc=%d(%s)\n", rc, DBERR_MSG(db));
		return rc;
	}

	return 0;
}

/**********************************************************************************
*Function name	: INSERT_TBL_PARA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int insert_tbl_para(para_data_t *para)
{
	int rc = 0;
	db_blob_t data = {0, };
	sql_cmd_t binsert = {0, };
	int col_no = 1;
	int src_data = 0;

	if(NULL == para) {
		debug_err("null input\n");
		return -1;
	}
	sqlite3 *db = lg_db_handle;

	debug_info("insert tbl para\n");

	db_sql_init(db, &binsert, DINSERT, tbl_para);
	
	data.data = (uint8_t*)&src_data;
	data.len = sizeof(int);

	/* sid */
	src_data = para->sid;
	col_no = 1;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, sys_para_db);
	/* val */ 
	src_data = para->val;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, sys_para_db);
	/* date */
	src_data = para->date;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, sys_para_db);
	/* time */
	src_data = para->time;
	rc = db_build_sql_stmt(&binsert, col_no++, &data, sys_para_db);

	if(DB_OK == rc) {
		db_free_cach(db, 0);
		rc = db_exec_state(db, binsert.state, TRUE);
	}
	
	return rc;
}


/**********************************************************************************
*Function name	: SQL_INIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sql_init(void)
{
	sqlite3 *db = NULL;
	int rc = 0; 
	printf("ruan++++++++++++++++\n");	
	sqlite3_initialize();	
	sqlite3_soft_heap_limit64(DB_HEAP_USE_LIMIT);
	sqlite3_enable_shared_cache(TRUE);

	rc = db_ctrl_init(1);
	if(rc < 0) {
		debug_err("db ctrl init failed\n");
		return -1;
	}	
	//prefix_signal_handler(exit_by_signal);

	rc = sqlite3_open(SQL_DB_DM365, &db);
	debug_info("open database: %s\n", SQL_DB_DM365);		
	if(SQLITE_OK != rc) {
		debug_err("Can't open database: %s\n", DBERR_MSG(db));
		sqlite3_close(db);
		return (-1);
	} else {
		rc = db_create_tbl(db, tbl_para, sys_para_db, FALSE);

		if(SQLITE_DONE == rc || SQLITE_OK == rc) {
			para_data_t snd_dflt = {0, };
			lg_db_handle = db;
			insert_tbl_para(&snd_dflt);
			snd_dflt.sid = 86;
			snd_dflt.val = 0x8C;
			snd_dflt.date = 0x2B;
			snd_dflt.time = 0x2;
			insert_tbl_para(&snd_dflt);
			lg_db_handle = NULL;
		}
	}
	
	if(SQLITE_OK == rc) {
		debug_info("create ok\n");
		rc = SUCCESS;
	}
	
	lg_db_handle = db;
	sqlite_data_init(db);

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
void sql_exit(void)
{	
	int rc = 0;
	if(NULL != lg_db_handle) {
		rc = sqlite3_close(lg_db_handle);
		if(SQLITE_OK != rc) {
			debug_err("rc=%d,sql close failed.%s\n", rc, DBERR_MSG(lg_db_handle));
		}
		lg_db_handle = NULL;
	}
	rc = sqlite3_shutdown();
	//debug_info("shutdown, rc=%d\n", rc);
	db_ctrl_exit();
}


/**********************************************************************************
*Function name	: get_contacts_data
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int get_contacts_data(net_addr_msg *huge_msg, uint8_t **dst_data)
{
	int data_len = 0;
	uint net_cmd = 0, list_num = 0, num = 0;
	
	net_addr_msg_split(&net_cmd, &list_num, &num, &data_len, huge_msg);
	if(data_len < 4 || data_len > 1024*300) {
		debug_err("mem data too less or too much,data_len=%d\n", data_len);
		return -1;
	}
	int ret = 0, list_count = 0;
	uint8_t *huge_buf = (uint8_t *)malloc(sizeof(uint8_t) * data_len);
	if(NULL == huge_buf) {
		debug_err("failed to get memory\n");
		return -1;
	}
	
	debug_info("get list data list_num=%d\n", list_num);
	ret = get_list_data(list_num, huge_buf);
	list_count = huge_buf[4] | (huge_buf[5] << 8);

	debug_info("ret=%d, len=%d\n", ret, data_len);			
	print_n_byte(huge_buf, data_len);
	debug_info("list_count=%d:\n", list_count);

#if 0
	int eid = 0, grade = 0, epid = 0, edev_no = 0;
	char ename[37] = {0,};
	uint8_t *ptr = &huge_buf[6];

	for(i=0; ptr && i<list_count; i++) {
		eid = (*ptr)|(ptr[1]<<8);
		grade = *(ptr+2);
		epid = *(ptr+3)|(ptr[4]<<8);
		memcpy(ename, (char*)(ptr+5), sizeof(ename)-1);
		memcpy((uint8_t*)&edev_no, ptr+41, sizeof(int));

		
		debug_print("eid=%.3d, grade=%d, epid=%d, dev_no=%08X\n", eid,\
			grade, epid, edev_no);
		ename[sizeof(ename)-1] = '\n';
		debug_print("name:%s\n", ename);
		ptr += 45;
	}
	free(huge_buf);
	return 0;
#else
	*dst_data = huge_buf;
	return list_count;
#endif
}



/**********************************************************************************
*Function name	: get_camera_list_data
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int get_camera_list_data(net_addr_msg *huge_msg, uint8_t **dst_data)
{
	int data_len = 0;
	uint net_cmd = 0, list_num = 0, num = 0;
	
	net_addr_msg_split(&net_cmd, &list_num, &num, &data_len, huge_msg);
	if(data_len < 4 || data_len > 1024*300) {
		debug_err("mem data too less or too much,data_len=%d\n", data_len);
		return -1;
	}
	int ret = 0, list_count = 0;
	uint8_t *huge_buf = (uint8_t *)malloc(sizeof(uint8_t) * data_len);
	if(NULL == huge_buf) {
		debug_err("failed to get memory\n");
		return -1;
	}
	
	debug_info("get list data list_num=%d\n", list_num);
	ret = get_list_data(list_num, huge_buf);
	list_count = huge_buf[4] | (huge_buf[5] << 8);

	debug_info("ret=%d, len=%d\n", ret, data_len);			
	print_n_byte(huge_buf, data_len);
	debug_info("list_count=%d:\n", list_count);

	*dst_data = huge_buf;
	return list_count;
}

/**********************************************************************************
*Function name	: contacts_store
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int contacts_store(net_addr_msg *huge_msg)
{
	int ret = 0, i = 0, list_count = 0;
	uint8_t *hbuf = NULL, *ptr = NULL;
	pack_contact_t address_book = {0,};
	sqlite3 * db = lg_db_handle;

	if(assert_ptr(huge_msg)) {
		return -1;
	}
	list_count = ret = get_contacts_data(huge_msg, &hbuf);

	if(ret < 0 || NULL == hbuf) {
		debug_err("get contacts data failed\n");
		return ret;
	}
	ptr = hbuf + 6;
	ret = delete_adress_book(0);
	
	debug_info("insert contacts\n");
	db_routine_begin(db, 0);
	for(i=0; ptr && i<list_count; i++) {
		memcpy(&address_book.uid, ptr, 45);
		ret = insert_contacts(&address_book);
		if(ret != 0) {
			debug_err("insert contacts failed,ret=%d\n", ret);
			ret = -1;
			break;
		}
		ptr += 45;
	}
	db_routine_submit(db, ret);
	free(hbuf);

	return ret;	
}

/**********************************************************************************
*Function name	: REQUEST_LINUX_CONTACTS
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int request_linux_contacts(const uint8_t *const dev_no)
{
	uint8_t net_buf[10] = {0,};
	
	memcpy(&net_buf[0], dev_no, 4);
	return send_addr_to_server(RTXLC, net_buf, 6);
	//return send_addr_to_server(LTXLC, net_buf, 6);
}

/**********************************************************************************
*Function name	: test_print_contacts
*Description		: 
*Input para 	: 
*Output para		: 
*Return 		: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void test_print_contacts(const contact_t *const src, int count)
{
	int i = 0;
	contact_t *list = (contact_t *)src;

	if(NULL == list) {
		debug_err("list is null\n");
		return;
	}
	debug_print("id     uid     grade    pid     dev_no ");
	debug_print("\t dev_name \n");
	for(i=0; list && i<count; i++) {
		debug_print("%04d   %04d    %04d    %04d    ", \
			list->id,list->uid,list->grade,list->pid);
		debug_print("%08X \t", list->dev_no);
		debug_print("%s\n", list->dev_name);
		list = list->next;
	}
}

/**********************************************************************************
*Function name	: READ_ADDRESS_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int read_address_data_test(void)
{
	contact_t add_book = {0, };
	int rc = 0;
	
	rc = query_address_book(&add_book, NULL, 0);
	if(rc > 0) {
		test_print_contacts(add_book.next, rc);
		free_query_list(add_book.next, sizeof(contact_t));
	}

	return rc;
}

/**********************************************************************************
*Function name	: REQUEST_CAMERAS_INF
*Description		: request for ip camera list
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int request_cameras_inf(const uint8_t *const dev_no)
{
	uint8_t net_buf[10] = {0,};
	
	memcpy(&net_buf[0], dev_no, 4);
	return send_addr_to_server(GNCPC, net_buf, 6);
}

/**********************************************************************************
*Function name	: IP_CAMERA_INFO_STORE
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int ip_camera_info_store(net_addr_msg *huge_msg)
{
	int ret = 0, i = 0, list_count = 0;
	uint8_t *hbuf = NULL, *ptr = NULL;
	ip_camera_inf_t camera_inf = {0,};
	sqlite3 * db = lg_db_handle;
	
	list_count = ret = get_camera_list_data(huge_msg, &hbuf);

	if(ret < 0 || NULL == hbuf) {
		debug_err("get contacts data failed\n");
		return ret;
	} else {
		debug_info("get list len=%d\n", ret);
	}
	ptr = hbuf + 6;
	
	db_routine_begin(db, 0);
	for(i=0; ptr && i<list_count; i++) {
		memcpy(&camera_inf.dev_ip, ptr, 30);
		ret = insert_ip_camers(&camera_inf);
		if(ret != 0) {
			debug_err("insert camera info failed,ret=%d\n", ret);
			ret = -1;
			break;
		}
		ptr += 30;
	}
	db_routine_submit(db, ret);
	free(hbuf);

	return ret; 
}

/**********************************************************************************
*Function name	: TEST_PRINT_CAMERS
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void test_print_camers(const ip_camera_inf_t *const src, int count)
{
	int i = 0;
	ip_camera_inf_t *list = (ip_camera_inf_t *)src;

	if(NULL == list) {
		debug_err("list is null\n");
		return;
	}
	debug_print("id     dev_ip     port    channel ");
	debug_print("\tdev_name \n");
	for(i=0; list && i<count; i++) {
		debug_print("%04d   %08X   %04d    %04d  ", \
			list->id,list->dev_ip,list->port,list->channel);
		debug_print("\t%s\n", list->dev_name);
		list = list->next;
	}
}


/**********************************************************************************
*Function name	: load_ip_camera_data
*Description		: should update 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int load_ip_camera_data(void)
{
	ip_camera_inf_t ip_camera = {0, };
	int rc = 0;
	int opt = 0;//0xF0;
	ip_camera.id = 1;
	ip_camera.dev_ip = 0xFC01A8C0;
	memcpy(ip_camera.dev_name, "a1", 2);
	rc = query_ip_camera_inf(opt, &ip_camera);
	if(rc > 0) {
		test_print_camers(ip_camera.next, rc);
		free_query_list(ip_camera.next, sizeof(ip_camera));
	}

	return rc;
}


#ifdef DOOR_ACCESS_CARD_LIST
#define			NET_DOOR_ACESS_VERSION			1


int ack_net_to_server(int net_cmd, void *buf,int len, int num)
{
	return respond_to_server(net_cmd, buf, len, num);
}
 

/**********************************************************************************
*Function name	: TEST_QUERY_CARD_INF
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int test_query_card_inf(void)
{
	int *pc_data = NULL;
	card_data_t test;
	card_list_sql_t test2;	
	uint64_t t1 = {0}, t2 = {0}, t3 = {0};
	int rc = 0;
	printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");	

	#if 0
	test.dev_no = 0x11010102u;
	//rc = load_ndoor_card_inf(NET_DOOR_ACESS_VERSION, &test.dev_no, (uint8_t*)&pc_data);

	
	test2.data.cardnumber = 0x110000000ull;
	test2.data.dev_no = 0x11010201u;
	
	//isp_time(NULL);
	//rc = query_ndoor_card_list(&test2, 0);
	rc = query_ndoor_card_list(&test2, 1);
	dbg_blue("rc=%d\n", rc);
	
	if(rc > 0) {
		if(assert_ptr(test2.next)) {
			return -1;
		}
		card_data_t *pc_data = &test2.next->data;
		dbg_blue("dev_no=[%0X],cardnumber=[%llX]\n", pc_data->dev_no, pc_data->cardnumber);	
		dbg_blue("init_date\n");
		print_n_byte(pc_data->init_date, 6);
		dbg_blue("exprirydate\n");
		print_n_byte(pc_data->exprirydate, 6);
		dbg_blue("card_type=%0X,card_type=%c\n", pc_data->blacklist, pc_data->card_type);	
		release_query_list(test2);
	}
	// = disp_time(NULL);
	dbg_blue("delta_time = %llu\n", t3 - t2);
	#else
	
	uint64_t tmp_number = 0;
	unsigned char dest[4]={0};	
	uint8_t tty_buff[4] = {0x00 , 0x56  ,0x21,  0x83};
	memcpy((uint8_t*)&tmp_number, tty_buff, 4);
	rc=match_du_card_no(tmp_number,dest);
	printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
	printf("rc=%X\n",rc);

	uint8_t tty_buff1[8] = {0x00,  0x01 , 0xB2 , 0xC4 , 0x00,  0x00 , 0x00 , 0x00};

	memcpy((uint8_t*)&tmp_number, tty_buff1, sizeof(tty_buff1));
	rc=match_du_card_no(tmp_number,dest);

	printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
	printf("rc=%X\n",rc);
	#endif


	return rc;

}

/**********************************************************************************
*Function name	: LOAD_NDOOR_CARD_INF
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int load_ndoor_card_inf(uint8_t version, uint8_t *my_devno, uint8_t **const dbuf)
{
	uint8_t *dptr = (uint8_t *)dbuf;

	int ret = 0, i = 0;
	card_list_sql_t dst_data = {0,}, *tmp = NULL;
	
	if(assert_ptr(my_devno) || assert_ptr(dbuf)) {
		return -1;
	}
	
	ret = query_ndoor_card_list(&dst_data, 2);
	if(ret > 0) {
		dptr = malloc(ret * (sizeof(card_data_t)) + 8);
		if(assert_ptr(dptr)) {
			*dbuf = NULL;
			return -1;
		}
		*dbuf = dptr;

		*dptr = version;//version
		dptr ++;

		*dptr = 0;//reserved
		dptr ++;
		
		memcpy(dptr, my_devno, 2);//my building_no
		dptr += 2;

		bzero(dptr, 4);
		memcpy(dptr, (uint8_t *)&ret, sizeof(ret));//count
		dptr += 4;

		tmp = dst_data.next;
		for(i=0; i<ret && tmp; i++) {
			dbg_inf("dev_no=%08X,number=%llu  [%llX]\n", \
					tmp->data.dev_no, tmp->data.cardnumber, tmp->data.cardnumber);
			dbg_inf("init_date=%02X%02X%02X%02X%02X%02X%02X%02X\n", \
					tmp->data.init_date[0],tmp->data.init_date[1]
					,tmp->data.init_date[2],tmp->data.init_date[3]
					,tmp->data.init_date[4],tmp->data.init_date[5]);

			dbg_inf("exprirydate=%02X%02X%02X%02X%02X%02X%02X%02X\n", \
					tmp->data.exprirydate[0],tmp->data.exprirydate[1]
					,tmp->data.exprirydate[2],tmp->data.exprirydate[3]
					,tmp->data.exprirydate[4],tmp->data.exprirydate[5]);
			
			//memcpy(dptr, (uint8_t*)&tmp->data.dev_no, 32);

#if 1
	
     memcpy(&dptr[0+32*i], &(tmp->data.dev_no), 4);
   	 memcpy(&dptr[4+32*i], &(tmp->data.cardnumber), 8);
     memcpy(&dptr[12+32*i], &(tmp->data.init_date), 6);
	 memcpy(&dptr[18+32*i], &(tmp->data.exprirydate), 6);	
	 memcpy(&dptr[24+32*i], &(tmp->data.blacklist), 1);
	 memcpy(&dptr[26+32*i], &(tmp->data.card_type), 1);	


#endif
		
			tmp = tmp->next;
			//dptr += 32;
		}

		release_query_list(dst_data);
	} else {
		dbg_err("load data failed\n");
		*dbuf = NULL;
		return -1;
	}

	dbg_lo("load door's card data,ret=%d,*dbuf=%08X\n", ret, (size_t)*dbuf);
	print_n_byte(*dbuf, ret * 32 + 2 + 2 + 4);
	printf("+++++++++++++++++++++++++++++++++++\n");
	//print_n_byte(*dptr, ret * 32 + 2 + 2 + 4);
	return (ret * 32 + 8);
}


/**********************************************************************************
*Function name	: DO_STORE_DOOR_CARD_INFO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int do_store_ndoor_card_info(const void *const *args)
{
	int ret = -1, net_num = 0;
	int count = 0, i = 0;
	card_net_cmd_t *net_cmd = NULL;
	uint8_t *head = NULL, *dptr = NULL;

	if(assert_ptr(args)) {
		ack_net_to_server(ANMJKC, &ret, sizeof(ret), net_num); 
		return -1;
	}

		

	net_cmd = (card_net_cmd_t*)args;
	dbg_lo("rsp_number=%d, head=%08X\n", net_cmd->rsp_number, (size_t)net_cmd->head);
	count = net_cmd->count;
	dptr = net_cmd->head;
	dptr += 8;

	dbg_blue("dptr=%08X\n", (size_t)dptr);
	print_n_byte(dptr, 20);
	dbg_inf("version=%d, cmd=%x, dev_no=%04X, count=%d\n", \
			net_cmd->version, net_cmd->cmd, net_cmd->dev_no, net_cmd->count);

	ret = sqldata_routine_begin(IPC_MUTEX_UNBLOCK);
	if(ret < 0) {
		free((uint8_t*)head);
		ack_net_to_server(ANMJKC, &ret, sizeof(ret), net_num); 
		return ret;
	}
	
	switch(net_cmd->version){
	case CARD_NET_VERSION:
		switch(net_cmd->cmd) {
		case DROPB4INSERT:
		//default:
			ret = man_ndoor_unit_tbl(DROPB4INSERT);
			break;

		case ADDCARDS:
			ret = man_ndoor_unit_tbl(ADDCARDS);
			break;

		case DLT_BY_DEVNO:
			ret = man_ndoor_unit_tbl(DLT_BY_DEVNO);
			break;

		case DLT_BY_CARD:
			ret = man_ndoor_unit_tbl(DLT_BY_CARD);
			break;

		case DLT_BY_BLACK:
			ret = man_ndoor_unit_tbl(DLT_BY_BLACK);
			break;

		case DLT_BY_EXPIRY:
			ret = man_ndoor_unit_tbl(DLT_BY_EXPIRY);
			break;

		default:
			//ret = man_ndoor_unit_tbl(DROPB4INSERT);
			break;			
		}
		break;
		
	default:
	{
		dbg_err("version[%d] isn't implemented or not supporting anymore\n",net_cmd->version);
		ret = -1;
		free((uint8_t*)head);
		ack_net_to_server(ANMJKC, &ret, sizeof(ret), net_num); 
		ret = sqldata_routine_submit(ret);
		return ret;
	}
	break;
	}

	if(ret < 0) {
		dbg_err("man door's info table failed\n");
		free((uint8_t*)head);
		ack_net_to_server(ANMJKC, &ret, sizeof(ret), net_num);
		ret = sqldata_routine_submit(ret);
		return ret;
	}
	
	for(i=0; i<count && dptr; i++) {
		ret = insert_ndoor_card_data_buf(dptr);
		//ret = insert_ndoor_card_data(dptr);
		
		if(ret < 0) {
			dbg_err("store door card\n");
			break;
		}		
		//dptr += sizeof(card_data_t);
		dptr += 32;
	}
	ret = sqldata_routine_submit(ret);
	free((uint8_t*)head);

	if(ret >= 0) {
		rebuild_ndoor_card_index(0);
		dbg_inf("store door card info success\n");
	} else {
		dbg_err("store door card info\n");
		ack_net_to_server(ANMJKC, &ret, sizeof(ret), net_num); 
	}
	
	return ret;
}


/**********************************************************************************
*Function name	: STORE_NDOOR_CARD_INFO
*Description		: 
*Input para 	: 
*Output para		: 
*Return 		: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int store_ndoor_card_info(const int num, const int len, int opt)
{
#if 0
	#define	TEST_INSERT_MAX		(30000)
	int rc = 0;
	int i = 0, j = 0;
	card_data_t test;
	card_list_sql_t test2;	
	uint64_t t1 = {0}, t2 = {0}, t3 = {0};

#if (TEST_INSERT_MAX < 100)
	for(j = 0; j < TEST_INSERT_MAX; j++)
#endif	
	{
		test.dev_no = 0x1000 + j;
		test.cardnumber = 1000000000000000ull + j ;

		for(i=0; i<sizeof(test.init_date); i++) {
			test.init_date[i] = 'a' + i;
		}
		for(i=0; i<sizeof(test.exprirydate); i++) {
			test.exprirydate[i] = 'A' + i;
		}
		test.blacklist = TRUE;
		test.card_type = 'A';
	}
	
	sqlite3 *db = NULL;
	rc = get_current_db_handle(&db);
	if(assert_ptr(db) || rc < 0) {
		dbg_err("rc=%d\n", rc);
		return -1;
	}

	t1 = disp_time(NULL);
	rc = db_routine_begin((sqlite3 *const )db, TRUE);
	if(rc < 0) {
		return rc;
	}	
	for(j=0; j<TEST_INSERT_MAX; j++) {
		rc = insert_ndoor_card_data(&test);
		if(rc < 0) {
			db_routine_submit(db, rc);
			dbg_err("insert failed\n");
			return rc;
		}
	}
	rc = db_routine_submit(db, rc);
	
	t2 = disp_time(NULL);
	dbg_blue("delta_time = %llu\n", t2 - t1);

	int *pc_data = NULL;
	test.dev_no = 0x1205;
	//rc = load_ndoor_card_inf(NET_DOOR_ACESS_VERSION, &test.dev_no, (uint8_t*)&pc_data);

	
	test2.data.cardnumber = 0x1000000000000000ull;
	test2.data.dev_no = 0x1000;
	//rc = query_ndoor_card_list(&test2, 0);
	rc = query_ndoor_card_list(&test2, 1);
	dbg_blue("rc=%d\n", rc);
	
	if(rc > 0) {
		if(assert_ptr(test2.next)) {
			return -1;
		}
		card_data_t *pc_data = &test2.next->data;
		dbg_blue("dev_no=[%0X],cardnumber=[%llX]\n", pc_data->dev_no, pc_data->cardnumber);	
		dbg_blue("init_date\n");
		print_n_byte(pc_data->init_date, 6);
		dbg_blue("exprirydate\n");
		print_n_byte(pc_data->exprirydate, 6);
		dbg_blue("card_type=%0X,card_type=%c\n", pc_data->blacklist, pc_data->card_type);	
		release_query_list(test2);
	}
	t3 = disp_time(NULL);
	dbg_blue("delta_time = %llu\n", t3 - t2);

#else
	int bytes = 0;
	pthread_t dsdci_td; 
	printf("len=%d,num=%d\n",len,num);
	
	uint8_t *tmp_buf = (uint8_t *)do_malloc(len);
	if(assert_ptr(tmp_buf)) {
		perror("malloc");
		return -1;
	}
#if 10	
	bytes = get_list_data(num, tmp_buf);
	if(bytes <= 0) {
		dbg_err("get list data failed, num=%d\n", num);
		free(tmp_buf);
		return -1; 
	}

	if(memcmp(tmp_buf+2, gData.DoorNo, 2)) {
		dbg_err("tmp_buf[0~1]=%02X%02X\n", tmp_buf[2], tmp_buf[3]);
		dbg_err("DoorNo[0~1]=%02X%02X\n", gData.DoorNo[2], gData.DoorNo[3]);
		dbg_err("not for me\n");
		//return -1;
	}

#else
	int i = 0;
	tmp_buf[i++] = NET_DOOR_ACESS_VERSION;
	tmp_buf[i++] = ADDCARDS;
	tmp_buf[i++] = 0x05;
	tmp_buf[i++] = 0x12;

	tmp_buf[i++] = 0x1;
	tmp_buf[i++] = 0x0;
	tmp_buf[i++] = 0x0;
	tmp_buf[i++] = 0x0;

	//dev_no
	tmp_buf[i++] = 0x02;
	tmp_buf[i++] = 0x01;
	tmp_buf[i++] = 0x05;
	tmp_buf[i++] = 0x12;

	//card_number;
	uint64_t card_number = 1234567890llu;
	memcpy(&tmp_buf[i],&card_number, sizeof(card_number));
	
	print_n_byte(tmp_buf, 20);
#endif

	card_net_cmd_t card_cmd = {0,};

	card_cmd.rsp_number = num;
	card_cmd.head = tmp_buf;
	#if 0
    int i;
	for(i=0;i<len;i++)
		printf("tmp_buf[%d]=%02x\t",i,tmp_buf[i]);
		#endif

	memcpy(&card_cmd.version, tmp_buf, 8);
	
	dbg_lo("bytes=%d, rsp_number=%d, head=%08X\n", bytes, num, (size_t)tmp_buf);
	dbg_inf("version=%d, cmd=%x, dev_no=%04X, count=%d\n", \
			card_cmd.version, card_cmd.cmd, card_cmd.dev_no, card_cmd.count);
	print_n_byte(tmp_buf, MIN(80, bytes));
	//pthread_create(&dsdci_td, NULL, (void*)&do_store_ndoor_card_info, (void*)&card_cmd);
	pthread_create(&dsdci_td, NULL, (void*)&do_store_door_card_number_8byte_info, (void*)&card_cmd);
	
	pthread_detach(dsdci_td);
	usleep(100*1000);
	
	return 0;
#endif	
}


/**********************************************************************************
*Function name	: TIME_COMPARATION
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int time_comparation(int smonth, size_t dmonth, size_t src_sec, size_t dst_sec)
{
	if(smonth > dmonth) {
		dbg_blue("input month=%d, cureent month=%d\n", smonth , dmonth);
		return 1;
	}
	else if(smonth < dmonth) {
		dbg_err("month expired,smonth=%d,dmonth=%d\n", smonth, dmonth);
		return 0;
	}
	else {
		if(src_sec > dst_sec) {
			dbg_blue("src_sec=%d, while dst_sec=%d\n", src_sec, dst_sec);
			return 1;
		} else {
			dbg_blue("src_sec=%d, while dst_sec=%d\n", src_sec, dst_sec);
			return 0;
		}
	}

}

/**********************************************************************************
*Function name	: CHECK_WITH_LOCAL_TIME
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		: 
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int check_with_local_time(uint8_t const date_time[6], const int true_ret)
{
	size_t i_year = 0, i_mon = 0, i_day = 0, i_sec = 0;
	size_t l_mon = 0, l_sec = 0;
	time_t timep; 
	int ret = 0;
	struct tm *p; 
	
	if(assert_ptr(date_time)) {
		return -1;
	}

	i_year = (date_time[0])+ 2000;
	i_mon = (date_time[1]) + i_year*12;
	i_day = (date_time[2]);

	i_sec = date_time[2]*24*3600 + date_time[3]*3600 + date_time[4]*60 + date_time[5];
	dbg_lo("i_date=%d, i_mon=%d, i_day=%d\n", i_year, i_mon, i_day);
	
	time(&timep); /*UTC TIME*/ 
	p = localtime(&timep); /*transfer time to struct tm format*/
	dbg_inf("Cureent time:%d/%d/%d, %d:%d:%d\n", 1900 + p->tm_year, 1 +p->tm_mon, p->tm_mday, p->tm_hour,p->tm_min, p->tm_sec); 
	l_sec = p->tm_mday*24*3600 + p->tm_hour*3600 + p->tm_min*60 + p->tm_sec;
	dbg_inf("i_sec=%u, l_sec=%u\n", i_sec, l_sec);
	l_mon = (1900 + p->tm_year)*12 + (1 +p->tm_mon);
	
	if(true_ret) {
		ret = time_comparation(i_mon, l_mon, i_sec, l_sec);
	} else {
		ret = time_comparation(l_mon, i_mon, l_sec, i_sec);
	}
	
	return ret;
}


/**********************************************************************************
*Function name	: MATCH_DU_CARD_NO
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int match_du_card_no(const uint64_t cardnumber, unsigned char room[])
{
	int ret = 0;
	int dev_no;
	card_list_sql_t dst_data = {0,};
	unsigned char i, hex[4];
	
#if 10
	dst_data.data.cardnumber = cardnumber;
	dbg_lo("uint64_t.number = 0x%llX = %llu\n", dst_data.data.cardnumber, cardnumber);
#else
	dbg_lo("uint64_t.number = 0x%X\n", (uint64_t)(card_no[0]<<24));
	dbg_lo("uint32_t.number = 0x%X\n", (uint32_t)(card_no[0]<<24));
	dbg_lo("int32_t.number = 0x%X\n", (int32_t)(card_no[0]<<24));

	dst_data.number = (uint32_t)(card_no[0]<<24);
	dbg_lo("dst_data.number = 0x%llX\n", dst_data.number);
	dst_data.number |= (uint64_t)(card_no[1]<<16);
	dbg_lo("dst_data.number = 0x%llX\n", dst_data.number);
	dst_data.number |= (uint64_t)(card_no[2]<<8);
	dbg_lo("dst_data.number = 0x%llX\n", dst_data.number);
	dst_data.number |= (uint64_t)card_no[3];
	dbg_lo("dst_data.number = 0x%llX\n", dst_data.number);
#endif

	ret = query_ndoor_card_list(&dst_data, 0);
	if(ret <= 0 || assert_ptr(dst_data.next)) {
		dbg_warn("card number doesn't exist\n");
		ret=-2;
	} else {
		card_data_t *data = &dst_data.next->data;
		dbg_blue("OK,dev_no=%08X\n", data->dev_no);	

		memcpy(hex, &data->dev_no, 4);
		for (i = 0; i < 4; i++) {
			//room[i] = (hex[i]/10*16) + (hex[i]%10);
			room[i] = hex[i];
			}
		printf("is_card_unlock_ex room:%02X%02X%02X%02X blacklist %d\n", room[0], room[1], room[2], room[3]);
		
		if(data->blacklist) {
			dbg_err("This card hasn't been blacklisted\n");
			ret = -3;
		} else if(!check_with_local_time(data->init_date, 0)){
			dbg_err("This card hasn't been initiated yet\n");
			ret = -1;
		} else if(!check_with_local_time(data->exprirydate, 1)) {
			dbg_err("This card has already been expired\n");
			ret = -1;
		} else {
		
			ret = 0; ;//可开锁
		
		}
		//dbg_inf("dst_data.next=%08X, &dst_data.next->next=%08X\n", dst_data.next, &dst_data.next->next);
		//dbg_inf("dst_data.next->id=%d, &dst_data.next->last=%08X\n", &dst_data.next->id, &dst_data.next->last);
		//free_query_list(dst_data.next, sizeof(card_list_sql_t)-4);
		release_query_list(dst_data);
			
	}

	return ret;
}


#endif

#ifndef new_card

static pthread_t mem_thread;
static int insert_comut=0;

inline void add_insert_comut(void)
{
	insert_comut++;
}
#if 0
typedef enum MSG_CTRL_MODE_DF{
	MCTRL_NORMAL_MODE,
	MCTRL_CALLING_PRO_MODE,
	MCTRL_CALLING_MODE,
	MCTRL_CALLED_PRO_MODE,
	MCTRL_CALLED_MODE,
	MCTRL_TALKING_MODE,
	MCTRL_MONITOR_PRO_MODE,
	MCTRL_MONITOR_MODE,
	MCTRL_WAIT_LEAVE_MSG,
	MCTRL_LEAVE_MSG,

	MCTRL_FJ_CALLING_PRO_MODE,     // 分机呼叫ING处理
	MCTRL_FJ_CALLING_MODE,         // 分机呼叫ING
	MCTRL_FJ_TALKING_MODE,         // 分机和管理机或则分机对讲ING,外部分机和分机,管理机和分机对讲
	MCTRL_FJ_MONITOR_PRO_MODE,     // 监视处理
	MCTRL_FJ_MONITOR_MODE,         // 监视
	MCTRL_FJ_QUERRY_MODE,          // 分机查询
}mctrl_mode_t;

void free_sqlite_mem(void)
{
	while(1)
	{
		if(MCTRL_NORMAL_MODE == get_current_mode())
		{
			if(insert_comut>50)
			{
				printf("clear cache\n");
				system("echo 1 > /proc/sys/vm/drop_caches");
				system("echo 0 > /proc/sys/vm/drop_caches");
				insert_comut=0;
			}
		}
		else
		{
			sleep(2);
		}

		sleep(5);
	}
}

void start_db_mem_release(void)
{
	pthread_create(&mem_thread,NULL,(void*)&free_sqlite_mem,NULL);
}

#endif
typedef struct
{
	char *column_id;
	char *val;
	int len;
}text_struct_t;


static char* db_sql_mprintf(char *format, ...)
{
	char *sql = NULL, *fsql = NULL;
	va_list argp;

	va_start(argp, format);
	fsql = va_arg(argp, char *);

	va_start(argp, format);
	sql = sqlite3_vmprintf(format, argp);
	va_end( argp );

	if(NULL != fsql) {
		sqlite3_free(fsql);
		fsql = NULL;
	}

	return sql;
}



int datebase_open(const char *filename,db_handle_t **ppDb)
{
	int rc=-1;
	*ppDb=malloc(sizeof(db_handle_t));
	if(*ppDb==NULL)
	{
		perror("malloc ppDb:\n");
		return rc;
	}
	rc = sqlite3_open(filename,&(*ppDb)->db);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr, "%s_ sqlite3_open err:%s !\n",__FUNCTION__,sqlite3_errmsg((*ppDb)->db));
		sqlite3_close((*ppDb)->db);
		if(*ppDb!=NULL){
			free(*ppDb);
		}
		*ppDb=NULL;
		rc=-1;
	}
	else
	{
		rc=0;
	}

	return rc;
}

void datebase_close(db_handle_t *ppDb)
{
	if(ppDb!=NULL)
	{
		ppDb->db=NULL;
		sqlite3_close(ppDb->db);
		free(ppDb);
	}
}


char * sqlite3_vmspf(char *fmt, ...)
{
	va_list argptr;
	char *r_char=NULL;
	va_start(argptr, fmt);
	r_char=sqlite3_vmprintf(fmt, argptr);
	va_end(argptr);
	return r_char;
}


int build_delete_cmd(const column_struct_t * column_array,const char *tabname,query_struct_g*qurey_cmd,\
		query_cmd_g *resault)
{
	if((column_array==NULL)||(resault==NULL))
	{
		fprintf(stderr, "%s:err qurey_cmd or column is NULL!\n",__FUNCTION__);
		return -1;
	}
	resault->query_cmd=NULL;
	int max_row_id=0;
	const column_struct_t *cur=column_array,*c_cur=column_array;
	for(;(NULL!=c_cur->data_type)&&(NULL!=c_cur);)
	{
		max_row_id++;
		c_cur++;
	}
	resault->query_cmd=sqlite3_mprintf("delete from %q ",tabname);

	if(qurey_cmd==NULL)
	{
		return 0;
	}
	column_struct_g *column_cur=qurey_cmd->column_id;

	if(column_cur!=NULL)
	{
		resault->query_cmd=db_sql_mprintf("%q where ",resault->query_cmd);
	}

	while (column_cur!=NULL)
	{
		if(column_cur->last!=NULL)
		{
			if(column_cur->row_pos<max_row_id)
			{
				if(column_cur->relationship[0]!='\0')
				{
					resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,column_cur->relationship);
				}
			}
		}

		if(column_cur->row_pos<max_row_id)
		{
			c_cur=cur+column_cur->row_pos;
			if((strncmp(c_cur->data_type,"BLOB",4)!=0))
			{
				resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,c_cur->column_id);
				if(column_cur->operator[0]!='\0')
				{
					resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,column_cur->operator);
				}

				if(column_cur->data!=NULL)
				{
					if(strncmp(c_cur->data_type,"INTEGER",7)==0)
					{
						resault->query_cmd=db_sql_mprintf("%q%d",resault->query_cmd,*(int *)column_cur->data);
					}
					else if(strncmp(c_cur->data_type,"FLOAT",5)==0)
					{
						resault->query_cmd=db_sql_mprintf("%q%f",resault->query_cmd,*(float *)column_cur->data);
					}
					else if(strncmp(c_cur->data_type,"REAL",4)==0)
					{
						resault->query_cmd=db_sql_mprintf("%q%g",resault->query_cmd,*(double *)column_cur->data);
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				fprintf(stderr, "%s:BLOB is not support!\n",__FUNCTION__);
			}
		}

		column_cur=column_cur->next;
	}
	return 0;
}


int delete_data_form_table(db_handle_t *db_h, query_cmd_g *query_cmd)
{
	char *err = NULL;
	if((query_cmd==NULL)||(query_cmd->query_cmd==NULL)||(db_h==NULL))
	{
		fprintf(stderr, "%s:err !\n",__FUNCTION__);
		return -1;
	}
	printf("delete_msg_form_table:%s\n", query_cmd->query_cmd);
	int rc = sqlite3_exec(db_h->db, query_cmd->query_cmd ,0 ,0, &err);
	if(rc != SQLITE_OK)
	{
		printf("%d  %s\n", rc,err);
		sqlite3_free(err);
		return -1;
	}
	return rc;
}



int update_data_to_table(db_handle_t *db_h, query_cmd_g *query_cmd)
{
	char *err = NULL;
	printf("update cmd:%s\n", query_cmd->query_cmd);
	int rc = sqlite3_exec(db_h->db, query_cmd->query_cmd ,0 ,0, &err);
	if(rc != SQLITE_OK) {
		printf("%s\n", err);
		sqlite3_free(err);
		return -1;
	}
	return 0;
}

int build_update_cmd(const column_struct_t * column_array,char *tabname,update_struct_i *update_list,query_cmd_g *resault)
{

	resault->query_cmd=NULL;
	if((update_list==NULL)||(column_array==NULL)||(resault==NULL)||(tabname==NULL)){
		fprintf(stderr, "%s:err qurey_cmd or column is NULL!\n",__FUNCTION__);
		return -1;
	}
	else if(update_list->update_array==NULL)
	{
		fprintf(stderr, "%s:err update_array is NULL!\n",__FUNCTION__);
		return -1;
	}
	int max_row_id=0;
	const column_struct_t *cur=column_array,*c_cur=column_array;
	for(;(NULL!=c_cur->data_type)&&(NULL!=c_cur);)
	{
		max_row_id++;
		c_cur++;
	}
	const column_struct_t *column_ptr=column_array,*column_cur=column_array;
	column_pack_i *col_pack_ptr=update_list->update_array;

	resault->query_cmd=sqlite3_mprintf("update %q set ",tabname);

	int i=0;
	int first=1;
	for(i=0;(i<update_list->list_count);i++)
		{
			if((col_pack_ptr->len>0)&&(col_pack_ptr->row_pos<max_row_id))
			{
				column_cur=column_ptr+col_pack_ptr->row_pos;
				if(strncmp(column_cur->data_type,"BLOB",4)==0)
				{
					continue;
				}
				resault->query_cmd=db_sql_mprintf("%q%q%q",resault->query_cmd,first?"":",",column_cur->column_id);

				if(strncmp(column_cur->data_type,"INTEGER",7)==0)
				{
					resault->query_cmd=db_sql_mprintf("%q=%d",resault->query_cmd,*(int *)col_pack_ptr->data);
				}
				else if(strncmp(column_cur->data_type,"FLOAT",5)==0)
				{
					resault->query_cmd=db_sql_mprintf("%q=%f",resault->query_cmd,*(float *)col_pack_ptr->data);
				}
				else if(strncmp(column_cur->data_type,"REAL",4)==0)
				{
					resault->query_cmd=db_sql_mprintf("%q=%g",resault->query_cmd,*(double *)col_pack_ptr->data);
				}
				else if((strncmp(column_cur->data_type,"TEXT",4)==0))
				{/*
					//sql_val=sqlite3_mprintf("%q%q%q",sql_val,first?"":",",(char *)col_pack_ptr->data);

				*/}
				first=0;
			}
			col_pack_ptr++;
			//printf("all sql:%s \n",ret_cmd.cmd);
		}
	if(	update_list->column_id!=NULL)
	{
		column_struct_g * column_cur=update_list->column_id;
		resault->query_cmd=db_sql_mprintf("%q where ",resault->query_cmd);
		while (column_cur!=NULL)
		{
			if(column_cur->last!=NULL)
			{
				if(column_cur->row_pos<max_row_id)
				{
					if(column_cur->relationship[0]!='\0')
					{
						resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,column_cur->relationship);
					}
				}
			}

			if(column_cur->row_pos<max_row_id)
			{
				c_cur=cur+column_cur->row_pos;
				resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,c_cur->column_id);
				printf("column_cur->row_pos:%d\n",max_row_id<column_cur->row_pos);
				if(column_cur->operator[0]!='\0')
				{
					resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,column_cur->operator);
				}

				if(column_cur->data!=NULL)
				{
					if(strncmp(c_cur->data_type,"INTEGER",7)==0)
					{
						resault->query_cmd=db_sql_mprintf("%q%d",resault->query_cmd,*(int *)column_cur->data);
					}
					else if(strncmp(c_cur->data_type,"FLOAT",5)==0)
					{
						resault->query_cmd=db_sql_mprintf("%q%f",resault->query_cmd,*(float *)column_cur->data);
					}
					/*	else if(strncmp(c_cur->data_type,"TEXT",4)==0)
					{
						text_ptr[text_count]=column_cur;
						text_count++;
						//resault->query_cmd=sqlite3_mprintf("%q%q",resault->query_cmd,(char *)column_cur->data);
					}*/
					else if(strncmp(c_cur->data_type,"REAL",4)==0)
					{
						resault->query_cmd=db_sql_mprintf("%q%g",resault->query_cmd,*(double *)column_cur->data);
					}
				}
			}

			column_cur=column_cur->next;
			if(resault->query_cmd!=NULL)
			{
				printf("update resault.query_cmd:%s\n",resault->query_cmd);
			}
		}
	}
	return 0;
}


update_struct_i *alloc_update_struct(void)
{

	update_struct_i *list=NULL;
	list=(update_struct_i *)malloc(sizeof(update_struct_i));
	if(list==NULL)
	{
		fprintf(stderr, "%s:",__FUNCTION__);
		perror("alloc_query_struct:");
	}
	else
	{
		list->column_id=NULL;
		list->update_array=NULL;
		list->list_count=0;
	}
	return list;

}
void free_update_struct(update_struct_i *update_list)
{
	if(update_list==NULL)
	{
		return;
	}
	column_struct_g *column_cur=update_list->column_id;
	column_struct_g * column_prv;

	while(column_cur!=NULL)
	{
		column_prv=column_cur;
		column_cur=column_cur->next;
		if(column_prv->data!=NULL){
			free(column_prv->data);
		}
		free(column_prv);
	}
	free(update_list);

}

//在未完成update之前update_array的内容不能释放
void add_update_array(update_struct_i*update_list,column_pack_i *update_array,int list_count)
{
	update_list->update_array=update_array;
	update_list->list_count=list_count;
}

int add_update_column_val(update_struct_i*update_list,void *data,int len,int row_pos,\
		char operator[10],char relationship[10])
{

	if((data==NULL)||(update_list==NULL))
	{
		printf("err:data or update_list is NULL!\n");
		return -1;
	}
	column_struct_g*cur_ptr=update_list->column_id,*column_prv;
	if(update_list->column_id==NULL)
	{
		update_list->column_id=malloc(sizeof(column_struct_g));
		if(update_list->column_id==NULL){
			perror("malloc for r_column_id:\n");
			return -1;
		}else
		{
			update_list->column_id->next=NULL;
			cur_ptr=update_list->column_id;
		}
	}
	else
	{
		while((cur_ptr!=NULL)&&(cur_ptr->next!=NULL))
		{
			cur_ptr=cur_ptr->next;
		}
		cur_ptr->next=malloc(sizeof(column_struct_g));
		if(cur_ptr->next==NULL)
		{
			perror("add_column_id:malloc for cur_ptr->next:\n");
			return -1;
		}else
		{
			column_prv=cur_ptr;
			cur_ptr=cur_ptr->next;
			cur_ptr->last=column_prv;
			cur_ptr->next=NULL;
		}
	}

	cur_ptr->data=malloc(len);
	if(cur_ptr->data==NULL)
	{
		goto err;
	}

	memcpy(cur_ptr->data,data,len);
	if(operator!=NULL)
	{
		strncpy(cur_ptr->operator,operator,10);
	}
	else
	{
		cur_ptr->operator[0]='\0';
	}
	if(relationship!=NULL){
		strncpy(cur_ptr->relationship,relationship,10);
	}else
	{
		cur_ptr->relationship[0]='\0';
	}
	cur_ptr->row_pos=row_pos;
	cur_ptr->next=NULL;

	return 0;
err:
	return -1;

}

void free_query_cmd(query_cmd_g *resault)
{
	if(resault==NULL)
	{
		return;
	}
	if(resault->query_cmd!=NULL)
	{
		sqlite3_free(resault->query_cmd);
	}
}


insert_cmd_i  build_insert_cmd(const column_struct_t * column,const char *tabname,column_pack_i *data,int data_items)
{
	printf("build_insert_cmd data_items=%d\n",data_items);
	insert_cmd_i ret_cmd;
	ret_cmd.cmd=NULL;
	ret_cmd.blob_ret=NULL;

	ret_cmd.cmd=sqlite3_mprintf("insert into %q (",tabname);
	char *sql_val=sqlite3_mprintf("values(");
	column_pack_i *col_pack_ptr = data;
	const column_struct_t *column_ptr=column,*column_cur=column;
	int i=0,max_row_id=0;
	int blob_pos=1;
	for(;(NULL!=column_cur->data_type)&&(NULL!=column_cur);){
		max_row_id++;
		column_cur++;
	}

	printf("max_row_id=%d\n",max_row_id);
	blob_struct* blob_cur=ret_cmd.blob_ret;
	int first=1;
	for(i=0;(i<data_items);i++)
	{
		if((col_pack_ptr->len>0)&&(col_pack_ptr->row_pos<max_row_id))
		{
			column_cur=column_ptr+col_pack_ptr->row_pos;
			ret_cmd.cmd=db_sql_mprintf("%q%q%q",ret_cmd.cmd,first?"":",",column_cur->column_id);

			if(strncmp(column_cur->data_type,"INTEGER",7)==0)
			{
				sql_val=db_sql_mprintf("%q%q%d",sql_val,first?"":",",*(int *)col_pack_ptr->data);
			}
			else if(strncmp(column_cur->data_type,"FLOAT",5)==0)
			{
				sql_val=db_sql_mprintf("%q%q%f",sql_val,first?"":",",*(float *)col_pack_ptr->data);
			}
			else if(strncmp(column_cur->data_type,"REAL",4)==0)
			{
				sql_val=db_sql_mprintf("%q%q%g",sql_val,first?"":",",*(double *)col_pack_ptr->data);
			}
			else if((strncmp(column_cur->data_type,"TEXT",4)==0)/*||strncmp(column_cur->data_type,"BLOB",4)==0*/)
			{
				//sql_val=sqlite3_mprintf("%q%q%q",sql_val,first?"":",",(char *)col_pack_ptr->data);

				sql_val=db_sql_mprintf("%q%q?",sql_val,first?"":",");
				if(ret_cmd.blob_ret==NULL)
				{
					ret_cmd.blob_ret=malloc(sizeof(blob_struct));
					ret_cmd.blob_ret->data=col_pack_ptr->data;
					ret_cmd.blob_ret->len=col_pack_ptr->len;
					ret_cmd.blob_ret->pos=blob_pos;
					ret_cmd.blob_ret->type_index=SI_TEXT;
					ret_cmd.blob_ret->next=NULL;
					blob_cur=ret_cmd.blob_ret;
				}
				else
				{
					while((blob_cur!=NULL)&&(blob_cur->next!=NULL))
					{
						blob_cur=blob_cur->next;
					}

					if(blob_cur->next==NULL)
					{
						blob_cur->next=malloc(sizeof(blob_struct));
						blob_cur->next->data=col_pack_ptr->data;
						blob_cur->next->len=col_pack_ptr->len;
						blob_cur->next->pos=blob_pos;
						blob_cur->next->type_index=SI_TEXT;
						blob_cur->next->next=NULL;
						blob_cur=blob_cur->next;
					}
				}
				blob_pos++;

			}
			else if(strncmp(column_cur->data_type,"BLOB",4)==0)
			{
				sql_val=db_sql_mprintf("%q%q?",sql_val,first?"":",");
				if(ret_cmd.blob_ret==NULL)
				{
					ret_cmd.blob_ret=malloc(sizeof(blob_struct));
					ret_cmd.blob_ret->data=col_pack_ptr->data;
					ret_cmd.blob_ret->len=col_pack_ptr->len;
					ret_cmd.blob_ret->pos=blob_pos;
					ret_cmd.blob_ret->type_index=SI_BLOB;
					ret_cmd.blob_ret->next=NULL;
					blob_cur=ret_cmd.blob_ret;
				}
				else
				{
					do{
						blob_cur=blob_cur->next;
					}while((blob_cur!=NULL)&&(blob_cur->next!=NULL));
					if(blob_cur->next==NULL)
					{
						blob_cur->next=malloc(sizeof(blob_struct));
						blob_cur->next->data=col_pack_ptr->data;
						blob_cur->next->len=col_pack_ptr->len;
						blob_cur->next->pos=blob_pos;
						blob_cur->next->type_index=SI_BLOB;
						blob_cur->next->next=NULL;
						blob_cur=blob_cur->next;
					}
				}
				blob_pos++;
			}
			first=0;
		}
		col_pack_ptr++;
		//printf("all sql:%s \n",ret_cmd.cmd);
	}

	sql_val=db_sql_mprintf("%q)",sql_val);
	ret_cmd.cmd=db_sql_mprintf("%q %q %q",ret_cmd.cmd,")",sql_val);
	if(sql_val!=NULL)
	{
		sqlite3_free(sql_val);
	}
	return ret_cmd;
}




//添加列条件
int add_column_id(query_struct_g *r_row_id,char *id)
{
	if(r_row_id->row_id==NULL)
	{
		r_row_id->row_id=malloc(sizeof(row_struct_g));
		if(r_row_id->row_id==NULL){
			perror("malloc for r_row_id:\n");
			return -1;
		}else
		{
			r_row_id->row_id->last=NULL;
			r_row_id->row_id->name=sqlite3_mprintf("%q",id);
			r_row_id->row_id->next=NULL;
		}
	}
	else
	{
		if(r_row_id->row_id->name==NULL)
		{
			r_row_id->row_id->name=sqlite3_mprintf("%q",id);
		}
		else
		{
			r_row_id->row_id->name=sqlite3_mprintf("%q,%q",r_row_id->row_id->name,id);
		}

	}
	return 0;
}

//添加行条件
int add_query_column_val(query_struct_g*r_column_id,void *data,int len,int row_pos,char operator[10],char relationship[10])
{
	if((data==NULL)||(r_column_id==NULL))
	{
		printf("err:data or r_column_id is NULL!\n");
		return -1;
	}
	column_struct_g *column_prv;
	column_struct_g*cur_ptr=r_column_id->column_id;
	if(r_column_id->column_id==NULL)
	{
		r_column_id->column_id=malloc(sizeof(column_struct_g));
		if(r_column_id->column_id==NULL){
			perror("malloc for r_column_id:\n");
			return -1;
		}else
		{
			r_column_id->column_id->next=NULL;
			r_column_id->column_id->last=NULL;
			cur_ptr=r_column_id->column_id;
		}
	}
	else
	{
		while(cur_ptr->next!=NULL)
		{
			cur_ptr=cur_ptr->next;
		}
		cur_ptr->next=malloc(sizeof(column_struct_g));
		if(cur_ptr->next==NULL)
		{
			perror("add_column_id:malloc for cur_ptr->next:\n");
			return -1;
		}
		else
		{
			column_prv=cur_ptr;
			cur_ptr=cur_ptr->next;
			cur_ptr->last=column_prv;
			cur_ptr->next=NULL;
		}
	}
	cur_ptr->data=malloc(len);
	if(cur_ptr->data==NULL)
	{
		goto err;
	}

	memcpy(cur_ptr->data,data,len);
	if(operator!=NULL)
	{
		strncpy(cur_ptr->operator,operator,10);
	}
	else
	{
		cur_ptr->operator[0]='\0';
	}
	if(relationship!=NULL)
	{
		strncpy(cur_ptr->relationship,relationship,10);
	}
	else
	{
		cur_ptr->relationship[0]='\0';
	}
	cur_ptr->row_pos=row_pos;
	cur_ptr->next=NULL;
	return 0;
err:
	return -1;
}


//添加排序条件
int add_order_pri(query_struct_g *r_order_id,int row_pos,order_dir_t dir)
{

	order_struct_g*cur_ptr=r_order_id->order_pri;
	if(r_order_id->order_pri==NULL)
	{
		r_order_id->order_pri=malloc(sizeof(order_struct_g));
		if(r_order_id->order_pri==NULL){
			perror("malloc for r_order_id:\n");
			return -1;
		}else
		{
			r_order_id->order_pri->next=NULL;
			cur_ptr=r_order_id->order_pri;
		}
	}
	else
	{
		while((cur_ptr!=NULL)&&(cur_ptr->next!=NULL))
		{
			cur_ptr=cur_ptr->next;
		}
		cur_ptr->next=malloc(sizeof(order_struct_g));
		if(cur_ptr->next==NULL){
			perror("add_order_pri:malloc for cur_ptr->next:\n");
			return -1;
		}else
		{
			cur_ptr->next->next=NULL;
			cur_ptr=cur_ptr->next;
		}
	}
	cur_ptr->dir=dir;
	cur_ptr->row_pos=row_pos;
	return 0;
}


query_struct_g *alloc_query_struct(void)
{
	query_struct_g *cmd=NULL;
	cmd=(query_struct_g *)malloc(sizeof(query_struct_g));
	if(cmd==NULL)
	{
		fprintf(stderr, "%s:",__FUNCTION__);
		perror("alloc_query_struct:");
	}
	else
	{
		cmd->row_id=NULL;
		cmd->column_id=NULL;
		cmd->order_pri=NULL;
	}
	return cmd;
}


void free_query_struct(query_struct_g *qurey_cmd)
{
	if(qurey_cmd==NULL)
	{
		return;
	}
	row_struct_g *row_id_cur=qurey_cmd->row_id;
	row_struct_g *row_id_prv;
	column_struct_g *column_cur=qurey_cmd->column_id;
	column_struct_g * column_prv;
	order_struct_g  *order_cur=qurey_cmd->order_pri;
	order_struct_g *order_prv;

	while(row_id_cur!=NULL)
	{
		row_id_prv=row_id_cur;
		row_id_cur=row_id_cur->next;
		if(row_id_prv->name!=NULL)
		{
			sqlite3_free(row_id_prv->name);
		}
		free(row_id_prv);
	}

	while(column_cur!=NULL)
	{
		column_prv=column_cur;
		column_cur=column_cur->next;
		if(column_prv->data!=NULL){
			free(column_prv->data);
		}
		free(column_prv);
	}

	while(order_cur!=NULL)
	{
		order_prv=order_cur;
		order_cur=order_cur->next;
		free(order_prv);
	}
	free(qurey_cmd);
}

int build_query_cmd_vspf(query_cmd_g *resault,char *fmt, ...)
{
	if(resault==NULL)
	{
		fprintf(stderr, "%s:err resault is NULL!\n",__FUNCTION__);
		return -1;
	}
	va_list argptr;
	va_start(argptr, fmt);
	resault->query_cmd=sqlite3_vmprintf(fmt, argptr);
	va_end(argptr);
	if(resault->query_cmd!=NULL)
	{
		return 0;
	}
	else
	{
		return -1;
	}

}


int build_query_cmd(const column_struct_t * column_array, char *tabname,\
		query_struct_g*qurey_cmd,char *add,query_cmd_g *resault)
{


	if((column_array==NULL)||(resault==NULL)){
		fprintf(stderr, "%s:err resault or column_array is NULL!\n",__FUNCTION__);
		return -1;
	}else
	resault->query_cmd=NULL;

	if(qurey_cmd==NULL)
	{
		resault->query_cmd=sqlite3_mprintf("select * from %q",tabname);
		return 0;
	}

	int max_row_id=0;
	const column_struct_t *cur=column_array,*c_cur=column_array;
	for(;(NULL!=c_cur->data_type)&&(NULL!=c_cur);){
		max_row_id++;
		c_cur++;
	}
	column_struct_g *column_cur=qurey_cmd->column_id;
	row_struct_g    *row_cur=qurey_cmd->row_id;
	order_struct_g  *order_cur=qurey_cmd->order_pri;

	resault->query_cmd=sqlite3_mprintf("select");
	if(row_cur!=NULL)
	{
		while (row_cur!=NULL)
		{
			resault->query_cmd=db_sql_mprintf("%q %q",resault->query_cmd,row_cur->name);
			row_cur=row_cur->next;
		}
	}
	else
	{
		resault->query_cmd=db_sql_mprintf("%q %q",resault->query_cmd,"*");
	}

	resault->query_cmd=db_sql_mprintf("%q from %q ",resault->query_cmd,tabname);
	if(column_cur!=NULL)
	{
		resault->query_cmd=db_sql_mprintf("%q where ",resault->query_cmd);
	}
	while (column_cur!=NULL)
	{
		if(column_cur->last!=NULL)
		{
			if(column_cur->row_pos<max_row_id)
			{
				if(column_cur->relationship[0]!='\0')
				{
					resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,column_cur->relationship);
				}
			}
		}

		if(column_cur->row_pos<max_row_id)
		{
			c_cur=cur+column_cur->row_pos;
			resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,c_cur->column_id);
			printf("column_cur->row_pos:%d\n",max_row_id<column_cur->row_pos);
			if(column_cur->operator[0]!='\0')
			{
				resault->query_cmd=db_sql_mprintf("%q %q ",resault->query_cmd,column_cur->operator);
			}

			if(column_cur->data!=NULL)
			{
				if(strncmp(c_cur->data_type,"INTEGER",7)==0)
				{
					resault->query_cmd=db_sql_mprintf("%q%d",resault->query_cmd,*(int *)column_cur->data);
				}
				else if(strncmp(c_cur->data_type,"FLOAT",5)==0)
				{
					resault->query_cmd=db_sql_mprintf("%q%f",resault->query_cmd,*(float *)column_cur->data);
				}
				/*	else if(strncmp(c_cur->data_type,"TEXT",4)==0)
				{
					text_ptr[text_count]=column_cur;
					text_count++;
					//resault->query_cmd=sqlite3_mprintf("%q%q",resault->query_cmd,(char *)column_cur->data);
				}*/
				else if(strncmp(c_cur->data_type,"REAL",4)==0)
				{
					resault->query_cmd=db_sql_mprintf("%q%g",resault->query_cmd,*(double *)column_cur->data);
				}
			}
		}

		column_cur=column_cur->next;
	}

	int first=1;
	if(order_cur!=NULL)
	{
		resault->query_cmd=db_sql_mprintf("%q order by ",resault->query_cmd);
	}
	while (order_cur!=NULL)
	{
		if(order_cur->row_pos<max_row_id)
		{

			switch(order_cur->dir)
			{
			case ORDER_DESC:
			{
				c_cur=cur+order_cur->row_pos;
				if(first)
				{
					resault->query_cmd=db_sql_mprintf("%q %q desc",resault->query_cmd,c_cur->column_id);
				}
				else
				{
					resault->query_cmd=db_sql_mprintf("%q,%q desc",resault->query_cmd,c_cur->column_id);
				}
				first=0;
			}
			break;
			case ORDER_ASC:
			{
				c_cur=cur+order_cur->row_pos;
				if(first){
					resault->query_cmd=db_sql_mprintf("%q %q asc",resault->query_cmd,c_cur->column_id);
				}
				else
				{
					resault->query_cmd=db_sql_mprintf("%q,%q asc",resault->query_cmd,c_cur->column_id);
				}
				first=0;
			}
			break;
			default :
				break;
			}

		}
		order_cur=order_cur->next;
	}

	if(add!=NULL)
	{
		resault->query_cmd=db_sql_mprintf("%q %q",resault->query_cmd,add);
	}

	return 0;
}





#if 0
int query_data_from_table(db_handle_t *db_h, query_cmd_g *query_cmd,void *fun(void *data,int *icol,int *len,int *type))
{
	sqlite3_stmt *stat;
	int rc = 0/*,size = 0*/;
	int column_count=0;
	char *left=NULL;
	//column_struct_t *cur_c=column_arry;
	if((query_cmd==NULL)||(query_cmd->query_cmd==NULL)||(db_h==NULL)||(fun==NULL))
	{
		fprintf(stderr, "%s:err !\n",__FUNCTION__);
		return -1;
	}

	rc = sqlite3_prepare_v2(db_h->db, query_cmd->query_cmd, -1, &stat,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr, "%s_ sqlite3_prepare err:%s !\n",__FUNCTION__,sqlite3_errmsg(db_h->db));
	}
	if(left!=NULL)
	{
		printf("ERR left :%s \n",left);
	}
	column_count=sqlite3_column_count(stat);
	int i=0;
	int type=SQLITE_NULL;
	while(sqlite3_step(stat)==SQLITE_ROW)
	{
		for(i=0;i<column_count;i++)
		{
			type=sqlite3_column_type(stat,i);
			switch(type)
			{
			case SI_INTEGER:
			{
				int val=sqlite3_column_int(stat,i);
				int len=sizeof(int);
				fun((void*)&val,&i,&len,&type);
			}
				break;
			case SI_FLOAT:
			{
				double val=sqlite3_column_double(stat,i);
				int len=sizeof(double);
				fun(&val,&i,&len,&type);
			}
				break;
			case SI_BLOB:
			{
				char *val=(char *)sqlite3_column_blob(stat,i);
				int len=sqlite3_column_bytes(stat,i);
				fun(val,&i,&len,&type);
			}
				break;
			case SI_NULL:
				//fun(NULL,i,sizeof(int));
				break;
			case SI_TEXT:
			{
				char *val=(char *)sqlite3_column_text(stat,i);
				int len=sqlite3_column_bytes(stat,i);
				printf("len=%d\n",len);
				fun(val,&i,&len,&type);
			}
				break;
			default:
				break;
			}
		}
		type=SI_COMMIT;
		fun(0,NULL,NULL,&type);
	}
	sqlite3_finalize(stat);
	return 0;
}

#else
void * query_data_from_table(db_handle_t *db_h, query_cmd_g *query_cmd,int *count,\
		void*call_back_fun(data_ret_g *ret,void **cur_item,void **list_head))
{
	sqlite3_stmt *stat;
	int rc = 0/*,size = 0*/;
	int column_count=0;
	void *head=NULL,*cur=NULL;
	int row_count=0;
	//column_struct_t *cur_c=column_arry;
	if(count!=NULL)
	{
		*count=row_count;
	}
	if((query_cmd==NULL)||(db_h==NULL)||(call_back_fun==NULL))
	{
		fprintf(stderr, "%s:err !\n",__FUNCTION__);
		return NULL;
	}
	if(query_cmd->query_cmd==NULL)
	{
		return NULL;
	}
	else
	{
		printf("query_data_from_table:%s\n",query_cmd->query_cmd);
	}

	rc = sqlite3_prepare_v2(db_h->db, query_cmd->query_cmd, -1, &stat,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr, "%s_ sqlite3_prepare err:%s !\n",__FUNCTION__,sqlite3_errmsg(db_h->db));
		return NULL;
	}
	column_count=sqlite3_column_count(stat);
	int i=0;
	data_ret_g ret;
	while((rc=sqlite3_step(stat))==SQLITE_ROW)
	{
		row_count++;
		ret.data_type=SI_NEW_ITEM;
		call_back_fun(&ret,&cur,&head);
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
				call_back_fun(&ret,&cur,&head);
			}
				break;
			case SI_FLOAT:
			{
				ret.dou_ret=sqlite3_column_double(stat,i);
				ret.icol=i;
				ret.data_len=sizeof(double);
				call_back_fun(&ret,&cur,&head);
			}
				break;
			case SI_BLOB:
			{
				ret.ptr_ret=(void *)sqlite3_column_blob(stat,i);
				ret.icol=i;
				ret.data_len=sqlite3_column_bytes(stat,i);
				call_back_fun(&ret,&cur,&head);
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
				call_back_fun(&ret,&cur,&head);

			}
				break;
			default:
				break;
			}
		}
	}
	if(rc==SQLITE_FULL)
	{
		row_count=-1;
	}
	sqlite3_finalize(stat);
	if(count!=NULL)
	{
		*count=row_count;
	}
	insert_comut++;
	return head;
}

#endif


int insert_data_to_table(db_handle_t *db_h,insert_cmd_i sql_cmd)
{
	blob_struct *blob_ptr=sql_cmd.blob_ret;
	sqlite3_stmt *stat;
	int rc = 0;
	if((sql_cmd.cmd==NULL)||(db_h==NULL))
	{
		printf("db_h or insert cmd is NULL\n");
		return -1;
	}
	rc = sqlite3_prepare_v2(db_h->db, sql_cmd.cmd, -1, &stat,NULL);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_prepare err:%s\n",sqlite3_errmsg(db_h->db));
		goto err;
	}
	while(blob_ptr!=NULL)
	{
		if(blob_ptr->type_index==SI_BLOB)
		{
			rc=sqlite3_bind_blob(stat,blob_ptr->pos,blob_ptr->data,blob_ptr->len, NULL);
			if(rc!=SQLITE_OK){
				printf("sqlite3_bind_blob err:%s\n",sqlite3_errmsg(db_h->db));
			}
		}
		if(blob_ptr->type_index==SI_TEXT)
		{
			rc=sqlite3_bind_text(stat,blob_ptr->pos,blob_ptr->data,blob_ptr->len, NULL);
			if(rc!=SQLITE_OK){
				printf("sqlite3_bind_text err:%s\n",sqlite3_errmsg(db_h->db));
			}
		}
		blob_ptr=blob_ptr->next;
	}
	rc = sqlite3_step(stat);
	if(rc!=SQLITE_DONE)
	{
		printf("sqlite3_step err:%s\n",sqlite3_errmsg(db_h->db));
		goto err;
	}
	rc = sqlite3_finalize(stat);
	if(rc!=SQLITE_OK)
	{
		printf("sqlite3_finalize err:%s\n",sqlite3_errmsg(db_h->db));
		goto err;
	}
err:
	sqlite3_db_release_memory(db_h->db);
	insert_comut++;
	return rc;
}


void free_insert_cmd(insert_cmd_i sql_cmd)
{
	blob_struct*blob_prv=sql_cmd.blob_ret,*blob_cur=sql_cmd.blob_ret;
	if(sql_cmd.cmd!=NULL)
	{
		sqlite3_free(sql_cmd.cmd);
	}
	while(blob_prv!=NULL)
	{
		blob_cur=blob_prv->next;
		free(blob_prv);
		blob_prv=blob_cur;
	}
}


int databsae_create_tbl(db_handle_t * db_h, char * tabname,const column_struct_t * column,int time_dflt)
{
	char *sql = NULL;
	char *err=NULL;
	int rc = -1;

	if(!db_h || !tabname || !column) {
		return -1;
	}

	sql=sqlite3_mprintf("");
	sql=sqlite3_mprintf("create table %q(",tabname);


	const column_struct_t *col_tmp = column;
	int i=0;
	for(i=0;(NULL != col_tmp)&&(NULL != col_tmp->column_id);i++)
	{
		if(col_tmp->id_more!=NULL)
		{
			sql=sqlite3_mprintf("%q%q %q",sql,column[i].id_more, column[i].data_type);
		}

		else
		{
			sql=sqlite3_mprintf("%q%q %q",sql,column[i].column_id, column[i].data_type);
		}
		col_tmp++;
		if((NULL != col_tmp)&&(NULL != col_tmp->column_id)){
			sql=sqlite3_mprintf("%q%q",sql,",");
		}
	}

	if(time_dflt==1) {
		sql = sqlite3_mprintf("%q,%q%Q,%Q%q",sql,"insert_time TimeStamp NOT NULL DEFAULT (datetime(","now","localtime",")))");
	} else {
		sql = sqlite3_mprintf("%q%q",sql,")");
	}

	printf("sql=%s\n", sql);

	rc = sqlite3_exec(db_h->db, sql ,NULL ,NULL, &err);
	if(rc != SQLITE_OK) {
		printf("%s\n", err);
		sqlite3_free(err);
	}

	sqlite3_free((void*)sql);
	return rc;
}


int databsae_create_table_vspf(db_handle_t * db_h, char *fmt, ...)
{

	char *sql = NULL;
	char *err=NULL;
	int rc = -1;
	if(db_h==NULL)
	{
		fprintf(stderr, "%s:err db_h is NULL!\n",__FUNCTION__);
		return -1;
	}
	va_list argptr;
	va_start(argptr, fmt);
	sql=sqlite3_vmprintf(fmt, argptr);
	va_end(argptr);
	if(sql==NULL)
	{
		fprintf(stderr, "%s:sqlite3_vmprintf sql failure,sql  is NULL!\n",__FUNCTION__);
		return -1;
	}
	rc = sqlite3_exec(db_h->db, sql ,NULL ,NULL, &err);
	if(rc != SQLITE_OK)
	{
		//printf("%s\n", err);
		sqlite3_free(err);
	}
	sqlite3_free((void*)sql);
	return rc;
}


#endif






