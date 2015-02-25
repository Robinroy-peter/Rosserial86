#define __ERR_LIB
////////////////////////////////////////////////////////////////////////////////
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////



#include <dmpcfg.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
    #include <unistd.h>
#endif

#include "io.h"
#include "irq.h"
#include "err.h"

#ifndef DMP_DOS_DJGPP
    #define DPMI_END_OF_LOCKED_FUNC(fname)
    #define DPMI_END_OF_LOCKED_STATIC_FUNC(fname)
#endif


static FILE* ERR_outputDevice = stderr;
static char* ERR_msgPool = NULL;
#ifdef DMP_DOS_BC
    #define MSGPOOL_SIZE    (1<<4)
#else
    #define MSGPOOL_SIZE    (1<<6)
#endif
static int ERR_msgStart = 0;
static int ERR_msgEnd   = 0;

// lock functions for ERR_msgPool[...]
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)

    #define MSGPOOL_LOCK()   io_DisableINT()
    #define MSGPOOL_UNLOCK() io_RestoreINT()

#elif defined(DMP_WINDOWS) || defined(DMP_LINUX)

    // TODO: use mutex in MSGPOOL()
    #define MSGPOOL_LOCK()   
    #define MSGPOOL_UNLOCK() 

#endif

__dmp_inline(void) print2pool(const char* fmt, va_list args) {
    if (ERR_msgPool == NULL) return;

    MSGPOOL_LOCK();
    {
        #ifdef DMP_DOS_BC
            vsprintf(&ERR_msgPool[ERR_msgEnd << 8], fmt, args);
        #else
            vsnprintf(&ERR_msgPool[ERR_msgEnd << 8], 256, fmt, args);
        #endif
        
        ERR_msgEnd = (ERR_msgEnd + 1) & MSGPOOL_SIZE;
        if (ERR_msgEnd == ERR_msgStart) ERR_msgStart = (ERR_msgStart + 1) & MSGPOOL_SIZE;
    }
    MSGPOOL_UNLOCK();
}

DMPAPI_C(void) err_print(const char* fmt, ...) {  // can be used without err_Init()
    va_list args;

    va_start(args, fmt);

    if (irq_InInterrupt() == true)
        print2pool(fmt, args);  // actually in DJGPP this isn't really safe if virtual memory isn't disabled :p
    else
    {
        vfprintf(ERR_outputDevice, fmt, args);
        fflush(ERR_outputDevice);
    }

    va_end(args);
} DPMI_END_OF_LOCKED_FUNC(err_print)

DMPAPI_C(void) err_printk(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    print2pool(fmt, args);
    va_end(args);
} DPMI_END_OF_LOCKED_FUNC(err_printk)


DMPAPI(bool) err_Dump(void) {
    bool has_msg;
    char buf[256];

    if (ERR_msgPool == NULL) return false;

    MSGPOOL_LOCK();
    {
        if (ERR_msgStart == ERR_msgEnd)
            buf[0] = '\0';
        else
        {
            strncpy(buf, &ERR_msgPool[ERR_msgStart << 8], 256);
            ERR_msgStart = (ERR_msgStart + 1) & MSGPOOL_SIZE;
        }
        has_msg = (ERR_msgStart != ERR_msgEnd)? true : false;
    }
    MSGPOOL_UNLOCK();

    err_print(buf);
    return has_msg;
}

DMPAPI(void) err_DumpAll(void) {
    while (err_Dump() == true);

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
        fsync(fileno(ERR_outputDevice));  // fflush() has no effect in DJGPP
    #endif
}

DMPAPI(bool) err_AutoDump(void) {
    if (atexit(err_DumpAll) == 0) return true; else return false;
}


DMPAPI(bool) err_Init(const char* logfile) {
    #if defined(DMP_DOS_DJGPP)
        DPMI_LOCK_VAR(ERR_msgPool);
        DPMI_LOCK_VAR(ERR_msgStart);
        DPMI_LOCK_VAR(ERR_msgEnd);
        
        DPMI_LOCK_FUNC(err_print);
        DPMI_LOCK_FUNC(err_printk);
    #endif

    if (ERR_msgPool == NULL)
    {
        if ((ERR_msgPool = (char*)ker_Malloc(MSGPOOL_SIZE << 8)) != NULL)
            ERR_msgPool[0] = ERR_msgPool[(MSGPOOL_SIZE << 8) - 1] = '\0';

        ERR_msgStart = 0; ERR_msgEnd = 0;
    }

	if (logfile == NULL)
	{
		ERR_outputDevice = stderr;
		return true;
	}

    if ((ERR_outputDevice != stderr) && (ERR_outputDevice != NULL)) fclose(ERR_outputDevice);
	if ((ERR_outputDevice = fopen(logfile, "w")) != NULL) return true;

	ERR_outputDevice = stderr;
    return false;
}

DMPAPI(bool) err_Close(void) {
    if (ERR_msgPool != NULL)
    {
        ker_Mfree(ERR_msgPool);
        ERR_msgPool = NULL;
    }

    if ((ERR_outputDevice != stderr) && (ERR_outputDevice != NULL)) fclose(ERR_outputDevice);
	ERR_outputDevice = stderr;
	return true;
}

