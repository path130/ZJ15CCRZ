#include "CCtrlModules.h"
#include "PhoneApp.h"
#include "ipfunc.h"

static char g_callcode[16];

class CCallRoomApp : public CAppBase
{
public:
	CCallRoomApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CCallRoomApp(void)
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
			break;
		case KBD_MESSAGE:
			if(wParam == KBD_CTRL)
			{
				if(lParam == '-')
				{
					if(m_pos == 0)
						break;

					m_string[m_pos - 1] = '-';
					m_pos--;
					ShowString();
				}
				else if(lParam == '*')
				{
					if(m_pos < 10)
					{
						DPPostMessage(MSG_SHOW_STATUE, 2002, 0, 0);
						break;
					}
					else
					{
						ip_get pGet;
						char termid[16] = {0};
						sprintf(termid, "1%s01", m_string);

						char lastid[16];
						GetTermId(lastid);
						if(strncmp(lastid, termid, 11) == 0)
						{
							DPPostMessage(MSG_SHOW_STATUE, 2006, 0, 0);		//此房号为本机房号！
							break;
						}

						if(!TermGet(&pGet, termid))
						{
							DPPostMessage(MSG_SHOW_STATUE, 2003, 0, 0);		//房号不存在
							break;
						}
						free(pGet.param);

						strcpy(g_callcode, termid);
						if(RequestTalk())
							DPPostMessage(MSG_START_FROM_ROOT, TALKING_APPID, GetPhonePkt(ROOM_TYPE, g_callcode), 0);
						else
							DPPostMessage(MSG_SHOW_STATUE, 606, 0, 0);	//	正在通话中
					}
				}
				else
				{
					if(m_pos < 10)
					{
						m_string[m_pos] = lParam;
						m_pos++;
						ShowString();
					}
				}
			}
			break;
		}
		return TRUE;
	}

	void ShowString()
	{
		char buf[64];
		sprintf(buf, "%c%c%s%c%c%s%c%c%s%c%c%c%c%s", 
			m_string[0], m_string[1], GetStringByID(617), 
			m_string[2], m_string[3], GetStringByID(618), 
			m_string[4], m_string[5], GetStringByID(619), 
			m_string[6], m_string[7], m_string[8], m_string[9], GetStringByID(620));

		m_pCode->SetSrc(buf);
		m_pCode->Show(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("callroom.xml");
		GetCtrlByName("back", &m_idBack);
		m_pCode = (CDPStatic*)GetCtrlByName("roomCode");

		GetTermId(m_string);
		memmove(m_string, &m_string[1], 13);
		m_string[10] = 0;
		m_pos = 10;
		ShowString();

		return TRUE;
	}
private:
	DWORD m_idBack;
	CDPStatic* m_pCode;
	char m_string[16];
	int m_pos;
};

CAppBase* CreateCallRoomApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CCallRoomApp* pApp = new CCallRoomApp(wParam);
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