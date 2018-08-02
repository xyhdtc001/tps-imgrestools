#include "7z/LZMA/Threads.h"

#ifdef WINDOWS
    #include "Threads_win.c"
#else
    #ifdef METRO
        #include "Threads_win.c"
    #else
        #include "Threads_unix.c"
    #endif
#endif
