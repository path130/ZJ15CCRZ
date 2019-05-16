#include <roomlib.h>
#include "dpgraphic.h"
#include "pngimage.h"
#include "dptext.h"
#include "dpgpio.h"

int JpegDecoderRGB(char *filename, int *width, int *height, char **outbuf);
void backtrace();
void print_trace (void);

#define	FRAME_CHECKID			0x53505254
//#define	PNGCACHE
#define	PNG_DIR					"/FlashDev/Image/"
#define	TTF_FILE				"/FlashDev/FONTS/MSYH2.TTF"

#define	SPRITE_WIDTH_LIMIT		512
#define	MAX_CHARSIZE			120
#define	MAX_WIDTH				1024
#define CHIP_X3

#pragma pack(1)
typedef struct 
{
	WORD bfType; 
	DWORD bfSize; 
	WORD bfReserved1; 
	WORD bfReserved2; 
	DWORD bfOffBits; 
} BmpHead; 

typedef struct
{ 
  DWORD biSize; 
  LONG biWidth; 
  LONG biHeight; 
  WORD biPlanes; 
  WORD biBitCount;
  DWORD biCompression; 
  DWORD biSizeImage; 
  LONG biXPelsPerMeter; 
  LONG biYPelsPerMeter; 
  DWORD biClrUsed; 
  DWORD biClrImportant; 
} BmpInfo; 
#pragma pack()

class ContentManage1
{
public:
	ContentManage1()
	{
		count = 0;
	}
	~ContentManage1(){}
	void Init(char* psrc)
	{
		ExtractParam(psrc);
	}
	char* FindContentByName(char* name)
	{
		DWORD i;
		for(i = 0; i < count; i++)
		{
			if(strcmp(pname[i], name) == 0)
				return pcontent[i];
		}
		return NULL;
	}
private:
	void ExtractParam(char* psrc)
	{
		count = 0;
		while(1)
		{
			if((psrc = strchr(psrc, '"')) == NULL)
				break;
			psrc++;
			pname[count] = psrc;
			if((psrc = strchr(psrc, '=')) == NULL)
				break;
			*psrc = 0;
			psrc++;
			pcontent[count] = psrc;
			if((psrc = strchr(psrc, '"')) == NULL)
				break;
			*psrc = 0;
			psrc++;
			count++;
		}
	}
	
	DWORD count;
	char* pname[32];
	char* pcontent[32];
};

// 将非2的幂值转换为2的幂
LONG CDPGraphic::convertmatch(DWORD width)
{
	LONG i;

	for(i = 10; i > 2; i--)
	{
		if(width & (1 << i))
			break;
	}
	if(width == (1 << i))
		return width;
	else
		return (1 << (i + 1));
}

BOOL CDPGraphic::Mix(RGBQUAD * pDst,RGBQUAD * pSrc,RGBQUAD * pSrc2)
{
	if(pSrc->rgbReserved < 5 || pSrc2->rgbReserved > 250)
		*pDst = *pSrc2;
	else if(pSrc2->rgbReserved < 5)
		*pDst = *pSrc;
	else
	{
		int a0,a1,a2;
		a2 = pSrc2->rgbReserved;
		a1 = (pSrc->rgbReserved * (255 - a2)) >> 8;
		a0 = a2 + a1;
		pDst->rgbReserved = a0;
		pDst->rgbBlue	= (BYTE)((pSrc2->rgbBlue  * a2	+ a1 * pSrc->rgbBlue  )/a0);
		pDst->rgbGreen	= (BYTE)((pSrc2->rgbGreen * a2	+ a1 * pSrc->rgbGreen )/a0);
		pDst->rgbRed	= (BYTE)((pSrc2->rgbRed	  * a2	+ a1 * pSrc->rgbRed   )/a0);
		return TRUE;
	}
	return FALSE;
}

// 将彩色点转换为黑白
void CDPGraphic::RGB2BW(RGBQUAD * pDst,RGBQUAD * pSrc)
{
	if(pSrc->rgbReserved < 5)
		*pDst = *pSrc;
	else
	{
		WORD y;
		y = (pSrc->rgbRed * 76 + pSrc->rgbGreen * 150 + pSrc->rgbBlue * 30) >> 8;
		pDst->rgbReserved = pSrc->rgbReserved;
		pDst->rgbBlue	= (BYTE)y;
		pDst->rgbGreen	= (BYTE)y;
		pDst->rgbRed	= (BYTE)y;
	}
}

void CDPGraphic::RGB2Color(RGBQUAD * pDst,RGBQUAD * pSrc)
{
	pDst->rgbBlue	= pSrc->rgbBlue;
	pDst->rgbGreen	= pSrc->rgbGreen;
	pDst->rgbRed	= pSrc->rgbRed;
}

// 将一个点的红色和蓝色调换
void CDPGraphic::RGB2Revert(RGBQUAD * pDst,RGBQUAD * pSrc)
{
	pDst->rgbReserved = pSrc->rgbReserved;
	pDst->rgbBlue	= pSrc->rgbRed;
	pDst->rgbGreen	= pSrc->rgbGreen;
	pDst->rgbRed	= pSrc->rgbBlue;
	//pDst->rgbReserved = pSrc->rgbReserved;
	//pDst->rgbBlue	= (pSrc->rgbBlue>> 1) + (pSrc->rgbBlue>> 2);
	//pDst->rgbGreen	= (pSrc->rgbGreen>> 1) + (pSrc->rgbGreen>> 2);
	//pDst->rgbRed	= (pSrc->rgbRed>> 1) + (pSrc->rgbRed>> 2);
}

CDPGraphic::CDPGraphic()
{
	HardBlockHead = NULL;
	SoftBlockHead = NULL;
	FreeBlockHead = NULL;
	m_strtemp = NULL;
	g_png = NULL;
	g_pngcount = 0;
	m_hScale = NULL;
	m_hPng = CreatePng();
	return;
}

CDPGraphic::~CDPGraphic()
{
	if(HardBlockHead != NULL)
	{
		Block_Req* frame;
		printf("~CDPGraphic: User dont close all block\r\n");
		while(HardBlockHead)
		{
			frame = HardBlockHead;
			HardBlockHead = frame->pnext;
			DPVirtualFree(frame->viraddr);
			frame->id = 0;
			free(frame);
		}
	}
	if(h_hspr != NULL)
		DeinitSpr(h_hspr);
	h_hspr = NULL;
	DestroyPng(m_hPng);
	UiniteWRText();
}

