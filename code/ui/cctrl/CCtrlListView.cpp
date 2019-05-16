#include "CCtrlListView.h"

#define	ALIGN_WIDTH	4
BOOL CItemView::InitBkFrame(ContentManage* pCm)
{
	char* pcontent;

	if((pcontent = pCm->FindContentByName("colornormal")) != NULL)
		m_hbakcolornormal = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("colorpressed")) != NULL)
		m_hbakcolorpressed = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("colorfocus")) != NULL)
		m_hbakcolorfocus = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("textcolor")) != NULL)
		m_textcolor = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("textcolorpressed")) != NULL)
		m_textcolorpressed = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("textcolorfocus")) != NULL)
		m_textcolorfocus = hexconvert(pcontent);
	return TRUE;
}

void CItemView::AddNewItem(ContentManage* pCm)
{
	char* pcontent;
	DWORD m_curptr;

	m_curptr = m_Count;
	m_Count++;
	m_pListItem[m_curptr].m_type = ITEM_TYPE_TEXT;
	m_pListItem[m_curptr].m_textsize = 22;

	if((pcontent = pCm->FindContentByName("type")) != NULL)
	{
		if(strcmp(pcontent, "png") == 0)
			m_pListItem[m_curptr].m_type = ITEM_TYPE_PNG;
		else if(strcmp(pcontent, "index") == 0)
			m_pListItem[m_curptr].m_type = ITEM_TYPE_INDEX;
		else
			m_pListItem[m_curptr].m_type = ITEM_TYPE_TEXT;
	}

	if((pcontent = pCm->FindContentByName("align")) != NULL)
	{
		if(strstr(pcontent, "center") != NULL)
			m_pListItem[m_curptr].m_align |= ALIGN_CENTER;
		else if(strstr(pcontent, "right") != NULL)
			m_pListItem[m_curptr].m_align |= ALIGN_RIGHT;
		else
			m_pListItem[m_curptr].m_align |= ALIGN_LEFT;
	}
	else
		m_pListItem[m_curptr].m_align |= ALIGN_LEFT;

	if((pcontent = pCm->FindContentByName("width")) == NULL)
		return;
	m_pListItem[m_curptr].m_width = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("title")) != NULL)
		m_pListItem[m_curptr].m_title = strtol(pcontent, NULL, 10);
	m_pListItem[m_curptr].m_left = m_width;
	m_pListItem[m_curptr].m_height = m_height;
	m_pListItem[m_curptr].m_top = 0;
	m_width += m_pListItem[m_curptr].m_width + m_intervel;

	if((pcontent = pCm->FindContentByName("left")) != NULL)
		m_pListItem[m_curptr].m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) != NULL)
		m_pListItem[m_curptr].m_top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("height")) != NULL)
		m_pListItem[m_curptr].m_height = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("textsize")) != NULL)
		m_pListItem[m_curptr].m_textsize = strtol(pcontent, NULL, 10);
	return;
}

BOOL CItemView::DoInit(char* fname)
{
	char temp[512];
	DWORD flen;
	char* pbuf;
	DWORD i;
	ContentManage pCm;
	char* pdesc;

	sprintf(temp, "%s/%s", LAYOUT_DIR, fname);

	flen = BReadFile(temp, &pbuf);
	if(flen == 0)
	{
		return FALSE;
	}
	flen/=2;

	for(i = 0; i < flen; i++)
	{
		if((pbuf[i] >= 0x41)
			&& (pbuf[i] <= 0x5a))
			pbuf[i] = pbuf[i] | 0x20;
	}

	if(CFindContent(pbuf, "frame", temp, FALSE) == NULL)
	{
		free(pbuf);
		return FALSE;
	}
	pCm.Init(temp);
	if(!InitBkFrame(&pCm))
	{
		free(pbuf);
		return FALSE;
	}

	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, "item", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		AddNewItem(&pCm);
	}
	m_width = 0;
	for(i = 0; i < m_Count; i++)
	{
		if((m_pListItem[i].m_left + m_pListItem[i].m_width) > m_width)
			m_width = m_pListItem[i].m_left + m_pListItem[i].m_width;
	}
	return TRUE;
}

