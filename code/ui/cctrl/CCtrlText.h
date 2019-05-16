#pragma once
#include "CCtrlBase.h"

#define	MAX_PAGE	64
class CText:public CCtrlBase
{
public:
	CText(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_textsize = 24;
		m_textcolor = 0x0;
		m_src = NULL;
		startline = 0;
		m_textsize = 24;
	}
	~CText()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}
	BOOL DoInit(ContentManage*);
	void SetSrc(char* fpng)
	{
		m_src = fpng;
		m_pcur = m_src;
		memset(m_pagestart, 0, 4 * MAX_PAGE);
		m_pagestart[0] = m_src;
		m_pagenum = 0;
	}
	void SetTextColor(DWORD color)
	{
		m_textcolor = color;
	}
	void SetTextSize(DWORD size)
	{
		m_textsize = size;
	}
	void SetStartLine(DWORD line)
	{
		startline = line;
	}
	void Show(BOOL isShow);
	void Next(void);
	void Prev(void);
	BOOL CheckBegin(void)
	{
		if(m_pagenum == 0)
			return TRUE;
		return FALSE;
	}
	BOOL CheckEnd(void)
	{
		if(m_pcur == NULL)
			return TRUE;
		if(*m_pcur == 0)
			return TRUE;
		return FALSE;
	}
private:
	DWORD m_lineheight;
	DWORD m_textsize;
	DWORD m_textcolor;
	char *m_src;
	char *m_pcur;
	char *m_pagestart[MAX_PAGE];	// 记录多个页的起始数据地址
	DWORD m_pagenum;
	BOOL m_bend;
	DWORD startline;
};

