#pragma once
#include "CCtrlBase.h"

#define	MAXKEY	34

class CEnKeyboard:public CCtrlBase
{
public:
	CEnKeyboard(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		memset(m_Range, 0, sizeof(RECT) * MAXKEY);
		m_bDigigal = FALSE;
		m_bBig = FALSE;
	}
	~CEnKeyboard()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}

	void SetWidth(const DWORD width)
	{
		m_width = width;
	}
	void SetHeight(const DWORD height)
	{
		m_height = height;
	}
	BOOL DoInit(ContentManage*);
	BOOL DoResponse(DWORD xoff, DWORD yoff, DWORD statue);
protected:
	void ShowFirstLine(void);
	void AddNewItem(DWORD id, ContentManage* pCm);
	void DoAction(DWORD id);	
	BOOL InitView(char* fname);
	void Show(DWORD status, DWORD id);
	DWORD m_lastId;
	RECT m_Range[MAXKEY];
	BOOL m_bDigigal;
	BOOL m_bBig;
};