#include "CCtrlMTable.h"

BOOL CMTable::DoInit(ContentManage* pCm)
{
	char* pcontent;
	DWORD i, j;

	if((pcontent = pCm->FindContentByName("bakpng")) != NULL)
	{
		SIZE sz;
		m_hBakPng = m_pSpr->LoadImage( pcontent, &sz );
	}

	if((pcontent = pCm->FindContentByName("tabtype")) != NULL)
		strcpy( m_tabType, pcontent );

	if((pcontent = pCm->FindContentByName("col")) == NULL)
		return FALSE;
	m_col = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("colwidth")) == NULL)
		return FALSE;
	m_colwidth = strtol(pcontent, NULL, 10);

	m_titlewidth = m_colwidth;
	if((pcontent = pCm->FindContentByName("titlewidth")) != NULL)
		m_titlewidth = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("row")) == NULL)
		return FALSE;
	m_row = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("rowheight")) == NULL)
		return FALSE;
	m_rowheight = strtol(pcontent, NULL, 10);

	m_titleheight = m_rowheight;
	if((pcontent = pCm->FindContentByName("titleheight")) != NULL)
		m_titleheight = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("colornorma")) != NULL)
		m_color[STATUS_NORMAL] = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("colorpressed")) != NULL)
		m_color[STATUS_PRESSED] = hexconvert(pcontent);

	m_color[STATUS_FOCUS] = m_color[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("colorfocus")) != NULL)
		m_color[STATUS_FOCUS] = hexconvert(pcontent);

	m_color[STATUS_UNACK] = m_color[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("colorunack")) != NULL)
		m_color[STATUS_UNACK] = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("textcolor")) != NULL)
		m_textcolor[STATUS_NORMAL] = hexconvert(pcontent);

	m_textcolor[STATUS_PRESSED] = m_textcolor[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("textcolorpressed")) != NULL)
		m_textcolor[STATUS_PRESSED] = hexconvert(pcontent);

	m_textcolor[STATUS_FOCUS] = m_textcolor[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("textcolorfocus")) != NULL)
		m_textcolor[STATUS_FOCUS] = hexconvert(pcontent);

	m_textcolor[STATUS_UNACK] = m_textcolor[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("textcolorunack")) != NULL)
		m_textcolor[STATUS_UNACK] = hexconvert(pcontent);

	m_titlecolor = m_textcolor[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("titlecolor")) != NULL)
		m_titlecolor = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("edgewidth")) != NULL)
		m_edgeheight = m_edgewidth = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("edgeheight")) != NULL)
		m_edgeheight = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("edgecolor")) != NULL)
		m_edgecolor = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("textsize")) != NULL)
		m_textsize = strtol(pcontent, NULL, 10);
	else
		m_textsize = (m_rowheight * 3) >> 2;

	m_titlesize = m_textsize;
	if((pcontent = pCm->FindContentByName("titlesize")) != NULL)
		m_titlesize = strtol(pcontent, NULL, 10);

	m_width = (m_col - 1) * (m_colwidth + m_edgewidth) + m_edgewidth + (m_titlewidth + m_edgewidth);
	m_height = (m_row - 1) * (m_rowheight + m_edgeheight) + m_edgeheight + (m_titleheight + m_edgeheight);
	m_left = (m_pLayer->m_width - m_width)/2;
	m_top = (m_pLayer->m_height - m_height)/2;

	if((pcontent = pCm->FindContentByName("left")) != NULL)
		m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) != NULL)
		m_top = strtol(pcontent, NULL, 10);

	m_tablebk = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(m_tablebk, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

	if((pcontent = pCm->FindContentByName("buttonleft")) != NULL)
		m_buttommap[BUTTON_LEFT] = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("buttonup")) != NULL)
		m_buttommap[BUTTON_UP] = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("buttonrignt")) != NULL)
		m_buttommap[BUTTON_RIGHT] = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("buttonup")) != NULL)
		m_buttommap[BUTTON_DOWN] = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("buttonprev")) != NULL)
		m_buttommap[BUTTON_PREV] = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("buttonnext")) != NULL)
		m_buttommap[BUTTON_NEXT] = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("buttonenter")) != NULL)
		m_buttommap[BUTTON_ENTER] = strtol(pcontent, NULL, 10);

	// 每一列的起点
	x_off[0] = 0;
	x_off[1] = m_edgewidth + m_titlewidth;
	for(i = 2; i < m_col + 1; i++)
		x_off[i] = x_off[i - 1] + (m_colwidth + m_edgewidth);

	// 每一行的起点
	y_off[0] = 0;
	y_off[1] = m_edgeheight + m_titleheight;
	for(i = 2; i < m_row + 1; i++)
		y_off[i] = y_off[i - 1] + (m_rowheight + m_edgeheight);

	// 画竖线
	/*for(i = 0; i < m_col + 1; i++)
	{
		rect.left = x_off[i];
		rect.right = x_off[i] + m_edgewidth;
		rect.top = y_off[0];
		rect.bottom = y_off[m_row] + m_edgewidth;
		m_pSpr->SetBK(m_tablebk, rect, m_edgecolor);
	}

	// 画横线
	for(i = 0; i < m_row + 1; i++)
	{
		rect.left = x_off[0];
		rect.right = x_off[m_col] + m_edgewidth;
		rect.top = y_off[i];
		rect.bottom = y_off[i] + m_edgewidth;
		m_pSpr->SetBK(m_tablebk, rect, m_edgecolor);
	}*/

	// 生成所有表格的位置
	m_tablist = (RECT*)malloc(sizeof(RECT) * m_col * m_row);
	m_btabresp = (BOOL*)malloc(sizeof(BOOL) * m_col * m_row);

	for(i = 0; i < m_row; i++)
	{
		for(j = 0; j < m_col; j++)
		{
			m_tablist[i * m_col + j].left = x_off[j] + m_edgewidth;
			m_tablist[i * m_col + j].right = x_off[j + 1];
			m_tablist[i * m_col + j].top = y_off[i] + m_edgeheight;
			m_tablist[i * m_col + j].bottom = y_off[i + 1];
			m_btabresp[i * m_col + j] = FALSE;
		}
	}

	// 生成响应触摸的表格位置,第一行和第一列不需响应
	for(i = 0; i < m_row; i++)
	{
		for(j = 0; j < m_col; j++)
			m_btabresp[i * m_col + j] = TRUE;
	}

	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	m_msgid = m_pLayer->RequestMsgId();
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_MTABLE, "table", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_MTABLE, pcontent, this, m_msgid);
	for(i = 0; i < BUTTON_MAX; i++)
	{
		if(m_buttommap[i] != 0)
		{
			m_pLayer->RegisterKeyEvent(m_buttommap[i], this);
		}
	}
	m_LastIndex = m_col + 1;	// 为第一行第一列
	m_maxkey = m_col * m_row;
	return TRUE;
}

