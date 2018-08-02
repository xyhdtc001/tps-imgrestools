#include "basecode/memstream.h"
#include "basecode/BaseLog.h"


MemStream::MemStream( uint32 size )
: _inStart(0), _outStart(0), _size( size )
{
	if ( size == 0 )
		setStatus( EOS );
	
	_data[_size] = '\0';
}


void* MemStream::operator new( size_t size, size_t len )
{
	return malloc( size+len );
}


void MemStream::operator delete( void * p, size_t size )
{
 	free( p );
}


void MemStream::operator delete( void * p )
{
 	free( p );
}


uint32 MemStream::getStreamSize()
{
	return _size;
}


void MemStream::setSteamSize( uint32 size )
{
	if ( size > _size )
		return;
	_size = size;
}


bool MemStream::hasCapability(const Capability caps) const
{
	return true;
}


bool MemStream::read( const uint32 size,  void* buff )
{
	if ( _outStart+size > _size )
		return false;
	memcpy( (void*)buff, _data+_outStart, size );
	_outStart += size;
	if ( _outStart >= _size )
		Stream::setStatus( EOS );
	return true;
}


uint32 MemStream::readEx( uint32 size, void *buf )
{
	if ( _outStart+size > _size )
		size = _size - _outStart;

	memcpy( (void*)buf, _data+_outStart, size );
	_outStart += size;
	if ( _outStart >= _size )
		Stream::setStatus( EOS );

	return size;
}


bool MemStream::eof()
{
	if ( _outStart >= _size )
		return true;
	else
		return false;
}


bool MemStream::write( const uint32 size, const void* buff )
{
	if ( _inStart+size > _size )
		return false;
	memcpy( _data+_inStart, (void*)buff, size );
	_inStart += size;
	return true;
}


uint32 MemStream::getWritePos() const
{
	return _inStart;
}


bool MemStream::setWritePos( const uint32 pos )
{
	if ( pos >= _size )
		return false;
	_inStart = pos;
	return true;
}


bool MemStream::setReadPos( const uint32 pos )
{
	if ( pos >= _size )
		return false;
	_outStart = pos;
	return true;
}


void* MemStream::getPoint()
{
	return _data;
}


ref_ptr<MemStream> MemStream::clone()
{
	ref_ptr<MemStream> memStream = new ( _size ) MemStream( _size );
	memcpy( memStream->getPoint(), getPoint(), getStreamSize() );
	return memStream;
}


/*
void* FileMemStream::getPoint()
{
	memcpy( _fileData, _data, _size );
	strcat((char*)_fileData, '\0');
	return _fileData;/
}

std::multimap<uint32,void*> FileMemStream::_memPool;
FileMemStream::PoolFree FileMemStream::poolFree;

#ifdef _DEBUG
std::multimap< uint32,uint32 > FileMemStream::_sizePool;
#endif

VMutex FileMemStream::_mutex;
void* FileMemStream::operator new( size_t size, uint32 len )
{
	_mutex.lock();
	uint32 totalSize = size + len;
	std::multimap<uint32,void*>::iterator iter = _memPool.lower_bound( totalSize );
	void* pVal;
	if ( iter == _memPool.end() )
	{
		pVal = (uint8*)malloc( totalSize );
	}
	else
	{
		uint32 flen = iter->first;
		pVal = iter->second;
		_memPool.erase( iter );
	}


	#ifdef _DEBUG
	uint memHeapSize = 0;
	for ( std::multimap<uint32,void*>::iterator ii = _memPool.begin(); ii!=_memPool.end(); ii++ )
	{
		memHeapSize += ii->first;
	}
	bool bFind = false;
	for ( std::multimap<uint32,uint32>::iterator it=_sizePool.begin(); it!=_sizePool.end(); it++ )
	{
		if ( it->second == len )
		{
			uint32 num = it->first;
			num++;
			_sizePool.erase( it );
			_sizePool.insert( std::multimap< uint32,uint32 >::value_type(num,len) );
			bFind = true;
			break;
		}
	}
	if ( !bFind )
	{
		_sizePool.insert( std::multimap< uint32,uint32 >::value_type(1,len) );
	}
	#endif
	_mutex.unlock();
	return pVal;
}


void FileMemStream::operator delete( void* p, size_t size )
{
	_mutex.lock();
	#ifdef _DEBUG
	bool bFind = false;
	for ( std::multimap<uint32,uint32>::iterator it=_sizePool.begin(); it!=_sizePool.end(); it++ )
	{
		if ( it->second == ((FileMemStream*)p)->_size )
		{
			bFind = true;
			uint32 num = it->first;
			num--;
			_sizePool.erase( it );
			if ( num > 0 )
			{
				_sizePool.insert( std::multimap< uint32,uint32 >::value_type(num,((FileMemStream*)p)->_size) );
			}
			break;
		}
	}
	if ( bFind == false )
		assert( 0 );
	#endif


	uint32 len = ((FileMemStream*)p)->_size + size;
	if ( len < 2500000 )
		_memPool.insert( std::multimap<uint32,void*>::value_type(len,p) );
	else
		free( p );

	_mutex.unlock();
}

*/