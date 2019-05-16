#ifndef __LIST_H__
#define __LIST_H__

typedef struct tagLISTOBJ {
	struct tagLISTOBJ *lpprev;
	struct tagLISTOBJ *lpnext;
} LISTOBJ, * LPLISTOBJ;

typedef struct tagLIST
{
	struct tagLISTOBJ *lphead;
	struct tagLISTOBJ *lptail;
	int ncount;
	CRITICAL_SECTION cs;
} LIST;

class CMyList : public LIST
{

public:
	CMyList()
	{
		DPInitCriticalSection(&cs);

		lphead = NULL;
		lptail = NULL;
		ncount = 0;
	}
	
	~CMyList() { DPDeleteCriticalSection(&cs); }

	int GetCount(void)   { return ncount; }
	LPLISTOBJ Head(void) { return lphead; }
	LPLISTOBJ Tail(void) { return lptail; }
	
	void Lock(BOOL fLock) {

		DPEnterCriticalSection(&cs);	

		if (fLock) { DPEnterCriticalSection(&cs);
		}else{ DPLeaveCriticalSection(&cs); }

		DPLeaveCriticalSection(&cs);	
	} /* Lock() */


	LPLISTOBJ IsObjectValid(LPLISTOBJ pobject) {
		LPLISTOBJ p;

		if (pobject == NULL) return NULL;

		DPEnterCriticalSection(&cs);	
		p = lphead;
		
		while (p != NULL) {
			if (p == pobject) {
				DPLeaveCriticalSection(&cs);	
				return pobject;
			}
			p = p->lpnext;
		}
		
		DPLeaveCriticalSection(&cs);	
		return NULL;   
	} /* IsObjectValid() */

	
	LPLISTOBJ AddHead(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;

		DPEnterCriticalSection(&cs);	

		pobject->lpnext = NULL;
		pobject->lpprev = NULL;
		
		if (ncount == 0) {
			lphead = pobject;
			lptail = pobject;
			
		}else{
			pobject->lpnext = lphead;
			lphead->lpprev = pobject;
			lphead = pobject;
		}
		
		ncount ++;
		DPLeaveCriticalSection(&cs);			
		return pobject;

	} /* AddHead() */

	
	LPLISTOBJ AddTail(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;

		DPEnterCriticalSection(&cs);	

		pobject->lpnext = NULL;
		pobject->lpprev = NULL;
		
		if (ncount == 0) {
			lphead = pobject;
			lptail = pobject;
			
		}else{
			pobject->lpprev = lptail;
			lptail->lpnext = pobject;
			lptail = pobject;
		}
		
		ncount ++;
		DPLeaveCriticalSection(&cs);		
		return pobject;

	} /* AddTail() */

	
	LPLISTOBJ Disconnect(LPLISTOBJ pobject) {
		LPLISTOBJ pprev, pnext; 

		if (pobject == NULL) return NULL;

		DPEnterCriticalSection(&cs);	
		
		if (--ncount == 0) {
			lphead = NULL;
			lptail = NULL; 
			
		}else{
			pprev = pobject->lpprev;
			pnext = pobject->lpnext;
			
			if (pprev == NULL) {
				pnext->lpprev = NULL;
				lphead = pnext;
				
			}else if (pnext == NULL) {
				pprev->lpnext = pnext;
				lptail = pprev;
				
			}else{
				pnext->lpprev = pprev;
				pprev->lpnext = pnext;
			}
		}
		
		DPLeaveCriticalSection(&cs);	
		return pobject;

	} /* Disconnect() */

	
	LPLISTOBJ MoveToHead(LPLISTOBJ pobject) {
		if (Disconnect(pobject)) {
			return AddHead(pobject);
		}
		return NULL;
	} /* MoveToHead() */

	
	LPLISTOBJ MoveToTail(LPLISTOBJ pobject) {
		if (Disconnect(pobject)) {
			return AddTail(pobject);
		}
		return NULL;
	} /* MoveToTail() */
	