BOOL CMTable::DoResponse(DWORD x, DWORD y, DWORD statue)
{
	DWORD col, row;
	DWORD index;

	for(col = 0; col < m_col; col++)
	{
		if((x >= x_off[col])
			&& (x < x_off[col + 1]))
			break;
	}
	if(col == m_col)
		col = m_col - 1;
	for(row = 0; row < m_row; row++)
	{
		if((y >= y_off[row])
			&& (y < y_off[row + 1]))
			break;
	}
	if(row == m_row)
		row = m_row - 1;
	index = row * m_col + col;

	switch(statue)
	{
		case TOUCH_DOWN:
			if(m_btabresp[index])
			{
				m_LastIndex = index;
				m_bIsPressed = TRUE;
				//Show(col, row, STATUS_PRESSED);
			}
			break;
		case TOUCH_VALID:
			if(m_bIsPressed)
			{
				if(m_LastIndex != index)
				{
					m_bIsPressed = FALSE;
					//Show(m_LastIndex%m_col, m_LastIndex/m_col, STATUS_NORMAL);
				}
			}
			break;
		case TOUCH_MOVEOUT:
			if(m_bIsPressed)
			{
				m_bIsPressed = FALSE;
				Show(col, row, STATUS_NORMAL);
			}
			break;
		case TOUCH_UP:
			if(m_bIsPressed)
			{
				m_bIsPressed = FALSE;
				//Show(col, row, STATUS_NORMAL);
				DPPostMessage(TOUCH_MESSAGE, m_msgid, col, row);
			}
			break;
	}
	return TRUE;
}

void CMTable::DoKeyResponse(DWORD rawkey, DWORD statue)
{
	DWORD newfocus;
	if(statue == KBD_UP)
	{
		if(m_bAllDis)
			return;
		if(rawkey == m_buttommap[BUTTON_PREV])
		{
			newfocus = m_LastIndex;
			while(1)
			{
				newfocus = (newfocus + m_maxkey - 1)%m_maxkey;
				if(m_btabresp[newfocus] == TRUE)
					break;
			}
			Show(newfocus%m_col, newfocus/m_col, STATUS_FOCUS);
			Show(m_LastIndex%m_col, m_LastIndex/m_col, STATUS_NORMAL);
			m_LastIndex = newfocus;
		}
		else if(rawkey == m_buttommap[BUTTON_NEXT])
		{
			newfocus = m_LastIndex;
			while(1)
			{
				newfocus = (newfocus + 1)%m_maxkey;
				if(m_btabresp[newfocus] == TRUE)
					break;
			}
			Show(newfocus%m_col, newfocus/m_col, STATUS_FOCUS);
			Show(m_LastIndex%m_col, m_LastIndex/m_col, STATUS_NORMAL);
			m_LastIndex = newfocus;
		}
		else if(rawkey == m_buttommap[BUTTON_ENTER])
		{
			DPPostMessage(TOUCH_MESSAGE, m_msgid, m_LastIndex%m_col, m_LastIndex/m_col);
		}
	}
}

