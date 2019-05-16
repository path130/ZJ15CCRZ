#pragma once

#define	MIN_KEYVALUE	1
#define	MAX_KEYVALUE	5

#define	LONG_PRESSED	0x80
#define	BUTTON_1		1			
#define	BUTTON_2		2
#define	BUTTON_3		3
#define	BUTTON_4		4
#define	BUTTON_5		5


#define	BUTTON_LEFT			0
#define	BUTTON_UP			1
#define	BUTTON_RIGHT		2
#define	BUTTON_DOWN			3
#define	BUTTON_PREV			4
#define	BUTTON_NEXT			5
#define	BUTTON_ENTER		6
#define	BUTTON_PAGEUP		7
#define	BUTTON_PAGEDOWN		8
#define	BUTTON_MAX			9

typedef struct
{
	int value; 				// 按键值
	HANDLE pCtrl;			// 对应的控件
} KeybdMap;

void KeyboardPreprocess(SYS_MSG* pmsg);
void RegKeyboardMap(DWORD eventcout, KeybdMap* pMap);
void UnregKeyboardMap(DWORD eventcout, KeybdMap* pMap);
void ClrKeyboardMap(void);

typedef struct
{
	int xStart;
	int yStart;
	int xEnd;
	int yEnd;
	HANDLE pCtrl;
} EventMap;

BOOL InitTouchCalibrate(void);
void SaveTouchCalibrate(int *val);
void SetTouchDirect(BOOL val);
void RegEventRegion(DWORD eventcout, EventMap* pMap, DWORD);
void UnRegEventRegion(DWORD eventcout, EventMap* pMap);
void ClrEventRegion(void);
BOOL TouchEventRemap(SYS_MSG* pmsg);
BOOL TouchEventRemapEx(SYS_MSG* pmsg);


