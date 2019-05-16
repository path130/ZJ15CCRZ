#pragma once
#include "dpplatform.h"

BOOL JPGDecStart(int left, int top, int width, int height);
BOOL H264DecStart(int left, int top, int width, int height);
void H264WriteData(char * pdata,DWORD dlen, DWORD ptr);
void H264SetDisplayRect(int left, int top, int width, int height);
void H264DecStop(void);

HANDLE VideoReEncStart(DWORD dec_format, int decx, int decy, int decw, int dech, DWORD enc_format, int encw, int ench);
void VideoReEncStop(HANDLE hHandle);
void VideoReEncWrite(HANDLE hVideoEnc, BYTE* pdata, DWORD len);
DWORD VideoReEncRead(HANDLE hVideoEnc, BYTE* pdata, DWORD len, DWORD* property);


HANDLE VideoEncStart(DWORD format, DWORD width, DWORD height, DWORD qu);
DWORD VideoEncRead(HANDLE hHandle, BYTE* pData, DWORD len, DWORD* property);
void VideoEncStop(HANDLE hHandle);

DWORD VideoEncSetQuality(HANDLE hHandle, DWORD qu);
DWORD VideoEncEnable(HANDLE hHandle, BOOL bOn);