void CDPGraphic::XmlLoadJpg(Block_Req* pframe, HANDLE pHandle)
{
	ContentManage1* pCm = (ContentManage1*)pHandle;
	char* pcontent;
	char wholename[128];
	int jwidth, jheight;
	char* pjpg;
	DWORD* pstart;
	char* psrc;
	BYTE* pdst;
	int left, top;
 	int i, j;

	if((pcontent = pCm->FindContentByName((char*)"name")) == NULL)
		return;
	sprintf(wholename, "%s%s", PNG_DIR, pcontent);

	if(JpegDecoderRGB(wholename, &jwidth, &jheight, &pjpg) == 0)
		return;
	if((pcontent = pCm->FindContentByName((char*)"left")) == NULL)
		left = 0;
	else
		left = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName((char*)"top")) == NULL)
		top = 0;
	else
		top = strtol(pcontent, NULL, 10);

 	pstart = pframe->viraddr + pframe->win.cx * top + left;

	for(i = 0; i < jheight && i < (pframe->win.cy - top); i++)
	{
		psrc = pjpg + 3 * i * jwidth;
		pdst = (BYTE*)(pstart + i * FRAME_WIDTH);
		for(j = 0; j < jwidth && j < (pframe->win.cx - left); j++)
		{
			pdst[2] = *psrc++;
			pdst[1] = *psrc++;
			pdst[0] = *psrc++;
			pdst[3] = 0xff;
			pdst += 4;
		}
	}
	free(pjpg);
}

void CDPGraphic::XmlLoadColor(Block_Req* pframe, HANDLE pHandle)
{
	ContentManage1* pCm = (ContentManage1*)pHandle;
	char* pcontent;
	DWORD color;
	RECT rect;

	if((pcontent = pCm->FindContentByName((char*)"left")) == NULL)
		return;
	rect.left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"top")) == NULL)
		return;
	rect.top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"width")) == NULL)
		return;
	rect.right = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"height")) == NULL)
		return;
	rect.bottom = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"color")) == NULL)
		return;
	color = hexconvert(pcontent);
	FillSolidRect(pframe, &rect, color);
}

void CDPGraphic::XmlLoadPng(Block_Req* pframe, HANDLE pHandle)
{
	ContentManage1* pCm = (ContentManage1*)pHandle;
	char* pcontent;
	char wholename[128];
	int left, top;

	Block_Req *hspr;
	PngImageInfo info;

	if((pcontent = pCm->FindContentByName((char*)"name")) == NULL)
		return;
	sprintf(wholename, "%s%s", PNG_DIR, pcontent);

	if(TRUE == LoadPng(m_hPng, (char*)wholename))
	{
		GetPngInfo(m_hPng, &info);
		hspr = (Block_Req*)ReqTempBlk(info.imageWidth, info.imageHeight);
		if(hspr != NULL)
			DrawPng(m_hPng, hspr);
		ReleasePng(m_hPng);
	}
	else
		return;

	if((pcontent = pCm->FindContentByName((char*)"left")) == NULL)
		left = (FRAME_WIDTH - info.imageWidth)/2;
	else
		left = strtol(pcontent, NULL, 10);
	if((pcontent = pCm->FindContentByName((char*)"top")) == NULL)
		top = (FRAME_HEIGHT - info.imageHeight)/2;
	else
		top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"isalpha")) == NULL)
		BitBlt(pframe, left, top, info.imageWidth, info.imageHeight, hspr, 0, 0);
	else
		AlphaBlend(pframe, left, top, info.imageWidth, info.imageHeight, hspr, 0, 0);
	CloseBlk(hspr);
}

void CDPGraphic::XmlLoadText(Block_Req* pframe, HANDLE pHandle)
{
	ContentManage1* pCm = (ContentManage1*)pHandle;
	char* pcontent;
	int left, top;
	int size;
	int text;
	DWORD color;
	HANDLE hspr;
	DWORD width;

	if((pcontent = pCm->FindContentByName((char*)"name")) == NULL)
		return;
	text = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"size")) == NULL)
		return;
	size = strtol(pcontent, NULL, 10);
	
	if((pcontent = pCm->FindContentByName((char*)"left")) == NULL)
		return;
	left = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"top")) == NULL)
		return;
	top = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"color")) == NULL)
		color = 0xffffff;
	else
		color = hexconvert(pcontent);

	hspr = LoadStr(GetStringByID(text), size, color, &width);
	AlphaBlend(pframe, left - width/2, top, width, size, hspr, 0, 0);
}

Block_Req* CDPGraphic::XmlInitFrame(HANDLE pHandle)
{
	ContentManage1* pCm = (ContentManage1*)pHandle;
	char* pcontent;
	DWORD width, height;

	if((pcontent = pCm->FindContentByName((char*)"width")) == NULL)
		width = FRAME_WIDTH;
	else
		width = strtol(pcontent, NULL, 10);

	if((pcontent = pCm->FindContentByName((char*)"height")) == NULL)
		height = FRAME_HEIGHT;
	else
		height = strtol(pcontent, NULL, 10);

	return (Block_Req*)ReqTempBlk(width, height);
}

Block_Req* CDPGraphic::LoadXmlFrame(char* pname)
{
	ContentManage1 pCm;
	char temp[512];
	char* pdesc;
	char* pbuf;
	int flen;
	int i;
	Block_Req* frame = NULL;

	flen = BReadFile(pname, &pbuf);
	if(flen == 0)
	{
		return NULL;
	}

	for(i = 0; i < flen/2; i++)
	{
		if((pbuf[i] >= 0x41)
			&& (pbuf[i] <= 0x5a))
			pbuf[i] = pbuf[i] | 0x20;
	}

	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, (char*)"frame", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		frame  = XmlInitFrame(&pCm);
	}

	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, (char*)"jpg", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		XmlLoadJpg(frame, &pCm);
	}

	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, (char*)"bkcolor", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		XmlLoadColor(frame, &pCm);
	}

	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, (char*)"png", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		XmlLoadPng(frame, &pCm);
	}

	pdesc = pbuf;
	while(1)
	{
		pdesc = CFindContent(pdesc, (char*)"text", temp, FALSE);
		if(pdesc == NULL)
			break;
		pCm.Init(temp);
		XmlLoadText(frame, &pCm);
	}
	return frame;
}

