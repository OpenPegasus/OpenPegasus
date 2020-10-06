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

#ifndef Pegasus_CIMPropertyList_h
#define Pegasus_CIMPropertyList_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class CIMPropertyListRep;

/** The CIMPropertyList class is used to represent a list of CIM
    properties in Pegasus.

    This class comprises an array of propertyNames and a flag indicating whether
    the list is null. There are three possibilities which must be represented;
    the property list is:

    <ul>
	<li>Non-empty (and non-null)</li>
	<li>Empty (and non-null)</li>
	<li>Null</li>
    </ul>

    To create a null property list use the default constructor. Otherwise 
    use the constructor which takes a property array (pass an empty property
    array to get an empty property list object).

    Methods are provided for accessing elements of the the internal property
    list. There are none for modifying elements (the entire array must be
    formed and passed to the constructor or replaced by calling set()).
*/
class PEGASUS_COMMON_LINKAGE CIMPropertyList
{
public:

    /** Default constructor (sets isNull to true).
    */
    CIMPropertyList();

    /** Copy constructor.
    */
    CIMPropertyList(const CIMPropertyList& x);

    /** Constructor. Initializes propertyNames (sets isNull to false).
    */
    CIMPropertyList(const Array<CIMName>& propertyNames);

    /** Destructor
    */
    ~CIMPropertyList();

    /** Modifier for propertyNames (sets isNull to false).
    */
    void set(const Array<CIMName>& propertyNames);

    /** Assignment operator.
    */
    CIMPropertyList& operator=(const CIMPropertyList& x);

    /** Clears the propertyNames array (sets isNull to true).
    */
    void clear();

    /** Returns true if the property list is null.
    */
    Boolean isNull() const;

    /** Get the number of propertyNames in the list.
    */
    Uint32 size() const;

    /** Get the property at the given index.
    */
    const CIMName& operator[](Uint32 index) const;

    /** Get an array of the property names
    */
    Array<CIMName> getPropertyNameArray() const;

private:

    CIMPropertyListRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMPropertyList_h */