	LPLISTOBJ Next(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;
		return pobject->lpnext;
	} /* Next() */

	
	LPLISTOBJ Prev(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;
		return pobject->lpprev;
	} /* Prev() */

	
	int GetObjectIndex(LPLISTOBJ pobject) {
		LPLISTOBJ p;
		int i;
		
		if (pobject == NULL)
			return 0;

		DPEnterCriticalSection(&cs);	
		for (i=0, p=lphead; (p!=NULL && p!=pobject); p=p->lpnext, i++);
		if(i == ncount)
			i = -1;		
		DPLeaveCriticalSection(&cs);	
		
		return i;
	} /* GetObjectIndex() */

	
	LPLISTOBJ GetAtIndex(int iIndex) {
		LPLISTOBJ p;
		int i;
		
		if (iIndex >= ncount) return NULL;
		
		DPEnterCriticalSection(&cs);	
		for (i=0, p=lphead; ((p!=NULL) && (i<iIndex)); p=p->lpnext, i++);
		DPLeaveCriticalSection(&cs);		
		return p;
		
	} /* GetAtIndex() */

	
	LPLISTOBJ DisconnectAtIndex(int iIndex)
	{
		LPLISTOBJ p;

		DPEnterCriticalSection(&cs);	
		if((p = GetAtIndex(iIndex)) != NULL)
		{
			p = Disconnect(p);
		}
		DPLeaveCriticalSection(&cs);	
		return p;
		
	} /* DisconnectAtIndex() */

	
	LPLISTOBJ InsertBeforeIndex(LPLISTOBJ pobject, int iIndex) {
		LPLISTOBJ pnext = GetAtIndex(iIndex);

		if (!pobject || (iIndex < 0) ||	(iIndex > ncount)) return NULL;

		if ((iIndex == 0) || (pnext == NULL)) {
			return AddHead(pobject);
			
		}else if (iIndex == ncount) {
			return AddTail(pobject);
			
		}else{
			LPLISTOBJ pprev;
			
			DPEnterCriticalSection(&cs);	

			pobject->lpnext = NULL;
			pobject->lpprev = NULL;
			
			pprev           = pnext->lpprev;
			pobject->lpnext = pnext;
			pobject->lpprev = pnext->lpprev;
			
			pnext->lpprev = pobject;
			if (pprev != NULL) {
				pprev->lpnext = pobject;
			}
			
			ncount ++;
			DPLeaveCriticalSection(&cs);	
		}
		return pobject;
		
	} /* InsertBeforeIndex() */

	
	LPLISTOBJ InsertAfterIndex(LPLISTOBJ pobject, long iIndex) {
		LPLISTOBJ pprev = GetAtIndex(iIndex);

		if (!pobject || (iIndex < -1) || (iIndex >= ncount)) return NULL;

		if ((iIndex == -1) || (pprev == NULL)) {
			return AddHead(pobject);
			
		}else if (iIndex == (ncount-1)) {
			return AddTail(pobject);
			
		}else{
			LPLISTOBJ pnext;

			DPEnterCriticalSection(&cs);	
			
			pobject->lpnext = NULL;
			pobject->lpprev = NULL;
			
			pnext           = pprev->lpnext;
			pobject->lpprev = pprev;
			pobject->lpnext = pprev->lpnext;
			
			pprev->lpnext = pobject;
			if (pnext != NULL) {
				pnext->lpprev = pobject;
			}
			
			ncount ++;
			DPLeaveCriticalSection(&cs);	
		}
		return pobject;
		
	} /* InsertAfterIndex() */

	
	LPLISTOBJ InsertAfterObject(LPLISTOBJ pobject, LPLISTOBJ pobjectAfter) {

		if (pobject == NULL) return NULL;

		if (pobjectAfter == NULL) {
			return AddHead(pobject);

		}else if (pobjectAfter->lpnext == NULL) {
			return AddTail(pobject);
			
		}else{
			LPLISTOBJ pobjectBefore;
			
			DPEnterCriticalSection(&cs);	
			
			pobject->lpnext = NULL;
			pobject->lpprev = NULL;
			
			pobjectBefore   = pobjectAfter->lpnext;
			pobject->lpprev = pobjectAfter;
			pobject->lpnext = pobjectAfter->lpnext;
			
			pobjectAfter->lpnext = pobject;
			if (pobjectBefore != NULL) {
				pobjectBefore->lpprev = pobject;
			}
			
			ncount++;
			DPLeaveCriticalSection(&cs);	
		}
		return pobject;
	
	} /* InsertAfterObject() */

	
	BOOL Swap(LPLISTOBJ pobject1, LPLISTOBJ pobject2) {
		int cbIndex1, cbIndex2;

		if (!pobject1 || !pobject2) return FALSE;

		DPEnterCriticalSection(&cs);	
		
		cbIndex1 = GetObjectIndex(pobject1);
		cbIndex2 = GetObjectIndex(pobject2);
		
		Disconnect(pobject1);
		Disconnect(pobject2);

		if (cbIndex1 < cbIndex2) {
			InsertAfterIndex(pobject2, cbIndex1 - 1);
			InsertAfterIndex(pobject1, cbIndex2 - 1);
		}else{
			InsertAfterIndex(pobject1, cbIndex2 - 1);
			InsertAfterIndex(pobject2, cbIndex1 - 1);
		}
		DPLeaveCriticalSection(&cs);	

		return TRUE;
		
	} /* Swap() */
		
}; /* CList */

/////////////////////////////////////////////////////////////////////////////

#endif /* __LIST_H__ */
