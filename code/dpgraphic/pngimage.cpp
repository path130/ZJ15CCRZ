#include <roomlib.h>
#include "pngimage.h"
#include "pnglibconf.h"
#include "pngconf.h"
#include "png.h"
#include "pngstruct.h"
#include "pnginfo.h"

static void Mix(RGBQUAD * pDst,RGBQUAD * pSrc,RGBQUAD * pSrc2)
{
	if(pSrc->rgbReserved < 5 || pSrc2->rgbReserved > 250)
		*pDst = *pSrc2;
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
	}
}

static void Mix2(RGBQUAD * pDst,RGBQUAD * pSrc,RGBQUAD * pSrc2)
{
	if(pSrc->rgbReserved < 5 || pSrc2->rgbReserved > 250)
		*pDst = *pSrc2;
	else
	{
		pDst->rgbRed = (pSrc->rgbRed * pSrc->rgbReserved + pSrc2->rgbRed * (255 - pSrc->rgbReserved)) >> 8;
		pDst->rgbGreen = (pSrc->rgbGreen * pSrc->rgbReserved + pSrc2->rgbGreen * (255 - pSrc->rgbReserved)) >> 8;
		pDst->rgbBlue = (pSrc->rgbBlue * pSrc->rgbReserved + pSrc2->rgbBlue * (255 - pSrc->rgbReserved)) >> 8;
		pDst->rgbReserved = pSrc2->rgbReserved;
	}
}

class CPngImage
{
public:
	CPngImage(void)
	{
		png_ptr = NULL;
		info_ptr = NULL;
		trfile = NULL;
	}
	
	~CPngImage(void)
	{
		Release();
	}
	
	BOOL GetImageInfo(PngImageInfo * pInfo)
	{
		if(info_ptr == NULL)
			return FALSE;
		pInfo->pixelDepth = info_ptr->pixel_depth;
		pInfo->imageWidth = info_ptr->width;
		pInfo->imageHeight = info_ptr->height;
		return TRUE;
	}
	
	BOOL Load(char * fileName)
	{
		if(fileName==NULL)
			return FALSE;
		trfile = fopen(fileName,"rb");
		if(trfile == NULL)
		{
			printf("fopen %s \n",fileName);
			return FALSE;
		}
		BYTE sig[9] = {0};
		fread(sig, 1, 8, trfile);
		if (png_sig_cmp(sig, 0, 8))
		{
			fclose(trfile);
			return FALSE;
		}
		png_ptr = png_create_read_struct(png_get_libpng_ver(NULL), NULL, NULL, NULL);
		 if (!png_ptr)
		{
			fclose(trfile);
			return FALSE;/* out of memory */
		}
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) 
		{
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			fclose(trfile);
			return FALSE;  /* out of memory */
		}
		
		if (setjmp(png_jmpbuf(png_ptr))) 
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(trfile);
			return FALSE;
		}
		png_init_io(png_ptr, trfile);
		png_set_sig_bytes(png_ptr, 8); 
		png_read_info(png_ptr, info_ptr);
		return TRUE;
	}
	BOOL Draw(Block_Req * pBlock,int xpos,int ypos,RECT * pSrcRect,DWORD flag)
	{
		RECT ttRect;
		if(pSrcRect == NULL)
		{
			pSrcRect = &ttRect;
			pSrcRect->left = 0;
			pSrcRect->top = 0;
			pSrcRect->bottom = info_ptr->height;
			pSrcRect->right = info_ptr->width;
		}
	
		if(pSrcRect->right > info_ptr->width || pSrcRect->bottom > info_ptr->height)
			return FALSE;
	
		int srcW = pSrcRect->right - pSrcRect->left;
		int srcH = pSrcRect->bottom - pSrcRect->top;
		
		int dstW = srcW;
		if(xpos + srcW > pBlock->win.cx)
			dstW = pBlock->win.cx - xpos;
	
		int dstH = srcH;
		if(ypos + srcH > pBlock->win.cy)
			dstH = pBlock->win.cy - ypos;
	
		DWORD * pDst = pBlock->viraddr;
		pDst += ypos*pBlock->win.cx + xpos;
	
		//png_bytep row_pointers = png_get_rows(png_ptr, info_ptr);
		png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep*)*info_ptr->height);
		for (DWORD row = 0; row < info_ptr->height; row++)
			row_pointers[row] = NULL;
	
		for (DWORD row = 0; row < info_ptr->height; row++)
			row_pointers[row] = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr,info_ptr));
		png_read_image(png_ptr, row_pointers);	   
