#include "CCtrlMButton.h"

BOOL CMButton::DoInit(ContentManage * pCm)
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

	m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

	if((pcontent = pCm->FindContentByName("bakpng")) != NULL)
		strcpy(bakpng[STATUS_NORMAL], pcontent);

	if((pcontent = pCm->FindContentByName("bakpngpress")) != NULL)
		strcpy(bakpng[STATUS_PRESSED], pcontent);

	if((pcontent = pCm->FindContentByName("bakpngfocus")) != NULL)
		strcpy(bakpng[STATUS_FOCUS], pcontent);

	if((pcontent = pCm->FindContentByName("bakpngunack")) != NULL)
		strcpy(bakpng[STATUS_UNACK], pcontent);

	if((pcontent = pCm->FindContentByName("srcpng")) != NULL)
		strcpy(srcpng[STATUS_NORMAL], pcontent);

	if((pcontent = pCm->FindContentByName("srcpngpress")) != NULL)
		strcpy(srcpng[STATUS_PRESSED], pcontent);

	if((pcontent = pCm->FindContentByName("srcpngfocus")) != NULL)
		strcpy(srcpng[STATUS_FOCUS], pcontent);

	if((pcontent = pCm->FindContentByName("ishide")) != NULL)
		m_bIsHide = TRUE;
	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	m_msgid = m_pLayer->RequestMsgId();
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_MBUTTON, "mbutton", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_MBUTTON, pcontent, this, m_msgid);
	if(!m_bIsHide)
		Show(STATUS_NORMAL);
	return TRUE;
}

BOOL CMButton::DoResponse(DWORD xoff, DWORD yoff, DWORD statue)
{
	if((m_bStatus == STATUS_FOCUS)
		|| m_bIsHide)
		return FALSE;
	switch(statue)
	{
		case TOUCH_DOWN:
			Show(STATUS_PRESSED);
			break;
		case TOUCH_UP:
			Show(STATUS_NORMAL);
			DPPostMessage(TOUCH_MESSAGE, m_msgid, 0, 0);
			break;
		case TOUCH_MOVEOUT:
			Show(STATUS_NORMAL);
			break;
	}
	return TRUE;
}

void CMButton::Hide(void)
{
	m_bIsHide = TRUE;
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hFrameBak, 0, 0);
}

void CMButton::Show(DWORD statue)
{
	HANDLE h_bk;
	SIZE srcsize;
	SIZE baksize;
	HANDLE h_png ;
	HANDLE h_bkpng;

	m_bStatus = statue;
	m_bIsHide = FALSE;
	// 如果有背景png，需要显示，无的话，会恢复初始状态
	h_bkpng = m_pSpr->LoadImage(bakpng[statue], &baksize);
	if(h_bkpng != NULL)
	{
		h_bk = m_pSpr->ReqTempBlk(m_width, m_height);
		m_pSpr->BitBlt(h_bk, 0, 0, m_width, m_height, m_hFrameBak, 0, 0);
		m_pSpr->BitBlt(h_bk, (m_width - baksize.cx)/2, (m_height - baksize.cy)/2,
							baksize.cx, baksize.cy, h_bkpng, 0, 0);
		m_pSpr->CloseBlk(h_bkpng);
		h_png = m_pSpr->LoadImage(srcpng[statue], &srcsize);
		if(h_png != NULL)
		{
			m_pSpr->BitBlt(h_bk, (m_width - srcsize.cx)/2, (m_height - srcsize.cy)/2,
								srcsize.cx, srcsize.cy, h_png, 0, 0);
		}
		m_pSpr->CloseBlk(h_png);
		m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, h_bk, 0, 0);
		m_pSpr->CloseBlk(h_bk);
	}
	else
		m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hFrameBak, 0, 0);
}

