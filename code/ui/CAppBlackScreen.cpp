#include "CCtrlModules.h"

class CBlackScreenApp : public CAppBase
{
public:
	CBlackScreenApp(DWORD hWnd) : CAppBase(hWnd)
	{

	}

	~CBlackScreenApp()
	{

	}

	BOOL DoPause(void)
	{
		SetScreenOnOff(TRUE);
		return CAppBase::DoPause();
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TOUCH_MESSAGE:
			DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
			break;
		case MSG_BROADCAST:
			if(wParam == ALARM_CHANGE && GetDefenseIsAlarming())
			{
				DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
			}
			break;
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("blackscreen.xml");
		SetScreenOnOff(FALSE);

		return TRUE;
	}

private:
};

CAppBase* CreateBlackScreenApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CBlackScreenApp* pApp = new CBlackScreenApp(wParam);
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