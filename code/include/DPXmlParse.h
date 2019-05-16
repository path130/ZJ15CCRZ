#pragma once

typedef struct _XmlNode
{
	char *name;
	char *content;
	BOOL isLeaf;
	struct _XmlNode* pChild;
	struct _XmlNode* pBrother;
} XmlNode;

class CXmlParse
{
public:
	CXmlParse()
	{
		m_pRoot = NULL;
		m_pLocate = NULL;
	}
	~CXmlParse()
	{
		if(m_pRoot != NULL)
		{
			FreeAllNode(m_pRoot);
			if(m_pRoot->name != NULL)
				delete [] m_pRoot->name;
			if(m_pRoot->content != NULL)
				delete [] m_pRoot->content;
			free(m_pRoot);
			m_pRoot = NULL;
		}
	}
	int Init(char* src);
	char* GetLocateName(void);
	BOOL SetLocate(char* str);
	BOOL SetRoot(char* str);
	char* GetNodeContent(char* str);
	BOOL RootCheck(char* str);
	BOOL NextNode(void);
private:
	char* dupStr(char* str);
	char* XmlNext(char * pstrXml, char ** ppstrKey, int & iused);
	BOOL ParseChild(XmlNode* pParent, char* src);
	void FreeAllNode(XmlNode* pParent);
	XmlNode* m_pRoot;
	XmlNode* m_pLocate;
};

