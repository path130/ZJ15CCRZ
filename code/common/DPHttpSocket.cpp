#include "roomlib.h"
#include "DPHttpSocket.h"

#define	TRANS_BUF_SIZE	0x10000
#define MAXHEADERSIZE 1024

CHttpSocket::CHttpSocket()
{
	m_sock = INVALID_SOCKET;
	m_port=80;
	m_bConnected = FALSE;
	m_time = 2000;	

	m_nCurIndex = 0;
	m_bResponsed = FALSE;
	m_nResponseHeaderSize = -1;
}

CHttpSocket::~CHttpSocket()
{
	CloseSocket();
}

BOOL CHttpSocket::SetTimeout(int nTime,int nType)
{
	m_time = nTime;
	return TRUE;
}

long CHttpSocket::Receive(char* pBuffer,long nMaxLength)
{
	if(!m_bConnected)
		return 0;

	return TcpRecvDataTry(m_sock, pBuffer, nMaxLength, m_time);
}

int	CHttpSocket::Send(const char * pdata,int dlen)
{
	if(!m_bConnected)
		return 0;

	return TcpSendData(m_sock, (char*)pdata, dlen, m_time);
}

BOOL CHttpSocket::Connect(char* szHostName,int nPort)
{
	if(szHostName==NULL)
		return FALSE;

	///若已经连接,则先关闭
	if(m_bConnected)
		CloseSocket();

	///保存端口号
	m_port=nPort;

	//m_sock = TcpConnect(inet_addr(szHostName), m_port, m_time);
	m_sock = TcpConnect(szHostName, m_port, m_time);
	if(m_sock != INVALID_SOCKET)
		m_bConnected = TRUE;
	else
		m_bConnected = FALSE;

	return m_bConnected;
}

BOOL CHttpSocket::Socket()
{
	return TRUE;
}

BOOL CHttpSocket::CloseSocket()
{
	SocketClose(m_sock);
	m_sock = INVALID_SOCKET;
	return TRUE;
}

int	CHttpSocket::GetServerState()
{
	//若没有取得响应头,返回失败
	if(!m_bResponsed) 
		return -1;

	char szState[3];
	szState[0] = m_ResponseHeader[9];
	szState[1] = m_ResponseHeader[10];
	szState[2] = m_ResponseHeader[11];

	return atoi(szState);
}

int CHttpSocket::GetField(const char* szSession,char *szValue,int nMaxLength)
{
	//取得某个域值
	char* pfind;
	char* pvalue;
	int cplen;
	if(!m_bResponsed)
		return -1;

	int nPos = -1;

	pfind = strstr(m_ResponseHeader, szSession);
	if(pfind != NULL)
	{
		pfind += strlen(szSession) + 1;
		pvalue = strstr(pfind, "\r\n");
		if(pvalue != NULL)
		{
			cplen = pvalue - pfind;
			if(cplen > nMaxLength)
				cplen = nMaxLength;
			memcpy(szValue, pfind, cplen);
			szValue[cplen] = 0;
			return cplen;
		}
	}
	return -1;
}

int CHttpSocket::GetResponseLine(char *pLine,int nMaxLength)
{
	if(m_nCurIndex >= m_nResponseHeaderSize)
	{
		m_nCurIndex = 0;
		return -1;
	}

	int nIndex = 0;
	char c = 0;
	do 
	{
		c = m_ResponseHeader[m_nCurIndex++];
		pLine[nIndex++] = c;
	} while(c != '\n' && m_nCurIndex < m_nResponseHeaderSize && nIndex < nMaxLength);

	return nIndex;
}

const char* CHttpSocket::GetResponseHeader(int &nLength)
{
	if(!m_bResponsed)
	{
		char c = 0;
		int nIndex = 0;
		BOOL bEndResponse = FALSE;
		while(!bEndResponse && nIndex < MAXHEADERSIZE)
		{
			//	recv(m_s,&c,1,0);
			Receive(&c,1);
			m_ResponseHeader[nIndex++] = c;
			if(nIndex >= 4)
			{
				if(m_ResponseHeader[nIndex - 4] == '\r' && m_ResponseHeader[nIndex - 3] == '\n'
					&& m_ResponseHeader[nIndex - 2] == '\r' && m_ResponseHeader[nIndex - 1] == '\n')
					bEndResponse = TRUE;
			}
		}
		m_nResponseHeaderSize = nIndex;
		m_bResponsed = TRUE;
	}

	nLength = m_nResponseHeaderSize;
	return m_ResponseHeader;
}

