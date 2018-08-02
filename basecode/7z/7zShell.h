#ifndef __7Z_SHELL_H__
#define __7Z_SHELL_H__

#include "basecode/memstream.h"

ref_ptr<MemStream> DecodeToMemStream_7z(const char *file, const char *data, const char* password = NULL);
int DecodeToBuffer_7z(void *buf, int size, const char *file, const char *data, const char* password = NULL);
int GetUnpackSize_7z(const char *file, const char *data, const char* password = NULL);

#endif