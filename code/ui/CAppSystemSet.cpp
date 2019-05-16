#include "CCtrlModules.h"

class CSystemSetApp : public CAppBase
{
public:
	CSystemSetApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSystemSetApp(void)
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
					DPPostMessage(MSG_START_APP, PRJ_SAFE_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, PRJ_COLOR_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[1])
			{
				if(m_nPage == 0)
					DPPostMessage(MSG_START_APP, PRJ_CODE_APPID, 0, 0);
				else
					DPPostMessage(MSG_START_APP, PRJ_UPGRADE_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[2])
			{
				DPPostMessage(MSG_START_APP, PRJ_DOOR_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[3])
			{
				DPPostMessage(MSG_START_APP, PRJ_PWD_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[4])
			{
				DPPostMessage(MSG_START_APP, PRJ_RESET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[5])
			{
				DPPostMessage(MSG_START_APP, PRJ_IPCAMERA_APPID, 0, 0);
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
			m_pButton[0]->SetSrcpng("safesetting.png");
			m_pButton[0]->SetSrcText(GetStringByID(90001));
			m_pButton[0]->Show(STATUS_NORMAL);

			m_pButton[1]->SetSrcpng("roomcodesetting.png");
			m_pButton[1]->SetSrcText(GetStringByID(90002));
			m_pButton[1]->Show(STATUS_NORMAL);

			m_pButton[2]->SetSrcpng("doorcodesetting.png");
			m_pButton[2]->SetSrcText(GetStringByID(90003));
			m_pButton[2]->Show(STATUS_NORMAL);

			m_pButton[3]->SetSrcpng("projectpwdsetting.png");
			m_pButton[3]->SetSrcText(GetStringByID(90004));
			m_pButton[3]->Show(STATUS_NORMAL);

			m_pButton[4]->SetSrcpng("resetsystem.png");
			m_pButton[4]->SetSrcText(GetStringByID(90005));
			m_pButton[4]->Show(STATUS_NORMAL);

			m_pButton[5]->SetSrcpng("ipcamerasetting.png");
			m_pButton[5]->SetSrcText(GetStringByID(90007));
			m_pButton[5]->Show(STATUS_NORMAL);

			m_pPrev->Show(STATUS_FOCUS);
			m_pNext->Show(STATUS_NORMAL);
		}
		else if(nPage == 1)
		{
			m_pButton[0]->SetSrcpng("colorsetting.png");
			m_pButton[0]->SetSrcText(GetStringByID(90006));
			m_pButton[0]->Show(STATUS_NORMAL);

			m_pButton[1]->SetSrcpng("softupgrade.png");
			m_pButton[1]->SetSrcText(GetStringByID(90009));
			m_pButton[1]->Show(STATUS_NORMAL);

			m_pButton[2]->Hide();
			m_pButton[3]->Hide();
			m_pButton[4]->Hide();
			m_pButton[5]->Hide();

			m_pPrev->Show(STATUS_NORMAL);
			m_pNext->Show(STATUS_FOCUS);
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("userset.xml");
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

CAppBase* CreateSystemSetApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSystemSetApp* pApp = new CSystemSetApp(wParam);
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