HANDLE CItemView::GetView(DWORD index, DWORD state, SIZE* rsize)
{
	HANDLE hret;
	HANDLE hitem;
	DWORD i;
	DWORD width;
	DWORD height;
	DWORD dindex;
	DWORD cwidth;
	DWORD cheight;
	DWORD align;
	char wIndex[8];
	DWORD backcolor;
	BOOL m_bPng;

	hret = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->SetBK(hret, 0);
	if(state == STATUS_NORMAL)
		backcolor = m_hbakcolornormal;
	else if(state == STATUS_PRESSED)
		backcolor = m_hbakcolorpressed;
	else
		backcolor = m_hbakcolorfocus;

	m_pSpr->SetBK(hret, backcolor);

	for(i = 0; i < m_Count; i++)
	{
		dindex = index * m_Count + i;
		m_bPng = FALSE;
		if(m_pListItem[i].m_type == ITEM_TYPE_PNG)
		{
			SIZE size;
			hitem = m_pSpr->LoadImage(m_pParent->m_pAdapter[dindex], &size);
			width = size.cx;
			height = size.cy;
			m_bPng = TRUE;
		}
		else if(m_pListItem[i].m_type == ITEM_TYPE_INDEX)
		{
			sprintf(wIndex, "%d", index+1);
			DWORD color;
			if(state == STATUS_NORMAL)
				color = m_textcolor;
			else if(state == STATUS_PRESSED)
				color = m_textcolorpressed;
			else
				color = m_textcolorfocus;
			hitem = m_pSpr->LoadStr(wIndex, m_pListItem[i].m_textsize, color, &width);
			height = m_pListItem[i].m_textsize;
		}
		else if(m_pListItem[i].m_type == ITEM_TYPE_TEXT)
		{
			DWORD color;
			if(state == STATUS_NORMAL)
				color = m_textcolor;
			else if(state == STATUS_PRESSED)
				color = m_textcolorpressed;
			else
				color = m_textcolorfocus;
			hitem = m_pSpr->LoadStr(m_pParent->m_pAdapter[dindex], m_pListItem[i].m_textsize, color, &width);
			height = m_pListItem[i].m_textsize;
		}

		if(hitem == NULL)
			continue;
		if(width > (m_pListItem[i].m_width - ALIGN_WIDTH))
			cwidth = m_pListItem[i].m_width - ALIGN_WIDTH;
		else
			cwidth = width;

		if(height > (m_pListItem[i].m_height))
			cheight = m_pListItem[i].m_height;
		else
			cheight = height;

		align = m_pListItem[i].m_align & (ALIGN_RIGHT | ALIGN_CENTER | ALIGN_LEFT);

		if(align == ALIGN_LEFT)
		{
			if(m_bPng)
				m_pSpr->BitBlt(hret, 
								m_pListItem[i].m_left + ALIGN_WIDTH, 
								m_pListItem[i].m_top + (m_pListItem[i].m_height - cheight)/2,
								cwidth, cheight, hitem, 0, 0);
			else
				m_pSpr->AlphaBlend(hret, 
								m_pListItem[i].m_left + ALIGN_WIDTH, 
								m_pListItem[i].m_top + (m_pListItem[i].m_height - cheight)/2,
								cwidth, cheight, hitem, 0, 0);
		}
		else if(m_pListItem[i].m_align == ALIGN_RIGHT)
		{
			if(m_bPng)
				m_pSpr->BitBlt(hret, 
								m_pListItem[i].m_left + m_pListItem[i].m_width - cwidth - ALIGN_WIDTH, 
								m_pListItem[i].m_top + (m_pListItem[i].m_height - cheight)/2,
								cwidth, cheight, hitem, width - cwidth, 0);
			else
				m_pSpr->AlphaBlend(hret, 
								m_pListItem[i].m_left + m_pListItem[i].m_width - cwidth - ALIGN_WIDTH, 
								m_pListItem[i].m_top + (m_pListItem[i].m_height - cheight)/2,
								cwidth, cheight, hitem, width - cwidth, 0);
		}
		else
		{
			if(m_bPng)
				m_pSpr->BitBlt(hret,
								m_pListItem[i].m_left + (m_pListItem[i].m_width - cwidth)/2,
								m_pListItem[i].m_top + (m_pListItem[i].m_height - cheight)/2,
								cwidth, cheight, hitem, (width - cwidth)/2, 0);
			else
				m_pSpr->AlphaBlend(hret,
								m_pListItem[i].m_left + (m_pListItem[i].m_width - cwidth)/2,
								m_pListItem[i].m_top + (m_pListItem[i].m_height - cheight)/2,
								cwidth, cheight, hitem, (width - cwidth)/2, 0);
		}
		m_pSpr->CloseBlk(hitem);
	}

	rsize->cx = m_width;
	rsize->cy = m_height;
	return hret;
}

CDPListView::~CDPListView()
{
	if(m_hFrameBak != NULL)
	{
		m_pSpr->CloseBlk(m_hFrameBak);
		m_hFrameBak = NULL;
	}
	if(m_pItemView != NULL)
	{
		delete m_pItemView;
		m_pItemView = NULL;
	}
	if(m_pPageView != NULL)
	{
		delete m_pPageView;
		m_pPageView = NULL;
	}
}

