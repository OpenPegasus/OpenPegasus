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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Name_h
#define Pegasus_Name_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMName
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMName class defines methods for handling CIM names.
    <p>
    The names of classes, properties, qualifiers, and methods are all
    CIM names. A CIM name must match the following regular
    expression:
    <PRE>

	[A-Z-a-z_][A-Za-z_0-9]*
    </PRE>
*/
class PEGASUS_COMMON_LINKAGE CIMName
{
public:

    ///
    CIMName();
    ///
    CIMName(const String& name);
    ///
    CIMName(const char* name);

    ///
    CIMName& operator=(const CIMName& name);
    ///
    CIMName& operator=(const String& name);


    ///
    const String& getString() const;


    ///
    Boolean isNull() const;

    ///
    void clear();

    /** Compares two names.
	@return true if the name passed is equal to the name in this
        class. CIM names are case insensitive and so is this method.
    */
    Boolean equal(const CIMName& name) const;

    /** Determines if the name string input is legal as
	defnined in the CIMName class definition.
	@param name String to test for legality.
	@return true if the given name is legal, false otherwise.
    */
    static Boolean legal(const String& name);

private:
    String cimName;
};

PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMName& name1,
    const CIMName& name2);

#define PEGASUS_ARRAY_T CIMName
# include "ArrayInter.h"
#undef PEGASUS_ARRAY_T


////////////////////////////////////////////////////////////////////////////////
//
// CIMNamespaceName
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMNamespaceName class defines methods for handling CIM namespace names.
    <p>
    A CIM namespace name must match the following expression:
    <PRE>
        &ltCIMName&gt[/&ltCIMName&gt]...
    </PRE>
    </p>
*/
class PEGASUS_COMMON_LINKAGE CIMNamespaceName
{
public:

    ///
    CIMNamespaceName();
    ///
    CIMNamespaceName(const String& name);
    ///
    CIMNamespaceName(const char* name);

    ///
    CIMNamespaceName& operator=(const CIMNamespaceName& name);
    ///
    CIMNamespaceName& operator=(const String& name);


    ///
    const String& getString() const;


    ///
    Boolean isNull() const;

    ///
    void clear();

    /** Compares two names.
	@return true if the name passed is equal to the name in this
        class. CIM names are case insensitive and so is this method.
    */
    Boolean equal(const CIMNamespaceName& name) const;

    /** Determines if the name string input is legal as
	defnined in the CIMNamespaceName class definition.
	@param name String to test for legality.
	@return true if the given name is legal, false otherwise.
    */
    static Boolean legal(const String& name);

private:
    String cimNamespaceName;
};

PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMNamespaceName& name1,
    const CIMNamespaceName& name2);

#define PEGASUS_ARRAY_T CIMNamespaceName
# include "ArrayInter.h"
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Name_h */
