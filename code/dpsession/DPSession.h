#pragma once

#include "dpmsg.h"

void DPSessionInit(int ip);
void DPSessionUninit();
void DPSessionUserOp(DWORD cmd, int id, DWORD wParam = 0);
DWORD GetSessionId();
BOOL RequestTalk();
void ReleaseTalk();

BOOL DPTransVideoStart(int ip, int port, int cport, int lport, int lcport, BOOL bMulticast);
BOOL DPTransVideoStop();
void DPTransVideoSetRect(int left, int top, int width, int height);
void DPTransVideoEnable(BOOL bEnable);
DWORD GetIFrameData(char** pdata);

BOOL DPTransAudioStart(int ip, int port, int localport);
BOOL DPTransAudioStop();
void DPTransAudioSetVol(DWORD dwVol);

BOOL DPTransLiuyanStart(int ip, int port, int localport);
BOOL DPTransLiuyanStop();
void DPTransLiuyanWait();		// 等待留言文件保存完毕