const char * CHttpSocket::FormatRequestHeader(char *pServer,char *pObject,long &Length,char* pCookie,char *pReferer,long nFrom,long nTo,int nServerType)
{
	char szTemp[32];
	memset(m_requestheader, '\0', 1024);

	///第1行:方法,请求的路径,版本
	strcat(m_requestheader,"GET ");
	strcat(m_requestheader,pObject);
	strcat(m_requestheader," HTTP/1.1");
    strcat(m_requestheader,"\r\n");

	///第2行:主机
    strcat(m_requestheader,"Host:");
	strcat(m_requestheader,pServer);
    strcat(m_requestheader,"\r\n");

	///第3行:
	if(pReferer != NULL)
	{
		strcat(m_requestheader,"Referer:");
		strcat(m_requestheader,pReferer);
		strcat(m_requestheader,"\r\n");		
	}

	///第4行:接收的数据类型
    strcat(m_requestheader,"Accept:*/*");
    strcat(m_requestheader,"\r\n");

	///第5行:浏览器类型
    strcat(m_requestheader, "User-Agent:Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");
	//"User-Agent:Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)"
    strcat(m_requestheader,"\r\n");

	///第6行:连接设置,保持
	strcat(m_requestheader,"Connection:Keep-Alive");
	strcat(m_requestheader,"\r\n");

	///第7行:Cookie.
	if(pCookie != NULL)
	{
		strcat(m_requestheader,"Set Cookie:0");
		strcat(m_requestheader,pCookie);
		strcat(m_requestheader,"\r\n");
	}

	///第8行:请求的数据起始字节位置(断点续传的关键)
	if(nFrom > 0)
	{
		strcat(m_requestheader,"Range: bytes=");
		sprintf(szTemp, "%d", nFrom);
		strcat(m_requestheader,szTemp);
		strcat(m_requestheader,"-");
		if(nTo > nFrom)
		{
			sprintf(szTemp, "%d", nTo);
			strcat(m_requestheader,szTemp);
		}
		strcat(m_requestheader,"\r\n");
	}
	
	///最后一行:空行
	strcat(m_requestheader,"\r\n");

	///返回结果
	Length=strlen(m_requestheader);
	return m_requestheader;
}

int CHttpSocket::GetRequestHeader(char *pHeader,int nMaxLength) const
{
	int nLength;
	if(int(strlen(m_requestheader))>nMaxLength)
	{
		nLength=nMaxLength;
	}
	else
	{
		nLength=strlen(m_requestheader);
	}
	memcpy(pHeader,m_requestheader,nLength);
	return nLength;
}

