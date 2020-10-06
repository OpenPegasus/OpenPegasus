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

#include <stdio.h>
#include <string.h>
#include "CWS_FileUtils.h"
#include "../CWS_Util/cwsutil.h"
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#define LOCALCLASSNAME "CWS_Directory"

static const CMPIBroker * _broker;
/* ------------------------------------------------------------------ *
 * Instance MI Cleanup
 * ------------------------------------------------------------------ */

CMPIStatus CWS_DirectoryCleanup( CMPIInstanceMI * mi,
                 const CMPIContext * ctx, CMPIBoolean term)
{
  CMReturn(CMPI_RC_OK);
}

/* ------------------------------------------------------------------ *
 * Instance MI Functions
 * ------------------------------------------------------------------ */

CMPIStatus CWS_DirectoryEnumInstanceNames( CMPIInstanceMI * mi,
                       const CMPIContext * ctx,
                       const CMPIResult * rslt,
                       const CMPIObjectPath * ref)

{
   CMPIObjectPath *op;
  CMPIStatus      st = {CMPI_RC_OK,NULL};
  void           *enumhdl;
  CWS_FILE        filebuf;

  if (!silentMode()) fprintf(stderr,"--- CWS_DirectoryEnumInstanceNames() \n");

  enumhdl = CWS_Begin_Enum(CWS_FILEROOT,CWS_TYPE_DIR);

  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
             "Could not begin file enumeration");
    return st;
  } else {
    while (CWS_Next_Enum(enumhdl,&filebuf)) {
      /* build object path from file buffer */
      op = makePath(_broker,
            LOCALCLASSNAME,
            CMGetCharsPtr(CMGetNameSpace(ref,NULL), NULL),
            &filebuf);
      if (CMIsNullObject(op)) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                 "Could not construct object path");
    break;
      }
      CMReturnObjectPath(rslt,op);
    }
    CMReturnDone(rslt);
    CWS_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus CWS_DirectoryEnumInstances( CMPIInstanceMI * mi,
                       const CMPIContext * ctx,
                       const CMPIResult * rslt,
                       const CMPIObjectPath * ref,
                       const char ** properties)
{
  CMPIInstance   *in;
  CMPIStatus      st = {CMPI_RC_OK,NULL};
  void           *enumhdl;
  CWS_FILE        filebuf;

  if (!silentMode()) fprintf(stderr,"--- CWS_DirectoryEnumInstances() \n");

  enumhdl = CWS_Begin_Enum(CWS_FILEROOT,CWS_TYPE_DIR);

  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
             "Could not begin file enumeration");
    return st;
  } else {
    while (CWS_Next_Enum(enumhdl,&filebuf)) {
      /* build instance from file buffer */
      in = makeInstance(_broker,
            LOCALCLASSNAME,
            CMGetCharsPtr(CMGetNameSpace(ref,NULL), NULL),
            &filebuf);
      if (CMIsNullObject(in)) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                 "Could not construct instance");
    break;
      }
      CMReturnInstance(rslt,in);
    }
    CMReturnDone(rslt);
    CWS_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus CWS_DirectoryGetInstance( CMPIInstanceMI * mi,
                     const CMPIContext * ctx,
                     const CMPIResult * rslt,
                     const CMPIObjectPath * cop,
                     const char ** properties)
{
  CMPIInstance *in = NULL;
  CMPIStatus    st = {CMPI_RC_OK,NULL};
  CMPIData      nd = CMGetKey(cop,"Name",&st);
  CWS_FILE      filebuf;

  if (!silentMode()) fprintf(stderr,"--- CWS_DirectoryGetInstance() \n");

  if (st.rc == CMPI_RC_OK &&
      nd.type == CMPI_string &&
      CWS_Get_File(CMGetCharsPtr(nd.value.string, NULL),&filebuf))
    in = makeInstance(_broker,
              LOCALCLASSNAME,
              CMGetCharsPtr(CMGetNameSpace(cop,NULL), NULL),
              &filebuf);

  if (CMIsNullObject(in)) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
             "Could not find or construct instance");
  } else {
    CMReturnInstance(rslt,in);
    CMReturnDone(rslt);
  }

  return st;
}

CMPIStatus CWS_DirectoryCreateInstance( CMPIInstanceMI * mi,
                    const CMPIContext * ctx,
                    const CMPIResult * rslt,
                    const CMPIObjectPath * cop,
                    const CMPIInstance * ci)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CWS_FILE   filebuf;
  CMPIData   data;

  if (!silentMode()) fprintf(stderr,"--- CWS_DirectoryCreateInstance() \n");

  data = CMGetProperty(ci,"Name",NULL);
  if (!CMGetCharsPtr(data.value.string, NULL) ||
      strncmp(CMGetCharsPtr(data.value.string, NULL),CWS_FILEROOT,
          strlen(CWS_FILEROOT))) {
    CMSetStatusWithChars(_broker,&st,CMPI_RC_ERR_FAILED,
             "Invalid path name");
  } else if (!makeFileBuf(ci,&filebuf) || !CWS_Create_Directory(&filebuf)) {
    CMSetStatusWithChars(_broker,&st,CMPI_RC_ERR_FAILED,
             "Could not create instance");
  }

  return st;
}
CMPIStatus CWS_DirectoryModifyInstance( CMPIInstanceMI * mi,
                     const CMPIContext * ctx,
                     const CMPIResult * rslt,
                     const CMPIObjectPath * cop,
                     const CMPIInstance * ci,
                     const char **properties)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CWS_FILE   filebuf;

  if (!silentMode()) fprintf(stderr,"--- CWS_DirectorySetInstance() \n");

  if (!makeFileBuf(ci,&filebuf) || !CWS_Update_File(&filebuf))
    CMSetStatusWithChars(_broker,&st,CMPI_RC_ERR_FAILED,
             "Could not update instance");

  return st;
}

CMPIStatus CWS_DirectoryDeleteInstance( CMPIInstanceMI * mi,
                    const CMPIContext * ctx,
                    const CMPIResult * rslt,
                    const CMPIObjectPath * cop)
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus CWS_DirectoryExecQuery( CMPIInstanceMI * mi,
                   const CMPIContext * ctx,
                   const CMPIResult * rslt,
                   const CMPIObjectPath * cop,
                   const char * lang,
                   const char * query)
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* ------------------------------------------------------------------ *
 * Instance MI Factory
 *
 * NOTE: This is an example using the convenience macros. This is OK
 *       as long as the MI has no special requirements, i.e. to store
 *       data between calls.
 * ------------------------------------------------------------------ */

CMInstanceMIStub( CWS_Directory,
          CWS_DirectoryProvider,
          _broker,
          CMNoHook);
