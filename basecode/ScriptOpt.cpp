#include "basecode/ScriptOpt.h"
#include "basecode/VFile.h"
#include "basecode/BaseLog.h"
#include "basecode/Directory.h"

#ifdef WIN32
#include <direct.h>
#endif

/*
bool PackScriptFunc( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam )
{
	VFile scriptFile;
	if ( !scriptFile.openFile(filepath+filename,VFile::READ) )
		return false;

	ref_ptr<MemStream> memStream = scriptFile.getAllData();
	Data relaPath = relaDir + filename;
	PS_FILE* file = ps_fopen( relaPath.c_str(), PS_FOPEN_CRW, &g_state );
	if ( file == NULL )
	{
		LogOut( RES, LOG_ERR, "open vfs file %s err", relaPath.c_str() );
		return false;
	}

	if (memStream)
	{
		uint32 streamLen = memStream->getStreamSize();
		if (SZ_OK != ps_fwrite_lzma((u8*)memStream->getPoint(), streamLen, file))
		{
			LogOut( RES, LOG_ERR, "write vfs zfile %s err", relaPath.c_str() );
			assert( 0 );
		}
	}
	else
	{
		if (SZ_OK != ps_fwrite_lzma(0, 0, file))
		{
			LogOut( RES, LOG_ERR, "write vfs zfile %s err", relaPath.c_str() );
			assert( 0 );
		}
	}

	ps_fclose( file );
	return true;
}


bool ScriptOpt::packFolder( const Data& folder, const char* filename )
{
	PVFS_INFO vfs_info = { filename, 30000, 1024, 1024, 1024 };
	ps_init( &g_state, &vfs_info, 1 );

	Data dtPath = folder;
	VDirectory dir;
	dtPath.makePath();
	if ( !dir.EnumAllFunc(dtPath,"",PackScriptFunc,NULL,this) )
		return false;
	ps_uninit( &g_state );
	return true;
}


i32 copyout(const char *cmd, const char *cmd2, PS_STATE* s)
{
	const u32 BUF_MAX = 1024;
	i32 ret;
	PS_FILE *fin;
	FILE *fout;
	u8* buf;
	u32 size;
	// 	PS_ZIP_STATE zstate;

	s->ret = PS_OK;
	fin = ps_fopen(cmd, PS_FOPEN_RW, s);
	if (s->ret != PS_OK) {
		return s->ret;
	}

	fopen_s(&fout, cmd2, "wb");
	if (!fout) {
		ps_fclose(fin);
		return PS_ITEM_NOT_FOUND;
	}

	// 	buf = (u8 *)malloc(BUF_MAX);
	size = ps_lzma_get_unpack_size(fin);
	buf = (u8 *)malloc(size);

	// 	ret = ps_fzread_begin(fin, 0, &zstate);
	// 	if (ret == Z_OK)
	// 		do {
	// 			size = ps_fzread(buf, BUF_MAX, &zstate);
	// 			fwrite(buf, 1, size, fout);
	// 		} while (size);
	// 	ret = ps_fzread_end(&zstate);
	ret = ps_fread_lzma(buf, size, fin);
	fwrite(buf, 1, size, fout);

	free(buf);
	fclose(fout);
	ps_fclose(fin);

	return ret;
}

i32 xcopyout(const char *cmd, const char *cmd2, PS_STATE* s)
{
	int ret;
	static char name[1024];
	_mkdir(cmd2);
	_chdir(cmd2);

	ret = ps_chdir(cmd, s);
	if (ret == PS_OK) {
		int num = pvfs_get_item_num(s->cur);
		for (int i = 0; i < num; i++)
		{
			if (!pvfs_get_item_handle(i, s->cur))
				continue;

			sprintf_s(name, "%s", pvfs_get_item_name(i, s->cur));
			if (pvfs_get_item_type(i, s->cur) == PVFS_DIR_ITEM_TYPE_FILE) {
				ret = copyout(name, name, s);
				if (ret != PS_OK)
					return ret;
			}
			else if (pvfs_get_item_type(i, s->cur) == PVFS_DIR_ITEM_TYPE_DIR)
			{
				char* const tmp = new char[128];
				if(strcmp(name, ".")==0 || strcmp(name, "..")==0)
					continue;

// 				if (cd(tmp))
// 				{
// 					xcopyout(tmp);
// 					cd("..");
// 				}
				ret = xcopyout(name, name, s);
				if (ret != PS_OK)
					return ret;
			}
		}

		ps_chdir("..", s);
	}

	_chdir("..");

	return ret;
}

bool ScriptOpt::fromVfsToDisk(const char *folder, const char* filename)
{
	PVFS_INFO vfs_info = { filename, 30000, 1024, 1024, 1024 };
	ps_init( &g_state, &vfs_info, 0 );

	return xcopyout(".", folder, &g_state);
}

*/