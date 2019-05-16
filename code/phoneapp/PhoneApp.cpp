#include "roomlib.h"
#include "PhoneApp.h"
#include "ipfunc.h"

DWORD GetPhonePkt(int type, char* code)
{
	ip_get pGet = {0};

	if(code != NULL)
		TermGet(&pGet, code);
	else if(type == MONITOR_TYPE)
		CellDoorGet(&pGet);		// 第一个监视地址
	else if(type == MANAGER_TYPE)
		ManagerGet(&pGet);
	else if(type == GUARD_TYPE)
		GuardGet(&pGet);

	PhonePkt* pPkt = NULL;
	pPkt = (PhonePkt*)malloc(sizeof(PhonePkt));
	memset(pPkt, 0, sizeof(PhonePkt));
	pPkt->bCallOut = TRUE;
	pPkt->dwCallType = type;
	if(pGet.param)
	{
		pPkt->ip = pGet.param[0].ip;
		strcpy(pPkt->code, pGet.param[0].code);
		free(pGet.param);
	}

	return (DWORD)pPkt;
}

DWORD GetCallInPkt(int ip, char* code, DWORD dwSessionId)
{
	PhonePkt* pKt = NULL;
	pKt = (PhonePkt*)malloc(sizeof(PhonePkt));
	memset(pKt, 0, sizeof(PhonePkt));
	pKt->ip = ip;
	pKt->bCallOut = FALSE;
	strcpy(pKt->code, code);
	pKt->dwCallType = code[0] - '0';
	pKt->dwSessionId = dwSessionId;
	return (DWORD)pKt;
}

void Code2Name(char* name, char* code)
{
	int type = Code2Type(code);
	switch(type)
	{
	case MANAGER_TYPE:
		strcpy(name, GetStringByID(611));	// 管理中心
		break;
	case GUARD_TYPE:
		strcpy(name, GetStringByID(612));	// 保安分机
		break;
	case ROOM_TYPE:
		{
			if(GetTelBookName(code, name))
				break;

			int i;
			char termId[16];
			GetTermId(termId);
			for(i = 0; i < 13; i++)
			{
				if(termId[i] != code[i])
					break;
			}

			switch(i)
			{
			case 1:
			case 2:
				sprintf(name, "%c%c%s%c%c%s%c%c%s%c%c%c%c%s", 
					code[1], code[2], GetStringByID(617),								// 区
					code[3], code[4], GetStringByID(618),								// 栋
					code[5], code[6], GetStringByID(619),								// 单元
					code[7], code[8], code[9], code[10], GetStringByID(620));	// 室
				break;
			case 3:
			case 4:
				sprintf(name, "%c%c%s%c%c%s%c%c%c%c%s", 
					code[3], code[4], GetStringByID(618),								// 栋
					code[5], code[6], GetStringByID(619),								// 单元
					code[7], code[8], code[9], code[10], GetStringByID(620));	// 室
				break;
			case 5:
			case 6:
				sprintf(name, "%c%c%s%c%c%c%c%s", 
					code[5], code[6], GetStringByID(619),								// 单元
					code[7], code[8], code[9], code[10], GetStringByID(620));	// 室
				break;
			case 7:
			case 8:
			case 9:
			case 10:
				sprintf(name, "%c%c%c%c%s", code[7], code[8], code[9], code[10], GetStringByID(620) );	// 室
				break;
			case 11:
			case 12:
				sprintf(name, "%c%c%s%s", code[11], code[12], GetStringByID(613), GetStringByID(621));	// 号分机
				break;
			}
		}
		break;
	case MONITOR_TYPE:
	case CELL_DOOR_TYPE:
	case SECOND_DOOR_TYPE:
	case ZONE_DOOR_TYPE:
	case AREA_DOOR_TYPE:
		{
			switch(code[0])
			{
			case '2':
				sprintf(name, "%c%c%s%s", code[11], code[12], GetStringByID(613), GetStringByID(614));	// 号门口机
				break;
			case '3':
				sprintf(name, "%c%c%s%s", code[11], code[12], GetStringByID(613), GetStringByID(615));	// 号别墅机
				break;
			case '7':
				sprintf(name, "%c%c%s%s", code[11], code[12], GetStringByID(613), GetStringByID(616));	// 01号大门口机
				break;
			}
		}
		break;
	}
}
