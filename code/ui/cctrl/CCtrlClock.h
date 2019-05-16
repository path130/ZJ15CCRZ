#pragma once
#include "CCtrlBase.h"

class CTimeClock:public CCtrlBase
{
public:
	CTimeClock(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_hClockBk = NULL;
		m_hSpinHour = NULL;
		m_hSpinMinute = NULL;
		m_hSpinSecond = NULL;
		hTempSpriteHM = NULL;
		hTempSpriteHMS = NULL;
	}
	~CTimeClock()
	{
		if(m_hClockBk)
		{
			m_pSpr->CloseBlk(m_hClockBk);
			m_hClockBk = NULL;
		}
		if(m_hSpinHour)
		{
			m_pSpr->CloseBlk(m_hSpinHour);
			m_hSpinHour = NULL;
		}
		if(m_hSpinMinute)
		{
			m_pSpr->CloseBlk(m_hSpinMinute);
			m_hSpinMinute = NULL;
		}
		if(m_hSpinSecond)
		{
			m_pSpr->CloseBlk(m_hSpinSecond);
			m_hSpinSecond = NULL;
		}
		if(hTempSpriteHM)
		{
			m_pSpr->CloseBlk(hTempSpriteHM);
			hTempSpriteHM = NULL;
		}
		if(hTempSpriteHMS)
		{
			m_pSpr->CloseBlk(hTempSpriteHMS);
			hTempSpriteHMS = NULL;
		}
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}
	BOOL DoInit(ContentManage *);
	void UpdataDateTime(BOOL IsInit = FALSE);
private:
	DWORD & Pixels2(const Block_Req * pframe,const long x16,const long y16);
	void GetRect(RECT * pRect,int RotaryAngle,double ZoomX,double ZoomY,int rx,int ry,int move_x,int move_y);
	void GetRect(RECT * pRect,long Ax_16,long Ay_16,long Bx_16,long By_16,int rx,int ry,int move_x,int move_y);
	void Mix(RGBQUAD * pDst,RGBQUAD * pSrc,RGBQUAD * pSrc2);
	void PicRotary21(const Block_Req * Dst,const Block_Req * Src,int RotaryAngle,double rx,double ry,double move_x,double move_y);
	void DrawTime();
	DWORD& Pixels(const Block_Req * pframe,const long x,const long y)
	{
		return *(pframe->viraddr + y*pframe->win.cx + x);
	}
	
	DWORD* PixelsP(const Block_Req * pframe,const long x,const long y)
	{
		return pframe->viraddr + y*pframe->win.cx + x;
	}
	
	//判断一个点是否在图片中
	BOOL PixelsIsInPic(const Block_Req * pframe,const long x,const long y)
	{
		return ( (x>=0)&&(x<pframe->win.cx) && (y>=0)&&(y<pframe->win.cy));
	}
	
	void SinCos(const double Angle,double& sina,double& cosa) 
	{ 
		sina=sin(Angle);
		cosa=cos(Angle);
	} 

	HANDLE m_hClockBk;
	HANDLE m_hSpinHour;
	HANDLE m_hSpinMinute;
	HANDLE m_hSpinSecond;
	HANDLE hTempSpriteHM;
	HANDLE hTempSpriteHMS;

	DWORD move_x;
	DWORD move_y;
	POINT	Dot[3];
	SYSTEMTIME lastsystem;
};

