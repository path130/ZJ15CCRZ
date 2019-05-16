#pragma once
#include "CCtrlBase.h"
class CMTable:public CCtrlBase
{
public:
	CMTable(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_edgeheight = m_edgewidth = 1;
		m_color[STATUS_NORMAL] = 0x00000000;
		m_color[STATUS_PRESSED] = 0x00000000;
		m_textcolor[STATUS_NORMAL] = 0xffffff;
		m_edgecolor = 0xffffffff;
		m_tablebk = 0;
		m_tablist = NULL;
		m_DataArray = NULL;
		m_btabresp = NULL;
		m_bIsPressed = FALSE;
		memset(m_tabType, 0, sizeof(m_tabType));
		memset(m_buttommap, 0, BUTTON_MAX * sizeof(DWORD));
		m_LastIndex = 0;

		m_hBakPng = NULL;
	}
	~CMTable()
	{
		if(m_tablebk != NULL)
		{
			m_pSpr->CloseBlk(m_tablebk);
			m_tablebk = NULL;
		}
		if(m_tablist != NULL)
		{
			free(m_tablist);
			m_tablist = NULL;
		}
		if(m_btabresp != NULL)
		{
			free(m_btabresp);
			m_btabresp = NULL;
		}
		if( m_hBakPng != NULL )
		{
			m_pSpr->CloseBlk( m_hBakPng );
			m_hBakPng = NULL;
		}
	}
	BOOL DoInit(ContentManage*);
	BOOL DoResponse(DWORD x, DWORD y, DWORD statue);
	void Show(DWORD col, DWORD row, DWORD statue);
	void SetDataArray(char** parry);
	void SetRespose(DWORD col, DWORD row, BOOL isres);
	void DoKeyResponse(DWORD rawkey, DWORD statue);
private:
	DWORD m_col;			// 列数
	DWORD m_colwidth;		// 每列的像素宽
	DWORD m_row;
	DWORD m_rowheight;
	DWORD m_titlewidth;
	DWORD m_titleheight;

	DWORD m_edgewidth;
	DWORD m_edgeheight;
	DWORD m_edgecolor;

	DWORD m_color[STATUS_MAX];
	DWORD m_textcolor[STATUS_MAX];

	DWORD m_textsize;

	DWORD m_titlecolor;
	DWORD m_titlesize;
	HANDLE m_hBakPng;

	char m_tabType[32];
	BOOL m_bIsPressed;
	DWORD m_LastIndex;
	HANDLE m_tablebk;
	RECT* m_tablist;
	BOOL* m_btabresp;
	DWORD x_off[16];
	DWORD y_off[16];
	char** m_DataArray;
	DWORD m_maxkey;
	BOOL m_bAllDis;
	DWORD m_buttommap[BUTTON_MAX];
};

