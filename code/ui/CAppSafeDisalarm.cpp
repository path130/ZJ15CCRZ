#include "CCtrlModules.h"

class CSafeDisalarmApp : public CAppBase
{
public:
	CSafeDisalarmApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSafeDisalarmApp(void)
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
				DPPostMessage( MSG_START_APP, SAFE_MAIN_APPID, 0, 0 );
				DPPostMessage( MSG_END_APP, (DWORD)this, 0, 0 );
			}
			break;
		case KBD_MESSAGE:
			if(wParam == KBD_CTRL)
			{
				if( lParam == '-' )
					m_pEditPwd->Delete();
				else if(lParam == '*')
				{
					char pwd[16];
					char hostage[16];
					GetSafePwd(pwd);
					GetHostagePwd(hostage);
					int ret = 0;
					if(strcmp(pwd, m_pEditPwd->GetString()) == 0)
						ret = 1;
					else if(strcmp(hostage, m_pEditPwd->GetString()) == 0)
						ret = 2;
		
					if(ret == 0)
					{
						// ÃÜÂë´íÎó
						DPPostMessage( MSG_SHOW_STATUE, 2001, 0, 0 );
						m_pEditPwd->SetString("");
					}
					else
					{
						
						// ³··À
						SetDefenseStatus(FALSE);
						AddDefenseRecord(SMODE_UNSET, SMODE_LEAVE);
						if(ret == 1)
							PostSafeMessage(SMSG_DISALARM, SAFE_MAX_NUMBER, 0, 0);
						else
							PostSafeMessage(SMSG_HOSTAGE, SAFE_MAX_NUMBER, 0, 0);
						DPPostMessage(MSG_BROADCAST, SAFE_CHANGE, FALSE, 0);
						//SendSyncData(SYNC_TYPE_CANCEL_DEFENSE, NULL, 0);

						// ³··À³É¹¦£¡
						DPPostMessage(MSG_SHOW_STATUE, 2015, 0, 0);
						DPPostMessage(MSG_START_APP, SAFE_MAIN_APPID, 0, 0);
						DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
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
		m_pTitle->SetSrc(GetStringByID(60001));
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

CAppBase* CreateSafeDisalarmApp(DWORD pHwnd, DWORD lParam, DWORD zParam)
{
	CSafeDisalarmApp* pMain;
	pMain = new CSafeDisalarmApp(pHwnd);
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