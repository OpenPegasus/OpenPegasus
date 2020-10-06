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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
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
    the list is null. There are three possibilities which must be represented by
    the CIMPropertyList object because the CIMOperations that use CIMPropertyList
    define functional differences based on these three conditions.
    The property list is:

    <ul>
    <li>Non-empty (and non-null) - Operations where some properties are to be
    returned by the operation. The values in the list are valid property names.
    <li>Empty (and non-null)- Operations where NO properties are to be
    returned by the operation. The propertyList is empty but not Null.
    <li>Null - Operations where there is no propertyList filter. The list
.       is Null (a specific attributed of the list).  There are, of course,
        no properties in the list.
    </ul>

    To create a null property list use the default constructor or use the
    clear() method.

    To create an empty property list use the constructor which takes
    a property array (pass an empty property array which produces an empty
    but not Null property list object).

    Methods are provided for accessing elements of the the internal property
    list. There are none for modifying elements (the entire array must be
    formed and passed to the constructor or replaced by calling set()).
*/
class PEGASUS_COMMON_LINKAGE CIMPropertyList
{
public:

    /** Default constructor (sets isNull attribute of the list to true).
        An array created with this constructor is Null.
        <pre>
            CIMPropertyList pl;
            assert(pl.isNull);
        </pre>
    */
    CIMPropertyList();

    /** Copies the property list to the value specified for the parameter x.
        @param x Specifies the name of the CIMPropertyList object to be copied.
    */
    CIMPropertyList(const CIMPropertyList& x);

    /** Constructor that initializes propertyNames and creates an array with
        non-null values (sets isNull to false).
        @param Array of CIMNames with which the propertyList object is
        initialized. For example:
        <pre>
            Array<CIMName> n;
            n.append("name");
            n.append("type");
            CIMPropertyList pl(n);
        </pre>
    */
    CIMPropertyList(const Array<CIMName>& propertyNames);

    /** CIMPropertyList destructor.
    */
    ~CIMPropertyList();

    /** Modifier for propertyNames (sets isNull to false) and
        sets the CIMName values in the input array into the
        propertyList object.
        @param Array of CIMNames. For example:
        <pre>
            Array<CIMName> n;
            n.append("name");
            n.append("type");
            CIMPropertyList pl;
            pl.set(n);
            assert pl.size() = 2);
        </pre>
    */
    void set(const Array<CIMName>& propertyNames);

    /** Assigns the values of the CIMPropertyList instance to the 
        CIMPropertyList.
        @param x Specifies the name of the CIMPropertyList instance 
        whose values are to be assigned to the CIMPropertyList object.
    */
    CIMPropertyList& operator=(const CIMPropertyList& x);

    /** Clears the propertyNames array (sets isNull to true).
    */
    void clear();

    /** Returns true if the property list is null.
        @return A Boolean value of true if the property list is Null.
        It may be null because it was created without input or because it
        was set to Null with the clear() method. Otherwise, a value of
        false is returned.  For example:
        <pre>
            CIMPropertyList pl;
            assert(pl.isNull());    // Newly created object is Null
        </pre>
        Therefore the a new instance of the CIMPropertyList object is
        created, pl, with null values.
    */
    Boolean isNull() const;

    /** Returns the number of propertyNames in the list.
        @return Uint32 with count of number of properties in the
        list. Returns 0 if property list is Null but this is not
        sufficient to determine if it is Null. Use isNull to determine
        if it is Null.
    */
    Uint32 size() const;

    /** Return the property at the given index.
        @param index Specifies the index value that contains the property
        list to retrieve.
        @return CIMName at the defined location.
        @exception out_of_index exception if the index is
        outside of the size of the propertyList. For example:
    <pre>
        Array<CIMName> n;
        n.append("name");
        n.append("type");
        CIMPropertyList pl;
        assert(pl[0] == CIMName("name"));
    </pre>
    */
    const CIMName& operator[](Uint32 index) const;

    /** Get an array of the property names.
        @return Array of CIMName containing the property names
        from the propertyList object.
        <pre>
            Array<CIMName> n = pl.getPropertyNameArray();
        </pre>
    */
    Array<CIMName> getPropertyNameArray() const;

private:

    CIMPropertyListRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMPropertyList_h */
