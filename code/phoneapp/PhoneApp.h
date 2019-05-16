#pragma once

#include "ipfunc.h"
#include "dpsession.h"

//PhoneApp.cpp
#define	MSG_NEW_CALLIN			0
#define	MSG_CALL_ERROR			1
#define	MSG_CALL_ACCEPT			2
#define	MSG_CALL_HUNGUP			3
#define	MSG_CALL_RING			4
#define	MSG_CLOUD_ACCEPT		5
#define	MSG_CLOUD_HANGUP		6
#define MSG_CLOUD_TRANS_BREAK	7

#define MSG_ERROR_BUSY			1
#define MSG_ERROR_NOMEDIA		2
#define	MSG_ERROR_OFFLINE		3
#define	MSG_ERROR_BREAK			4

typedef struct
{
	int		ip;						// �Է�IP
	char	code[16];				// �Է�����
	BOOL	bCallOut;				// �������
	DWORD 	dwCallType;				// ��������
	DWORD   dwSessionId;			// �Ựid
} PhonePkt;

DWORD GetPhonePkt(int type, char* code);
DWORD GetCallInPkt(int ip, char* code, DWORD dwSessionId);