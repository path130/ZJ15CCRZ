#pragma once
#include "CCtrlBase.h"

class CEmpty:public CCtrlBase
{
public:
	CEmpty(CLayOut* pLayer):CCtrlBase(pLayer)
	{
	}
	~CEmpty()
	{
	}
	BOOL DoInit(ContentManage *);
	BOOL DoResponse(DWORD xoff, DWORD yoff, DWORD statue);
	void Show(DWORD color);
};



