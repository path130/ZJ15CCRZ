#pragma once

#include "roomlib.h"
#include "dpgraphic.h"
#include "input.h"

#define	MAX_CTL_ITEM			68
#define	MAX_LAYER_FRAME			10

typedef struct
{
	char name[32];
	DWORD property;
	DWORD IdBegin;
	HANDLE hCtrl;
} CtrlList;

class ContentManage
{
public:
	ContentManage()
	{
		count = 0;
	}
	~ContentManage(){}
	void Init(char* psrc)
	{
		ExtractParam(psrc);
	}
	char* FindContentByName(const char* name)
	{
		DWORD i;
		for(i = 0; i < count; i++)
		{
			if(strcmp(pname[i], name) == 0)
				return pcontent[i];
		}
		return NULL;
	}
private:
	void ExtractParam(char* psrc)
	{
		count = 0;
		while(1)
		{
			if((psrc = strchr(psrc, '"')) == NULL)
				break;
			psrc++;
			pname[count] = psrc;
			if((psrc = strchr(psrc, '=')) == NULL)
				break;
			*psrc = 0;
			psrc++;
			pcontent[count] = psrc;
			if((psrc = strchr(psrc, '"')) == NULL)
				break;
			*psrc = 0;
			psrc++;
			count++;
		}
	}
	
	DWORD count;
	char* pname[32];
	char* pcontent[32];
};

class CAppBase;

class CLayOut
{
public:
	CLayOut(CAppBase* pHwnd);
	~CLayOut(void);
	BOOL InitLay(char* pbuf);
	void DeinitLay(void);
	CAppBase* GetAppBase() const { return m_pWin; }
	void ResumeAck(void);
	void PauseAck(void);
	void ShowLay();
	void HideLay();
	BOOL IsHide();
	void ActivateCtrl(DWORD left, DWORD top, DWORD width, DWORD height , HANDLE hctrl);
	void DeActivate(HANDLE hctrl);

	// 注册控件
	void RegisterCtrl(DWORD type, char* name, HANDLE, DWORD m_msgid);
	// 注册触摸屏响应
	void RegisterEvent(DWORD left, DWORD top, DWORD width, DWORD height, HANDLE hctrl);
	void RegisterJustEvent(DWORD left, DWORD top, DWORD width, DWORD height, HANDLE hctrl);
	// 注册按键响应
	void RegisterKeyEvent(int but, HANDLE hctl);
	// 注册时间相应
	void RegisterTickEvent(HANDLE hctl);
	// 获取控件的句柄
	HANDLE GetCtrlByName(char* name, DWORD* idmap = NULL);
	// 申请控件的ID
	DWORD RequestMsgId(void);
	// 获取图层名称
	char* GetName(void);
	// 显示或隐藏图层
	void SwitchLay(BOOL isShow);
	void DoTickProcess(void);

	CDPGraphic* m_pSpr;				// 精灵支持
	HANDLE m_frame;					// 图层对应的硬件frame
	DWORD m_width;					// 图层宽度
	DWORD m_height;					// 图层高度
	DWORD m_left;					// 图层在屏幕上的x方向起始点
	DWORD m_top;					// 图层在屏幕上的y方向的起始点
	DWORD m_isHide;					// 图层是否隐藏,缺省为FALSE,显示
private:
	// 用于初始化背景
	void InitListFrame(ContentManage* pCm);
	BOOL InitBkFrame(ContentManage* pCm);
	void InitPartFrame(ContentManage* pCm);
	void InitAlphaFrame(ContentManage* pCm);
	char name[32];				// 图层名称,缺省为mainlay

	CAppBase* m_pWin;				// 图层所在窗口指针
	DWORD m_layer;					// 图层所在硬件层 
	DWORD m_isUnack;				// 图层是否响应触摸屏

	KeybdMap m_KeyEvent[MAX_CTL_ITEM];	// 按键响应事件
	DWORD m_KeyCount;					// 按键响应事件个数
	EventMap CtlEvent[MAX_CTL_ITEM];	// 触摸屏响应事件
	DWORD m_CtlCount;					// 触摸屏响应时间个数
	HANDLE TickEvent[MAX_CTL_ITEM];
	DWORD m_TickCount;
	CtrlList m_CtrlList[MAX_CTL_ITEM];	// 图层内控件
	DWORD m_CtlListCount;				// 图层内控件个数
};

class CAppBase
{
public:
	CAppBase(DWORD pHwnd);
	virtual ~CAppBase(void);
public:
	// 消息处理函数
	virtual BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam) = 0;
	// 窗口销毁
	virtual BOOL Destroy(void);
	// 窗口暂停
	virtual BOOL DoPause(void);
	// 窗口继续
	virtual void DoResume(void);
	static void Show(void);
	// 窗口恢复响应
	void ResumeAck(void);
	// 窗口停止响应,但是在显示
	void PauseAck(void);
	DWORD RequestMsgId(void);

	DWORD GetWindownId(void)
	{
		return m_IdBase;
	}

	void DoTickProcess(void)
	{
		DWORD i;
		for(i = 0; i < m_layindex; i++)
			m_layer[i]->DoTickProcess();
	}

	void InitDisplay();
	BOOL InitFrame(const char* xmlname);
	HANDLE GetCtrlByName(const char* name, DWORD* begin = NULL);
	DWORD m_dwTimeout;		// 据上次操作间隔时间
	DWORD m_screenoff;		// 黑屏时间
	BOOL InitLayer(const char* xmlname);
	void DumpImage(char* filename);
public:
	int InitXmlFile(const char* xmlname, char** rpbuf);
	CLayOut* m_layer[MAX_LAYER_FRAME];
	DWORD m_layindex;		// 使用到的图层数量
	CDPGraphic* m_pSpr;
	DWORD m_IdBase;			// 窗口的ID
	DWORD m_IdOrg;			// 控件的ID,在窗口ID的基础上往上加
};