// 缓存所有的png图片
void CDPGraphic::CacheAllPng()
{
	char szFind[MAX_PATH];
	char FindFileData[64];
	HANDLE hFind;
	Block_Req *hspr;
	PngImageInfo info;
	DWORD mtotal, mfree;

	DumpMemory(&mtotal, &mfree);
	DBGMSG(DPINFO, "Berore Cache  %u %u %u", mfree*100/mtotal, mfree, mtotal);
	hFind = DPFindFirstFile((char*)PNG_DIR, FindFileData);
	if(INVALID_HANDLE_VALUE != hFind) 
	{
		while(TRUE)
		{
			if(strstr(FindFileData, ".png") != NULL)
			{
				DBGMSG(DISP_MOD, "cachepng %s\r\n", FindFileData);
				sprintf(szFind, "%s%s", PNG_DIR, FindFileData);
				hspr = NULL;
				if(TRUE == LoadPng(m_hPng, szFind))
				{
					GetPngInfo(m_hPng, &info);
					hspr = (Block_Req*)ReqTempBlk(info.imageWidth, info.imageHeight);
					if(hspr != NULL)
						DrawPng(m_hPng, hspr);
				}
				ReleasePng(m_hPng);
				if(hspr != NULL)
				{
					hspr->issystem = TRUE;
					g_png = (PNG_CACHE*)realloc(g_png, (g_pngcount + 1) * sizeof(PNG_CACHE));
					g_png[g_pngcount].pframe = hspr;
					strcpy(g_png[g_pngcount].filename, FindFileData);
					g_pngcount++;
				}
			}
			else if(strstr(FindFileData, ".xml") != NULL)
			{
				sprintf(szFind, "%s%s", PNG_DIR, FindFileData);
				hspr = LoadXmlFrame(szFind);
				if(hspr != NULL)
				{
					hspr->issystem = TRUE;
					g_png = (PNG_CACHE*)realloc(g_png, (g_pngcount + 1) * sizeof(PNG_CACHE));
					g_png[g_pngcount].pframe = hspr;
					strcpy(g_png[g_pngcount].filename, FindFileData);
					g_pngcount++;
				}
			}
			if(!DPFindNextFile(hFind, FindFileData))
				break;
		}
		DPFindClose(hFind);
	}
	else
		printf("[APP]GetDirFilePath ring file:%s Invalid handle and should not happend\n",szFind);
	DumpMemory(&mtotal, &mfree);
	DBGMSG(DPINFO, "After cachepng %u %u %u", mfree*100/mtotal, mfree, mtotal);
}

BOOL CDPGraphic::Inite()
{
	h_hspr = InitSpr();
	if(!IniteWRText((char*)TTF_FILE))
	{
		printf("[APP]IniteWRText  error\n");
		return FALSE;
	}
	m_strtemp = (Block_Req*)ReqTempBlk(MAX_WIDTH, MAX_CHARSIZE);
	m_dwCharHeight = MAX_CHARSIZE;
	m_strtemp->issystem = TRUE;
#ifdef PNGCACHE
	CacheAllPng();
#endif
	return TRUE;
}

HANDLE CDPGraphic::ReqBakFrame(LONG x, LONG y, LONG w, LONG h)
{
	Block_Req* frame;
	DWORD memsize;

	if((w == 0)
		|| (h == 0))
		return NULL;

	frame = FreeBlockHead;
	if(frame == NULL)
	{
		frame = (Block_Req*)malloc(sizeof(Block_Req));
	}
	else
		FreeBlockHead = FreeBlockHead->pnext;

	memset(frame, 0, sizeof(Block_Req));
#ifdef ROTATE_ANGLE
	frame->lt.x = y;
	frame->lt.y = SCREEN_HEIGHT - w - x;
	frame->win.cx = h;
	frame->win.cy = w;
#else
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = w;
	frame->win.cy = h;
#endif

	memsize = (frame->win.cx * frame->win.cy * 4 + 0xfff) & 0xfffff000;
	if(!DPPhisycalAlloc(h_hspr, frame, memsize, 0))
	{
		printf("SprReqRollBar VirtualAlloc %d fail\r\n", memsize);
		frame->pnext = FreeBlockHead;
		FreeBlockHead = frame;
		return NULL;
	}

	frame->id = FRAME_CHECKID;
	frame->isHard = TRUE;
#ifdef ROTATE_ANGLE
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = w;
	frame->win.cy = h;
#endif
	frame->pnext = HardBlockHead;
	HardBlockHead = frame;
	return frame;
}

HANDLE CDPGraphic::ReqFrame(LONG x, LONG y, LONG w, LONG h)
{
	Block_Req* frame;
	DWORD memsize;

	if((w == 0)
		|| (h == 0))
		return NULL;

	frame = FreeBlockHead;
	if(frame == NULL)
		frame = (Block_Req*)malloc(sizeof(Block_Req));
	else
		FreeBlockHead = FreeBlockHead->pnext;
	memset(frame, 0, sizeof(Block_Req));

#ifdef ROTATE_ANGLE
	frame->lt.x = y;
	frame->lt.y = SCREEN_HEIGHT - w - x;
	frame->win.cx = h;
	frame->win.cy = w;
#else
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = w;
	frame->win.cy = h;
#endif

	memsize = (frame->win.cx * frame->win.cy * 4 + 0xfff) & 0xfffff000;
	if(!DPPhisycalAlloc(h_hspr, frame, memsize, 1))
	{
		printf("SprReqFrame VirtualAlloc %d fail\r\n", memsize);
		frame->pnext = FreeBlockHead;
		FreeBlockHead = frame;
		return NULL;
	}

	frame->id = FRAME_CHECKID;
	frame->isHard = TRUE;
#ifdef ROTATE_ANGLE
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = w;
	frame->win.cy = h;
#endif
	frame->pnext = HardBlockHead;
	HardBlockHead = frame;
	return frame;
}

