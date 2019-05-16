#pragma once

#include "dpgraphic.h"

typedef struct {
	int pixelDepth;
    int imageWidth;
    int imageHeight;
}PngImageInfo;

HANDLE CreatePng(void);
BOOL LoadPng(HANDLE, char* fileName);
BOOL GetPngInfo(HANDLE, PngImageInfo *);
BOOL DrawPng(HANDLE, Block_Req * pBlock);
void ReleasePng(HANDLE);
void DestroyPng(HANDLE);

