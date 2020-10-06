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

// Please be aware that the CMPI C++ API is NOT a standard currently.

#include <Pegasus/Provider/CMPI/CmpiBooleanData.h>
#include "CWS_FileUtils.h"
#include <Providers/sample/CMPI/CWS_Util/cwsutil.h>

#if !defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
#include <unistd.h>
#endif

#include <string.h>
#include <stdlib.h>


static char cscn[] = "CIM_UnitaryComputerSystem";
static char fscn[] = "CIM_FileSystem";
static char csn[500] = "";
static char fsn[] = CWS_FILEROOT;

char * CSCreationClassName()
{
  return cscn;
}

char * CSName()
{
  if (*csn == 0)
#ifdef SIMULATED
    strcpy(csn,"localhost");
#else
    gethostname(csn,sizeof(csn));
    csn[sizeof(csn)-1] = 0;
#endif
  return csn;
}

char * FSCreationClassName()
{
  return fscn;
}

char * FSName()
{
  return fsn;
}


CmpiObjectPath makePath(const char * classname,
            const char * nameSpace, const CWS_FILE *cwsf)
{
  CmpiObjectPath op(nameSpace,classname);
#ifdef SIMULATED
  op.setHostname(CSName());
#endif
  op.setKey("CSCreationClassName",CmpiData (CSCreationClassName()));
  op.setKey("CSName",CmpiData (CSName()));
  op.setKey("FSCreationClassName",CmpiData (FSCreationClassName()));
  op.setKey("FSName",CmpiData (FSName()));
  op.setKey("CreationClassName",CmpiData (classname));
  op.setKey("Name",CmpiData (cwsf->cws_name));
  return op;
}

CmpiInstance makeInstance(const char * classname,
              const char * nameSpace, const CWS_FILE *cwsf,
              const char ** filter)
{
  CmpiObjectPath op(nameSpace,classname);
  CmpiInstance in(op);

  if (filter) {
    static const char * filterKeys[] = {
      "CSCreationClassName",
      "CSName",
      "FSCreationClassName",
      "FSName",
      "CreationClassName",
      "Name",
      NULL
    };
    in.setPropertyFilter(filter,filterKeys);
  }

  in.setProperty("CSCreationClassName",CmpiData (CSCreationClassName()));
  in.setProperty("CSName",CmpiData (CSName()));
  in.setProperty("FSCreationClassName",CmpiData (FSCreationClassName()));
  in.setProperty("FSName",CmpiData (FSName()));
  in.setProperty("CreationClassName",CmpiData (classname));
  in.setProperty("Name",CmpiData (cwsf->cws_name));
  in.setProperty("FileSize",CmpiData ((CMPIUint64)cwsf->cws_size));
#ifndef SIMULATED
/* We don't want this code in the simulated env - time is dynamic
   (diff timezones)
 * and the testing system might using a diff timezone and report failure */
  in.setProperty("CreationDate",CmpiDateTime(cwsf->cws_ctime*1000000LL,0));
  in.setProperty("LastModified",CmpiDateTime(cwsf->cws_mtime*1000000LL,0));
  in.setProperty("LastAccessed",CmpiDateTime(cwsf->cws_atime*1000000LL,0));
#endif
  in.setProperty("Readable",CmpiBooleanData(cwsf->cws_mode & 0400?1:0));
  in.setProperty("Writeable",CmpiBooleanData(cwsf->cws_mode & 0200?1:0));
  in.setProperty("Executable",CmpiBooleanData(cwsf->cws_mode & 0100?1:0));
  return in;
}

int makeFileBuf(const CmpiInstance &instance, CWS_FILE *cwsf)
{
  if (cwsf) {
    strcpy(cwsf->cws_name, instance.getProperty("Name").getString().charPtr());
    CmpiData data=instance.getProperty("FileSize");
    if (!data.isNullValue())
      cwsf->cws_size = data.getUint64();
    data=instance.getProperty("Readable");
    if (!data.isNullValue())
      cwsf->cws_mode=data.getBoolean() ? 0400 : 0;
    data=instance.getProperty("Writeable");
    if (!data.isNullValue())
      cwsf->cws_mode+=data.getBoolean() ? 0200 : 0;
    data=instance.getProperty("Executable");
    if (!data.isNullValue())
      cwsf->cws_mode+=data.getBoolean() ? 0100 : 0;
    return 1;
  }
  return 0;
}

#ifdef NAGNAG

char **projection2Filter(CmpiArray *ar)
{
  CmpiStatus rc = {Cmpi_RC_OK,NULL};
  char     **filter;
  int        i;
  int        count=0;

  if (ar)
    count = CMGetArrayCount(ar,NULL);
  if (count==0) return NULL;
  filter = calloc(count+1,sizeof(char*));
  for (i=0; i<count;i++) {
    CmpiData dt = CMGetArrayElementAt(ar,i,&rc);
    if (rc.rc != Cmpi_RC_OK) {
      freeFilter(filter);
      filter=NULL;
      break;
    }
    filter[i] = CMGetCharsPtr(dt.value.string,NULL);
  }
  return filter;
}

void freeFilter(char **filter)
{
  if (filter) free(filter);
}

#endif
