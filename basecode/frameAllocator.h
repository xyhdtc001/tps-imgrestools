#ifndef _FRAMEALLOCATOR_H_
#define _FRAMEALLOCATOR_H_

#include "basecode/basetype.h"


#define FRAMEALLOCATOR_BYTE_ALIGNMENT 4

class FrameAllocator
{
   static uint8*   smBuffer;
   static uint32   smHighWaterMark;
   static uint32   smWaterMark;

public:
   inline static void init(const uint32 frameSize);
   inline static void destroy();

   inline static void* alloc(const uint32 allocSize);

   inline static void setWaterMark(const uint32);
   inline static uint32  getWaterMark();
   inline static uint32  getHighWaterMark();

};

void FrameAllocator::init(const uint32 frameSize)
{
   smBuffer = new uint8[frameSize];
   smWaterMark = 1;
   smHighWaterMark = frameSize;
}

void FrameAllocator::destroy()
{
   delete [] smBuffer;
   smBuffer = NULL;
   smWaterMark = 0;
   smHighWaterMark = 0;
}


void* FrameAllocator::alloc(const uint32 allocSize)
{
   uint32 _allocSize = allocSize; 
   smWaterMark = ( smWaterMark + ( FRAMEALLOCATOR_BYTE_ALIGNMENT - 1 ) ) & (~( FRAMEALLOCATOR_BYTE_ALIGNMENT - 1 ));

   uint8* p = &smBuffer[smWaterMark];
   smWaterMark += _allocSize;

   return p;
}


void FrameAllocator::setWaterMark(const uint32 waterMark)
{
   smWaterMark = waterMark;
}

uint32 FrameAllocator::getWaterMark()
{
   return smWaterMark;
}

uint32 FrameAllocator::getHighWaterMark()
{
   return smHighWaterMark;
}


class FrameAllocatorMarker
{
   uint32 mMarker;

public:
   FrameAllocatorMarker()
   {
      mMarker = FrameAllocator::getWaterMark();
   }

   ~FrameAllocatorMarker()
   {
      FrameAllocator::setWaterMark(mMarker);
   }

   void* alloc(const uint32 allocSize) const
   {
      return FrameAllocator::alloc(allocSize);
   }

   template<typename T>
   T* alloc(const uint32 numElements) const
   {
      return reinterpret_cast<T *>(FrameAllocator::alloc(numElements * sizeof(T)));
   }
};

template <class T>
inline T* constructInPlace(T* p)
{
	return new ( p ) T;
}

template <class T>
inline void destructInPlace(T* p)
{
	p->~T();
}

template<class T>
class FrameTemp
{
protected:
   uint32 mWaterMark;
   T *mMemory;
   uint32 mNumObjectsInMemory;

public:
   FrameTemp( const uint32 count = 1 ) : mNumObjectsInMemory( count )
   {
      mWaterMark = FrameAllocator::getWaterMark();
      mMemory = reinterpret_cast<T *>( FrameAllocator::alloc( sizeof( T ) * count ) );

      for( int i = 0; i < mNumObjectsInMemory; i++ )
         constructInPlace<T>( &mMemory[i] );
   }

   /// Destructor restores the watermark
   ~FrameTemp()
   {
      for( int i = 0; i < mNumObjectsInMemory; i++ )
         destructInPlace<T>( &mMemory[i] );

      FrameAllocator::setWaterMark( mWaterMark );
   }

   T* operator ~() { return mMemory; };

   const T* operator ~() const { return mMemory; };


   T& operator +() { return *mMemory; };

   const T& operator +() const { return *mMemory; };

   T& operator *() { return *mMemory; };
   const T& operator *() const { return *mMemory; };

   T** operator &() { return &mMemory; };
   const T** operator &() const { return &mMemory; };

   operator T*() { return mMemory; }
   operator const T*() const { return mMemory; }

   operator T&() { return *mMemory; }
   operator const T&() const { return *mMemory; }

   operator T() { return *mMemory; }
   operator const T() const { return *mMemory; }
   
   T& operator []( uint32 i ) { return mMemory[ i ]; }
   const T& operator []( uint32 i ) const { return mMemory[ i ]; }

   T& operator []( int32 i ) { return mMemory[ i ]; }
   const T& operator []( int32 i ) const { return mMemory[ i ]; }

   T *address() const { return mMemory; }
   uint32 size() const { return mNumObjectsInMemory; }
};

//-----------------------------------------------------------------------------
// FrameTemp specializations for types with no constructor/destructor
#define FRAME_TEMP_NC_SPEC(type) \
   template<> \
   inline FrameTemp<type>::FrameTemp( const uint32 count ) \
   { \
      mWaterMark = FrameAllocator::getWaterMark(); \
      mMemory = reinterpret_cast<type *>( FrameAllocator::alloc( sizeof( type ) * count ) ); \
   } \
   template<>\
   inline FrameTemp<type>::~FrameTemp() \
   { \
      FrameAllocator::setWaterMark( mWaterMark ); \
   } \

FRAME_TEMP_NC_SPEC(char);
FRAME_TEMP_NC_SPEC(float);
FRAME_TEMP_NC_SPEC(double);
FRAME_TEMP_NC_SPEC(bool);
FRAME_TEMP_NC_SPEC(int);
FRAME_TEMP_NC_SPEC(short);

FRAME_TEMP_NC_SPEC(unsigned char);
FRAME_TEMP_NC_SPEC(unsigned int);
FRAME_TEMP_NC_SPEC(unsigned short);

#undef FRAME_TEMP_NC_SPEC

//-----------------------------------------------------------------------------

#endif  // _H_FRAMEALLOCATOR_
