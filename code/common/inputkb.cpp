#include "roomlib.h"
#include "input.h"

#define	MAX_KMAP		10
typedef struct
{
	KeybdMap* pevent;
	DWORD count;
} KeyList;
void  DoKeybdOp(KeybdMap* pEvent, DWORD mmap, DWORD statue);
static KeyList sKeybdMap[MAX_KMAP];
static DWORD KeybdTotal = 0;

void KeyboardPreprocess(SYS_MSG* pmsg)
{
	DWORD i, j;

	if(pmsg->wParam == 0)
		PlayWav(KEYPAD_INDEX, GetRingVol());

	DPPostMessage(HARDKBD_MESSAGE, pmsg->wParam, pmsg->lParam, 0);
	printf("kbd %d %d\r\n", pmsg->wParam, pmsg->lParam);
	for(i = 0; i < KeybdTotal; i++)
	{
		for(j = 0; j < sKeybdMap[i].count; j++)
		{
			if(sKeybdMap[i].pevent[j].value == (int)pmsg->lParam)
			{
				DoKeybdOp(&sKeybdMap[i].pevent[j], pmsg->lParam, pmsg->wParam);
			}
		}
	}
}

void RegKeyboardMap(DWORD eventcout, KeybdMap* pMap)
{
	DWORD i;
	if(eventcout == 0)
		return;
	if(KeybdTotal < MAX_KMAP)
	{
		for(i = 0; i < KeybdTotal; i++)
		{
			if((sKeybdMap[i].pevent == pMap)
				&& (sKeybdMap[i].count == eventcout))
				return;
		}
		sKeybdMap[KeybdTotal].pevent = pMap;
		sKeybdMap[KeybdTotal].count = eventcout;
		KeybdTotal++;
	}
}

void UnregKeyboardMap(DWORD eventcout, KeybdMap* pMap)
{
	DWORD i;
	if(eventcout == 0)
		return;
	for(i = 0; i < KeybdTotal; i++)
	{
		if((sKeybdMap[i].pevent == pMap)
			&& (sKeybdMap[i].count == eventcout))
		{
			memmove(&sKeybdMap[i], &sKeybdMap[i + 1], (KeybdTotal - 1) * sizeof(KeyList));
			KeybdTotal -= 1;
			return;
		}
	}
	DBGMSG(DPWARNING, "UnregKeyboardMap Dont find %d\r\n", eventcout);
}

void ClrKeyboardMap(void)
{
	memset(sKeybdMap, 0, sizeof(KeyList) * MAX_KMAP);
	KeybdTotal = 0;
}

