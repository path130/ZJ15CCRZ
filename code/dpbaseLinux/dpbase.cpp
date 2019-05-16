#include <semaphore.h>
#include "dpplatform.h"

VOID DPInitCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);  
	pthread_mutex_init(pcsCriticalSection, &attr);
	pthread_mutexattr_destroy(&attr);
}

VOID DPDeleteCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	pthread_mutex_destroy(pcsCriticalSection);
}

VOID DPEnterCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	pthread_mutex_lock(pcsCriticalSection);
}

VOID DPLeaveCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	pthread_mutex_unlock(pcsCriticalSection);
}

HANDLE DPCreateSemaphore(DWORD dwInitCount, DWORD dwMaxCount)
{
	sem_t *semid = (sem_t*)malloc(sizeof(sem_t));
	sem_init(semid, 0, dwInitCount);
	return semid;
}

void DPSetSemaphore(HANDLE hSem)
{
	sem_post((sem_t *)hSem);
}

BOOL DPGetSemaphore(HANDLE hSem, DWORD timeout)
{
	struct timespec abstime;
	int wret;

	if(timeout == 0)
		wret = sem_trywait((sem_t *)hSem);
	else if(timeout == INFINITE)
		wret = sem_wait((sem_t *)hSem);
	else
	{
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec += timeout/1000;
		abstime.tv_nsec += (timeout % 1000) * 1000000;
		abstime.tv_sec += abstime.tv_nsec/1000000000;
		abstime.tv_nsec = abstime.tv_nsec%1000000000;
		wret = sem_timedwait((sem_t *)hSem, &abstime);
	}
	return (wret == 0);
}

DWORD DPGetLastError()
{
	return errno;
}


typedef struct _MSG_Q
{
	DWORD wptr;
	DWORD rptr;
	DWORD itemsize;
	DWORD maxsize;
	DWORD count;
	sem_t semid;
	pthread_mutex_t mutex;
	DWORD tick;					// 创建次数，用来确定什么时候free
	char msgdata[0];
} MSG_Q;

BOOL DPCreateMsgQueue(const char* name, DWORD vol, DWORD itemsize, HANDLE* pRead, HANDLE* pWrite)
{
	MSG_Q* pMsg = (MSG_Q*)malloc(sizeof(MSG_Q) + vol * itemsize);
	memset(pMsg, 0, sizeof(MSG_Q) + vol * itemsize);
	pMsg->itemsize = itemsize;
	pMsg->maxsize = vol;
	sem_init(&pMsg->semid, 0, 0);
	pthread_mutex_init(&pMsg->mutex, NULL);
	if(pRead != NULL)
	{
		*pRead = pMsg;
		pMsg->tick++;
	}
	if(pWrite != NULL)
	{
		*pWrite = pMsg;
		pMsg->tick++;
	}
	return TRUE;
}

BOOL DPWriteMsgQueue(HANDLE hMsg, void* data, DWORD len, DWORD timeout)
{
	BOOL ret = FALSE;
	MSG_Q* pMsg = (MSG_Q*)hMsg;
	if(len != pMsg->itemsize)
	{
		printf("DPWriteMsgQueue fail %d, %d", len, pMsg->itemsize);
		return FALSE;
	}
	pthread_mutex_lock(&pMsg->mutex);
	if(pMsg->count < pMsg->maxsize)
	{
		memcpy(pMsg->msgdata + pMsg->wptr * pMsg->itemsize, data, len);
		pMsg->wptr++;
		pMsg->wptr %= pMsg->maxsize;
		pMsg->count++;
		sem_post(&pMsg->semid);
		ret = TRUE;
	}
	else
	{
		printf("DPWriteMsgQueue fail 2 %d, %d", pMsg->count, pMsg->maxsize);
	}
	pthread_mutex_unlock(&pMsg->mutex);
	return ret;
}

