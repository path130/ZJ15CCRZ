#include "CCtrlImeInput.h"
#include "LoadPinyin.h"

#define	STRING_SIZE		36
typedef struct
{
	DWORD index;
	DWORD type;
	char* match;
} keymap;

static keymap g_map[UNITSIZE] =
{
	{CHNPINYIN, SHOW_KEY, "chinput"},
	{CHN1, SHOW_KEY, "chn1"},
	{CHN2, SHOW_KEY, "chn2"},
	{CHN3, SHOW_KEY, "chn3"},
	{CHN4, SHOW_KEY, "chn4"},
	{CHN5, SHOW_KEY, "chn5"},
	{CHN6, SHOW_KEY, "chn6"},
	{ENGA, CHAR_KEY, "a"},
	{ENGB, CHAR_KEY, "b"},
	{ENGC, CHAR_KEY, "c"},
	{ENGD, CHAR_KEY, "d"},
	{ENGE, CHAR_KEY, "e"},
	{ENGF, CHAR_KEY, "f"},
	{ENGG, CHAR_KEY, "g"},
	{ENGH, CHAR_KEY, "h"},
	{ENGI, CHAR_KEY, "i"},
	{ENGJ, CHAR_KEY, "j"},
	{ENGK, CHAR_KEY, "k"},
	{ENGL, CHAR_KEY, "l"},
	{ENGM, CHAR_KEY, "m"},
	{ENGN, CHAR_KEY, "n"},
	{ENGO, CHAR_KEY, "o"},
	{ENGP, CHAR_KEY, "p"},
	{ENGQ, CHAR_KEY, "q"},
	{ENGR, CHAR_KEY, "r"},
	{ENGS, CHAR_KEY, "s"},
	{ENGT, CHAR_KEY, "t"},
	{ENGU, CHAR_KEY, "u"},
	{ENGV, CHAR_KEY, "v"},
	{ENGW, CHAR_KEY, "w"},
	{ENGX, CHAR_KEY, "x"},
	{ENGY, CHAR_KEY, "y"},
	{ENGZ, CHAR_KEY, "z"},
	{SPACE, CHAR_KEY, "space"},
	{PREV, CTRL_KEY, "prev"},
	{NEXT, CTRL_KEY, "next"},
	{UPPERCASE, CTRL_KEY, "cap"},
	{BACK, CTRL_KEY, "back"},
	{NUM, CTRL_KEY, "num"},
	{LANG, CTRL_KEY, "lang"},
	{SYMBOL, CTRL_KEY, "symbol"},
	{ENTER, CTRL_KEY, "enter"},
};

char tstr[MODE_MAX * PREV] =
{
	0, 'q', 'q', 'Q', '1', '~',
	0, 'w', 'w', 'W', '2', '`',
	0, 'e', 'e', 'E', '3', '!',
	0, 'r', 'r', 'R', '4', '@',
	0, 't', 't', 'T', '5', '#',
	0, 'y', 'y', 'Y', '6', '$',
	0, 'u', 'u', 'U', '7', '%',
	0, 'i', 'i', 'I', '8', '^',
	0, 'o', 'o', 'O', '9', '&',
	0, 'p', 'p', 'P', '0', '*',
	0, 'a', 'a', 'A', '-', '(',
	0, 's', 's', 'S', '_', ')',
	0, 'd', 'd', 'D', ',', '{',
	0, 'f', 'f', 'F', '.', '}',
	0, 'g', 'g', 'G', '?', '[',
	0, 'h', 'h', 'H', '/', ']',
	0, 'j', 'j', 'J', 'j', ':',
	0, 'k', 'k', 'K', 'k', ';',
	0, 'l', 'l', 'L', 'l', '"',
	0, 'z', 'z', 'Z', 'z', '\'',
	0, 'x', 'x', 'X', 'x', '<',
	0, 'c', 'c', 'C', 'c', '>',
	0, 'v', 'v', 'V', 'v', '|',
	0, 'b', 'b', 'B', 'b', '\\',
	0, 'n', 'n', 'N', 'n', '+',
	0, 'm', 'm', 'M', 'm', '=',
};

