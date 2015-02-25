#ifndef __ERR_H
#define __ERR_H

#include <dmpcfg.h>


#ifdef __cplusplus
extern "C" {
#endif

DMPAPI_C(void) err_print(const char* fmt, ...);
DMPAPI_C(void) err_printk(const char* fmt, ...);

DMPAPI(bool) err_Dump(void);
DMPAPI(void) err_DumpAll(void);
DMPAPI(bool) err_AutoDump(void);

DMPAPI(bool) err_Init(const char* logfile);
DMPAPI(bool) err_Close(void);

#ifdef __cplusplus
}
#endif
#endif

