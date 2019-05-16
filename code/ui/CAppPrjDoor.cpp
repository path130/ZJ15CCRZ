#include "CCtrlModules.h"
#include "ipfunc.h"

class CPrjDoorApp : public CAppBase
{
public:
	CPrjDoorApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPrjDoorApp(void)
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
			else if(wParam == m_idCardAdd)
			{
				if( ReqDoorAddCard(SECOND_DOOR_TYPE) )
					DPPostMessage(MSG_SHOW_STATUE, 90063, 0, 0);
				else
					DPPostMessage(MSG_SHOW_STATUE, 2012, 0, 0);
			}
			else if(wParam == m_idCardRemove)
			{
				if( ReqDoorDelCard(SECOND_DOOR_TYPE) )
					DPPostMessage(MSG_SHOW_STATUE, 90073, 0, 0);
				else
					DPPostMessage(MSG_SHOW_STATUE, 2012, 0, 0);
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
					if(m_pos < 15)
					{
						DPPostMessage(MSG_SHOW_STATUE, 2002, 0, 0);
					}
					else
					{
						char setCode[16] = {0};
						setCode[0] = '3';
						strncpy(&setCode[1], m_string, 12);

						DWORD setDelay;
						setDelay = atol(&m_string[12]);

						if(setDelay < 1 || setDelay > 150)
						{
							DPPostMessage(MSG_SHOW_STATUE, 2034, 0, 0);		//开锁延时(1-150秒)！
							break;
						}

						ip_get pGet = {0};
						if(!GetDefaultTerm(ROOM_TYPE, &pGet))
						{
							DPPostMessage(MSG_SHOW_STATUE, 2011, 0, 0);			//获取默认房号失败！
							break;
						}

						char termid[16];
						GetTermId(termid);
						if(strcmp(pGet.param->code, termid) != 0)
						{
							free(pGet.param);
							DPPostMessage(MSG_SHOW_STATUE, 2009, 0, 0);	// 请先设置室内机为默认房号！
							break;
						}

						free(pGet.param);
						if(!TermGet(&pGet, setCode))
						{
							DPPostMessage(MSG_SHOW_STATUE, 2003, 0, 0);		//房号不存在
							break;
						}

						UINT64 setId = pGet.param->id;
						free(pGet.param);

						if(Code2Type(setCode) != SECOND_DOOR_TYPE)
						{
							DPPostMessage(MSG_SHOW_STATUE, 2010, 0, 0);		//房号不是别墅机房号
							break;
						}

						if(!SecDoorGet(&pGet))
						{
							DPPostMessage(MSG_SHOW_STATUE, 2013, 0, 0);		//获取默认别墅机房号失败
							break;
						}

						if(!SetSecDoorDelay(pGet.param->code, 0, setDelay))
						{
							DBGMSG(DPINFO, "SetSecDoorDelay fail!\r\n");
							DPPostMessage(MSG_SHOW_STATUE, 2012, 0, 0);	//对方不在线
							free(pGet.param);
							break;
						}

						if(!SetSecDoorCode(pGet.param->code, setId))
						{
							DBGMSG(DPINFO, "SetSecDoorCode fail!\r\n");
							DPPostMessage(MSG_SHOW_STATUE, 2012, 0, 0);	//对方不在线
							free(pGet.param);
							break;
						}

						free(pGet.param);
						DPPostMessage(MSG_SHOW_STATUE, 2004, 0, 0);	//设置成功
					}
				}
				else
				{
					if(m_pos < 15)
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
		sprintf(buf, "%c%c%s%c%c%s%c%c%s%c%c%c%c%s%c%c%s", 
			m_string[0], m_string[1], GetStringByID(617), 
			m_string[2], m_string[3], GetStringByID(618), 
			m_string[4], m_string[5], GetStringByID(619), 
			m_string[6], m_string[7], m_string[8], m_string[9], GetStringByID(620), 
			m_string[10], m_string[11], GetStringByID(621));

		m_pCode->SetSrc(buf);
		m_pCode->Show(TRUE);

		sprintf(buf, "%c%c%c%s", m_string[12], m_string[13], m_string[14], GetStringByID(1016) );
		m_pDelay->SetSrc(buf);
		m_pDelay->Show(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prjdoor.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("removeICCard", &m_idCardRemove);
		GetCtrlByName("addICCard", &m_idCardAdd);
		m_pCode = (CDPStatic*)GetCtrlByName("houseCode");
		m_pDelay = (CDPStatic*)GetCtrlByName("unlockDelay");

		m_pos = 0;
		strcpy(m_string, "---------------");
		ShowString();
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idCardRemove;
	DWORD m_idCardAdd;
	CDPStatic* m_pCode;
	CDPStatic* m_pDelay;

	int m_pos;			// 当前输入位置
	char m_string[32];	// 输入的字符串
};

CAppBase* CreatePrjDoorApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjDoorApp* pApp = new CPrjDoorApp(wParam);
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