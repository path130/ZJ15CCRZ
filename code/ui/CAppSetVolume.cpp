#include "CCtrlModules.h"

class CSetVolumeApp : public CAppBase
{
public:
	CSetVolumeApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetVolumeApp(void)
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
			else if(wParam == m_idOK)
			{
				SetRingVol(m_dwRingVol);
				SetTalkVol(m_dwTalkVol);
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idRingAdd)
			{
				if(m_dwRingVol < 0xF)
				{
					m_pRingVol->SetSrc(++m_dwRingVol);
					m_pRingVol->Show(TRUE);
				}
			}
			else if(wParam == m_idRingSub)
			{
				if(m_dwRingVol > 0)
				{
					m_pRingVol->SetSrc(--m_dwRingVol);
					m_pRingVol->Show(TRUE);
				}
			}
			else if(wParam == m_idTalkAdd)
			{
				if(m_dwTalkVol < 0xF)
				{
					m_pTalkVol->SetSrc(++m_dwTalkVol);
					m_pTalkVol->Show(TRUE);
				}
			}
			else if(wParam == m_idTalkSub)
			{
				if(m_dwTalkVol > 0)
				{
					m_pTalkVol->SetSrc(--m_dwTalkVol);
					m_pTalkVol->Show(TRUE);
				}
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setvolume.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("ok", &m_idOK);
		GetCtrlByName("ringVolAdd", &m_idRingAdd);
		GetCtrlByName("ringVolSub", &m_idRingSub);
		GetCtrlByName("talkVolAdd", &m_idTalkAdd);
		GetCtrlByName("talkVolSub", &m_idTalkSub);
		m_pRingVol = (CDPStatic*)GetCtrlByName("ringVol");
		m_pTalkVol = (CDPStatic*)GetCtrlByName("talkVol");

		m_dwRingVol = GetRingVol();
		m_pRingVol->SetSrc(m_dwRingVol);
		m_pRingVol->Show(TRUE);

		m_dwTalkVol = GetTalkVol();
		m_pTalkVol->SetSrc(m_dwTalkVol);
		m_pTalkVol->Show(TRUE);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idOK;
	DWORD m_idRingAdd;
	DWORD m_idRingSub;
	DWORD m_idTalkAdd;
	DWORD m_idTalkSub;
	CDPStatic* m_pRingVol;
	CDPStatic* m_pTalkVol;

	DWORD m_dwRingVol;
	DWORD m_dwTalkVol;
};

CAppBase* CreateSetVolumeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetVolumeApp* pApp = new CSetVolumeApp(wParam);
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