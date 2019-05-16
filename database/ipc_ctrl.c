/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: sem_ctrl.c
* Author				: Ritchie
* Version				: V1.0.0
* Date				: 2013Äê9ÔÂ23ÈÕ
* Description			: 
* Modify by			: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>   
#include <errno.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/wait.h>
#define	DEBUG_ERROR
#include "my_debug.h"
#include "ipc_ctrl.h"

/**********************************************************************************
*Function name	: GET_VAL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int get_val(int sid, int sno)
{
	//semun_t sem_info = {1};
	int ret = 0;
	if(sid < 0 || sno < 0) {
		debug_err("bad args input\n");
	}
	ret = semctl(sid, sno, GETVAL);
	if(ret < 0) {
		perror("\nsemctl");
	}
	return ret;
}
/**********************************************************************************
*Function name	: SET_VAL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int set_val(int sid, int sno, int val)
{
	semun_t sem_info = {1};
	int ret = 0;
	if(sid < 0 || sno < 0) {
		debug_err("bad args input\n");
	}

	sem_info.val = val;
	ret = semctl(sid, sno, SETVAL, sem_info);
	if(ret < 0) {
		perror("\nsemctl");
	}
	//debug_print("setval: semval=%d\n", get_val(sid, sno));
	return ret;
}
/**********************************************************************************
*Function name	: SEM_P
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sem_p(int fd, int idx)
{
	struct sembuf p_array = {0, -1, SEM_UNDO};
	
	if(fd < 0 || idx < 0) {
		debug_err("sem v, bad args\n");
		return -1;
	}
	
	//debug_print("p0:the semval=%d\n", get_val(fd, idx));
	p_array.sem_num = idx;
	if(semop(fd, &p_array, 1) == -1) {
		perror("semop");
		debug_err("semop,p failed\n");
		return -1;
	}
	//debug_print("p1:the semval=%d\n", get_val(fd, idx));
	return 0;
}

/**********************************************************************************
*Function name	: SEM_V
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sem_v(int fd, int idx)
{
	struct sembuf v_array = {0, 1, SEM_UNDO};
	
	if(fd < 0 || idx < 0) {
		debug_err("sem v, bad args\n");
		return -1;
	}
	
	//debug_print("v0:the semval=%d\n", get_val(fd, idx));
	v_array.sem_num = idx;
	if(semop(fd, &v_array, 1) == -1) {
		perror("semop");
		debug_err("semop,v failed\n");
		return -1;
	}
	//debug_print("v1:the semval=%d\n", get_val(fd, idx));

	return 0;
}

/**********************************************************************************
*Function name	: _PATH_TO_KEY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
static int _path_to_key(const char *const path, int key_id)
{
	if(access(path, F_OK) != 0)	{
		perror("access");
		return -1;
	}

	key_t k_sem = ftok(".", key_id);
	
	if(k_sem < 0) {
		perror("ftok");
		debug_err("ftok failed\n");
		return -1;
	} else {
		debug_info("semph: key=0x%08X\n", k_sem);
	}

	return k_sem;
}
/**********************************************************************************
*Function name	: SEM_CTRL_INIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sem_ctrl_create(const char *const path, int key_id)
{	
	key_t k_sem = _path_to_key(path, key_id);
	int semfd = semget(k_sem, 1, IPC_PRIVATE|SEM_UNDO|IPC_CREAT|S_IRUSR|S_IWUSR);

	if(semfd < 0) {
		perror("semget");
		debug_err("semget failed,semfd=%d\n",semfd);
		int error_no = errno;
		switch(error_no) {
			case EEXIST:
				semfd = semget(k_sem, 1, IPC_CREAT|SEM_UNDO);
				if(semfd < 0) {
					perror("semget again");
					debug_err("semget again failed,semfd=%d\n",semfd);
					return -1;					
				}
				//semctrl();
				break;
			/*	
			case EACCESS:
				debug_err("access\n");
				break;
			*/	
			default:
				debug_err("errno=%d\n", error_no);
				return -1;
		}
	}else {
		set_val(semfd, 0, 1);
		debug_info("semph: semfd=%d\n", semfd);
	}

	return semfd;
}

/**********************************************************************************
*Function name	: SEM_CTRL_EXIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int sem_ctrl_destroy(int sem_id)
{
	int ret = 0;
	if(sem_id < 0) {
		debug_err("bad args input\n");
		return -1;
	}
	ret = semctl(sem_id, 0, IPC_RMID);
	if(ret < 0) {
		debug_info("semctl failed\n");
		perror("semctl:");
	}

	return ret;
}

/**********************************************************************************
*Function name	: MSG_CTRL_INIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int msg_ctrl_create(char *path, int key_id)
{	
	int fd = -1;
	key_t k_msg = _path_to_key(path, key_id);
	fd = msgget(k_msg, IPC_CREAT|0666);
	if(-1 == fd) {
		debug_err(" error\n");
		return -1;
	} else {
		debug_info("msgid is %d\n", fd);
	}

	return fd;
}

/**********************************************************************************
*Function name	: msg_ctrl_destroy
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int msg_ctrl_destroy(int fd)
{
	int ret = 0;
	if(fd < 0) {
		debug_err("bad args input\n");
		return -1;
	}
	ret = msgctl(fd, IPC_RMID, NULL);
	close(fd);
	fd = -1;

	return 0;
}

/**********************************************************************************
*Function name	: SIGNAL_CATCH
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void prefix_signal_handler(fun_signal fun)
{
	signal(SIGINT, fun);	/* ctrl+c */
	signal(SIGTSTP, fun);	/* ctrl+z */	
	//signal(SIGTERM, fun);	/* reboot */
}


