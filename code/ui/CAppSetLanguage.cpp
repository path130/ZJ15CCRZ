#include "CCtrlModules.h"

class CSetLanguageApp : public CAppBase
{
public:
	CSetLanguageApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetLanguageApp(void)
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
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOK)
			{
				if(m_language != GetLanguage())
				{
					SetLanguage(m_language);
					if(m_language == 0)
						LoadAllString("/FlashDev/str/chinese.txt");
					else
						LoadAllString("/FlashDev/str/English.txt");
				}
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[0])
			{
				m_language = 0;
				m_pButton[0]->Show(STATUS_FOCUS);
				m_pButton[1]->Show(STATUS_NORMAL);
			}
			else if(wParam == m_idButton[1])
			{
				m_language = 1;
				m_pButton[0]->Show(STATUS_NORMAL);
				m_pButton[1]->Show(STATUS_FOCUS);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setlanguage.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("ok", &m_idOK);

		m_pButton[0] = (CDPButton*)GetCtrlByName("Chinese", &m_idButton[0]);
		m_pButton[1] = (CDPButton*)GetCtrlByName("English", &m_idButton[1]);

		m_language = GetLanguage();
		if(m_language == 0)
		{
			m_pButton[0]->Show(STATUS_FOCUS);
			m_pButton[1]->Show(STATUS_NORMAL);
		}
		else
		{
			m_pButton[0]->Show(STATUS_NORMAL);
			m_pButton[1]->Show(STATUS_FOCUS);
		}

		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idOK;
	DWORD m_idButton[2];		// 0 ÖÐÎÄ 1 Ó¢Óï
	CDPButton* m_pButton[2];	
	DWORD m_language;
};

CAppBase* CreateSetLanguageApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetLanguageApp* pApp = new CSetLanguageApp(wParam);
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