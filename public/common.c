/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: common.c
* Author				: Ritchie
* Version				: V1.0.0
* Date				: 2012-07-06
* Description			: 
* Modify by			: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#include "common.h"
#include "my_types.h"
#include "my_debug.h"



/**********************************************************************************
*Function name	: FIND_STR_IN_BUF
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int find_str_in_buf(const uint8_t *const str, int slen, const uint8_t * const key, int klen)
{
	if(!str || !key)
		return -1;

	int tmp_s = slen, i = 0;
	const uint8_t *tmp_str = str;
	int ret = -1;
	//debug_print("slen=%d, key_len=%d\n", slen, klen);
	while(i<slen && tmp_s >= klen) {
		if((*tmp_str == *key ) && (!memcmp(tmp_str, key, klen))) {
			//debug_print("\n\ti=%d,slen=%d\n", i, slen);
			//debug_print("%s\n", tmp_str);
			//ret = (slen - i )- 1;
			ret = i-1;
			break;
		}
		tmp_s--;
		tmp_str++;
		i ++;
	}

	return ret;
}

/**********************************************************************************
*Function name	: PRINT_N_BYTE
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void print_n_byte(const uint8_t * const str, const int len)
{
	int i = 0;
	if(NULL == str || len <= 0)
		return;
	
	for(i = 0; i < len; i++) {
		if(i && (0 == i%10))
			debug_print("\n");
        debug_print("0x%02X  ", str[i]);
    }
    debug_print("\n");
}

/**********************************************************************************
*Function name	: PRINT_N_INT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void print_n_int(const int *const src, const int len)
{
	int i = 0;
	if(NULL == src || len <= 0)
		return;
	
	for(i = 0; i < len; i++) {
        debug_print("0x%08X  ", src[i]);
    }
    debug_print("\n");
}
#if 10
/**********************************************************************************
*Function name	: SUMMATION
*Description	: 必须保证输入不为空
*Input para		: 
*Output para	: 
*Return			: 
*Modify by		:
*Modify Date	: 
*Modify Note	: 
************************************************************************************/
uint8_t summation(const uint8_t *const src, const int lenth)
{
    int i = 0;
    uint8_t sum = 0;

	if(NULL == src || lenth < 0)
		return 0;
	
    if(0 == lenth)
        return *src;
    
    for(i=0; i<lenth; i++)
    {
        sum += src[i];    
    }    
    return sum;
}

/**********************************************************************************
*Function name	: check_sum
*Description	: 
*Input para		: 
*Output para	: 
*Return			: 
*Modify by		:
*Modify Date	: 
*Modify Note	: 
************************************************************************************/
int check_sum(const uint8_t * const src, const int length)
{    
    if(NULL == src || length < 0)
    {
        debug_err("NULL input\n");
        return -1;
    }
    return (summation(src, length-1) == src[length-1])? (1):(0);
}
#endif

