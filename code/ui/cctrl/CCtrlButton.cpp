#include "CCtrlButton.h"

BOOL CDPButton::DoInit(ContentManage* pCm)
{
	char* pcontent;
	int text;

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

	if((pcontent = pCm->FindContentByName("reverse")) != NULL)
		m_bReverse = TRUE;

	if((pcontent = pCm->FindContentByName("twotext")) != NULL)
		m_bpngistxt = TRUE;

	if((pcontent = pCm->FindContentByName("afterpress")) != NULL)
		m_afterpress = strtol(pcontent, NULL, 10);

	m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

	if((pcontent = pCm->FindContentByName("bakpng")) != NULL)
	{
		strcpy(m_bakpng[STATUS_NORMAL], pcontent);

		if((pcontent = pCm->FindContentByName("bakpngpress")) != NULL)
			strcpy(m_bakpng[STATUS_PRESSED], pcontent);
		else
			strcpy(m_bakpng[STATUS_PRESSED], m_bakpng[STATUS_NORMAL]);
		
		if((pcontent = pCm->FindContentByName("bakpngfocus")) != NULL)
			strcpy(m_bakpng[STATUS_FOCUS], pcontent);
		else
			strcpy(m_bakpng[STATUS_FOCUS], m_bakpng[STATUS_NORMAL]);
		
		if((pcontent = pCm->FindContentByName("bakpngunack")) != NULL)
			strcpy(m_bakpng[STATUS_UNACK], pcontent);
		else
			strcpy(m_bakpng[STATUS_UNACK], m_bakpng[STATUS_NORMAL]);
	}

	if((pcontent = pCm->FindContentByName("bakcolor")) != NULL)
	{
		m_bakColor[STATUS_NORMAL] = hexconvert(pcontent);

		if((pcontent = pCm->FindContentByName("bakcolorpress")) != NULL)
			m_bakColor[STATUS_PRESSED] = hexconvert(pcontent);
		else
			m_bakColor[STATUS_PRESSED] = m_bakColor[STATUS_NORMAL];

		if((pcontent = pCm->FindContentByName("bakcolorfocus")) != NULL)
			m_bakColor[STATUS_FOCUS] = hexconvert(pcontent);
		else
			m_bakColor[STATUS_FOCUS] = m_bakColor[STATUS_NORMAL];

		if((pcontent = pCm->FindContentByName("bakcolorpress")) != NULL)
			m_bakColor[STATUS_UNACK] = hexconvert(pcontent);
		else
			m_bakColor[STATUS_UNACK] = m_bakColor[STATUS_NORMAL];

		m_bBakColor = TRUE;
	}

	if((pcontent = pCm->FindContentByName("srcpng")) != NULL)
	{
		if(m_bpngistxt)
		{
			text = strtol(pcontent, NULL, 10);
			strcpy(m_srcpng[STATUS_NORMAL], GetStringByID(text));
			strcpy(m_srcpng[STATUS_PRESSED], GetStringByID(text));
			strcpy(m_srcpng[STATUS_FOCUS], GetStringByID(text));
			strcpy(m_srcpng[STATUS_UNACK], GetStringByID(text));
		}
		else
		{
			strcpy(m_srcpng[STATUS_NORMAL], pcontent);

			if((pcontent = pCm->FindContentByName("srcpngpress")) != NULL)
				strcpy(m_srcpng[STATUS_PRESSED], pcontent);
			else
				strcpy(m_srcpng[STATUS_PRESSED], m_srcpng[STATUS_NORMAL]);
		
			if((pcontent = pCm->FindContentByName("srcpngfocus")) != NULL)
				strcpy(m_srcpng[STATUS_FOCUS], pcontent);
			else
				strcpy(m_srcpng[STATUS_FOCUS], m_srcpng[STATUS_NORMAL]);

			if((pcontent = pCm->FindContentByName("srcpngunack")) != NULL)
				strcpy(m_srcpng[STATUS_UNACK], pcontent);
			else
				strcpy(m_srcpng[STATUS_UNACK], m_srcpng[STATUS_NORMAL]);
		}
	}

	if((pcontent = pCm->FindContentByName("cscnormal")) != NULL)
		m_postprocess[STATUS_NORMAL] = GetCscVal(pcontent);
	if((pcontent = pCm->FindContentByName("cscpress")) != NULL)
		m_postprocess[STATUS_PRESSED] = GetCscVal(pcontent);
	if((pcontent = pCm->FindContentByName("cscfocus")) != NULL)
		m_postprocess[STATUS_FOCUS] = GetCscVal(pcontent);
	if((pcontent = pCm->FindContentByName("cscunack")) != NULL)
		m_postprocess[STATUS_UNACK] = GetCscVal(pcontent);

	if((pcontent = pCm->FindContentByName("srctext")) != NULL)
	{
		text = strtol(pcontent, NULL, 10);
		strcpy(srctext, GetStringByID(text));
	}

	if((pcontent = pCm->FindContentByName("textsize")) != NULL)
	{
		m_textsize[STATUS_NORMAL] = strtol(pcontent, NULL, 10);
		m_textsize[STATUS_PRESSED] = m_textsize[STATUS_NORMAL];
		m_textsize[STATUS_FOCUS] = m_textsize[STATUS_NORMAL];
		m_textsize[STATUS_UNACK] = m_textsize[STATUS_NORMAL];
	}
	if((pcontent = pCm->FindContentByName("textsizepress")) != NULL)
		m_textsize[STATUS_PRESSED] = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("textcolor")) != NULL)
	{
		m_textcolor[STATUS_NORMAL] = hexconvert(pcontent);
		m_textcolor[STATUS_PRESSED] = m_textcolor[STATUS_NORMAL];
		m_textcolor[STATUS_FOCUS] = m_textcolor[STATUS_NORMAL];
		m_textcolor[STATUS_UNACK] = m_textcolor[STATUS_NORMAL];
	}
	if((pcontent = pCm->FindContentByName("textcolorpress")) != NULL)
		m_textcolor[STATUS_PRESSED] = hexconvert(pcontent);
	if((pcontent = pCm->FindContentByName("textcolorfocus")) != NULL)
		m_textcolor[STATUS_FOCUS] = hexconvert(pcontent);
	if((pcontent = pCm->FindContentByName("textcolorunack")) != NULL)
		m_textcolor[STATUS_UNACK] = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("text")) != NULL)
	{
		text = strtol(pcontent, NULL, 10);
		SetSrcText(GetStringByID(text));
	}

	if((pcontent = pCm->FindContentByName("align")) != NULL)
	{
		if(strcmp(pcontent, "userdef") == 0)
		{
			m_direct = DIRECT_USERDEF;
			if((pcontent = pCm->FindContentByName("srcleft")) == NULL)
				m_srcleft = -1;
			else
				m_srcleft = strtol(pcontent, NULL, 10);

			if((pcontent = pCm->FindContentByName("srctop")) == NULL)
				m_srctop = -1;
			else
				m_srctop = strtol(pcontent, NULL, 10);

			if((pcontent = pCm->FindContentByName("textleft")) == NULL)
				m_txtleft = -1;
			else
				m_txtleft = strtol(pcontent, NULL, 10);

			if((pcontent = pCm->FindContentByName("texttop")) == NULL)
				m_txttop = -1;
			else
				m_txttop = strtol(pcontent, NULL, 10);
		}
		else if(strcmp(pcontent,"vertica") == 0)
			m_direct = DIRECT_VERTICAL;
		else
			m_direct = DIRECT_HORIZON;
	}

	if((pcontent = pCm->FindContentByName("splitpoint")) != NULL)
		m_split = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("pngalign")) != NULL)
	{
		if(strcmp(pcontent, "top") == 0)
			m_srcpngalign = ALIGN_TOP;
		else if(strcmp(pcontent, "bottom") == 0)
			m_srcpngalign = ALIGN_BOTTOM;
		else
			m_srcpngalign = ALIGN_MIDDLE;
	}
	else
		m_srcpngalign = ALIGN_MIDDLE;

	if((pcontent = pCm->FindContentByName("textalign")) != NULL)
	{
		if(strcmp(pcontent, "top") == 0)
			m_txtalign = ALIGN_TOP;
		else if(strcmp(pcontent, "bottom") == 0)
			m_txtalign = ALIGN_BOTTOM;
		else
			m_txtalign = ALIGN_MIDDLE;
	}
	else
		m_txtalign = ALIGN_MIDDLE;

	if((pcontent = pCm->FindContentByName("ishide")) != NULL)
		m_isHide = TRUE;
	if((pcontent = pCm->FindContentByName("isalpha")) != NULL)
		m_isAlpha = TRUE;

	// 设置点击区域
	int left = m_left;
	int top = m_top;
	int width = m_width;
	int height = m_height;
	if((pcontent = pCm->FindContentByName("active_left")) != NULL)
		left = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("active_top")) != NULL)
		top = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("active_width")) != NULL)
		width = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName("active_height")) != NULL)
		height = strtol(pcontent, NULL, 10);

	m_pLayer->RegisterEvent(left, top, width, height, this);

	m_msgid = m_pLayer->RequestMsgId();
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_BUTTON, "button", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_BUTTON, pcontent, this, m_msgid);
	if(!m_isHide)
		Show(STATUS_NORMAL);
	return TRUE;
}

