#include "CCtrlModules.h"

#define MAX_WALLPAPER_COUNT			64

typedef struct
{
	char jpgPath[MAX_WALLPAPER_COUNT][MAX_PATH];
	char jpgName[MAX_WALLPAPER_COUNT][32];
	char* pArray[MAX_WALLPAPER_COUNT];
	int jpgCount;
}JpgList;

class CSetWallPaperApp : public CAppBase
{
public:
	CSetWallPaperApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetWallPaperApp(void)
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
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
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
			else if(wParam == m_idSetting)
			{
				char jpgName[128];
				sprintf(jpgName, "%s/%s", m_jpgList.jpgPath[m_pList->GetCurPtr()], m_jpgList.jpgName[m_pList->GetCurPtr()] );

				int width, height;
				BOOL ret = GetJpgSize(jpgName, &width, &height);
				if(ret
					&& (width == FRAME_WIDTH)
					&& (height == FRAME_HEIGHT))
				{
					SetBgName(jpgName);
					DPDeleteFile(DEFAULT_BK_JPG);
					DPCopyFile(DEFAULT_BK_JPG, jpgName);
					DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
				else
				{
					DPPostMessage(MSG_SHOW_STATUE, 70033, 0, 0);		// Í¼Æ¬¸ñÊ½´íÎó
					break;
				}
			}
			else if(wParam == m_idList)
			{
				m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP + SELECT_HEIGHT * m_pList->GetCurPagePtr());
				m_pSelect->Show(TRUE);
			}
			break;
		}
		return TRUE;
	}

	static void AddJpgList(char* dir, char* fileName, void* param)
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
		if(!(strstr(buf, ".jpg") || strstr(buf, ".jpeg")))
			return;

		JpgList* pJpgList = (JpgList*)param;
		if(pJpgList->jpgCount >= MAX_WALLPAPER_COUNT)
			return;

		strcpy(pJpgList->jpgPath[pJpgList->jpgCount], dir);
		strcpy(pJpgList->jpgName[pJpgList->jpgCount], fileName);
		pJpgList->jpgCount++;
	}

	void InitJpgList(JpgList *pJpgList)
	{
		memset(pJpgList, 0, sizeof(JpgList));
		for(int i = 0; i < MAX_WALLPAPER_COUNT; i++)
			pJpgList->pArray[i] = pJpgList->jpgName[i];

		char flashDir[MAX_PATH];
		char sdCardDir[MAX_PATH];
		sprintf(flashDir, "%s/wallpaper", FLASHDIR);
		sprintf(sdCardDir, "%s/wallpaper", SDCARD);

		FindFileFromDirectory(flashDir, AddJpgList, pJpgList);
		FindFileFromDirectory(sdCardDir, AddJpgList, pJpgList);
	}

	char* GetJpgName(char* buf)
	{
		char* jpgName = GetBgName();
		int i = strlen(jpgName) - 1;
		for( ; i > 0; i-- )
		{
			if(jpgName[i] == '/')
				break;
		}
		sprintf(buf, "%s%s", GetStringByID(70023), &jpgName[i+1]);
		return buf;
	}

	BOOL GetJpgSize(char* filename, int* width, int* height)
	{
		FILE* pFile = fopen(filename, "rb");
		if(pFile)
		{
			WORD head, end;
			fread(&head, 1, 2, pFile);

			fseek(pFile, -2, SEEK_END);
			fread(&end, 1, 2, pFile);

			if(head != 0xD8FF || end != 0xD9FF)
			{
				fclose(pFile);
				return FALSE;
			}

			fseek(pFile, 2, SEEK_SET);

			int type = -1;
			do
			{
				int ff = -1;
				int pos = 0;

				do{
					ff = fgetc(pFile);
				}while(ff != 0xFF);

				do{
					type = fgetc(pFile);
				}while(type == 0xFF);

				switch (type)
				{
				case 0x00:
				case 0x01:
				case 0xD0:
				case 0xD1:
				case 0xD2:
				case 0xD3:
				case 0xD4:
				case 0xD5:
				case 0xD6:
				case 0xD7:
					break;
				case 0xC0:
				case 0xC2:
					{
						fseek(pFile, 3, SEEK_CUR);
						int h = fgetc(pFile) * 256;
						h += fgetc(pFile);
						int w = fgetc(pFile) * 256;
						w += fgetc(pFile);
						*height = h;
						*width = w;
						fclose(pFile);
						return TRUE;
					}
					break;
				default:
					pos = fgetc(pFile) * 256 + fgetc(pFile);
					fseek(pFile, pos - 3, SEEK_CUR);
					break;
				}
			}while(type != 0xDA);

			fclose(pFile);
		}

		return FALSE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setwallpaper.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("prev", &m_idPrev);
		GetCtrlByName("next", &m_idNext);
		GetCtrlByName("setting", &m_idSetting);
		m_pTip = (CDPStatic*)GetCtrlByName("wallPaperTip");
		m_pSelect = (CDPStatic*)GetCtrlByName("selected");
		m_pList = (CDPListView*)GetCtrlByName("listview", &m_idList);

		InitJpgList(&m_jpgList);
		m_pList->SetDataArray(m_jpgList.jpgCount, m_jpgList.pArray);
		m_pList->Show();

		if(m_jpgList.jpgCount > 0)
		{
			m_pSelect->SetStart(SELECT_LEFT, SELECT_TOP);
			m_pSelect->Show(TRUE);
		}

		char buf[128];
		m_pTip->SetSrc(GetJpgName(buf));
		m_pTip->Show(TRUE);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idPrev;
	DWORD m_idNext;
	DWORD m_idSetting;
	CDPStatic* m_pTip;
	CDPStatic* m_pSelect;

	DWORD m_idList;
	CDPListView* m_pList;
	JpgList m_jpgList;
};

CAppBase* CreateSetWallPaperApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetWallPaperApp* pApp = new CSetWallPaperApp(wParam);
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