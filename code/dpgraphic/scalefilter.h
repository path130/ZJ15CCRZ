#ifndef _2_PASS_SCALE_H_
#define _2_PASS_SCALE_H_

#include <math.h> 

#include "Filters.h" 

#define	STRIP_ALPHA(x)		(((x) >> 24) & 0xff)
#define	STRIP_RED(x)		(((x) >> 16) & 0xff)
#define	STRIP_GREEN(x)		(((x) >> 8) & 0xff)
#define	STRIP_BLUE(x)		((x) & 0xff)
#define	CMB_ARGB(a, r, g, b)	(((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
typedef struct 
{ 
   double *Weights;  // Normalized weights of neighboring pixels
   int Left,Right;   // Bounds of source pixels window
} ContributionType;  // Contirbution information for a single pixel

typedef struct 
{ 
   ContributionType *ContribRow; // Row (or column) of contribution weights 
   UINT WindowSize,              // Filter window size (of affecting source pixels) 
        LineLength;              // Length of line (no. or rows / cols) 
} LineContribType;               // Contribution information for an entire line (row or column)

typedef BOOL (*ProgressAnbAbortCallBack)(BYTE bPercentComplete);

template <class FilterClass>
class C2PassScale 
{
public:

    C2PassScale (ProgressAnbAbortCallBack callback = NULL) : 
        m_Callback (callback) {}

    virtual ~C2PassScale() {}

    DWORD * Scale (  
        DWORD   *pOrigImage, 
        UINT        uOrigWidth, 
        UINT        uOrigHeight, 
        DWORD   *pDstImage,
        UINT        uNewWidth, 
        UINT        uNewHeight);

private:

    ProgressAnbAbortCallBack    m_Callback;
    BOOL                        m_bCanceled;

    LineContribType *AllocContributions (   UINT uLineLength, 
                                            UINT uWindowSize);

    void FreeContributions (LineContribType * p);

    LineContribType *CalcContributions (    UINT    uLineSize, 
                                            UINT    uSrcSize, 
                                            double  dScale);

    void ScaleRow ( DWORD           *pSrc, 
                    UINT                uSrcWidth,
                    DWORD           *pRes, 
                    UINT                uResWidth,
                    UINT                uRow, 
                    LineContribType    *Contrib);

    void HorizScale (   DWORD           *pSrc, 
                        UINT                uSrcWidth,
                        UINT                uSrcHeight,
                        DWORD           *pDst,
                        UINT                uResWidth,
                        UINT                uResHeight);

    void ScaleCol ( DWORD           *pSrc, 
                    UINT                uSrcWidth,
                    DWORD           *pRes, 
                    UINT                uResWidth,
                    UINT                uResHeight,
                    UINT                uCol, 
                    LineContribType    *Contrib);

    void VertScale (    DWORD           *pSrc, 
                        UINT                uSrcWidth,
                        UINT                uSrcHeight,
                        DWORD           *pDst,
                        UINT                uResWidth,
                        UINT                uResHeight);
};

template <class FilterClass>
LineContribType *
C2PassScale<FilterClass>::
AllocContributions (UINT uLineLength, UINT uWindowSize)
{
    LineContribType *res = new LineContribType; 
        // Init structure header 
    res->WindowSize = uWindowSize; 
    res->LineLength = uLineLength; 
        // Allocate list of contributions 
    res->ContribRow = new ContributionType[uLineLength];
    for (UINT u = 0 ; u < uLineLength ; u++) 
    {
        // Allocate contributions for every pixel
        res->ContribRow[u].Weights = new double[uWindowSize];
    }
    return res; 
} 
 
template <class FilterClass>
void
C2PassScale<FilterClass>::
FreeContributions (LineContribType * p)
{ 
    for (UINT u = 0; u < p->LineLength; u++) 
    {
        // Free contribs for every pixel
        delete [] p->ContribRow[u].Weights;
    }
    delete [] p->ContribRow;    // Free list of pixels contribs
    delete p;                   // Free contribs header
} 
 
