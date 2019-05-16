#pragma once
#include "CCtrlBase.h"

class CMButton:public CCtrlBase
{
public:
	CMButton(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		DWORD i;
		for(i = 0; i < STATUS_MAX; i++)
		{
			srcpng[i][0] = 0;
			bakpng[i][0] = 0;
		}
		m_bStatus = STATUS_NORMAL;
		m_bIsHide = FALSE;
	}
	~CMButton()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}
	BOOL DoInit(ContentManage *);
	BOOL DoResponse(DWORD xoff, DWORD yoff, DWORD statue);
	void Show(DWORD statue);
	void Hide(void);
	void SetBakPng(char* pngname)
	{
		DWORD i;
		for(i = 0; i < STATUS_MAX; i++)
			strcpy(bakpng[i], pngname);
	}
private:
	char bakpng[STATUS_MAX][64];
	char srcpng[STATUS_MAX][64];
	DWORD m_bStatus;
	BOOL m_bIsHide;
};


