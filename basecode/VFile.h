#ifndef VFile_hxx
#define VFile_hxx

#include <time.h>

#include "basecode/BaseType.h"
#include "basecode/StringData.h"
#include "basecode/MemStream.h"

class VFile
{
public:
	enum SEEKFLAG
	{
		VFILE_BEGIN = SEEK_SET,
		VFILE_CUR = SEEK_CUR,
		VFILE_END = SEEK_END,
	};

	enum OPENFLAG
	{
		NONE = 1,
		READ = 2,
		READWRITE = 4,
		APPEND = 8,
		CREATE = 16,
		MODIFY = 32,
	};

public:
	VFile();
	VFile( const Data& fullFilePath, int mode=NONE );
	virtual ~VFile();

	bool openFile( const Data& fullFilePath, int mode=NONE );
	void closeFile();
	bool seekTo( int to, int orign );
	int read( unsigned char* buf, int len );
	bool readData( unsigned char* buf, int len );

	int write( const unsigned char* buff, uint32 len );
	int write( uint32 len, const unsigned char* buff );
	int write( const Data& data );
	int writeLine( const Data& date="" );
	bool writeData( const void* buff, uint32 len );
	int readLine( char* buff, int maxlen=1024 );
	int readLine( Data& dtLine, int maxlen=1024 );
	void flush();
	inline bool isEof()
	{
		return ( feof( hFile ) != 0 );
	}

	Data getFilePath() const;
	Data getExtName() const;
	uint32 getFileLen();
	uint32 getFileLeftLen();

	static uint32 getFileLen( const Data &file );
	static bool chmode( const Data& file, const Data& mod );
	static bool isFileExist( const Data& fullFilePath );
	static bool deleteFile( const Data& fullFilePath );
	static bool rename( const Data& path, const Data& from, const Data& to );
	static bool move( const Data& from, const Data& to, bool replace );
	static bool copyFile( const Data& from, const Data& to, bool replace=true );
	static bool getTime( const Data& path, uint64& create, uint64& access, uint64& write );
	static bool resetAccessTime( const Data& path );
	
	long getFilePos();
	bool copyTo( const Data& target );
	ref_ptr<MemStream> getAllData();
	bool getAllData( uint8* data );
	ref_ptr<MemStream> getLeftData();
	bool isOpen()
	{
		return _bOpen;
	}
private:
	Data _path;
	bool _bOpen;
	FILE* hFile;

};

#endif
