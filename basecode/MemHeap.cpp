// MyHeap.cpp: implementation of the CMemHeap class.
//
//////////////////////////////////////////////////////////////////////

#include "basecode/MemHeap.h"
#include "basecode/BaseMacro.h"
#include <assert.h>

#ifdef WINDOWS
CMemHeap::CMemHeap( LPCTSTR pszClassName, int size)
{
#ifdef MEMHEAP
	m_hHeap	= ::HeapCreate(0, 0, 0);
	m_uNumAllocsInHeap	= 0;
#else
	m_nClassSize = size;
#endif
	
	m_lMaxAllocsInHeap	= 0;
	if ( pszClassName )
		strncpy( m_strName, pszClassName, 32 );
}


CMemHeap::~CMemHeap()
{
#ifdef MEMHEAP
	if ( m_hHeap )
	{
		::HeapDestroy(m_hHeap);
		char szOutput[1024];
		sprintf(szOutput, "heap of class %s destroied, with max alloc(%ld).\n", m_strName, m_lMaxAllocsInHeap);
		OutputDebugStr(szOutput);
	}

	if (m_uNumAllocsInHeap != 0)
	{
		char szOutput[1024];
		sprintf(szOutput, "CMemHeap::~CMemHeap() 的[%s]有[%d]个内存块没有释放\n", m_strName, m_uNumAllocsInHeap);
		OutputDebugStr(szOutput);
	}
#else
	if(m_lMaxAllocsInHeap > 0)
	{
		char szOutput[1024];
		sprintf(szOutput, "heap of class %s destroied, with max alloc(%ld).\n", m_strName, m_lMaxAllocsInHeap);
		//OutputDebugStr(szOutput);
	}

	LONG nAllocs = m_lMaxAllocsInHeap-m_vecFree.size();
	if (nAllocs > 0)
	{
		char szOutput[1024];
		sprintf(szOutput, "CMemHeap::~CMemHeap() 的[%s]有[%d]个内存块没有释放\n", m_strName, nAllocs);
		//OutputDebugStr(szOutput);
	}

	for (std::vector<void*>::iterator iter = m_vecFree.begin();iter != m_vecFree.end();++iter)
	{
		free(*iter);
	}
	m_vecFree.clear();
#endif
}

//////////////////////////////////////////////////////////////////////
void * CMemHeap::Alloc(size_t size)
{
#ifdef MEMHEAP
	if ( !m_hHeap )
		return NULL;
	if ( size == 0 )
		return false;
	// alloc mem for new obj
	void* p	=::HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, size);
	if (p)
	{
		::InterlockedIncrement(&m_uNumAllocsInHeap);

		if (m_uNumAllocsInHeap > m_lMaxAllocsInHeap)
			::InterlockedExchange(&m_lMaxAllocsInHeap, m_uNumAllocsInHeap);
	}
	
	return p;
#else
	if(m_nClassSize != size)
	{
		assert(false);
		return NULL;
	}

	if (m_vecFree.empty())
	{
		++m_lMaxAllocsInHeap;
		return malloc(size);
	}
	
	void* p = m_vecFree.back();
	m_vecFree.pop_back();
	return p;
#endif

}


void CMemHeap::Free( void* p )
{
#ifdef MEMHEAP
	if ( NULL == p )
		return;
	if ( !m_hHeap )
		return;
	// free it...
	if ( ::HeapFree(m_hHeap, 0, p) )
	{
		::InterlockedDecrement(&m_uNumAllocsInHeap);
	}
#else
	m_vecFree.push_back(p);
#endif
}

#ifdef MEMHEAP
bool CMemHeap::IsValidPt(void* p)
{
	if (!m_hHeap || !p)
		return false;
	
	if ((DWORD)p < 0x00010000 || (DWORD)p >= 0x7FFEFFFF)	// user address range from 1M--2G-64k
		return false;
	
	if (::IsBadCodePtr((FARPROC)p))
		return false;
	
	return (::HeapValidate(m_hHeap, 0, p)!=FALSE);
}
#endif
#endif