HANDLE CDPGraphic::ReqCtrl(LONG x, LONG y, LONG w, LONG h)
{
	Block_Req* frame;
	DWORD memsize;

	if((w == 0)
		|| (h == 0))
		return NULL;

#ifndef	DP3000_CHIP
#ifndef CHIP_X3
	if(w > SPRITE_WIDTH_LIMIT)
	{
		return NULL;
	}
#endif
#endif

	frame = FreeBlockHead;
	if(frame == NULL)
	{
		frame = (Block_Req*)malloc(sizeof(Block_Req));
	}
	else
		FreeBlockHead = FreeBlockHead->pnext;
	memset(frame, 0, sizeof(Block_Req));

#ifdef DP3000_CHIP
	#ifdef ROTATE_ANGLE
	frame->lt.x = y;
	frame->lt.y = SCREEN_HEIGHT - w - x;
	frame->win.cx = h;
	frame->win.cy = w;
	#else
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = w;
	frame->win.cy = h;
	#endif
#else
	#ifdef CHIP_X3
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = w;
	frame->win.cy = h;
	#else
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = convertmatch(w);
	frame->win.cy = convertmatch(h);
	#endif
#endif

	memsize = (frame->win.cx * frame->win.cy * 4 + 0xfff) & 0xfffff000;
	if(!DPPhisycalAlloc(h_hspr, frame, memsize, 2))
	{
		printf("SprReqFrame VirtualAlloc %d fail\r\n", memsize);
		frame->pnext = FreeBlockHead;
		FreeBlockHead = frame;
		return NULL;
	}

	frame->id = FRAME_CHECKID;
	frame->isHard = TRUE;
#ifdef ROTATE_ANGLE
	frame->lt.x = x;
	frame->lt.y = y;
	frame->win.cx = w;
	frame->win.cy = h;
#endif
	frame->pnext = HardBlockHead;
	HardBlockHead = frame;
	return frame;
}

HANDLE CDPGraphic::ReqTempBlk(LONG w, LONG h)
{
	Block_Req* frame;
	DWORD size;

	if((w == 0)
		|| (h == 0))
		return NULL;

	frame = FreeBlockHead;
	if(frame == NULL)
	{
		frame = (Block_Req*)malloc(sizeof(Block_Req));
	}
	else
		FreeBlockHead = FreeBlockHead->pnext;
	memset(frame, 0, sizeof(Block_Req));

	frame->lt.x = 0;
	frame->lt.y = 0;
	frame->win.cx = w;
	frame->win.cy = h;

	size = frame->win.cx * frame->win.cy * 4;
	size = (size + 0xfff) & 0xffff000;
#ifdef DP3000_CHIP
	frame->viraddr = (DWORD*)VirtualAlloc(0, size, MEM_RESERVE,PAGE_NOACCESS);
	if(frame->viraddr == NULL)
	{
		printf("ReqTempBlk VirtualAlloc %d fail\r\n", size);
		frame->pnext = FreeBlockHead;
		FreeBlockHead = frame;
		return NULL;
	}

	if(!DeviceIoControl(h_hspr, IOCTL_TEMP_FRM_REQUEST, frame, sizeof(Block_Req), NULL, NULL, NULL, NULL))
	{
		VirtualFree(frame->viraddr, 0, MEM_RELEASE);
		frame->pnext = FreeBlockHead;
		FreeBlockHead = frame;
		return NULL;
	}
#else
	frame->viraddr = (DWORD*)DPVirtualAlloc(size);
	if(frame->viraddr == NULL)
	{
		frame->pnext = FreeBlockHead;
		FreeBlockHead = frame;
		return NULL;
	}
#endif

	frame->id = FRAME_CHECKID;
	frame->pnext = SoftBlockHead;
	SoftBlockHead = frame;
	return frame;
}

void CDPGraphic::CloseBlk(HANDLE hframe)
{
	Block_Req* frame = (Block_Req*)hframe;
	Block_Req* cur, *prev;

	if(frame == NULL)
		return;
	if(frame->id != FRAME_CHECKID)
		return;
	if(frame->issystem)
		return;

	if(frame->isHard)
	{
		prev = NULL;
		cur = HardBlockHead;
		while(cur != NULL)
		{
			if(cur == frame)
				break;
			prev = cur;
			cur = cur->pnext;
		}
		if(cur != NULL)
		{
			if(prev == NULL)
				HardBlockHead = cur->pnext;
			else
				prev->pnext = cur->pnext;
			DPPhisycalFree(h_hspr, cur);
			cur->viraddr = NULL;
			cur->id = 0;

			cur->pnext = FreeBlockHead;
			FreeBlockHead = cur;
		}
		else
			 printf("SprCloseBlk dont find given hard hframe\r\n");
	}
	else
	{
		prev = NULL;
		cur = SoftBlockHead;
		while(cur != NULL)
		{
			if(cur == frame)
				break;
			prev = cur;
			cur = cur->pnext;
		}
		if(cur != NULL)
		{
			if(prev == NULL)
				SoftBlockHead = cur->pnext;
			else
				prev->pnext = cur->pnext;
			
#ifdef DP3000_CHIP
			DeviceIoControl(h_hspr, IOCTL_TEMP_FRM_RELEASE, cur, sizeof(Block_Req), NULL, NULL, NULL, NULL);
#endif
			DPVirtualFree(frame->viraddr);
			cur->pnext = FreeBlockHead;
			FreeBlockHead = cur;
		}
		else
			 printf("SprCloseBlk dont find given soft hframe\r\n");
	}
}

void CDPGraphic::MoveBlk(HANDLE hframe, LONG x, LONG y)
{
	Block_Req* frame = (Block_Req*)hframe;
	if(frame == NULL)
		return;
	if(frame->id != FRAME_CHECKID)
		return;
	frame->lt.x = x;
	frame->lt.y = y;
	DPSprCtrl(h_hspr, IOCTL_BLK_MOVE, frame, sizeof(Block_Req));
}

void CDPGraphic::ShowBlk(HANDLE hframe)
{
	Block_Req* frame = (Block_Req*)hframe;

	if(frame == NULL)
		return;
	if(frame->id != FRAME_CHECKID)
		return;
	DPSprCtrl(h_hspr, IOCTL_BLK_SHOW, frame, sizeof(Block_Req));
}

void CDPGraphic::HideBlk(HANDLE hframe)
{
	Block_Req* frame = (Block_Req*)hframe;
	if(frame == NULL)
		return;
	if(frame->id != FRAME_CHECKID)
		return;
	DPSprCtrl(h_hspr, IOCTL_BLK_HIDE, frame, sizeof(Block_Req));
}

void CDPGraphic::Show(void)
{
	DPSprCtrl(h_hspr, IOCTL_DEV_SHOW, NULL, 0);
}

void CDPGraphic::Hide(void)
{
	DPSprCtrl(h_hspr, IOCTL_DEV_HIDE, NULL, 0);
}

void CDPGraphic::HideGDI(void)
{
	DPSprCtrl(h_hspr, IOCTL_B0_HIDE, NULL, 0);
}

void CDPGraphic::ShowGDI(void)
{
	DPSprCtrl(h_hspr, IOCTL_B0_SHOW, NULL, 0);
}

