#include "CCtrlModules.h"

class CUserSetApp : public CAppBase
{
public:
	CUserSetApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CUserSetApp(void)
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
				DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idPrev)
			{
				OnPage(0);
			}
			else if(wParam == m_idNext)
			{
				OnPage(1);
			}
			else if(wParam == m_idButton[0])
			{
				if(m_nPage == 0)
					DPPostMessage(MSG_START_APP, SET_INFO_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, SET_BRIGHT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[1])
			{
				if(m_nPage == 0)
					DPPostMessage(MSG_START_APP, SET_RING_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, SET_CLEAN_SCREEN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[2])
			{
				if(m_nPage == 0)
					DPPostMessage(MSG_START_APP, SET_TIME_DATE_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, SET_WALLPAPER_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[3])
			{
				if(m_nPage == 0)
					DPPostMessage(MSG_START_APP, SET_PWD_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, SET_LANGUAGE_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[4])
			{
				if(m_nPage == 0)
					DPPostMessage(MSG_START_APP, SET_VOLUME_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, SET_SCREEN_SAVER_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[5])
			{
				if(m_nPage == 0)
					DPPostMessage(MSG_START_APP, SET_DELAY_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, SET_CALIBRATE_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		}
		return TRUE;
	}

	void OnPage(int nPage)
	{
		m_nPage = nPage;
		if(nPage == 0)
		{
			m_pButton[0]->SetSrcpng("systeminfo.png");
			m_pButton[0]->SetSrcText(GetStringByID(70009));
			m_pButton[0]->Show(STATUS_NORMAL);

			m_pButton[1]->SetSrcpng("ringsetting.png");
			m_pButton[1]->SetSrcText(GetStringByID(70005));
			m_pButton[1]->Show(STATUS_NORMAL);

			m_pButton[2]->SetSrcpng("timedatesetting.png");
			m_pButton[2]->SetSrcText(GetStringByID(70001));
			m_pButton[2]->Show(STATUS_NORMAL);

			m_pButton[3]->SetSrcpng("pwdsetting.png");
			m_pButton[3]->SetSrcText(GetStringByID(70008));
			m_pButton[3]->Show(STATUS_NORMAL);

			m_pButton[4]->SetSrcpng("volumesetting.png");
			m_pButton[4]->SetSrcText(GetStringByID(70006));
			m_pButton[4]->Show(STATUS_NORMAL);

			m_pButton[5]->SetSrcpng("delaysetting.png");
			m_pButton[5]->SetSrcText(GetStringByID(70007));
			m_pButton[5]->Show(STATUS_NORMAL);

			m_pPrev->Show(STATUS_FOCUS);
			m_pNext->Show(STATUS_NORMAL);
		}
		else if(nPage == 1)
		{
			m_pButton[0]->SetSrcpng("brightsetting.png");
			m_pButton[0]->SetSrcText(GetStringByID(70011));
			m_pButton[0]->Show(STATUS_NORMAL);

			m_pButton[1]->SetSrcpng("clearscreen.png");
			m_pButton[1]->SetSrcText(GetStringByID(70010));
			m_pButton[1]->Show(STATUS_NORMAL);

			m_pButton[2]->SetSrcpng("wallpapersetting.png");
			m_pButton[2]->SetSrcText(GetStringByID(70003));
			m_pButton[2]->Show(STATUS_NORMAL);

			m_pButton[3]->SetSrcpng("languagesetting.png");
			m_pButton[3]->SetSrcText(GetStringByID(70004));
			m_pButton[3]->Show(STATUS_NORMAL);

			m_pButton[4]->SetSrcpng("screensaversetting.png");
			m_pButton[4]->SetSrcText(GetStringByID(70002));
			m_pButton[4]->Show(STATUS_NORMAL);

#ifdef SCREEN_CALIBRATE
			m_pButton[5]->SetSrcpng("screencalibrate.png");
			m_pButton[5]->SetSrcText(GetStringByID(70012));
			m_pButton[5]->Show(STATUS_NORMAL);
#else
			m_pButton[5]->Hide();
#endif

			m_pPrev->Show(STATUS_NORMAL);
			m_pNext->Show(STATUS_FOCUS);
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("systemset.xml");
		GetCtrlByName("back", &m_idBack);
		m_pPrev = (CDPButton*)GetCtrlByName("prev", &m_idPrev);
		m_pNext = (CDPButton*)GetCtrlByName("next", &m_idNext);
		m_pButton[0] = (CDPButton*)GetCtrlByName("button1", &m_idButton[0]);
		m_pButton[1] = (CDPButton*)GetCtrlByName("button2", &m_idButton[1]);
		m_pButton[2] = (CDPButton*)GetCtrlByName("button3", &m_idButton[2]);
		m_pButton[3] = (CDPButton*)GetCtrlByName("button4", &m_idButton[3]);
		m_pButton[4] = (CDPButton*)GetCtrlByName("button5", &m_idButton[4]);
		m_pButton[5] = (CDPButton*)GetCtrlByName("button6", &m_idButton[5]);
		OnPage(lParam);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	CDPButton* m_pPrev;
	CDPButton* m_pNext;
	DWORD m_idButton[6];
	CDPButton* m_pButton[6];
	DWORD m_nPage;
};

CAppBase* CreateUserSetApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CUserSetApp* pApp = new CUserSetApp(wParam);
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