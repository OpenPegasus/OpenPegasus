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

#include "CWS_FileUtils.h"
#include <Providers/sample/CMPI/CWS_Util/cwsutil.h>
#include "CWS_PlainFile.h"
#include <iostream>

#ifndef PEGASUS_PLATFORM_HPUX_PARISC_ACC
using namespace std;
#endif

static const char *  LOCALCLASSNAME="CWS_PlainFile_CXX";

/* -----------------------------------------------------------------------*/
/*      Provider Factory - IMPORTANT for entry point generation           */
/* -----------------------------------------------------------------------*/

CMProviderBase(CWS_PlainFileProvider_CXX);

CMInstanceMIFactory(CWS_PlainFile,CWS_PlainFileProvider_CXX);
CMMethodMIFactory(CWS_PlainFile,CWS_PlainFileProvider_CXX);

/* -----------------------------------------------------------------------*/
/*                          Base Provider Interface                       */
/* -----------------------------------------------------------------------*/

CWS_PlainFile::CWS_PlainFile (const CmpiBroker &mbp,
                  const CmpiContext& ctx)
  : CmpiBaseMI(mbp, ctx), CmpiInstanceMI(mbp,ctx),
    CmpiMethodMI(mbp,ctx), cppBroker(mbp)
{
#ifndef SIMULATED
  cout<<"CWS_PlainFile Provider was constructed"<<endl;
#endif
}

CWS_PlainFile::~CWS_PlainFile()
{
#ifndef SIMULATED
  cout<<"CWS_PlainFile Provider was destructed"<<endl;
#endif
}

int CWS_PlainFile::isUnloadable() const
{
  return 1;  // may be unloaded
}

/* -----------------------------------------------------------------------*/
/*                      Instance Provider Interface                       */
/* -----------------------------------------------------------------------*/


CmpiStatus CWS_PlainFile::enumInstanceNames(const CmpiContext& ctx,
                        CmpiResult& rslt,
                        const CmpiObjectPath& cop)
{
  void           *enumhdl;
  CWS_FILE        filebuf;

#ifndef SIMULATED
  cout<<"CWS_PlainFile enumerating instanceNames"<<endl;
#endif

  CmpiString nameSpace=cop.getNameSpace();

  enumhdl = CWS_Begin_Enum(CWS_FILEROOT,CWS_TYPE_PLAIN);

  if (enumhdl == NULL) {
    throw CmpiStatus(CMPI_RC_ERR_FAILED,
             "Could not begin file enumeration");
  }
  while (CWS_Next_Enum(enumhdl,&filebuf)) {
    /* build object path from file buffer */
    CmpiObjectPath objectPath =
      makePath(LOCALCLASSNAME,nameSpace.charPtr(),&filebuf);
    rslt.returnData(objectPath);
  }
  CWS_End_Enum(enumhdl);
  rslt.returnDone();
  return CmpiStatus(CMPI_RC_OK);
}

CmpiStatus CWS_PlainFile::enumInstances(const CmpiContext& ctx,
                    CmpiResult& rslt,
                    const CmpiObjectPath& cop,
                    const char* *properties)
{
  void           *enumhdl;
  CWS_FILE        filebuf;

#ifndef SIMULATED
  cout<<"CWS_PlainFile enumerating instances"<<endl;
#endif
  CmpiString nameSpace=cop.getNameSpace();

  enumhdl = CWS_Begin_Enum(CWS_FILEROOT,CWS_TYPE_PLAIN);

  if (enumhdl == NULL) {
    throw CmpiStatus(CMPI_RC_ERR_FAILED,
             "Could not begin file enumeration");
  }
  while (CWS_Next_Enum(enumhdl,&filebuf)) {
    /* build instance from file buffer */
    CmpiInstance instance =
      makeInstance(LOCALCLASSNAME,
           nameSpace.charPtr(),
           &filebuf,
           properties);
    rslt.returnData(instance);
  }
  CWS_End_Enum(enumhdl);
  rslt.returnDone();
  return CmpiStatus(CMPI_RC_OK);
}


CmpiStatus CWS_PlainFile::getInstance(const CmpiContext& ctx,
                      CmpiResult& rslt,
                      const CmpiObjectPath& cop,
                      const char* *properties)
{
#ifndef SIMULATED
  cout<<"CWS_PlainFile getting instance"<<endl;
#endif

  CmpiString key = cop.getKey("Name").getString();
  CmpiString nameSpace=cop.getNameSpace();
  CWS_FILE  filebuf;

  if (CWS_Get_File(key.charPtr(),&filebuf)) {
    CmpiInstance instance =
      makeInstance(LOCALCLASSNAME,
           nameSpace.charPtr(),
           &filebuf,
           properties);
    rslt.returnData(instance);
    rslt.returnDone();
    return CmpiStatus(CMPI_RC_OK);
  }
  throw CmpiStatus(CMPI_RC_ERR_NOT_FOUND,key.charPtr());
}

CmpiStatus CWS_PlainFile::setInstance(const CmpiContext& ctx,
                      CmpiResult& rslt,
                      const CmpiObjectPath& cop,
                      const CmpiInstance& inst,
                      const char* *properties)
{
#ifndef SIMULATED
  cout<<"CWS_PlainFile modifying instance"<<endl;
#endif
  CWS_FILE   filebuf;
  CmpiString key = cop.getKey("Name").getString();

  if (!CWS_Get_File(key.charPtr(),&filebuf)) {
    throw CmpiStatus(CMPI_RC_ERR_NOT_FOUND,key.charPtr());
  }

  if (!makeFileBuf(inst,&filebuf) || !CWS_Update_File(&filebuf)) {
    throw CmpiStatus(CMPI_RC_ERR_FAILED,"Could not update instance");
  }
  return CmpiStatus(CMPI_RC_OK);
}

CmpiStatus CWS_PlainFile::createInstance(const CmpiContext& ctx,
                     CmpiResult& rslt,
                     const CmpiObjectPath& cop,
                     const CmpiInstance& inst)
{
  return CmpiStatus(CMPI_RC_ERR_NOT_SUPPORTED,
           "CWS_PlainFile cannot create");
}

CmpiStatus CWS_PlainFile::deleteInstance(const CmpiContext& ctx,
                     CmpiResult& rslt,
                     const CmpiObjectPath& cop)
{
  // we can return or throw - whatever we like
  return CmpiStatus(CMPI_RC_ERR_NOT_SUPPORTED,
            "CWS_PlainFile cannot delete");
}


/* -----------------------------------------------------------------------*/
/*                        Method Provider Interface                       */
/* -----------------------------------------------------------------------*/


CmpiStatus CWS_PlainFile::invokeMethod(const CmpiContext& ctx,
                       CmpiResult& rslt,
                       const CmpiObjectPath& cop,
                       const char * method,
                       const CmpiArgs& in,
                       CmpiArgs& out)
{
#ifndef SIMULATED
  cout<<"CWS_PlainFile invoking method "<<method<<endl;
#endif
  char typebuf[1000];
  CmpiString key=cop.getKey("Name").getString();

  if (strcasecmp("filetype",method)) {
    throw CmpiStatus(CMPI_RC_ERR_METHOD_NOT_FOUND,method);
  }
  if (CWS_Get_FileType(key.charPtr(),typebuf,
               sizeof(typebuf))) {
    throw CmpiStatus(CMPI_RC_ERR_FAILED,
             "Could not get type");
  }

  rslt.returnData(CmpiData (typebuf));
  rslt.returnDone();
  return CmpiStatus(CMPI_RC_OK);
}

