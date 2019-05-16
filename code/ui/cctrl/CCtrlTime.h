#pragma once
#include "CCtrlBase.h"
#include "CCtrlStatic.h"
class CTimeDate:public CCtrlBase
{
public:
	CTimeDate(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		colon_frame = NULL;
		m_timecolor = 0xffffff;
		m_datecolor = 0xffffff;
		m_weekcolor = 0xffffff;
		m_existbit = 7;
		m_pTimeHour = NULL;
		m_pTimeMinute = NULL;
		colon_frame = NULL;
		m_pDate = NULL;
		m_pWeek = NULL;
		m_weekalgin = 0;
		m_datealgin = 0;
		iHide = FALSE;
		m_timesize = 32;
		m_datesize = 24;
		m_weeksize = 24;
	}
	~CTimeDate()
	{
		if(m_pTimeHour != NULL)
		{
			delete m_pTimeHour;
			m_pTimeHour = NULL;
		}
		if(m_pTimeMinute != NULL)
		{
			delete m_pTimeMinute;
			m_pTimeMinute = NULL;
		}
		if(m_pDate != NULL)
		{
			delete m_pDate;
			m_pDate = NULL;
		}
		if(m_pWeek != NULL)
		{
			delete m_pWeek;
			m_pWeek = NULL;
		}
		if(colon_frame != NULL)
			m_pSpr->CloseBlk(colon_frame);
	}
	BOOL DoInit(ContentManage *);
	void UpdataDateTime(BOOL IsInit = FALSE);
	void Hide(BOOL isHide);

private:
	void DrawTime(SYSTEMTIME* pTime);
	void DrawDate(SYSTEMTIME* pTime);
	DWORD m_timesize;
	DWORD m_timecolor;
	DWORD m_timeleft;
	DWORD m_timetop;
	CDPStatic* m_pTimeHour;
	CDPStatic* m_pTimeMinute;

	DWORD m_datesize;
	DWORD m_datecolor;
	DWORD m_dateleft;
	DWORD m_datetop;
	DWORD m_datealgin;
	CDPStatic* m_pDate;

	DWORD m_weeksize;
	DWORD m_weekcolor;
	DWORD m_weekleft;
	DWORD m_weektop;
	DWORD m_weekalgin;
	CDPStatic* m_pWeek;

	DWORD m_existbit;

	SYSTEMTIME lastsystem;
	DWORD m_WorkSec;
	HANDLE colon_frame;
	DWORD colon_width;
	BOOL  iHide;    //是否隐藏时间显示。
};


