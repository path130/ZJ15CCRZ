#include "CCtrlModules.h"
#include "dpsession.h"

class CUpgradeApp : public CAppBase
{
public:
	CUpgradeApp(DWORD hWnd) : CAppBase(hWnd)
	{

	}

	~CUpgradeApp()
	{

	}

	BOOL DoPause()
	{
		return FALSE;
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		char buf[8];
		switch(uMsg)
		{
		case MSG_PRIVATE:
			if(wParam == m_IdBase)
			{
				switch(lParam)
				{
				case 0:
					DPThreadJoin(m_hThread);
					switch(zParam)
					{
					case UPDATE_FILE_OK:
						DBGMSG(DPINFO, "Update Image OK!\n");
						DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
						break;
					case UPDATE_FILE_NOT_NEW:
						DPDeleteFile(DOWNLOAD_IMAGE_PATH);
						SetObjectMemorySpace_GWF(1*1024*1024);
						SetIPAddress(GetLocalIp(), GetLocalMask(), GetLocalGw());
						DPSessionUserOp(DPMSG_IP_CHANGE, 0, GetLocalIp());
						DPPostMessage(MSG_END_APP, (DWORD)this, m_IdBase, 0, 0);
						break;
					default:
						DPDeleteFile(DOWNLOAD_IMAGE_PATH);
						SetObjectMemorySpace_GWF(1*1024*1024);
						SetIPAddress(GetLocalIp(), GetLocalMask(), GetLocalGw());
						DPSessionUserOp(DPMSG_IP_CHANGE, 0, GetLocalIp());
						DPPostMessage(MSG_END_APP, (DWORD)this, m_IdBase, 0, 0);
						break;
					}
					break;
				case 1:
					sprintf(buf, "%d%%", zParam);
					m_percent->SetSrc(buf);
					m_percent->Show(TRUE);
					break;
				}
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;	
	}

	static DWORD UpdataAppStub(HANDLE param)
	{
		CUpgradeApp* pApp = (CUpgradeApp*)param;
		pApp->UpdateApp(param);
		return 0;
	}

	void UpdateApp(HANDLE param)
	{
		char szFileName[MAX_PATH] = {0};
		sprintf(szFileName,"%s", DOWNLOAD_IMAGE_PATH);
		StartUpdate(szFileName, m_IdBase);
		DBGMSG(DPINFO, "UpdateApp Exit\r\n");
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("upgrade.xml");

		int ip = inet_addr("127.0.0.1");
		SetIPAddress(ip, inet_addr("255.255.255.0"), ip);
		while(1)
		{
			// 等待IP设置完毕再升级dd，不然会升级失败
			DPSleep(2000);
			if(ip == GetIpAddress())
				break;
		}

		m_percent = (CDPStatic*)GetCtrlByName("percent");
		m_percent->SetSrc("0%");
		m_percent->Show(TRUE);
		m_hThread = DPThreadCreate(0x4000, UpdataAppStub, this, TRUE, 3);

		return TRUE;
	}

private:
	HANDLE m_hThread;
	CDPStatic* m_percent;
};

CAppBase* CreateUpgradeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CUpgradeApp* pApp = new CUpgradeApp(wParam);
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