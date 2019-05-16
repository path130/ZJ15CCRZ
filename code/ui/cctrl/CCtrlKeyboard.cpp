#include "CCtrlKeyboard.h"

BOOL CKeyboard::DoInit(ContentManage* pCm)
{
	char* pcontent;
	DWORD i;

	if((pcontent = pCm->FindContentByName("row")) == NULL)
		return FALSE;
	m_row = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("co")) == NULL)
		return FALSE;
	m_col = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("width")) == NULL)
		return FALSE;
	m_width = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("height")) == NULL)
		return FALSE;
	m_height = strtol(pcontent, NULL, 10);

	m_left = (m_pLayer->m_width - m_width)/2;
	m_top = (m_pLayer->m_height - m_height)/2;

	if((pcontent = pCm->FindContentByName("left")) != NULL)
		m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) != NULL)
		m_top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("baknormal")) == NULL)
		return FALSE;
	strcpy(m_baknormal, pcontent);
	strcpy(m_bakpressed, pcontent);
	strcpy(m_bakfocus, pcontent);
	if((pcontent = pCm->FindContentByName("bakpressed")) != NULL)
		strcpy(m_bakpressed, pcontent);
	if((pcontent = pCm->FindContentByName("bakfocus")) != NULL)
		strcpy(m_bakfocus, pcontent);

	m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

	if((pcontent = pCm->FindContentByName("keymap")) == NULL)
		return FALSE;

	memset(m_keymap, 0, 512);
	strcpy(m_keymap, pcontent);
	for(i = 0; i < strlen(m_keymap); i++)
	{
		if((m_keymap[i] >= 'a')
			&& (m_keymap[i] <='z'))
			m_keymap[i] -= 0x20;
	}

	m_kwidth = m_width/m_col;
	m_kheight = m_height/m_row;
	m_maxkey = m_col * m_row;

	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_KEYBOARD, "keybd", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_KEYBOARD, pcontent, this, m_msgid);

	for(i = 0; i < m_col * m_row; i++)
		Show(i, STATUS_NORMAL);
	return TRUE;
}

void CKeyboard::Show(DWORD index, DWORD status)
{
	DWORD col, row;
	HANDLE hbak = NULL;
	HANDLE himage = NULL;
	SIZE size;
	char keyname[32];
	DWORD wide;
	DWORD csize;
	DWORD ccolor;

	row = index/m_col;
	col = index%m_col;
	hbak = m_pSpr->ReqTempBlk(m_kwidth, m_kheight);
	m_pSpr->BitBlt(hbak, 0, 0,
					m_kwidth, m_kheight,
					m_hFrameBak, col * m_kwidth, row * m_kheight);
	csize = textsize;
	ccolor = textcolor;
	if(status == STATUS_NORMAL)
		himage = m_pSpr->LoadImage(m_baknormal, &size);
	else if(status == STATUS_PRESSED)
	{
		himage = m_pSpr->LoadImage(m_bakpressed, &size);
		csize = textsizep;
		ccolor = textcolorp;
	}
	else
		himage = m_pSpr->LoadImage(m_bakfocus, &size);
		
	m_pSpr->BitBlt(hbak, (m_kwidth - size.cx)/2, (m_kheight - size.cy)/2, size.cx, size.cy, himage, 0, 0);
	m_pSpr->CloseBlk(himage);
	keyname[0] = m_keymap[index];
	keyname[1] = 0;
	himage = m_pSpr->LoadStr(keyname, csize, ccolor, &wide);
	m_pSpr->AlphaBlend(hbak, (m_kwidth - wide)/2, (m_kheight - csize)/2, wide, csize, himage, 0, 0);
	m_pSpr->CloseBlk(himage);
	m_pSpr->BitBlt(m_pLayer->m_frame, 
					m_left + col * m_kwidth,
					m_top + row * m_kheight,
					m_kwidth, m_kheight,
					hbak, 0, 0);
	m_pSpr->CloseBlk(hbak);
}

BOOL CKeyboard::DoResponse(DWORD x, DWORD y, DWORD statue)
{
	DWORD col, row;

	switch(statue)
	{
		case TOUCH_DOWN:
			col = x/m_kwidth;
			row = y/m_kheight;
			
			m_isPressed = TRUE;
			m_dwPressed = row * m_col + col;
			Show(m_dwPressed, STATUS_PRESSED);
			break;
		case TOUCH_VALID:
			if(m_isPressed)
			{
				col = x/m_kwidth;
				row = y/m_kheight;

 				if(m_dwPressed != row * m_col + col)
				{
					Show(m_dwPressed, STATUS_NORMAL);
					m_isPressed = FALSE;
				}
			}
			break;
		case TOUCH_MOVEOUT:
			if(m_isPressed)
			{
				Show(m_dwPressed, STATUS_NORMAL);
				m_isPressed = FALSE;
			}
			break;
		case TOUCH_UP:
			if(m_isPressed)
			{
				DPPostMessage(KBD_MESSAGE, m_keymap[m_dwPressed], 1, 0);
				Show(m_dwPressed, STATUS_NORMAL);
				m_isPressed = FALSE;
			}
			break;
	}
	return TRUE;
}

void CKeyboard::SetKeyMap(char* keymap)
{
	if(strlen(keymap) != strlen(m_keymap))
		return;

	memset(m_keymap, 0, 512);
	strcpy(m_keymap, keymap);
	for(int i = 0; i < m_col * m_row; i++)
		Show(i, STATUS_NORMAL);
}