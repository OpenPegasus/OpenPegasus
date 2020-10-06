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
#include "CWS_DirectoryContainsFile.h"
#include <string.h>
#if !defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
#include <libgen.h>
#endif

static const char* LOCALCLASSNAME="CWS_DirectoryContainsFile_CXX";
static const char* DIRECTORYCLASS="CWS_Directory_CXX";
static const char* FILECLASS="CWS_PlainFile_CXX";

// helper function

enum FuncType{
  FUNC_ASSOC,
  FUNC_ASSOCNAME,
  FUNC_REF,
  FUNC_REFNAME
};

static void DirectoryContainsFileReturnHelper
(CmpiResult& rslt,
 const CmpiObjectPath& cop,
 const CWS_FILE* filebuf,
 int isparent,
 const char* filetype,
 int functype);

static CmpiStatus CWS_DirectoryContainsFileAssocHelper
(CmpiResult& rslt,
 const CmpiObjectPath& op,
 const char* asscClass, const char* resultClass,
 const char* role, const char* resultRole, const char** properties,
 int functype);

#ifndef PEGASUS_PLATFORM_HPUX_PARISC_ACC
using namespace std;
#endif

/* -----------------------------------------------------------------------*/
/*      Provider Factory - IMPORTANT for entry point generation           */
/* -----------------------------------------------------------------------*/

CMProviderBase(CWS_DirectoryContainsFileProvider_CXX);

CMAssociationMIFactory(
    CWS_DirectoryContainsFile,
    CWS_DirectoryContainsFileProvider_CXX);

/* -----------------------------------------------------------------------*/
/*                          Base Provider Interface                       */
/* -----------------------------------------------------------------------*/

CWS_DirectoryContainsFile::CWS_DirectoryContainsFile (const CmpiBroker &mbp,
                              const CmpiContext& ctx)
  : CmpiBaseMI(mbp, ctx), CmpiAssociationMI(mbp,ctx),
    cppBroker(mbp)
{
#ifndef SIMULATED
  cout<<"CWS_DirectoryContainsFile Provider was constructed"<<endl;
#endif
}

CWS_DirectoryContainsFile::~CWS_DirectoryContainsFile()
{
#ifndef SIMULATED
  cout<<"CWS_DirectoryContainsFile Provider was destructed"<<endl;
#endif
}

int CWS_DirectoryContainsFile::isUnloadable() const
{
  return 1;  // may be unloaded
}

/* -----------------------------------------------------------------------*/
/*                      Instance Provider Interface                       */
/* -----------------------------------------------------------------------*/

CmpiStatus CWS_DirectoryContainsFile::associators
(const CmpiContext& ctx, CmpiResult& rslt,
 const CmpiObjectPath& op, const char* assocClass, const char* resultClass,
 const char* role, const char* resultRole, const char** properties)
{
  return CWS_DirectoryContainsFileAssocHelper(rslt,op,assocClass,
                          resultClass,role,resultRole,
                          properties,FUNC_ASSOC);
}

CmpiStatus CWS_DirectoryContainsFile::associatorNames
(const CmpiContext& ctx, CmpiResult& rslt,
 const CmpiObjectPath& op, const char* assocClass, const char* resultClass,
 const char* role, const char* resultRole)
{
  return CWS_DirectoryContainsFileAssocHelper(rslt,op,assocClass,
                          resultClass,role,resultRole,
                          0,FUNC_ASSOCNAME);
}

CmpiStatus CWS_DirectoryContainsFile::references
(const CmpiContext& ctx, CmpiResult& rslt,
 const CmpiObjectPath& op, const char* resultClass, const char* role ,
 const char** properties)
{
  return CWS_DirectoryContainsFileAssocHelper(rslt,op,0,
                          resultClass,role,0,
                          properties,FUNC_REF);
}

CmpiStatus CWS_DirectoryContainsFile::referenceNames
(const CmpiContext& ctx, CmpiResult& rslt,
 const CmpiObjectPath& op, const char* resultClass, const char* role)
{
  return CWS_DirectoryContainsFileAssocHelper(rslt,op,0,
                          resultClass,role,0,
                          0,FUNC_REFNAME);
}

