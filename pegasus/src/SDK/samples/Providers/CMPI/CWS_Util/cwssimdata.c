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

//   FILE_ENTRY(/Simulated/CMPI/tests/Providers: directory

#define FILE_ENTRY(n,s,c,m,a,o,t) {'f',#n,s,c,m,a,o,#t}
#define DIR_ENTRY(n,s,c,m,a,o,t) {'d',#n,s,c,m,a,o,#t}

#include <time.h>

typedef struct fileData {
   char ftype;
   char *name;
   long size;
   time_t cTime,mTime,aTime;
   int mode;
   char *type;
} FileData;

FileData files[]={
  DIR_ENTRY(/Simulated/CMPI/tests/Providers,
   4096,1078617123,1078617429,1078617123,0775,
   directory),
  DIR_ENTRY(/Simulated/CMPI/tests/Providers/mof,
   4096,1078582741,1078617428,1074173331,0755,
   directory),
  DIR_ENTRY(/Simulated/CMPI/tests/Providers/util,
   4096,1078617429,1078617429,1078617429,0755,
   directory),

  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_Directory.c,
   6291,1078582741,1078609332,1074173331,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_DirectoryContainsFile.c,
   10175,1078582741,1078609332,1074173331,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_DirectoryContainsFile.d,
   168,1078584175,1078609332,1078584175,0644,
   ASCII text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_DirectoryContainsFile.o,
   36592,1078589725,1078609332,1078589725,664,
   ELF 32-bit LSB relocatable; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_Directory.d,
   128,1078584047,1078609332,1078584047,0644,
   ASCII text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_Directory.o,
   32528,1078589724,1078609332,1078589724,664,
   ELF 32-bit LSB relocatable; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_FileUtils.c,
   4499,1078586815,1078609332,1078586815,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_FileUtils.d,
   299,1078586818,1078609332,1078586818,664,
   ASCII text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_FileUtils.h,
   1201,1078586790,1078609332,1078586790,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_FileUtils.o,
   39152,1078589724,1078609332,1078589724,664,
   ELF 32-bit LSB relocatable; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_PlainFile.c,
   7232,1078582741,1078609332,1074173331,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_PlainFile.d,
   128,1078584300,1078609332,1078584300,0644,
   ASCII text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/CWS_PlainFile.o,
   33768,1078589724,1078609332,1078589724,664,
   ELF 32-bit LSB relocatable; Intel 80386; version 1 (SYSV); not stripped),

  FILE_ENTRY(/Simulated/CMPI/tests/Providers/env,
   145,1078583810,1078609332,1078583810,0644,
   ASCII text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/libCWS_DirectoryContainsFile.so,
   36530,1078589725,1078609332,1078589725,0775,
   ELF 32-bit LSB shared object; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/libCWS_Directory.so,
   32688,1078589724,1078609332,1078589724,0775,
   ELF 32-bit LSB shared object; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/libCWS_FileUtils.so,
   36969,1078589724,1078609332,1078589724,0775,
   ELF 32-bit LSB shared object; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/libCWS_PlainFile.so,
   33832,1078589724,1078609332,1078589724,0775,
   ELF 32-bit LSB shared object; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/makefile,
   1031,1078582741,1078609332,1074173331,0644,
   ASCII make commands text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/README,
   2373,1078582741,1078609332,1074173331,0644,
  ASCII English text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/rules,
   214,1078582741,1078609332,1044439953,0644,
   ASCII text),

//   FILE_ENTRY(/Simulated/CMPI/tests/Providers/mof: directory

  FILE_ENTRY(/Simulated/CMPI/tests/Providers/mof/CWS_Excercise.mof,
   7598,1078582732,1078607940,1074173331,0644,
   C++ program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/mof/CWS_ExcerciseRegistration.mof,
   3496,1078582732,1078589724,1067876635,0644,
   C++ program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/mof/makefile,
   403,1078582732,1078589722,1074173331,0644,
   ASCII make commands text),

//   FILE_ENTRY(/Simulated/CMPI/tests/Providers/util: directory

  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwstest,
   20800,1078590300,1078608953,1078590300,0775,
   ELF 32-bit LSB executable; Intel 80386; version 1 (SYSV);
   for GNU/Linux 2.2.5; dynamically linked (uses shared libs); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwstest.c,
   1671,1078582732,1078608953,1044457030,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwstest.d,
   42,1078584303,1078608953,1078584303,664,
   ASCII text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwstest.o,
   14876,1078590300,1078608953,1078590300,664,
   ELF 32-bit LSB relocatable; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwsutil.c,
   3191,1078589696,1078608953,1078589696,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwsutil.d,
   42,1078589724,1078608953,1078589724,664,
   ASCII text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwsutil.h,
   1911,1078582732,1078608953,1074173331,0644,
   ASCII C program text),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/cwsutil.o,
   17716,1078589724,1078608953,1078589724,664,
   ELF 32-bit LSB relocatable; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/libcwsutil.so,
   20427,1078589724,1078608953,1078589724,0775,
   ELF 32-bit LSB shared object; Intel 80386; version 1 (SYSV); not stripped),
  FILE_ENTRY(/Simulated/CMPI/tests/Providers/util/makefile,
   895,1078582732,1078609495,1074173331,0644,
   ASCII make commands text),
   {' ',NULL,0,0,0,0,0,NULL}
};



