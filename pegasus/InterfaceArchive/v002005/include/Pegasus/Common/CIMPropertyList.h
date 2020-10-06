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

/**
    The CIMPropertyList class represents a propertyList parameter in a CIM
    operation request, as defined in the DMTF Specification for CIM
    Operations over HTTP.

    <p>This class consists of an array of property names and a flag
    indicating whether the list is null.  A null property list indicates
    that no filtering is performed on the basis of this parameter.  A
    non-null property list indicates that any property not specified in the
    list is to be filtered from the CIM operation response.  (An empty
    property list implies that all properties should be filtered from the
    response.)

    <p>A null property list is created by using the default constructor or the
    clear method.  An empty property list is created by setting the value to
    an empty Array.
*/
class PEGASUS_COMMON_LINKAGE CIMPropertyList
{
public:

    /**
        Constructs a null property list.
        <p><b>Example:</b>
        <pre>
            CIMPropertyList pl;
            assert(pl.isNull());
        </pre>
    */
    CIMPropertyList();

    /**
        Constructs a CIMPropertyList object from the value of a specified
        CIMPropertyList object.
        @param x The CIMPropertyList object from which to construct a new
            CIMPropertyList object.
    */
    CIMPropertyList(const CIMPropertyList& x);

    /**
        Constructs a non-null property list with the specified property names.
        <p><b>Example:</b>
        <pre>
            Array<CIMName> n;
            n.append("name");
            n.append("type");
            CIMPropertyList pl(n);
        </pre>
        @param propertyNames An Array of CIMNames specifying the property
            names in the list.
    */
    CIMPropertyList(const Array<CIMName>& propertyNames);

    /**
        Destructs the CIMPropertyList object.
    */
    ~CIMPropertyList();

    /**
        Sets the property list with the specified property names.  The
        resulting property list is non-null.
        <p><b>Example:</b>
        <pre>
            Array<CIMName> n;
            n.append("name");
            n.append("type");
            CIMPropertyList pl;
            pl.set(n);
            assert(pl.size() = 2);
        </pre>
        @param propertyNames An Array of CIMNames specifying the property
            names in the list.
    */
    void set(const Array<CIMName>& propertyNames);

    /**
        Assigns the value of the specified CIMPropertyList object to this
        object.
        @param x The CIMPropertyList object from which to assign this
            CIMPropertyList object.
        @return A reference to this CIMPropertyList object.
    */
    CIMPropertyList& operator=(const CIMPropertyList& x);

    /**
        Sets the property list to a null value.
    */
    void clear();

    /**
        Determines whether the property list is null.
        @return True if the property list is null, false otherwise.
    */
    Boolean isNull() const;

    /**
        Gets the number of property names in the property list.
        @return An integer count of the property names in the CIMPropertyList.
        A value of 0 is returned if the list is null or empty.
    */
    Uint32 size() const;

    /**
        Gets the property name at a specified index.
        <p><b>Example:</b>
        <pre>
            Array<CIMName> n;
            n.append("name");
            n.append("type");
            CIMPropertyList pl(n);
            assert(pl[0] == CIMName("name"));
        </pre>
        @param index The index of the property name to be retrieved.
        @return A CIMName containing the property name at the specified index.
        @exception IndexOutOfBoundsException If the index is outside the
            range of property names in the property list or if the property
            list is null.
    */
    const CIMName& operator[](Uint32 index) const;

    /**
        Gets an Array of the property names in the property list.
        <p><b>Example:</b>
        <pre>
            Array<CIMName> n = pl.getPropertyNameArray();
        </pre>
        @return An Array of CIMName objects containing the property names
        in the property list.
    */
    Array<CIMName> getPropertyNameArray() const;

private:

    CIMPropertyListRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMPropertyList_h */
