#include "CCtrlText.h"

BOOL CText::DoInit(ContentManage* pCm)
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

	if((pcontent = pCm->FindContentByName("lineheight")) == NULL)
		return FALSE;
	m_lineheight = strtol(pcontent, NULL, 10);
	
	if((pcontent = pCm->FindContentByName("textsize")) != NULL)
		m_textsize = strtol(pcontent, NULL, 10);
		
	if((pcontent = pCm->FindContentByName("textcolor")) != NULL)
		m_textcolor = hexconvert(pcontent);
		
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		return FALSE;
	m_pLayer->RegisterCtrl(CTRL_TEXT, pcontent, this, m_msgid);
	return TRUE;
}

void CText::Show(BOOL isShow)
{
	DWORD left, top, width;
	HANDLE hbak;
	HANDLE hpng;
	char text[2];
	text[1] = 0;

	if(isShow)
	{
		if(strlen(m_pcur) == 0)	// 无资源，不需显示
			return;
		hbak = m_pSpr->DupBlock(m_hFrameBak);
		left = 0;
		top = startline;
		while(*m_pcur != 0)
		{
			if(*m_pcur == 0x0d)
			{
				m_pcur++;
				if(*m_pcur == 0x0a)
					m_pcur++;
				top += m_lineheight;
				left = 0;
				if((top + m_lineheight) >= m_height)
					break;
			}
			else if(*m_pcur == 0x09)
			{
				m_pcur++;
				left += m_textsize * 4;
				if(left > m_width)
				{
					top += m_lineheight;
					left = 0;
					if((top + m_lineheight) >= m_height)
					{
						m_pSpr->CloseBlk(hpng);
						break;
					}
				}
			}
			else
			{
				text[0] = *m_pcur++;
				hpng = m_pSpr->LoadStr(text, m_textsize, m_textcolor, &width);
				if(left + width > m_width)
				{
					top += m_lineheight;
					left = 0;
					if((top + m_lineheight) >= m_height)
					{
						m_pSpr->CloseBlk(hpng);
						break;
					}
				}
				m_pSpr->AlphaBlend(hbak, left, top + (m_lineheight-m_textsize)/2, width, m_textsize, hpng, 0, 0);
				m_pSpr->CloseBlk(hpng);
				left += width;
			}
		}
		m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top + startline, m_width, m_height - startline, hbak, 0, startline);
		m_pSpr->CloseBlk(hbak);
 	}
	else
		m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top + startline, m_width, m_height - startline, m_hFrameBak, 0, startline);
}

void CText::Next(void)
{
	if(*m_pcur != 0)
	{
		m_pagenum++;
		m_pagestart[m_pagenum] = m_pcur;
		Show(TRUE);
	}
}

void CText::Prev(void)
{
	if(m_pagenum != 0)
	{
		m_pagenum--;
		m_pcur = m_pagestart[m_pagenum];
		Show(TRUE);
	}
}

