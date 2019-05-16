#pragma once
#include "CCtrlBase.h"
class CKeyboard:public CCtrlBase
{
public:
	CKeyboard(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_isPressed = FALSE;
		m_dwPressed = 0;
		textsize = 32;
		textsizep = 28;
		textcolorp = 400196;
		textcolor = 0;
	}
	~CKeyboard()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}
	BOOL DoInit(ContentManage*);
	BOOL DoResponse(DWORD x, DWORD y, DWORD statue);
	void SetKeyMap(char* keymap);
private:
	void Show(DWORD index, DWORD status);

	DWORD m_kwidth;
	DWORD m_kheight;
	DWORD m_col;
	DWORD m_row;
	DWORD m_maxkey;

	DWORD textsize;
	DWORD textsizep;
	DWORD textcolor;
	DWORD textcolorp;
	char m_baknormal[64];
	char m_bakpressed[64];
	char m_bakfocus[64];
	char m_keymap[256];

	BOOL m_isPressed;
	DWORD m_dwPressed;
};