/* ------------------------------------------------------------------ *
 * Association MI Helper Functions
 * ------------------------------------------------------------------ */

static void DirectoryContainsFileReturnHelper
(CmpiResult& rslt,
 const CmpiObjectPath& copP,
 const CWS_FILE* filebuf,
 int isparent,
 const char* filetype,
 int functype)
{
  CmpiObjectPath cop(copP);
  CmpiObjectPath assocop = makePath(filetype,
                    cop.getNameSpace().charPtr(),
                    filebuf);
#ifdef SIMULATED
    // We have to set the hostname to 'localhost' otherwise the automated
    // test-cases blow up.
      cop.setHostname(CSName());
      assocop.setHostname(CSName());
#endif
  switch (functype) {
  case FUNC_ASSOC:
    rslt.returnData(makeInstance(filetype,
                 cop.getNameSpace().charPtr(),
                 filebuf,0));
    break;
  case FUNC_ASSOCNAME:
    rslt.returnData(assocop);
    break;
  case FUNC_REF:
    {
      CmpiObjectPath refop(cop.getNameSpace().charPtr(),
               LOCALCLASSNAME);
#ifdef SIMULATED
      refop.setHostname(CSName());
#endif
      CmpiInstance refinst(refop);
      refinst.setProperty(
          "GroupComponent",
          isparent?CmpiData (cop):CmpiData (assocop));
      refinst.setProperty(
          "PartComponent",
          isparent?CmpiData (assocop):CmpiData (cop));
      rslt.returnData(refinst);
    }
    break;
  case FUNC_REFNAME:
    {
      CmpiObjectPath refop(cop.getNameSpace().charPtr(),
               LOCALCLASSNAME);
#ifdef SIMULATED
      refop.setHostname(CSName());
#endif
      refop.setKey("GroupComponent",isparent?CmpiData (cop):CmpiData (assocop));
      refop.setKey("PartComponent",isparent?CmpiData (assocop):CmpiData (cop));
      rslt.returnData(refop);
    }
    break;
  }
}

static CmpiStatus CWS_DirectoryContainsFileAssocHelper
(CmpiResult& rslt,
 const CmpiObjectPath& cop,
 const char* assocClass, const char* resultClass,
 const char* role, const char* resultRole, const char** properties,
 int functype)
{
#ifndef SIMULATED
  cout << "CWS_DirectoryContainsFile Associator Helper" << endl;
#endif
  void * enumhdl;
  CWS_FILE filebuf;

  //  Check if the object path belongs to a supported class
  if (cop.classPathIsA(DIRECTORYCLASS)) {
    // we have a directory and can return the children
    CmpiString key = cop.getKey("Name").getString();

    // first the plain files
    enumhdl = CWS_Begin_Enum(key.charPtr(),CWS_TYPE_PLAIN);
    if (enumhdl == NULL) {
      throw CmpiStatus( CMPI_RC_ERR_FAILED,
            "Could not begin file enumeration");
    }
    while (CWS_Next_Enum(enumhdl,&filebuf)) {
      DirectoryContainsFileReturnHelper(rslt,cop,&filebuf,
                    true,FILECLASS,functype);
    }
    CWS_End_Enum(enumhdl);

    // then the directories
    enumhdl = CWS_Begin_Enum(key.charPtr(),CWS_TYPE_DIR);
    if (enumhdl == NULL) {
      throw CmpiStatus( CMPI_RC_ERR_FAILED,
            "Could not begin directory enumeration");
    }
    while (CWS_Next_Enum(enumhdl,&filebuf)) {
      DirectoryContainsFileReturnHelper(rslt,cop,&filebuf,
                    true,DIRECTORYCLASS,functype);
    }
    CWS_End_Enum(enumhdl);

  }

  if (cop.classPathIsA(FILECLASS) ||
      cop.classPathIsA(DIRECTORYCLASS)) {
    // we can always return the parent
    CmpiString key = cop.getKey("Name").getString();

    if (CWS_Get_File(dirname((char*)key.charPtr()),&filebuf)) {
     DirectoryContainsFileReturnHelper(rslt,cop,&filebuf,
                    false,DIRECTORYCLASS,functype);
    }
  }
  rslt.returnDone();
  return CmpiStatus(CMPI_RC_OK);
}

