#include "CCtrlModules.h"

#define MAX_MP3_COUNT			64

typedef struct
{
	char mp3Path[MAX_MP3_COUNT][MAX_PATH];
	char mp3Name[MAX_MP3_COUNT][32];
	char* pArray[MAX_MP3_COUNT];
	int mp3Count;
}MP3List;

class CSetRingApp : public CAppBase
{
public:
	CSetRingApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetRingApp(void)
	{
	}

	BOOL DoPause()
	{
		StopRing();
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
		case TOUCH_MESSAGE:
			if(wParam == m_idBack)
			{
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idPrev)
			{
				if(m_pList->PrevPage())
				{
					m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP);
					m_pSelect->Show(TRUE);
				}
			}
			else if(wParam == m_idNext)
			{
				if(m_pList->NextPage())
				{
					m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP);
					m_pSelect->Show(TRUE);
				}
			}
			else if(wParam == m_idStop)
			{
				StopRing();
			}
			else if(wParam == m_idPlay)
			{
				StopRing();

				char filename[128];
				int curPtr = m_pList->GetCurPtr();
				sprintf(filename, "%s/%s", m_mp3List.mp3Path[curPtr], m_mp3List.mp3Name[curPtr]);
				m_hMp3 = PlayMp3(filename, 1);
				//if(m_hMp3)
				//{
				//	SetMp3Volume(m_hMp3, GetRingVol() * 0x11111111);
				//}
			}
			else if(wParam == m_idList)
			{
				m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
				m_pSelect->Show(TRUE);
			}
			else if(wParam == m_idCallIn)
			{
				int ptr = m_pList->GetCurPtr();
				char ringName[128];
				sprintf(ringName, "%s/%s", m_mp3List.mp3Path[ptr], m_mp3List.mp3Name[ptr] );
				SetRingName(FALSE, ringName);
				m_pCallIn->SetSrc(m_mp3List.mp3Name[ptr]);
				m_pCallIn->Show(TRUE);
				DPPostMessage(MSG_SHOW_STATUE, 2004, 0, 0);
			}
			else if(wParam == m_idCallOut)
			{
				int ptr = m_pList->GetCurPtr();
				char ringName[128];
				sprintf(ringName, "%s/%s", m_mp3List.mp3Path[ptr], m_mp3List.mp3Name[ptr] );
				SetRingName(TRUE, ringName);
				m_pCallOut->SetSrc(m_mp3List.mp3Name[ptr]);
				m_pCallOut->Show(TRUE);
				DPPostMessage(MSG_SHOW_STATUE, 2004, 0, 0);
			}
			break;
		}
		return TRUE;
	}

	static void AddMp3List(char* dir, char* fileName, void* param)
	{
		char buf[MAX_PATH] = {0};
		int len = strlen(fileName);
		int i = 0;
		for(; i < len; i++)
		{
			buf[i] = fileName[i];
			if(buf[i] >= 'A' && buf[i] <= 'Z')
				buf[i] += 'a' - 'A';
		}
		buf[i] = 0;
		if(strstr(buf, ".mp3") == NULL)
			return;

		MP3List* pMp3List = (MP3List*)param;
		if(pMp3List->mp3Count >= MAX_MP3_COUNT)
			return;

		strcpy(pMp3List->mp3Path[pMp3List->mp3Count], dir);
		strcpy(pMp3List->mp3Name[pMp3List->mp3Count], fileName);
		pMp3List->mp3Count++;
	}

	void InitMp3List(MP3List *pMp3List)
	{
		memset(pMp3List, 0, sizeof(MP3List));
		for(int i = 0; i < MAX_MP3_COUNT; i++)
			pMp3List->pArray[i] = pMp3List->mp3Name[i];

		char flashDir[MAX_PATH];
		char sdCardDir[MAX_PATH];

		sprintf(flashDir, "%s/ring", FLASHDIR);
		sprintf(sdCardDir, "%s/ring", SDCARD);

		FindFileFromDirectory(flashDir, AddMp3List, pMp3List);
		FindFileFromDirectory(sdCardDir, AddMp3List, pMp3List);
	}

	char* GetRingMp3(BOOL bCallOut, char* buf)
	{
		char* ringName = GetRingName(bCallOut);
		int i = strlen(ringName) - 1;
		for( ; i > 0; i-- )
		{
			if(ringName[i] == '/')
				break;
		}
		sprintf(buf, "%s%s", GetStringByID(1042), &ringName[i+1]);
		return buf;
	}

	void StopRing()
	{
		if(m_hMp3)
		{
			StopMp3(m_hMp3);
			m_hMp3 = NULL;
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setring.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("prev", &m_idPrev);
		GetCtrlByName("next", &m_idNext);
		GetCtrlByName("pause", &m_idStop);
		GetCtrlByName("listen", &m_idPlay);
		GetCtrlByName("callout", &m_idCallOut);
		GetCtrlByName("callin", &m_idCallIn);
		m_pCallOut = (CDPStatic*)GetCtrlByName("callOutTip");
		m_pCallIn = (CDPStatic*)GetCtrlByName("callInTip");
		m_pSelect = (CDPStatic*)GetCtrlByName("selected");
		m_pList = (CDPListView*)GetCtrlByName("listview", &m_idList);

		m_hMp3 = NULL;
		InitMp3List(&m_mp3List);
		m_pList->SetDataArray(m_mp3List.mp3Count, m_mp3List.pArray);
		m_pList->Show();

		if(m_mp3List.mp3Count > 0)
		{
			m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP);
			m_pSelect->Show(TRUE);
		}

		char buf[128];
		m_pCallIn->SetSrc(GetRingMp3(FALSE, buf));
		m_pCallOut->SetSrc(GetRingMp3(TRUE, buf));
		m_pCallIn->Show(TRUE);
		m_pCallOut->Show(TRUE);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	DWORD m_idStop;
	DWORD m_idPlay;
	DWORD m_idCallIn;
	DWORD m_idCallOut;
	CDPStatic* m_pCallIn;
	CDPStatic* m_pCallOut;
	CDPStatic* m_pSelect;

	DWORD m_idList;
	CDPListView* m_pList;
	MP3List m_mp3List;
	HANDLE m_hMp3;
};

CAppBase* CreateSetRingApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetRingApp* pApp = new CSetRingApp(wParam);
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