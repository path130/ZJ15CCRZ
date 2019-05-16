#include "CCtrlModules.h"
#include "dpvideo.h"
#include "PhoneApp.h"

#define COL		3

class CRecLiuyingApp : public CAppBase
{
public:
	CRecLiuyingApp(DWORD pHwnd):CAppBase(pHwnd)
	{
		m_bPlay = FALSE;
		m_hAudio = NULL;
		m_pLiuyanBuf = NULL;
	}

	~CRecLiuyingApp(void)
	{
	}

	BOOL DoPause()
	{
		StopLiuyan();
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
				StopLiuyan();
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
					int len = GetLiuyingData(ptr, &pbuf, &m_nLiuyanLen, &m_pLiuyanBuf);
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

					// 播放留言
					if(m_pLiuyanBuf)
						PlayLiuyan();
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
				ShowLiuyingList();
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

	static DWORD AddPlayProc(HANDLE pParam)
	{
		CRecLiuyingApp* pThis = (CRecLiuyingApp*)pParam;
		int pos = 0;

		while(pThis->m_bPlay)
		{
			if(pos < pThis->m_nLiuyanLen)
			{
				AudioPlayAddPlay(pThis->m_hAudio, pThis->m_pLiuyanBuf + pos, 640);
				pos += 640;
				DPSleep(40);
			}
			else
			{
				break;
			}
		}
		return 0;
	}

	void PlayLiuyan()
	{
		StopLiuyan();

		m_bPlay = TRUE;
		m_hAudio = AudioPlayCreate();
		AudioPlayStart(m_hAudio, 8000, 1);
		m_hLiuyanThread = DPThreadCreate(0x4000, AddPlayProc, this, TRUE, 5);
	}

	void StopLiuyan()
	{
		if(m_bPlay)
		{
			m_bPlay = FALSE;

			AudioPlayStoped(m_hAudio);
			AudioPlayDestroy(m_hAudio);
			DPThreadJoin(m_hLiuyanThread);
			free(m_pLiuyanBuf);
			m_pLiuyanBuf = NULL;
		}
	}

	void DoDelete(BOOL bDeleteAll)
	{
		if(bDeleteAll)
		{
			ResetLiuying();
			m_count = 0;
		}
		else
		{
			int ptr = m_pList->GetCurPtr();
			DeleteLiuying(ptr);
			memmove(m_DataArray[ptr * COL], m_DataArray[(ptr + 1) * COL], sizeof(m_buf[0]) * COL * (m_count - ptr - 1));
			m_count--;
		}
	}

	void GetLiuyingList()
	{
		// 获得留影留言内容
		LiuyingRecord item[MAX_LIUYING_NUMBER];
		SYSTEMTIME systime;
		int language = GetLanguage();
		int count = GetLiuyingRecord(item);

		for(int i = 0; i < MAX_LIUYING_NUMBER * COL; i++)
		{
			m_DataArray[i] = m_buf[i];
		}

		for(int i = 0; i < count; i++)
		{
			Code2Name(m_DataArray[i * COL], item[i].code);

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

	void ShowLiuyingList()
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

			// 没有留影留言
			m_pTip->SetSrc(GetStringByID(30036));
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
		InitFrame("recliuying.xml");
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

		GetLiuyingList();
		ShowLiuyingList();
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
	char m_buf[MAX_LIUYING_NUMBER * COL][32];
	char* m_DataArray[MAX_LIUYING_NUMBER * COL];
	BOOL m_bRead[MAX_PHOTO_NUMBER];

	// 留言
	int m_nLiuyanLen;
	char* m_pLiuyanBuf;
	HANDLE m_hAudio;
	HANDLE m_hLiuyanThread;
	BOOL m_bPlay;

	// 确定删除
	CLayOut* m_pDlg;
	CDPStatic* m_pDlgTip;
	DWORD m_idOK;
	DWORD m_idCancel;
	BOOL m_bDeleteAll;
};

CAppBase* CreateRecLiuyingApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CRecLiuyingApp* pApp = new CRecLiuyingApp(wParam);
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