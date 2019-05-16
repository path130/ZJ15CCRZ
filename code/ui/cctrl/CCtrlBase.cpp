#include "CCtrlBase.h"

BOOL DoCtrlOp(EventMap* pEvent, DWORD x, DWORD y, DWORD statue)
{
	CCtrlBase* pCtrl;
	if(pEvent->pCtrl != NULL)
	{
		pCtrl = (CCtrlBase*)pEvent->pCtrl;
		if(pCtrl != NULL)
		{
			return pCtrl->DoResponse(x, y, statue);
		}
	}
	return FALSE;
}

void DoKeybdOp(KeybdMap* pEvent, DWORD rawkey, DWORD statue)
{
	CCtrlBase* pCtrl;
	if(pEvent->pCtrl != NULL)
	{
		pCtrl = (CCtrlBase*)pEvent->pCtrl;
		pCtrl->DoKeyResponse(rawkey, statue);
	}
}

HANDLE CCtrlBase::DrawCscBmp(HANDLE horg, DWORD csc, SIZE* rsize)
{
	SIZE size;
	HANDLE hstep1;
	SIZE newsize;

	if(!m_pSpr->GetSize(horg, &size))
		return NULL;

	hstep1 = m_pSpr->DupBlock(horg);
	switch(csc & 0xf)
	{
	case CSC_BW:
		m_pSpr->CscBw(hstep1);
		break;
	case CSC_REVERSION:
		m_pSpr->CscRevert(hstep1);
		break;
	case CSC_COLOR:
		m_pSpr->CscColor(hstep1, csc >> 8);
		break;
	}
	return hstep1;
}

HANDLE CCtrlBase::DrawCscBmp(char* pngname, DWORD csc, SIZE* size)
{
	HANDLE hstep1;
	HANDLE hstep2;

	if(strlen(pngname) == 0)
		return NULL;
	hstep2 = m_pSpr->LoadImage(pngname, size);
	if(hstep2 == NULL)
		return NULL;
	hstep1 = m_pSpr->DupBlock(hstep2);
	m_pSpr->CloseBlk(hstep2);
	switch(csc & 0xf)
	{
		case CSC_BW:
			m_pSpr->CscBw(hstep1);
			break;
		case CSC_REVERSION:
			m_pSpr->CscRevert(hstep1);
			break;
		case CSC_COLOR:
			m_pSpr->CscColor(hstep1, csc >> 8);
			break;
	}

	return hstep1;
}


