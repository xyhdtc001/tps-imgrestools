#pragma once

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

typedef unsigned int uint32;

const int MAX_MEM_CHUNK_NUM = 32768;

struct chunk_info
{
	unsigned short used : 1;
	unsigned short size : 15;
};

template < uint32 CHUNK_SIZE, uint32 CHUNK_NUM > 
class MemroyPage
{
public:
	MemroyPage()
	{
		_chunkIndex = 0;
		_unusedChunkNum = CHUNK_NUM;
		memset( _mapInfo, 0, CHUNK_NUM*2 );
		_memory = (uint8*)malloc( CHUNK_SIZE*CHUNK_NUM );
		_next = NULL;
	}


	~MemroyPage()
	{
		free( _memory );
	}


	void* alloc( uint32 size )
	{
		uint32 num = size/CHUNK_SIZE + (size%CHUNK_SIZE>0?1:0);
		assert ( num < CHUNK_NUM );
		uint32 index = _chunkIndex;
		uint32 searchedNum = 0;
		uint32 searchMaxNum = CHUNK_NUM-num;
		while ( true )
		{
			uint32 iLeft = CHUNK_NUM-index;
			if ( iLeft < num )
			{
				searchedNum += iLeft;
				if ( searchedNum >= searchMaxNum )
					return NULL;
				index = 0;
			}
			else
			{
				bool bFind = true;
				for ( uint32 i=0; i<num; i++ )
				{
					if ( _mapInfo[index+i].used != 0 )
					{
						bFind = false;
						index = index + i + 1;
						searchedNum += i+1;
						if ( searchedNum >= searchMaxNum )
							return NULL;
						break;
					}
				}
				if ( bFind )
				{
					_mapInfo[index].size = num;
					for ( uint32 i=0; i<num; i++ )
						_mapInfo[index+i].used = 1;
					_chunkIndex = (index+num) % CHUNK_NUM;
					_unusedChunkNum -= num;
					return _memory + index * CHUNK_SIZE;
				}
			}
		}
	}


	void dealloc( void* ptr )
	{
		uint32 chunkIndex = ((uint8*)ptr-_memory)/CHUNK_SIZE;
		uint32 chunkNum = _mapInfo[chunkIndex].size;
		for ( uint32 i=0; i<chunkNum; i++ )
			_mapInfo[chunkIndex+i].used = 0;
		_unusedChunkNum += chunkNum;
	}

public:
	MemroyPage* _next;

protected:
	chunk_info _mapInfo[CHUNK_NUM];
	uint32 _chunkIndex;
	uint32 _unusedChunkNum;
	uint8* _memory;
};


template < uint32 CHUNK_SIZE, uint32 CHUNK_NUM > 
class MemPool
{
public:
	MemPool()
	{
		_endPage = _curPage = _beginPage = new MemroyPage< CHUNK_SIZE, CHUNK_NUM >;
		_pageSize = 1;
		
	}

	void* alloc( uint32 size )
	{
		uint8* buff;
		uint32 allocSize = size+sizeof(void*);
		for ( uint32 i=0; i<_pageSize; i++ )
		{
			if ( (buff=(uint8*)_curPage->alloc(allocSize)) == NULL )
			{
				_curPage = _curPage->_next;
				if ( _curPage == NULL )
					_curPage = _beginPage;
			}
			else
			{
				*((MemroyPage<CHUNK_SIZE,CHUNK_NUM>**)buff) = _curPage;
				return buff+sizeof(void*);
			}
		}
		_curPage = new MemroyPage< CHUNK_SIZE, CHUNK_NUM >;
		_pageSize++;
		_endPage->_next = _curPage;
		_endPage = _curPage;
		buff = (uint8*)_curPage->alloc( allocSize );
		*((MemroyPage<CHUNK_SIZE,CHUNK_NUM>**)buff) = _curPage;
		return  buff+sizeof(void*);
	}

	void dealloc( void* ptr )
	{
		void* pt = ((uint8*)ptr)-sizeof(void*);
		(*((MemroyPage<CHUNK_SIZE,CHUNK_NUM>**)pt))->dealloc( pt );
	}

	~MemPool()
	{
		MemroyPage< CHUNK_SIZE, CHUNK_NUM >* page = _beginPage;
		MemroyPage< CHUNK_SIZE, CHUNK_NUM >* nextPage = NULL;
		do 
		{
			nextPage = page->_next;
			delete page;
		} while ( (page=nextPage) != NULL );
	}
protected:
	MemroyPage< CHUNK_SIZE, CHUNK_NUM >* _beginPage;
	MemroyPage< CHUNK_SIZE, CHUNK_NUM >* _endPage;
	MemroyPage< CHUNK_SIZE, CHUNK_NUM >* _curPage;

	uint32 _pageSize;
};