BOOL CImeInput::InitGlobal(ContentManage* pCm)
{
	char* pcontent;
	HANDLE htemp;

	// 标准尺寸大小
	if((pcontent = pCm->FindContentByName("width")) == NULL)
		return FALSE;
	m_standardw = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("height")) == NULL)
		return FALSE;
	m_standardh = strtol(pcontent, NULL, 10);

	// 2中按键，功能按键和字母按键的图片，按下状态的图片
	if((pcontent = pCm->FindContentByName("interval")) != NULL)
		m_interval = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("bakcolor")) != NULL)
		m_bakcolor = hexconvert(pcontent);

	if((pcontent = pCm->FindContentByName("charnormalpng")) != NULL)
	{
		htemp = m_pSpr->LoadImage(pcontent, &m_charsize);
		m_charpng = m_pSpr->ReqTempBlk(m_charsize.cx, m_charsize.cy);
		m_pSpr->SetBK(m_charpng, m_bakcolor);
		m_pSpr->AlphaBlend(m_charpng, 0, 0, m_charsize.cx, m_charsize.cy, htemp, 0, 0);
		m_pSpr->CloseBlk(htemp);
	}
	else
	{
		m_charsize.cx = 180;
		m_charsize.cy = 60;
		m_charpng = m_pSpr->ReqTempBlk(m_charsize.cx, m_charsize.cy);
		m_pSpr->InitBakRect(m_charpng, 0xffffffff, 0xffc0c0c0);
	}
	
	if((pcontent = pCm->FindContentByName("ctrlnormalpng")) != NULL)
	{
		htemp = m_pSpr->LoadImage(pcontent, &m_ctrlsize);
		m_ctrlpng = m_pSpr->ReqTempBlk(m_ctrlsize.cx, m_ctrlsize.cy);
		m_pSpr->SetBK(m_ctrlpng, m_bakcolor);
		m_pSpr->AlphaBlend(m_ctrlpng, 0, 0, m_ctrlsize.cx, m_ctrlsize.cy, htemp, 0, 0);
		m_pSpr->CloseBlk(htemp);
	}
	else
	{
		m_ctrlsize.cx = 180;
		m_ctrlsize.cy = 60;
		m_ctrlpng = m_pSpr->ReqTempBlk(m_ctrlsize.cx, m_ctrlsize.cy);
		m_pSpr->InitBakRect(m_ctrlpng, 0xffffffff, 0xff808080);
	}

	if((pcontent = pCm->FindContentByName("presspng")) != NULL)
	{
		htemp = m_pSpr->LoadImage(pcontent, &m_presssize);
		m_pressedpng = m_pSpr->ReqTempBlk(m_presssize.cx, m_presssize.cy);
		m_pSpr->SetBK(m_pressedpng, m_bakcolor);
		m_pSpr->AlphaBlend(m_pressedpng, 0, 0, m_presssize.cx, m_presssize.cy, htemp, 0, 0);
		m_pSpr->CloseBlk(htemp);
	}
	else
	{
		m_presssize.cx = 180;
		m_presssize.cy = 60;
		m_pressedpng = m_pSpr->ReqTempBlk(m_presssize.cx, m_presssize.cy);
		m_pSpr->InitBakRect(m_pressedpng, 0xffffffff, 0xff404040);
	}
	return TRUE;
}