void CDPGraphic::SetDisClosed(HANDLE frame, BOOL sys)
{
	Block_Req* pframe = (Block_Req*)frame;
	pframe->issystem = sys;
}

void CDPGraphic::SetBK(HANDLE hframe, DWORD val)
{
	Block_Req* frame = (Block_Req*)hframe;
	RECT rect;

	if(frame == NULL)
		return;
	if(frame->id != FRAME_CHECKID)
		return;

	rect.left = 0;
	rect.top = 0;
	rect.right = frame->win.cx;
	rect.bottom = frame->win.cy;
	FillSolidRect(hframe, &rect, val);
}

BOOL CDPGraphic::BitBlt(HANDLE dstSpr,int nXDest,int nYDest,int nWidth,int nHeight,HANDLE srcSpr,int nXSrc,int nYSrc)
{
	Block_Req * dstframe = (Block_Req*)dstSpr;
	Block_Req * srcframe = (Block_Req*)srcSpr;

	if(srcframe == NULL || dstframe == NULL)
	{
		printf("BitBlt frame error\r\n");
//		print_trace();
		return FALSE;
	}
	if(srcframe->id != FRAME_CHECKID || dstframe->id != FRAME_CHECKID)
	{
		printf("BitBlt id error\r\n");
		return FALSE;
	}

	if(nXSrc + nWidth > srcframe->win.cx || nYSrc + abs(nHeight) > srcframe->win.cy)
	{
		printf("BitBlt src error\r\n");
		return FALSE;
	}

	if(nXDest + nWidth > dstframe->win.cx || nYDest + abs(nHeight) > dstframe->win.cy)
	{
		printf("BitBlt dst error\r\n");
		return FALSE;
	}

	if((nWidth == 0) || (nHeight== 0))
		return FALSE;

#ifdef DP3000_CHIP
	Ctrl_Op  pctrl;
	memset(&pctrl,0,sizeof(Ctrl_Op));
	pctrl.op = BITBLT;
	pctrl.dstframe = dstframe->rethd;
	pctrl.srcframe = srcframe->rethd;
#ifdef	ROTATE_ANGLE
	if(dstframe->isHard)
	{
		DWORD w, h;
		pctrl.op = ROTATE_BLT;
		w = dstframe->win.cy;
		h = dstframe->win.cx;
		pctrl.dststartx = nYDest;
		pctrl.dststarty = h - nXDest - nWidth;
		pctrl.srcstartx = nXSrc;
		pctrl.srcstarty = nYSrc;
		pctrl.width = nWidth;
		pctrl.height = nHeight;
	}
	else if(srcframe->isHard)
	{
		DWORD w, h;
		pctrl.op = ROTATE_BLT;
		w = dstframe->win.cy;
		h = dstframe->win.cx;
		pctrl.dststartx = nYDest;
		pctrl.dststarty = h - nXDest - nWidth;
		pctrl.srcstartx = nXSrc;
		pctrl.srcstarty = nYSrc;
		pctrl.width = nWidth;
		pctrl.height = nHeight;
	}
	else
	{
		pctrl.dststartx = nXDest;
		pctrl.dststarty = nYDest;
		pctrl.srcstartx = nXSrc;
		pctrl.srcstarty = nYSrc;
		pctrl.width = nWidth;
		pctrl.height = nHeight;
	}
#else
	while(nWidth > 512)
	{
		pctrl.dststartx = nXDest;
		nXDest += 512;
		pctrl.dststarty = nYDest;
		pctrl.srcstartx = nXSrc;
		nXSrc += 512;
		pctrl.srcstarty = nYSrc;
		pctrl.width = 512;
		nWidth-=512;
		pctrl.height = nHeight;
		DeviceIoControl(h_hspr, IOCTL_BITBLT, &pctrl, sizeof(Ctrl_Op), NULL, NULL, NULL, NULL);
	}
	pctrl.dststartx = nXDest;
	pctrl.dststarty = nYDest;
	pctrl.srcstartx = nXSrc;
	pctrl.srcstarty = nYSrc;
	pctrl.width = nWidth;
	pctrl.height = nHeight;
#endif
	if(!DeviceIoControl(h_hspr, IOCTL_BITBLT, &pctrl, sizeof(Ctrl_Op), NULL, NULL, NULL, NULL))
	{
		printf("BitBlt  fail\r\n");
		return FALSE;
	}
#else
	DWORD * pDst = dstframe->viraddr;
	pDst += dstframe->win.cx * nYDest + nXDest;

	if(nHeight > 0)
	{
		DWORD * pSrc = srcframe->viraddr;
		pSrc += srcframe->win.cx * nYSrc + nXSrc;

		for(int i=0;i<nHeight;i++)
		{
			memcpy(pDst,pSrc,nWidth*4);
			pDst += dstframe->win.cx;
			pSrc += srcframe->win.cx;
		}
	}
	else
	{
		DWORD * pSrc = srcframe->viraddr;
		pSrc += srcframe->win.cx * (nYSrc - nHeight - 1) + nXSrc;

		for(int i=0;i<-nHeight;i++)
		{
			memcpy(pDst,pSrc,nWidth*4);
			pDst += dstframe->win.cx;
			pSrc -= srcframe->win.cx;
		}		
	}
#endif
	return TRUE;
}

//ARGB
void CDPGraphic::FillSolidRect(HANDLE hframe, RECT* lpRect, DWORD clr)
{
	Block_Req * frame = (Block_Req*)hframe;

	if(frame == NULL)
		return;
	if(frame->id != FRAME_CHECKID)
		return;

	if(lpRect == NULL)
		return;

	if(lpRect->bottom > frame->win.cy || lpRect->right > frame->win.cx)
		return;

#ifdef DP3000_CHIP	
	Ctrl_Op  pctrl;
	memset(&pctrl,0,sizeof(Ctrl_Op));
	pctrl.op = FILLCOLOR;
	pctrl.colorfill = clr;
	pctrl.dstframe = frame->rethd;
#ifdef ROTATE_ANGLE
	if(frame->isHard)
	{
		pctrl.width = lpRect->bottom - lpRect->top;
		pctrl.height = lpRect->right-lpRect->left;
		pctrl.dststartx = lpRect->top;
		pctrl.dststarty = frame->win.cx - pctrl.height - lpRect->left;
	}
	else
	{
		pctrl.dststartx = lpRect->left;
		pctrl.dststarty = lpRect->top;
		pctrl.width = lpRect->right-lpRect->left;
		pctrl.height = lpRect->bottom - lpRect->top;
	}
#else
	pctrl.dststartx = lpRect->left;
	pctrl.dststarty = lpRect->top;
	pctrl.width = lpRect->right-lpRect->left;
	pctrl.height = lpRect->bottom - lpRect->top;
#endif
	if(!DeviceIoControl(h_hspr, IOCTL_BITBLT, &pctrl, sizeof(Ctrl_Op), NULL, NULL, NULL, NULL))
	{
		printf("BitBlt  fail\r\n");
		return ;
	}
#else
	DWORD * pDst = frame->viraddr;
	pDst += frame->win.cx * lpRect->top + lpRect->left;
	int i,j;

	for(i= lpRect->top;i < lpRect->bottom;i++)
	{
		for(j=0;j<lpRect->right-lpRect->left;j++)
		{
			pDst[j] = clr;
		}
		pDst += frame->win.cx;
	}
#endif
}


