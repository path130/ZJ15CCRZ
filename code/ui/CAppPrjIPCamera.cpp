#include "CCtrlModules.h"

#define IPC_IDLE			0
#define IPC_INPUT_NAME		1
#define IPC_INPUT_IP		2
#define IPC_INPUT_USER		3
#define IPC_INPUT_PASSWD	4
#define IPC_DELETE			5

class CPrjIPCameraApp : public CAppBase
{
public:
	CPrjIPCameraApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPrjIPCameraApp(void)
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
				DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idAdd)
			{
				if(m_nStage == IPC_IDLE)
				{
					if(GetIPCameraCount() == MAX_IPCAMERA_NUMBER)
						break;

					m_pImeTip->SetSrc(GetStringByID(90017));		// 请输入名称
					m_pImeTip->Show(TRUE);
					m_pImeEdit->SetString("");
					m_pImeEdit->SetMaxLen(6);
					m_pIme->SwitchLay(TRUE);
					m_nStage = IPC_INPUT_NAME;
				}
			}
			else if(wParam == m_idDelete)
			{
				if(m_nFocus < GetIPCameraCount())
				{
					char buf[32];
					sprintf(buf, "%s%s?", GetStringByID(2005), GetStringByID(1019));	// 确认删除？
					m_pDlgTip->SetSrc(buf);
					m_pDlgTip->Show(TRUE);
					m_pDlg->SwitchLay(TRUE);
					m_nStage = IPC_DELETE;
				}
			}
			else if(wParam == m_idOK)
			{
				if(m_nFocus < GetIPCameraCount())
				{
					DeleteIPCamera(m_nFocus);
					ShowIPCameraList();
					m_pDlg->SwitchLay(FALSE);
					m_nStage = IPC_IDLE;
				}
			}
			else if(wParam == m_idCancel)
			{
				m_pDlg->SwitchLay(FALSE);
				m_nStage = IPC_IDLE;
			}
			else if(wParam == m_idImeBack)
			{
				m_pIme->SwitchLay(FALSE);
			}
			else if(wParam >= m_idIPCamera[0] && wParam <= m_idIPCamera[5])
			{
				if(m_nFocus < MAX_IPCAMERA_NUMBER)
					m_pIPCamera[m_nFocus]->Show(STATUS_NORMAL);
				m_nFocus = wParam - m_idIPCamera[0];
			}
			break;
		case KBD_MESSAGE:
			if(wParam == KBD_CTRL)
			{
				if(lParam == 0x08)	// 退格
					m_pImeEdit->Delete();
				else if(lParam == 0x0d)	// 确认
					InputProc();
				else
					m_pImeEdit->Input(lParam);
			}
			break;
		}
		return TRUE;
	}

	void InputProc()
	{
		if(m_pImeEdit->GetCurCount() == 0)
		{
			DPPostMessage(MSG_SHOW_STATUE, 2028, 0, 0);	// 输入不能为空
			return;
		}

		if(m_nStage == IPC_INPUT_NAME)
		{
			strcpy(m_item.name, m_pImeEdit->GetString());
			m_pImeEdit->SetString("");
			m_pImeEdit->SetMaxLen(15);
			m_pImeTip->SetSrc(GetStringByID(90027));		// 请输入设备IP地址
			m_pImeTip->Show(TRUE);
			m_nStage = IPC_INPUT_IP;
		}
		else if(m_nStage == IPC_INPUT_IP)
		{
			strcpy(m_item.ip, m_pImeEdit->GetString());
			if(!dp_inet_addr(m_item.ip, NULL))
			{
				DPPostMessage(MSG_SHOW_STATUE, 2029, 0, 0);	// 请输入正确的IP地址，如x.x.x.x
				return;
			}

			m_pImeEdit->SetString("");
			m_pImeEdit->SetMaxLen(15);
			m_pImeTip->SetSrc(GetStringByID(90037));		// 请输入用户名
			m_pImeTip->Show(TRUE);
			m_nStage = IPC_INPUT_USER;
		}
		else if(m_nStage == IPC_INPUT_USER)
		{
			strcpy(m_item.user, m_pImeEdit->GetString());
			m_pImeEdit->SetString("");
			m_pImeEdit->SetMaxLen(15);
			m_pImeTip->SetSrc(GetStringByID(90047));		// 请输入密码
			m_pImeTip->Show(TRUE);
			m_nStage = IPC_INPUT_PASSWD;
		}
		else if(m_nStage == IPC_INPUT_PASSWD)
		{
			strcpy(m_item.pwd, m_pImeEdit->GetString());
			AddIPCamera(m_item.name, m_item.ip, m_item.user, m_item.pwd);

			ShowIPCameraList();
			m_pIme->SwitchLay(FALSE);
		}
	}

	void ShowIPCameraList()
	{
		IPCameraRecord item;
		int count = GetIPCameraCount();
		int i = 0;
		for(; i < count; i++)
		{
			GetIPCameraData(i, &item);
			m_pIPCamera[i]->SetSrcText(item.name);
			m_pIPCamera[i]->Show(STATUS_NORMAL);
			m_pIpText[i]->SetSrc(item.ip);
			m_pIpText[i]->Show(TRUE);
		}
		for(; i < 6; i++)
		{
			m_pIPCamera[i]->Hide();
			m_pIpText[i]->Show(FALSE);
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prjipcamera.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("add", &m_idAdd);
		GetCtrlByName("delete", &m_idDelete);

		m_pIPCamera[0] = (CDPButton*)GetCtrlByName("button1", &m_idIPCamera[0]);
		m_pIPCamera[1] = (CDPButton*)GetCtrlByName("button2", &m_idIPCamera[1]);
		m_pIPCamera[2] = (CDPButton*)GetCtrlByName("button3", &m_idIPCamera[2]);
		m_pIPCamera[3] = (CDPButton*)GetCtrlByName("button4", &m_idIPCamera[3]);
		m_pIPCamera[4] = (CDPButton*)GetCtrlByName("button5", &m_idIPCamera[4]);
		m_pIPCamera[5] = (CDPButton*)GetCtrlByName("button6", &m_idIPCamera[5]);

		m_pIpText[0] = (CDPStatic*)GetCtrlByName("ip1");
		m_pIpText[1] = (CDPStatic*)GetCtrlByName("ip2");
		m_pIpText[2] = (CDPStatic*)GetCtrlByName("ip3");
		m_pIpText[3] = (CDPStatic*)GetCtrlByName("ip4");
		m_pIpText[4] = (CDPStatic*)GetCtrlByName("ip5");
		m_pIpText[5] = (CDPStatic*)GetCtrlByName("ip6");

		m_pDlg = (CLayOut*)GetCtrlByName("layout_ask");
		m_pDlgTip = (CDPStatic*)GetCtrlByName("ask_tip");
		GetCtrlByName("ask_cancle", &m_idCancel);
		GetCtrlByName("ask_ok", &m_idOK);

		m_pIme = (CLayOut*)GetCtrlByName("layout_input");
		GetCtrlByName("input_back", &m_idImeBack);
		m_pImeTip = (CDPStatic*)GetCtrlByName("input_tip");
		m_pImeEdit = (CEditBox*)GetCtrlByName("editbox");

		m_nStage = IPC_IDLE;
		m_nFocus = MAX_IPCAMERA_NUMBER;
		ShowIPCameraList();
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idAdd;
	DWORD m_idDelete;
	DWORD m_idIPCamera[6];
	CDPButton* m_pIPCamera[6];
	CDPStatic* m_pIpText[6];

	// 确定删除
	CLayOut* m_pDlg;
	CDPStatic* m_pDlgTip;
	DWORD m_idOK;
	DWORD m_idCancel;
	BOOL m_bDeleteAll;

	// 中英文输入法
	CLayOut* m_pIme;
	DWORD m_idImeBack;
	CDPStatic* m_pImeTip;
	CEditBox* m_pImeEdit;

	DWORD m_nFocus;
	DWORD m_nStage;
	IPCameraRecord m_item;
};

CAppBase* CreatePrjIPCameraApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjIPCameraApp* pApp = new CPrjIPCameraApp(wParam);
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