template <class FilterClass>
LineContribType *
C2PassScale<FilterClass>::
CalcContributions (UINT uLineSize, UINT uSrcSize, double dScale)
{ 
    FilterClass CurFilter;

    double dWidth;
    double dFScale = 1.0;
    double dFilterWidth = CurFilter.GetWidth();
	int iSrc;

    if (dScale < 1.0) 
    {    // Minification
        dWidth = dFilterWidth / dScale; 
        dFScale = dScale; 
    } 
    else
    {    // Magnification
        dWidth= dFilterWidth; 
    }
 
    // Window size is the number of sampled pixels
    int iWindowSize = 2 * (int)ceil(dWidth) + 1; 
 
    // Allocate a new line contributions strucutre
    LineContribType *res = AllocContributions (uLineSize, iWindowSize); 
 
    for (UINT u = 0; u < uLineSize; u++) 
    {   // Scan through line of contributions
        double dCenter = (double)u / dScale;   // Reverse mapping
        // Find the significant edge points that affect the pixel
        int iLeft = max (0, (int)floor (dCenter - dWidth)); 
        res->ContribRow[u].Left = iLeft; 
        int iRight = min ((int)ceil (dCenter + dWidth), int(uSrcSize) - 1); 
        res->ContribRow[u].Right = iRight;
 
        // Cut edge points to fit in filter window in case of spill-off
        if (iRight - iLeft + 1 > iWindowSize) 
        {
            if (iLeft < (int(uSrcSize) - 1 / 2)) 
            {
                iLeft++; 
            }
            else 
            {
                iRight--; 
            }
        }
        double dTotalWeight = 0.0;  // Zero sum of weights
        for (iSrc = iLeft; iSrc <= iRight; iSrc++)
        {   // Calculate weights
            dTotalWeight += (res->ContribRow[u].Weights[iSrc-iLeft] =  
                                dFScale * CurFilter.Filter (dFScale * (dCenter - (double)iSrc))); 
        }
        ASSERT (dTotalWeight >= 0.0);   // An error in the filter function can cause this 
        if (dTotalWeight > 0.0)
        {   // Normalize weight of neighbouring points
            for (iSrc = iLeft; iSrc <= iRight; iSrc++)
            {   // Normalize point
                res->ContribRow[u].Weights[iSrc-iLeft] /= dTotalWeight; 
            }
        }
   } 
   return res; 
} 
 
 
template <class FilterClass>
void 
C2PassScale<FilterClass>::
ScaleRow (  DWORD           *pSrc, 
            UINT                uSrcWidth,
            DWORD           *pRes, 
            UINT                uResWidth,
            UINT                uRow, 
            LineContribType    *Contrib)
{
    DWORD *pSrcRow = &(pSrc[uRow * uSrcWidth]);
    DWORD *pDstRow = &(pRes[uRow * uResWidth]);
    for (UINT x = 0; x < uResWidth; x++) 
    {   // Loop through row
    	BYTE a = 0;
		BYTE r;
		BYTE g;
		BYTE b;
        int iLeft = Contrib->ContribRow[x].Left;    // Retrieve left boundries
        int iRight = Contrib->ContribRow[x].Right;  // Retrieve right boundries
        for (int i = iLeft; i <= iRight; i++)
        {   // Scan between boundries
            // Accumulate weighted effect of each neighboring pixel
            a += (BYTE)(Contrib->ContribRow[x].Weights[i-iLeft] * (double)(STRIP_ALPHA(pSrcRow[i]))); 
        }
		if(a > 5)
		{
			r = 0;
			g = 0;
			b = 0;
	        for (int i = iLeft; i <= iRight; i++)
		    {   // Scan between boundries
			    // Accumulate weighted effect of each neighboring pixel
				r += (BYTE)(Contrib->ContribRow[x].Weights[i-iLeft] * (double)(STRIP_RED(pSrcRow[i]))); 
				g += (BYTE)(Contrib->ContribRow[x].Weights[i-iLeft] * (double)(STRIP_GREEN(pSrcRow[i]))); 
				b += (BYTE)(Contrib->ContribRow[x].Weights[i-iLeft] * (double)(STRIP_BLUE(pSrcRow[i]))); 
			} 
		}
        pDstRow[x] = CMB_ARGB(a, r,g,b); // Place result in destination pixel
    } 
} 

template <class FilterClass>
void
C2PassScale<FilterClass>::
HorizScale (    DWORD           *pSrc, 
                UINT                uSrcWidth,
                UINT                uSrcHeight,
                DWORD           *pDst, 
                UINT                uResWidth,
                UINT                uResHeight)
{ 
    if (uResWidth == uSrcWidth)
    {   // No scaling required, just copy
        memcpy (pDst, pSrc, sizeof (DWORD) * uSrcHeight * uSrcWidth);
		return;
    }
    // Allocate and calculate the contributions
    LineContribType * Contrib = CalcContributions (uResWidth, uSrcWidth, double(uResWidth) / double(uSrcWidth)); 
    for (UINT u = 0; u < uResHeight; u++)
    {   // Step through rows
        ScaleRow (  pSrc, 
                    uSrcWidth,
                    pDst,
                    uResWidth,
                    u,
                    Contrib);    // Scale each row 
    }
    FreeContributions (Contrib);  // Free contributions structure
} 
 
