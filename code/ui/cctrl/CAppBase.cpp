#include "CCtrlModules.h"
static CDPGraphic* base_disp = NULL;

CLayOut::CLayOut(CAppBase* pHwnd)
{
	m_frame = NULL;
	m_CtlCount = 0;
	m_KeyCount = 0;
	m_TickCount = 0;
	m_CtlListCount = 0;
	m_pWin = pHwnd;
	m_pSpr = pHwnd->m_pSpr;
	name[0] = 0;
	m_isHide = FALSE;
	m_isUnack = FALSE;
}

CLayOut::~CLayOut(void)
{
	if(m_frame != NULL)
	{
		m_pSpr->CloseBlk(m_frame);
		m_frame = NULL;
	}
	DBGMSG(DISP_MOD, "delete CLayOut\r\n");
}

char* CLayOut::GetName(void)
{
	return name;
}

void CLayOut::ShowLay()
{
	if(!m_isHide)
	{
		m_pSpr->ShowBlk(m_frame);
		DBGMSG(DISP_MOD, "ShowLay %p %d %p\r\n", this, m_CtlCount, CtlEvent);
		if(!m_isUnack)
		{
			RegEventRegion(m_CtlCount, CtlEvent, m_layer);
			RegKeyboardMap(m_KeyCount, m_KeyEvent);
		}
	}
}

void CLayOut::HideLay()
{
	m_pSpr->HideBlk(m_frame);
	UnRegEventRegion(m_CtlCount, CtlEvent);
	UnregKeyboardMap(m_KeyCount, m_KeyEvent);
}

BOOL CLayOut::IsHide(){return m_isHide;}

void CLayOut::SwitchLay(BOOL isShow)
{
	if(isShow)
	{
		m_isHide = FALSE;
		ShowLay();
		m_pSpr->Show();
	}
	else
	{
		m_isHide = TRUE;
		HideLay();
		m_pSpr->Show();
	}
}

DWORD CLayOut::RequestMsgId(void)
{
	return m_pWin->RequestMsgId();
}

void CLayOut::ActivateCtrl(DWORD left, DWORD top, DWORD width, DWORD height , HANDLE hctrl)
{
	DWORD i;
	for(i = 0 ; i < m_CtlCount  ; i++)
	{
		if(CtlEvent[i].pCtrl == hctrl)
		{
			CtlEvent[i].xStart = m_left + left;
			CtlEvent[i].xEnd = m_left + left + width;
			CtlEvent[i].yStart = m_top + top;
			CtlEvent[i].yEnd = m_top + top + height;
			break;
		}
	}
}

void CLayOut::DeActivate(HANDLE hctrl)
{
	DWORD i;
	for(i = 0 ; i < m_CtlCount  ; i++)
	{
		if(CtlEvent[i].pCtrl == hctrl)
		{
			CtlEvent[i].xStart = 0;
			CtlEvent[i].xEnd = 0;
			CtlEvent[i].yStart = 0;
			CtlEvent[i].yEnd = 0;
			break;
		}
	}
}

void CLayOut::RegisterEvent(DWORD left, DWORD top, DWORD width, DWORD height, HANDLE hctrl)
{
	CtlEvent[m_CtlCount].xStart = m_left + left;
	CtlEvent[m_CtlCount].xEnd = m_left + left + width;
	CtlEvent[m_CtlCount].yStart = m_top + top;
	CtlEvent[m_CtlCount].yEnd = m_top + top + height;
	CtlEvent[m_CtlCount].pCtrl = hctrl;
	m_CtlCount++;
}

void CLayOut::RegisterJustEvent(DWORD left, DWORD top, DWORD width, DWORD height, HANDLE hctrl)
{
	CtlEvent[m_CtlCount].xStart = left;
	CtlEvent[m_CtlCount].xEnd = left + width;
	CtlEvent[m_CtlCount].yStart = top;
	CtlEvent[m_CtlCount].yEnd = top + height;
	CtlEvent[m_CtlCount].pCtrl = hctrl;
	m_CtlCount++;
}

void CLayOut::RegisterKeyEvent(int but, HANDLE hctl)
{
	m_KeyEvent[m_KeyCount].value = but;
	m_KeyEvent[m_KeyCount].pCtrl = hctl;
	m_KeyCount++;
}

void CLayOut::RegisterTickEvent(HANDLE hctl)
{
	TickEvent[m_TickCount] = hctl;
	m_TickCount++;
}

void CLayOut::RegisterCtrl(DWORD type, char* name, HANDLE hctl, DWORD m_msgid)
{
	if(m_CtlListCount < MAX_CTL_ITEM)
	{
		m_CtrlList[m_CtlListCount].property = type;
		strcpy(m_CtrlList[m_CtlListCount].name, name);
		m_CtrlList[m_CtlListCount].hCtrl = hctl;
		m_CtrlList[m_CtlListCount].IdBegin = m_msgid;
		m_CtlListCount++;
	}
	else
		DBGMSG(DPERROR, "too mach ctrl\r\n");
}

