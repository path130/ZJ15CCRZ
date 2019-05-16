#include "CCtrlEnKeyboard.h"

#define	STRING_SIZE		36

static char* m_ppng[MAXKEY] =
{
	// 第一行
	"kdq",
	"kdw",
	"kde",
	"kdr",
	"kdt",
	"kdy",
	"kdu",
	"kdi",
	"kdo",
	"kdp",
	// 第二行
	"kda",
	"kds",
	"kdd",
	"kdf",
	"kdg",
	"kdh",
	"kdj",
	"kdk",
	"kd",
	// 第三行
	"kdcap",
	"kdz",
	"kdx",
	"kdc",
	"kdv",
	"kdb",
	"kdn",
	"kdm",
	"kbd9",
	// 第四行
	"kdfh",
	"kd123",
	"kdcom",
	"kdem",
	"kbdc",
	"kden",
};

static char m_send[MAXKEY][3]=
{
	'q', 'Q', '1',
	'w', 'W', '2',
	'e', 'E', '3',
	'r', 'R', '4',
	't', 'T', '5',
	'y', 'Y', '6',
	'u', 'U', '7',
	'i', 'I', '8',
	'o', 'O', '9',
	'p', 'P', '0',
	'a', 'A', 'a',
	's', 'S', 's',
	'd', 'D', 'd',
	'f', 'F', 'f',
	'g', 'G', 'g',
	'h', 'H', 'h',
	'j', 'J', 'j',
	'k', 'K', 'k',
	'l', 'L', 'l',
	'l', 'L', 'l',
	'z', 'Z', 'z',
	'x', 'X', 'x',
	'c', 'C', 'c',
	'v', 'V', 'v',
	'b', 'B', 'b',
	'n', 'N', 'n',
	'm', 'M', 'm',
	0x08, 0x08, 0x08,
	0x09, 0x09, 0x09,
	0x20, 0x20, 0x20,
	',', ',', ',',
	0x20, 0x20, 0x20,
	'.', '.', '.',
	0x0d, 0x0d, 0x0d,
};

