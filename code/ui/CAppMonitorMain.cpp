#include "CCtrlModules.h"
#include "ipfunc.h"
#include "PhoneApp.h"

#define MTYPE_DOOR		0
#define MTYPE_HOUSE		1	
#define MTYPE_IPC		2
#define MTYPE_MAX		3

class CMonitorMainApp : public CAppBase
{
public:
	CMonitorMainApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CMonitorMainApp(void)
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
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
				DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam >= m_idPage[0] && wParam <= m_idPage[2])
			{
				OnPage(wParam - m_idPage[0]);
			}
			else if(wParam >= m_idObject[0] && wParam <= m_idObject[5])
			{
				int index = wParam - m_idObject[0];
				switch(m_nPage)
				{
				case MTYPE_DOOR:
				case MTYPE_HOUSE:
					DPPostMessage(MSG_START_FROM_ROOT, TALKING_APPID, GetPhonePkt(MONITOR_TYPE, m_strCode[m_nPage][index]), 0);
					break;
				case MTYPE_IPC:
					DPPostMessage(MSG_START_FROM_ROOT, IPC_MONITOR_APPID, index, 0);
					break;
				default:
					break;
				}
			}
			break;
		}

		return TRUE;
	}

	void InitMonitorList()
	{
		int nLanuage = GetLanguage();

		ip_get pGet = {0};
		CellDoorGet(&pGet);
		m_dwCount[MTYPE_DOOR] = pGet.num;
		for(int i = 0; i < pGet.num; i++)
		{
			if(nLanuage == LANGUAGE_CH)	
				sprintf(m_strName[MTYPE_DOOR][i], "%d%s%s", i + 1, GetStringByID(613), GetStringByID(614));		// 1号门口机
			else	
				sprintf(m_strName[MTYPE_DOOR][i], "%s%d%s", GetStringByID(613), i + 1, GetStringByID(614));		// No.1Door

			strcpy(m_strCode[MTYPE_DOOR][i], pGet.param[i].code);
		}
		if(pGet.param)
		{
			free(pGet.param);
			pGet.param = NULL;
		}

		SecDoorGet(&pGet);
		m_dwCount[MTYPE_HOUSE] = pGet.num;
		for(int i = 0; i < pGet.num; i++)
		{
			if(nLanuage == LANGUAGE_CH)
				sprintf(m_strName[MTYPE_HOUSE][i], "%d%s%s", i + 1, GetStringByID(613), GetStringByID(615));	// 1号别墅机
			else	
				sprintf(m_strName[MTYPE_HOUSE][i], "%s%d%s", GetStringByID(613), i + 1, GetStringByID(615));	// No.1Villa

			strcpy(m_strCode[MTYPE_HOUSE][i], pGet.param[i].code);
		}
		if(pGet.param)
		{
			free(pGet.param);
			pGet.param = NULL;
		}

		IPCameraRecord item;
		m_dwCount[MTYPE_IPC] = GetIPCameraCount();
		for(int i = 0; i < m_dwCount[MTYPE_IPC]; i++ )
		{
			GetIPCameraData(i, &item);
			strcpy(m_strName[MTYPE_IPC][i], item.name);
		}
	}

	void OnPage(int nPage)
	{
		m_pPage[m_nPage]->Show(STATUS_NORMAL);
		m_nPage = nPage;
		m_pPage[nPage]->Show(STATUS_FOCUS);

		int i = 0;
		for(; i < 6 && i < m_dwCount[nPage]; i++)
		{
			m_pObject[i]->SetSrcText(m_strName[nPage][i]);
			m_pObject[i]->Show(STATUS_NORMAL);
		}
		for(; i < 6; i++)
		{
			m_pObject[i]->Hide();
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("monitormain.xml");
		GetCtrlByName("back", &m_idBack);
		m_pPage[0] = (CDPButton*)GetCtrlByName("door", &m_idPage[0]);
		m_pPage[1] = (CDPButton*)GetCtrlByName("house", &m_idPage[1]);
		m_pPage[2] = (CDPButton*)GetCtrlByName("ipcamera", &m_idPage[2]);
		m_pObject[0] = (CDPButton*)GetCtrlByName("monitor1", &m_idObject[0]);
		m_pObject[1] = (CDPButton*)GetCtrlByName("monitor2", &m_idObject[1]);
		m_pObject[2] = (CDPButton*)GetCtrlByName("monitor3", &m_idObject[2]);
		m_pObject[3] = (CDPButton*)GetCtrlByName("monitor4", &m_idObject[3]);
		m_pObject[4] = (CDPButton*)GetCtrlByName("monitor5", &m_idObject[4]);
		m_pObject[5] = (CDPButton*)GetCtrlByName("monitor6", &m_idObject[5]);

		m_nPage = 0;
		InitMonitorList();
		OnPage(lParam);

		return TRUE;
	}
private:
	DWORD m_idBack;

	DWORD m_idPage[MTYPE_MAX];
	CDPButton* m_pPage[MTYPE_MAX];
	int m_nPage;

	DWORD m_idObject[6];
	CDPButton* m_pObject[6];

	// monitor info
	DWORD m_dwCount[MTYPE_MAX];
	char m_strName[MTYPE_MAX][16][16];
	char m_strCode[MTYPE_MAX][16][16];
};

CAppBase* CreateMonitorMainApp(DWORD pHwnd, DWORD lParam, DWORD zParam)
{
	CMonitorMainApp* pMain;
	pMain = new CMonitorMainApp(pHwnd);
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



/************************************************************************/
/*                                                                      */
/************************************************************************/