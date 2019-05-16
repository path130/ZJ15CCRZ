#pragma once

#include "CCtrlBase.h"

class CMKeyboard:public CCtrlBase
{
public:
	CMKeyboard(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_isPressed = FALSE;
		memset(m_buttommap, 0, BUTTON_MAX * sizeof(DWORD));
		m_dwPressed = 0;
		m_textsize = 36;
	}
	~CMKeyboard()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}
	BOOL DoInit(ContentManage*);
	BOOL DoResponse(DWORD x, DWORD y, DWORD statue);
private:
	void Show(DWORD index, DWORD status);

	DWORD m_kwidth;
	DWORD m_kheight;
	DWORD m_col;
	DWORD m_row;
	DWORD m_maxkey;
	DWORD m_textsize;
	char m_keymap[64];
	char m_keyout[64];

	BOOL m_isPressed;
	DWORD m_dwPressed;
	DWORD m_buttommap[BUTTON_MAX];
};

