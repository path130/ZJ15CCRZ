#include "CCtrlModules.h"

class CPrjPwdApp : public CAppBase
{
public:
	CPrjPwdApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPrjPwdApp(void)
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch( uMsg )
		{
		case TIME_MESSAGE:
			m_pEdit[m_nFocus]->Flick();
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
				DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam >= m_idEdit[0] && wParam <= m_idEdit[2])
			{
				m_pEdit[m_nFocus]->Show(FALSE);
				m_nFocus = wParam - m_idEdit[0];
				m_pEdit[m_nFocus]->Show(TRUE);
			}
			break;
		case KBD_MESSAGE:
			if(wParam == KBD_CTRL)
			{
				if(lParam == '-')
					m_pEdit[m_nFocus]->Delete();
				else if(lParam == '*')
				{
					do
					{
						char pwd[16];
						GetProjectPwd(pwd);
						if(strcmp(m_pEdit[0]->GetString(), pwd) != 0 )
						{
							DPPostMessage(MSG_SHOW_STATUE, 2001, 0, 0);	//Ô­ÃÜÂë´íÎó
							break;
						}

						if(strlen(m_pEdit[1]->GetString()) != 6 || strlen(m_pEdit[2]->GetString()) != 6 )
						{
							DPPostMessage(MSG_SHOW_STATUE, 2007, 0, 0);	//ÇëÊäÈëÁùÎ»ÐÂÃÜÂë
							break;
						}

						if(strcmp(m_pEdit[1]->GetString(), m_pEdit[2]->GetString()) != 0 )
						{
							DPPostMessage(MSG_SHOW_STATUE, 2008, 0, 0);	//Á½´ÎÊäÈëµÄÐÂÃÜÂëÒ»ÖÂ
							break;
						}		

						SetProjectPwd(m_pEdit[1]->GetString());
						DPPostMessage(MSG_SHOW_STATUE, 2004, 0, 0);
						DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
						DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
						return TRUE;
					}while(0);

					m_nFocus = 0;
					m_pEdit[0]->SetString("");
					m_pEdit[1]->SetString("");
					m_pEdit[2]->SetString("");
				}
				else
				{
					m_pEdit[m_nFocus]->Input(lParam);
				}
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prjpwd.xml");
		GetCtrlByName("back", &m_idBack);
		m_pEdit[0] = (CEditBox*)GetCtrlByName("oldPwd", &m_idEdit[0]);
		m_pEdit[1] = (CEditBox*)GetCtrlByName("newPwd", &m_idEdit[1]);
		m_pEdit[2] = (CEditBox*)GetCtrlByName("newPwd2", &m_idEdit[2]);

		m_nFocus = 0;
		m_pEdit[0]->SetIsPwd(TRUE);
		m_pEdit[0]->SetMaxLen(6);
		m_pEdit[0]->SetString("");

		m_pEdit[1]->SetIsPwd(TRUE);
		m_pEdit[1]->SetMaxLen(6);
		m_pEdit[1]->SetString("");

		m_pEdit[2]->SetIsPwd(TRUE);
		m_pEdit[2]->SetMaxLen(6);
		m_pEdit[2]->SetString("");

		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_nFocus;
	DWORD m_idEdit[3];		//0¾ÉÃÜÂë 1ÐÂÃÜÂë 2ÐÂÃÜÂë2
	CEditBox* m_pEdit[3];
};

CAppBase* CreatePrjPwdApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjPwdApp* pApp = new CPrjPwdApp(wParam);
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