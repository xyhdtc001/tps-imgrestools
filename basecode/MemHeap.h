// MyHeap.h: interface for the CMemHeap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYHEAP_H__CFFE0003_9790_4E6B_B0C8_16CF55F89B19__INCLUDED_)
#define AFX_MYHEAP_H__CFFE0003_9790_4E6B_B0C8_16CF55F89B19__INCLUDED_


#include <string>
#include "windows.h"
#include <vector>

#ifdef WIN32
class /*PLATFORM_API*/ CMemHeap
{
public:
	CMemHeap(LPCTSTR pszClassName, int size);
	~CMemHeap();

public:
	void*	Alloc		(size_t size);
	void	Free		(void* ptr);
#ifdef MEMHEAP
	bool	IsValidPt	(void* p);
	LONG	GetAllocCount	(void) const { return m_uNumAllocsInHeap; }
#endif

protected:
#ifdef MEMHEAP
	HANDLE			m_hHeap;
	LONG			m_uNumAllocsInHeap;
#else
	int m_nClassSize;
	std::vector<void*> m_vecFree;
#endif

	LONG			m_lMaxAllocsInHeap;
	char			m_strName[32];

	
};

#define	MYHEAP_DECLARATION(cls)										\
public:																\
	void   operator delete	(void* p)	{ s_heap##cls.Free(p); }			\
	void*  operator new	(size_t size)	{ return s_heap##cls.Alloc(size); }	\
	static CMemHeap	s_heap##cls;											\


#define	MYHEAP_IMPLEMENTATION(cls)		CMemHeap	cls::s_heap##cls(#cls, sizeof(cls));\


#else
    #define	MYHEAP_DECLARATION(VAR)
    #define	MYHEAP_IMPLEMENTATION(T,VAR)
#endif
#endif // !defined(AFX_MYHEAP_H__CFFE0003_9790_4E6B_B0C8_16CF55F89B19__INCLUDED_)