void CImeInput::AddNewItem(ContentManage* pCm)
{
	char* pcontent;
	DWORD m_curptr;
	DWORD i;
	DWORD temp;

	if((pcontent = pCm->FindContentByName("id")) == NULL)
		return;
	for(i = 0; i < UNITSIZE; i++)
	{
		if(strcmp(pcontent, g_map[i].match) == 0)
			break;
	}
	if(i == UNITSIZE)
		return;
	m_curptr = g_map[i].index;
	m_keyUnit[m_curptr].type = (WORD)g_map[i].type;
	if((pcontent = pCm->FindContentByName("left")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_keyUnit[m_curptr].rt.left = temp * m_width/m_standardw;

	if((pcontent = pCm->FindContentByName("top")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_keyUnit[m_curptr].rt.top = temp * m_height/m_standardh;

	if((pcontent = pCm->FindContentByName("width")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_keyUnit[m_curptr].rt.right = temp * m_width/m_standardw;

	if((pcontent = pCm->FindContentByName("height")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_keyUnit[m_curptr].rt.bottom = temp * m_height/m_standardh;

	return;
}

BOOL CImeInput::InitView(char* fname)
{
	char temp[512];
	DWORD flen;
	char* pbuf;
	DWORD i;
	ContentManage pCm;
	char* pdesc;

	sprintf(temp, "%s/%s", LAYOUT_DIR, fname);
	flen = BReadFile(temp, &pbuf);
	if(flen == 0)
	{
		return FALSE;
	}
	flen/=2;

	for(i = 0; i < flen; i++)
	{
		if((pbuf[i] >= 0x41)
			&& (pbuf[i] <= 0x5a))
			pbuf[i] = pbuf[i] | 0x20;
	}

	if(CFindContent(pbuf, "frame", temp, FALSE) == NULL)
	{
		free(pbuf);
		return FALSE;
	}
	pCm.Init(temp);
	if(!InitGlobal(&pCm))
	{
		free(pbuf);
		return FALSE;
	}

	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, "item", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		AddNewItem(&pCm);
	}

	return TRUE;
}

BOOL CImeInput::DoInit(ContentManage* pCm)
{
	char* pcontent;
	DWORD width;
	DWORD height;
	DWORD i;

	if((pcontent = pCm->FindContentByName("left")) == NULL)
		m_left = 0;
	else
		m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) == NULL)
		m_top = 80;
	else
		m_top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("width")) == NULL)
		m_width = 800;
	else
		m_width = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("height")) == NULL)
		m_height = 400;
	else
		m_height = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("itemview")) == NULL)
		return FALSE;
	if(!InitView(pcontent))
		return FALSE;
	// 第一行
	width = m_width / 10;
	height = m_height / 5;

	m_hFrameBak = m_pSpr->ReqTempBlk(m_width,m_height);

	m_msgid = m_pLayer->RequestMsgId();
	
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_IMEINPUT, "imeinput", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_IMEINPUT, pcontent, this, m_msgid);
	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	for(i = 0; i < UNITSIZE; i++)
		Show(STATUS_NORMAL, i);
	return TRUE;
}

HANDLE CImeInput::GetShowContent(DWORD lastmode, DWORD mode, DWORD btn, SIZE* size)
{
	HANDLE hpng = NULL;
	DWORD wide;
	if(btn < PREV)
	{
		char drawstr[2];
		drawstr[0] = tstr[MODE_MAX * btn + mode];
		drawstr[1] = 0;
		hpng = m_pSpr->LoadStr(drawstr, STRING_SIZE, 0, &wide);
		size->cx = wide;
		size->cy = STRING_SIZE;
	}
	else if(btn < UPPERCASE)
	{
		switch(btn)
		{
			case PREV:
				hpng = m_pSpr->LoadImage("kprev.png", size);
				break;
			case NEXT:
				hpng = m_pSpr->LoadImage("knext.png", size);
				break;
			case BACK:
				hpng = m_pSpr->LoadImage("kback.png", size);
				break;
			case ENTER:
				hpng = m_pSpr->LoadImage("kenter.png", size);
				break;
			case SPACE:
				break;
		}
	}
	else if(btn == UPPERCASE)
	{
		if(mode == MODE_BENGLISH)
			hpng = m_pSpr->LoadImage("kbeng.png", size);
		else
			hpng = m_pSpr->LoadImage("kleng.png", size);
	}
	else if(btn == NUM)
	{
		if(mode == MODE_NUMBER)
			hpng = m_pSpr->LoadStr("123", STRING_SIZE, 0xc04040, &wide);
		else
			hpng = m_pSpr->LoadStr("123", STRING_SIZE, 0, &wide);
		size->cx = wide;
		size->cy = STRING_SIZE;
	}
	else if(btn == LANG)
	{
		if(mode == MODE_CHINESE)
			hpng = m_pSpr->LoadStr(GetStringByID(1046), STRING_SIZE, 0, &wide);
		else
			hpng = m_pSpr->LoadStr(GetStringByID(1048), STRING_SIZE, 0, &wide);
		size->cx = wide;
		size->cy = STRING_SIZE;
	}
	else if(btn == SYMBOL)
	{
		if(mode == MODE_SYMBOL)
			hpng = m_pSpr->LoadStr(GetStringByID(1047), STRING_SIZE, 0xc04040, &wide);
		else
			hpng = m_pSpr->LoadStr(GetStringByID(1047), STRING_SIZE, 0, &wide);
		size->cx = wide;
		size->cy = STRING_SIZE;
	}
	else if(btn == CHNPINYIN)
	{
		if(strlen(m_wpinyininput) != 0)
		{
			hpng = m_pSpr->LoadStr(m_wpinyininput, STRING_SIZE, 0x0, &wide);
			size->cx = wide;
			size->cy = STRING_SIZE;
		}
	}
	else if(btn < UNITSIZE)
	{
		btn -= CHN1;
		if(m_hanzi[btn] != 0)
		{
			char hanzi[2];
			hanzi[0] = m_hanzi[btn];
			hanzi[1] = 0;
			hpng = m_pSpr->LoadStr(hanzi, STRING_SIZE, 0, &wide);
			size->cx = wide;
			size->cy = STRING_SIZE;
		}
	}
	return hpng;
}

