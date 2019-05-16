#include "CCtrlModules.h"

#define COL		3

class CRecAlarmApp : public CAppBase
{
public:
	CRecAlarmApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CRecAlarmApp(void)
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
				DPPostMessage(MSG_START_APP, RECORD_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idPrev)
			{
				if(m_pList->PrevPage())
				{
					m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
					m_pSelect->Show(TRUE);
				}
			}
			else if(wParam == m_idNext)
			{
				if(m_pList->NextPage())
				{
					m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
					m_pSelect->Show(TRUE);
				}
			}
			else if(wParam == m_idDelete)
			{
				if(m_count > 0)
				{
					ShowDlg(GetStringByID(1019));	// 确认删除？
					m_bDeleteAll = FALSE;
				}
			}
			else if(wParam == m_idDeleteAll)
			{
				if(m_count > 0)
				{
					ShowDlg(GetStringByID(1029));	// 确认全删除？
					m_bDeleteAll = TRUE;
				}
			}
			else if(wParam == m_idList)
			{
				m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
				m_pSelect->Show(TRUE);
			}
			else if(wParam == m_idOK)
			{
				DoDelete(m_bDeleteAll);
				ShowAlarmList();
				m_pDlg->SwitchLay(FALSE);
			}
			else if(wParam == m_idCancel)
			{
				m_pDlg->SwitchLay(FALSE);
			}
			break;
		}
		return TRUE;
	}

	void DoDelete(BOOL bDeleteAll)
	{
		if(bDeleteAll)
		{
			ResetAlarmRecord();
			m_count = 0;
		}
		else
		{
			int ptr = m_pList->GetCurPtr();
			DeleteAlarmRecord(ptr);
			memmove(m_DataArray[ptr * COL], m_DataArray[(ptr + 1) * COL], sizeof(m_buf[0]) * COL * (m_count - ptr - 1));
			m_count--;
		}
	}

	void GetAlarmList()
	{
		// 获得报警记录内容
		AlarmRecord item;
		SYSTEMTIME systime;
		int language = GetLanguage();
		int count = GetAlarmRecordCount();

		for(int i = 0; i < MAX_ALARM_RECORD_NUMBER * COL; i++)
		{
			m_DataArray[i] = m_buf[i];
		}

		for(int i = 0; i < count; i++)
		{
			GetAlarmRecordData(i, &item);
			DPFileTimeToSystemTime(&item.time, &systime);

			strcpy(m_DataArray[i * COL], GetStringByID(item.area + 3001));
			strcpy(m_DataArray[i * COL + 1], GetStringByID(item.type + 3101));

			if(language == 0)
			{
				//中文 2006-01-01
				sprintf(m_DataArray[i * COL + 2], "%04d-%02d-%02d  %02d:%02d",   
					systime.wYear, systime.wMonth, systime.wDay,
					systime.wHour, systime.wMinute);
			}
			else
			{
				//英文 01-01-2006
				sprintf(m_DataArray[i * COL + 2], "%02d-%02d-%04d  %02d:%02d", 
					systime.wMonth, systime.wDay, systime.wYear,
					systime.wHour, systime.wMinute);
			}
		}

		m_count = count;
	}

	void ShowAlarmList()
	{
		// 列表显示
		m_pList->SetDataArray(m_count, m_DataArray);
		m_pList->Show();

		if(m_count > 0)
		{
			m_pTip->Show(FALSE);

			// 选择显示
			m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
			m_pSelect->Show(TRUE);
		}
		else
		{
			m_pSelect->Show(FALSE);

			// 没有报警记录
			m_pTip->SetSrc(GetStringByID(30013));
			m_pTip->Show(TRUE);
		}
	}

	void ShowDlg(char* tip)
	{
		char buf[32];
		sprintf(buf, "%s%s?", GetStringByID(2005), tip);
		m_pDlgTip->SetSrc(buf);
		m_pDlgTip->Show(TRUE);
		m_pDlg->SwitchLay(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("recalarm.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("prev", &m_idPrev);
		GetCtrlByName("next", &m_idNext);
		GetCtrlByName("delete", &m_idDelete);
		GetCtrlByName("deleteall", &m_idDeleteAll);
		m_pTip = (CDPStatic*)GetCtrlByName("tip");
		m_pSelect = (CDPStatic*)GetCtrlByName("selected");
		m_pList = (CDPListView*)GetCtrlByName("listview", &m_idList);

		m_pDlg = (CLayOut*)GetCtrlByName("layout_ask");
		m_pDlgTip = (CDPStatic*)GetCtrlByName("ask_tip");
		GetCtrlByName("ask_cancle", &m_idCancel);
		GetCtrlByName("ask_ok", &m_idOK);

		GetAlarmList();
		ShowAlarmList();
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	DWORD m_idDelete;
	DWORD m_idDeleteAll;
	CDPStatic* m_pTip;
	CDPStatic* m_pSelect;

	DWORD m_idList;
	CDPListView* m_pList;

	int m_count;
	char m_buf[MAX_ALARM_RECORD_NUMBER * COL][32];
	char* m_DataArray[MAX_ALARM_RECORD_NUMBER * COL];

	// 确定删除
	CLayOut* m_pDlg;
	CDPStatic* m_pDlgTip;
	DWORD m_idOK;
	DWORD m_idCancel;
	BOOL m_bDeleteAll;
};

CAppBase* CreateRecAlarmApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CRecAlarmApp* pApp = new CRecAlarmApp(wParam);
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