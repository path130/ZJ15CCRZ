/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: sqlite_ui.h
* Author				: Ritchie
* Version			: V1.0.0
* Date				: 2013-3-06
* Description			: 
* Modify by			: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#ifndef __SQLITE_UI_H
#define	__SQLITE_UI_H

#include "my_types.h"
#include "sqlite3.h"

#define		DB_HEAP_USE_LIMIT			(4*1024*1024)

#define	SQLPRMRYKEY			"INTEGER NOT NULL PRIMARY KEY"
#define	SQLIPKEY			"INTEGER PRIMARY KEY"
#define	SQLINT				"INTEGER"
#define	SQLFLOAT			"FLOAT"
#define	SQLTEXT 			"TEXT"
#define	SQLBLOB				"BLOB"
#define	SQLREAL				"REAL"
#define	SQLDATETIME			"DATETIME"

#define	SQLVCHAR			"VARCHAR "
#define	SQLVNCHAR(N)		"VARCHAR ("#N")"
#define	ODBASC				"ASC"
#define	ODBDSC				"DESC"
#define	DISTINCT			"DISTINCT"
#ifndef DB_OK
	#define DB_OK	SQLITE_OK
#endif

#define	DBERR_MSG(db)					sqlite3_errmsg(db)
typedef enum{NONE_EX=0, TIME_EX, EX_KEY, EX_TM_KEY}sql_ckey_t;
typedef enum {ORDNULL=0, ORDASC=1, ORDDSC, ORDDIS, ORCUNT}sql_odr_t;
typedef enum{DSELECT=0u, DINSERT, DUPDATE, DQUERY, DALTER, DDELETE, DDROP,DCOUNT}sql_sttmt_t;
typedef enum{EQUALTO, LESSTHAN, BIGGERTHAN, BETWEENOF, LIKE,SMIN, SMAX, ONE_MORE,CNDTN_NULL=0xFFu}sql_cndtn_t;

typedef int (*call_bk_fun)(uint8_t *dest_buf,int idx, int vlen, uint8_t *data);


typedef struct SQL_QUERY_DATA{
	uint id;			//column[0]
	uint dev_no;		//column[1]
	uint dev_ip;		//column[2]
	uint level;			//column[3]
	uint fun;			//column[4]
	uint hdptr;			//column[5]	// huge data pointer
	char datetime[20];	//column[6]
	struct SQL_QUERY_DATA *last;
	struct SQL_QUERY_DATA *next;
}sql_qdata_t;

//typedef int (*sqlite3_callback)(void*,int,char**, char**);

typedef struct SQL_CMD_CALLBACK{
	char *sql;
	sqlite3_callback fun;
}sql_callback_t;

typedef struct SQL_COLUMN_DEF{
	char *name;
	char *type;
}sql_column_t;

typedef struct SQL_BLOB_DATA_DEF{
	uint8_t *data;
	size_t len;
}sql_blob_t, db_blob_t;

typedef struct SQL_Q_ORDER_DEF{
	sql_odr_t ord;
	int col;
}sql_qord_t;

typedef struct SQL_Q_PAGE_DEF{
	int size;
	int offset;
}sql_page_t;

typedef struct SQL_Q_CDTN_DEF{
	sql_qord_t qord;
	sql_page_t page;
	sql_cndtn_t condition;
}sql_q_cdtn_t;

typedef struct SQL_U_CDTN_DEF{
	int bcol; 	// by column
	db_blob_t bdata; 	// by data
	int scol;	// set column
	db_blob_t sdata;	// set data
	sql_cndtn_t condition;
}sql_u_cdtn_t;


typedef struct SQL_BUILD_DEF{
	sqlite3 *db;
	sql_sttmt_t cmd;		// sql cmd
	const char * name;		// table name
	const sql_column_t *columns;	// column structure
	sqlite3_stmt *state;
	char *sql;
	sql_cndtn_t condition;
	sql_q_cdtn_t query;
	sql_u_cdtn_t update;
	int order;
	int done;
	uint8_t *dest;
	int (*deal_fun)(uint8_t * ,sqlite3_stmt **);
	int fd_set;
}sql_cmd_t;




int db_exec_state(sqlite3 *db, sqlite3_stmt *state, const int opt);
int db_step_state(sqlite3 *db, sql_cmd_t *cmd, const int opt);
int db_exec_sql(sqlite3 *db, char *sql, const int opt);
int db_step_sql(sqlite3 *db, char *sql, const int opt);
int db_do_step(sqlite3_stmt *state);
int db_step_finish(sqlite3_stmt *state);
int db_add_column(sqlite3 * db,const char *const tabname,const char *const colname,const char *const type);
int db_fix_column(sqlite3 * db,const char *const tabname, const sql_column_t *const column);
int db_drop_column(const sqlite3 * const db,const char *const tabname,const char *const colname,const char *const type);
/* ret=0:success, ret=1:already exsit, other:failed */
int db_create_tbl(sqlite3 * db,const char *const tabname, const sql_column_t * const column, int time_dflt);
int db_count_column(sqlite3 *db, sqlite3_stmt *state);
int db_update_data(sqlite3 *db, sql_cmd_t *update, const sql_column_t *const column, const char *const more);
int db_routine_begin(sqlite3 *const db, int opt);
int db_routine_submit(sqlite3 *const db, const int result);
int db_column_data(sqlite3_stmt* state, int j, int *pdata);
int db_state_query(sqlite3 *db, sql_cmd_t *qcmd);
int db_build_sql_stmt(sql_cmd_t *const sqlb, int col_no, const db_blob_t *const data, const sql_column_t *const col);
int db_sql_init(sqlite3 *db, sql_cmd_t *const cmd, sql_sttmt_t sql_cmd, const char *const tbl_name);
int db_build_sql_init(sqlite3 *db, sql_cmd_t *const cmd, sql_sttmt_t sql_cmd, const char *const tbl_name, const sql_column_t *const column);
int db_resize(sqlite3 *db, int sz);
int db_free_cach(sqlite3 *db,int opt);
int db_fill_blob(sql_blob_t *const src, uint8_t *src_data, size_t src_len);
int db_ctrl_init(int key_id);
void db_ctrl_exit(void);
int db_initialize(void);

int db_create_index(const sql_cmd_t *const ccmd);
int db_drop_index(sql_cmd_t *dcmd);
int db_drop_table(sqlite3*db, const char *const tbl_name);

int db_open_file(const char *filename, sqlite3 **ppdb);
int db_close_file(sqlite3 *db);

int user_sql_statement(sqlite3 *db, int argc, char **argv);
int db_free_sql(char *sql);
int db_free_cmdt(sql_cmd_t *cmd);
int db_pass_key(sqlite3 *db, const void *pKey, const int nKey);
int db_clear_key(sqlite3 *db, const void *pkey, const int nkey);


#endif

