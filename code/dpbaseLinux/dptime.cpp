#include "dpplatform.h"
#include "dpdebug.h"

#define	SECOND_FROM_1601_TO_1970	11644473600
#define	SECOND_FROM_1601_TO_1900	9435513600

DWORD DPGetTickCount(void)
{
	struct timespec t_start;
	clock_gettime(CLOCK_MONOTONIC, &t_start);
	return (t_start.tv_sec * 1000 + t_start.tv_nsec / 1000000);
}

void DPSleep(DWORD dwMilliseconds)
{
	DWORD curtick;
	DWORD starttick = DPGetTickCount();
	DWORD leftms = dwMilliseconds;
	while(1)
	{
		if(0 == usleep(leftms * 1000))
			break;

		curtick = DPGetTickCount();
		if((curtick - starttick) > leftms)
			break;

		leftms -= curtick - starttick;
		starttick = curtick;
		//printf("leftms:%d\r\n", leftms);
	}
}

void DPGetLocalTime(SYSTEMTIME* lpSystemTime)
{
	if(lpSystemTime)
	{
		struct tm *pst = NULL;
		time_t t = time(NULL);
		pst = localtime(&t);
		memcpy(lpSystemTime,pst,sizeof(SYSTEMTIME));
		lpSystemTime->wYear += 1900;
		lpSystemTime->wMonth += 1;
	}
}

BOOL DPSetLocalTime(SYSTEMTIME *st)
{
	struct timeval time_tv;
	time_t timep;
	int ret;
	struct tm pst = {0};

	memcpy(&pst, st, sizeof(tm));
	st->wDayOfWeek = 0;
	st->tm_yday = 0;
	st->tm_isdst = 0;
	DBGMSG(DPINFO, "SetLocalTime %d %d %d %d %d %d\r\n", st->wYear, st->wMonth, st->wDay, st->wHour, st->wMinute, st->wSecond);

	pst.tm_year -= 1900;
	pst.tm_mon -= 1;
	timep = mktime((tm*)&pst);
	time_tv.tv_sec = timep;
	time_tv.tv_usec = 0;

	ret = settimeofday(&time_tv, NULL);
	if(ret != 0)
	{
		DBGMSG(DPERROR, "settimeofday failed %d\r\n", DPGetLastError());
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL DPSystemTimeToFileTime(SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime)
{
	time_t timep;
	UINT64 ui;

	lpSystemTime->wYear -= 1900;
	lpSystemTime->wMonth -= 1;
	timep = mktime((tm*)lpSystemTime);
	ui = timep + SECOND_FROM_1601_TO_1970;
	ui *= 10000000;
	lpFileTime->dwHighDateTime = ui >> 32;
	lpFileTime->dwLowDateTime = ui;
	return TRUE;
}

BOOL DPFileTimeToSystemTime(FILETIME* lpFileTime, SYSTEMTIME* lpSystemTime)
{
	UINT64 ui;
	time_t timep;
	struct tm *pst;

	ui = lpFileTime->dwHighDateTime;
	ui <<= 32;
	ui += lpFileTime->dwLowDateTime; 
	ui /= 10000000;
	timep = ui - SECOND_FROM_1601_TO_1970;
	pst = localtime(&timep);
	memcpy(lpSystemTime,pst,sizeof(SYSTEMTIME));
	lpSystemTime->wYear += 1900;
	lpSystemTime->wMonth += 1;
	return TRUE;
}
