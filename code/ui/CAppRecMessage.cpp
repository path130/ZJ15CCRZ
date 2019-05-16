#include "CCtrlModules.h"
#include "dpvideo.h"

#define PUBLIC_TYPE			0
#define PERSON_TYPE			1
#define MAX_TYPE			2

#define COL					3

class CRecMessageApp : public CAppBase
{
public:
	CRecMessageApp(DWORD pHwnd):CAppBase(pHwnd)
	{
		m_pdata = NULL;
	}

	~CRecMessageApp(void)
	{
	}

	BOOL DoPause()
	{
		H264DecStop();
		SetTouchDirect(FALSE);
		return CAppBase::DoPause();
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
			if(wParam == MESSAGE_CHANGE)
			{
				GetMsgList();
				ShowMsgList(m_nFocus);
			}
			break;
		case TOUCH_RAW_MESSAGE:
			if(zParam == TOUCH_UP)
			{
				if(m_bShowPwd)
				{
					m_bShowPwd = FALSE;
					m_pPwd->SwitchLay(FALSE);
				}
				else
				{
					H264DecStop();
				}
				SetTouchDirect(FALSE);
			}
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBack)
			{
				DPPostMessage(MSG_START_APP, m_dwReturnID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[PUBLIC_TYPE])
			{
				m_nFocus = PUBLIC_TYPE;
				ShowMsgList(m_nFocus);
			}
			else if(wParam == m_idButton[PERSON_TYPE])
			{
				m_nFocus = PERSON_TYPE;
				ShowMsgList(m_nFocus);
			}
			else if(wParam == m_idView)
			{
				if(m_count[m_nFocus] > 0)
				{
					int ptr = m_pList->GetCurPtr();
					// 未读 -> 已读
					if(m_bRead[m_nFocus][ptr] == FALSE)
					{
						m_bRead[m_nFocus][ptr] = TRUE;
						strcpy(m_DataArray[m_nFocus][ptr * COL + 2], GetStringByID(1024));	// 已读	
						m_pList->Show(m_pList->GetCurPagePtr(), STATUS_PRESSED);
					}
					// 显示信息
					char *pbuf;
					int len = GetMessageData(m_nFocus == PUBLIC_TYPE, ptr, &pbuf);
					if(len > 0)
					{
						if(len == 128)
						{
							// 公共开锁消息
							ShowPwd(pbuf);
						}
						else
						{
							// 图片消息
							JPGDecStart(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
							H264WriteData(pbuf, len);
						}
						SetTouchDirect(TRUE);
						free(pbuf);
					}
				}
			}
			else if(wParam == m_idPrev)
			{
				if(m_pList->PrevPage())
				{
					m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
					m_pSelect->Show(TRUE);
				}
			}
			else if(wParam == m_idNext)
			{
				if(m_pList->NextPage())
				{
					m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
					m_pSelect->Show(TRUE);
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
				m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
				m_pSelect->Show(TRUE);
			}
			else if(wParam == m_idOK)
			{
				DoDelete(m_bDeleteAll);
				ShowMsgList(m_nFocus);
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

	void ShowPwd(char* pbuf)
	{
		// 开锁消息
		char buf[64];
		char user[16];
		char hostage[16];
		sscanf(pbuf, "%s %s", user, hostage);

		sprintf(buf, "%s%s", GetStringByID(30051), user);		// 公共开锁密码:
		m_pUserPwd->SetSrc(buf);
		m_pUserPwd->Show(TRUE);

		sprintf(buf, "%s%s", GetStringByID(30061), hostage);	// 公共开锁挟持密码:
		m_pHostagePwd->SetSrc(buf);
		m_pHostagePwd->Show(TRUE);

		m_pPwd->SwitchLay(TRUE);
		m_bShowPwd = TRUE;
	}

	void DoDelete(BOOL bDeleteAll)
	{
		if(bDeleteAll)
		{
			DeleteMessageAll(m_nFocus == PUBLIC_TYPE);
			m_count[m_nFocus] = 0;
		}
		else
		{
			int ptr = m_pList->GetCurPtr();
			DeleteMessage(m_nFocus == PUBLIC_TYPE, ptr);
			memmove(&m_bRead[m_nFocus][ptr], &m_bRead[m_nFocus][ptr + 1], sizeof(BOOL) * (m_count[m_nFocus] - ptr - 1));
			memmove(m_DataArray[m_nFocus][ptr * COL], m_DataArray[m_nFocus][(ptr + 1) * COL], sizeof(m_buf[0]) * COL * (m_count[m_nFocus] - ptr - 1));
			m_count[m_nFocus]--;
		}
	}

	void GetMsgList()
	{
		MsgRecord item[MAX_TYPE][MAX_MESSAGE_NUMBER];
		SYSTEMTIME systime;
		int language = GetLanguage();
		wchar_t buf[128];

		for(int i = 0; i < MAX_MESSAGE_NUMBER * COL; i++)
		{
			m_DataArray[PUBLIC_TYPE][i] = m_buf[i];		
			m_DataArray[PERSON_TYPE][i] = m_buf[i + MAX_MESSAGE_NUMBER * COL];		
		}
		m_count[PUBLIC_TYPE] = GetMessageRecord(TRUE, item[PUBLIC_TYPE]);
		m_count[PERSON_TYPE] = GetMessageRecord(FALSE, item[PERSON_TYPE]);
		for(int k = 0; k < MAX_TYPE; k++)
		{
			for(int i = 0; i < m_count[k]; i++)
			{
				// 标题
				if(item[k][i].title[0] == 0)
					strcpy(m_DataArray[k][i * COL], GetStringByID(30031));	// 无标题
				else if(utf82unicode((WORD*)buf, (BYTE*)item[k][i].title) > 10)
				{
					buf[8]	= '.';
					buf[9]	= '.';
					buf[10] = '.';
					buf[11] = 0;
					unicode2utf8((BYTE*)item[k][i].title, buf);
					strcpy(m_DataArray[k][i * COL], item[k][i].title);
				}
				else
					strcpy(m_DataArray[k][i * COL], item[k][i].title);

				// 时间
				DPFileTimeToSystemTime(&item[k][i].time, &systime);
				if(language == 0)	
				{
					// 中文 2006-01-01
					sprintf(m_DataArray[k][i * COL + 1], "%04d-%02d-%02d  %02d:%02d",   
						systime.wYear, systime.wMonth, systime.wDay,
						systime.wHour, systime.wMinute);
				}
				else
				{
					// 英文 01-01-2006
					sprintf(m_DataArray[k][i * COL + 1], "%02d-%02d-%04d  %02d:%02d", 
						systime.wMonth, systime.wDay, systime.wYear,
						systime.wHour, systime.wMinute);
				}

				// 已读未读
				if(item[k][i].bRead)
					strcpy(m_DataArray[k][i * COL + 2], GetStringByID(1024));
				else
					strcpy(m_DataArray[k][i * COL + 2], GetStringByID(1025));

				m_bRead[k][i] = item[k][i].bRead;
			}
		}
	}

	void ShowMsgList(int nFocus)
	{
		for(int i = 0; i < MAX_TYPE; i++)
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
		}
		else
		{
			// 列表显示
			m_pList->SetDataArray(m_count[nFocus], m_DataArray[nFocus]);
			m_pList->SetCurPtr(0);
			m_pList->Show();

			m_pSelect->Show(FALSE);

			if(nFocus == PUBLIC_TYPE)
				m_pTip->SetSrc(GetStringByID(30071));	// 没有公共消息
			else
				m_pTip->SetSrc(GetStringByID(30081));	// 没有个人消息
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
		InitFrame("recmessage.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("prev", &m_idPrev);
		GetCtrlByName("next", &m_idNext);
		GetCtrlByName("delete", &m_idDelete);
		GetCtrlByName("deleteall", &m_idDeleteAll);
		GetCtrlByName("see", &m_idView);
		m_pTip = (CDPStatic*)GetCtrlByName("tip");
		m_pSelect = (CDPStatic*)GetCtrlByName("selected");
		m_pList = (CDPListView*)GetCtrlByName("listview", &m_idList);

		m_pDlg = (CLayOut*)GetCtrlByName("layout_ask");
		m_pDlgTip = (CDPStatic*)GetCtrlByName("ask_tip");
		GetCtrlByName("ask_cancle", &m_idCancel);
		GetCtrlByName("ask_ok", &m_idOK);

		m_pButton[PUBLIC_TYPE] = (CDPButton*)GetCtrlByName("publicMsg", &m_idButton[PUBLIC_TYPE]);
		m_pButton[PERSON_TYPE] = (CDPButton*)GetCtrlByName("privateMsg", &m_idButton[PERSON_TYPE]);

		m_pPwd = (CLayOut*)GetCtrlByName("layout_text");
		m_pUserPwd = (CDPStatic*)GetCtrlByName("userPwd");
		m_pHostagePwd = (CDPStatic*)GetCtrlByName("hostagePwd");
		m_bShowPwd = FALSE;

		m_dwReturnID = lParam;
		m_nFocus = 0;
		GetMsgList();
		ShowMsgList(m_nFocus);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	DWORD m_idDelete;
	DWORD m_idDeleteAll;
	DWORD m_idView;
	CDPStatic* m_pTip;
	CDPStatic* m_pSelect;

	DWORD m_idList;
	CDPListView* m_pList;

	int m_count[MAX_TYPE];
	char m_buf[MAX_TYPE * MAX_MESSAGE_NUMBER * COL][32];
	char* m_DataArray[MAX_TYPE][MAX_MESSAGE_NUMBER * COL];
	DWORD m_bRead[MAX_TYPE][MAX_MESSAGE_NUMBER];

	// 确定删除
	CLayOut* m_pDlg;
	CDPStatic* m_pDlgTip;
	DWORD m_idOK;
	DWORD m_idCancel;
	BOOL m_bDeleteAll;

	// 开锁密码信息显示
	CLayOut* m_pPwd;
	CDPStatic* m_pUserPwd;
	CDPStatic* m_pHostagePwd;
	BOOL m_bShowPwd;

	int m_nFocus;
	CDPButton* m_pButton[MAX_TYPE];
	DWORD m_idButton[MAX_TYPE];

	char* m_pdata;
	DWORD m_dwReturnID;		// 返回界面的ID
};

CAppBase* CreateRecMessageApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CRecMessageApp* pApp = new CRecMessageApp(wParam);
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