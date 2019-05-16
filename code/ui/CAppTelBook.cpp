#include "CCtrlModules.h"
#include "PhoneApp.h"
#include "ipfunc.h"

#define COL	 2
#define INPUT_NAME	0
#define INPUT_AREA	1
#define INPUT_BUILD	2
#define INPUT_UNIT	3
#define INPUT_ROOM	4

static char g_callcode[16];
class CTelBookApp : public CAppBase
{
public:
	CTelBookApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CTelBookApp(void)
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
			else if(wParam == m_idCall)
			{
				if(m_count > 0)
				{
					TelBookRecord item;
					GetTelBookData(m_pList->GetCurPtr(), &item);

					ip_get pGet;
					char termid[16] = {0};
					strcpy(termid, item.strCode);

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
			else if(wParam == m_idAdd)
			{
				m_pImeTip->SetSrc(GetStringByID(40015));		// 请输入联系人名字
				m_pImeTip->Show(TRUE);
				m_pImeEdit->SetString("");
				m_pImeEdit->SetMaxLen(6);
				m_pIme->SwitchLay(TRUE);
				m_stage = INPUT_NAME;
			}
			else if(wParam == m_idList)
			{
				m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
				m_pSelect->Show(TRUE);
			}
			else if(wParam == m_idOK)
			{
				DoDelete(m_bDeleteAll);
				ShowTelBookList();
				m_pDlg->SwitchLay(FALSE);
			}
			else if(wParam == m_idCancel)
			{
				m_pDlg->SwitchLay(FALSE);
			}
			else if(wParam == m_idImeBack)
			{
				m_pAdd->Show(STATUS_NORMAL);
				m_pIme->SwitchLay(FALSE);
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

	void ShowDlg(char* tip)
	{
		char buf[32];
		sprintf(buf, "%s%s?", GetStringByID(2005), tip);
		m_pDlgTip->SetSrc(buf);
		m_pDlgTip->Show(TRUE);
		m_pDlg->SwitchLay(TRUE);
	}

	void DoDelete(BOOL bDeleteAll)
	{
		if(bDeleteAll)
		{
			ResetTelBook();
			m_count = 0;
		}
		else
		{
			int ptr = m_pList->GetCurPtr();
			DeleteTelBook(ptr);
			memmove(m_DataArray[ptr * COL], m_DataArray[(ptr + 1) * COL], sizeof(m_buf[0]) * COL * (m_count - ptr - 1));
			m_count--;
		}
	}

	void InputProc()
	{
		if(m_pImeEdit->GetCurCount() == 0)
		{
			DPPostMessage(MSG_SHOW_STATUE, 2028, 0, 0);		// 输入不能为空！
			return;
		}

		switch(m_stage)
		{
		case INPUT_NAME:
			strcpy(m_item.strName, m_pImeEdit->GetString());
			m_pImeEdit->SetString("");
			m_pImeEdit->SetMaxLen(2);
			m_pImeTip->SetSrc(GetStringByID(40025));		// 请输入联系人所在区
			m_pImeTip->Show(TRUE);
			m_stage = INPUT_AREA;
			break;
		case INPUT_AREA:
			sprintf(m_item.strCode, "1%02d", strtol(m_pImeEdit->GetString(), NULL, 10));
			m_pImeTip->SetSrc(GetStringByID(40035));		// 请输入联系人所在楼栋
			m_pImeTip->Show(TRUE);
			m_pImeEdit->SetString("");
			m_stage = INPUT_BUILD;
			break;
		case INPUT_BUILD:
			sprintf(&m_item.strCode[3], "%02d", strtol(m_pImeEdit->GetString(), NULL, 10));
			m_pImeTip->SetSrc(GetStringByID(40045));		// 请输入联系人所在单元
			m_pImeTip->Show(TRUE);
			m_pImeEdit->SetString("");
			m_stage = INPUT_UNIT;
			break;
		case INPUT_UNIT:
			sprintf(&m_item.strCode[5], "%02d", strtol(m_pImeEdit->GetString(), NULL, 10));
			m_pImeEdit->SetMaxLen(4);
			m_pImeTip->SetSrc(GetStringByID(40055));		// 请输入联系人所在房号
			m_pImeTip->Show(TRUE);
			m_pImeEdit->SetString("");
			m_stage = INPUT_ROOM;
			break;
		case INPUT_ROOM:
			sprintf(&m_item.strCode[7], "%04d01", strtol(m_pImeEdit->GetString(), NULL, 10));
			AddTelBook(m_item.strName, m_item.strCode);
			
			memmove(m_DataArray[COL], m_DataArray[0], sizeof(m_buf[0]) * COL * m_count);
			strcpy(m_DataArray[0], m_item.strName);
			sprintf(m_DataArray[1], "%c%c%s%c%c%s%c%c%s%c%c%c%c%s", 
				m_item.strCode[1], m_item.strCode[2], GetStringByID(617),								// 区
				m_item.strCode[3], m_item.strCode[4], GetStringByID(618),								// 栋
				m_item.strCode[5], m_item.strCode[6], GetStringByID(619),								// 单元
				m_item.strCode[7], m_item.strCode[8], m_item.strCode[9], m_item.strCode[10], GetStringByID(620));	// 室
			m_count++;
			ShowTelBookList();

			m_pAdd->Show(STATUS_NORMAL);
			m_pIme->SwitchLay(FALSE);
			break;
		default:
			break;
		}
	}

	void GetTelBookList()
	{
		// 获得电话本内容
		for(int i = 0; i < MAX_TELBOOK_NUMBER * COL; i++)
		{
			m_DataArray[i] = m_buf[i];
		}

		TelBookRecord item;
		int count = GetTelBookCount();
		for(int i = 0; i < count; i++)
		{
			GetTelBookData(i, &item);
			strcpy(m_DataArray[i * COL], item.strName);
			sprintf(m_DataArray[i * COL + 1], "%c%c%s%c%c%s%c%c%s%c%c%c%c%s", 
				item.strCode[1], item.strCode[2], GetStringByID(617),								// 区
				item.strCode[3], item.strCode[4], GetStringByID(618),								// 栋
				item.strCode[5], item.strCode[6], GetStringByID(619),								// 单元
				item.strCode[7], item.strCode[8], item.strCode[9], item.strCode[10], GetStringByID(620));	// 室
		}

		m_count = count;
	}

	void ShowTelBookList()
	{
		// 列表显示
		m_pList->SetDataArray(m_count, m_DataArray);
		m_pList->Show();

		if(m_count > 0)
		{
			m_pTip->Show(FALSE);

			// 选择显示
			m_pSelect->SetStart(SELECT_LEFT2, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
			m_pSelect->Show(TRUE);
		}
		else
		{
			m_pSelect->Show(FALSE);

			// 没有电话本记录
			m_pTip->SetSrc(GetStringByID(40065));
			m_pTip->Show(TRUE);
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("calltelbook.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("prev", &m_idPrev);
		GetCtrlByName("next", &m_idNext);
		GetCtrlByName("deleteall", &m_idDeleteAll);
		GetCtrlByName("call", &m_idCall);
		m_pAdd = (CDPButton*)GetCtrlByName("add", &m_idAdd);
		m_pDelete = (CDPButton*)GetCtrlByName("delete", &m_idDelete);

		m_pTip = (CDPStatic*)GetCtrlByName("tip");
		m_pSelect = (CDPStatic*)GetCtrlByName("selected");
		m_pList = (CDPListView*)GetCtrlByName("listview", &m_idList);

		m_pDlg = (CLayOut*)GetCtrlByName("layout_ask");
		m_pDlgTip = (CDPStatic*)GetCtrlByName("ask_tip");
		GetCtrlByName("ask_cancle", &m_idCancel);
		GetCtrlByName("ask_ok", &m_idOK);

		m_pIme = (CLayOut*)GetCtrlByName("layout_input");
		GetCtrlByName("input_back", &m_idImeBack);
		m_pImeTip = (CDPStatic*)GetCtrlByName("input_tip");
		m_pImeEdit = (CEditBox*)GetCtrlByName("editbox");

		GetTelBookList();
		ShowTelBookList();
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	DWORD m_idDeleteAll;
	DWORD m_idCall;

	DWORD m_idAdd;
	CDPButton* m_pAdd;
	DWORD m_idDelete;
	CDPButton* m_pDelete;

	DWORD m_idList;
	CDPListView* m_pList;
	CDPStatic* m_pTip;
	CDPStatic* m_pSelect;

	int m_count;
	char m_buf[MAX_TELBOOK_NUMBER * COL][64];
	char* m_DataArray[MAX_TELBOOK_NUMBER * COL];

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

	TelBookRecord m_item;
	int m_stage;
};

CAppBase* CreateTelBookApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTelBookApp* pApp = new CTelBookApp(wParam);
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