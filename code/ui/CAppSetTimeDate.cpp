#include "CCtrlModules.h"

class CSetTimeDateApp : public CAppBase
{
public:
	CSetTimeDateApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetTimeDateApp(void)
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch( uMsg )
		{
		case TIME_MESSAGE:
			if(m_dwTimeout < m_screenoff)
			{
				m_dwTimeout++;
				if(m_dwTimeout == m_screenoff)
					DPPostMessage(MSG_START_FROM_ROOT, BLACKSCREEN_APPID, 0, 0);
			}
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBack)
			{
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idSave)
			{
				DPSetLocalTime(&m_curTime);
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idDateAdd[0])
			{
				if(m_curTime.wYear < 9999)
				{
					m_pDate[0]->SetSrc(++m_curTime.wYear);
					m_pDate[0]->Show(TRUE);
				}
			}
			else if(wParam == m_idDateSub[0])
			{
				if(m_curTime.wYear > 0)
				{
					m_pDate[0]->SetSrc(--m_curTime.wYear);
					m_pDate[0]->Show(TRUE);
				}
			}
			else if(wParam == m_idDateAdd[1])
			{
				if(m_curTime.wMonth < 12)
				{
					m_pDate[1]->SetSrc(++m_curTime.wMonth);
					m_pDate[1]->Show(TRUE);
				}
			}
			else if(wParam == m_idDateSub[1])
			{
				if(m_curTime.wMonth > 1)
				{
					m_pDate[1]->SetSrc(--m_curTime.wMonth);
					m_pDate[1]->Show(TRUE);
				}
			}
			else if(wParam == m_idDateAdd[2])
			{
				int maxDay[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
				if( (m_curTime.wYear % 400 == 0) || (m_curTime.wYear % 100 != 0 && m_curTime.wYear % 4 == 0) )
					maxDay[2] = 29;

				if(m_curTime.wDay < maxDay[m_curTime.wMonth - 1])
				{
					m_pDate[2]->SetSrc(++m_curTime.wDay);
					m_pDate[2]->Show(TRUE);
				}
			}
			else if(wParam == m_idDateSub[2])
			{
				if(m_curTime.wDay > 1)
				{
					m_pDate[2]->SetSrc(--m_curTime.wDay);
					m_pDate[2]->Show(TRUE);
				}
			}
			else if(wParam == m_idTimeAdd[0])
			{
				if(m_curTime.wHour < 23)
				{
					m_pTime[0]->SetSrc(++m_curTime.wHour);
					m_pTime[0]->Show(TRUE);
				}
			}
			else if(wParam == m_idTimeSub[0])
			{
				if(m_curTime.wHour > 0)
				{
					m_pTime[0]->SetSrc(--m_curTime.wHour);
					m_pTime[0]->Show(TRUE);
				}
			}
			else if(wParam == m_idTimeAdd[1])
			{
				if(m_curTime.wMinute < 59)
				{
					m_pTime[1]->SetSrc(++m_curTime.wMinute);
					m_pTime[1]->Show(TRUE);
				}
			}
			else if(wParam == m_idTimeSub[1])
			{
				if(m_curTime.wMinute > 0)
				{
					m_pTime[1]->SetSrc(--m_curTime.wMinute);
					m_pTime[1]->Show(TRUE);
				}
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("settimedate.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("save", &m_idSave);
		GetCtrlByName("year_add", &m_idDateAdd[0]);
		GetCtrlByName("year_sub", &m_idDateSub[0]);
		GetCtrlByName("month_add", &m_idDateAdd[1]);
		GetCtrlByName("month_sub", &m_idDateSub[1]);
		GetCtrlByName("day_add", &m_idDateAdd[2]);
		GetCtrlByName("day_sub", &m_idDateSub[2]);
		GetCtrlByName("hour_add", &m_idTimeAdd[0]);
		GetCtrlByName("hour_sub", &m_idTimeSub[0]);
		GetCtrlByName("minute_add", &m_idTimeAdd[1]);
		GetCtrlByName("minute_sub", &m_idTimeSub[1]);

		m_pDate[0] = (CDPStatic*)GetCtrlByName("year");
		m_pDate[1] = (CDPStatic*)GetCtrlByName("month");
		m_pDate[2] = (CDPStatic*)GetCtrlByName("day");
		m_pTime[0] = (CDPStatic*)GetCtrlByName("hour");
		m_pTime[1] = (CDPStatic*)GetCtrlByName("minute");

		DPGetLocalTime(&m_curTime);
		m_pDate[0]->SetSrc(m_curTime.wYear);
		m_pDate[0]->Show(TRUE);
		m_pDate[1]->SetSrc(m_curTime.wMonth);
		m_pDate[1]->Show(TRUE);
		m_pDate[2]->SetSrc(m_curTime.wDay);
		m_pDate[2]->Show(TRUE);

		m_pTime[0]->SetSrc(m_curTime.wHour);
		m_pTime[0]->Show(TRUE);
		m_pTime[1]->SetSrc(m_curTime.wMinute);
		m_pTime[1]->Show(TRUE);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idSave;
	CDPStatic* m_pDate[3];	//0年 1月 2日
	CDPStatic* m_pTime[2];	//0时 1分
	DWORD m_idDateAdd[3];
	DWORD m_idDateSub[3];
	DWORD m_idTimeAdd[2];
	DWORD m_idTimeSub[2];
	SYSTEMTIME m_curTime;
};

CAppBase* CreateSetTimeDateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetTimeDateApp* pApp = new CSetTimeDateApp(wParam);
	if(pApp)
	{
		if(!pApp->Create(lParam, zParam))
		{
			delete pApp;
			pApp = NULL;		
		}
	}
	return pApp;
}