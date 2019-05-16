#include "CCtrlModules.h"
#include "PhoneApp.h"
#include "ipfunc.h"

#define MISS_CALLLOG_TYPE		0
#define ACCEPT_CALLLOG_TYPE		1
#define	CALLOUT_CALLLOG_TYPE	2
#define MAX_CALLLOG_TYPE		3

#define COL					3

static char g_callcode[16];

class CRecCallApp : public CAppBase
{
public:
	CRecCallApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CRecCallApp(void)
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
			else if(wParam == m_idButton[MISS_CALLLOG_TYPE])
			{
				m_nFocus = MISS_CALLLOG_TYPE;
				ShowCallList(m_nFocus);
			}
			else if(wParam == m_idButton[ACCEPT_CALLLOG_TYPE])
			{
				m_nFocus = ACCEPT_CALLLOG_TYPE;
				ShowCallList(m_nFocus);
			}
			else if(wParam == m_idButton[CALLOUT_CALLLOG_TYPE])
			{
				m_nFocus = CALLOUT_CALLLOG_TYPE;
				ShowCallList(m_nFocus);
			}
			else if(wParam == m_idRedial)
			{
				if(m_count[m_nFocus] > 0)
				{
					GetCallRecordCode(g_callcode, m_nFocus, m_pList->GetCurPtr());
					if(!RequestTalk())
						DPPostMessage(MSG_SHOW_STATUE, 606, 0, 0);	//	正在通话中
					else
						DPPostMessage(MSG_START_FROM_ROOT, TALKING_APPID, GetPhonePkt(Code2Type(g_callcode), g_callcode), 0);
				}
			}
			else if(wParam == m_idPrev)
			{
				if(m_pList->PrevPage())
				{
					m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
					m_pSelect->Show(TRUE);

					if(m_bDoor[m_nFocus][m_pList->GetCurPtr()])
						m_pRedial->Show(STATUS_FOCUS);
					else
						m_pRedial->Show(STATUS_NORMAL);
				}
			}
			else if(wParam == m_idNext)
			{
				if(m_pList->NextPage())
				{
					m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
					m_pSelect->Show(TRUE);

					if(m_bDoor[m_nFocus][m_pList->GetCurPtr()])
						m_pRedial->Show(STATUS_FOCUS);
					else
						m_pRedial->Show(STATUS_NORMAL);
				}
			}
			else if(wParam == m_idDelete)
			{
				if(m_count[m_nFocus] > 0)
				{
					ShowDlg(GetStringByID(1019));	// 确认删除？
					m_bDeleteAll = FALSE;
				}
			}
			else if(wParam == m_idDeleteAll)
			{
				if(m_count[m_nFocus] > 0)
				{
					ShowDlg(GetStringByID(1029));	// 确认全删除？
					m_bDeleteAll = TRUE;
				}
			}
			else if(wParam == m_idList)
			{
				int ptr = m_pList->GetCurPtr();
				if(m_nFocus == MISS_CALLLOG_TYPE)
				{
					// 未读 -> 已读
					if(!m_bRead[ptr])
					{
						m_bRead[ptr] = TRUE;
						strcpy(m_DataArray[MISS_CALLLOG_TYPE][ptr * COL + 2], GetStringByID(1024));		// 已读
						m_pList->Show(m_pList->GetCurPagePtr(), STATUS_PRESSED);
						SetCallRecordRead(ptr);
					}
				}
				if(m_bDoor[m_nFocus][m_pList->GetCurPtr()])
					m_pRedial->Show(STATUS_FOCUS);
				else
					m_pRedial->Show(STATUS_NORMAL);

				m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
				m_pSelect->Show(TRUE);
			}
			else if(wParam == m_idOK)
			{
				DoDelete(m_bDeleteAll);
				ShowCallList(m_nFocus);
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
			DeleteCallRecordAll(m_nFocus);
			m_count[m_nFocus] = 0;
		}
		else
		{
			int ptr = m_pList->GetCurPtr();
			DeleteCallRecord(m_nFocus, ptr);
			memmove(m_DataArray[m_nFocus][ptr * COL], m_DataArray[m_nFocus][(ptr + 1) * COL], sizeof(m_buf[0]) * COL * (m_count[m_nFocus] - ptr - 1));
			m_count[m_nFocus]--;
		}
	}

	void GetCallList()
	{
		// 获得通话记录内容
		for(int i = 0; i < MAX_CALLLOG * COL; i++)
		{
			m_DataArray[MISS_CALLLOG_TYPE][i] = m_buf[i];
			m_DataArray[ACCEPT_CALLLOG_TYPE][i] = m_buf[i + MAX_CALLLOG * COL];
			m_DataArray[CALLOUT_CALLLOG_TYPE][i] = m_buf[i + MAX_CALLLOG * COL * 2];
		}

		CallRecord m_item[MAX_CALLLOG_TYPE][MAX_CALLLOG];
		SYSTEMTIME systime;
		int language = GetLanguage();
		m_count[MISS_CALLLOG_TYPE] = GetCallRecord(MISS_CALLLOG_TYPE, m_item[MISS_CALLLOG_TYPE]);
		m_count[ACCEPT_CALLLOG_TYPE] = GetCallRecord(ACCEPT_CALLLOG_TYPE, m_item[ACCEPT_CALLLOG_TYPE]);
		m_count[CALLOUT_CALLLOG_TYPE] = GetCallRecord(CALLOUT_CALLLOG_TYPE, m_item[CALLOUT_CALLLOG_TYPE]);

		for(int k = 0; k < MAX_CALLLOG_TYPE; k++)
		{
			for(int i = 0; i < m_count[k]; i++)
			{
				// 名称
				Code2Name(m_DataArray[k][i * COL], m_item[k][i].code);

				// 时间
				DPFileTimeToSystemTime(&m_item[k][i].time, &systime);
				if(language == 0)
				{
					//中文 2006-01-01
					sprintf(m_DataArray[k][i * COL + 1], "%04d-%02d-%02d  %02d:%02d",   
						systime.wYear, systime.wMonth, systime.wDay,
						systime.wHour, systime.wMinute);
				}
				else
				{
					//英文 01-01-2006
					sprintf(m_DataArray[k][i * COL + 1], "%02d-%02d-%04d  %02d:%02d", 
						systime.wMonth, systime.wDay, systime.wYear,
						systime.wHour, systime.wMinute);
				}

				// 已读未读
				if(k == MISS_CALLLOG_TYPE)
				{
					if(m_item[k][i].bRead)
						strcpy(m_DataArray[k][i * COL + 2], GetStringByID(1024));
					else
						strcpy(m_DataArray[k][i * COL + 2], GetStringByID(1025));

					m_bRead[i] = m_item[k][i].bRead;
				}
				else
				{
					m_DataArray[k][i * COL + 2][0] = 0;
				}

				switch(Code2Type(m_item[k][i].code))
				{
					case CELL_DOOR_TYPE:
					case SECOND_DOOR_TYPE:
					case ZONE_DOOR_TYPE:
					case AREA_DOOR_TYPE:
						m_bDoor[k][i] = TRUE;
						break;
					default:
						m_bDoor[k][i] = FALSE;
						break;
				}
			}
		}
	}

	void ShowCallList(int nFocus)
	{
		for(int i = 0; i < MAX_CALLLOG_TYPE; i++)
		{
			if(nFocus == i)
				m_pButton[i]->Show(STATUS_FOCUS);
			else
				m_pButton[i]->Show(STATUS_NORMAL);
		}

		if(m_count[nFocus] > 0)
		{
			// 先隐藏提示
			m_pTip->Show(FALSE);

			// 列表显示
			m_pList->SetDataArray(m_count[nFocus], m_DataArray[nFocus]);
			m_pList->SetCurPtr(0);
			m_pList->Show();

			// 选择显示
			m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
			m_pSelect->Show(TRUE);

			if(m_bDoor[m_nFocus][0])
				m_pRedial->Show(STATUS_FOCUS);
			else
				m_pRedial->Show(STATUS_NORMAL);
		}
		else
		{
			// 列表显示
			m_pList->SetDataArray(m_count[nFocus], m_DataArray[nFocus]);
			m_pList->SetCurPtr(0);
			m_pList->Show();

			m_pSelect->Show(FALSE);

			// 没有通话记录
			m_pTip->SetSrc(GetStringByID(30045));
			m_pTip->Show(TRUE);

			m_pRedial->Show(STATUS_NORMAL);
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
		InitFrame("reccall.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("prev", &m_idPrev);
		GetCtrlByName("next", &m_idNext);
		GetCtrlByName("delete", &m_idDelete);
		GetCtrlByName("deleteall", &m_idDeleteAll);
		m_pRedial = (CDPButton*)GetCtrlByName("redial", &m_idRedial);
		m_pTip = (CDPStatic*)GetCtrlByName("tip");
		m_pSelect = (CDPStatic*)GetCtrlByName("selected");
		m_pList = (CDPListView*)GetCtrlByName("listview", &m_idList);

		m_pDlg = (CLayOut*)GetCtrlByName("layout_ask");
		m_pDlgTip = (CDPStatic*)GetCtrlByName("ask_tip");
		GetCtrlByName("ask_cancle", &m_idCancel);
		GetCtrlByName("ask_ok", &m_idOK);

		m_pButton[MISS_CALLLOG_TYPE] = (CDPButton*)GetCtrlByName("miss", &m_idButton[MISS_CALLLOG_TYPE]);
		m_pButton[ACCEPT_CALLLOG_TYPE] = (CDPButton*)GetCtrlByName("accept", &m_idButton[ACCEPT_CALLLOG_TYPE]);
		m_pButton[CALLOUT_CALLLOG_TYPE] = (CDPButton*)GetCtrlByName("dial", &m_idButton[CALLOUT_CALLLOG_TYPE]);

		m_nFocus = 0;
		GetCallList();
		ShowCallList(m_nFocus);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	DWORD m_idDelete;
	DWORD m_idDeleteAll;
	DWORD m_idRedial;
	CDPButton* m_pRedial;

	CDPStatic* m_pTip;
	CDPStatic* m_pSelect;

	DWORD m_idList;
	CDPListView* m_pList;

	int m_count[MAX_CALLLOG_TYPE];
	char m_buf[MAX_CALLLOG_TYPE * MAX_CALLLOG * COL][32];
	char* m_DataArray[MAX_CALLLOG_TYPE][MAX_CALLLOG * COL];
	BOOL m_bRead[MAX_CALLLOG];
	BOOL m_bDoor[MAX_CALLLOG][MAX_CALLLOG];

	// 确定删除
	CLayOut* m_pDlg;
	CDPStatic* m_pDlgTip;
	DWORD m_idOK;
	DWORD m_idCancel;
	BOOL m_bDeleteAll;

	int m_nFocus;
	CDPButton* m_pButton[MAX_CALLLOG_TYPE];
	DWORD m_idButton[MAX_CALLLOG_TYPE];
};

CAppBase* CreateRecCallApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CRecCallApp* pApp = new CRecCallApp(wParam);
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