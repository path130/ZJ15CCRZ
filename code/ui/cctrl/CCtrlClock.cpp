#include "CCtrlClock.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define	M_P			0.0174532925
typedef struct 
{
	unsigned char  a,b,g,r;
}TARGB32;      //32 bit color

//那么访问一个点的函数可以写为：
DWORD & CTimeClock::Pixels2(const Block_Req * pframe,const long x16,const long y16)
{
	long x = (x16>>16);
	long y = (y16>>16);
	long long u = x16 - (x<<16);
	long long v = y16 - (y<<16);

	long pm0 = (65536-u)*(65536-v) >> 16;
	long pm1 = v*(65536-u) >> 16;
	long pm2 = u*(65536-v) >> 16;
	long pm3 = u*v >> 16;

	int x2,y2;
	if(x == pframe->win.cx - 1)
		x2 = x;
	else
		x2 = x + 1;
	if(y == pframe->win.cy - 1)
		y2 = y;
	else
		y2 = y + 1;

	TARGB32 * Src1 = (TARGB32*)PixelsP(pframe,x,y);
	TARGB32 * Src2 = (TARGB32*)PixelsP(pframe,x,y2);
	TARGB32 * Src3 = (TARGB32*)PixelsP(pframe,x2,y);
	TARGB32 * Src4 = (TARGB32*)PixelsP(pframe,x2,y2);

	TARGB32 Dst;
	Dst.a = (Src1->a*pm0 + Src2->a*pm1 + Src3->a*pm2 + Src4->a*pm3)>>16;
	Dst.r = (Src1->r*pm0 + Src2->r*pm1 + Src3->r*pm2 + Src4->r*pm3)>>16;
	Dst.g = (Src1->g*pm0 + Src2->g*pm1 + Src3->g*pm2 + Src4->g*pm3)>>16;
	Dst.b = (Src1->b*pm0 + Src2->b*pm1 + Src3->b*pm2 + Src4->b*pm3)>>16;

	return *(DWORD*)&Dst;
}

void CTimeClock::GetRect(RECT * pRect,int RotaryAngle,double ZoomX,double ZoomY,int rx,int ry,int move_x,int move_y)
{
//	DWORD time1 = GetTickCount();
	int dstx;
	int dsty;
	
	long sinA = sin(RotaryAngle*M_P)*(1<<16);
	long cosA = cos(RotaryAngle*M_P)*(1<<16);
	//旋转后 围绕点坐标
	long ttt = (rx + move_x)*(1<<16) - rx*cosA - ry*sinA;
	long ttt2 =(ry + move_y)*(1<<16) + rx*sinA - ry*cosA;

	long ZoomX1 = ZoomX*(1<<8);
	long ZoomY1 = ZoomY*(1<<8);
	POINT plt = {pRect->left,pRect->top};
	dstx = ((plt.x*cosA + plt.y*sinA + ttt)>>16) * ZoomX1;
	dsty = ((ttt2 - plt.x*sinA + plt.y*cosA)>>16) * ZoomY1;
	plt.x = dstx >> 8;
	plt.y = dsty >> 8;

	POINT plb = {pRect->left,pRect->bottom};
	dstx = ((plb.x*cosA + plb.y*sinA + ttt)>>16) * ZoomX1;
	dsty = ((ttt2 - plb.x*sinA + plb.y*cosA)>>16) * ZoomY1;
	plb.x = dstx >> 8;
	plb.y = dsty >> 8;

	POINT prt = {pRect->right,pRect->top};
	dstx = ((prt.x*cosA + prt.y*sinA + ttt)>>16) * ZoomX1;
	dsty = ((ttt2 - prt.x*sinA + prt.y*cosA)>>16) * ZoomY1;
	prt.x = dstx >> 8;
	prt.y = dsty >> 8;

	POINT prb = {pRect->right,pRect->bottom};
	dstx = ((prb.x*cosA + prb.y*sinA + ttt)>>16) * ZoomX1;
	dsty = ((ttt2 - prb.x*sinA + prb.y*cosA)>>16) * ZoomY1;
	prb.x = dstx >> 8;
	prb.y = dsty >> 8;

	int minX = min(plt.x,min(plb.x,min(prt.x,prb.x)));
	int minY = min(plt.y,min(plb.y,min(prt.y,prb.y)));
	int maxX = max(plt.x,max(plb.x,max(prt.x,prb.x)));
	int maxY = max(plt.y,max(plb.y,max(prt.y,prb.y)));

	pRect->left = minX;
	pRect->top = minY;
	pRect->bottom = maxY;
	pRect->right = maxX;
//	printf("RECT:%d,%d,%d,%d\n",pRect->left,pRect->top,pRect->right,pRect->bottom);
//	printf("GetRecttime1:%d\n",GetTickCount() -time1);

}