template <class FilterClass>
void
C2PassScale<FilterClass>::
ScaleCol (  DWORD           *pSrc, 
            UINT                uSrcWidth,
            DWORD           *pRes, 
            UINT                uResWidth,
            UINT                uResHeight,
            UINT                uCol, 
            LineContribType    *Contrib)
{ 
    for (UINT y = 0; y < uResHeight; y++) 
    {    // Loop through column
    	BYTE a = 0;
        BYTE r = 0;
        BYTE g = 0;
        BYTE b = 0;
        int iLeft = Contrib->ContribRow[y].Left;    // Retrieve left boundries
        int iRight = Contrib->ContribRow[y].Right;  // Retrieve right boundries
        for (int i = iLeft; i <= iRight; i++)
        {   // Scan between boundries
            // Accumulate weighted effect of each neighboring pixel
            DWORD pCurSrc = pSrc[i * uSrcWidth + uCol];
			a += BYTE(Contrib->ContribRow[y].Weights[i-iLeft] * (double)(STRIP_ALPHA(pCurSrc)));
        }
		if(a > 5)
		{
	        for (int i = iLeft; i <= iRight; i++)
		    {   // Scan between boundries
			    // Accumulate weighted effect of each neighboring pixel
				DWORD pCurSrc = pSrc[i * uSrcWidth + uCol];
				r += BYTE(Contrib->ContribRow[y].Weights[i-iLeft] * (double)(STRIP_RED(pCurSrc)));
		        g += BYTE(Contrib->ContribRow[y].Weights[i-iLeft] * (double)(STRIP_GREEN(pCurSrc)));
			    b += BYTE(Contrib->ContribRow[y].Weights[i-iLeft] * (double)(STRIP_BLUE(pCurSrc)));
			}
        }
        pRes[y * uResWidth + uCol] = CMB_ARGB (a, r,g,b);   // Place result in destination pixel
    }
} 
 

template <class FilterClass>
void
C2PassScale<FilterClass>::
VertScale ( DWORD           *pSrc, 
            UINT                uSrcWidth,
            UINT                uSrcHeight,
            DWORD           *pDst, 
            UINT                uResWidth,
            UINT                uResHeight)
{ 
    if (uSrcHeight == uResHeight)
    {   // No scaling required, just copy
        memcpy (pDst, pSrc, sizeof (DWORD) * uSrcHeight * uSrcWidth);
    }
    // Allocate and calculate the contributions
    LineContribType * Contrib = CalcContributions (uResHeight, uSrcHeight, double(uResHeight) / double(uSrcHeight)); 
    for (UINT u = 0; u < uResWidth; u++)
    {   // Step through columns
        if (NULL != m_Callback)
        {
            //
            // Progress and report callback supplied
            //
            if (!m_Callback (BYTE(double(u) / double (uResWidth) * 50.0) + 50))
            {
                //
                // User wished to abort now
                //
                m_bCanceled = TRUE;
                FreeContributions (Contrib);  // Free contributions structure
                return;
            }
        }
        ScaleCol (  pSrc,
                    uSrcWidth,
                    pDst,
                    uResWidth,
                    uResHeight,
                    u,
                    Contrib);   // Scale each column
    }
    FreeContributions (Contrib);     // Free contributions structure
} 

template <class FilterClass>
DWORD *
C2PassScale<FilterClass>::
Scale ( 
    DWORD   *pOrigImage, 
    UINT        uOrigWidth, 
    UINT        uOrigHeight, 
    DWORD   *pDstImage, 
    UINT        uNewWidth, 
    UINT        uNewHeight)
{ 
	DWORD start0, start1;
    // Scale source image horizontally into temporary image
    m_bCanceled = FALSE;
	start0 = GetTickCount();
    DWORD *pTemp = new DWORD [uNewWidth * uOrigHeight];
    HorizScale (pOrigImage, 
                uOrigWidth,
                uOrigHeight,
                pTemp,
                uNewWidth,
                uOrigHeight);
    if (m_bCanceled)
    {
        delete [] pTemp;
        return NULL;
    }

	start1 = GetTickCount();
    // Scale temporary image vertically into result image    
    VertScale ( pTemp,
                uNewWidth,
                uOrigHeight,
                pDstImage,
                uNewWidth,
                uNewHeight);
    delete [] pTemp;
    if (m_bCanceled)
    {
        return NULL;
    }
//	printf("scal cost %d %d\r\n", GetTickCount() - start1, start1 - start0);
    return pDstImage;
} 


#endif //   _2_PASS_SCALE_H_
 

