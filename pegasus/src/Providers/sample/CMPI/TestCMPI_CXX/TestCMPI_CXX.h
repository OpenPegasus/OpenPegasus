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

#ifndef TESTCMPI_CXX_H
#define TESTCMPI_CXX_H

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/CmpiArray.h>
#include <Pegasus/Provider/CMPI/CmpiBooleanData.h>
#include <Pegasus/Provider/CMPI/CmpiBroker.h>
#include <Pegasus/Provider/CMPI/CmpiData.h>
#include <Pegasus/Provider/CMPI/CmpiDateTime.h>
#include <Pegasus/Provider/CMPI/CmpiEnumeration.h>
#include <Pegasus/Provider/CMPI/CmpiInstance.h>
#include <Pegasus/Provider/CMPI/CmpiObject.h>
#include <Pegasus/Provider/CMPI/CmpiObjectPath.h>
#include <Pegasus/Provider/CMPI/CmpiString.h>
#include <Pegasus/Provider/CMPI/CmpiStatus.h>

#include <Pegasus/Provider/CMPI/CmpiInstanceMI.h>
#include <Pegasus/Provider/CMPI/CmpiMethodMI.h>

class TestCMPI_CXX : public CmpiInstanceMI
{
 public:

                     TestCMPI_CXX      (const CmpiBroker&     mbp,
                                        const CmpiContext&    ctx);

  virtual           ~TestCMPI_CXX      ();

  virtual CmpiStatus initialize        (const CmpiContext&    ctx);
  virtual int        isUnloadable      () const;

  virtual CmpiStatus enumInstanceNames (const CmpiContext&    ctx,
                                        CmpiResult&           rslt,
                                        const CmpiObjectPath& cop);

  virtual CmpiStatus enumInstances     (const CmpiContext&    ctx,
                                        CmpiResult&           rslt,
                                        const CmpiObjectPath& cop,
                                        const char*          *properties);

  virtual CmpiStatus getInstance       (const CmpiContext&    ctx,
                                        CmpiResult&           rslt,
                                        const CmpiObjectPath& cop,
                                        const char*          *properties);

  virtual CmpiStatus createInstance    (const CmpiContext&    ctx,
                                        CmpiResult&           rslt,
                                        const CmpiObjectPath& cop,
                                        const CmpiInstance&   inst);

  virtual CmpiStatus setInstance       (const CmpiContext&    ctx,
                                        CmpiResult&           rslt,
                                        const CmpiObjectPath& cop,
                                        const CmpiInstance&   inst,
                                        const char*          *properties);

  virtual CmpiStatus deleteInstance    (const CmpiContext&    ctx,
                                        CmpiResult&           rslt,
                                        const CmpiObjectPath& cop);

private:
  CmpiBroker cppBroker;
};

void printInstance (const CmpiInstance& inst);

#endif