BOOL CLayOut::InitBkFrame(ContentManage* pCm)
{
	char* pcontent;
	HANDLE bakframe = NULL;
	BOOL justonce = FALSE;

	if((pcontent = pCm->FindContentByName("layer")) == NULL)
		return FALSE;
	m_layer = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("name")) != NULL)
		strcpy(name, pcontent);
	else
		strcpy(name, "mainlayer");

	if((pcontent = pCm->FindContentByName("ishide")) != NULL)
		m_isHide = TRUE;

	m_width = FRAME_WIDTH;
	m_height = FRAME_HEIGHT;
	m_left = 0;
	m_top = 0;

	if((pcontent = pCm->FindContentByName("statusbak")) != NULL)
		justonce = TRUE;

	if((pcontent = pCm->FindContentByName("bakpng")) != NULL)
	{
		SIZE size;
		bakframe = m_pSpr->LoadImage(pcontent, &size, justonce);
		if(bakframe == NULL)
			return FALSE;

		m_width = size.cx;
		m_height = size.cy;
		if((pcontent = pCm->FindContentByName("left")) != NULL)
			m_left = strtol(pcontent, NULL, 10);
		else
			m_left = (FRAME_WIDTH - m_width)/2;
		if((pcontent = pCm->FindContentByName("top")) != NULL)
			m_top = strtol(pcontent, NULL, 10);
		else
			m_top = (FRAME_HEIGHT - m_height)/2;
	}
	else
	{
		if((pcontent = pCm->FindContentByName("width")) != NULL)
		{
			m_width = strtol(pcontent, NULL, 10);
			m_left = (FRAME_WIDTH - m_width)/2;
		}
		if((pcontent = pCm->FindContentByName("height")) != NULL)
		{
			m_height = strtol(pcontent, NULL, 10);
			m_top = (FRAME_HEIGHT - m_height)/2;
		}
		if((pcontent = pCm->FindContentByName("left")) != NULL)
			m_left = strtol(pcontent, NULL, 10);

		if((pcontent = pCm->FindContentByName("top")) != NULL)
			m_top = strtol(pcontent, NULL, 10);
	}

	if(m_layer == 0)
		m_frame = m_pSpr->ReqBakFrame(m_left, m_top, m_width, m_height);
	else if(m_layer == 1)
		m_frame = m_pSpr->ReqFrame(m_left, m_top, m_width, m_height);
	else
		m_frame = m_pSpr->ReqCtrl(m_left, m_top, m_width, m_height);
	if(m_frame == NULL)
	{
		m_pSpr->CloseBlk(bakframe);
		return FALSE;
	}
	if(bakframe != NULL)
	{
		m_pSpr->BitBlt(m_frame, 0, 0, m_width, m_height, bakframe, 0, 0);
		m_pSpr->CloseBlk(bakframe);
	}
	else if((pcontent = pCm->FindContentByName("bakcolor")) != NULL)
	{
		DWORD bkcolor;
		DWORD edgecolor;

		bkcolor = hexconvert(pcontent);
		if((pcontent = pCm->FindContentByName("edgecolor")) != NULL)
		{
			edgecolor = hexconvert(pcontent);
			m_pSpr->InitBakRect(m_frame, edgecolor, bkcolor);
		}
		else
			m_pSpr->SetBK(m_frame, bkcolor);
	}
	return TRUE;
}

void CLayOut::InitListFrame(ContentManage* pCm)
{
	char* pcontent;
	DWORD width, rowheight, left, top, row, color[2];
	DWORD i;
	RECT rect;

	width = m_width;
	left = 0;
	top = 0;

	if((pcontent = pCm->FindContentByName("width"))!=NULL)
	{
		width = strtol(pcontent, NULL, 10);
		left = (m_width - width)/2;

	}
	if((pcontent = pCm->FindContentByName("rowheight")) == NULL)
		return;
	rowheight = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("row")) == NULL)
		return;
	row = strtol(pcontent, NULL, 10);
	top = (m_height - row*rowheight)/2;
	if((pcontent = pCm->FindContentByName("left")) != NULL)
		left = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("top")) != NULL)
		top = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("color1")) !=NULL)
		color[0] = hexconvert(pcontent);
	if((pcontent = pCm->FindContentByName("color2")) !=NULL)
		color[1] = hexconvert(pcontent);

	rect.left = left ;
	rect.right = left+width;

	for(i = 0; i < row; i++)
	{
		rect.top = top + rowheight*i;
		rect.bottom =rect.top + rowheight;
		if(i & 1)
			m_pSpr->FillSolidRect(m_frame, &rect, color[1]);
		else
			m_pSpr->FillSolidRect(m_frame, &rect, color[0]);
	}
	return;
}

