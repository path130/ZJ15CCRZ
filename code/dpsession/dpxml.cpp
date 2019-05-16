#include "roomlib.h"
#include "dpmsg.h"
#include "dpxml.h"
#include "dpgpio.h"

static char* g_pXmlBuf = NULL;
static DPMedia* g_pMedia = NULL;	

static const char* XmlParse(const char* pbuf, const char* pmatch)
{
	char pMatch[32];
	sprintf(pMatch, "<%s>", pmatch);

	const char* pStart = strstr(pbuf, pMatch);
	if(pStart == NULL)
		return NULL;
	pStart += strlen(pMatch);

	sprintf(pMatch, "</%s>", pmatch);
	const char* pEnd = strstr(pStart, pMatch);
	if(pEnd == NULL)
		return NULL;

	strncpy(g_pXmlBuf, pStart, pEnd - pStart);
	g_pXmlBuf[pEnd - pStart] = 0;
	return g_pXmlBuf;
}

BOOL GetXmlString(const char* pbuf, const char* pmatch, char* pStr, int nStr)
{
	const char* p = XmlParse(pbuf, pmatch);
	if(p == NULL)
		return FALSE;

	int count = strlen(p);
	if(count >= nStr)
		count = nStr - 1;

	memcpy(pStr, p, count);
	pStr[count] = 0;
	return TRUE;
}

BOOL GetXmlNumber(const char* pbuf, const char* pmatch, int* val)
{
	const char* p = XmlParse(pbuf, pmatch);
	if(p == NULL)
		return FALSE;

	*val = atol(p);
	return TRUE;
}

BOOL CompareXmlID(char* pContent, DPSession* pSession)
{
	int lid, rid;
	if(GetXmlNumber(pContent, "lid", &lid)
		&& GetXmlNumber(pContent, "rid", &rid))
	{
		if(pSession
			&& pSession->lid == rid
			&& pSession->rid == lid)
		{
			return TRUE;
		}
	}

	return FALSE;
}

DWORD ParseCmd(char* pContent)
{
	int nCmd = DPMSG_NONE;

	const char* pCmd = XmlParse(pContent, "cmd");
	if(strcmp(pCmd, "Tick") == 0)
		nCmd = DPMSG_HEARTBEAT;
	else if(strcmp(pCmd, "Call") == 0)
		nCmd = DPMSG_NEWCALLIN;
	else if(strcmp(pCmd, "Accpet") == 0)
		nCmd = DPMSG_ACCEPT;
	else if(strcmp(pCmd, "HangUp") == 0)
		nCmd = DPMSG_HANGUP;
	else if(strcmp(pCmd, "Message") == 0)
		nCmd = DPMSG_INFO;
	else if(strcmp(pCmd, "CallAck") == 0)
	{
		pCmd = XmlParse(pContent, "result");
		if(strcmp(pCmd, "ring") == 0)
			nCmd = DPMSG_IDLE;
		else if(strcmp(pCmd, "busy") == 0)
			nCmd = DPMSG_BUSY;
	}

	if(nCmd == DPMSG_NONE)
		DBGMSG(DPINFO, "Recv Unkonw Call Msg:%s\r\n", pContent);

	return nCmd;
}

