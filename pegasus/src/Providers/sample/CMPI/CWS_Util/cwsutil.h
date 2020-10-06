//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef CWSUTIL_H
#define CWSUTIL_H

#include <time.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h> // Only needed for CMPI_EXTERN_C
#ifdef __cplusplus
//extern "C" {
#endif

# define CWSSetStatusWithChars(rcp_,chars_) \
    st.msg=_broker->eft->newString(_broker,(chars_),NULL); \
    st.rc = rcp_;

/* ------------------------------------------------------------------
 * Utilities for file info retrieval
 * ----------------------------------------------------------------- */

#define CWS_MAXPATH    1025

#define CWS_TYPE_DIR   'd'
#define CWS_TYPE_PLAIN 'f'

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
#define SINT64 __int64
#define strcasecmp _stricmp
CMPI_EXTERN_C char * dirname(char *path);
#else
#define SINT64 long long
#endif

struct _CWS_FILE {
  char      cws_name[CWS_MAXPATH];
  SINT64 cws_size;
  time_t    cws_ctime;
  time_t    cws_mtime;
  time_t    cws_atime;
  unsigned  cws_mode;
};
typedef struct _CWS_FILE CWS_FILE;

/* ------------------------------------------------------------------
 * File Enumeration Support, use like this:
 *
 *  CWS_FILE filebuf;
 *  void * hdl = CWS_Begin_Enum("/test",CWS_TYPE_FILE);
 *  if (hdl) {
 *    while(CWS_Next_Enum(hdl,&filebuf) {...}
 *    CWS_End_Enum(hdl);
 *  }
 * ----------------------------------------------------------------- */



CMPI_EXTERN_C void* CWS_Begin_Enum(const char *topdir, int filetype);
CMPI_EXTERN_C int CWS_Next_Enum(void *handle, CWS_FILE* cwsf);
CMPI_EXTERN_C void CWS_End_Enum(void *handle);

CMPI_EXTERN_C int CWS_Get_File(const char *file, CWS_FILE* cwsf);
CMPI_EXTERN_C int CWS_Update_File(CWS_FILE* cwsf);
CMPI_EXTERN_C int CWS_Update_FileSize(CWS_FILE* cwsf, const char *fn);
CMPI_EXTERN_C int CWS_Create_Directory(CWS_FILE* cwsf);
CMPI_EXTERN_C int CWS_Get_FileType(
                      const char *file,
                      char* typestring,
                      size_t tslen);

#ifdef __cplusplus
/*}*/
#endif

#endif