BOOL DPReadMsgQueue(HANDLE hMsg, void* data, DWORD len, DWORD timeout)
{
	MSG_Q* pMsg = (MSG_Q*)hMsg;
	struct timespec abstime;
	BOOL ret = FALSE;
	int wret;

	if(timeout == 0)
		wret = sem_trywait(&pMsg->semid);
	else if(timeout == INFINITE)
		wret = sem_wait(&pMsg->semid);
	else
	{
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec += timeout/1000;
		abstime.tv_nsec += (timeout % 1000) * 1000000;
		abstime.tv_sec += abstime.tv_nsec/1000000000;
		abstime.tv_nsec = abstime.tv_nsec%1000000000;
		wret = sem_timedwait(&pMsg->semid, &abstime);
	}

	if(wret == 0)
	{
		pthread_mutex_lock(&pMsg->mutex);
		if(pMsg->count > 0)
		{
			memcpy(data, pMsg->msgdata + pMsg->rptr * pMsg->itemsize, len);
			pMsg->rptr++;
			pMsg->rptr %= pMsg->maxsize;
			pMsg->count--;
			ret = TRUE;
		}
		pthread_mutex_unlock(&pMsg->mutex);
	}
	return ret;
}

void DPCloseMsgQueue(HANDLE hMsg)
{
	MSG_Q* pMsg = (MSG_Q*)hMsg;
	if(pMsg->tick > 0)
	{
		pMsg->tick--;
		if(pMsg->tick == 0)
		{
			sem_destroy(&pMsg->semid);
			pthread_mutex_destroy(&pMsg->mutex);
			free(pMsg);
		}
	}

}


typedef DWORD (threadfun) (void* pdata);

static int PPIORITYMap[8] =
{
	99,
	80,
	60,
	50,
	40,
	30,
	20,
	10
};

typedef struct
{
	threadfun* pfunc;
	void* pdata;
	DWORD priproty;
} ThreadInfo;

static void* threadthub(void* pdata)
{
	ThreadInfo* pinfo = (ThreadInfo*)pdata;
	pthread_attr_t attr;
	int rs;
	int policy;
	DWORD dRet;
	struct sched_param sp;

	rs = pthread_getattr_np(pthread_self(), &attr);
	if(rs != 0)
		printf("getattr fail %d\r\n", errno);
	rs = SCHED_FIFO;
	policy = pthread_attr_setschedpolicy(&attr, rs);
	sp.sched_priority = PPIORITYMap[pinfo->priproty];
	policy = pthread_attr_setschedparam(&attr, &sp);
	//printf("Show current configuration of priority %d %d\r\n", policy, sp.sched_priority);
	rs = pthread_attr_destroy( &attr );
	dRet = pinfo->pfunc(pinfo->pdata);
	free(pinfo);
	return (void*)dRet;
}

HANDLE DPThreadCreate(int stacksize, DWORD (*func) (void *), void *arg, BOOL isjoin, DWORD level)
{
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t));
	pthread_attr_t attr;
	pthread_attr_init(&attr); /*初始化线程属性*/
	pthread_attr_setstacksize(&attr, stacksize);
	if(!isjoin)
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ThreadInfo* pinfo = (ThreadInfo*)malloc(sizeof(ThreadInfo));
	pinfo->pfunc = func;
	pinfo->pdata = arg;
	pinfo->priproty = level;
	pthread_create(thread, &attr, threadthub, (void*)pinfo);
	pthread_attr_destroy(&attr);
	if(!isjoin)
	{
		free(thread);
		return NULL;
	}
	return thread;
}

BOOL DPThreadJoin(HANDLE hThread, DWORD* pRet)
{
	pthread_t *thread = (pthread_t *) hThread;
	BOOL bret;
	void* pret;

	if (thread == NULL)
		return TRUE;
	bret =  (pthread_join(*thread, &pret) == 0);
	free(thread);
	if(pRet != NULL)
		*pRet = (DWORD)pret;
	return bret;
}