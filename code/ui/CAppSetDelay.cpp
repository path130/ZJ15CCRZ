#include "CCtrlModules.h"

class CSetDelayApp : public CAppBase
{
public:
	CSetDelayApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetDelayApp(void)
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
			else if(wParam == m_idSave)
			{
				// 有改变的就写文件保存
				SetDelay(m_dwDelay);
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else
			{
				for(int i = 0; i < DELAY_MAX; i++)
				{
					if(wParam == m_idDelayAdd[i])
					{
						ChangeDelay(i, TRUE);
						break;
					}
					if(wParam == m_idDelaySub[i])
					{
						ChangeDelay(i, FALSE);
						break;
					}
				}
			}
			break;
		}
		return TRUE;
	}

	void ChangeDelay(int index, BOOL bAdd)
	{
		int option = 0;
		for(int i = 0; i < m_option[index]; i++)
		{
			if(m_dwDelay[index] == m_optionVal[index][i])
			{
				option = i;
				break;
			}
		}

		if(bAdd 
			&& (option < m_option[index] - 1))
			option++;
		else if(!bAdd
			&& (option > 0))
			option--;

		m_dwDelay[index] = m_optionVal[index][option];

		char buf[32];
		if(index == DELAY_DURATION)
			sprintf(buf, "%d%s", m_dwDelay[index], GetStringByID(1044));	// 分
		else
			sprintf(buf, "%d%s", m_dwDelay[index], GetStringByID(1043));	// 秒
		m_pDelay[index]->SetSrc(buf);
		m_pDelay[index]->Show(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setdelay.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("save", &m_idSave);

		GetDelay(m_dwDelay);

		char buf[32];
		char buf1[DELAY_MAX][32] = {"safeadd", "alarmadd", "alarmdurationadd", "callinadd", "screensaveradd", "unlockadd"};
		char buf2[DELAY_MAX][32] = {"safesub", "alarmsub", "alarmdurationsub", "callinsub", "screensaversub", "unlocksub"};
		char buf3[DELAY_MAX][32] = {"safetip", "alarmtip", "alarmdurationtip", "callintip", "screensavertip", "unlocktip"};
		for(int i = 0; i < DELAY_MAX; i++)
		{
			GetCtrlByName(buf1[i], &m_idDelayAdd[i]);
			GetCtrlByName(buf2[i], &m_idDelaySub[i]);
			m_pDelay[i] = (CDPStatic*)GetCtrlByName(buf3[i]);

			if(i == DELAY_DURATION)
				sprintf(buf, "%d%s", m_dwDelay[i], GetStringByID(1044));	// 分
			else
				sprintf(buf, "%d%s", m_dwDelay[i], GetStringByID(1043));	// 秒
			m_pDelay[i]->SetSrc(buf);
			m_pDelay[i]->Show(TRUE);
		}

		DWORD option[DELAY_MAX] = {3, 3, 3, 3, 3, 4};
		DWORD optionVal[DELAY_MAX][6] = {
			{30, 60, 99, -1, -2, -3},
			{0, 30, 60, -1, -2, -3}, 
			{3, 5, 10, -1, -2, -3}, 
			{30, 60, 90, -1, -2, -3}, 
			{30, 60, 90, -1, -2, -3}, 
			{1, 5, 10, 15, -1, -2}};
		
		memcpy(m_option, option, sizeof(m_option));
		memcpy(m_optionVal, optionVal, sizeof(m_optionVal));
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idSave;
	CDPStatic* m_pDelay[DELAY_MAX];
	DWORD m_idDelayAdd[DELAY_MAX];
	DWORD m_idDelaySub[DELAY_MAX];

	DWORD m_dwDelay[DELAY_MAX];
	DWORD m_option[DELAY_MAX];				// 每个延时有几个选项
	DWORD m_optionVal[DELAY_MAX][6];		// 每个选项的值
};

CAppBase* CreateSetDelayApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetDelayApp* pApp = new CSetDelayApp(wParam);
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