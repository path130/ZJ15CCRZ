#include <roomlib.h>
#include "dptext.h"
#include "dpgraphic.h"
#include "ft2build.h"
#include FT_FREETYPE_H 

#define	TEXTCACHE
#ifdef TEXTCACHE
#define	CACHE_ENTRY		256
#define	CACHE_LINE		8
#define	MAX_SIZE		32
typedef struct
{
	WORD textheight;	// 0
	wchar_t charindex;	// 2
	DWORD refcount;		// 4
	int  x;				// 8
	int left;			// 12
	int top;			// 16
	int rows;			// 20
	int width;			// 24
} TextCache;	// 28

static TextCache S_TextCache[CACHE_ENTRY * CACHE_LINE];
static BYTE* S_PTextCache = NULL;
#endif
static FT_Library library;
static FT_Face face; 

BOOL IniteWRText(char * pszFontPath)
{
	int bError = FT_Init_FreeType(&library);
	if(bError)
		return FALSE;
	bError = FT_New_Face(library,pszFontPath,0, &face);
	if(bError)
		return FALSE;
#ifdef TEXTCACHE
	memset(S_TextCache, 0, sizeof(TextCache) * CACHE_ENTRY * CACHE_LINE);
	S_PTextCache = (BYTE*)malloc(MAX_SIZE * MAX_SIZE * CACHE_ENTRY * CACHE_LINE);
	memset(S_PTextCache, 0, (MAX_SIZE * MAX_SIZE * CACHE_ENTRY * CACHE_LINE));
#endif
	return TRUE;
}

