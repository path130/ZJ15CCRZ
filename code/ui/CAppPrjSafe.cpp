#include "CCtrlModules.h"

#define COL 4		// 表格中的列数
#define ROW 4		// 表格中的行数

class CPrjSafeApp : public CAppBase
{
public:
	CPrjSafeApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPrjSafeApp(void)
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
			else if(wParam == m_idSave)
			{
				if(!SetSafeAreaSet(m_pSafeSet))
				{
					DPPostMessage(MSG_SHOW_STATUE, 2024, 0, 0);
				}
				else
				{
					PostSafeMessage(SMSG_SETTING_CHANGE, 0, 0, 0);
					DPPostMessage(MSG_SHOW_STATUE, 2004, 0, 0);
					DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			else if(wParam == m_idPrev)
			{
				if(m_nPage > 0)
				{
					m_nPage--;
					m_pTable->SetDataArray(&m_DataArray[m_nPage * COL * ROW]);
				}
			}
			else if(wParam == m_idNext)
			{
				if(m_nPage < SAFE_MAX_NUMBER / ROW - 1)
				{
					m_nPage++;
					m_pTable->SetDataArray(&m_DataArray[m_nPage * COL * ROW]);
				}
			}
			else if(wParam == m_idTable)
			{
				int col = lParam;
				int row = zParam + m_nPage*4;
				char* text;

				if (m_pSafeSet[row].Type > STYPE_MAX)
					break;

				switch(col)
				{
				case 0:				//Area
					m_pSafeSet[row].Area = (m_pSafeSet[row].Area + 1) % SAREA_MAX;
					strcpy(m_DataArray[row * COL +col], GetStringByID(m_pSafeSet[row].Area + 3001));
					break;
				case 1:				//Type
					m_pSafeSet[row].Type = (m_pSafeSet[row].Type + 1) % STYPE_MAX;
					strcpy(m_DataArray[row*COL+col], GetStringByID( m_pSafeSet[row].Type + 3101 ) );
					if( m_pSafeSet[row].Type < STYPE_MAGNETIC )
						m_pSafeSet[row].Status = SSTATUS_SET_DEFENSE;
					else
						m_pSafeSet[row].Status = SSTATUS_CANCEL_DEFENSE;
					break;
				case 2:				//onOff
					m_pSafeSet[row].OnOff = !m_pSafeSet[row].OnOff;
					strcpy(m_DataArray[row*COL+col], GetStringByID( m_pSafeSet[row].OnOff + 3201 ) );
					break;
				case 3:
					m_pSafeSet[row].Level = !m_pSafeSet[row].Level;
					strcpy(m_DataArray[row*COL+col], GetStringByID( m_pSafeSet[row].Level + 3203 ) );
					break;
				}
				m_pTable->Show( lParam, zParam, STATUS_NORMAL );
			}
			break;
		case MSG_PRIVATE:
			if(wParam == m_IdBase)
			{
				GetSafeAreaSet(m_pSafeSet);
				for(int i = 0; i < SAFE_MAX_NUMBER * COL; i++)
				{
					m_DataArray[i] = m_buf[i];
				}

				char *strArea, *strType;
				for( int i = 0; i < SAFE_MAX_NUMBER; i++ )
				{
					if (m_pSafeSet[i].Area == SAREA_LOCALHOST)
					{
						// 室内机
						strArea = GetStringByID(3089);
					}
					else
					{
						// 厨房、卧室...
						strArea = GetStringByID(m_pSafeSet[i].Area + 3001);
					}

					if (m_pSafeSet[i].Type == STYPE_TAMPER)
					{
						// 防拆
						strType = GetStringByID(3109);
					}
					else if (m_pSafeSet[i].Type == STYPE_VOLTAGE)
					{
						// 低压
						strType = GetStringByID(3110);
					}
					else
					{
						// 紧急、烟感
						strType = GetStringByID(m_pSafeSet[i].Type + 3101 );
					}

					strcpy(m_DataArray[i * COL], strArea);
					strcpy(m_DataArray[i * COL + 1], strType);
					strcpy(m_DataArray[i * COL + 2], GetStringByID(m_pSafeSet[i].OnOff + 3201));
					strcpy(m_DataArray[i * COL + 3], GetStringByID(m_pSafeSet[i].Level + 3203));
				}

				m_pTable->SetDataArray(m_DataArray);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prjsafe.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("prev", &m_idPrev);
		GetCtrlByName("next", &m_idNext);
		GetCtrlByName("save", &m_idSave);
		m_pTable = (CMTable*)GetCtrlByName("table", &m_idTable);

		if(GetDefenseIsAlarming())
		{
			// 正在报警中,请先撤防!
			DPPostMessage(MSG_SHOW_STATUE, 2018, 0, 0);
			DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
			DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
		}
		else if(GetDefenseStatus())
		{
			// 已布防,请先撤防
			DPPostMessage(MSG_SHOW_STATUE, 2019, 0, 0);
			DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
			DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
		}
		else
		{
			m_nPage = 0;
			DPPostMessage(MSG_PRIVATE, PRJ_SAFE_APPID, 0, 0);
		}
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	DWORD m_idSave;
	DWORD m_idTable;
	CMTable* m_pTable;

	char m_buf[SAFE_MAX_NUMBER * COL][16];
	char* m_DataArray[SAFE_MAX_NUMBER * COL];

	int m_nPage;
	AreaSet m_pSafeSet[SAFE_MAX_NUMBER];
};

CAppBase* CreatePrjSafeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjSafeApp* pApp = new CPrjSafeApp(wParam);
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