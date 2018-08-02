#ifndef _PLATFORMINTRINSICS_H_
#define _PLATFORMINTRINSICS_H_

#include "basecode/basetype.h"

#ifdef WINDOWS
#include <intrin.h>
#include <windows.h>


inline void dFetchAndAdd( volatile uint32& ref, uint32 val )
{  
	InterlockedExchangeAdd( ( volatile long* ) &ref, val );
}
inline void dFetchAndAdd( volatile int32& ref, int32 val )
{
	InterlockedExchangeAdd( ( volatile long* ) &ref, val );
}


// Compare-And-Swap

inline bool dCompareAndSwap( volatile uint32& ref, uint32 oldVal, uint32 newVal )
{
 	return ( InterlockedCompareExchange( ( volatile long* ) &ref, newVal, oldVal ) == oldVal );
}

inline bool dCompareAndSwap( volatile uint64& ref, uint64 oldVal, uint64 newVal )
{
	return ( InterlockedCompareExchange64( ( volatile __int64* ) &ref, newVal, oldVal ) == oldVal );
}

/// Performs an atomic read operation.
inline uint32 dAtomicRead( volatile uint32 &ref )
{
	return InterlockedExchangeAdd( ( volatile long* )&ref, 0 );
}



template< typename T >
inline bool dCompareAndSwap( T* volatile& refPtr, T* oldPtr, T* newPtr )
{
   return dCompareAndSwap( *reinterpret_cast< volatile uint32* >( &refPtr ), ( uint32 ) oldPtr, ( uint32 ) newPtr );
}

// Test-And-Set

inline bool dTestAndSet( volatile uint32& ref )
{
   return dCompareAndSwap( ref, 0, 1 );
}
inline bool dTestAndSet( volatile uint64& ref )
{
   return dCompareAndSwap( ref, 0, 1 );
}
#endif
#endif // _PLATFORMINTRINSICS_H_