BOOL CDPGraphic::AlphaBlend(HANDLE dstSpr,int nXDest,int nYDest,int nWidth,int nHeight,HANDLE srcSpr,int nXSrc,int nYSrc)
{
	Block_Req * dstframe = (Block_Req*)dstSpr;
	Block_Req * srcframe = (Block_Req*)srcSpr;

	if(srcframe == NULL || dstframe == NULL)
	{
		printf("AlphaBlend frame error\r\n");
		return FALSE;
	}
	if(srcframe->id != FRAME_CHECKID || dstframe->id != FRAME_CHECKID)
	{
		printf("AlphaBlend id error\r\n");
		return FALSE;
	}

	if(nXSrc + nWidth > srcframe->win.cx || nYSrc + nHeight > srcframe->win.cy)
	{
		printf("AlphaBlend src error\r\n");
		return FALSE;
	}

	if(nXDest + nWidth > dstframe->win.cx || nYDest + nHeight > dstframe->win.cy)
	{
		printf("AlphaBlend dst error\r\n");
		return FALSE;
	}

#ifdef DP3000_CHIP
	Ctrl_Op  pctrl;
	memset(&pctrl,0,sizeof(Ctrl_Op));
	pctrl.op = ALPHABLEND_BLT;
	pctrl.dstframe = dstframe->rethd;
	pctrl.srcframe = srcframe->rethd;
	pctrl.dststartx = nXDest;
	pctrl.dststarty = nYDest;
	pctrl.srcstartx = nXSrc;
	pctrl.srcstarty = nYSrc;
	pctrl.width = nWidth;
	pctrl.height = nHeight;

	if(!DeviceIoControl(h_hspr, IOCTL_BITBLT, &pctrl, sizeof(Ctrl_Op), NULL, NULL, NULL, NULL))
	{
		printf("BitBlt  fail\r\n");
		return FALSE;
	}
#else
	DWORD * pSrc = srcframe->viraddr;
	pSrc += srcframe->win.cx * nYSrc + nXSrc;

	DWORD * pDst = dstframe->viraddr;
	pDst += dstframe->win.cx * nYDest + nXDest;

	RGBQUAD src,src2,dst;
	int i,j;
	for(i = 0; i < nHeight; i++)
	{
		for(j=0;j<nWidth;j++)
		{
			src2 = *(RGBQUAD*)&(pSrc[j]);
			src =  *(RGBQUAD*)&(pDst[j]);
			Mix(&dst,&src,&src2);
			pDst[j] = *(DWORD*)&dst;
		}
		pDst += dstframe->win.cx;
		pSrc += srcframe->win.cx;
	}
#endif
	return TRUE;
}

BOOL CDPGraphic::AlphaBlend(HANDLE dstSpr,int nXDest,int nYDest,int nWidth,int nHeight, DWORD color)
{
	Block_Req * dstframe = (Block_Req*)dstSpr;
	DWORD count = 0;

	if(dstframe->id != FRAME_CHECKID)
	{
		printf("AlphaBlend frame error\r\n");
		return FALSE;
	}

	if(nXDest + nWidth > dstframe->win.cx || nYDest + nHeight > dstframe->win.cy)
	{
		printf("AlphaBlend src error\r\n");
		return FALSE;
	}

	DWORD * pDst = dstframe->viraddr;
	pDst += dstframe->win.cx * nYDest + nXDest;

	RGBQUAD src,src2,dst;
	int i,j;
	src2 = *(RGBQUAD*)&color;
	for(i = 0; i < nHeight; i++)
	{
		for(j=0;j<nWidth;j++)
		{
			src =  *(RGBQUAD*)&(pDst[j]);
			if(Mix(&dst,&src,&src2))
				count++;
			pDst[j] = *(DWORD*)&dst;
		}
		pDst += dstframe->win.cx;
	}
//	printf("%d %d mix %d point\r\n", nWidth, nHeight, count);
	return TRUE;
}

BOOL CDPGraphic::CscBw(HANDLE srcSpr)
{
	Block_Req * srcframe = (Block_Req*)srcSpr;
	int i;

	if(srcframe == NULL)
		return FALSE;
	if(srcframe->id != FRAME_CHECKID)
		return FALSE;

	DWORD * pSrc = srcframe->viraddr;

	for(i = 0; i < srcframe->win.cy * srcframe->win.cx; i++)
	{
		RGBQUAD src, dst;
		src = *(RGBQUAD*)&(pSrc[i]);
		RGB2BW(&dst,&src);
		pSrc[i] = *(DWORD*)&dst;
	}

	return TRUE;
}

BOOL CDPGraphic::CscRevert(HANDLE srcSpr)
{
	Block_Req * srcframe = (Block_Req*)srcSpr;
	int i;

	if(srcframe == NULL)
		return FALSE;
	if(srcframe->id != FRAME_CHECKID)
		return FALSE;

	DWORD * pSrc = srcframe->viraddr;

	for(i = 0; i < srcframe->win.cy * srcframe->win.cx; i++)
	{
		RGBQUAD src, dst;
		src = *(RGBQUAD*)&(pSrc[i]);
		RGB2Revert(&dst,&src);
		pSrc[i] = *(DWORD*)&dst;
	}

	return TRUE;
}

