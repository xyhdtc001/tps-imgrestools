#pragma once

#ifdef METRO
#include "basecode/basehdr.h"

#ifdef METRO
#include <SDKDDKVer.h>
#include <collection.h>
#include <ppltasks.h>
#endif

using namespace Concurrency;
using namespace Windows::Foundation;

Platform::String^ str2wstr( const char* str, uint32 len );
const char* wstr2str( Platform::String^ str );

#endif