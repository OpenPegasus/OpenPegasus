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

#ifndef _CMPI_Object_H_
#define _CMPI_Object_H_

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/SCMOClass.h>
#include <Pegasus/Common/SCMOInstance.h>

#include <Pegasus/General/CIMError.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include "CMPI_Array.h"

PEGASUS_NAMESPACE_BEGIN

class CMPI_Object
{
    friend class CMPI_ThreadContext;
    void *hdl;
    void *ftab;
    CMPI_Object *next,*prev;
public:
    void *priv;    // CMPI type specific usage
    void *getHdl()
    {
        return hdl;
    }
    void *getFtab()
    {
        return ftab;
    }

    enum SCMOInstanceObjectType
    {
        ObjectTypeInstance,
        ObjectTypeObjectPath
    };

    CMPI_Object(CIMInstance*);
    CMPI_Object(SCMOInstance*, SCMOInstanceObjectType type);
    CMPI_Object(CIMObjectPath*);
    CMPI_Object(CIMDateTime*);
    CMPI_Object(CIMError*);
    CMPI_Object(OperationContext*);
    CMPI_Object(const String&);
    CMPI_Object(const char*);
    CMPI_Object(const char*, Uint32 len);
    CMPI_Object(Array<CIMParamValue>*);
    CMPI_Object(CMPI_Array*);
    CMPI_Object(CMPI_Object*);
    CMPI_Object(CMPISelectCond*);
    CMPI_Object(CMPISubCond*);
    CMPI_Object(CMPIPredicate*);
    CMPI_Object(struct CMPI_InstEnumeration*);
    CMPI_Object(struct CMPI_ObjEnumeration*);
    CMPI_Object(struct CMPI_OpEnumeration*);

    ~CMPI_Object() {};
    void unlinkAndDelete();
    void unlink();
};

PEGASUS_NAMESPACE_END

#endif
