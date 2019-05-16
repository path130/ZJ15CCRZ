#include "CCtrlModules.h"

#define COL		2

class CSafeSetApp : public CAppBase
{
public:
	CSafeSetApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSafeSetApp(void)
	{
	}

	BOOL DoPause()
	{
		if(!m_bExit)
		{
			m_bExit = TRUE;
			SetDefenseStatus(TRUE);
			AddDefenseRecord(SMODE_LEAVE, SMODE_UNSET);
			DPPostMessage(MSG_BROADCAST, SAFE_CHANGE, TRUE, 0);
			//SendSyncData(SYNC_TYPE_SET_DEFENSE, NULL, 0);
		}
		return CAppBase::DoPause();
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			if(m_dwTrigger == 0)
			{
				// ��û��̽ͷ�쳣��ʱ���ٿ�ʼ������ʱ
				if(m_dwTimeout < m_dwDelay)
				{
					m_dwTimeout++;
					if(m_dwTimeout < 10)
						PlayWav(DUDU_INDEX, GetRingVol());
					else 
						PlayWav(OK_INDEX, GetRingVol());

					if(m_dwTimeout == 10)
					{
						// �����˳�����
						m_pBack->Hide();
					}

					m_pDelay->SetSrc(m_dwDelay - m_dwTimeout);
					m_pDelay->Show(TRUE);
				}

				if(m_dwTimeout == m_dwDelay)
				{
					// �����ɹ�
					SetDefenseStatus(TRUE);
					AddDefenseRecord(SMODE_LEAVE, SMODE_UNSET);
					DPPostMessage(MSG_BROADCAST, SAFE_CHANGE, TRUE, 0);
					DPPostMessage(MSG_SHOW_STATUE, 2014, 0, 0);
					//SendSyncData(SYNC_TYPE_SET_DEFENSE, NULL, 0);

					m_bExit = TRUE;
					DPPostMessage(MSG_START_APP, m_dwReturnID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			else
			{
				m_dwTrigger = CheckSafeGpio();
			}
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBack)
			{
				m_bExit = TRUE;
				DPPostMessage(MSG_START_APP, m_dwReturnID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		case MSG_BROADCAST:
			if(wParam == ALARM_CHANGE && GetDefenseIsAlarming())
			{
				// ��������
				m_bExit = TRUE;
				DPPostMessage(MSG_START_APP, m_dwReturnID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		}
		return TRUE;
	}

	void InitSafeGpioList()
	{
		AreaSet areaSet[SAFE_MAX_NUMBER];
		GetSafeAreaSet(areaSet);

		int count = 0;
		for(int i = 0; i < SAFE_MAX_NUMBER; i++)
		{
			if(m_dwTrigger & (1 << i))
			{
				strcpy(m_DataArray[COL * count], GetStringByID(areaSet[i].Area + 3001));
				strcpy(m_DataArray[COL * count + 1], GetStringByID(areaSet[i].Type + 3101));
				count++;
			}
		}
		for(int i = count; i < SAFE_MAX_NUMBER; i++)
		{
			m_DataArray[COL * i][0] = 0;
			m_DataArray[COL * i + 1][0] = 0;
		}

		m_pTable->SetDataArray(m_DataArray);
	}

	BOOL Create(DWORD lparam, DWORD zParam)
	{
		InitFrame("safeset.xml");
		m_pBack = (CDPButton*)GetCtrlByName("back", &m_idBack);
		m_pDelay = (CDPStatic*)GetCtrlByName("delay");
		m_pTitle = (CDPStatic*)GetCtrlByName("title");
		m_pTable = (CMTable*)GetCtrlByName("table");

		m_bExit = FALSE;
#ifdef _DEBUG
		m_dwDelay = 5;
#else
		m_dwDelay = GetDelay(DELAY_SAFE);
#endif
		for(int i = 0; i < SAFE_MAX_NUMBER * COL; i++)
			m_DataArray[i] = m_buf[i];

		m_dwTrigger = CheckSafeGpio();
		if(m_dwTrigger)
		{
			// ��ʼ����ʱ������Ƿ���̽ͷ�쳣
			m_pTitle->SetSrc(GetStringByID(60002));
			m_pTitle->Show(TRUE);
			InitSafeGpioList();
		}
		else
		{
			m_pDelay->SetSrc(m_dwDelay);
			m_pDelay->Show(TRUE);
		}

		m_dwReturnID = lparam;
		return TRUE;
	}
private:
	DWORD m_idBack;
	CDPButton* m_pBack;
	CDPStatic* m_pDelay;
	CDPStatic* m_pTitle;
	DWORD m_dwDelay;
	BOOL m_bExit;				// �Ƿ������˳������֮����ͨ�������������ϲ����ɹ�
	DWORD m_dwReturnID;			// ����ID

	char m_buf[SAFE_MAX_NUMBER * COL][8];
	char* m_DataArray[SAFE_MAX_NUMBER * COL];
	DWORD m_dwTrigger;
	CMTable* m_pTable;
};

CAppBase* CreateSafeSetApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSafeSetApp* pApp = new CSafeSetApp(wParam);
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