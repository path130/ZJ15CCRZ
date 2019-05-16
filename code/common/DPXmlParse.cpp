#include "roomlib.h"
#include "DPXmlParse.h"

char* CXmlParse::dupStr(char* str)
{
	char* newstr = new char[strlen(str) + 1];
	strcpy(newstr, str);
	return newstr;
}

char* CXmlParse::XmlNext(char * pstrXml, char ** ppstrKey, int & iused)
{
	char * pStar,*pEnd;
	char* pKey;
	char key[64];
	char * p = strchr(pstrXml,'<');

	if(p == NULL)
		return NULL;
	pStar = ++p;

	p = strchr(pStar,'>');
	if(p == NULL)
		return NULL;
	pEnd = p;

	pKey = new char[pEnd - pStar + 1];
	strncpy(pKey,pStar,pEnd - pStar);
	pKey[pEnd - pStar] = 0;

	pStar = pEnd + 1;

	sprintf(key,"</%s>",pKey);
	pEnd = strstr(pStar,key);
	if(pEnd == NULL)
	{
		delete [] pKey;
		return NULL;
	}

	p = new char[pEnd - pStar + 1];
	if( p == NULL)
	{
		delete [] pKey;
		return NULL;
	}
	strncpy(p,pStar,pEnd - pStar);
	p[pEnd - pStar] = 0;
	iused = pEnd - pstrXml + strlen(key);
	*ppstrKey = pKey;
	return p;
}

void CXmlParse::FreeAllNode(XmlNode* pParent)
{
	XmlNode* pchild, *plast;

	if(pParent == NULL)
		return;
	pchild = pParent->pChild;
	while(pchild != NULL)
	{
		FreeAllNode(pchild);
		if(pchild->name != NULL)
			delete []pchild->name;
		if(pchild->content != NULL)
			delete [] pchild->content;
		plast = pchild;
		pchild = plast->pBrother;
		free(plast);
	}
}

BOOL CXmlParse::ParseChild(XmlNode* pParent, char* src)
{
	XmlNode* pNode;
	char * pChildXml = NULL;
	char * pStar = src;
	char* wczKey;
	int used;
	BOOL isBranch = FALSE;

	while(1)
	{
		if(pChildXml = XmlNext(pStar, &wczKey, used))
		{
			pNode = (XmlNode*)malloc(sizeof(XmlNode));
			memset(pNode, 0, sizeof(XmlNode));
			pNode->name = wczKey;
			ParseChild(pNode, pChildXml);
			if(pParent->pChild == NULL)
				pParent->pChild = pNode;
			else
			{
				XmlNode* pBrother = pParent->pChild;
				while(pBrother->pBrother != NULL)
					pBrother = pBrother->pBrother;
				pBrother->pBrother = pNode;
			}
			pStar += used;
			isBranch = TRUE;
			delete pChildXml;
		}
		else if(!isBranch)
		{
			pParent->isLeaf = TRUE;
			pParent->content = dupStr(src);
			break;
		}
		else
			break;
	}
	return TRUE;
}

int CXmlParse::Init(char* src)
{
	char * pChildXml = NULL;
	char * pStar = src;
	char* wczKey;
	int used;
	XmlNode* pNode;

	pStar = strstr(src, "<msg>");
	if(pStar == NULL)
		return 0;
	if(pChildXml = XmlNext(pStar, &wczKey, used))
	{
		pNode = (XmlNode*)malloc(sizeof(XmlNode));
		memset(pNode, 0, sizeof(XmlNode));
		m_pRoot = pNode;
		pNode->name = wczKey;
		ParseChild(pNode, pChildXml);
		m_pLocate = m_pRoot;
		delete pChildXml;
	}
	return used;
}

BOOL CXmlParse::SetLocate(char* str)
{
	XmlNode* pChild;
	pChild = m_pLocate->pChild;
	while(pChild != NULL)
	{
		if(strcmp(str, pChild->name) == 0)
		{
			m_pLocate = pChild;
			return TRUE;
		}
		pChild = pChild->pBrother;
	}
	return FALSE;
}

BOOL CXmlParse::NextNode(void)
{
	if(m_pLocate->pBrother != NULL)
	{
		m_pLocate = m_pLocate->pBrother;
		return TRUE;
	}
	return FALSE;
}

char* CXmlParse::GetLocateName(void)
{
	if(m_pLocate != NULL)
		return m_pLocate->name;
	else
		return NULL;
}

char* CXmlParse::GetNodeContent(char* str)
{
	XmlNode* pChild;
	pChild = m_pLocate->pChild;
	while(pChild != NULL)
	{
		if(strcmp(str, pChild->name) == 0)
		{
			return pChild->content;
		}
		pChild = pChild->pBrother;
	}
	return NULL;
}

BOOL CXmlParse::SetRoot(char* str)
{
	XmlNode* pChild;
	pChild = m_pRoot->pChild;
	while(pChild != NULL)
	{
		if(strcmp(str, pChild->name) == 0)
		{
			m_pLocate = pChild;
			return TRUE;
		}
		pChild = pChild->pBrother;
	}
	return FALSE;
}

BOOL CXmlParse::RootCheck(char* str)
{
	if(strcmp(str, m_pRoot->name) == 0)
		return TRUE;
	return FALSE;
}