static char* m_ppngdig[10] =
{
	// 第一行
	"kbd0",
	"kbd1",
	"kbd2",
	"kbd3",
	"kbd4",
	"kbd5",
	"kbd6",
	"kbd7",
	"kbd8",
	"kbda",
};
void CEnKeyboard::AddNewItem(DWORD i, ContentManage* pCm)
{
	char* pcontent;
	DWORD temp;

	if((pcontent = pCm->FindContentByName("left")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_Range[i].left = temp;

	if((pcontent = pCm->FindContentByName("top")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_Range[i].top = temp;

	if((pcontent = pCm->FindContentByName("width")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_Range[i].right = temp;

	if((pcontent = pCm->FindContentByName("height")) == NULL)
		return;
	temp = strtol(pcontent, NULL, 10);
	m_Range[i].bottom = temp;

	return;
}

BOOL CEnKeyboard::InitView(char* fname)
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

	pdesc = pbuf;
	i = 0;
	while(1)
	{
		pdesc = CFindContent(pdesc, "item", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		AddNewItem(i, &pCm);
		i++;
	}

	return TRUE;
}

BOOL CEnKeyboard::DoInit(ContentManage* pCm)
{
	char* pcontent;
	DWORD i;
	DWORD height;
	DWORD width;

	if((pcontent = pCm->FindContentByName("left")) == NULL)
		m_left = 0;
	else
		m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) == NULL)
		m_top = 80;
	else
		m_top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("itemview")) == NULL)
		return FALSE;
	if(!InitView(pcontent))
		return FALSE;
	// 第一行
	m_msgid = m_pLayer->RequestMsgId();
	height = 0;
	width = 0;
	for(i = 0; i < MAXKEY; i++)
	{
		if(height < (m_Range[i].top + m_Range[i].bottom))
			height = m_Range[i].top + m_Range[i].bottom;
		if(width < (m_Range[i].left + m_Range[i].right))
			width = m_Range[i].left + m_Range[i].right;
	}
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_ENKEYBOARD, "enkeyboard", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_ENKEYBOARD, pcontent, this, m_msgid);
	m_pLayer->RegisterEvent(m_left, m_top, width, height, this);
	for(i = 0; i < MAXKEY; i++)
		Show(STATUS_NORMAL, i);
	return TRUE;
}

BOOL CEnKeyboard::DoResponse(DWORD xoff, DWORD yoff, DWORD status)
{
	int i;

	for(i = 0; i < MAXKEY; ++i)
	{
		if(xoff >= (DWORD)m_Range[i].left
			&& xoff < ((DWORD)m_Range[i].left + m_Range[i].right)
			&& yoff >= (DWORD)m_Range[i].top
			&& yoff < (DWORD)(m_Range[i].top + m_Range[i].bottom))
			break;
	}
	if(i == MAXKEY)
		return FALSE;
	switch(status)
	{
	case TOUCH_DOWN:
		Show(STATUS_PRESSED, i);
		m_lastId = i;
		break;
	case TOUCH_UP:
		//			Show(STATUS_NORMAL,i);
		if (m_lastId == i)
			DoAction(i);  
		break;
	case TOUCH_VALID:
		if(i != m_lastId)
			Show(STATUS_NORMAL,m_lastId);
		break;
	case TOUCH_MOVEOUT:
		Show(STATUS_NORMAL,m_lastId);
		break;
	}
	return TRUE;
}

void CEnKeyboard::Show(DWORD status, DWORD id)
{
	HANDLE htemp;
	DWORD width;
	DWORD height;
	SIZE size;
	HANDLE hbak;
	char* psrcname;
	char srcname[32];

	width = m_Range[id].right;
	height = m_Range[id].bottom;
	hbak = m_pSpr->ReqTempBlk(width, height);
	if(status == STATUS_PRESSED)
		htemp = m_pSpr->LoadImage("kbtbkp.png", &size);
	else
		htemp = m_pSpr->LoadImage("kbtbk.png", &size);
	m_pSpr->BitBlt(hbak, 0, 0, width/2, height/2, htemp, 0, 0);
	m_pSpr->BitBlt(hbak, width/2, 0, width - width/2, height/2, htemp, size.cx - (width - width/2), 0);
	m_pSpr->BitBlt(hbak, 0, height/2, width/2, height - height/2, htemp, 0, size.cy - (height - height/2));
	m_pSpr->BitBlt(hbak, width/2, height/2, width - width/2, height - height/2, htemp, size.cx - (width - width/2), size.cy - (height - height/2));
	m_pSpr->CloseBlk(htemp);
	if(id < 10)
	{
		if(m_bDigigal)
			psrcname = m_ppngdig[id];
		else
			psrcname = m_ppng[id];
	}
	else
		psrcname = m_ppng[id];
	if(status == STATUS_PRESSED)
		sprintf(srcname, "%sp.png", psrcname);
	else
		sprintf(srcname, "%s.png", psrcname);
	htemp = m_pSpr->LoadImage(srcname, &size);
	m_pSpr->BitBlt(hbak, (width - size.cx)/2, (height - size.cy)/2, size.cx, size.cy, htemp, 0, 0);
	m_pSpr->CloseBlk(htemp);
	m_pSpr->BitBlt(m_pLayer->m_frame, m_left + m_Range[id].left, m_top + m_Range[id].top, 
		width, height, hbak, 0, 0);
	m_pSpr->CloseBlk(hbak);
}

void CEnKeyboard::ShowFirstLine(void)
{
	Show(STATUS_NORMAL, 0);
	Show(STATUS_NORMAL, 1);
	Show(STATUS_NORMAL, 2);
	Show(STATUS_NORMAL, 3);
	Show(STATUS_NORMAL, 4);
	Show(STATUS_NORMAL, 5);
	Show(STATUS_NORMAL, 6);
	Show(STATUS_NORMAL, 7);
	Show(STATUS_NORMAL, 8);
	Show(STATUS_NORMAL, 9);
}

void CEnKeyboard::DoAction(DWORD id)
{
	if(id == 19)
	{
		// 大小写
		if(m_bBig)
		{
			m_bBig = FALSE;
			Show(STATUS_NORMAL, id);
		}
		else
		{
			m_bBig = TRUE;
			Show(STATUS_PRESSED, id);
		}
		if(m_bDigigal)
		{
			m_bDigigal = FALSE;
			ShowFirstLine();
		}
	}
	else if(id == 29)
	{
		// 数字
		if(m_bDigigal)
		{
			m_bDigigal = FALSE;
			Show(STATUS_NORMAL, id);
			ShowFirstLine();
		}
		else
		{
			m_bDigigal = TRUE;
			Show(STATUS_PRESSED, id);
			ShowFirstLine();
		}
		if(m_bBig)
		{
			m_bBig = FALSE;
			Show(STATUS_NORMAL, 19);
		}
	}
	else
	{
		if(m_bDigigal)
			DPPostMessage(KBD_MESSAGE, KBD_CTRL, (DWORD)m_send[id][2], 0);
		else if(m_bBig)
			DPPostMessage(KBD_MESSAGE, KBD_CTRL, (DWORD)m_send[id][1], 0);
		else
			DPPostMessage(KBD_MESSAGE, KBD_CTRL, (DWORD)m_send[id][0], 0);
		Show(STATUS_NORMAL,id);
	}
}