void CLayOut::InitAlphaFrame(ContentManage* pCm)
{
	char* pcontent;
	char pngname[32];
	HANDLE hframe;
	DWORD width, height, left, top;
	SIZE size;
	DWORD middlecolor = 0;
	DWORD wtop, wleft, wbottom, wright;
	RECT rect;

	if((pcontent = pCm->FindContentByName("width")) != NULL)
	{
		width = strtol(pcontent, NULL, 10);
		left = (m_width - width)/2;
	}
	if((pcontent = pCm->FindContentByName("height")) != NULL)
	{
		height = strtol(pcontent, NULL, 10);
		top = (m_height - height)/2;
	}

	if((pcontent = pCm->FindContentByName("left")) != NULL)
		left = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("top")) != NULL)
		top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("bakpng")) == NULL)
		return;

	// 画上边,上边以png的高为高，以给的宽度为宽
	sprintf(pngname, "%st.png", pcontent);
	hframe = m_pSpr->LoadImage(pngname, &size);
	if(hframe == NULL)
		return;
	wtop = size.cy;
	m_pSpr->AlphaBlend(m_frame, left, top, width/2, wtop, hframe, 0, 0);
	m_pSpr->AlphaBlend(m_frame, left + width/2, top, width - width/2, wtop, hframe, size.cx - (width - width/2), 0);
	m_pSpr->CloseBlk(hframe);

	// 画下边,下边以png的高为高，已给的宽度为宽
	sprintf(pngname, "%sb.png", pcontent);
	hframe = m_pSpr->LoadImage(pngname, &size);
	if(hframe == NULL)
		return;
	wbottom = size.cy;
	m_pSpr->AlphaBlend(m_frame, left, top + height - wbottom, width/2, wbottom, hframe, 0, 0);
	m_pSpr->AlphaBlend(m_frame, left + width/2, top + height - wbottom, width - width/2, wbottom, hframe, size.cx - (width - width/2), 0);
	m_pSpr->CloseBlk(hframe);

	// 计算新的垂直方向的起点和高度,左边和右边都只需画剩余的高度即可
	top = wtop + top;
	height = height - wtop - wbottom;
	
	// 画左边,左边用图片重复填充即可
	sprintf(pngname, "%sl.png", pcontent);
	hframe = m_pSpr->LoadImage(pngname, &size);
	if(hframe == NULL)
		return;
	wleft = size.cx;
	wbottom = height;
	wtop = top;
	while(1)
	{
		if(wbottom > size.cy)
			m_pSpr->AlphaBlend(m_frame, left, wtop, wleft, size.cy, hframe, 0, 0);
		else
		{
			m_pSpr->AlphaBlend(m_frame, left, wtop, wleft, wbottom, hframe, 0, 0);
			break;
		}
		wbottom -= size.cy;
		wtop += size.cy;
	}
	m_pSpr->CloseBlk(hframe);

	// 画右边
	sprintf(pngname, "%sr.png", pcontent);
	hframe = m_pSpr->LoadImage(pngname, &size);
	if(hframe == NULL)
		return;
	wright = size.cx;
	wbottom = height;
	wtop = top;
	while(1)
	{
		if(wbottom > size.cy)
			m_pSpr->AlphaBlend(m_frame, left + width - wright, wtop, wright, size.cy, hframe, 0, 0);
		else
		{
			m_pSpr->AlphaBlend(m_frame, left + width - wright, wtop, wright, wbottom, hframe, 0, 0);
			break;
		}
		wbottom -= size.cy;
		wtop += size.cy;
	}
	m_pSpr->CloseBlk(hframe);

	// 计算新的水平方向的起点和高度,中间的填充色都只需画剩余的高度即可
	left = wleft + left;
	width = width - wleft - wright;

	if((pcontent = pCm->FindContentByName("middlecolor")) != NULL)
		middlecolor = hexconvert(pcontent);

	if(middlecolor != 0)
	{
		rect.left = left;
		rect.top = top;
		rect.right = left + width;
		rect.bottom = top + height;
		m_pSpr->FillSolidRect(m_frame, &rect, middlecolor);
		
	}
}

