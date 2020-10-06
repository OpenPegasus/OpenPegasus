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
#include "CWS_Directory.h"
#include <iostream>

#ifndef PEGASUS_PLATFORM_HPUX_PARISC_ACC
using namespace std;
#endif

static const char *  LOCALCLASSNAME="CWS_Directory_CXX";

/* -----------------------------------------------------------------------*/
/*      Provider Factory - IMPORTANT for entry point generation           */
/* -----------------------------------------------------------------------*/

CMProviderBase(CWS_DirectoryProvider_CXX);

CMInstanceMIFactory(CWS_Directory,CWS_DirectoryProvider_CXX);

/* -----------------------------------------------------------------------*/
/*                          Base Provider Interface                       */
/* -----------------------------------------------------------------------*/

CWS_Directory::CWS_Directory (const CmpiBroker &mbp,
                  const CmpiContext& ctx)
  : CmpiBaseMI(mbp, ctx), CmpiInstanceMI(mbp,ctx),
    cppBroker(mbp)
{
#ifndef SIMULATED
  cout<<"CWS_Directory Provider was constructed"<<endl;
#endif
}

CWS_Directory::~CWS_Directory()
{
#ifndef SIMULATED
  cout<<"CWS_Directory Provider was destructed"<<endl;
#endif
}

int CWS_Directory::isUnloadable() const
{
  return 1;  // may be unloaded
}

/* -----------------------------------------------------------------------*/
/*                      Instance Provider Interface                       */
/* -----------------------------------------------------------------------*/


CmpiStatus CWS_Directory::enumInstanceNames(const CmpiContext& ctx,
                        CmpiResult& rslt,
                        const CmpiObjectPath& cop)
{
  void           *enumhdl;
  CWS_FILE        filebuf;

#ifndef SIMULATED
  cout<<"CWS_Directory enumerating instanceNames"<<endl;
#endif

  CmpiString nameSpace=cop.getNameSpace();

  enumhdl = CWS_Begin_Enum(CWS_FILEROOT,CWS_TYPE_DIR);

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

CmpiStatus CWS_Directory::enumInstances(const CmpiContext& ctx,
                    CmpiResult& rslt,
                    const CmpiObjectPath& cop,
                    const char* *properties)
{
  void           *enumhdl;
  CWS_FILE        filebuf;

#ifndef SIMULATED
  cout<<"CWS_Directory enumerating instances"<<endl;
#endif
  CmpiString nameSpace=cop.getNameSpace();

  enumhdl = CWS_Begin_Enum(CWS_FILEROOT,CWS_TYPE_DIR);

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


CmpiStatus CWS_Directory::getInstance(const CmpiContext& ctx,
                       CmpiResult& rslt,
                       const CmpiObjectPath& cop,
                       const char* *properties)
{
#ifndef SIMULATED
  cout<<"CWS_Directory getting instance"<<endl;
#endif

  CmpiString key = cop.getKey("Name").getString ();
  CmpiString nameSpace = cop.getNameSpace();
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

CmpiStatus CWS_Directory::setInstance(const CmpiContext& ctx,
                      CmpiResult& rslt,
                      const CmpiObjectPath& cop,
                      const CmpiInstance& inst,
                      const char* *properties)
{
#ifndef SIMULATED
  cout<<"CWS_Directory modifying instance"<<endl;
#endif
  CWS_FILE   filebuf;
  CmpiString key = cop.getKey("Name").getString ();

  if (!CWS_Get_File(key.charPtr(),&filebuf)) {
    throw CmpiStatus(CMPI_RC_ERR_NOT_FOUND,key.charPtr());
  }

  if (!makeFileBuf(inst,&filebuf) || !CWS_Update_File(&filebuf)) {
    throw CmpiStatus(CMPI_RC_ERR_FAILED,
             "Could not modify instance");
  }
  return CmpiStatus(CMPI_RC_OK);
}

CmpiStatus CWS_Directory::createInstance(const CmpiContext& ctx,
                     CmpiResult& rslt,
                     const CmpiObjectPath& cop,
                     const CmpiInstance& inst)
{
  CWS_FILE   filebuf;
  CmpiString key;

#ifndef SIMULATED
  cout<<"CWS_Directory creating instance"<<endl;
#endif
  key = inst.getProperty("Name").getString ();
  if (strncmp(key.charPtr(),CWS_FILEROOT, strlen(CWS_FILEROOT))) {
    throw CmpiStatus(CMPI_RC_ERR_FAILED,
             "Invalid path name");
  }
  if (!makeFileBuf(inst,&filebuf) || !CWS_Create_Directory(&filebuf)) {
    throw CmpiStatus(CMPI_RC_ERR_FAILED,
             "Could not create instance");
  }
  CmpiObjectPath op= makePath(LOCALCLASSNAME,
                  cop.getNameSpace().charPtr(),
                  &filebuf);
  rslt.returnData(op);
  rslt.returnDone();
  return CmpiStatus(CMPI_RC_OK);
}

CmpiStatus CWS_Directory::deleteInstance(const CmpiContext& ctx,
                          CmpiResult& rslt,
                          const CmpiObjectPath& cop)
{
  // we can return or throw - whatever we like
  return CmpiStatus(CMPI_RC_ERR_NOT_SUPPORTED,
            "CWS_Directory cannot delete");
}
