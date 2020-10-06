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
// This code implements part of PEP#348 - The CMPI infrastructure using SCMO
// (Single Chunk Memory Objects).
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _CMPISCMOUtilities_H_
#define _CMPISCMOUtilities_H_

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include "CMPI_Broker.h"

PEGASUS_NAMESPACE_BEGIN


class CMPISCMOUtilities
{
public:
    static CMPIrc scmoValue2CMPIData(
        const SCMBUnion* scmoValue,
        CMPIType type,
        CMPIData *data,
        Uint32 arraySize = 0);

    static CMPIrc scmoValue2CMPIKeyData(
        const SCMBUnion* scmoValue,
        CMPIType type,
        CMPIData *data);

    static CIMDateTimeRep* scmoDateTimeFromCMPI(CMPIDateTime* cmpidt);

    static SCMOInstance* getSCMOFromCIMInstance(
        const CIMInstance&,
        const char* ns=0,
        const char* cls=0);

    static SCMOInstance* getSCMOFromCIMObjectPath(
        const CIMObjectPath&,
        const char* ns=0,
        const char* cls=0);

    static CMPIrc copySCMOKeyProperties(
        const SCMOInstance* sourcePath,
        SCMOInstance* targetPath );
};


PEGASUS_NAMESPACE_END

#endif
