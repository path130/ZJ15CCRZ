#include "CCtrlModules.h"
#include "dpvideo.h"
#include "PhoneApp.h"

#define COL		3

class CRecPhotoApp : public CAppBase
{
public:
	CRecPhotoApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CRecPhotoApp(void)
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
		case TOUCH_RAW_MESSAGE:
			if(zParam == TOUCH_UP)
			{
				H264DecStop();
				SetTouchDirect(FALSE);
			}
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBack)
			{
				DPPostMessage(MSG_START_APP, RECORD_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idView)
			{
				if(m_count > 0)
				{
					int ptr = m_pList->GetCurPtr();
					// 未读 -> 已读
					if(m_bRead[ptr] == FALSE)
					{
						m_bRead[ptr] = TRUE;
						strcpy(m_DataArray[ptr * COL + 2], GetStringByID(1024));	// 已读	
						m_pList->Show(m_pList->GetCurPagePtr(), STATUS_PRESSED);
					}
					// 显示信息
					char *pbuf;
					int len = GetPhotoData(ptr, &pbuf);
					if(len > 0)
					{
						H264DecStart(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
						for(int i = 0; i < 5; i++)
						{
							H264WriteData(pbuf, len);
							DPSleep(40);
						}
						free(pbuf);
						SetTouchDirect(TRUE);
					}
				}
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
				ShowPhotoList();
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
			ResetPhoto();
			m_count = 0;
		}
		else
		{
			int ptr = m_pList->GetCurPtr();
			DeletePhoto(ptr);
			memmove(m_DataArray[ptr * COL], m_DataArray[(ptr + 1) * COL], sizeof(m_buf[0]) * COL * (m_count - ptr - 1));
			m_count--;
		}
	}

	void GetPhotoList()
	{
		// 获得拍照记录内容
		PhotoRecord item[MAX_PHOTO_NUMBER];
		SYSTEMTIME systime;
		int language = GetLanguage();
		int count = GetPhotoRecord(item);

		for(int i = 0; i < MAX_PHOTO_NUMBER * COL; i++)
		{
			m_DataArray[i] = m_buf[i];
		}

		for(int i = 0; i < count; i++)
		{
			Code2Name(m_DataArray[i * COL], item[i].name);

			DPFileTimeToSystemTime(&item[i].time, &systime);
			if(language == 0)	
			{
				// 中文 2006-01-01
				sprintf(m_DataArray[i * COL + 1], "%04d-%02d-%02d  %02d:%02d",   
					systime.wYear, systime.wMonth, systime.wDay,
					systime.wHour, systime.wMinute);
			}
			else
			{
				// 英文 01-01-2006
				sprintf(m_DataArray[i * COL + 1], "%02d-%02d-%04d  %02d:%02d", 
					systime.wMonth, systime.wDay, systime.wYear,
					systime.wHour, systime.wMinute);
			}

			// 已读未读
			if(item[i].bRead)
				strcpy(m_DataArray[i * COL + 2], GetStringByID(1024));
			else
				strcpy(m_DataArray[i * COL + 2], GetStringByID(1025));

			m_bRead[i] = item[i].bRead;
		}

		m_count = count;
	}

	void ShowPhotoList()
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

			// 没有拍照记录
			m_pTip->SetSrc(GetStringByID(30014));
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
		InitFrame("recphoto.xml");
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

		GetPhotoList();
		ShowPhotoList();
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

	int m_count;
	char m_buf[MAX_PHOTO_NUMBER * COL][32];
	char* m_DataArray[MAX_PHOTO_NUMBER * COL];
	BOOL m_bRead[MAX_PHOTO_NUMBER];

	// 确定删除
	CLayOut* m_pDlg;
	CDPStatic* m_pDlgTip;
	DWORD m_idOK;
	DWORD m_idCancel;
	BOOL m_bDeleteAll;
};

CAppBase* CreateRecPhotoApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CRecPhotoApp* pApp = new CRecPhotoApp(wParam);
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