BOOL CDPListView::DoInit(ContentManage* pCm)
{
	char* pcontent;
	DWORD j;
	DWORD left, width;

	if((pcontent = pCm->FindContentByName("row")) == NULL)
		return FALSE;
	m_row = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("rowheight")) == NULL)
		return FALSE;
	m_rowheight = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("edgewidth")) != NULL)
		m_interval = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("itemview")) == NULL)
		return FALSE;
	m_pItemView = new CItemView(this, m_rowheight, m_interval);
	if(!m_pItemView->DoInit(pcontent))
		return FALSE;

	m_width = m_pItemView->GetWidth() + 2 * m_interval;
	m_height = m_row * m_rowheight;
	m_left = (m_pLayer->m_width - m_width)/2;
	m_top = (m_pLayer->m_height - m_height)/2;
	if((pcontent = pCm->FindContentByName("left")) != NULL)
		m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) != NULL)
		m_top = strtol(pcontent, NULL, 10);

	m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

	if((pcontent = pCm->FindContentByName("titlesize")) != NULL)
		m_titlesize = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("titlecolor")) != NULL)
		m_titlecolor = hexconvert(pcontent);

	m_titleabove = m_interval;
	if((pcontent = pCm->FindContentByName("titleabove")) != NULL)
		m_titleabove = strtol(pcontent, NULL, 10);

	m_PageRight = m_left + m_width;
	m_PageTop = m_top - m_interval;		

	if((pcontent = pCm->FindContentByName("pagetext")) != NULL)
	{
		m_PageTxt = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("pageright")) != NULL)
			m_PageRight = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("pagebottom")) != NULL)
			m_PageTop = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("pagecolor")) != NULL)
			m_PageColor = hexconvert(pcontent);
		if((pcontent = pCm->FindContentByName("pagetextsize")) != NULL)
			m_PageTextSize = strtol(pcontent, NULL, 10);
		m_pPageView = new CDPStatic(m_pLayer);
		m_pPageView->SetTextColor(m_PageColor);
		m_pPageView->SetTextSize(m_PageTextSize);
		m_pPageView->SetStart(m_PageRight, m_PageTop);
		m_pPageView->SetAlign(ALIGN_RIGHT | ALIGN_BOTTOM);
	}

	for(j = 0; j < m_pItemView->GetCount(); j++)
	{
		DWORD wide;
		HANDLE hlist;
		left = m_pItemView->GetLeft(j) + m_interval;
		width = m_pItemView->GetWidth(j) + m_interval * 2;
		hlist = m_pSpr->LoadStr(GetStringByID(m_pItemView->GetTitle(j)), m_titlesize, m_titlecolor, &wide);
		if(hlist != NULL)
		{
			m_pSpr->AlphaBlend(m_pLayer->m_frame, m_left + left + (width - wide)/2, m_top - m_titlesize - m_titleabove,
				wide, m_titlesize, hlist, 0, 0);
			m_pSpr->CloseBlk(hlist);
		}
	}

	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	m_msgid = m_pLayer->RequestMsgId();
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_LISTVIEW, "listview", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_LISTVIEW, pcontent, this, m_msgid);
	return TRUE;
}

BOOL CDPListView::DoResponse(DWORD x, DWORD y, DWORD statue)
{
	DWORD rindex;
	rindex = y/m_rowheight;
	if(rindex >= m_row)
		rindex = m_row - 1;

	switch(statue)
	{
		case TOUCH_DOWN:
			if((rindex + m_pagebegin) < m_count)
			{
				m_bIsPressed = TRUE;
				m_lastRow = rindex;
				Show(m_lastRow, STATUS_PRESSED);
			}
			break;
		case TOUCH_VALID:
			if(m_bIsPressed)
			{
				if(m_lastRow != rindex)
				{
					if((m_lastRow + m_pagebegin) == m_curptr)
						Show(m_lastRow, STATUS_FOCUS);
					else
						Show(m_lastRow, STATUS_NORMAL);
					m_bIsPressed = FALSE;
				}
			}
			break;
		case TOUCH_MOVEOUT:
			if(m_bIsPressed)
			{
				if((m_lastRow + m_pagebegin) == m_curptr)
					Show(m_lastRow, STATUS_FOCUS);
				else
					Show(m_lastRow, STATUS_NORMAL);
				m_bIsPressed = FALSE;
			}
			break;
		case TOUCH_UP:
			if(m_bIsPressed)
			{
				if((m_curptr >= m_pagebegin)
					&& (m_curptr < (m_pagebegin + m_row)))
				{
					Show(m_curptr - m_pagebegin, STATUS_NORMAL);
				}
				Show(m_lastRow, STATUS_FOCUS);
				m_curptr = m_pagebegin + m_lastRow;
				DPPostMessage(TOUCH_MESSAGE, m_msgid, m_curptr, 0);
				m_bIsPressed = FALSE;
			}
			break;
	}
	return TRUE;
}

