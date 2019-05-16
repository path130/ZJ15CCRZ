#pragma once

class  CHttpSocket  
{
public:
	CHttpSocket();
	virtual ~CHttpSocket();
	int				GetServerState();						
	int				GetField(const char* szSession,char *szValue,int nMaxLength);
	int				GetResponseLine(char *pLine,int nMaxLength);				
	const char*		GetResponseHeader(int &Length);	
	const char *	FormatRequestHeader(char *pServer,char *pObject,long &Length,char* pCookie=NULL,char *pReferer=NULL,long nFrom=0,long nTo=0,int nServerType=0);								
	int				GetRequestHeader(char *pHeader,int nMaxLength) const;
	BOOL			SendRequest(const char* pRequestHeader = NULL,long Length = 0);

	BOOL			SetTimeout(int nTime,int nType=0);
	long			Receive(char* pBuffer,long nMaxLength);
	int				Send(const char * pdata,int dlen);
	BOOL			Connect(char* szHostName,int nPort=80);
	BOOL			Socket();
	BOOL			CloseSocket();

protected:	

	char m_requestheader[1024];
	char m_ResponseHeader[1024];
	BOOL m_bResponsed;
	int m_nResponseHeaderSize;
	int m_nCurIndex;

	unsigned short m_port;
	SOCKET m_sock;
	BOOL m_bConnected;
	DWORD m_time;
};

