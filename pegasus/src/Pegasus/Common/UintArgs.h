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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_UintArgs_h
#define Pegasus_UintArgs_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

/************************************************************
 The classes in this file provide a means to pass Nullable Uint32
 and Uin64 arguments  APIs and to manage them in the Pegasus client and server.
 Each class contains state (NULL) and value.

 These classes were created specifically to support API extensions for
 the DMTF defined client Pull operations (See DSP0200, version 1.3.1)

 Each class defines the functions for a singleCIMType.  Today the possible
 type are Uint32 and Uint64.

 They are not reference counted and do not provide a separate representation
 class.

 The classes provide the means to construct, destruct, assign, set and test
 the characteristics of a client argument that is nullable. They may be
 constructed with the default NULL or a specific integer value.

 *************************************************************/

PEGASUS_NAMESPACE_BEGIN

class Uint32ArgRep;

class PEGASUS_COMMON_LINKAGE Uint32Arg
{
public:

    /** constructs an object with NULL state (default constructor)
    */
    Uint32Arg();

    /** Constructs one object from another assigning value and state
        to the new object
    */
    Uint32Arg(const Uint32Arg& x);

    /** Construct a new object with the value defined by the input
        parameter and with state !NULL

        @param x Uint32 integer that is assigned to the new object
        <pre>
            Uint32Arg maxObjectCount(9);
        </pre>
    */
    Uint32Arg(Uint32 x);

    /** destruct the object
    */
    ~Uint32Arg();

    /** Assigns one object to another
    */
    Uint32Arg& operator=(const Uint32Arg& x);

    /** Returns the value component of the object. If the object is in
        NULL state, this function still returns a value.  It is the
        responsibility of the user to test for NULLness before using any
        value.

        @return Uint32 containing the current value of the object.
        <pre>
            Uint32Arg x(999);
            Uint32 y = x.getValue();
        </pre>
    */
    const Uint32& getValue() const;

    /** Sets the input value into the object and sets the state to
        !NULL.
        @param x Uint32 value to be set into the object
        <pre>
            Uint32Arg maxObjectCount(9);
            if (!maxObjectCount.isNull && maxObjectCount.getValue() == 9)
                ...
        </pre>
    */
    void setValue(Uint32 x);

    /** Tests the object for NULL state.
        @return This method returns true if the object is NULL. Otherwise
        it returns false.
        <pre>
            Uint32Arg maxObjectCount;
            assert(maxObjectCount.isNull);
        </pre>
    */
    Boolean isNull() const;

    /** Sets the state of the object to NULL.  This also sets the
        value component to zero.
        <pre>
            Uint32Arg operationTimeout(9);
            assert(!operationTimeout.isNULL())
        </pre>
    */
    void setNullValue();

    /** Output String with value of the object or String "NULL".

       @return String containing the numeric value of the object or
           NULL if the state of the object is NULL
     */
    String toString();

    Boolean equal(const Uint32Arg& x) const;

private:
    Uint32ArgRep* _rep;
};

class Uint64ArgRep;

class PEGASUS_COMMON_LINKAGE Uint64Arg
{
public:

    /** constructs an object with NULL state (default constructor)
    */
    Uint64Arg();

    /** Constructs one object from another assigning value and state
        to the new object
    */
    Uint64Arg(const Uint64Arg& x);

    /** constructs a new object with the value defined by the input
        parameter and with state !NULL

        @param x Uint64 integer that is assigned to the new object
        <pre>
            Uint64Arg maxObjectCount(9);
        </pre>
    */
    Uint64Arg(Uint64 x);

    /** destructs the object
    */
    ~Uint64Arg();

    /** Assigns one object to another
    */
    Uint64Arg& operator=(const Uint64Arg& x);
    /** Returns the value component of the object. If the object is in
        NULL state, this function still returns a value.  It is the
        responsibility of the user to test for NULLness before using any
        value.

        @return Uint32 containing the current value of the object.
        <pre>
            Uint64Arg x(999);
            Uint64 y = x.getValue();
        </pre>
    */
    const Uint64& getValue() const;

    /** Sets the input value into the object and sets the state to
        !NULL.
        @param x Uint64 value to be set into the object
        <pre>
            Uint64Arg maxObjectCount(9);
            if (!maxObjectCount.isNull && maxObjectCount.getValue()
                ...
        </pre>
    */
    void setValue(Uint64 x);

    /** Tests the object for NULL state.
        @return This method returns true if the object is NULL. Otherwise
        it returns false.
        <pre>
            Uint64Arg maxObjectCount;
            assert(!maxObjectCount.isNull);
        </pre>
    */
    Boolean isNull() const;

    /** Sets the state of the object to NULL.  This also sets the
        value component to zero.
        <pre>
            Uint64Arg operationTimeout(9);
            operationTimeout.setNullValue();
            assert(operationTimeout.isNULL())
        </pre>
    */
    void setNullValue();

    /** Output String with value of the object or String "NULL".

       @return String containing the numeric value of the object or
           NULL if the state of the object is NULL
     */
    String toString();

    Boolean equal(const Uint64Arg& x) const;

private:
    Uint64ArgRep* _rep;
};

/**
    Test for equality of two NullableArg objects
 */
PEGASUS_COMMON_LINKAGE Boolean operator==(const Uint32Arg& x,
    const Uint32Arg& y);

PEGASUS_COMMON_LINKAGE Boolean operator==(const Uint64Arg& x,
    const Uint64Arg& y);

PEGASUS_NAMESPACE_END

#endif /* Pegasus_UintArgs_h */
