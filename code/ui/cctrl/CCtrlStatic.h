#pragma once
#include "CCtrlBase.h"

class CDPStatic:public CCtrlBase
{
public:
	CDPStatic(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_type = ITEM_TYPE_TEXT;
		m_textcolor = 0xFFFFFFFF;
		m_src[0] = 0;
		m_swidth = 0;
		m_sheight = 0;
		m_align = 0;
		m_sleft = 0;
		m_stop = 0;
		m_textsize = 23;
	}
	~CDPStatic()
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
		strcpy(m_src, fpng);
	}

	void SetSrc(DWORD fpng)
	{
		sprintf(m_src, "%d", fpng);
	}

	BOOL SrcPngCompare(char* png)
	{
		if(strcmp(m_src, png) == 0)
			return TRUE;
		else
			return FALSE;
	}

	void SetTextColor(DWORD color)
	{
		m_textcolor = color;
	}
	
	void SetTextSize(DWORD size)
	{
		m_textsize = size;
	}
	void SetAlign(DWORD align)
	{
		m_align = align;
	}
	void SetType(DWORD type)
	{
		m_type = type;
	}
	void SetStart(DWORD x, DWORD y)
	{
		m_left = x;
		m_top = y;
	}
	DWORD GetWidth()
	{
		return m_swidth;
	}
	DWORD GetTop()
	{
		return m_top;
	}
	void Show(BOOL isShow);
	void RefreshBak();	// 刷新背景
private:
	DWORD m_textsize;	// 文字的按下
	DWORD m_textcolor;	// 文字的颜色
	char m_src[256];	// 保存要显示的文字的字符串或要显示的图片文件名
	DWORD m_type;		// 当前显示的是文字还是图片
	DWORD m_align;		// 对齐方式，包括垂直和水平方向的对齐方式
	DWORD m_sleft;		// 这四个参数保存的是当前显示的位置和大小，背景数据保存在m_hFrameBak中
	DWORD m_stop;
	DWORD m_swidth;
	DWORD m_sheight;
};


