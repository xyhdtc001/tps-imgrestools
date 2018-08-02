/* Threads.h -- multithreading library
2008-11-22 : Igor Pavlov : Public domain */

#ifndef ___THREADS_H__
#define ___THREADS_H__

#ifdef WINDOWS
#include "Threads_win.h"
#else
#ifdef METRO
#include "Threads_win.h"
#else
#include "Threads_unix.h"
#endif
#endif


#endif