void CLayOut::InitPartFrame(ContentManage* pCm)
{
	char* pcontent;
	HANDLE hframe;
	DWORD width, height, left, top;
	DWORD isAlpha = FALSE;
	BOOL isCutOff = FALSE;
	BOOL isRepeat = FALSE;

	if((pcontent = pCm->FindContentByName("listframe")) != NULL)
	{
		InitListFrame(pCm);
		return;
	}
	else if((pcontent = pCm->FindContentByName("alphaframe")) != NULL)
	{
		InitAlphaFrame(pCm);
		return;
	}
	width = m_width;
	height = m_height;
	left = 0;
	top = 0;

	if((pcontent = pCm->FindContentByName("width")) != NULL)
	{
		width = strtol(pcontent, NULL, 10);
		left = (m_width - width)/2;
	}
	if((pcontent = pCm->FindContentByName("height")) != NULL)
	{
		height = strtol(pcontent, NULL, 10);
		top = (m_height - height)/2;
	}

	if((pcontent = pCm->FindContentByName("left")) != NULL)
		left = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("top")) != NULL)
		top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("cutoff")) != NULL)
		isCutOff = TRUE;

	if((pcontent = pCm->FindContentByName("repeat")) != NULL)
		isRepeat = TRUE;

	if((pcontent = pCm->FindContentByName("bakpng")) != NULL)
	{
		SIZE size;

		hframe = m_pSpr->LoadImage(pcontent, &size);
		if(hframe == NULL)
			return;
		if((pcontent = pCm->FindContentByName("isalpha")) != NULL)
			isAlpha = TRUE;
		if(isCutOff)
		{
			if(isAlpha)
			{
				if((size.cx > width) && (size.cy > height))
					m_pSpr->AlphaBlend(m_frame, left, top, width, height, hframe, (size.cx - width)/2, (size.cy - height)/2);
				else if(size.cx > width)
				{
					m_pSpr->AlphaBlend(m_frame, left, top, width, height/2, hframe, (size.cx - width)/2, 0);
					m_pSpr->AlphaBlend(m_frame, left, top + height/2, width, height - height/2, hframe, (size.cx - width)/2, size.cy - (height - height/2));
				}
				else if(size.cy > height)
				{
					m_pSpr->AlphaBlend(m_frame, left, top, width/2, height, hframe, 0, (size.cy - height)/2);
					m_pSpr->AlphaBlend(m_frame, left + width/2, top, width - width/2, height, hframe, size.cx - (width - width/2), (size.cy - height)/2);
				}
				else
				{
					m_pSpr->AlphaBlend(m_frame, left, top, width/2, height/2, hframe, 0, 0);
					m_pSpr->AlphaBlend(m_frame, left + width/2, top, width - width/2, height/2, hframe, size.cx - (width - width/2), 0);
					m_pSpr->AlphaBlend(m_frame, left, top + height/2, width/2, height - height/2, hframe, 0, size.cy - (height - height/2));
					m_pSpr->AlphaBlend(m_frame, left + width/2, top + height/2, width - width/2, height - height/2, hframe, size.cx - (width - width/2), size.cy - (height - height/2));
				}
			}
			else
			{
				if((size.cx > width) && (size.cy > height))
					m_pSpr->BitBlt(m_frame, left, top, width, height, hframe, (size.cx - width)/2, (size.cy - height)/2);
				else if(size.cx > width)
				{
					m_pSpr->BitBlt(m_frame, left, top, width, height/2, hframe, (size.cx - width)/2, 0);
					m_pSpr->BitBlt(m_frame, left, top + height/2, width, height - height/2, hframe, (size.cx - width)/2, size.cy - (height - height/2));
				}
				else if(size.cy > height)
				{
					m_pSpr->BitBlt(m_frame, left, top, width/2, height, hframe, 0, (size.cy - height)/2);
					m_pSpr->BitBlt(m_frame, left + width/2, top, width - width/2, height, hframe, size.cx - (width - width/2), (size.cy - height)/2);
				}
				else
				{
					m_pSpr->BitBlt(m_frame, left, top, width/2, height/2, hframe, 0, 0);
					m_pSpr->BitBlt(m_frame, left + width/2, top, width - width/2, height/2, hframe, size.cx - (width - width/2), 0);
					m_pSpr->BitBlt(m_frame, left, top + height/2, width/2, height - height/2, hframe, 0, size.cy - (height - height/2));
					m_pSpr->BitBlt(m_frame, left + width/2, top + height/2, width - width/2, height - height/2, hframe, size.cx - (width - width/2), size.cy - (height - height/2));
				}
			}
		}
		else if(isRepeat)
		{
			DWORD tleft, twidth;
			tleft = left;
			twidth = width;
			while(1)
			{
				if(twidth > size.cx)
					m_pSpr->BitBlt(m_frame, tleft, top, size.cx, size.cy, hframe, 0, 0);
				else
				{
					m_pSpr->BitBlt(m_frame, tleft, top, twidth, size.cy, hframe, 0, 0);
					break;
				}
				twidth -= size.cx;
				tleft += size.cx;
			}
		}
		else
		{
			if(isAlpha)
			{
				m_pSpr->AlphaBlend(m_frame, left, top, width/2, height/2, hframe, 0, 0);
				m_pSpr->AlphaBlend(m_frame, left + width/2, top, width - width/2, height/2, hframe, size.cx - (width - width/2), 0);
				m_pSpr->AlphaBlend(m_frame, left, top + height/2, width/2, height - height/2, hframe, 0, size.cy - (height - height/2));
				m_pSpr->AlphaBlend(m_frame, left + width/2, top + height/2, width - width/2, height - height/2, hframe, size.cx - (width - width/2), size.cy - (height - height/2));
			}
			else
			{
				m_pSpr->BitBlt(m_frame, left, top, width/2, height/2, hframe, 0, 0);
				m_pSpr->BitBlt(m_frame, left + width/2, top, width - width/2, height/2, hframe, size.cx - (width - width/2), 0);
				m_pSpr->BitBlt(m_frame, left, top + height/2, width/2, height - height/2, hframe, 0, size.cy - (height - height/2));
				m_pSpr->BitBlt(m_frame, left + width/2, top + height/2, width - width/2, height - height/2, hframe, size.cx - (width - width/2), size.cy - (height - height/2));
			}
		}
		m_pSpr->CloseBlk(hframe);
	}
	else if((pcontent = pCm->FindContentByName("bakcolor")) != NULL)
	{
		DWORD bkcolor;
		RECT rect;

		rect.left = left;
		rect.top = top;
		rect.right = left + width;
		rect.bottom = top + height;
		bkcolor = hexconvert(pcontent);
		if((pcontent = pCm->FindContentByName("isalpha")) != NULL)
			isAlpha = TRUE;
		if(isAlpha)
			m_pSpr->AlphaBlend(m_frame, left, top, width, height, bkcolor);
		else
			m_pSpr->FillSolidRect(m_frame, &rect, bkcolor);
	}
}

