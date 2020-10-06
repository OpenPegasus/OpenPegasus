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
// This example of how to call lib$initialize from C++ was taken
// from the internal notes file: cxxc_bugs 11466.4
//
#include <unixlib.h>      // decc$feature_get_index(), set_value()

#if defined __USE_STD_IOSTREAM
#include <iostream>       // cout.sync_with_stdio(false), etc.
#endif

#include <stdio.h>        // perror()
#include <errno.h>        // errno

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*fptr_t)(void);

//
// Sets current value for a feature
//
static void set(char *name, int value)
{
  int index;
  errno = 0;

  index = decc$feature_get_index(name);

  if (index == -1 ||
      (decc$feature_set_value(index, 1, value) == -1 &&
       errno != 0))
  {
    perror(name);
  }
}

static void set_coe(void)
{
  int logname_cache = 60; // 60 seconds = 1 min. cache.

  set ("DECC$ARGV_PARSE_STYLE", TRUE);
  set ("DECC$ENABLE_GETENV_CACHE", TRUE);
  set ("DECC$ENABLE_TO_VMS_LOGNAME_CACHE", logname_cache);
  set ("DECC$FILE_SHARING", TRUE);
  set ("DECC$DISABLE_TO_VMS_LOGNAME_TRANSLATION", TRUE);
  set ("DECC$EFS_CASE_PRESERVE", TRUE);
  set ("DECC$EFS_CHARSET", TRUE);
  set ("DECC$EFS_FILE_TIMESTAMPS", TRUE);
  set ("DECC$FILENAME_UNIX_NO_VERSION", TRUE);
  set ("DECC$STDIO_CTX_EOL", TRUE);
#if !defined FILENAME_UNIX_REPORT_FALSE
  set ("DECC$FILENAME_UNIX_REPORT", TRUE);
#else
  set ("DECC$FILENAME_UNIX_REPORT", FALSE);
#endif
  set ("DECC$FILE_OWNER_UNIX", TRUE);
  set ("DECC$FILE_PERMISSION_UNIX", TRUE);
  set ("DECC$READDIR_DROPDOTNOTYPE", TRUE);
  set ("DECC$FILENAME_UNIX_ONLY", FALSE);
  set ("DECC$UMASK", 027);
  set ("DECC$RENAME_NO_INHERIT", TRUE);
//  set ("DECC$PIPE_BUFFER_QUOTA", 4096);
  set ("DECC$PIPE_BUFFER_SIZE", 4096);

// 11JAN07 [cbh] With _USE_STD_STAT defined, POSIX_STYLE_UID can be enabled.
//               false (the default) is required because getpwnam_r()
//               always returns the UIC style group name which will not
//               be found by getgrgid_r in POSIX (not UIC) mode
//               when POSIX_STYLE_UID is enabled.
  set ("DECC$POSIX_STYLE_UID", FALSE);

// PTR 75-109-857: Set DECC$SETVBUF_BUFFERRED
// crtl_internal note 2232.
// for one-char-per-line problem where lines are greater than 256.
//
// Note: SETVBUF_BUFFERED requires OpenVMS 8.3 or you get a message
//       like this at run-time:
// $ TestXmlPrint x.xml
// DECC$SETVBUF_BUFFERED: no such file or directory
//
// 1-Feb-08 [cbh] Enable since only 8.3 or later is now supported.
// 9-Aug-06 [cbh] Comment out SETVBUF_BUFFERED until we see a case
//                where we fail without it since it causes problems on
//                pre-OpenVMS 8.3 and I don't know if it will go in any
//                tima kits and don't know what happens on Alpha.
  set ("DECC$SETVBUF_BUFFERED", TRUE);
}

//
// unsync_streams_from_stdout
//
// When applications are run with the output directed to a file,
// the output can be broken up across multiple lines.
//
// On Unix, output files are streams, not record oriented. On OpenVMS
// you will see this problem when under circumstances such as:
//
// - A batch job (all output goes to the .log file)
// - spawn/out=a.log
// - A subprocess
// - set host/log=a.log
//
// This code fixes that problem.
//
// Note: This fix only works for __USE_STD_IOSTREAM.
// Note: See also: 75-109-857
//
#if defined __USE_STD_IOSTREAM

static void unsync_streams_from_stdout(void)
{

  cout.sync_with_stdio(false);
  cerr.sync_with_stdio(false);
  cerr.unsetf(ios::unitbuf);

  wcout.sync_with_stdio(false);
  wcerr.sync_with_stdio(false);
  wcerr.unsetf(ios::unitbuf);

}
#endif

static void init_lib(void)
{
  set_coe();
#if defined __USE_STD_IOSTREAM
  unsync_streams_from_stdout();
#endif
}

#pragma extern_model save

#pragma extern_model strict_refdef "LIB$INITIALIZE" gbl,noexe,nowrt,noshr,long
  extern const fptr_t y = init_lib;

#pragma extern_model restore

#ifdef __cplusplus
} // extern "C"
#endif

