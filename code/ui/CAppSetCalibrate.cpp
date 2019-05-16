#include "CCtrlModules.h"

#define	EDGE_CALIB		100
typedef struct
{
	int x[5];
	int xfb[5]; 
	int y[5];
	int yfb[5]; 
	int a[7]; 
} calibration;

static BOOL perform_calibration(calibration *cal)
{ 
	int j; 
	float n, x, y, x2, y2, xy, z, zx, zy; 
	float det, a, b, c, e, f, i; 
	float scaling = 65536.0; 

	// Get sums for matrix 
	n = x = y = x2 = y2 = xy = 0; 
	for(j=0;j<5;j++) { 
		n += 1.0; 
		x += (float)cal->x[j]; 
		y += (float)cal->y[j]; 
		x2 += (float)(cal->x[j]*cal->x[j]); 
		y2 += (float)(cal->y[j]*cal->y[j]); 
		xy += (float)(cal->x[j]*cal->y[j]); 
	} 

	// Get determinant of matrix -- check if determinant is too small 
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2); 
	if(det < 0.1 && det > -0.1) { 
		printf("ts_calibrate: determinant is too small -- %f\n",det); 
		return FALSE; 
	} 

	// Get elements of inverse matrix 
	a = (x2*y2 - xy*xy)/det; 
	b = (xy*y - x*y2)/det; 
	c = (x*xy - y*x2)/det; 
	e = (n*y2 - y*y)/det; 
	f = (x*y - n*xy)/det; 
	i = (n*x2 - x*x)/det; 

	// Get sums for x calibration 
	z = zx = zy = 0; 
	for(j=0;j<5;j++) { 
		z += (float)cal->xfb[j]; 
		zx += (float)(cal->xfb[j]*cal->x[j]); 
		zy += (float)(cal->xfb[j]*cal->y[j]); 
	} 

	// Now multiply out to get the calibration for framebuffer x coord 
	cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling)); 
	cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling)); 
	cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling)); 

	printf("%f %f %f\n",(a*z + b*zx + c*zy), 
		(b*z + e*zx + f*zy), 
		(c*z + f*zx + i*zy)); 

	// Get sums for y calibration 
	z = zx = zy = 0; 
	for(j=0;j<5;j++) { 
		z += (float)cal->yfb[j]; 
		zx += (float)(cal->yfb[j]*cal->x[j]); 
		zy += (float)(cal->yfb[j]*cal->y[j]); 
	} 

	// Now multiply out to get the calibration for framebuffer y coord 
	cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling)); 
	cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling)); 
	cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling)); 

	printf("%f %f %f\n",(a*z + b*zx + c*zy), 
		(b*z + e*zx + f*zy), 
		(c*z + f*zx + i*zy)); 

	// If we got here, we're OK, so assign scaling to a[6] and return 
	cal->a[6] = (int)scaling; 

	printf("%d %d %d %d %d %d %d\n", cal->a[0], cal->a[1], cal->a[2], cal->a[3], cal->a[4], cal->a[5], cal->a[6]); 
	for(j = 0; j < 5; j++)
	{
		int calx, caly;
		int diffx, diffy;

		calx = (cal->a[1] * cal->x[j] + cal->a[2] * cal->y[j] + cal->a[0]) >> 16;
		caly = (cal->a[4] * cal->x[j] + cal->a[5] * cal->y[j] + cal->a[3]) >> 16;
		printf("calx %d caly %d\r\n", calx, caly);
		diffx = calx - cal->xfb[j];
		diffy = caly - cal->yfb[j];
		if(diffx < 0)
			diffx = -diffx;
		if(diffy < 0)
			diffy = -diffy;
		if((diffx > 10) || (diffy > 10))
			break;
	}
	if(j == 5)
		return TRUE; 
	else
		return FALSE;
} 

