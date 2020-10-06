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

#ifndef Pegasus_Scope_h
#define Pegasus_Scope_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The CIMScope class implements the concept of the scope of a CIM qualifier 
    object.  The scope of a qualifier defines the set of types of CIM objects 
    with which the qualifier may be used.  The possible values are:
    NONE, CLASS, ASSOCIATION, INDICATION, PROPERTY, REFERENCE, METHOD, 
    PARAMETER, ANY.
    The scope is a set of one or more of these possible values.
    The value "NONE" implies a CIMScope object that has not yet been assigned a
    value (uninitialized).  It is not a valid value for the scope of a 
    qualifier object.
    The value "ANY" means that the qualifier may be used with any of the CIM 
    object types, and is equivalent to listing each of the object types in the 
    scope.
*/
class PEGASUS_COMMON_LINKAGE CIMScope
{
public:

    /** Constructs a CIMScope object with no value set (default constructor).
        The values are null.
    */
    CIMScope ();

    /** Constructs a CIMScope object from an existing CIMScope object (copy 
        constructor).
        @param   scope   Specifies an instance of CIMScope object.
    */
    CIMScope (const CIMScope & scope);

    /** Assigns the value of one CIMScope object to another (assignment 
        operator).
        @param   scope Specifies the name of a CIMScope object that contains
        the values that you want to assign to another CIMScope object.
        @return  The CIMScope object with the values of the specified CIMScope
        object. For example:
        <pre>
            CIMScope s0;
            CIMScope s1(s0);
        </pre>
        The values in CIMScope s0 are assigned to the CIMScope s1 instance.
    */
    CIMScope & operator= (const CIMScope & scope);

    /** Determines if every value in the specified CIMScope object is included 
        in this CIMScope object.
        @param   scope - Specifies a name of a CIMScope object.
        @return  True if every value in the specified CIMScope object is 
                 included in the CIMScope object; otherwise, false. 
    */
    Boolean hasScope (const CIMScope & scope) const;

    /** Adds the specified scope value to the CIMScope object.
        @param   scope - Specifies a scope value. For example:
        <pre>
            CIMScope s0;
            s0.addScope (CIMScope::INDICATION);
            if(s0.hasScope (CIMScope::INDICATION))
                ...				..
        </pre>
    */
    void addScope (const CIMScope & scope);

    /** Compares two CIMScope objects.
        @param  scope - Specifies a CIMScope object.
        @return True if the two CIMScope objects are equal; otherwise, false.
        For example,
        <pre>
            CIMScope s0;
            s0.addScope (CIMScope::CLASS + CIMScope::PARAMETER);
            if(s0.hasScope (CIMScope::CLASS))
                ...	..
    </pre>
    */
    Boolean equal (const CIMScope & scope) const;

    /** Combines two CIMScope objects.
        @param  scope - Specifies a CIMScope object to add.
        @return A new CIMScope object that represents the combination of this
                scope object with the specified scope object.
        <pre>
            CIMScope s0(CIMScope::CLASS);
            CIMScope s1(CIMScope::PARAMETER);
            CIMScope s3 = s0 + S1;
        </pre>
    */
    CIMScope operator+ (const CIMScope & scope) const;

    /** Returns a String representation of the CIMScope object.
        This method is for diagnostic purposes. The format of the output
        is subject to change.
    */
    String toString () const;

    /** Indicates that the CIMScope object has no value (is uninitialized).
    */
    static const CIMScope NONE;

    /** Indicates that the qualifier may be used with classes.
    */
    static const CIMScope CLASS;

    /** Indicates that the qualifier may be used with associations.
    */
    static const CIMScope ASSOCIATION;

    /** Indicates that the qualifier may be used with indications.
    */
    static const CIMScope INDICATION;

    /** Indicates that the qualifier may be used with properties.
    */
    static const CIMScope PROPERTY;

    /** Indicates that the qualifier may be used with references.
    */
    static const CIMScope REFERENCE;

    /** Indicates that the qualifier may be used with methods.
    */
    static const CIMScope METHOD;

    /** Indicates that the qualifier may be used with parameters.
    */
    static const CIMScope PARAMETER;

    /** Indicates that the qualifier may be used with any of the types
        of objects (classes, associations, indications, properties, references,
        methods, parameters).
    */
    static const CIMScope ANY;

private:

    // 
    CIMScope (const Uint32 scope);
    // Private member for storing he CIMScope
    Uint32 cimScope;
    
    friend class BinaryStreamer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Scope_h */
