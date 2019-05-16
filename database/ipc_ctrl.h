/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: ipc_ctrl.h
* Author				: Ritchie
* Version				: V1.0.0
* Date				: 2013Äê9ÔÂ23ÈÕ
* Description			: 
* Modify by			: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#ifndef __SEM_CTRL_H
#define	__SEM_CTRL_H


typedef union semun {
	int 			 val;	 /* Value for SETVAL */
	struct semid_ds *buf;	 /* Buffer for IPC_STAT, IPC_SET */
	unsigned short	*array;  /* Array for GETALL, SETALL */
	struct seminfo	*__buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
}semun_t;

typedef void (*const fun_signal)(int n);

#ifndef IPC_MUTEX_UNBLOCK
	#define	IPC_MUTEX_UNBLOCK			(1)
#endif

#ifndef IPC_MUTEX_BLOCK
	#define	IPC_MUTEX_BLOCK				(0)
#endif

int get_val(int sid, int sno);
int set_val(int sid, int sno, int val);
int sem_p(int fd, int idx);
int sem_v(int fd, int idx);
int sem_ctrl_create(const char *const path, int key_id);
int sem_ctrl_destroy(int sem_id);
int msg_ctrl_create(char *path, int key_id);
int msg_ctrl_destroy(int fd);
void prefix_signal_handler(fun_signal fun);

#endif