BOOL CImeInput::DoResponse(DWORD xoff, DWORD yoff, DWORD status)
{
	int i;

	for(i = 0; i < UNITSIZE; ++i)
	{
		if(xoff >= (DWORD)m_keyUnit[i].rt.left
			&& xoff < ((DWORD)m_keyUnit[i].rt.left + m_keyUnit[i].rt.right)
			&& yoff >= (DWORD)m_keyUnit[i].rt.top
			&& yoff < (DWORD)(m_keyUnit[i].rt.top + m_keyUnit[i].rt.bottom))
			break;
	}
	if(i == UNITSIZE)
		return FALSE;
	switch(status)
	{
		case TOUCH_DOWN:
			Show(STATUS_PRESSED, i);
			m_lastId = i;
			break;
		case TOUCH_UP:
			Show(STATUS_NORMAL,i);
			if (m_lastId == i)
				DoAction(i);  
			break;
		case TOUCH_VALID:
			if(i != m_lastId)
				Show(STATUS_NORMAL,m_lastId);
			break;
		case TOUCH_MOVEOUT:
			Show(STATUS_NORMAL,i);
			break;
	}

	return TRUE;
}

void CImeInput::Show(DWORD status, DWORD id)
{
	HANDLE htemp;
	HANDLE hcontent;
	DWORD width;
	DWORD height;
	SIZE size;
	HANDLE hbak;
	SIZE baksize;

	if(m_keyUnit[id].type == 0)
		return;
	if(status == STATUS_PRESSED)
	{
		hbak = m_pressedpng;
		baksize.cx = m_presssize.cx;
		baksize.cy = m_presssize.cy;
	}
	else
	{
		if(m_keyUnit[id].type == CHAR_KEY)
		{
			hbak = m_charpng;
			baksize.cx = m_charsize.cx;
			baksize.cy = m_charsize.cy;
		}
		else
		{
			hbak = m_ctrlpng;
			baksize.cx = m_ctrlsize.cx;
			baksize.cy = m_ctrlsize.cy;
		}
	}
	width = m_keyUnit[id].rt.right;
	height = m_keyUnit[id].rt.bottom;
	htemp = m_pSpr->ReqTempBlk(m_keyUnit[id].rt.right, m_keyUnit[id].rt.bottom);
	m_pSpr->SetBK(htemp, m_bakcolor);
	if(m_keyUnit[id].type == SHOW_KEY)
	{
		RECT rect;
		rect.left = 0;
		rect.top = m_interval;
		rect.right = width;
		rect.bottom = height - m_interval * 3;
		if(id == CHNPINYIN)
			m_pSpr->FillSolidRect(htemp, &rect, 0xffc0c0c0);
		else
		{
			m_pSpr->FillSolidRect(htemp, &rect, 0xfff0f0f0);
		}
	}
	else
	{
		m_pSpr->BitBlt(htemp, m_interval, m_interval, 
							width/2 - m_interval, height/2 - m_interval, 
							hbak, 0, 0);
		m_pSpr->BitBlt(htemp, width/2, m_interval, 
							width - width/2  - m_interval, height/2 - m_interval, 
							hbak, baksize.cx - (width - width/2  - m_interval), 0);
		m_pSpr->BitBlt(htemp, m_interval, height/2, 
							width/2 - m_interval, height - height/2 - m_interval, 
							hbak, 0, baksize.cy - (height - height/2 - m_interval));
		m_pSpr->BitBlt(htemp, width/2, height/2,
							width - width/2  - m_interval, height - height/2 - m_interval,
							hbak, baksize.cx - (width - width/2  - m_interval), baksize.cy - (height - height/2 - m_interval));
	}
	hcontent = GetShowContent(m_LastMode, m_Mode, id, &size);
	if(hcontent != NULL)
	{
		if(id == CHNPINYIN)
			m_pSpr->AlphaBlend(htemp, 10, (height - size.cy)/2,
						size.cx, size.cy, hcontent, 0, 0);
		else
			m_pSpr->AlphaBlend(htemp, (width - size.cx)/2, (height - size.cy)/2,
						size.cx, size.cy, hcontent, 0, 0);
		m_pSpr->CloseBlk(hcontent);
	}
	m_pSpr->BitBlt(m_pLayer->m_frame, m_keyUnit[id].rt.left + m_left, m_keyUnit[id].rt.top + m_top,
					width, height, htemp, 0, 0);
	m_pSpr->CloseBlk(htemp);
}

