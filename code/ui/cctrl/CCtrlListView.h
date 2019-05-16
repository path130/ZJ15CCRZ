#pragma once
#include "CCtrlBase.h"
#include "CCtrlStatic.h"
class CItemView;

class CDPListView:public CCtrlBase
{
public:
	CDPListView(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_pItemView = NULL;
		m_pPageView = NULL;

		m_pAdapter = NULL;
		m_edgecolor = 0x0;
		m_PageColor = 0xffffff;
		m_PageTxt = 0;
		m_count = 0;
		m_pagebegin = 0;
		m_curptr = 0;

		m_titlecolor = 0xffffff;
		m_bIsPressed = FALSE;
		m_interval = 2;
		m_PageTextSize = 26;
		m_titlesize = 26;
	}
	~CDPListView();
	BOOL DoInit(ContentManage*);
	BOOL DoResponse(DWORD x, DWORD y, DWORD statue);
	void SetDataArray(DWORD count, char** parry);
	BOOL NextPage(void);
	BOOL PrevPage(void);
	BOOL IsHeadPage();
	BOOL IsLastPage();
	DWORD GetCurPtr(void);
	DWORD GetCurPagePtr(void);
	void SetCurPtr(DWORD cur);
	void Show(void);
	void Show(DWORD row, DWORD statue);
	char** m_pAdapter;
private:
	HANDLE m_hFrameBak;
	CItemView* m_pItemView;

	DWORD m_edgecolor;
	DWORD m_row;
	DWORD m_rowheight;
	DWORD m_interval;

	DWORD m_titlecolor;
	DWORD m_titlesize;
	DWORD m_titleabove;

	BOOL m_bIsPressed;
	DWORD m_lastRow;
	DWORD m_count;
	DWORD m_curptr;
	DWORD m_pagebegin;

	DWORD m_PageTxt;
	DWORD m_PageRight;
	DWORD m_PageTop;
	DWORD m_PageColor;
	DWORD m_PageTextSize;
	CDPStatic* m_pPageView;
};

typedef struct
{
	DWORD m_type;
	DWORD m_align;
	DWORD m_left;
	DWORD m_top;
	DWORD m_width;
	DWORD m_height;
	DWORD m_textsize;
	DWORD m_title;
} ListItem;

class CItemView
{
public:
	CItemView(CDPListView* pParent, DWORD height, DWORD interval)
	{
		m_pParent = pParent;
		m_pSpr = m_pParent->m_pSpr;
		m_height = height;
		m_intervel = interval;

		m_Count = 0;
		m_width = 0;

		m_hbakcolornormal = 0x00000000;
		m_hbakcolorpressed = 0x00000000;
		m_hbakcolorfocus = 0x00000000;
		m_textcolor = 0x000000;
		m_textcolorpressed = 0x808080;
		m_textcolorfocus = 0x1663F2;
		memset(m_pListItem, 0, sizeof(ListItem) * 16);
	}
	~CItemView()
	{
	}
	DWORD GetWidth(void){return m_width;}
	DWORD GetCount(void){return m_Count;}
	DWORD GetLeft(DWORD index){return m_pListItem[index].m_left;}
	DWORD GetWidth(DWORD index){return m_pListItem[index].m_width;}
	DWORD GetTitle(DWORD index){return m_pListItem[index].m_title;}
	BOOL DoInit(char* fname);
	HANDLE GetView(DWORD index, DWORD state, SIZE* size);
private:
	BOOL InitBkFrame(ContentManage* pCm);
	void AddNewItem(ContentManage* pCm);
	CDPListView* m_pParent;

	DWORD m_height;
	DWORD m_intervel;	// 两项间隔
	DWORD m_width;
	DWORD m_Count;		// 每个view有几项
	CDPGraphic* m_pSpr;

	ListItem m_pListItem[16];
	DWORD m_textcolor;
	DWORD m_textcolorpressed;
	DWORD m_textcolorfocus;
	DWORD m_hbakcolornormal;
	DWORD m_hbakcolorpressed;
	DWORD m_hbakcolorfocus;

};

