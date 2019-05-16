#include "CCtrlTable.h"

BOOL CTable::DoInit(ContentManage* pCm)
{
	char* pcontent;
	DWORD i, j;
	RECT rect;

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

	m_titlecolortop = m_textcolor[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("titlecolortop")) != NULL)
		m_titlecolortop = hexconvert(pcontent);
	m_titlecolorleft = m_textcolor[STATUS_NORMAL];
	if((pcontent = pCm->FindContentByName("titlecolorleft")) != NULL)
		m_titlecolorleft = hexconvert(pcontent);

	m_bHideTitle = FALSE;
	if((pcontent = pCm->FindContentByName("hidetitle")) != NULL)
	{
		if(strcmp("true" , pcontent) == 0)
			m_bHideTitle = TRUE;
	}

	if((pcontent = pCm->FindContentByName("edgewidth")) != NULL)
		m_edgewidth = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("edgecolor")) != NULL)
		m_edgecolor = hexconvert(pcontent);

	m_bHideGrid = FALSE;
	if((pcontent = pCm->FindContentByName("hidegrid")) != NULL)
	{
		if(strcmp(pcontent , "true") == 0)
			m_bHideGrid = TRUE;
	}

	if((pcontent = pCm->FindContentByName("textsize")) != NULL)
		m_textsize = strtol(pcontent, NULL, 10);
	else
		m_textsize = (m_rowheight * 3) >> 2;

	m_titlesize = m_textsize;
	if((pcontent = pCm->FindContentByName("titlesize")) != NULL)
		m_titlesize = strtol(pcontent, NULL, 10);

	m_width = (m_col - 1) * (m_colwidth + m_edgewidth) + m_edgewidth + (m_titlewidth + m_edgewidth);
	m_height = (m_row - 1) * (m_rowheight + m_edgewidth) + m_edgewidth + (m_titleheight + m_edgewidth);
	m_left = (m_pLayer->m_width - m_width)/2;
	m_top = (m_pLayer->m_height - m_height)/2;

	if((pcontent = pCm->FindContentByName("left")) != NULL)
		m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) != NULL)
		m_top = strtol(pcontent, NULL, 10);

	m_tablebk = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(m_tablebk, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

	// 每一列的起点
	x_off[0] = 0;
	x_off[1] = m_edgewidth + m_titlewidth;
	for(i = 2; i < m_col + 1; i++)
		x_off[i] = x_off[i - 1] + (m_colwidth + m_edgewidth);

	// 每一行的起点
	y_off[0] = 0;
	y_off[1] = m_edgewidth + m_titleheight;
	for(i = 2; i < m_row + 1; i++)
		y_off[i] = y_off[i - 1] + (m_rowheight + m_edgewidth);

	if (m_edgewidth)
	{
		// 画竖线
		for(i = 0; i < m_col + 1; i++)
		{
			rect.left = x_off[i];
			rect.right = x_off[i] + m_edgewidth;
			rect.top = y_off[0];
			rect.bottom = y_off[m_row] + m_edgewidth;
			m_pSpr->FillSolidRect(m_tablebk, &rect, m_edgecolor);
		}
		// 画横线
		for(i = 0; i < m_row + 1; i++)
		{
			rect.left = x_off[0];
			rect.right = x_off[m_col] + m_edgewidth;
			rect.top = y_off[i];
			rect.bottom = y_off[i] + m_edgewidth;
			m_pSpr->FillSolidRect(m_tablebk, &rect, m_edgecolor);
		}
	}

	// 生成所有表格的位置
	m_tablist = (RECT*)malloc(sizeof(RECT) * m_col * m_row);
	m_btabresp = (BOOL*)malloc(sizeof(BOOL) * m_col * m_row);

	for(i = 0; i < m_row; i++)
	{
		for(j = 0; j < m_col; j++)
		{
			m_tablist[i * m_col + j].left = x_off[j] + m_edgewidth;
			m_tablist[i * m_col + j].right = x_off[j + 1];
			m_tablist[i * m_col + j].top = y_off[i] + m_edgewidth;
			m_tablist[i * m_col + j].bottom = y_off[i + 1];
			m_btabresp[i * m_col + j] = FALSE;

			{
				if(i == 0 || j == 0)
				{
					// 标题和第一列不用花背景色
					continue;
				}
				//画背景
				rect.left = x_off[j] + m_edgewidth;
				rect.right = x_off[j + 1];
				rect.top = y_off[i] + m_edgewidth;
				rect.bottom = y_off[i + 1];
				m_pSpr->FillSolidRect(m_tablebk, &rect, m_color[0]);
			}
		}
	}

	// 生成响应触摸的表格位置,第一行和第一列不需响应
	int start = -1;
	if(m_bHideTitle == TRUE)
		start = 0;
	else
		start = 1;
	for(i = start; i < m_row; i++)
	{
		for(j = start; j < m_col; j++)
			m_btabresp[i * m_col + j] = TRUE;

		// 如果只有一行
		if(m_col == 1)
			m_btabresp[i] = TRUE;
	}

	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	m_msgid = m_pLayer->RequestMsgId();
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_TABLE, "table", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_TABLE, pcontent, this, m_msgid);
	m_LastIndex = m_col + 1;	// 为第一行第一列
	m_maxkey = m_col * m_row;
	return TRUE;
}
void CTable::FocusOnFirst()
{
	if(m_bHideTitle)
	{
		m_curFocusIndex[0] = 0;
		m_curFocusIndex[1] = 0;
	}
	else
	{
		m_curFocusIndex[0] = 1;
		m_curFocusIndex[1] = 1;
	}
	
	Show(m_curFocusIndex[0] , m_curFocusIndex[1] , STATUS_PRESSED);
}
void CTable::FocusNext(BOOL rowBased)
{
	if(m_curFocusIndex[0] == -1 || m_curFocusIndex[1] == -1)
	{
		FocusOnFirst();
		return ;
	}
	if(rowBased == FALSE)
	{
		Show(m_curFocusIndex[0] , m_curFocusIndex[1] , STATUS_NORMAL);
		m_curFocusIndex[1]++;
		m_curFocusIndex[1] %= m_row;
		if(!m_bHideTitle)
			m_curFocusIndex[1] = m_curFocusIndex[1] ? m_curFocusIndex[1] : 1;

		int i = 0 ;
		for(; i < m_row ; i++)//查找启动响应的表格项
		{
			if(m_btabresp[m_curFocusIndex[0] + m_curFocusIndex[1] * m_col] == TRUE)
				break;
			else
			{
				m_curFocusIndex[1]++;
				m_curFocusIndex[1] %= m_row;
				if(!m_bHideTitle)
					m_curFocusIndex[1] = m_curFocusIndex[1] ? m_curFocusIndex[1] : 1;
			}
		}
		if(i <= m_row) //找到，做响应
			Show(m_curFocusIndex[0] , m_curFocusIndex[1] , STATUS_PRESSED);
	}
	else
	{
		Show(m_curFocusIndex[0] , m_curFocusIndex[1] , STATUS_NORMAL);
		m_curFocusIndex[0]++;
		m_curFocusIndex[0] %= m_col;
		if(!m_bHideTitle)
			m_curFocusIndex[0] = m_curFocusIndex[0] ? m_curFocusIndex[0] : 1;
		int i = 0 ;
		for(; i < m_col ; i++)//查找启动响应的表格项
		{
			if(m_btabresp[m_curFocusIndex[0] + m_curFocusIndex[1] * m_col] == TRUE)
				break;
			else
			{
				m_curFocusIndex[0]++;
				m_curFocusIndex[0] %= m_row;
				if(!m_bHideTitle)
					m_curFocusIndex[0] = m_curFocusIndex[0] ? m_curFocusIndex[0] : 1;
			}
		}
		if(i <= m_row) //找到，做响应
			Show(m_curFocusIndex[0] , m_curFocusIndex[1] , STATUS_PRESSED);
	}
}
void CTable::StrikeOut()
{
	if(m_curFocusIndex[0] >= 0 || m_curFocusIndex[1] >= 0)
		DPPostMessage(TOUCH_MESSAGE, m_msgid, m_curFocusIndex[0] , m_curFocusIndex[1]);
}