class CSetCalibrateApp: public CAppBase
{
public:
	CSetCalibrateApp(DWORD pHwnd):CAppBase(pHwnd)
	{
		int screen_width = FRAME_WIDTH;
		int screen_height = FRAME_HEIGHT;

		m_calstep = 0;
		memset(&m_Point, 0, sizeof(calibration));
		m_Point.xfb[0] = EDGE_CALIB;
		m_Point.yfb[0] = EDGE_CALIB;
		m_Point.xfb[1] = screen_width - EDGE_CALIB;
		m_Point.yfb[1] = EDGE_CALIB;
		m_Point.xfb[2] = screen_width - EDGE_CALIB;
		m_Point.yfb[2] = screen_height - EDGE_CALIB;
		m_Point.xfb[3] = EDGE_CALIB;
		m_Point.yfb[3] = screen_height - EDGE_CALIB;
		m_Point.xfb[4] = screen_width/2;
		m_Point.yfb[4] = screen_height/2;
	}

	~CSetCalibrateApp(void)
	{
	}

	BOOL DoPause(void)
	{
		SetTouchDirect(FALSE);
		return CAppBase::DoPause();
	}

	void DoResume(void)
	{
		SetTouchDirect(TRUE);
		CAppBase::DoResume();
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TOUCH_RAW_MESSAGE:
			switch(zParam)
			{
			case TOUCH_DOWN:
				m_count = 0;
				m_satify = 0;
				m_begin = TRUE;
				break;
			case TOUCH_VALID:
				if(m_begin)
				{
					if(m_count == 0)
					{
						m_curx = lParam;
						m_cury = wParam;
					}
					else
					{
						DWORD diffx, diffy;
						if(m_curx > lParam)
							diffx = m_curx - lParam;
						else
							diffx = lParam - m_curx;

						if(m_cury > wParam)
							diffy = m_cury - wParam;
						else
							diffy = wParam - m_cury;

						printf("x:%d, y:%d\r\n", diffx, diffy);
						if((diffx < 20) && (diffy < 20))
						{
							m_satify++;
							if(m_satify > 100)
							{
								m_Point.x[m_calstep] = m_curx;
								m_Point.y[m_calstep] = m_cury;
								DBGMSG(DPINFO, "xframe %d yframe %d touchx %d touchy %d\r\n", 
									m_Point.xfb[m_calstep], m_Point.yfb[m_calstep], m_Point.x[m_calstep], m_Point.y[m_calstep]);
								m_calstep++;
								if(m_calstep == 5)
								{
									if(perform_calibration(&m_Point))
									{
										SaveTouchCalibrate(m_Point.a);
										DPPostMessage(MSG_SHOW_STATUE, 23003, TRUE, 0);
										DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
										DPPostMessage(MSG_END_APP, (DWORD)this, m_IdBase, 0);
									}
									else
									{
										DPPostMessage(MSG_SHOW_STATUE, 23002, 0, 0);
										m_calstep = 0;
										MovePoint();
										m_begin = FALSE;
									}
								}
								else
								{
									MovePoint();
									m_begin = FALSE;
								}
							}
						}
						else
							m_satify = 0;
						m_curx = (m_curx + lParam)/2;
						m_cury = (m_cury + wParam)/2;
					}
					m_count++;
				}
				break;
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lparam, DWORD zParam)
	{
		InitFrame("calibrate.xml");
		m_pTaget = (CDPStatic*)GetCtrlByName("target");
		SetTouchDirect(TRUE);
		MovePoint();
		return TRUE;
	}
private:
	void MovePoint(void)
	{
		m_pTaget->Show(FALSE);
		m_pTaget->SetStart(m_Point.xfb[m_calstep], m_Point.yfb[m_calstep]);
		m_pTaget->Show(TRUE);
	}
	CDPStatic* m_pTaget;
	DWORD m_calstep;
	DWORD m_curx;
	DWORD m_cury;
	DWORD m_count;
	DWORD m_satify;
	BOOL m_begin;
	calibration m_Point;
};

CAppBase* CreateSetCalibrateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetCalibrateApp* pApp = new CSetCalibrateApp(wParam);
	if(pApp)
	{
		if(!pApp->Create(lParam, zParam))
		{
			delete pApp;
			pApp = NULL;		
		}
	}
	return pApp;
}