#include "CCtrlModules.h"
#include "PhoneApp.h"
#include "ipfunc.h"

static char g_callcode[16];

class CCallLineApp : public CAppBase
{
public:
	CCallLineApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CCallLineApp(void)
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
				DPPostMessage(MSG_START_APP, CALL_MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else
			{
				for(int i = 0; i < 6; i++)
				{
					if(wParam == m_idButton[i])
					{
						strcpy(g_callcode, m_callcode[i]);
						if(!RequestTalk())
							DPPostMessage(MSG_SHOW_STATUE, 606, 0, 0);	//	正在通话中
						else
							DPPostMessage(MSG_START_FROM_ROOT, TALKING_APPID, GetPhonePkt(ROOM_TYPE, g_callcode), 0);
						break;
					}
				}
			}
			break;
		}
		return TRUE;
	}

	void InitRoomList()
	{
		// 暂时只支持6个分机显示

		char termId[16];
		ip_get pGet;
		int count = 0;
		char buf[32];

		GetTermId(termId);
		termId[11] = 0;
		if(RoomGet(&pGet, &termId[7]))
		{
			for(int i = 0; i < pGet.num && count < 6; i++)	
			{
				if(pGet.param[i].code[12] == termId[12])
					continue;

				sprintf(buf, "%d%s%s", i + 1, GetStringByID(613), GetStringByID(621));
				m_pButton[count]->SetSrcText(buf);
				m_pButton[count]->Show(STATUS_NORMAL);
				strcpy(m_callcode[count], pGet.param[i].code);
				count++;
			}

			free(pGet.param);
		}

		if(count == 0)
		{
			m_pTip->SetSrc(GetStringByID(2021));
			m_pTip->Show(TRUE);
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("callline.xml");
		GetCtrlByName("back", &m_idBack);
		m_pButton[0] = (CDPButton*)GetCtrlByName("monitor1", &m_idButton[0]);
		m_pButton[1] = (CDPButton*)GetCtrlByName("monitor2", &m_idButton[1]);
		m_pButton[2] = (CDPButton*)GetCtrlByName("monitor3", &m_idButton[2]);
		m_pButton[3] = (CDPButton*)GetCtrlByName("monitor4", &m_idButton[3]);
		m_pButton[4] = (CDPButton*)GetCtrlByName("monitor5", &m_idButton[4]);
		m_pButton[5] = (CDPButton*)GetCtrlByName("monitor6", &m_idButton[5]);
		m_pTip = (CDPStatic*)GetCtrlByName("tip");

		InitRoomList();
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idButton[6];
	CDPButton* m_pButton[6];
	CDPStatic* m_pTip;

	char m_callcode[6][16];
};

CAppBase* CreateCallLineApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CCallLineApp* pApp = new CCallLineApp(wParam);
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