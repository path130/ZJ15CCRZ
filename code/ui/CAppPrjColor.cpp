#include "CCtrlModules.h"

class CPrjColorApp : public CAppBase
{
public:
	CPrjColorApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPrjColorApp(void)
	{
	}

	BOOL DoPause()
	{
		SetScreenOnOff(TRUE);
		return CAppBase::DoPause();
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
				DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idSave)
			{
				SetDisplayParam(m_dwVal);
				AdjustCscparam(m_dwVal[3], m_dwVal[4], m_dwVal[5], m_dwVal[6]);
				DPPostMessage(MSG_SHOW_STATUE, 2004, 0, 0);
				DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else
			{
				for(int i = 0; i < 6; i++)
				{
					if(wParam == m_idAdd[i])
					{
						m_dwVal[i + 1] = (m_dwVal[i + 1] + 1) % 100;
						m_pVal[i]->SetSrc(m_dwVal[i + 1]);
						m_pVal[i]->Show(TRUE);

						if(i < 2)
							AdjustScreen(m_dwVal[0], m_dwVal[1], m_dwVal[2]);
						break;
					}

					if(wParam == m_idSub[i])
					{
						if(m_dwVal[i + 1] == 0)
							m_dwVal[i + 1] = 99;
						else
							m_dwVal[i + 1]--;
						m_pVal[i]->SetSrc(m_dwVal[i + 1]);
						m_pVal[i]->Show(TRUE);

						if(i < 2)
							AdjustScreen(m_dwVal[0], m_dwVal[1], m_dwVal[2]);
						break;
					}
				}
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prjcolor.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("save", &m_idSave);

		GetDisplayParam(m_dwVal);
		char buf1[6][32] = {"screenContrastAdd", "screenSaturationAdd", "videoHueadd", "videoBrightadd", "videoContrastadd", "videoSaturationadd"};
		char buf2[6][32] = {"screenContrastSub", "screenSaturationSub", "videoHuesub", "videoBrightsub", "videoContrastsub", "videoSaturationsub"};
		char buf3[6][32] = {"screenContrast", "screenSaturation", "videoHue", "videoBright", "videoContrast", "videoSaturation"};
		for(int i = 0; i < 6; i++)
		{
			GetCtrlByName(buf1[i], &m_idAdd[i]);
			GetCtrlByName(buf2[i], &m_idSub[i]);
			m_pVal[i] = (CDPStatic*)GetCtrlByName(buf3[i]);
			m_pVal[i]->SetSrc(m_dwVal[i + 1]);
			m_pVal[i]->Show(TRUE);
		}
		
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idSave;
	DWORD m_idAdd[6];
	DWORD m_idSub[6];
	CDPStatic* m_pVal[6];
	DWORD m_dwVal[7];
};

CAppBase* CreatePrjColorApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjColorApp* pApp = new CPrjColorApp(wParam);
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