BOOL GetXmlMediaInfo(DPMedia* pMedia, char* pContent)
{
	do
	{
		memset(pMedia, 0, sizeof(DPMedia));

		const char *pXml = NULL;
		char trans_type[32];
		char enc_type[32];
		char buf[512];

		// Media Video
		pXml = XmlParse(pContent, "media");
		if(pXml == NULL)
			break;

		pXml = XmlParse(pXml, "Video");
		if(pXml)
		{
			strncpy(buf, pXml, 512);
			buf[512 - 1] = 0;

			if(!GetXmlString(buf, "dir", trans_type, 32)
				|| !GetXmlString(buf, "codec", enc_type, 32)
				|| !GetXmlNumber(buf, "width", &pMedia->video.width)
				|| !GetXmlNumber(buf, "height", &pMedia->video.height)
				|| !GetXmlNumber(buf, "bitrate", &pMedia->video.bitrate))
			{
				break;
			}

			if(strcmp(enc_type, "h264") == 0)
				pMedia->video.enc_type = ENCODE_H264;
			else if(strcmp(enc_type, "mpg4") == 0)
				pMedia->video.enc_type = ENCODE_H264;
			else
				break;

			pMedia->video.trans_type = 0;
			if(strstr(trans_type, "send"))
			{
				pMedia->video.trans_type |= TRANS_TYPE_SEND;
			}
			if(strstr(trans_type, "recv"))
			{
				pMedia->video.trans_type |= TRANS_TYPE_RECV;
			}
			if(pMedia->video.trans_type == 0)
				break;

			pXml = XmlParse(buf, "laddr");
			if(pXml == NULL)
				break;
			strncpy(buf, pXml, 512);
			buf[512 - 1] = 0;

			if(!GetXmlNumber(buf, "ip", &pMedia->video.addr.ip)
				|| !GetXmlNumber(buf, "port", &pMedia->video.addr.port)
				|| !GetXmlNumber(buf, "cport", &pMedia->video.addr.cport))
			{
				break;
			}
		}

		// Media Audio
		pXml = XmlParse(pContent, "media");
		if(pXml == NULL)
			break;

		pXml = XmlParse(pXml, "Audio");
		if(pXml)
		{
			strncpy(buf, pXml, 512);
			buf[512 - 1] = 0;

			if(!GetXmlString(buf, "dir", trans_type, 32)
				|| !GetXmlString(buf, "codec", enc_type, 32))
			{
				break;
			}

			if(strcmp(enc_type, "silk") == 0)
				pMedia->audio.enc_type = 1;
			else
				break;

			pMedia->audio.trans_type = 0;
			if(strstr(trans_type, "send"))
			{
				pMedia->audio.trans_type |= TRANS_TYPE_SEND;
			}
			if(strstr(trans_type, "recv"))
			{
				pMedia->audio.trans_type |= TRANS_TYPE_RECV;
			}
			if(pMedia->audio.trans_type == 0)
				break;

			pXml = XmlParse(buf, "laddr");
			if(pXml == NULL)
				break;
			strncpy(buf, pXml, 512);
			buf[512 - 1] = 0;

			if(!GetXmlNumber(buf, "ip", &pMedia->audio.addr.ip)
				|| !GetXmlNumber(buf, "port", &pMedia->audio.addr.port)
				|| !GetXmlNumber(buf, "cport", &pMedia->audio.addr.cport))
			{
				break;
			}
		}

		return TRUE;
	}while(0);

	return FALSE;
}

static void BuildAcceptXml(char* pContent, DPSession* pSession)
{
	int len = sprintf(pContent, "<cmd>Accpet</cmd><lid>%d</lid><rid>%d</rid>", pSession->lid, pSession->rid);
	len += sprintf(&pContent[len], "<media>");
	len += sprintf(&pContent[len], "<nVideo>1</nVideo><nAudio>1</nAudio>");
	len += sprintf(&pContent[len], "<Video>");
	len += sprintf(&pContent[len], "<laddr><ip>%d</ip><port>%d</port><cport>%d</cport></laddr>", g_pMedia->video.addr.ip, g_pMedia->video.addr.port, g_pMedia->video.addr.cport);
	len += sprintf(&pContent[len], "<raddr><ip>%d</ip><port>%d</port><cport>%d</cport></raddr>", pSession->rmedia.video.addr.ip, pSession->rmedia.video.addr.port, pSession->rmedia.video.addr.cport);
	len += sprintf(&pContent[len], "<dir>sendrecv</dir><codec>h264</codec><width>%d</width><height>%d</height><bitrate>%d</bitrate>", g_pMedia->video.width, g_pMedia->video.height, g_pMedia->video.bitrate);
	len += sprintf(&pContent[len], "</Video>");
	len += sprintf(&pContent[len], "<Audio>");
	len += sprintf(&pContent[len], "<laddr><ip>%d</ip><port>%d</port><cport>%d</cport></laddr>", g_pMedia->audio.addr.ip, g_pMedia->audio.addr.port, g_pMedia->audio.addr.cport);
	len += sprintf(&pContent[len], "<laddr><ip>%d</ip><port>%d</port><cport>%d</cport></laddr>", pSession->rmedia.audio.addr.ip, pSession->rmedia.audio.addr.port, pSession->rmedia.audio.addr.cport);
	len += sprintf(&pContent[len], "<dir>sendrecv</dir><codec>silk</codec>");
	len += sprintf(&pContent[len], "</Audio>");
	len += sprintf(&pContent[len], "</media>");
}

