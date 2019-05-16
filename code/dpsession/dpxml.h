#pragma once

void InitXmlServer(DPMedia* pMedia);
void UnInitXmlServer();

DWORD ParseCmd(char* pContent);
BOOL GetXmlString(const char* pbuf, const char* pmatch, char* pStr, int nStr);
BOOL GetXmlNumber(const char* pbuf, const char* pmatch, int* val);
BOOL CompareXmlID(char* pContent, DPSession* pSession);
BOOL GetXmlMediaInfo(DPMedia* pMedia, char* pContent);
void BuildPacketXml(char* pContent, DWORD cmd, int lid, int rid, int remotemsgid, DPSession* pSession);
