#include "CCtrlTime.h"

BOOL CTimeDate::DoInit(ContentManage * pCm)
{
	char* pcontent;

	if((pcontent = pCm->FindContentByName("exclude")) != NULL)
	{
		if(strstr(pcontent, "time") != 0)
			m_existbit &= 0x0e;
		if(strstr(pcontent, "date") != 0)
			m_existbit &= 0x0d;
		if(strstr(pcontent, "week") != 0)
			m_existbit &= 0x0b;
	}

	if(m_existbit & 1)
	{
		if((pcontent = pCm->FindContentByName("ttextsize")) != NULL)
			m_timesize = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("ttextcolor")) != NULL)
			m_timecolor = hexconvert(pcontent);
		if((pcontent = pCm->FindContentByName("timeleft")) == NULL)
			return FALSE;
		m_timeleft = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("timetop")) == NULL)
			return FALSE;
		m_timetop = strtol(pcontent, NULL, 10);

		HANDLE Htemp  = m_pSpr->LoadStr(":", m_timesize, m_timecolor, &colon_width);
		colon_frame = m_pSpr->ReqTempBlk(colon_width, m_timesize);
		if(Htemp != NULL)
		{
			m_pSpr->BitBlt(colon_frame, 0, 0, colon_width, m_timesize, Htemp, 0, 0);
			m_pSpr->CloseBlk(Htemp);
		}
		m_hFrameBak = m_pSpr->ReqTempBlk(colon_width, m_timesize);
		m_pSpr->BitBlt(m_hFrameBak, 0, 0, colon_width, m_timesize, m_pLayer->m_frame, 
						m_timeleft - colon_width/2, m_timetop - m_timesize/2);
		m_pTimeHour = new CDPStatic(m_pLayer);
		m_pTimeHour->SetTextColor(m_timecolor);
		m_pTimeHour->SetTextSize(m_timesize);
		m_pTimeHour->SetStart(m_timeleft - colon_width/2, m_timetop);
		m_pTimeHour->SetAlign(ALIGN_RIGHT | ALIGN_MIDDLE);
		m_pTimeMinute = new CDPStatic(m_pLayer);
		m_pTimeMinute->SetTextColor(m_timecolor);
		m_pTimeMinute->SetTextSize(m_timesize);
		m_pTimeMinute->SetStart(m_timeleft + colon_width/2, m_timetop);
		m_pTimeMinute->SetAlign(ALIGN_LEFT | ALIGN_MIDDLE);
	}
	else
		m_timesize = 0;

	if(m_existbit & 2)
	{
		if((pcontent = pCm->FindContentByName("dtextsize")) != NULL)
			m_datesize = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("dtextcolor")) != NULL)
			m_datecolor = hexconvert(pcontent);
		if((pcontent = pCm->FindContentByName("dateleft")) == NULL)
			return FALSE;
		m_dateleft = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("datetop")) == NULL)
			return FALSE;
		m_datetop = strtol(pcontent, NULL, 10);

		if((pcontent = pCm->FindContentByName("datealign")) != NULL)
		{
			if(strstr(pcontent, "right") != NULL)
				m_datealgin |= ALIGN_RIGHT;
			else if(strstr(pcontent, "center") != NULL)
				m_datealgin |= ALIGN_CENTER;
			else
				m_datealgin |= ALIGN_LEFT;

			if(strstr(pcontent, "middle") != NULL)
				m_datealgin |= ALIGN_MIDDLE;
			else if(strstr(pcontent, "top") != NULL)
				m_datealgin |= ALIGN_TOP;
			else
				m_datealgin |= ALIGN_BOTTOM;
		}
		else
		{
			m_datealgin |= ALIGN_LEFT;
			m_datealgin |= ALIGN_BOTTOM;
		}
		m_pDate = new CDPStatic(m_pLayer);
		m_pDate->SetTextColor(m_datecolor);
		m_pDate->SetTextSize(m_datesize);
		m_pDate->SetStart(m_dateleft, m_datetop);
		m_pDate->SetAlign(m_datealgin);
	}

	if(m_existbit & 4)
	{
		if((pcontent = pCm->FindContentByName("wtextsize")) != NULL)
			m_weeksize = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("wtextcolor")) != NULL)
			m_weekcolor = hexconvert(pcontent);
		if((pcontent = pCm->FindContentByName("weekleft")) == NULL)
			return FALSE;
		m_weekleft = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("weektop")) == NULL)
			return FALSE;
		m_weektop = strtol(pcontent, NULL, 10);
		if((pcontent = pCm->FindContentByName("weekalign")) != NULL)
		{
			if(strstr(pcontent, "left") != NULL)
				m_weekalgin |= ALIGN_LEFT;
			else if(strstr(pcontent, "center") != NULL)
				m_weekalgin |= ALIGN_CENTER;
			else
				m_weekalgin |= ALIGN_RIGHT;

			if(strstr(pcontent, "middle") != NULL)
				m_weekalgin |= ALIGN_MIDDLE;
			else if(strstr(pcontent, "top") != NULL)
				m_weekalgin |= ALIGN_TOP;
			else
				m_weekalgin |= ALIGN_BOTTOM;
		}
		else
		{
			m_weekalgin |= ALIGN_RIGHT;
			m_weekalgin |= ALIGN_BOTTOM;
		}
		m_pWeek = new CDPStatic(m_pLayer);
		m_pWeek->SetTextColor(m_weekcolor);
		m_pWeek->SetTextSize(m_weeksize);
		m_pWeek->SetStart(m_weekleft, m_weektop);
		m_pWeek->SetAlign(m_weekalgin);
	}

	m_msgid = m_pLayer->RequestMsgId();
	if((pcontent = pCm->FindContentByName("name")) == NULL)
		m_pLayer->RegisterCtrl(CTRL_TIME, "time", this, m_msgid);
	else
		m_pLayer->RegisterCtrl(CTRL_TIME, pcontent, this, m_msgid);
	UpdataDateTime(TRUE);

	return TRUE;
}