void CDPListView::SetDataArray(DWORD count, char** parry)
{
	m_pAdapter = parry;
	m_count = count;
}

void CDPListView::Show(DWORD row, DWORD statue)
{
	HANDLE hbk;
	HANDLE hitem;
	SIZE size;

	row %= m_row;
	hbk = m_pSpr->ReqTempBlk(m_width, m_rowheight);
	m_pSpr->BitBlt(hbk, 0, 0, m_width, m_rowheight, m_hFrameBak, 0, row * m_rowheight);
	if(statue == STATUS_PRESSED)
		hitem = m_pItemView->GetView(row + m_pagebegin, STATUS_PRESSED, &size);
	else if(statue == STATUS_NORMAL)
		hitem = m_pItemView->GetView(row + m_pagebegin, STATUS_NORMAL, &size);
	else
		hitem = m_pItemView->GetView(row + m_pagebegin, STATUS_FOCUS, &size);
	m_pSpr->AlphaBlend(hbk, (m_width - size.cx)/2, (m_rowheight - size.cy)/2,
			size.cx, size.cy, hitem, 0, 0);
	m_pSpr->CloseBlk(hitem);
	
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top + row * m_rowheight, m_width, m_rowheight, hbk, 0, 0);
	m_pSpr->CloseBlk(hbk);
}

void CDPListView::Show(void)
{
	DWORD i;
	HANDLE hbk;
	HANDLE hitem;
	SIZE size;
	
	hbk = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(hbk, 0, 0, m_width, m_height, m_hFrameBak, 0, 0);
	if(m_count != 0)
	{
		if(m_curptr >= m_count)
			m_curptr = m_count -1;
		if(m_pagebegin >= m_count)
			m_pagebegin -= m_row;
	}
	else
	{
		m_curptr = 0;
		m_pagebegin = 0;
	}
	for(i = 0; (i < m_row) && ((m_pagebegin + i) < m_count); i++)
	{
		if((m_pagebegin + i) == m_curptr)
			hitem = m_pItemView->GetView(m_pagebegin + i, STATUS_FOCUS, &size);
		else
			hitem = m_pItemView->GetView(m_pagebegin + i, STATUS_NORMAL, &size);
		m_pSpr->BitBlt(hbk, (m_width - size.cx)/2, i * m_rowheight + (m_rowheight - size.cy)/2,
				size.cx, size.cy, hitem, 0, 0);
		m_pSpr->CloseBlk(hitem);
	}
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, hbk, 0, 0);
	m_pSpr->CloseBlk(hbk);

	if(m_pPageView != NULL)
	{
		char buf[32];
		if(m_count == 0)
			sprintf(buf, "0/0%s", GetStringByID(m_PageTxt));
		else			
			sprintf(buf, "%d/%d%s", m_pagebegin/m_row + 1, (m_count + m_row - 1)/m_row, GetStringByID(m_PageTxt));
		m_pPageView->SetSrc(buf);
		m_pPageView->Show(TRUE);
	}
}

BOOL CDPListView::NextPage(void)
{
	if((m_pagebegin + m_row) < m_count)
	{
		m_pagebegin += m_row;
		m_curptr = m_pagebegin;
		Show();
		return TRUE;
	}
	return FALSE;
}

BOOL CDPListView::PrevPage(void)
{
	if(m_pagebegin > 0)
	{
		if(m_pagebegin >= m_row)
			m_pagebegin -= m_row;
		else
			m_pagebegin = 0;
		m_curptr = m_pagebegin;
		Show();
		return TRUE;
	}
	return FALSE;
}
BOOL CDPListView::IsHeadPage()
{
	if(m_pagebegin <= 0)
		return TRUE;
	else
		return FALSE;
}
BOOL CDPListView::IsLastPage()
{
	if((m_pagebegin + m_row) >= m_count)
		return TRUE;
	else 
		return FALSE;
}
DWORD CDPListView::GetCurPtr(void)
{
	return m_curptr;
}

void CDPListView::SetCurPtr(DWORD cur)
{
	m_curptr = cur;
	m_pagebegin = (m_curptr/m_row)*m_row;
}

DWORD CDPListView::GetCurPagePtr(void)
{
	return m_curptr % m_row;
}