#if 0
/**********************************************************************************
*Function name	: HEX_TO_STRING
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int hex_to_string(uint8_t hex, uint8_t *dest)
{
	if(assert_ptr(dest)) {
		return -1;
	}
	sprintf((char*)dest, "%02X", hex);
	return 0;
}

/**********************************************************************************
*Function name	: THREAD_ATTR_INIT
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int thread_attr_init(pthread_attr_t *attr)
{
	 int inherit = 0;
	 
	if(assert_ptr(attr)) {
		return -1;
	}
	
	if(pthread_attr_init(attr)) {	
		perror("pthread_attr_init");
		dbg_err("speech Failed to initialize thread attrs\n");
		return -1;
	}
	if(pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)) {
		perror("pthread_attr_setdetachstate");
		dbg_err("speech Failed to set schedule inheritance attribute\n");
		return -1;
	}
	if(pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED)) {
		perror("pthread_attr_setinheritsched");
		dbg_err("speech Failed to set schedule inheritance attribute\n");
		return -1;
	}
	
	if(pthread_attr_getinheritsched(attr, &inherit)) {
		perror("pthread_attr_getinheritsched");
		dbg_err("speech Failed to set schedule inheritance attribute\n");
		return -1;		
	} else {
	 	if (inherit == PTHREAD_INHERIT_SCHED) {			
			dbg_inf("PTHREAD_INHERIT_SCHED\n");
	 	} else if (inherit == PTHREAD_EXPLICIT_SCHED) {
			dbg_inf("PTHREAD_INHERIT_SCHED\n");
		} else {
			dbg_warn("unknown inherit\n");
		}
	}
	

	return 0;
}

/**********************************************************************************
*Function name	: GET_THREAD_POLICY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int get_thread_policy(pthread_attr_t *attr)
{
    int policy;
    int rs = pthread_attr_getschedpolicy(attr, &policy );
	
	if(rs < 0) {
		perror("pthread_attr_getschedpolicy");
		dbg_err("\n");
		return rs;
	}
	
    switch (policy)
    {
	    case SCHED_FIFO:
            dbg_inf("policy = SCHED_FIFO\n");
            break;

	    case SCHED_RR:
            dbg_inf("policy = SCHED_RR\n");
            break;

	    case SCHED_OTHER:
            dbg_inf("policy = SCHED_OTHER\n");
            break;

	    default:
            dbg_inf("policy = UNKNOWN\n");
            break;
	}

    return policy;
}

/**********************************************************************************
*Function name	: SHOW_THREAD_PRIORITY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int show_thread_priority(pthread_attr_t *attr, int policy)
{
	if(assert_ptr(attr)) {
		return -1;
	}
    int priority = sched_get_priority_max(policy);
    if(priority < 0) {
		dbg_err("\n");
		return priority;
    }
    dbg_inf("max_priority = %d\n", priority);

    priority = sched_get_priority_min(policy);
    if(priority < 0) {
		dbg_err("\n");
		return priority;
    }
    dbg_inf("min_priority = %d\n", priority);

	return 0;
}

/**********************************************************************************
*Function name	: GET_THREAD_PRIORITY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int get_thread_priority(pthread_attr_t *attr)
{
    struct sched_param param;

    int rs = pthread_attr_getschedparam(attr, &param);
	if(rs < 0) {
		dbg_err("\n");
		return rs;
	}
    dbg_inf("present priority = %d\n", param.__sched_priority);

    return param.__sched_priority;
}

/**********************************************************************************
*Function name	: GET_THREAD_MAX_PRIORITY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int get_thread_max_priority(int policy)
{
	int ret = 0;
	
	ret = sched_get_priority_max(policy);
	
	return ret;
}

/**********************************************************************************
*Function name	: GET_THREAD_MIN_PRIORITY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int get_thread_min_priority(int policy)
{
	int ret = 0;
	ret = sched_get_priority_min(policy);
	
	return ret;
}

/**********************************************************************************
*Function name	: SET_THREAD_POLICY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int set_thread_policy(pthread_attr_t *attr, int policy)
{
	if(assert_ptr(attr)) {
		return -1;
	}
    int rs = pthread_attr_setschedpolicy(attr, policy);
    if(rs < 0) {
		perror("pthread_attr_setschedpolicy");
		dbg_err("speech Failed to set FIFO scheduling policy\n");
		return rs;
    }
	
	show_thread_priority(attr, policy);
	
	//rs = get_thread_policy(attr);

	return rs;
}

/**********************************************************************************
*Function name	: SET_THREAD_PRIORITY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int set_thread_priority(pthread_attr_t *attr, int priority)
{
	struct sched_param schedParam;
	if(assert_ptr(attr)) {
		return -1;
	}

	schedParam.sched_priority = priority;
	if (pthread_attr_setschedparam(attr, &schedParam)) {
		perror("pthread_attr_setschedparam");
		dbg_err("speech Failed to set scheduler parameters\n");
		return -1;
	}

	return 0;
}

/**********************************************************************************
*Function name	: THREAD_ATTR_DESTROY
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
int thread_attr_destroy(pthread_attr_t *attr)
{
	if(assert_ptr(attr)) {
		return -1;
	}
	pthread_attr_destroy(attr);

	return 0;
}
#endif

/*===========================================================*/

typedef union LL_TO_WORD_DEF{
	uint64_t dword;
	 struct{
		uint32_t word_l;
		uint32_t word_h;
	}word;
}byte_inverse_t;


/**********************************************************************************
*Function name	: NTOHLL
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
void ntohll(const uint64_t src_dword, uint64_t *const dst_dword) 
{
	byte_inverse_t card_no = {0};

	if(assert_ptr(dst_dword)) {
		return;
	}
	card_no.dword = src_dword;
//	dbg_inf("src_dword=%llX,word_l=%08X,word_h=%08X\n", card_no.dword,card_no.word.word_l, card_no.word.word_h);
			
	((byte_inverse_t*)dst_dword)->word.word_l = ntohl(card_no.word.word_h);
	((byte_inverse_t*)dst_dword)->word.word_h = ntohl(card_no.word.word_l);
	
//	dbg_inf("dst_dword=%llX,word_l=%08X,word_h=%08X\n", ((byte_inverse_t*)dst_dword)->dword,card_no.word.word_l, card_no.word.word_h);

}
