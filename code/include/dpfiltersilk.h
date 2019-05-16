#pragma once
#include "dpplatform.h"

HANDLE SilkDecCreate();
int SilkDecRun(HANDLE hDecoder, char* pInBuf, int nInLen, char* pOutBuf);
void SilkDecDestroy(HANDLE hDecoder);

HANDLE SilkEncCreate();
int SilkEncRun(HANDLE hEncoder, char* pInBuf, int nInLen, char* pOutBuf);
void SilkEncDestroy(HANDLE hEncoder);