BOOL CDPButton::DoResponse(DWORD xoff, DWORD yoff, DWORD statue)
{
	if((m_isHide)
		|| (m_statue == STATUS_FOCUS)
		|| (m_statue == STATUS_UNACK))
	{
		return FALSE;
	}
	switch(statue)
	{
		case TOUCH_DOWN:
			Show(STATUS_PRESSED);
			break;
		case TOUCH_UP:
			Show(m_afterpress);
			DPPostMessage(TOUCH_MESSAGE, m_msgid, 0, 0);
			break;
		case TOUCH_MOVEOUT:
			Show(STATUS_NORMAL);
			break;
	}
	return TRUE;
}

void CDPButton::Hide(void)
{
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hFrameBak, 0, 0);
	m_isHide = TRUE;
}

void CDPButton::Show(DWORD statue)
{
	HANDLE h_bk;
	SIZE bksize;
	SIZE srcsize;
	HANDLE h_png = NULL;
	HANDLE h_str = NULL;
	DWORD wide;
	WORD pos[256];
	DWORD i;

	m_isHide = FALSE;
	m_statue = statue;
	if(strlen(m_bakpng[statue]) !=0)
	{
		h_bk = DrawCscBmp(m_bakpng[statue], CSC_ORG, &bksize);
		if((m_width != bksize.cx)
			|| (m_height != bksize.cy))
		{
			h_png = m_pSpr->ReqTempBlk(m_width, m_height);
			m_pSpr->BitBlt(h_png, 0, 0, 
								m_width/2, m_height/2, 
								h_bk, 0, 0);
			m_pSpr->BitBlt(h_png, m_width/2, 0, 
								m_width - m_width/2, m_height/2, 
								h_bk, bksize.cx - (m_width - m_width/2), 0);
			m_pSpr->BitBlt (h_png, 0, m_height/2, 
								m_width/2, m_height - m_height/2, 
								h_bk, 0, bksize.cy - (m_height - m_height/2));
			m_pSpr->BitBlt(h_png, m_width/2, m_height/2, 
								m_width - m_width/2, m_height - m_height/2, 
								h_bk, bksize.cx - (m_width - m_width/2), bksize.cy - (m_height - m_height/2));
			m_pSpr->CloseBlk(h_bk);
			h_bk = h_png;
			bksize.cx = m_width;
			bksize.cy = m_height;
		}
		h_png = NULL;
	}
	else if(m_bBakColor)
	{
		h_bk = m_pSpr->ReqTempBlk(m_width, m_height);
		m_pSpr->SetBK(h_bk, m_bakColor[statue]);
		bksize.cx = m_width;
		bksize.cy = m_height;
	}
	else
	{
		h_bk = m_pSpr->ReqTempBlk(m_width, m_height);
		if(m_isAlpha)
			m_pSpr->SetBK(h_bk, 0);
		else
			m_pSpr->BitBlt(h_bk, 0, 0, m_width, m_height, m_hFrameBak, 0, 0);
		bksize.cx = m_width;
		bksize.cy = m_height;
	}

	if(m_bpngistxt)
	{
		h_png = m_pSpr->LoadStr(m_srcpng[statue], m_textsize[statue], m_textcolor[statue], &wide, pos);
		if(wide > m_width)
		{
			for(i = 0; i < strlen(m_srcpng[statue]); i++)
				if(pos[i] > m_width)
					break;
			wide = pos[i-1];
		}
		srcsize.cx = wide;
		srcsize.cy = m_textsize[statue];
	}
	else
	{
		if(strlen(m_srcpng[statue]) !=0)
			h_png = m_pSpr->LoadImage(m_srcpng[statue], &srcsize);
	}

	if(strlen(srctext) != 0)
	{
		h_str = m_pSpr->LoadStr(srctext, m_textsize[statue], m_textcolor[statue], &wide, pos);
		if(wide > m_width)
		{
			for(i = 0; i < strlen(srctext); i++)
				if(pos[i] > m_width)
					break;
			wide = pos[i-1];
		}
	}

	if((h_png != NULL)
		&& (h_str != NULL))
	{
		if(m_direct == DIRECT_USERDEF)
		{
			if(m_srcleft == -1)
				m_srcleft = (bksize.cx - srcsize.cx)/2;
			if(m_txtleft == -1)
				m_txtleft = (bksize.cx - wide)/2;

			m_pSpr->AlphaBlend(h_bk, m_srcleft, m_srctop, srcsize.cx, srcsize.cy, h_png, 0, 0);
			m_pSpr->AlphaBlend(h_bk, m_txtleft, m_txttop, wide, m_textsize[statue], h_str, 0, 0);
		}
		else if(m_direct == DIRECT_VERTICAL)
		{
			if(m_split == 0)
				m_split = bksize.cy - m_textsize[statue];

			// 图片为top时，需顶着最上面画
			if(m_srcpngalign == ALIGN_TOP)
				m_pSpr->AlphaBlend(h_bk, (bksize.cx - srcsize.cx)/2, 0, srcsize.cx, srcsize.cy, h_png, 0, 0);
			else if(m_srcpngalign == ALIGN_MIDDLE)	// 为居中时，在最上和分割点间
				m_pSpr->AlphaBlend(h_bk, (bksize.cx - srcsize.cx)/2, (m_split - srcsize.cy)/2, srcsize.cx, srcsize.cy, h_png, 0, 0);
			else// 为bottom时，已分割点为底来画,所以分割点的高度要大于图片高度
				m_pSpr->AlphaBlend(h_bk, (bksize.cx - srcsize.cx)/2, m_split - srcsize.cy, srcsize.cx, srcsize.cy, h_png, 0, 0);

			// 文字为top时，需顶着分割点画
			if(m_txtalign == ALIGN_TOP)
				m_pSpr->AlphaBlend(h_bk, (bksize.cx - wide)/2, m_split, wide, m_textsize[statue], h_str, 0, 0);
			else if(m_srcpngalign == ALIGN_MIDDLE)	// 为居中时，在分割点和底之间
				m_pSpr->AlphaBlend(h_bk, (bksize.cx - wide)/2, m_split + ((bksize.cy - m_split) - m_textsize[statue])/2, wide, m_textsize[statue], h_str, 0, 0);
			else	// 为bottom时，已底边为底画
				m_pSpr->AlphaBlend(h_bk, (bksize.cx - wide)/2, bksize.cy - m_textsize[statue], wide, m_textsize[statue], h_str, 0, 0);
		}
		else
		{
			if(m_bReverse)
			{
				// 水平方向
				// 图片为top时，顶着最左边话
				if(m_srcpngalign == ALIGN_TOP)
					m_pSpr->BitBlt(h_bk, m_split, (bksize.cy - srcsize.cy)/2, srcsize.cx, srcsize.cy, h_png, 0, 0);
				else if(m_srcpngalign == ALIGN_MIDDLE)	// 为居中时，在最左边和分割点间画
					m_pSpr->BitBlt(h_bk, m_split + (bksize.cx - m_split - srcsize.cx)/2, (bksize.cy - srcsize.cy)/2, srcsize.cx, srcsize.cy, h_png, 0, 0);
				else	// 为bottom时，已分割点为右边画
					m_pSpr->BitBlt(h_bk, bksize.cx - srcsize.cx, (bksize.cy - srcsize.cy)/2, srcsize.cx, srcsize.cy, h_png, 0, 0);

				// 图片为top时，顶着分割点画
				if(m_txtalign == ALIGN_TOP)
					m_pSpr->AlphaBlend(h_bk, 0, (bksize.cy - m_textsize[statue])/2, wide, m_textsize[statue], h_str, 0, 0);
				else if(m_txtalign == ALIGN_MIDDLE)	// 为居中时，在分割点和最右边间画
					m_pSpr->AlphaBlend(h_bk, (m_split - wide)/2, (bksize.cy - m_textsize[statue])/2, wide, m_textsize[statue], h_str, 0, 0);
				else
					m_pSpr->AlphaBlend(h_bk, m_split - wide, (bksize.cy - m_textsize[statue])/2, wide, m_textsize[statue], h_str, 0, 0);
			}
			else
			{
				// 水平方向
				// 图片为top时，顶着最左边话
				if(m_srcpngalign == ALIGN_TOP)
					m_pSpr->AlphaBlend(h_bk, 0, (bksize.cy - srcsize.cy)/2, srcsize.cx, srcsize.cy, h_png, 0, 0);
				else if(m_srcpngalign == ALIGN_MIDDLE)	// 为居中时，在最左边和分割点间画
					m_pSpr->AlphaBlend(h_bk, (m_split - srcsize.cx)/2, (bksize.cy - srcsize.cy)/2, srcsize.cx, srcsize.cy, h_png, 0, 0);
				else	// 为bottom时，已分割点为右边画
					m_pSpr->AlphaBlend(h_bk, m_split - srcsize.cx, (bksize.cy - srcsize.cy)/2, srcsize.cx, srcsize.cy, h_png, 0, 0);

				// 图片为top时，顶着分割点画
				if(m_txtalign == ALIGN_TOP)
					m_pSpr->AlphaBlend(h_bk, m_split, (bksize.cy - m_textsize[statue])/2, wide, m_textsize[statue], h_str, 0, 0);
				else if(m_txtalign == ALIGN_MIDDLE)	// 为居中时，在分割点和最右边间画
					m_pSpr->AlphaBlend(h_bk, m_split + (bksize.cx - m_split- wide)/2, (bksize.cy - m_textsize[statue])/2, wide, m_textsize[statue], h_str, 0, 0);
				else
					m_pSpr->AlphaBlend(h_bk, bksize.cx - wide, (bksize.cy - m_textsize[statue])/2, wide, m_textsize[statue], h_str, 0, 0);
			}
		}
		m_pSpr->CloseBlk(h_png);
		m_pSpr->CloseBlk(h_str);
	}
	else if(h_png != NULL)
	{
		m_pSpr->BitBlt(h_bk, (bksize.cx - srcsize.cx)/2, (bksize.cy - srcsize.cy)/2,
						srcsize.cx, srcsize.cy, h_png, 0, 0);
		m_pSpr->CloseBlk(h_png);
	}
	else if(h_str != NULL)
	{
		m_pSpr->AlphaBlend(h_bk, (bksize.cx - wide)/2, (bksize.cy - m_textsize[statue])/2,
						wide, m_textsize[statue], h_str, 0, 0);
		m_pSpr->CloseBlk(h_str);
	}

	if(m_postprocess[statue] != CSC_ORG)
	{
		h_png = DrawCscBmp(h_bk, m_postprocess[statue], &bksize);
		m_pSpr->CloseBlk(h_bk);
	}
	else
		h_png = h_bk;

	h_bk = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(h_bk, 0, 0, m_width, m_height, m_hFrameBak, 0, 0);

	if(m_isAlpha)
		m_pSpr->AlphaBlend(h_bk, (m_width - bksize.cx)/2, (m_height - bksize.cy)/2,
						bksize.cx, bksize.cy, h_png, 0, 0);
	else
		m_pSpr->BitBlt(h_bk, (m_width - bksize.cx)/2, (m_height - bksize.cy)/2,
						bksize.cx, bksize.cy, h_png, 0, 0);
	m_pSpr->CloseBlk(h_png);


	m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, h_bk, 0, 0);
	m_pSpr->CloseBlk(h_bk);
}

