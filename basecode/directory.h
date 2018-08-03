#if !defined VDIRECTORY_H__
#define VDIRECTORY_H__

#ifdef WINDOWS
#ifdef METRO
#else
#endif
#else
#include <sys/types.h>
#include <dirent.h>
#endif

#include <vector>
#include <set>
#include "basecode/VFile.h"

typedef bool (*EnumFunc)( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam );
typedef bool (*FolderFunc)( const Data& folderPath, const Data& folder, void* pParam );

class VDirectory
{
public:
	VDirectory();
	VDirectory( const Data& dir, bool create = false );
	bool open( const Data& directory, bool create = false );
	bool EnumFiles( EnumFunc func, void* pParam=NULL );
	bool EnumDirs( FolderFunc func, void* pParam=NULL );

	Data getPath();

	void getFiles();
	void getFilesVec( std::vector<Data>* vecData );
	void getSubDirs();
	void getSubDirsSet( std::set<Data>* pSet );
	Data getFileName( unsigned int index );
	Data getDirName( unsigned int index );
	static bool removeAll( const Data& path, bool recursive=true );
	static bool removeAllBySubfix( const Data& path, const Data& subfix, bool recursive=true );

	uint32 getFileNum();
	uint32 getDirNum();
	const std::vector<Data>& getDirVec() { return _dirVec; };

	void setIncludeFile( const Data& data );
	void setExcludeFile( const Data& data );
	void setExcludeDir( const Data& data );

	bool isIncludeFile( const Data& file );
	bool isExcludeFile( const Data& file );
	bool isExcludeDir( const Data& dir );


	static bool rename( const Data& from, const Data& to );
	static bool EnumAllFunc( const Data& fullDir, const Data& relaDir, EnumFunc func, FolderFunc folderFunc=NULL, void* pParam=NULL ,bool bChildDir=true);
	static bool removeDir( const Data& dir );
	static bool isDirectoryExist( const Data& dir );
	static bool createDir( const Data& dir );
	static bool createFileDir( const Data& dir );
	static bool chDir( const Data& dir );
	static bool copyDir( const Data& srcDir, const Data& tarDir );
	static bool isDirEmpty( const Data& fullDir );

public:
	std::vector< Data > _fileVec;
	std::vector< Data > _dirVec;

protected:
	Data _directory;
	std::vector< Data > _vecIncludeFile;
	std::vector< Data > _vecExcludeFile;
	std::vector< Data > _vecExcludeDir;
	bool _bIncludeFile;
	bool _bExcludeFile;
	bool _bExcludeDir;
};

#endif