void CTimeClock::GetRect(RECT * pRect,long Ax_16,long Ay_16,long Bx_16,long By_16,int rx,int ry,int move_x,int move_y)
{
//	DWORD time1 = GetTickCount();
	int dstx;
	int dsty;

	//旋转后 围绕点(图片左上角)坐标
	long ttt =  (rx + move_x)*(1<<16) - (rx*Ax_16 + ry*Ay_16);
	long ttt2 = (ry + move_y)*(1<<16) - (rx*Bx_16 + ry*By_16);

	POINT plt = {pRect->left,pRect->top};
	dstx = plt.x*Ax_16 + plt.y*Ay_16 + ttt;
	dsty = ttt2 + plt.x*Bx_16 + plt.y*By_16;
	plt.x = dstx >> 16;
	plt.y = dsty >> 16;

	POINT plb = {pRect->left,pRect->bottom};
	dstx = plb.x*Ax_16 + plb.y*Ay_16 + ttt;
	dsty = ttt2 + plb.x*Bx_16 + plb.y*By_16;
	plb.x = dstx >> 16;
	plb.y = dsty >> 16;

	POINT prt = {pRect->right,pRect->top};
	dstx = prt.x*Ax_16 + prt.y*Ay_16 + ttt;
	dsty = ttt2 + prt.x*Bx_16 + prt.y*By_16;
	prt.x = dstx >> 16;
	prt.y = dsty >> 16;

	POINT prb = {pRect->right,pRect->bottom};
	dstx = prb.x*Ax_16 + prb.y*Ay_16 + ttt;
	dsty = ttt2 + prb.x*Bx_16 + prb.y*By_16;
	prb.x = dstx >> 16;
	prb.y = dsty >> 16;

	int minX = min(plt.x,min(plb.x,min(prt.x,prb.x)));
	int minY = min(plt.y,min(plb.y,min(prt.y,prb.y)));
	int maxX = max(plt.x,max(plb.x,max(prt.x,prb.x)));
	int maxY = max(plt.y,max(plb.y,max(prt.y,prb.y)));

	pRect->left = minX;
	pRect->top = minY;
	pRect->bottom = maxY;
	pRect->right = maxX;
//	printf("RECT:%d,%d,%d,%d\n",pRect->left,pRect->top,pRect->right,pRect->bottom);
//	printf("GetRecttime2:%d\n",GetTickCount() -time1);
}

void CTimeClock::Mix(RGBQUAD * pDst,RGBQUAD * pSrc,RGBQUAD * pSrc2)
{
	if(pSrc->rgbReserved < 5 || pSrc2->rgbReserved > 250)
		*pDst = *pSrc2;
	else
	{
		int a0,a1,a2;
		a2 = pSrc2->rgbReserved;
		a1 = (pSrc->rgbReserved * (256 - a2)) >> 8;
		a0 = a2 + a1;
		pDst->rgbReserved = a0;
		pDst->rgbBlue	= (BYTE)((pSrc2->rgbBlue  * a2	+ a1 * pSrc->rgbBlue  )/a0);
		pDst->rgbGreen	= (BYTE)((pSrc2->rgbGreen * a2	+ a1 * pSrc->rgbGreen )/a0);
		pDst->rgbRed	= (BYTE)((pSrc2->rgbRed	  * a2	+ a1 * pSrc->rgbRed   )/a0);
	}
}
void CTimeClock::PicRotary21(const Block_Req * Dst,
							const Block_Req * Src,
							int RotaryAngle,
							double rx,
							double ry,
							double move_x,
							double move_y)
{
	double sinA,cosA;

	SinCos(RotaryAngle*M_P, sinA, cosA);
	long Ax_16=(long)(cosA*(1<<16));
	long Ay_16=(long)(sinA*(1<<16));
	long Bx_16=(long)(-sinA*(1<<16));
	long By_16=(long)(cosA*(1<<16));
	RECT dstRect = {0, 0, Src->win.cx, Src->win.cy};
	GetRect(&dstRect,Ax_16,Ay_16,Bx_16,By_16,rx,ry,move_x,move_y);
	long Cx_16 =(long)(dstRect.left - rx - move_x)*Ax_16 + (dstRect.top - ry - move_y)*Bx_16 + rx*(1<<16);
	long Cy_16 =(long)(dstRect.left - rx - move_x)*Ay_16 + (dstRect.top - ry - move_y)*By_16 + ry*(1<<16);

	DWORD * pDstLine = Dst->viraddr + dstRect.top*Dst->win.cx;
	long srcx0_16=(Cx_16);
	long srcy0_16=(Cy_16);
	RGBQUAD src,src2,dst;
	for (long y=dstRect.top;y<=dstRect.bottom;++y)
	{
		long srcx_16=srcx0_16;
		long srcy_16=srcy0_16;

		for (long x=dstRect.left;x<=dstRect.right;++x)
		{
			long srcx=(srcx_16>>16);
			long srcy=(srcy_16>>16);
 			if (PixelsIsInPic(Src,srcx,srcy))
 			{
				src2 = *(RGBQUAD*)&Pixels2(Src,srcx_16,srcy_16);
				src = *(RGBQUAD*)&Pixels(Dst,x,y);
				Mix(&dst,&src,&src2);
				pDstLine[x] = *(DWORD*)&dst;
			}
			srcx_16+=Ax_16;
			srcy_16+=Ay_16;
		}
		srcx0_16+=Bx_16;
		srcy0_16+=By_16;
		pDstLine += Dst->win.cx;
	}
}

