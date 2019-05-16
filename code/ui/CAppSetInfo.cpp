#include "CCtrlModules.h"
#include "UIConfig.h"

class CSetInfoApp : public CAppBase
{
public:
	CSetInfoApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetInfoApp(void)
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
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setinfo.xml");
		GetCtrlByName("back", &m_idBack);
		char buf[128];
		for(int i = 0; i < 7; i++)
		{
			sprintf(buf, "tip%d", i + 1);
			m_pTip[i] = (CDPStatic*)GetCtrlByName(buf);
			sprintf(buf, "info%d", i + 1);
			m_pInfo[i] = (CDPStatic*)GetCtrlByName(buf);
		}

		char info[7][64];
		GetTermId(info[0]);
		int ip = GetLocalIp();
		sprintf(info[1], "%s", inet_ntoa(*(in_addr*)&ip));
		int mask = GetLocalMask();
		sprintf(info[2], "%s", inet_ntoa(*(in_addr*)&mask));
		int gw = GetLocalGw();
		sprintf(info[3], "%s", inet_ntoa(*(in_addr*)&gw));
		sprintf(info[4], "%d", GetNetCfgVersion());
		GetHardInfo(info[5]);
		GetCompanyInfo(GetLanguage(), info[6]);

		for(int i = 0; i < 7; i++)
		{
			m_pInfo[i]->SetStart(SETINFO_X + m_pTip[i]->GetWidth() + SETINFO_INTERVAL, m_pTip[i]->GetTop());
			m_pInfo[i]->SetSrc(info[i]);
			m_pInfo[i]->Show(TRUE);
		}

		return TRUE;
	}
private:
	DWORD m_idBack;

	CDPStatic* m_pTip[7];
	CDPStatic* m_pInfo[7];

};

CAppBase* CreateSetInfoApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetInfoApp* pApp = new CSetInfoApp(wParam);
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