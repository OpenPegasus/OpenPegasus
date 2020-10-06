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

#ifndef Pegasus_Scope_h
#define Pegasus_Scope_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The CIMScope class represents the DMTF standard CIM qualifier scope
    definition.  The scope of a qualifier defines the types of CIM objects
    with which the qualifier may be used.  A CIMScope contains zero or more
    of these values:  CLASS, ASSOCIATION, INDICATION, PROPERTY, REFERENCE,
    METHOD, PARAMETER.
*/
class PEGASUS_COMMON_LINKAGE CIMScope
{
public:

    /**
        Constructs a CIMScope object with the value NONE.
    */
    CIMScope();

    /**
        Constructs a CIMScope object from the value of a specified
        CIMScope object.
        @param scope The CIMScope object from which to construct a new
            CIMScope object.
    */
    CIMScope(const CIMScope& scope);

    /**
        Assigns the value of the specified CIMScope object to this object.
        @param scope The CIMScope object from which to assign this
            CIMScope object.
        @return A reference to this CIMScope object.
    */
    CIMScope& operator=(const CIMScope& scope);

    /**
        Checks whether the scope contains specified scope values.
        @param scope A CIMScope specifying the scope values to check.
        @return True if the scope contains all the values in the specified
            CIMScope object, false otherwise.
    */
    Boolean hasScope(const CIMScope& scope) const;

    /**
        Adds scopes value to the CIMScope object.
        <p><b>Example:</b>
        <pre>
            CIMScope s;
            s.addScope(CIMScope::INDICATION);
            assert(s.hasScope(CIMScope::INDICATION));
        </pre>
        @param scope A CIMScope containing the scope values to add.
    */
    void addScope(const CIMScope & scope);

    /**
        Compares the CIMScope with a specified CIMScope.
        @param scope The CIMScope to be compared.
        @return True if this scope has the same set of values as the
            specified scope, false otherwise.
    */
    Boolean equal (const CIMScope & scope) const;

    /**
        Adds two scope values.
        <p><b>Example:</b>
        <pre>
            CIMScope s0(CIMScope::CLASS);
            CIMScope s1(CIMScope::PARAMETER);
            CIMScope s3 = s0 + S1;
        </pre>
        @param scope A CIMScope containing the scope value to add to this
            scope.
        @return A new CIMScope object containing a union of the values in the
            two scope objects.
    */
    CIMScope operator+(const CIMScope & scope) const;

    /**
        Returns a String representation of the CIMScope object.
        This method is for diagnostic purposes. The format of the output
        is subject to change.
        @return A String containing a human-readable representation of the
            scope value.
    */
    String toString() const;

    /**
        Indicates that the CIMScope object has no value (is uninitialized).
        This is not a valid qualifier scope.
    */
    static const CIMScope NONE;

    /**
        Indicates that the qualifier may be used with classes.
    */
    static const CIMScope CLASS;

    /**
        Indicates that the qualifier may be used with associations.
    */
    static const CIMScope ASSOCIATION;

    /**
        Indicates that the qualifier may be used with indications.
    */
    static const CIMScope INDICATION;

    /**
        Indicates that the qualifier may be used with properties.
    */
    static const CIMScope PROPERTY;

    /**
        Indicates that the qualifier may be used with references.
    */
    static const CIMScope REFERENCE;

    /**
        Indicates that the qualifier may be used with methods.
    */
    static const CIMScope METHOD;

    /**
        Indicates that the qualifier may be used with parameters.
    */
    static const CIMScope PARAMETER;

    /**
        Indicates that the qualifier may be used with any of the types
        of objects (classes, associations, indications, properties, references,
        methods, parameters).
    */
    static const CIMScope ANY;

private:

    CIMScope (const Uint32 scope);
    Uint32 cimScope;

    friend class BinaryStreamer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Scope_h */