void CImeInput::DoAction(DWORD id)
{
	DWORD i;
	if(id < PREV)
	{
		char val = tstr[MODE_MAX * id + m_Mode];
		if(m_Mode == MODE_CHINESE)
		{
			if(m_pinyincount < 6)
			{
				m_wpinyininput[m_pinyincount] = val;
				m_pinyincount++;
				m_wpinyininput[m_pinyincount] = 0;
				m_hanzimap = GetStringByPinyin(m_wpinyininput);
				memset(m_hanzi, 0, 16);
				if(m_hanzimap != NULL)
				{
					m_hanziptr = 0;
					m_hanzitotal = strlen(m_hanzimap);
					strncpy(m_hanzi,m_hanzimap, 6);
				}
				else
				{
					m_hanzitotal = 0;
					m_hanziptr = 0;
				}
				for(i = CHNPINYIN; i < UNITSIZE; i++)
					Show(STATUS_NORMAL, i);
			}
		}
		else
		{
			DPPostMessage(KBD_MESSAGE, KBD_CTRL, val, 0);
		}
	}
	else if(id == BACK)
	{
		if(m_Mode == MODE_CHINESE)
		{
			if(m_pinyincount != 0)
			{
				m_pinyincount--;
				m_wpinyininput[m_pinyincount] = 0;
				if(m_pinyincount != 0)
				{
					m_hanzimap = GetStringByPinyin(m_wpinyininput);
					memset(m_hanzi, 0, MAX_HANZI_SHOW * 2);
					if(m_hanzimap != NULL)
					{
						m_hanziptr = 0;
						m_hanzitotal = strlen(m_hanzimap);
						strncpy(m_hanzi,m_hanzimap, MAX_HANZI_SHOW);
					}
				}
				else
					memset(m_hanzi, 0, MAX_HANZI_SHOW * 2);
				for(i = CHNPINYIN; i < UNITSIZE; i++)
					Show(STATUS_NORMAL, i);
			}
			else
				DPPostMessage(KBD_MESSAGE, KBD_CTRL, 0x08, 0);
		}
		else
			DPPostMessage(KBD_MESSAGE, KBD_CTRL, 0x08, 0);
	}
	else if(id == NEXT)
	{
		if(m_Mode == MODE_CHINESE)
		{
			if(((m_hanziptr + MAX_HANZI_SHOW) < m_hanzitotal) && 
				m_hanzitotal !=0)
			{
				memset(m_hanzi, 0, MAX_HANZI_SHOW * 2);
				m_hanziptr += MAX_HANZI_SHOW;
				strncpy(m_hanzi, m_hanzimap + m_hanziptr, MAX_HANZI_SHOW);
				for(i = CHN1; i < UNITSIZE; i++)
					Show(STATUS_NORMAL, i);
			}
		}
//		else
//			PostMessage(KBD_MESSAGE , CURSOR_MOVE_NEXT , 0 , 0);
	}
	else if(id == PREV)
	{
		if(m_Mode == MODE_CHINESE)
		{
			if(m_hanziptr != 0 && m_hanzitotal !=0)
			{
				memset(m_hanzi, 0, MAX_HANZI_SHOW * 2);
				m_hanziptr -= MAX_HANZI_SHOW;
				strncpy(m_hanzi, m_hanzimap + m_hanziptr, MAX_HANZI_SHOW);
				for(i = CHN1; i < UNITSIZE; i++)
					Show(STATUS_NORMAL, i);
			}
		}
//		else
//			PostMessage(KBD_MESSAGE , CURSOR_MOVE_NEXT, 0 , 0);
	}
	else if(id == ENTER)
		DPPostMessage(KBD_MESSAGE, KBD_CTRL, 0x0d, 0);
	else if(id == SPACE)
		DPPostMessage(KBD_MESSAGE, KBD_CTRL, 0x20, 0);
	else if((id >= CHN1) && (id <= CHN6))
	{
		id -= CHN1;
		if(m_hanzi[id] != 0)
		{
			DPPostMessage(KBD_MESSAGE, KBD_CTRL, m_hanzi[id], 0);
			m_pinyincount = 0;
			m_wpinyininput[0] = 0;
			memset(m_hanzi, 0, MAX_HANZI_SHOW * 2);
			m_hanziptr = 0;
			m_hanzitotal = 0;
			for(i = CHNPINYIN; i < UNITSIZE; i++)
				Show(STATUS_NORMAL, i);
		}
	}
	else if(id == UPPERCASE)
	{
		m_LastMode = m_Mode;
		if(m_LastMode == MODE_BENGLISH)
			m_Mode = MODE_LENGLISH;
		else
			m_Mode = MODE_BENGLISH;
		// 修改模式按键
		switch(m_LastMode)
		{
			case MODE_CHINESE:
				Show(STATUS_NORMAL, LANG);
				break;
			case MODE_NUMBER:
				Show(STATUS_NORMAL, NUM);
				break;
			case MODE_SYMBOL:
				Show(STATUS_NORMAL, SYMBOL);
				break;
		}
		Show(STATUS_NORMAL, UPPERCASE);
		// 字符按键全部更新
		for(i = 0; i < PREV; i++)
			Show(STATUS_NORMAL, i);
	}
	else if(NUM == id)
	{
		if(m_Mode != MODE_NUMBER)
		{
			m_LastMode = m_Mode;
			m_Mode = MODE_NUMBER;
			switch(m_LastMode)
			{
				case MODE_SYMBOL:
					Show(STATUS_NORMAL, SYMBOL);
					break;
			}
			Show(STATUS_NORMAL, NUM);
			for(i = 0; i < ENGJ; i++)
				Show(STATUS_NORMAL, i);
		}
		else
		{
			m_Mode = m_LastMode;
			m_LastMode = MODE_NUMBER;
			switch(m_Mode)
			{
				case MODE_SYMBOL:
					Show(STATUS_NORMAL, SYMBOL);
					break;
 				case MODE_CHINESE:
 					Show(STATUS_NORMAL , CHNPINYIN);
 					break;
			}
			Show(STATUS_NORMAL, NUM);
			for(i = 0; i < ENGJ; i++)
				Show(STATUS_NORMAL, i);
		}
	}
	else if(LANG == id)
	{
		m_LastMode = m_Mode;
		if(m_LastMode == MODE_CHINESE)
		{
			m_Mode = MODE_LENGLISH;
			Show(STATUS_NORMAL, LANG);
		}
		else
		{
			m_Mode = MODE_CHINESE;
			Show(STATUS_NORMAL, LANG);
			switch(m_LastMode)
			{
				case MODE_BENGLISH:
					for(i = 0; i < PREV; i++)
						Show(STATUS_NORMAL, i);
				case MODE_LENGLISH:
					Show(STATUS_NORMAL, LANG);
					break;
				case MODE_NUMBER:
					Show(STATUS_NORMAL, NUM);
					for(i = 0; i < PREV; i++)
						Show(STATUS_NORMAL, i);
					break;
				case MODE_SYMBOL:
					Show(STATUS_NORMAL, SYMBOL);
					for(i = 0; i < PREV; i++)
						Show(STATUS_NORMAL, i);
					break;
			}
		}
	}
	else if(SYMBOL == id)
	{
		if(m_Mode != MODE_SYMBOL)
		{
			m_LastMode = m_Mode;
			m_Mode = MODE_SYMBOL;
			switch(m_LastMode)
			{
				case MODE_NUMBER:
					Show(STATUS_NORMAL, NUM);
					break;
			}
			Show(STATUS_NORMAL, SYMBOL);
			for(i = 0; i < PREV; i++)
				Show(STATUS_NORMAL, i);
		}
		else
		{
			m_Mode = m_LastMode;
			m_LastMode = MODE_SYMBOL;
			switch(m_Mode)
			{
				case MODE_NUMBER:
					Show(STATUS_NORMAL, NUM);
					break;
			}
			Show(STATUS_NORMAL, SYMBOL);
			for(i = 0; i < PREV; i++)
				Show(STATUS_NORMAL, i);
		}
	}

	DPPostMessage(TOUCH_MESSAGE, m_msgid, 0, 0);
}

