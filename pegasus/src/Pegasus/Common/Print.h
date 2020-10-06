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

#ifndef Pegasus_Print_h
#define Pegasus_Print_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/Linkage.h>
#include <iostream>

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// PrintValue()
// PrintProperty()
// PrintObjectPath()
// PrintInstance()
// PrintQualifierDecl()
// PrintQualifier()
//
//     These functions print selected CIM objects in a format suitable for
//     debugging. These functions print every element of the object. Some
//     more conventional formats exclude some elements. For example, MOF
//     instances do not include the property-type and XML instances exclude
//     the host and namespace.
//
//     You may use these functions for diffing two objects using the following
//     procedure. (1) Print each object to a file. (2) Diff the files with the
//     diff command.
//     WARNING: These functions are a diagnostic tool and generate output ONLY
//     in PEGASUS_DEBUG mode. When pegasus is compiled withPEGASUS_DEBUG set
//     false, empty functions are built.
//
//==============================================================================

PEGASUS_COMMON_LINKAGE void PrintValue(
    PEGASUS_STD(ostream)& os,
    const CIMValue& x,
    Uint32 n = 0);

PEGASUS_COMMON_LINKAGE void PrintProperty(
    PEGASUS_STD(ostream)& os,
    const CIMConstProperty& x,
    Uint32 n = 0);

PEGASUS_COMMON_LINKAGE void PrintObjectPath(
    PEGASUS_STD(ostream)& os,
    const CIMObjectPath& x,
    Uint32 n = 0);

PEGASUS_COMMON_LINKAGE void PrintInstance(
    PEGASUS_STD(ostream)& os,
    const CIMConstInstance& cimInstance,
    Uint32 n = 0);

PEGASUS_COMMON_LINKAGE void PrintQualifierDecl(
    PEGASUS_STD(ostream)& os,
    const CIMConstQualifierDecl& x,
    Uint32 n = 0);

PEGASUS_COMMON_LINKAGE void PrintParamValue(
    PEGASUS_STD(ostream)& os,
    const CIMParamValue& x,
    Uint32 n = 0);

PEGASUS_COMMON_LINKAGE void PrintParamValueArray(
    PEGASUS_STD(ostream)& os,
    const Array<CIMParamValue>& x,
    Uint32 n = 0);

PEGASUS_COMMON_LINKAGE void PrintQualifier(
    PEGASUS_STD(ostream)& os,
    const CIMConstQualifier& x,
    Uint32 n = 0);
PEGASUS_NAMESPACE_END

#endif /* Pegasus_Print_h */
