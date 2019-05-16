#pragma once
#include "CCtrlBase.h"

class CDPButton:public CCtrlBase
{
public:
	CDPButton(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		DWORD i;
		srctext[0] = 0;
		for(i = 0; i < STATUS_MAX; i++)
		{
			m_srcpng[i][0] = 0;
			m_bakpng[i][0] = 0;
			m_textcolor[i] = 0xFFFFFFFF;
			m_textsize[i] = 20;
			m_postprocess[i] = CSC_ORG;
		}
		m_direct = DIRECT_VERTICAL;
		m_isAlpha = FALSE;
		m_split = 0;
		m_bReverse = FALSE;
		m_isHide = FALSE;
		m_statue = STATUS_NORMAL;
		m_bpngistxt = FALSE;
		m_bBakColor = FALSE;
		m_afterpress = STATUS_PRESSED;
	}
	~CDPButton()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}

	BOOL DoInit(ContentManage*);
	void SetSrcpng(char * srcpng)
	{
		strcpy(m_srcpng[STATUS_NORMAL], srcpng);
		strcpy(m_srcpng[STATUS_PRESSED], srcpng);
		strcpy(m_srcpng[STATUS_FOCUS], srcpng);
		strcpy(m_srcpng[STATUS_UNACK], srcpng);
	}
	
	void SetBkpng(char * srcpng)
	{
		strcpy(m_bakpng[STATUS_NORMAL], srcpng);
		strcpy(m_bakpng[STATUS_PRESSED], srcpng);
		strcpy(m_bakpng[STATUS_FOCUS], srcpng);
		strcpy(m_bakpng[STATUS_UNACK], srcpng);
	}

	void SetSrcpng(DWORD i, char * srcpresspng)
	{
		if(i < STATUS_MAX)
			strcpy(m_srcpng[i] , srcpresspng);
	}

	void SetBkpng(DWORD i, char * bkpng)
	{
		if(i < STATUS_MAX)
			strcpy(m_bakpng[i] , bkpng);
	}
	
	void SetSrcText(char* text)
	{
		strcpy(srctext, text);
	}

	void SetTextSize(DWORD textSize)
	{
		for(int i = 0; i < STATUS_MAX; i++)
			m_textsize[i] = textSize;
	}

	void SetTextColor(DWORD statue, DWORD color)
	{
		m_textcolor[statue] = color;
	}

	BOOL DoResponse(DWORD xoff, DWORD yoff, DWORD statue);
	void Show(DWORD statue);
	void Hide(void);

private:
	char m_bakpng[STATUS_MAX][64];
	char m_srcpng[STATUS_MAX][64];

	DWORD m_bakColor[STATUS_MAX];
	BOOL m_bBakColor;
	DWORD m_srcleft;
	DWORD m_srctop;
	DWORD m_txtleft;
	DWORD m_txttop;
	DWORD m_afterpress;

	BOOL m_bpngistxt;
	char srctext[64];
	DWORD m_postprocess[STATUS_MAX];
	DWORD m_textsize[STATUS_MAX];
	DWORD m_textcolor[STATUS_MAX];
	DWORD m_srcpngalign;			// 上层png对齐方式
	DWORD m_txtalign;				// 上层文字对齐方式
	DWORD m_split;					// 图片文字分割点
	DWORD m_direct;					// 图片文字是垂直还是水平
	BOOL m_isAlpha;					// 图像和图像间是否做alpha混合
	BOOL m_bReverse;				// 图像在前还是文字在前
	BOOL m_isHide;					// 创建的时候是否需要显示
	DWORD m_statue;

};