BOOL CTable::DoResponse(DWORD x, DWORD y, DWORD statue)
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
				Show(col, row, STATUS_PRESSED);
			}
			break;
		case TOUCH_VALID:
			if(m_bIsPressed)
			{
				if(m_LastIndex != index)
				{
					m_bIsPressed = FALSE;
					Show(m_LastIndex%m_col, m_LastIndex/m_col, STATUS_NORMAL);
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
				Show(col, row, STATUS_NORMAL);
				DPPostMessage(TOUCH_MESSAGE, m_msgid, col, row);
			}
			break;
	}
	return TRUE;
}

void CTable::Show(DWORD col, DWORD row, DWORD statue)
{
	HANDLE hbk;
	HANDLE hstr;
	DWORD index;
	DWORD wide;

	index = row * m_col + col;
	
	hbk = m_pSpr->ReqTempBlk(m_colwidth, m_rowheight);
	hstr = m_pSpr->ReqTempBlk(m_colwidth, m_rowheight);
	if(hbk == NULL)
		printf("[error]ReqTempBlk fail (hbk), width = %d , height = %d\n", m_colwidth , m_rowheight);
	else if(hstr == NULL)
		printf("[error]ReqTempBlk fail (hstr), width = %d , height = %d\n", m_colwidth , m_rowheight);

	m_pSpr->BitBlt(hbk, 0, 0, m_colwidth, m_rowheight, 
				m_tablebk, m_tablist[index].left, m_tablist[index].top);
	m_pSpr->SetBK(hstr, m_color[statue]);
	m_pSpr->AlphaBlend(hbk, 0, 0, m_colwidth, m_rowheight, hstr, 0, 0);
	m_pSpr->CloseBlk(hstr);

	hstr = m_pSpr->LoadStr(m_DataArray[index], m_textsize, m_textcolor[statue], &wide);
	if(hstr != NULL)
		m_pSpr->AlphaBlend(hbk, (m_colwidth - wide)/2, (m_rowheight - m_textsize)/2, wide, m_textsize,
				hstr, 0, 0);

	m_pSpr->CloseBlk(hstr);
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left + m_tablist[index].left, m_top + m_tablist[index].top, m_colwidth, m_rowheight,
			hbk, 0, 0);
	m_pSpr->CloseBlk(hbk);
}

