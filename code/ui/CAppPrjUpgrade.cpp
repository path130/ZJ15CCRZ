#include "CCtrlModules.h"

class CPrjUpgradeApp : public CAppBase
{
public:
	CPrjUpgradeApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPrjUpgradeApp(void)
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
				DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOK)
			{
				switch(m_nFocus)
				{
				case 0:
					{
						char fileName[3][128];
						sprintf(fileName[0], "%s/%s", USERDIR, IPTABLE_NAME);
						sprintf(fileName[1], "%s/%s", USERDIR, IPTABLE_BAK);
						sprintf(fileName[2], "%s/%s", SDCARD, IPTABLE_NAME);

						if(-1 == DPGetFileAttributes(fileName[2]))
							DPPostMessage(MSG_SHOW_STATUE, 2031, 0, 0);
						else
						{
							DPDeleteFile(fileName[1]);
							DPMoveFile(fileName[1], fileName[0]);
							DPCopyFile(fileName[0], fileName[2]);
							DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
						}
					}
					break;
				case 1:
					{
						char fileName[MAX_PATH];
						sprintf(fileName, "%s/image.dd", SDCARD);
						if(-1 == DPGetFileAttributes(fileName))
							DPPostMessage(MSG_SHOW_STATUE, 2030, 0, 0);
						else
						{
							SetObjectMemorySpace_GWF(IMAGE_OBJECT_SIZE + NORMAL_OBJECT_SIZE);
							if(CheckSpareSpace(WINDOWSDIR) < IMAGE_OBJECT_SIZE)
							{
								// ÄÚ´æ²»×ã
								SetObjectMemorySpace_GWF(NORMAL_OBJECT_SIZE);
								break;
							}

							DPCopyFile(DOWNLOAD_IMAGE_PATH, fileName);
							DPUnloadFile(SDCARD);
							DPPostMessage(MSG_START_FROM_ROOT, UPGRATE_APPID, 0, 0, TRUE);
						}
					}
					break;
				case 2:
					break;
				}
			}
			else if(wParam == m_idButton[0] || wParam == m_idButton[1])
			{
				m_nFocus = (wParam == m_idButton[1]);
				m_pButton[m_nFocus]->Show(STATUS_FOCUS);
				m_pButton[1 - m_nFocus]->Show(STATUS_NORMAL);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prjupgrade.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("ok", &m_idOK);
		m_pButton[0] = (CDPButton*)GetCtrlByName("netcfg", &m_idButton[0]);
		m_pButton[1] = (CDPButton*)GetCtrlByName("image", &m_idButton[1]);
		m_nFocus = 2;
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idOK;
	DWORD m_idButton[2];
	CDPButton* m_pButton[2];
	DWORD m_nFocus;
};

CAppBase* CreatePrjUpgradeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjUpgradeApp* pApp = new CPrjUpgradeApp(wParam);
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