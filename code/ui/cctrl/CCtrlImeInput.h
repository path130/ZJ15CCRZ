#pragma once
#include "CCtrlBase.h"
#define CURSOR_MOVE_NEXT		123456
#define CURSOR_MOVE_PREV		123455
typedef struct 
{
	RECT rt;
	WORD id;
	WORD type;
	char str[32];
} KEYUNIT;

enum
{
	ENGQ = 0,
	ENGW,
	ENGE,
	ENGR,
	ENGT,
	ENGY,
	ENGU,
	ENGI,
	ENGO,
	ENGP,
	ENGA,
	ENGS,
	ENGD,
	ENGF,
	ENGG,
	ENGH,
	ENGJ,
	ENGK,
	ENGL,
	ENGZ,
	ENGX,
	ENGC,
	ENGV,
	ENGB,
	ENGN,
	ENGM,
	PREV,	// 
	NEXT,
	BACK,
	ENTER,
	SPACE,
	UPPERCASE,
	NUM,
	LANG,
	SYMBOL,
	CHNPINYIN,
	CHN1,
	CHN2,
	CHN3,
	CHN4,
	CHN5,
	CHN6,
	UNITSIZE
};

#define	CHAR_KEY	2
#define	CTRL_KEY	1
#define	SHOW_KEY	3
enum
{
	ADDALPHA,
	DELALPHA,
	ADDPINYIN,
	DELPINYIN,
	EMPTYPINYIN,
	CLOSEAPP,
	HUICHE
};

#define	MODE_INIT		0
#define	MODE_CHINESE	1
#define	MODE_LENGLISH	2
#define	MODE_BENGLISH	3
#define	MODE_NUMBER		4
#define	MODE_SYMBOL		5
#define	MODE_MAX		6

#define	MAX_HANZI_SHOW		6
class CImeInput:public CCtrlBase
{
public:
	CImeInput(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_left = 0;
		m_top = 80;
		m_width = 800;
		m_height = 400;
		m_interval = 2;
		m_Mode = MODE_CHINESE;
		m_LastMode = MODE_INIT;
		m_charpng = NULL;
		m_ctrlpng = NULL;
		m_pressedpng = NULL;
		m_bakcolor = 0xff555d70;
		m_pinyincount = 0;
		m_wpinyininput[0] = 0;
		m_hanziptr = 0;
		m_hanzitotal = 0;
		memset(m_hanzi, 0, MAX_HANZI_SHOW * 2);
		memset(m_keyUnit, 0, sizeof(KEYUNIT) * UNITSIZE);
	}
	~CImeInput()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
		if(m_charpng != NULL)
		{
			m_pSpr->CloseBlk(m_charpng);
			m_charpng = NULL;
		}
		if(m_ctrlpng != NULL)
		{
			m_pSpr->CloseBlk(m_ctrlpng);
			m_ctrlpng = NULL;
		}
		if(m_pressedpng != NULL)
		{
			m_pSpr->CloseBlk(m_pressedpng);
			m_pressedpng = NULL;
		}
	}

	void SetWidth(const DWORD width)
	{
		m_width = width;
	}
	void SetHeight(const DWORD height)
	{
		m_height = height;
	}
	BOOL DoInit(ContentManage*);
	BOOL DoResponse(DWORD xoff, DWORD yoff, DWORD statue);
protected:
	HANDLE GetShowContent(DWORD lastmode, DWORD mode, DWORD btn, SIZE* size);
	BOOL InitGlobal(ContentManage* pCm);
	void AddNewItem(ContentManage* pCm);
	void DoAction(DWORD id);
	BOOL InitView(char* fname);
	void Show(DWORD status, DWORD id);
	KEYUNIT m_keyUnit[UNITSIZE];
	DWORD m_Mode;	  // true is chinese, false is english
	DWORD m_LastMode;
	DWORD m_interval;
	DWORD m_standardw;
	DWORD m_standardh;

	DWORD m_bakcolor;
	HANDLE m_charpng;
	SIZE m_charsize;
	HANDLE m_ctrlpng;
	SIZE m_ctrlsize;
	HANDLE m_pressedpng;
	SIZE m_presssize;

	DWORD m_lastId;

	char m_wpinyininput[8];
	char m_pinyincount;
	char m_hanzi[8];				//	4个字节保存一个汉字
	char* m_hanzimap;
	DWORD m_hanziptr;
	DWORD m_hanzitotal;
};

