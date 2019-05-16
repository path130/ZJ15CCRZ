#include "CCtrlModules.h"
extern BOOL GetOnVifStr(DWORD ip, char* user, char* pwd, char* string);

class CIPCMonitorApp : public CAppBase
{
public:
	CIPCMonitorApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CIPCMonitorApp(void)
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch( uMsg )
		{
		case TOUCH_MESSAGE:
			if(wParam == m_idBottomBtn[0])
			{
				DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idBottomBtn[1])
			{
				
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lparam, DWORD zParam)
	{
		InitFrame("talking.xml");

		m_pBottomBtn[0] = (CDPButton*)GetCtrlByName("bottom1", &m_idBottomBtn[0]);
		m_pBottomBtn[1] = (CDPButton*)GetCtrlByName("bottom2", &m_idBottomBtn[1]);

		m_pBottomBtn[0]->SetSrcText(GetStringByID(1027));	// ·µ»Ø
		m_pBottomBtn[0]->Show(STATUS_NORMAL);
		m_pBottomBtn[1]->SetSrcText(GetStringByID(1050));	// ÅÄÕÕ
		m_pBottomBtn[1]->Show(STATUS_NORMAL);

		//char* pbuf = (char*)malloc(0x10000);
		//GetOnVifStr(inet_addr("192.168.1.64"), "admin", "ASDF1234", pbuf);
		//free(pbuf);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idBottomBtn[2];
	CDPButton* m_pBottomBtn[2];		// µ×²¿Á½¸ö°´Å¥£¬¹Ò¶Ï¡¢½ÓÌý¡¢ÅÄÕÕ	
};

CAppBase* CreateIPCMonitorApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CIPCMonitorApp* pApp = new CIPCMonitorApp(wParam);
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