void CMTable::Show(DWORD col, DWORD row, DWORD statue)
{
	HANDLE hbk;
	HANDLE hstr;
	DWORD index;
	DWORD wide;

	index = row * m_col + col;
	
	hbk = m_pSpr->ReqTempBlk(m_colwidth, m_rowheight);
	hstr = m_pSpr->ReqTempBlk(m_colwidth, m_rowheight);
	m_pSpr->BitBlt(hbk, 0, 0, m_colwidth, m_rowheight, 
				m_tablebk, m_tablist[index].left, m_tablist[index].top);
	m_pSpr->SetBK(hstr, m_color[statue]);
	m_pSpr->AlphaBlend(hbk, 0, 0, m_colwidth, m_rowheight, hstr, 0, 0);
	m_pSpr->CloseBlk(hstr);

	if( m_hBakPng )
		m_pSpr->BitBlt( hbk, 0, 0, m_colwidth, m_rowheight, m_hBakPng, 0, 0 );

	hstr = m_pSpr->LoadStr(m_DataArray[index], m_textsize, m_textcolor[statue], &wide);	
	m_pSpr->AlphaBlend(hbk, (m_colwidth - wide)/2, (m_rowheight - m_textsize)/2, wide, m_textsize,
				hstr, 0, 0);
	m_pSpr->CloseBlk(hstr);
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left + m_tablist[index].left, m_top + m_tablist[index].top, m_colwidth, m_rowheight,
			hbk, 0, 0);
	m_pSpr->CloseBlk(hbk);
}

void CMTable::SetRespose(DWORD col, DWORD row, BOOL isres)
{
	BOOL last;
	DWORD index; 

	index = row * m_col + col;
	last = m_btabresp[index];
	if(last != isres)
	{
		m_btabresp[col + row * m_col] = isres;
		if(m_DataArray != NULL)
		{
			if(isres)
				Show(col, row, STATUS_NORMAL);
			else
				Show(col, row, STATUS_UNACK);
		}
	}
}

void CMTable::SetDataArray(char** parry)
{
	DWORD i, j;
	HANDLE hstr;
	DWORD wide;
	DWORD index;
	HANDLE hbak;
	DWORD width;
	DWORD height;

	m_DataArray = parry;

	hbak = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(hbak, 0, 0, m_width, m_height, m_tablebk, 0, 0);
	for(i = 0; i < m_row; i++)
	{
		for(j = 0; j < m_col; j++)
		{
			index = i * m_col +j;
			if(m_btabresp[index])
				hstr = m_pSpr->LoadStr(m_DataArray[index], m_textsize, m_textcolor[STATUS_NORMAL], &wide);
			else
				hstr = m_pSpr->LoadStr(m_DataArray[index], m_textsize, m_textcolor[STATUS_UNACK], &wide);
			
			//==============================================================================================
			if(hstr == NULL)
				goto forend;

			width = m_tablist[index].right - m_tablist[index].left;
			height = m_tablist[index].bottom - m_tablist[index].top;

			RECT Rectbak;
			Rectbak.left = m_tablist[index].left;
			Rectbak.right = m_tablist[index].right;
			Rectbak.top = m_tablist[index].top;
			Rectbak.bottom = m_tablist[index].bottom;

			if(m_hBakPng)
			{
				m_pSpr->BitBlt(hbak, m_tablist[index].left, m_tablist[index].top, m_tablist[index].right - m_tablist[index].left, \
					m_tablist[index].bottom - m_tablist[index].top, m_hBakPng, 0, 0);
			}

			m_pSpr->AlphaBlend(hbak, m_tablist[index].left + (width - wide)/2, m_tablist[index].top + (height - m_textsize)/2,
				wide, m_textsize, hstr, 0, 0);
			m_pSpr->CloseBlk(hstr);
		}
	}

forend:
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, hbak, 0, 0);
	m_pSpr->CloseBlk(hbak);

	for(i = 0; i < m_maxkey; i++)
	{
		if(m_btabresp[i] == TRUE)
			break;
	}
	if(i == m_maxkey)
		m_bAllDis = TRUE;
	else
	{
		m_bAllDis = FALSE;
		m_LastIndex = i;
		//Show(m_LastIndex%m_col, m_LastIndex/m_col, STATUS_FOCUS);
	}
}