BOOL CHttpSocket::SendRequest(const char* pRequestHeader,long Length)
{
	if(!m_bConnected)
		return FALSE;

	if(pRequestHeader==NULL)
		pRequestHeader=m_requestheader;
	if(Length==0)
		Length=strlen(m_requestheader);

	if(Send(pRequestHeader,Length) < 0)
	{
		printf("SendRequest send() Failed!\r\n");
		return FALSE;
	}
	int nLength;
	GetResponseHeader(nLength);
	return TRUE;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

struct FILEHEAD{
	int		updatetype; //升级类型： 0x11 驱动  0x22  应用程序
	char	softCo[20];
	UINT64	softver;   //升级包版本号
	int		havereg;	//是否有注册表信息 注册表信息存储在一个文件中
	int		endiantype;  //程序运行的终端类型：0x88 门口机  0x99 室内机
};

static BOOL WRTParseURL(char * strUrl, char* strServer, char* strObject, unsigned short & port)
{
	char* pcur;
	int len;

	strUrl = strstr(strUrl, "http://");
	if(strUrl == NULL)
		return FALSE;
	strUrl += 7;

	pcur = strchr(strUrl, ':');
	if(pcur == NULL)
		return FALSE;
	len = pcur - strUrl;
	strncpy(strServer, strUrl, len);
	strServer[len] = 0;
	strUrl = pcur + 1;
	pcur = strchr(strUrl, '/');
	if(pcur == NULL)
	{
		pcur = strchr(strUrl, '\\');
		if(pcur == NULL)
			return FALSE;
	}

	len = pcur - strUrl;
	strncpy(strObject, strUrl, len);
	strObject[len] = 0;
	port = (short)strtol(strObject, NULL, 10);

	strcpy(strObject, pcur);

	return TRUE;
}

static BOOL HttpInit(CHttpSocket* pHttp, char *strHttpUrl)
{
	char szLine[256];
	int nLineSize = 0;
	char strServer[128],strObject[128];
	unsigned short nPort;
	const char *pRequestHeader = NULL;
	long nLength;

	sprintf(szLine, "%s", strHttpUrl);
	if(!WRTParseURL(szLine, strServer, strObject, nPort))
		return FALSE;
	printf("server:%s\r\n", strServer);
	printf("object:%s\r\n", strObject);
	printf("Port:%d\r\n", nPort);

	pRequestHeader = pHttp->FormatRequestHeader(strServer, strObject, nLength);	
	pHttp->Socket();
	if (!pHttp->Connect(strServer, nPort))
	{
		return FALSE;
	}
	pHttp->SendRequest();

	while(nLineSize != -1)
	{
		nLineSize = pHttp->GetResponseLine(szLine,256);
		if(nLineSize > -1)
		{
			szLine[nLineSize] = '\0';		
		}
	}
	return TRUE;
}

static BOOL HttpDownLoad(CHttpSocket* pHttp, char *strFileName, int nFileSize)
{
	FILE* DownloadFile;
	int nReceSize = 0, nWrite = 0;
	DWORD dwStartTime,dwEndTime;
	char* pData;

	int nCompletedSize = 0;

	DownloadFile = fopen(strFileName, "wb");
	if (DownloadFile == NULL)
		return FALSE;

	pData = (char*)malloc(TRANS_BUF_SIZE);
	//去掉打包工具加到image.dd前的文件头。

	if(strcmp(strFileName, DOWNLOAD_IMAGE_PATH) == 0)
	{
		int nFileHeadlen = 	sizeof(FILEHEAD);
		printf("nFileHeadlen is %d\r\n", nFileHeadlen);
		nReceSize = pHttp->Receive(pData, nFileHeadlen);
		if(nReceSize != nFileHeadlen)
		{
			printf("Receive buffer head error buflen is %d Fileheadlen is %d\r\n",nReceSize, nFileHeadlen);
			free(pData);
			return FALSE;
		}
		nFileSize -= nFileHeadlen;
	}

	while(nCompletedSize < nFileSize)
	{
		dwStartTime = DPGetTickCount();
		nReceSize = pHttp->Receive(pData, TRANS_BUF_SIZE);
		if(nReceSize == 0)
		{
			printf("Server Close Connect.\r\n");
			break;
		}
		if(nReceSize < 0)
		{
			printf("Recv Data Timerout.\r\n");
			break;
		}
		dwEndTime = DPGetTickCount();		
		if ((nWrite = fwrite(pData, 1, nReceSize, DownloadFile)) != nReceSize)
		{
			printf("Write File Error:%d\r\n",DPGetLastError());
			break;
		}
		nCompletedSize += nReceSize;
	}

	fclose(DownloadFile);
	free(pData);
	printf("Filesize:%d,CompletedSize:%d\r\n",nFileSize, nCompletedSize);
	if (nCompletedSize < nFileSize)
	{
		DPDeleteFile(strFileName);
		return FALSE;
	}
	return TRUE;
}

BOOL HttpDownloadFile(char *strFileName, char *strHttpUrl)
{
	CHttpSocket HttpSocket;
	char szValue[30];
	int nFileSize;

	if(!HttpInit(&HttpSocket, strHttpUrl))
		return FALSE;

	HttpSocket.GetField("Content-Length", szValue, 30);
	nFileSize = atoi(szValue);
	if (nFileSize < 1)
		return FALSE;

	return HttpDownLoad(&HttpSocket, strFileName, nFileSize);
}