void CTable::SetRespose(DWORD col, DWORD row, BOOL isres)
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

void CTable::SetDataArray(char** parry)
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
			index = i * m_col + j;
			if(((i == 0)) && (m_bHideTitle == FALSE))
				hstr = m_pSpr->LoadStr(m_DataArray[index], m_titlesize, m_titlecolortop, &wide);
			else if((/*(i == 0) || */(j == 0)) && (m_bHideTitle == FALSE))
				hstr = m_pSpr->LoadStr(m_DataArray[index], m_titlesize, m_titlecolorleft, &wide);
			else
			{
				if(m_btabresp[index])
					hstr = m_pSpr->LoadStr(m_DataArray[index], m_textsize, m_textcolor[STATUS_NORMAL], &wide);
				else
					hstr = m_pSpr->LoadStr(m_DataArray[index], m_textsize, m_textcolor[STATUS_UNACK], &wide);
			}
			width = m_tablist[index].right - m_tablist[index].left;
			height = m_tablist[index].bottom - m_tablist[index].top;
			if(hstr)
				m_pSpr->AlphaBlend(hbak, m_tablist[index].left + (width - wide)/2, m_tablist[index].top + (height - m_textsize)/2,
					wide, m_textsize, hstr, 0, 0);
			m_pSpr->CloseBlk(hstr);
		}
	}
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
		Show(m_LastIndex%m_col, m_LastIndex/m_col, STATUS_FOCUS);
	}
}