void UiniteWRText()
{
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

static void SprDrawChar(Block_Req * pfram,int xPos,int yPos, COLORREF textColor, int rows, int width, BYTE* buffer)
{
	int i, j;
	DWORD * pDst;
	RGBQUAD srcchar;

	srcchar.rgbRed = (BYTE)textColor;
	srcchar.rgbGreen = (BYTE)(textColor >> 8);
	srcchar.rgbBlue = (BYTE)(textColor >> 16);
	srcchar.rgbReserved = 0;

	pDst = pfram->viraddr + yPos*pfram->win.cx + xPos;
	for(j = 0; j < rows; j++)
	{
		for(i = 0; i < width; i++)
		{
			srcchar.rgbReserved = buffer[i + width*j];
			pDst[i] = *(DWORD*)&srcchar;;
		}
		pDst += pfram->win.cx;
	}
}

BOOL LoadChar(wchar_t cvalue, int textsize, int *x, int* bitmap_left, int* bitmap_top, int* rows, int* width, BYTE** buf)
{
	wchar_t match = 0;
	int i;
#ifdef TEXTCACHE
	TextCache* pCache = NULL;
	//if(cvalue == 0x5ef6)
//		i++;
	if(textsize <= MAX_SIZE)
	{
		match = cvalue & (CACHE_ENTRY - 1);
		pCache = &S_TextCache[match * CACHE_LINE];
		for(i = 0; i < CACHE_LINE; i++)
		{
			if((pCache[i].charindex == cvalue)
				&& (pCache[i].textheight == (WORD)textsize))
				break;
		}
		if(i != CACHE_LINE)
		{
			*x = pCache[i].x;
			*bitmap_left = pCache[i].left;
			*bitmap_top = pCache[i].top;
			*rows = pCache[i].rows;
			*width = pCache[i].width;
			*buf = S_PTextCache + (match * CACHE_LINE + i) * MAX_SIZE * MAX_SIZE;
			pCache[i].refcount++;
			return TRUE;
		}
	}
#endif
//	printf("char miss\r\n");
	if(FT_Load_Char(face, cvalue, FT_LOAD_RENDER))
		return FALSE;
#ifdef TEXTCACHE
	if(pCache != NULL)
	{
		DWORD ref = 0xffffffff;
		DWORD min = 0;
		for(i = 0; i < CACHE_LINE; i++)
		{
			if(pCache[i].refcount == 0)
			{
				ref = 0;
				min = i;
				break;
			}
			if(pCache[i].refcount < ref)
			{
				min = i;
				ref = pCache[i].refcount;
			}
		}
//		if(ref != 0)
//			printf("entry %d is full\r\n", match);
		pCache[min].textheight = textsize;
		pCache[min].left = face->glyph->bitmap_left;
		pCache[min].x = face->glyph->advance.x;
		pCache[min].top = face->glyph->bitmap_top;
		pCache[min].rows = face->glyph->bitmap.rows;
		pCache[min].width = face->glyph->bitmap.width;
		pCache[min].charindex = cvalue;
		pCache[min].refcount = 1;
		memcpy(S_PTextCache + (match * CACHE_LINE + min) * MAX_SIZE * MAX_SIZE, face->glyph->bitmap.buffer, pCache[min].rows * pCache[min].width);
	}
#endif
	*x = face->glyph->advance.x;
	*bitmap_left = face->glyph->bitmap_left;
	*bitmap_top = face->glyph->bitmap_top;
	*rows = face->glyph->bitmap.rows;
	*width = face->glyph->bitmap.width;
	*buf = face->glyph->bitmap.buffer;
	return TRUE;
}

#if 0
int SprDrawStr(HANDLE hFrame, wchar_t* dchar, COLORREF textColor, int textHeight, WORD* pos)
{
	int i;
	int xPos,yPos;
	int left, top, rows, width, x;
	BYTE* buf;
	int gPos;
	Block_Req * pfram = (Block_Req*)hFrame;

	FT_GlyphSlot  slot = face->glyph;


	FT_Set_Pixel_Sizes(face, textHeight, 0);

	i = 0;
	gPos = 0;
	if(pos != NULL)
		pos[0] = 0;
	while(dchar[i] != 0)
	{
		if(!LoadChar(dchar[i], textHeight, &x, &left, &top, &rows, &width, &buf))
		{
			i++;
			continue;
		}

#ifndef NEWFONT
        if(dchar[i] < 0x100)
            x >>= 1;
#endif
		if((gPos + (x >> 6)) > pfram->win.cx)
			break;

		xPos = left;
		if(xPos < 0)
		{
			xPos = 0;
		}

		yPos = textHeight - top - 4;
		if((yPos + rows) > textHeight)
			yPos = textHeight - rows;
		if(yPos < 0)
		{
			yPos = 0;
		}

		SprDrawChar(pfram, gPos + xPos, yPos, textColor, rows, width, buf);

		gPos += (x >> 6) + 1;
		i++;
		if(pos != NULL)
			pos[i] = gPos;
	}
//	printf("drawend\r\n");
	return gPos;
}

#else

int SprDrawStr(HANDLE hFrame, char* cchar, COLORREF textColor, int textHeight, WORD* pos)
{
	int i;
	int xPos,yPos;
	int left, top, rows, width, x;
	BYTE* buf;
	int gPos;
	Block_Req * pfram = (Block_Req*)hFrame;

	WORD unicode[1024];
	wchar_t* dchar = (wchar_t*)malloc((strlen(cchar) + 1)*sizeof(wchar_t));

	utf82unicode(unicode, (BYTE*)cchar);
	unicode2wchar(dchar, unicode);
//	FT_GlyphSlot  slot = face->glyph;

	FT_Set_Pixel_Sizes(face, textHeight*7/8, 0);

	i = 0;
	gPos = 0;
	if(pos != NULL)
		pos[0] = 0;
	while(dchar[i] != 0)
	{
		if(!LoadChar(dchar[i], textHeight*7/8, &x, &left, &top, &rows, &width, &buf))
		{
			i++;
			continue;
		}

		if((gPos + (x >> 6)) > pfram->win.cx)
			break;

		xPos = left;
		if(xPos < 0)
		{
			xPos = 0;
		}

		yPos = textHeight*3/4 - top;
		if((yPos + rows) > textHeight)
			yPos = textHeight - rows;
		if(yPos < 0)
		{
			yPos = 0;
		}

		SprDrawChar(pfram, gPos + xPos, yPos, textColor, rows, width, buf);

		gPos += (x >> 6) + 1;
		i++;
		if(pos != NULL)
			pos[i] = gPos;
	}
//	printf("drawend\r\n");
	free(dchar);
	return gPos;
}
#endif