void CTimeDate::Hide(BOOL isHide)
{
	if(isHide == TRUE)
	{
		iHide = TRUE;
		m_pWeek->Show(FALSE);
		m_pTimeHour->Show(FALSE);
		m_pTimeMinute->Show(FALSE);
		m_pDate->Show(FALSE);
		m_pSpr->BitBlt(m_pLayer->m_frame, 
			m_timeleft - colon_width/2, m_timetop - m_timesize/2,
			colon_width, m_timesize, 
			m_hFrameBak, 0, 0);
	}
	else
		isHide = FALSE;
	
}

void CTimeDate::UpdataDateTime(BOOL IsInit)
{
	if(iHide  == TRUE)
	{
		return;
	}
	SYSTEMTIME cursystem;

	if(IsInit)
		memset(&lastsystem, 0xff, sizeof(SYSTEMTIME));
	DPGetLocalTime(&cursystem);
	if((lastsystem.wYear != cursystem.wYear)
		|| (lastsystem.wMonth != cursystem.wMonth)
		|| (lastsystem.wDay != cursystem.wDay))
	{
		DrawDate(&cursystem);
	}

	if((lastsystem.wHour != cursystem.wHour)
		|| (lastsystem.wMinute != cursystem.wMinute))
	{
		DrawTime(&cursystem);
	}
	else
	{
		if(cursystem.wSecond & 1)
			m_pSpr->AlphaBlend(m_pLayer->m_frame,
							m_timeleft - colon_width/2, m_timetop - m_timesize/2,
							colon_width, m_timesize,
							colon_frame, 0, 0);
		else
			m_pSpr->BitBlt(m_pLayer->m_frame, 
							m_timeleft - colon_width/2, m_timetop - m_timesize/2,
							colon_width, m_timesize, 
							m_hFrameBak, 0, 0);
	}
	m_WorkSec++;
	memcpy(&lastsystem, &cursystem, sizeof(SYSTEMTIME));
}	

void CTimeDate::DrawDate(SYSTEMTIME* pTime)
{
	char wbuf[64];

	if(m_pDate != NULL)
	{
		sprintf(wbuf, "%d%s%02d%s%02d%s", 
			pTime->wYear, GetStringByID(1001), pTime->wMonth, GetStringByID(1002), pTime->wDay, GetStringByID(1003));
		m_pDate->SetSrc(wbuf);
		m_pDate->Show(TRUE);
	}
	if(m_pWeek)
	{
		m_pWeek->SetSrc(GetStringByID(1004 + pTime->wDayOfWeek));
		m_pWeek->Show(TRUE);
	}
}

void CTimeDate::DrawTime(SYSTEMTIME* pTime)
{
	char wbuf[32];

	if(m_existbit & 1)
	{
		sprintf(wbuf, "%02d", pTime->wHour);
		m_pTimeHour->SetSrc(wbuf);
		m_pTimeHour->Show(TRUE);

		sprintf(wbuf, "%02d", pTime->wMinute);
		m_pTimeMinute->SetSrc(wbuf);
		m_pTimeMinute->Show(TRUE);

		if((pTime->wSecond & 1) == 0)
			m_pSpr->AlphaBlend(m_pLayer->m_frame, 
				m_timeleft - colon_width/2,
				m_timetop - m_timesize/2,
				colon_width, m_timesize, colon_frame, 0, 0);
		else
			m_pSpr->BitBlt(m_pLayer->m_frame, 
				m_timeleft - colon_width/2,
				m_timetop - m_timesize/2,
				colon_width, m_timesize, m_hFrameBak,
				0, 0);
	}
	return;
}