//		png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
		if(flag == 0)
		{
			if(info_ptr->pixel_depth == 24)
			{
				DWORD pixelR,pixelG,pixelB;
				BYTE * pSrc;
				int i,j;
				for(j=0;j<dstH;j++)
				{
					pSrc = row_pointers[pSrcRect->top+j] + pSrcRect->left * 3;
					for(i=0;i<dstW;i++)
					{
						pixelR = *pSrc++;
						pixelG = *pSrc++;
						pixelB = *pSrc++;
						pDst[i] = 0xFF000000 | (pixelR << 16) | (pixelG << 8) | pixelB;
					}
					pDst += pBlock->win.cx;
				}
				
			}
			
			else if(info_ptr->pixel_depth == 32)
			{
				DWORD pixelR,pixelG,pixelB,pixelA;
				BYTE * pSrc;
				int i,j;
				for(j=0;j<dstH;j++)
				{
					pSrc = row_pointers[pSrcRect->top+j] + pSrcRect->left * 4;
					for(i=0;i<dstW;i++)
					{
						pixelR = *pSrc++;
						pixelG = *pSrc++;
						pixelB = *pSrc++;
						pixelA = *pSrc++;
						pDst[i] = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
					}
					pDst += pBlock->win.cx;
				}
			}
		}
		else if(flag == 1)
		{
			if(info_ptr->pixel_depth == 24)
			{
				DWORD pixelR,pixelG,pixelB;
				BYTE * pSrc;
				int i,j;
				for(j=0;j<dstH;j++)
				{
					pSrc = row_pointers[pSrcRect->top+j] + pSrcRect->left * 3;
					for(i=0;i<dstW;i++)
					{
						pixelR = *pSrc++;
						pixelG = *pSrc++;
						pixelB = *pSrc++;
						pDst[i] = 0xFF000000 | (pixelR << 16) | (pixelG << 8) | pixelB;
					}
					pDst += pBlock->win.cx;
				}
			}
			else if(info_ptr->pixel_depth == 32)
			{
				BYTE temp;
				RGBQUAD src,src2,dst;
				DWORD * pSrc;
				int i,j;
				for(j=0;j<dstH;j++)
				{
					pSrc = (DWORD*)(row_pointers[pSrcRect->top+j] + pSrcRect->left * 4);
					for(i=0;i<dstW;i++)
					{
						src2 = *(RGBQUAD*)&(pSrc[i]);
						temp = src2.rgbRed;
						src2.rgbRed = src2.rgbBlue;
						src2.rgbBlue = temp;
						src =  *(RGBQUAD*)&(pDst[i]);
						Mix2(&dst,&src,&src2);
						pDst[i] = *(DWORD*)&dst;
					}
					pDst += pBlock->win.cx;
				}
			}
		}
		for (DWORD row = 0; row < info_ptr->height; row++)
		{
			png_free(png_ptr,row_pointers[row]);
		}
		free(row_pointers);
	
		return TRUE;
	}
	
	BOOL GetData(char * filename,unsigned char * outbuf,DWORD flag)
	{
		if(filename && outbuf)
			Load(filename);
	
		if(info_ptr == NULL || png_ptr == NULL)
			return FALSE;
	
		DWORD * pDst = (DWORD *)outbuf;
		png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
	
		if(flag == 0)
		{
			if(info_ptr->pixel_depth == 24)
			{
				DWORD pixelR,pixelG,pixelB;
				BYTE * pSrc;
				int i,j;
				for(j=0;j<info_ptr->height;j++)
				{
					pSrc = row_pointers[j];
					for(i=0;i<info_ptr->width;i++)
					{
						pixelR = *pSrc++;
						pixelG = *pSrc++;
						pixelB = *pSrc++;
						pDst[i] = 0xFF000000 | (pixelR << 16) | (pixelG << 8) | pixelB;
					}
					pDst += info_ptr->width;
				}
			}
			else if(info_ptr->pixel_depth == 32)
			{
				DWORD pixelR,pixelG,pixelB,pixelA;
				BYTE * pSrc;
				int i,j;
				for(j=0;j<info_ptr->height;j++)
				{
					pSrc = row_pointers[j] ;
					for(i=0;i<info_ptr->width;i++)
					{
						pixelR = *pSrc++;
						pixelG = *pSrc++;
						pixelB = *pSrc++;
						pixelA = *pSrc++;
						pDst[i] = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
					}
					pDst += info_ptr->width;
				}
			}
		}
		else if(flag == 1)
		{
			if(info_ptr->pixel_depth == 24)
			{
				DWORD pixelR,pixelG,pixelB;
				BYTE * pSrc;
				int i,j;
				for(j=0;j<info_ptr->height;j++)
				{
					pSrc = row_pointers[j] ;
					for(i=0;i<info_ptr->width;i++)
					{
						pixelR = *pSrc++;
						pixelG = *pSrc++;
						pixelB = *pSrc++;
						pDst[i] = 0xFF000000 | (pixelR << 16) | (pixelG << 8) | pixelB;
					}
					pDst += info_ptr->width;
				}
			}
			else if(info_ptr->pixel_depth == 32)
			{
				BYTE temp;
				RGBQUAD src,src2,dst;
				DWORD * pSrc;
				int i,j;
				for(j=0;j<info_ptr->height;j++)
				{
					pSrc = (DWORD*)(row_pointers[j]);
					for(i=0;i<info_ptr->width;i++)
					{
						src2 = *(RGBQUAD*)&(pSrc[i]);
						temp = src2.rgbRed;
						src2.rgbRed = src2.rgbBlue;
						src2.rgbBlue = temp;
						src =  *(RGBQUAD*)&(pDst[i]);
						Mix(&dst,&src,&src2);
						pDst[i] = *(DWORD*)&dst;
					}
					pDst += info_ptr->width;
				}
			}
		}
	
		return TRUE;
	}
	void Release()
	{
		if(png_ptr || info_ptr)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			png_ptr = NULL;
			info_ptr = NULL;
		}
		if(trfile)
		{
			fclose(trfile);
			trfile = NULL;
		}
	}
private:
	png_structp png_ptr;
	png_infop info_ptr;
	FILE * trfile;
};
HANDLE CreatePng(void)
{
	return new CPngImage();
}
BOOL LoadPng(HANDLE hpng, char* fileName)
{
	CPngImage* pPng = (CPngImage*)hpng;
	return pPng->Load(fileName);
}

BOOL GetPngInfo(HANDLE hpng, PngImageInfo* pInfo)
{
	CPngImage* pPng = (CPngImage*)hpng;
	return pPng->GetImageInfo(pInfo);
}

BOOL DrawPng(HANDLE hpng, Block_Req * pBlock)
{
	CPngImage* pPng = (CPngImage*)hpng;
	return pPng->Draw(pBlock, 0, 0, NULL, 0);
}

void ReleasePng(HANDLE hPng)
{
	CPngImage* pPng = (CPngImage*)hPng;
	pPng->Release();
}

void DestroyPng(HANDLE hPng)
{
	CPngImage* pPng = (CPngImage*)hPng;
	delete pPng;
}


