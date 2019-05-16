#include "CCtrlModules.h"
#include "UIConfig.h"

class CSetPwdApp : public CAppBase
{
public:
	CSetPwdApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetPwdApp(void)
	{
	}

	BOOL DoPause()
	{
		SetTouchDirect(FALSE);
		return CAppBase::DoPause();
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
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam >= m_idEdit[0] && wParam <= m_idEdit[2])
			{
				m_pEdit[m_nFocus]->Show(FALSE);
				m_nFocus = wParam - m_idEdit[0];
				m_pEdit[m_nFocus]->Show(TRUE);
			}
			break;
		case TOUCH_RAW_MESSAGE:
			if(zParam == TOUCH_UP)
			{
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		case KBD_MESSAGE:
			if(wParam == KBD_CTRL)
			{
				if(lParam == '-')
					m_pEdit[m_nFocus]->Delete();
				else if(lParam == '*')
				{
					if(PwdSetting(m_pEdit[0]->GetString(), m_pEdit[1]->GetString(), m_pEdit[2]->GetString()))
					{
						ShowTextPwd();
					}
					else
					{
						m_pEdit[0]->SetString("");
						m_pEdit[1]->SetString("");
						m_pEdit[2]->SetString("");
						m_nFocus = 0;
					}
				}
				else
					m_pEdit[m_nFocus]->Input(lParam);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setpwd.xml");
		GetCtrlByName("back", &m_idBack);
		m_pEdit[0] = (CEditBox*)GetCtrlByName("oldPwd", &m_idEdit[0]);
		m_pEdit[1] = (CEditBox*)GetCtrlByName("newPwd", &m_idEdit[1]);
		m_pEdit[2] = (CEditBox*)GetCtrlByName("newPwd2", &m_idEdit[2]);

		m_layoutText = (CLayOut*)GetCtrlByName("layout_text");
		m_pTip[0] = (CDPStatic*)GetCtrlByName("tip1");
		m_pTip[1] = (CDPStatic*)GetCtrlByName("tip2");
		m_pTip[2] = (CDPStatic*)GetCtrlByName("tip3");
		m_pTip[3] = (CDPStatic*)GetCtrlByName("tip4");
		m_pText[0] = (CDPStatic*)GetCtrlByName("text1");
		m_pText[1] = (CDPStatic*)GetCtrlByName("text2");
		m_pText[2] = (CDPStatic*)GetCtrlByName("text3");
		m_pText[3] = (CDPStatic*)GetCtrlByName("text4");

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

		char code[16];
		GetTermId(code);
		if(code[12] != '1')
		{
			//请在1号分机上设置！
			DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
			DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
		}

		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_nFocus;
	DWORD m_idEdit[3];		//0旧密码 1新密码 2新密码2
	CEditBox* m_pEdit[3];

	CLayOut* m_layoutText;
	CDPStatic* m_pTip[4];	
	CDPStatic* m_pText[4];

	BOOL PwdSetting(char* oldPwd, char* newPwd, char* newPwd2);
	void ShowTextPwd();
};

CAppBase* CreateSetPwdApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetPwdApp* pApp = new CSetPwdApp(wParam);
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



BOOL CSetPwdApp::PwdSetting(char* oldPwd, char* newPwd, char* newPwd2)
{
	char pwd[16];
	GetSafePwd(pwd);
	if(strcmp(oldPwd, pwd) != 0 )
	{
		DPPostMessage(MSG_SHOW_STATUE, 2001, 0, 0);	//原密码错误
		return FALSE;	
	}

	if( strlen(newPwd) != 6 || strlen(newPwd2) != 6 )
	{
		DPPostMessage(MSG_SHOW_STATUE, 2007, 0, 0);	//请输入六位新密码
		return FALSE;	
	}

	if(strcmp(newPwd, newPwd2) != 0 )
	{
		DPPostMessage(MSG_SHOW_STATUE, 2008, 0, 0);	//两次输入的新密码不一致
		return FALSE;	
	}

	char reversePwd[16] = {0};
	for(int i = strlen(newPwd) - 1; i >= 0; i--)
		reversePwd[5-i] = newPwd[i];

	if(strcmp(reversePwd, newPwd) == 0)
	{
		DPPostMessage(MSG_SHOW_STATUE, 2020, 0, 0);	//密码不合适，请换一个
		return FALSE;	
	}

	BOOL ret = FALSE;
	if( ChangeUserPwd(newPwd) )
		ret = ChangeHostagePwd(reversePwd);

	if(ret)
	{
		SetSafePwd(newPwd);
		SetHostagePwd(reversePwd);
		//SendSyncData(SYNC_TYPE_CHANGE_PASSWD, (char*)newPwd, wcslen(newPwd)*2);
	}
	else
		DPPostMessage(MSG_SHOW_STATUE, 2012, 0, 0);	//门口机不在线

	return ret;
}

void CSetPwdApp::ShowTextPwd()
{
	char userPwd[32];
	char hostagePwd[32];
	GetSafePwd(userPwd);
	GetHostagePwd(hostagePwd);

	for(int i = 0; i < 4; i++)
	{
		m_pText[i]->SetStart(SETPWD_X + m_pTip[i]->GetWidth() + SETPWD_INTERVAL, m_pTip[i]->GetTop());
		if((i % 2) == 0)
			m_pText[i]->SetSrc(userPwd);
		else
			m_pText[i]->SetSrc(hostagePwd);
		m_pText[i]->Show(TRUE);
	}

	m_layoutText->SwitchLay(TRUE);
	SetTouchDirect(TRUE);
}