#pragma once
#include "CAppBase.h"

#define	STATUS_NORMAL			0
#define	STATUS_PRESSED			1
#define	STATUS_FOCUS			2
#define	STATUS_UNACK			3
#define	STATUS_MAX				4

#define	ITEM_TYPE_PNG			1
#define	ITEM_TYPE_TEXT			2
#define	ITEM_TYPE_INDEX			3

#define	DIRECT_HORIZON			1
#define	DIRECT_VERTICAL			2
#define	DIRECT_USERDEF			3

#define	ALIGN_LEFT				(1 << 0)
#define	ALIGN_CENTER			(1 << 1)
#define	ALIGN_RIGHT				(1 << 2)
#define	ALIGN_TOP				(1 << 3)
#define	ALIGN_MIDDLE			(1 << 4)
#define	ALIGN_BOTTOM			(1 << 5)

#define	CSC_ORG					0		// 0 使用原始图片
#define	CSC_BW					1		// 1 使用黑白图片
#define	CSC_REVERSION			2		// 3 使用反转图片
#define	CSC_COLOR				3		

#define	CTRL_BUTTON				1
#define	CTRL_EDITBOX			2
#define	CTRL_KEYBOARD			3
#define	CTRL_MBUTTON			4
#define	CTRL_TIME				5
#define	CTRL_TABLE				6
#define	CTRL_LISTVIEW			7
#define	CTRL_STATIC				8
#define	CTRL_PROGRESS			9
#define	CTRL_CLOCK				10
#define	CTRL_EMPTY				11
#define	CTRL_TEXT				12
#define CTRL_MKEYBOARD			13
#define CTRL_ENKEYBOARD			14
#define CTRL_IMEINPUT			15
#define CTRL_MTABLE				16

class CCtrlBase
{
public:
	CCtrlBase(CLayOut* pLayer)
	{
		m_pSpr = pLayer->m_pSpr;
		m_pLayer = pLayer;
		m_width = 0;
		m_height = 0;
		m_msgid = 0;
		m_hFrameBak = NULL;
	}
	~CCtrlBase()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}
	virtual void Show(){}
	virtual BOOL DoInit(ContentManage*) = 0;
	virtual BOOL DoResponse(DWORD xoff, DWORD yoff, DWORD statue)
	{
		return FALSE;
	}

	void Move(DWORD left , DWORD top)
	{
		SetStart(left , top);
	}
	void SetStart(DWORD left, DWORD top)
	{
		m_left = left;
		m_top = top;
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
			m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
			m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);
		}
	}
	virtual void DoKeyResponse(DWORD rawkey, DWORD statue)
	{
	}
	virtual void TickProcess(void)
	{
	}
	void GetWindow(RECT* prect)
	{
		prect->left = m_left + m_pLayer->m_left;
		prect->right = m_width;
		prect->top = m_top + m_pLayer->m_top;
		prect->bottom = m_height;
	}
	HANDLE DrawCscBmp(HANDLE horg, DWORD csc, SIZE* rsize);
	HANDLE DrawCscBmp(char* pngname, DWORD csc, SIZE* size);
	DWORD GetCscVal(char* str)
	{
		DWORD ret = 0;
		if(strstr(str, "bw") != NULL)
			ret |= CSC_BW;
		else if(strstr(str, "revert") != NULL)
			ret |= CSC_REVERSION;
		else if(strstr(str, "org") != NULL)
			ret |= CSC_ORG;
		else
			ret = CSC_COLOR | (hexconvert(str) << 8);

		return ret;
	}
	CDPGraphic* m_pSpr;
	CLayOut* m_pLayer;
	HANDLE m_hFrameBak;

	DWORD m_msgid;
	DWORD m_width;
	DWORD m_height;
	DWORD m_left;
	DWORD m_top;
};

