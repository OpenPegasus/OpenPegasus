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

#ifndef Pegasus_AssocInstTable_h
#define Pegasus_AssocInstTable_h

#include <iostream>
#include <fstream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Repository/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** Maintains all associations for a given namesspace.
*/
class PEGASUS_REPOSITORY_LINKAGE AssocInstTable
{
public:

    /** Appends a row into the association table. There is no checking
        for duplicate entries (the caller ensures this). The case of
        the arguments doesn't matter. They are ignored during comparison.
    */
    static void append(
        PEGASUS_STD(ofstream)& os,
        const String& assocInstanceName,
        const CIMName& assocClassName,
        const String& fromInstanceName,
        const CIMName& fromClassName,
        const CIMName& fromPropertyName,
        const String& toInstanceName,
        const CIMName& toClassName,
        const CIMName& toPropertyName);

    /** Appends a row into the association table. There is no checking
        for duplicate entries (the caller ensures this). The case of the
        arguments doesn't matter. Case is ignored during comparison.
    */
    static void append(
        const String& path,
        const String& assocInstanceName,
        const CIMName& assocClassName,
        const String& fromInstanceName,
        const CIMName& fromClassName,
        const CIMName& fromPropertyName,
        const String& toInstanceName,
        const CIMName& toClassName,
        const CIMName& toPropertyName);

    /** Deletes the given association from the table by removing every entry
        with an assocInstanceName equal to the assocInstanceName parameter.
        @returns true if such an association was found.
    */
    static Boolean deleteAssociation(
        const String& path,
        const CIMObjectPath& assocInstanceName);

    /** Finds all associators of the given object. See
        CIMOperations::associators() for a full description.
    */
    static Boolean getAssociatorNames(
        const String& path,
        const CIMObjectPath& objectName,
        const Array<CIMName>& assocClassList,
        const Array<CIMName>& resultClassList,
        const String& role,
        const String& resultRole,
        Array<String>& associatorNames);

    /** Obtain all references (association instance names) in which the
        given object is involved. See CIMOperations::associators() for a
        full description.
    */
    static Boolean getReferenceNames(
        const String& path,
        const CIMObjectPath& objectName,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames);

private:

    AssocInstTable() { /* private */ }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_AssocInstTable_h */
