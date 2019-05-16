#include "CCtrlModules.h"

BOOL SafeDisAlarm(AreaSet* areaSet)
{
	if(GetDefenseStatus() == FALSE && GetDefenseIsAlarming() == FALSE)
	{
		DPPostMessage(MSG_SHOW_STATUE, 2017, 0, 0);
		return FALSE;
	}
	else
	{
		DPPostMessage(MSG_START_APP, SAFE_DISALARM_APPID, 0, 0);
		return TRUE;
	}
}

BOOL SafeSetting(int mode)
{
	BOOL ret = FALSE;
	if(GetDefenseStatus() == TRUE)
	{
		DPPostMessage(MSG_SHOW_STATUE, 2016, 0, 0);
	}
	else
	{
		if( GetDefenseIsAlarming())
		{
			DPPostMessage(MSG_SHOW_STATUE, 2018, 0, 0);
		}
		else
		{
			DPPostMessage(MSG_START_APP, SAFE_SET_APPID, ~mode, 0);
			ret = TRUE;
		}
	}

	return ret;
}


class CSafeMainApp : public CAppBase
{
public:
	CSafeMainApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSafeMainApp()
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
		case MSG_BROADCAST:
			if(wParam == ALARM_CHANGE)
			{
				GetSafeAreaSet(m_areaSet);
				if(lParam < SAFE_MAX_NUMBER)
					UpdateAreaStatus(lParam);
			}
			else if(wParam == SAFE_CHANGE)
			{
				GetSafeAreaSet(m_areaSet);
				for(int i = 0; i < SAFE_MAX_NUMBER; i++)
					UpdateAreaStatus(i);
			}
			break;
		case TOUCH_MESSAGE:
			if( wParam == m_idBack )
			{
				DPPostMessage( MSG_START_APP, MAIN_APPID, 0, 0 );
				DPPostMessage( MSG_END_APP, (DWORD)this, 0, 0 );
			}
			else if(wParam == m_idCancelDefense)
			{
				if(SafeDisAlarm(m_areaSet))
					DPPostMessage( MSG_END_APP, (DWORD)this, 0, 0 );
			}
			else if(wParam == m_idSetDefense)
			{
				if(GetDefenseStatus())
					DPPostMessage(MSG_SHOW_STATUE, 2016, 0, 0);		// 已布防
				else if(GetDefenseIsAlarming())	
					DPPostMessage(MSG_SHOW_STATUE, 2018, 0, 0);		// 正在报警中,请先撤防!
				else
				{
					DPPostMessage(MSG_START_APP, SAFE_SET_APPID, SAFE_MAIN_APPID, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			break;
		}
		return TRUE;
	}

	void UpdateAreaStatus(int index)
	{
		char bakPng[32];
		char srcPng[32];
		char srcText[32];

		if(m_areaSet[index].OnOff == FALSE)
		{
			// 禁用
			sprintf(srcText, "%s(%s)", GetStringByID(m_areaSet[index].Type + 3101), GetStringByID(3201));
			strcpy(bakPng, "areabk_blue.png");
		}
		else
		{	
			if(m_areaSet[index].Type == STYPE_URGENT)
			{
				// 已布防
				sprintf(srcText, "%s(%s)", GetStringByID(m_areaSet[index].Type + 3101), GetStringByID(3302));
				strcpy(bakPng, "areabk_green.png");
			}
			else
			{
				switch(m_areaSet[index].Status)
				{
					case SSTATUS_CANCEL_DEFENSE:
						// 未布防
						strcpy(bakPng, "areabk_blue.png");
						sprintf(srcText, "%s(%s)", GetStringByID(m_areaSet[index].Type + 3101), GetStringByID(3301));
						break;
					case SSTATUS_SET_DEFENSE:
						// 已布防
						strcpy(bakPng, "areabk_green.png");
						sprintf(srcText, "%s(%s)", GetStringByID(m_areaSet[index].Type + 3101), GetStringByID(3302));
						break;
					case SSTATUS_ALARM_DELAY:
						// 报警延时
						strcpy(bakPng, "areabk_red.png");
						sprintf(srcText, "%s(%s)", GetStringByID(m_areaSet[index].Type + 3101), GetStringByID(3303));
						break;
					case SSTATUS_ALARMING:
						// 报警中
						strcpy(bakPng, "areabk_red.png");
						sprintf(srcText, "%s(%s)", GetStringByID(m_areaSet[index].Type + 3101), GetStringByID(3304));
						break;
				}
			}
		}

		switch(m_areaSet[index].Area)
		{
		case SAREA_KITCHEN:
			strcpy(srcPng, "area_kitchen.png");
			break;
		case SAREA_BEDROOM:
			strcpy(srcPng, "area_badroom.png");
			break;
		case SAREA_HALL:
			strcpy(srcPng, "area_hall.png");
			break;
		case SAREA_WINDOW:
			strcpy(srcPng, "area_window.png");
			break;
		case SAREA_DOOR:
			strcpy(srcPng, "area_door.png");
			break;
		case SAREA_BALCONY:
			strcpy(srcPng, "area_balcony.png");
			break;
		case SAREA_GUEST:
			strcpy(srcPng, "area_guest.png");
			break;
		}

		m_pArea[index]->SetBkpng(bakPng);
		m_pArea[index]->SetSrcpng(srcPng);
		m_pArea[index]->SetSrcText(srcText);
		m_pArea[index]->Show(STATUS_UNACK);

		m_pText[index]->SetSrc(GetStringByID(m_areaSet[index].Area + 3001));
		m_pText[index]->RefreshBak();
		m_pText[index]->Show(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("safemain.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("safe", &m_idSetDefense);
		GetCtrlByName("unsafe", &m_idCancelDefense);

		char buf[32];
		for(int i = 0; i < 8; i++)
		{
			sprintf(buf, "tip%d", i + 1);
			m_pText[i] = (CDPStatic*)GetCtrlByName(buf);
			sprintf(buf, "area%d", i + 1);
			m_pArea[i] = (CDPButton*)GetCtrlByName(buf);
		}

		GetSafeAreaSet(m_areaSet);
		for(int i = 0; i < SAFE_MAX_NUMBER; i++)
		{
			UpdateAreaStatus(i);
		}
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idSetDefense;
	DWORD m_idCancelDefense;
	CDPStatic* m_pText[8];
	CDPButton* m_pArea[8];	//防区1-8
	DWORD m_idArea[8];
	AreaSet m_areaSet[SAFE_MAX_NUMBER];
};

CAppBase* CreateSafeMainApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSafeMainApp* pApp = new CSafeMainApp(wParam);
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