#pragma once
#include "CCtrlBase.h"
class CTable:public CCtrlBase
{
public:
	CTable(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_edgewidth = 1;
		m_color[STATUS_NORMAL] = 0x00000000;
		m_color[STATUS_PRESSED] = 0x80000000;
		m_textcolor[STATUS_NORMAL] = 0xffffff;
		m_edgecolor = 0xffffffff;
		m_tablebk = 0;
		m_tablist = NULL;
		m_DataArray = NULL;
		m_btabresp = NULL;
		m_bIsPressed = FALSE;
		m_LastIndex = 0;
		m_curFocusIndex[0] = -1;
		m_curFocusIndex[1] = -1;
	}
	~CTable()
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
	}
	BOOL DoInit(ContentManage*);
	void FocusOnFirst();
	void FocusNext(BOOL rowBased);
	void StrikeOut();
	BOOL DoResponse(DWORD x, DWORD y, DWORD statue);
	void Show(DWORD col, DWORD row, DWORD statue);
	void SetDataArray(char** parry);
	void SetRespose(DWORD col, DWORD row, BOOL isres);
private:
	DWORD m_col;			// 列数
	DWORD m_colwidth;		// 每列的像素宽
	DWORD m_row;
	DWORD m_rowheight;
	DWORD m_titlewidth;
	DWORD m_titleheight;

	DWORD m_edgewidth;
	DWORD m_edgecolor;
	BOOL m_bHideGrid;

	DWORD m_color[STATUS_MAX];
	DWORD m_textcolor[STATUS_MAX];

	DWORD m_textsize;

	DWORD m_titlecolortop;
	DWORD m_titlecolorleft;
	DWORD m_titlesize;
	BOOL m_bHideTitle;

	int m_curFocusIndex[2];   //0为列 1为行
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
};