HANDLE CLayOut::GetCtrlByName(char* name, DWORD* idmap)
{
	DWORD i;

	for(i = 0; i < m_CtlListCount; i++)
	{
		if(strcmp(m_CtrlList[i].name, name) == 0)
		{
			if(idmap != NULL)
				*idmap = m_CtrlList[i].IdBegin;
			return m_CtrlList[i].hCtrl;
		}
	}
	return NULL;
}

void CLayOut::DoTickProcess(void)
{
	DWORD i;
	CCtrlBase* pCtrl;
	for(i = 0; i < m_TickCount; i++)
	{
		pCtrl = (CCtrlBase*)TickEvent[i];
		pCtrl->TickProcess();
	}
}

void CLayOut::PauseAck(void)
{
	UnRegEventRegion(m_CtlCount, CtlEvent);
	UnregKeyboardMap(m_KeyCount, m_KeyEvent);
	m_isUnack = TRUE;
}

void CLayOut::ResumeAck(void)
{
	if(!m_isHide)
	{
		RegEventRegion(m_CtlCount, CtlEvent, m_layer);
		RegKeyboardMap(m_KeyCount, m_KeyEvent);
		m_isUnack = FALSE;
	}
}

void CLayOut::DeinitLay(void)
{
	DWORD i;

	for(i = 0; i < m_CtlListCount; i++)
	{
		switch(m_CtrlList[i].property)
		{
			case CTRL_BUTTON:
				{
					CDPButton* pbuttona = (CDPButton*)m_CtrlList[i].hCtrl;
					delete pbuttona;
				}
				break;
			case CTRL_EDITBOX:
				{
					CEditBox* pEditBox = (CEditBox*)m_CtrlList[i].hCtrl;
					delete pEditBox;
				}
				break;
			case CTRL_KEYBOARD:
				{
					CKeyboard* pKeybd = (CKeyboard*)m_CtrlList[i].hCtrl;
					delete pKeybd;
				}
				break;
			case CTRL_MBUTTON:
				{
					CMButton* pMbutton = (CMButton*)m_CtrlList[i].hCtrl;
					delete pMbutton;
				}
				break;
			case CTRL_TIME:
				{
					CTimeDate* pTime = (CTimeDate*)m_CtrlList[i].hCtrl;
					delete pTime;
				}
				break;
			case CTRL_TABLE:
				{
					CTable* pTable = (CTable*)m_CtrlList[i].hCtrl;
					delete pTable;
				}
				break;
			case CTRL_MTABLE:
				{
					CMTable* pTable = (CMTable*)m_CtrlList[i].hCtrl;
					delete pTable;
				}
				break;
			case CTRL_LISTVIEW:
				{
					CDPListView* pListView = (CDPListView*)m_CtrlList[i].hCtrl;
					delete pListView;
				}
				break;
			case CTRL_STATIC:
				{
					CDPStatic* pStatue = (CDPStatic*)m_CtrlList[i].hCtrl;
					delete pStatue;
				}
				break;
			case CTRL_PROGRESS:
				{
					CDPProgress* pProgress = (CDPProgress*)m_CtrlList[i].hCtrl;
					delete pProgress;
				}
				break;
			case CTRL_CLOCK:
				{
					CTimeClock* pClock = (CTimeClock*)m_CtrlList[i].hCtrl;
					delete pClock;
				}
				break;
			case CTRL_EMPTY:
				{
					CEmpty* pEmpty = (CEmpty*)m_CtrlList[i].hCtrl;
					delete pEmpty;
				}
				break;
			case CTRL_TEXT:
				{
					CText* pText = (CText*)m_CtrlList[i].hCtrl;
					delete pText;
				}
				break;
			case CTRL_MKEYBOARD:
				{
					CMKeyboard* pMKeybd = (CMKeyboard*)m_CtrlList[i].hCtrl;
					delete pMKeybd;
				}
				break;
			case CTRL_ENKEYBOARD:
				{
					CEnKeyboard* pEnKeybd = (CEnKeyboard*)m_CtrlList[i].hCtrl;
					delete pEnKeybd;
				}
				break;
			case CTRL_IMEINPUT:
				{
					CImeInput* pImeInput = (CImeInput*)m_CtrlList[i].hCtrl;
					delete pImeInput;
				}
				break;
		}
	}
	m_CtlListCount = 0;
	if(m_frame != NULL)
	{
		m_pSpr->CloseBlk(m_frame);
		m_frame = NULL;
	}
}