BOOL CDPGraphic::CscColor(HANDLE srcSpr, DWORD color)
{
	Block_Req * srcframe = (Block_Req*)srcSpr;
	int i;
	RGBQUAD src;
	DWORD * pSrc = srcframe->viraddr;

	if(srcframe == NULL)
		return FALSE;
	if(srcframe->id != FRAME_CHECKID)
		return FALSE;
	src = *(RGBQUAD*)&color;
	for(i = 0; i < srcframe->win.cy * srcframe->win.cx; i++)
	{
		RGBQUAD dst;
		dst = *(RGBQUAD*)&(pSrc[i]);
		RGB2Color(&dst,&src);
		pSrc[i] = *(DWORD*)&dst;
	}

	return TRUE;
}

#ifdef DP3000_CHIP
HANDLE CDPGraphic::CscScale(HANDLE srcSpr, DWORD width, DWORD height)
{
	Block_Req * srcframe = (Block_Req*)srcSpr;
	Block_Req* scaleframe;
	DWORD * pSrc;
	DWORD * pDst;

	if(srcframe == NULL)
		return FALSE;
	if(srcframe->id != FRAME_CHECKID)
		return FALSE;

	pSrc = srcframe->viraddr;
	scaleframe = (Block_Req*)ReqTempBlk(width, height);
	pDst = scaleframe->viraddr;

	Ctrl_Op  pctrl;
	memset(&pctrl,0,sizeof(Ctrl_Op));
	pctrl.op = STRETCH_BLT;
	pctrl.dstframe = scaleframe->rethd;
	pctrl.srcframe = srcframe->rethd;

	if(!DeviceIoControl(h_hspr, IOCTL_BITBLT, &pctrl, sizeof(Ctrl_Op), NULL, NULL, NULL, NULL))
	{
		printf("BitBlt  fail\r\n");
		return FALSE;
	}
	return scaleframe;
}
#endif

void CDPGraphic::InitBakRect(HANDLE hframe, DWORD edge, DWORD middle)
{
	Block_Req * frame = (Block_Req*)hframe;
	DWORD color;
	int i;
	DWORD * pSrc;

	if(frame == NULL)
		return;
	if(frame->id != FRAME_CHECKID)
		return;

	pSrc = frame->viraddr;
	for(i = 0; i < frame->win.cy * frame->win.cx; i++)
		*pSrc++ = middle;

	color = 0xff000000 | edge;
	pSrc = frame->viraddr;
	for(i = 0; i < frame->win.cx; i++)
		*pSrc++ = color;
	pSrc = frame->viraddr;
	for(i = 0; i < frame->win.cy; i++)
	{
		*pSrc = color;
		pSrc += frame->win.cx;
	}
	pSrc = frame->viraddr + frame->win.cx - 1;
	for(i = 0; i < frame->win.cy; i++)
	{
		*pSrc = color;
		pSrc += frame->win.cx;
	}
	pSrc = frame->viraddr + frame->win.cx * (frame->win.cy - 1);
	for(i = 0; i < frame->win.cx; i++)
		*pSrc++ = color;

	pSrc = frame->viraddr + frame->win.cx + 1;
	for(i = 0; i < frame->win.cx - 2; i++)
		*pSrc++ = color;
	pSrc = frame->viraddr + frame->win.cx + 1;
	for(i = 0; i < frame->win.cy - 2; i++)
	{
		*pSrc = color;
		pSrc += frame->win.cx;
	}
	pSrc = frame->viraddr + 2 * frame->win.cx - 2;
	for(i = 0; i < frame->win.cy - 2; i++)
	{
		*pSrc = color;
		pSrc += frame->win.cx;
	}
	pSrc = frame->viraddr + frame->win.cx * (frame->win.cy - 2) + 1;
	for(i = 0; i < frame->win.cx - 2; i++)
		*pSrc++ = color;

	pSrc = frame->viraddr + 2 * frame->win.cx + 2;
	for(i = 0; i < frame->win.cx - 4; i++)
		*pSrc++ = color;
	pSrc = frame->viraddr + 2 * frame->win.cx + 2;
	for(i = 0; i < frame->win.cy - 4; i++)
	{
		*pSrc = color;
		pSrc += frame->win.cx;
	}
	pSrc = frame->viraddr + 3 * frame->win.cx - 3;
	for(i = 0; i < frame->win.cy - 4; i++)
	{
		*pSrc = color;
		pSrc += frame->win.cx;
	}
	pSrc = frame->viraddr + frame->win.cx * (frame->win.cy - 3) + 2;
	for(i = 0; i < frame->win.cx - 4; i++)
		*pSrc++ = color;
}

HANDLE CDPGraphic::DupBlock(HANDLE hframe)
{
	Block_Req * frame = (Block_Req*)hframe;
	Block_Req *hspr;

	if(frame == NULL)
		return NULL;
	if(frame->id != FRAME_CHECKID)
		return NULL;

	hspr = (Block_Req*)ReqTempBlk(frame->win.cx, frame->win.cy);
#ifdef DP3000_CHIP
	Ctrl_Op  pctrl;
	memset(&pctrl,0,sizeof(Ctrl_Op));
	pctrl.op = BITBLT;
	pctrl.dstframe = hspr->rethd;
	pctrl.srcframe = frame->rethd;
	pctrl.dststartx = 0;
	pctrl.dststarty = 0;
	pctrl.srcstartx = 0;
	pctrl.srcstarty = 0;
	pctrl.width = frame->win.cx;
	pctrl.height = frame->win.cy;

	if(!DeviceIoControl(h_hspr, IOCTL_BITBLT, &pctrl, sizeof(Ctrl_Op), NULL, NULL, NULL, NULL))
	{
		printf("BitBlt  fail\r\n");
		return FALSE;
	}
#else
	memcpy(hspr->viraddr, frame->viraddr, frame->win.cx * frame->win.cy * 4);
#endif
	return hspr;
}

BOOL CDPGraphic::GetSize(HANDLE hframe, SIZE* size)
{
	Block_Req * frame = (Block_Req*)hframe;

	if(frame == NULL)
		return FALSE;
	if(frame->id != FRAME_CHECKID)
		return FALSE;
	size->cx = frame->win.cx;
	size->cy = frame->win.cy;
	return TRUE;
}

HANDLE CDPGraphic::Rotate(HANDLE hpng, SIZE* size)
{
	Block_Req *hspr;
	Block_Req* hsrc = (Block_Req*)hpng;
	int i, j;
	DWORD * pSrc = hsrc->viraddr;
	DWORD * pDst;
	

	hspr = (Block_Req*)ReqTempBlk(size->cy, size->cx);
	for(i = 0; i < size->cy; i++)
	{
		pDst = hspr->viraddr + i;
		for(j = 0; j < size->cx; j++)
		{
			*pDst = *pSrc++;
			pDst += size->cy;
		}
	}
	i = size->cx;
	size->cx = size->cy;
	size->cy = i;
	return (HANDLE)hspr;
}