BOOL CTimeClock::DoInit(ContentManage * pCm)
{
	SIZE size;
	HANDLE hclock;
	char* pcontent;

	if((pcontent = pCm->FindContentByName("left")) == NULL)
		return FALSE;
	m_left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("top")) == NULL)
		return FALSE;
	m_top = strtol(pcontent, NULL, 10);
	
	if((pcontent = pCm->FindContentByName("clockpng")) == NULL)
		return FALSE;
	hclock = m_pSpr->LoadImage(pcontent, &size);
	if(hclock == NULL)
		return FALSE;

	m_width = size.cx;
	m_height = size.cy;
	move_x = m_width/2;
	move_y = m_height/2;

	m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
	m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

	m_hClockBk = m_pSpr->DupBlock(m_hFrameBak);
	m_pSpr->BitBlt(m_hClockBk, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);
	m_pSpr->AlphaBlend(m_hClockBk, 0, 0, m_width, m_height, hclock, 0, 0);
	m_pSpr->CloseBlk(hclock);

	hTempSpriteHM = m_pSpr->ReqTempBlk(m_width, m_height);
	if(hTempSpriteHM == NULL)
		return FALSE;

	if((pcontent = pCm->FindContentByName("hour")) == NULL)
		return FALSE;
	m_hSpinHour = m_pSpr->LoadImage(pcontent, &size);
	if (m_hSpinHour == NULL)
		return FALSE;
	Dot[0].x = (size.cx-1)/2;
	Dot[0].y = size.cy;
	if((pcontent = pCm->FindContentByName("hourhole")) != NULL)
		Dot[0].y -= strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("minute")) == NULL)
		return FALSE;
	m_hSpinMinute = m_pSpr->LoadImage(pcontent, &size);
	if (m_hSpinMinute == NULL)
		return FALSE;
	Dot[1].x = (size.cx-1)/2;
	Dot[1].y = size.cy;
	if((pcontent = pCm->FindContentByName("minutehole")) != NULL)
		Dot[1].y -= strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName("second")) != NULL)
	{
		m_hSpinSecond = m_pSpr->LoadImage(pcontent, &size);
		if (m_hSpinSecond != NULL)
		{
			Dot[2].x = (size.cx-1)/2;
			Dot[2].y = size.cy;
			if((pcontent = pCm->FindContentByName("secondhole")) != NULL)
				Dot[2].y -= strtol(pcontent, NULL, 10);
			hTempSpriteHMS = m_pSpr->ReqTempBlk(m_width, m_height);
			if(hTempSpriteHMS == NULL)
			{
				m_pSpr->CloseBlk(m_hSpinSecond);
				m_hSpinSecond = NULL;
			}
		}
	}
	UpdataDateTime(TRUE);
//	m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
	m_msgid = m_pLayer->RequestMsgId();
	m_pLayer->RegisterCtrl(CTRL_CLOCK, "clock", this, m_msgid);
	return TRUE;
}


void CTimeClock::UpdataDateTime(BOOL IsInit)
{
	SYSTEMTIME stime;
	int hour;

	if(IsInit)
		memset(&lastsystem, 0, sizeof(SYSTEMTIME));
	DPGetLocalTime(&stime);
	if (stime.wHour >= 12)
		hour = stime.wHour - 12;
	else
		hour = stime.wHour;

	RECT rc = {0};
	if ((stime.wHour != lastsystem.wHour || stime.wMinute != lastsystem.wMinute)
		|| IsInit)
	{
		// 如果分针和时针变化，重画hTempSpriteHM
		m_pSpr->BitBlt(hTempSpriteHM, 0, 0, m_width, m_height, m_hClockBk, 0, 0);
		PicRotary21((Block_Req *)hTempSpriteHM,
					(Block_Req *)m_hSpinHour,
					(360 - hour*30 - (stime.wMinute>>1)),
					Dot[0].x,
					Dot[0].y,
					move_x-Dot[0].x,
					move_y-Dot[0].y);
		PicRotary21((Block_Req *)hTempSpriteHM,
					(Block_Req *)m_hSpinMinute,
					(360 - stime.wMinute*6),
					Dot[1].x,
					Dot[1].y,
					move_x-Dot[1].x,
					move_y-Dot[1].y);
		lastsystem = stime;
		if(m_hSpinSecond == NULL)
		{
			m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height,hTempSpriteHM,0,0);
			return;
		}
	}
	if(m_hSpinSecond != NULL)
	{
		m_pSpr->BitBlt(hTempSpriteHMS, 0, 0, m_width, m_height, hTempSpriteHM, 0, 0);
		PicRotary21((Block_Req *)hTempSpriteHMS,
					(Block_Req *)m_hSpinSecond,
					(360 - stime.wSecond*6),
					Dot[2].x,
					Dot[2].y,
					move_x-Dot[2].x,
					move_y-Dot[2].y);	
		m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height,hTempSpriteHMS,0,0);
	}
}