BOOL CLayOut::InitLay(char* pbuf)
{
	ContentManage pCm;
	char temp[512];
	char* pdesc;

	m_pSpr = m_pWin->m_pSpr;
	DBGMSG(DISP_MOD, "InitLay start\r\n");

	DBGMSG(DISP_MOD, "InitLay frame\r\n");
	if(CFindContent(pbuf, "frame", temp, FALSE) == NULL)
	{
		return FALSE;
	}

	pCm.Init(temp);
	if(!InitBkFrame(&pCm))
	{
		return FALSE;
	}

	DBGMSG(DISP_MOD, "InitLay partframe\r\n");
	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, "partframe", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		InitPartFrame(&pCm);
	}

	DBGMSG(DISP_MOD, "InitLay static\r\n");
	pdesc = pbuf;
	while(1)
	{
		CDPStatic* pStatic;

		pdesc = CFindContent(pdesc, "static", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pStatic = new CDPStatic(this);
		if(pStatic != NULL)
		{
			if(!pStatic->DoInit(&pCm))
			{
				delete pStatic;
			}
		}
	}

	DBGMSG(DISP_MOD, "InitLay text\r\n");
	pdesc = pbuf;
	while(1)
	{
		CText* pText;

		pdesc = CFindContent(pdesc, "text", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pText = new CText(this);
		if(pText != NULL)
		{
			if(!pText->DoInit(&pCm))
			{
				delete pText;
			}
		}
	}

	DBGMSG(DISP_MOD, "InitLay editbox\r\n");
	pdesc = pbuf;
	while(1)
	{
		CEditBox* pEditBox;
		pdesc = CFindContent(pdesc, "editbox", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pEditBox = new CEditBox(this);
		if(pEditBox != NULL)
		{
			if(!pEditBox->DoInit(&pCm))
				delete pEditBox;
		}
	}

	DBGMSG(DISP_MOD, "InitLay keyboard\r\n");
	pdesc = pbuf;
	while(1)
	{
		CKeyboard* pKeyboard;
		pdesc = CFindContent(pbuf, "keyboard", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pKeyboard = new CKeyboard(this);
		if(pKeyboard != NULL)
		{
			if(!pKeyboard->DoInit(&pCm))
				delete pKeyboard;
		}
	}

	DBGMSG(DISP_MOD, "InitLay mkeyboard\r\n");
	pdesc = pbuf;
	while(1)
	{
		CMKeyboard* pKeyboard;
		pdesc = CFindContent(pbuf, "mkeyboard", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pKeyboard = new CMKeyboard(this);
		if(pKeyboard != NULL)
		{
			if(!pKeyboard->DoInit(&pCm))
				delete pKeyboard;
		}
	}

	DBGMSG(DISP_MOD, "InitLay enkeyboard\r\n");
	pdesc = pbuf;
	while(1)
	{
		CEnKeyboard* pKeyboard;
		pdesc = CFindContent(pbuf, "enkeyboard", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pKeyboard = new CEnKeyboard(this);
		if(pKeyboard != NULL)
		{
			if(!pKeyboard->DoInit(&pCm))
				delete pKeyboard;
		}
	}

	DBGMSG(DISP_MOD, "InitLay imeinput\r\n");
	pdesc = pbuf;
	while(1)
	{
		CImeInput* pKeyboard;
		pdesc = CFindContent(pbuf, "imeinput", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pKeyboard = new CImeInput(this);
		if(pKeyboard != NULL)
		{
			if(!pKeyboard->DoInit(&pCm))
				delete pKeyboard;
		}
	}

	DBGMSG(DISP_MOD, "InitLay time\r\n");
	pdesc = CFindContent(pbuf, "time", temp, FALSE);
	if(pdesc != NULL)
	{
		CTimeDate* pTimeDate;
		pCm.Init(temp);
		pTimeDate = new CTimeDate(this);
		if(pTimeDate != NULL)
		{
			if(!pTimeDate->DoInit(&pCm))
				delete pTimeDate;
		}
	}

	DBGMSG(DISP_MOD, "InitLay progress\r\n");
	pdesc = pbuf;
	while(1)
	{
		CDPProgress* pProgress;

		pdesc = CFindContent(pdesc, "progress", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pProgress = new CDPProgress(this);
		if(pProgress != NULL)
		{
			if(!pProgress->DoInit(&pCm))
			{
				delete pProgress;
			}
		}
	}

	DBGMSG(DISP_MOD, "InitLay mbutton\r\n");
	pdesc = pbuf;
	while(1)
	{
		CMButton* pMButton;

		pdesc = CFindContent(pdesc, "mbutton", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pMButton = new CMButton(this);
		if(pMButton != NULL)
		{
			if(!pMButton->DoInit(&pCm))
			{
				delete pMButton;
			}
		}
	}

	DBGMSG(DISP_MOD, "InitLay table\r\n");
	pdesc = CFindContent(pbuf, "table", temp, FALSE);
	if(pdesc != NULL)
	{
		CTable* pTable;
		pCm.Init(temp);
		pTable = new CTable(this);
		if(pTable != NULL)
		{
			if(!pTable->DoInit(&pCm))
				delete pTable;
		}
	}

	DBGMSG(DISP_MOD, "InitLay mtable\r\n");
	pdesc = CFindContent(pbuf, "mtable", temp, FALSE);
	if(pdesc != NULL)
	{
		CMTable* pTable;
		pCm.Init(temp);
		pTable = new CMTable(this);
		if(pTable != NULL)
		{
			if(!pTable->DoInit(&pCm))
				delete pTable;
		}
	}

	DBGMSG(DISP_MOD, "InitLay listview\r\n");
	pdesc = pbuf;
	while(1)
	{
		CDPListView* pButtonA;
		pdesc = CFindContent(pdesc, "listview", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pButtonA = new CDPListView(this);
		if(pButtonA != NULL)
		{
			if(!pButtonA->DoInit(&pCm))
				delete pButtonA;
		}
	}

	DBGMSG(DISP_MOD, "InitLay clock\r\n");
	pdesc = CFindContent(pbuf, "clock", temp, FALSE);
	if(pdesc != NULL)
	{
		CTimeClock* pClock;
		pCm.Init(temp);
		pClock = new CTimeClock(this);
		if(pClock != NULL)
		{
			if(!pClock->DoInit(&pCm))
				delete pClock;
		}
	}

	DBGMSG(DISP_MOD, "InitLay button\r\n");
	pdesc = pbuf;
	while(1)
	{
		CDPButton* pButton;
		pdesc = CFindContent(pdesc, "button", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pButton = new CDPButton(this);
		if(pButton != NULL)
		{
			if(!pButton->DoInit(&pCm))
				delete pButton;
		}
	}

	DBGMSG(DISP_MOD, "InitLay empty\r\n");
	pdesc = pbuf;
	while(1)
	{
		CEmpty* pEbutton;
		pdesc = CFindContent(pdesc, "empty", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		pEbutton = new CEmpty(this);
		if(pEbutton != NULL)
		{
			if(!pEbutton->DoInit(&pCm))
				delete pEbutton;
		}
	}

	DBGMSG(DISP_MOD, "InitLay end\r\n");
	ShowLay();
	return TRUE;
}

CAppBase::CAppBase(DWORD pHwnd)
{
	m_IdBase = pHwnd;
	m_IdOrg = m_IdBase;
	m_pSpr = NULL;
	m_dwTimeout = 0;
	memset(&m_layer, 0, sizeof(CLayOut*) * MAX_LAYER_FRAME);
	m_layindex = 0;
#ifdef _DEBUG
	m_screenoff = 100000;
#else
	m_screenoff = GetDelay(DELAY_SCREEN);
#endif
}

CAppBase::~CAppBase(void)
{
}

BOOL CAppBase::Destroy(void)
{
	DWORD i;
	DoPause();
	for(i = 0; i < m_layindex; i++)
	{
		m_layer[i]->DeinitLay();
		delete m_layer[i];
	}
	return TRUE;
}

BOOL CAppBase::DoPause(void)
{
	DWORD i;
	for(i = 0; i < m_layindex; i++)
		m_layer[i]->HideLay();
	//m_pSpr->Show();
	return TRUE;
}

void CAppBase::DoResume(void)
{
	DWORD i;
	m_dwTimeout = 0;
	for(i = 0; i < m_layindex; i++)
		m_layer[i]->ShowLay();
}

DWORD CAppBase::RequestMsgId(void)
{
	return m_IdOrg++;
}

void CAppBase::PauseAck(void)
{
	DWORD i;
	for(i = 0; i < m_layindex; i++)
		m_layer[i]->PauseAck();
}

void CAppBase::ResumeAck(void)
{
	DWORD i;
	for(i = 0; i < m_layindex; i++)
		m_layer[i]->ResumeAck();
}

void CAppBase::InitDisplay()
{
	if(base_disp == NULL)
	{
		base_disp = new CDPGraphic();
		base_disp->Inite();
		DBGMSG(DPINFO, "CAppBase InitDisplay\r\n");
	}
	m_pSpr = base_disp;
}

void CAppBase::Show()
{
	base_disp->Show();
}

int CAppBase::InitXmlFile(const char* xmlname, char** rpbuf)
{
	char temp[128];
	char name[128];
	int i;
	int flen;
	char* pbuf;
	char* pdesc;
	ContentManage pCm;
	char* psubbuf;
	int fsublen;

	sprintf(temp, "%s/%s", LAYOUT_DIR, xmlname);
	DBGMSG(DISP_MOD, "InitXmlFile %s\r\n", temp);
	flen = BReadFile(temp, &pbuf);
	if(flen == 0)
	{
		return FALSE;
	}

	for(i = 0; i < flen; i++)
	{
		if((pbuf[i] >= 0x41)
			&& (pbuf[i] <= 0x5a))
			pbuf[i] = pbuf[i] | 0x20;
	}
	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, "include", temp, TRUE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		xmlname = pCm.FindContentByName("xmlfile");
		sprintf(name, "%s/%s", LAYOUT_DIR, xmlname);
		DBGMSG(DISP_MOD, "Include %s\r\n", name);
		fsublen = BReadFile(name, &psubbuf);
		if(fsublen == 0)
			continue;
		for(i = 0; i < fsublen; i++)
		{
			if((psubbuf[i] >= 0x41)
				&& (psubbuf[i] <= 0x5a))
				psubbuf[i] = psubbuf[i] | 0x20;
		}
		{
			char* newbuf;
			DWORD offset = (pdesc - pbuf);
			newbuf = (char*)malloc(flen + fsublen + 2);
			strncpy(newbuf, pbuf, offset);
			newbuf[offset]=0;
			strcat(newbuf, psubbuf);
			free(psubbuf);
			strcat(newbuf, pdesc);
			free(pbuf);
			pbuf = newbuf;
			pdesc = pbuf + offset;
			flen += fsublen;
		}
	}
	*rpbuf = pbuf;
	return flen;
}


BOOL CAppBase::InitFrame(const char* xmlname)
{
	char* pdesc;
	char* pbuf;
	int flen;
	char* pcache;
	CLayOut* pLayOut;

	InitDisplay();

	flen = InitXmlFile(xmlname, &pbuf);
	if(flen == 0)
	{
		DBGMSG(DPWARNING, "%s InitFrame %s fail!\r\n", WARNINGINFO, xmlname);
		return FALSE;
	}

	DBGMSG(DISP_MOD, "Load file %s %d\r\n", xmlname, flen);
	pcache = (char*)malloc(flen);
	pdesc = pbuf;

	while(1)
	{
		if((pdesc = CFindContent(pdesc, "layout", pcache, TRUE)) != NULL)
		{
			pLayOut = new CLayOut(this);
			if(pLayOut->InitLay(pcache))
			{
				m_layer[m_layindex] = pLayOut;
				m_layindex++;
			}
			else
				delete pLayOut;
		}
		else
			break;
	}
	free(pbuf);
	free(pcache);
	return TRUE;
}

BOOL CAppBase::InitLayer(const char* xmlname)
{
	char* pbuf;
	int flen;
	CLayOut* pLayOut;
	BOOL ret = FALSE;
	flen = InitXmlFile(xmlname, &pbuf);
	if(flen > 0)
	{
		pLayOut = new CLayOut(this);
		if(pLayOut->InitLay(pbuf))
		{
			m_layer[m_layindex] = pLayOut;
			m_layindex++;
			Show();
			ret = TRUE;
		}
		free(pbuf);
	}
	return ret;
}

HANDLE CAppBase::GetCtrlByName(const char* name, DWORD* idmap)
{
	char convert[32];
	DWORD i;
	HANDLE hret;
	for(i = 0; i < strlen(name); i++)
	{
		if((name[i] >= 0x41)
			&& (name[i] <= 0x5a))
			convert[i] = name[i] | 0x20;
		else
			convert[i] = name[i];
	}
	convert[i] = 0;
	for(i = 0; i < m_layindex; i++)
	{
		if(strcmp(m_layer[i]->GetName(), convert) == 0)
		{
			if(idmap != NULL)
				*idmap = 0;
			return m_layer[i];
		}
		else
		{
			hret = m_layer[i]->GetCtrlByName(convert, idmap);
			if(hret != NULL)
				return hret;
		}
	}
	return NULL;
}

void CAppBase::DumpImage(char* filename)
{
	HANDLE hframe;
	DWORD i;
	hframe = m_pSpr->ReqTempBlk(FRAME_WIDTH, FRAME_HEIGHT);
	m_pSpr->SetBK(hframe, 0xffC2BCBE);
	for(i = 0; i < m_layindex; i++)
	{
		if(!m_layer[i]->m_isHide)
			m_pSpr->AlphaBlend(hframe, m_layer[i]->m_left, m_layer[i]->m_top,
					m_layer[i]->m_width, m_layer[i]->m_height, m_layer[i]->m_frame, 0, 0);
	}
	m_pSpr->DumpBmp(hframe, filename);
	m_pSpr->CloseBlk(hframe);
}