HANDLE CDPGraphic::LoadImage(char* name, SIZE* size, BOOL justonec)
{
	Block_Req *hspr;
	PngImageInfo info;
	char szFind[MAX_PATH];

	if(name == NULL)
		return NULL;
	if(strlen(name) == 0)
		return NULL;
#ifdef PNGCACHE
	for(int i = 0; i < g_pngcount; i++)
	{
		if(strcmp(name, g_png[i].filename) == 0)
		{
			if(size != NULL)
			{
				size->cx = g_png[i].pframe->win.cx;
				size->cy = g_png[i].pframe->win.cy;
			}
			return g_png[i].pframe;
		}
	}
#endif

	size->cx = 0;
	size->cy = 0;

	sprintf(szFind, "%s%s", PNG_DIR, name);
	hspr = NULL;
	if(strstr(name, ".png") != NULL)
	{
		if(TRUE == LoadPng(m_hPng, szFind))
		{
			GetPngInfo(m_hPng, &info);
			hspr = (Block_Req*)ReqTempBlk(info.imageWidth, info.imageHeight);
			if(hspr != NULL)
				DrawPng(m_hPng, hspr);
			if(size != NULL)
			{
				size->cx = info.imageWidth;
				size->cy = info.imageHeight;
			}
		}
		ReleasePng(m_hPng);
	}
	else if(strstr(name, ".jpg") != NULL)
	{
		int jwidth, jheight;
		char* pjpg;
		DWORD* pstart;
		char* psrc;
		char* pdst;
		int i, j;

		// 因为Linux FlashDev是只读的，不能写文件进去，故把背景图片放在Windows目录下
		if(strcmp(name, "bk.jpg") == 0)
			strcpy(szFind, DEFAULT_BK_JPG);

		if(JpegDecoderRGB(szFind, &jwidth, &jheight, &pjpg) != 0)
		{
			hspr = (Block_Req*)ReqTempBlk(jwidth, jheight);
			size->cx = jwidth;
			size->cy = jheight;

			for(i = 0; i < jheight; i++)
			{
				psrc = pjpg + 3 * i * jwidth;
				pdst = (char*)(hspr->viraddr + i * jwidth);
				for(j = 0; j < jwidth; j++)
				{
					pdst[2] = *psrc++;
					pdst[1] = *psrc++;
					pdst[0] = *psrc++;
					pdst[3] = 0xff;
					pdst += 4;
				}
			}
			free(pjpg);
		}
	}
	//DBGMSG(DPWARNING, "Dont find image %s\r\n", name);
	return hspr;
}

BOOL CDPGraphic::ReloadXml(const char* cstr)
{
	char szFind[MAX_PATH];
	Block_Req *hspr;
	DWORD i;

	sprintf(szFind, "%s%s", PNG_DIR, cstr);
	hspr = LoadXmlFrame(szFind);
	if(hspr != NULL)
	{
		for(i = 0; i < g_pngcount; i++)
		{
			if(strcmp(cstr, g_png[i].filename) == 0)
			{
				BitBlt(g_png[i].pframe, 0, 0, g_png[i].pframe->win.cx, g_png[i].pframe->win.cy, hspr, 0, 0);
				CloseBlk(hspr);
				break;
			}
		}
		if(i == g_pngcount)
		{
			hspr->issystem = TRUE;
			g_png = (PNG_CACHE*)realloc(g_png, (g_pngcount + 1) * sizeof(PNG_CACHE));
			g_png[g_pngcount].pframe = hspr;
			strcpy(g_png[g_pngcount].filename, cstr);
			g_pngcount++;
		}
	}
	return TRUE;
}

HANDLE CDPGraphic::LoadStr(char* cstr, LONG h, COLORREF textColor, DWORD* wide, WORD* pos)
{
	LONG wid;

	if(cstr == NULL)
		return NULL;
	if(strlen(cstr) == 0)
		return NULL;
	if((DWORD)h > m_dwCharHeight)
	{
		m_strtemp->issystem = FALSE;
		CloseBlk(m_strtemp);
		m_dwCharHeight = h;
		m_strtemp = (Block_Req*)ReqTempBlk(MAX_WIDTH, m_dwCharHeight);
		m_strtemp->issystem = TRUE;
	}
	SetBK(m_strtemp, 0);
	wid = SprDrawStr(m_strtemp, cstr, textColor, h, pos);
	if(wid > 0)
	{
		*wide = wid;
		return m_strtemp;
	}

//	printf("draw %d char cost %d %d\r\n", wcslen(cstr), starttick1 - starttick, GetTickCount() - starttick1);
	return NULL;
}

void CDPGraphic::DumpBmp(HANDLE frame, char* filename)
{
	FILE* fd;
	char blank[2] = {0, 0};
	Block_Req* pframe = (Block_Req*)frame;
	BmpHead head;
	BmpInfo info;
	int i;

	head.bfType = 0x4d42;
	head.bfSize = pframe->win.cx * pframe->win.cy * 4 + sizeof(BmpInfo) + sizeof(BmpHead) + 2;
	head.bfOffBits = sizeof(BmpInfo) + sizeof(BmpHead);
	head.bfReserved1 = 0;
	head.bfReserved2 = 0;

	info.biSize = sizeof(BmpInfo);
	info.biWidth = pframe->win.cx;
	info.biHeight = pframe->win.cy; 
	info.biPlanes = 1; 
	info.biBitCount = 0x20;
	info.biCompression = 0; 
	info.biSizeImage = pframe->win.cx * pframe->win.cy * 4 + 2; 
	info.biXPelsPerMeter = 0x0b12; 
	info.biYPelsPerMeter = 0x0b12; 
	info.biClrUsed = 0; 
	info.biClrImportant = 0; 
	fd = fopen(filename, "wb");
	if(fd != NULL)
	{
		fwrite(&head, 1, sizeof(BmpHead), fd);
		fwrite(&info, 1, sizeof(BmpInfo), fd);
		for(i = 0; i < pframe->win.cy; i++)
			fwrite(pframe->viraddr + pframe->win.cx * (pframe->win.cy - 1 - i), 1, pframe->win.cx  * 4, fd);
		fwrite(blank, 1, 2, fd);
		fclose(fd);
	}
}
