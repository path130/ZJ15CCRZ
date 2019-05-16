#include "CCtrlModules.h"

class CPwdInputApp : public CAppBase
{
public:
	CPwdInputApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPwdInputApp(void)
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
			if( wParam == m_idBack )
			{
				DPPostMessage( MSG_START_APP, MAIN_APPID, 0, 0 );
				DPPostMessage( MSG_END_APP, (DWORD)this, 0, 0 );
			}
			break;
		case KBD_MESSAGE:
			if(wParam == KBD_CTRL)
			{
				if( lParam == '-' )
					m_pEditPwd->Delete();
				else if( lParam == '*' )
				{
					char pwd[16];
					GetProjectPwd(pwd);
#ifdef _DEBUG
					if(1)
#else
					if(strcmp(pwd, m_pEditPwd->GetString()) == 0)
#endif
					{
						DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
						DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
					}
					else
					{
						DPPostMessage( MSG_SHOW_STATUE, 2001, 0, 0 );
						m_pEditPwd->SetString("");
					}
				}
				else
					m_pEditPwd->Input(lParam);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("pwdinput.xml");
		GetCtrlByName("back", &m_idBack);
		m_pEditPwd = (CEditBox*)GetCtrlByName("pwd");
		m_pTitle = (CDPStatic*)GetCtrlByName("title");
		m_pTitle->SetSrc(GetStringByID(90008));
		m_pTitle->Show(TRUE);

		m_pEditPwd->SetMaxLen(6);
		m_pEditPwd->SetIsPwd(TRUE);
		m_pEditPwd->SetFocus(TRUE);

		return TRUE;
	}
private:
	DWORD m_idBack;
	CDPStatic* m_pTitle;
	CEditBox* m_pEditPwd;
};

CAppBase* CreatePwdInputApp(DWORD pHwnd, DWORD lParam, DWORD zParam)
{
	CPwdInputApp* pMain;
	pMain = new CPwdInputApp(pHwnd);
	if(pMain != NULL)
	{
		if(!pMain->Create(lParam, zParam))
		{
			delete pMain;
			pMain = NULL;
		}
	}

	return pMain;
}