static void BuildCallXml(char* pContent, DPSession* pSession, char* termId)
{
	int len = sprintf(pContent, "<cmd>Call</cmd><lid>%d</lid><rid>0</rid>", pSession->lid);
	len += sprintf(&pContent[len], "<exinfo><Number>%s</Number></exinfo>", termId);
	len += sprintf(&pContent[len], "<media>");
	len += sprintf(&pContent[len], "<nVideo>1</nVideo><nAudio>1</nAudio>");
	len += sprintf(&pContent[len], "<Video>");
	len += sprintf(&pContent[len], "<laddr><ip>%d</ip><port>%d</port><cport>%d</cport></laddr>", g_pMedia->video.addr.ip, g_pMedia->video.addr.port, g_pMedia->video.addr.cport);
	len += sprintf(&pContent[len], "<dir>sendrecv</dir><codec>h264</codec><width>%d</width><height>%d</height><bitrate>%d</bitrate>", g_pMedia->video.width, g_pMedia->video.height, g_pMedia->video.bitrate);
	len += sprintf(&pContent[len], "</Video>");
	len += sprintf(&pContent[len], "<Audio>");
	len += sprintf(&pContent[len], "<laddr><ip>%d</ip><port>%d</port><cport>%d</cport></laddr>", g_pMedia->audio.addr.ip, g_pMedia->audio.addr.port, g_pMedia->audio.addr.cport);
	len += sprintf(&pContent[len], "<dir>sendrecv</dir><codec>silk</codec>");
	len += sprintf(&pContent[len], "</Audio>");
	len += sprintf(&pContent[len], "</media>");
}

void BuildPacketXml(char* pContent, DWORD cmd, int lid, int rid, int remotemsgid, DPSession* pSession)
{
	char termId[16];
	switch(cmd)
	{
		case DPMSG_BUSY:
			sprintf(pContent, "<cmd>CallAck</cmd><lid>%d</lid><rid>%d</rid><result>%s</result>", lid, rid, "busy");
			break;
		case DPMSG_IDLE:
			sprintf(pContent, "<cmd>CallAck</cmd><lid>%d</lid><rid>%d</rid><result>%s</result>", lid, rid, "ring");
			break;
		case DPMSG_ACCEPT:
			BuildAcceptXml(pContent, pSession);
			break;
		case DPMSG_HANGUP:
			sprintf(pContent, "<cmd>HangUp</cmd><lid>%d</lid><rid>%d</rid>", lid, rid);
			break;
		case DPMSG_HEARTBEAT:
			sprintf(pContent, "<cmd>Tick</cmd><lid>%d</lid><rid>%d</rid>", lid, rid);
			break;
		case DPMSG_CALL:
			GetTermId(termId);
			BuildCallXml(pContent, pSession, termId);
			break;
		case DPMSG_MONITOR:
			GetTermId(termId);
			sprintf(pContent, "<cmd>Call</cmd><lid>%d</lid><rid>0</rid><exinfo><Number>%s</Number></exinfo>", pSession->lid, termId);
			break;
		case DPMSG_INFO:
			break;
	}
}

void InitXmlServer(DPMedia* pMedia)
{
	g_pXmlBuf = (char*)malloc(2048);
	g_pMedia = pMedia;
}

void UnInitXmlServer()
{
	free(g_pXmlBuf);
}