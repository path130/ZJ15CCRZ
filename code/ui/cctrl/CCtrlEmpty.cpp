#include "CCtrlEmpty.h"

BOOL CEmpty::DoInit(ContentManage * pCm)
{
	char* pcontent;

	if((pcontent = pCm->FindContentByName("left")) == NULL)
		return FALSE;
	m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) == NULL)
		return FALSE;
	m_top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("width")) == NULL)
		return FALSE;
	m_width = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("height")) == NULL)
		return FALSE;
	m_height = strtol(pcontent, NULL, 10);

	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	m_msgid = m_pLayer->RequestMsgId();
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_EMPTY, "ebutton", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_EMPTY, pcontent, this, m_msgid);
	return TRUE;
}

BOOL CEmpty::DoResponse(DWORD xoff, DWORD yoff, DWORD statue)
{
	switch(statue)
	{
		case TOUCH_UP:
			DPPostMessage(TOUCH_MESSAGE, m_msgid, 0, 0);
			break;
	}
	return TRUE;
}

void CEmpty::Show(DWORD color)
{
	HANDLE hbk = m_pSpr->ReqTempBlk(m_width , m_height);
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = m_width;
	rect.bottom = m_height;
	m_pSpr->FillSolidRect(hbk , &rect , color);
	m_pSpr->BitBlt(m_pLayer->m_frame , m_left , m_top , m_width , m_height , hbk , 0 , 0);
	m_pSpr->CloseBlk(hbk);
}

