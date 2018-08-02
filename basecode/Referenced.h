#pragma once

#include "basecode/basehdr.h"
#include "basecode/MemHeap.h"

#ifdef WIN32
	#define MEMHEAP_DECLARATION(cls)												\
	public:																			\
		/*·ÀÖ¹ÄÚ´æËéÆ¬*/															\
		void   operator delete	(void* p)	{ s_heap##cls.Free(p); }			\
		void*  operator new	(size_t size)	{ return s_heap##cls.Alloc(size); }	\
		static CMemHeap	s_heap##cls;											\


#define	MEMHEAP_IMPLEMENTATION(cls)		CMemHeap	cls::s_heap##cls(#cls, sizeof(cls));

#else
	#define MEMHEAP_DECLARATION(cls)
	#define	MEMHEAP_IMPLEMENTATION(cls)
#endif

/** Base class from providing referencing counted objects.*/
class CReferenced
{
public:
	virtual ~CReferenced();
	virtual int ref() const;
	virtual int unref() const;

public:
	CReferenced(){_refCount = 0;}
    
    explicit CReferenced(bool threadSafeRefUnref){_refCount = 0;}

    CReferenced(const CReferenced&){_refCount = 0;}

    inline CReferenced& operator = (const CReferenced&) { return *this; }

	int unref_nodelete() const{return --_refCount;}
    
    /** Return the number pointers currently referencing this object. */
    inline int referenceCount() const { return _refCount; }
   
protected:
    mutable int _refCount;
};

inline CReferenced::~CReferenced()
{
	if (_refCount>0)
	{
		//printf("·Ç·¨ÊÍ·Å\n");
	}
}
inline int CReferenced::ref() const
{
    return ++_refCount;
}

inline int CReferenced::unref() const
{
    int newRef;
    newRef = --_refCount;
    bool needDelete = (newRef == 0);
	if (needDelete)
	{
		delete this;
	}
	
    return newRef;
}

// intrusive_ptr_add_ref and intrusive_ptr_release allow
// use of osg CReferenced classes with boost::intrusive_ptr
inline void intrusive_ptr_add_ref(CReferenced* p) { p->ref(); }
inline void intrusive_ptr_release(CReferenced* p) { p->unref(); }
