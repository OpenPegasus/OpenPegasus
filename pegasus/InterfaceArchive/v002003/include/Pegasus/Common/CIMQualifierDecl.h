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
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_QualifierDecl_h
#define Pegasus_QualifierDecl_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMScope.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMValue.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

class CIMConstQualifierDecl;
class CIMClassRep;
class CIMQualifierDeclRep;

/** The CIMQualifierDecl class is used to represent CIM qualifier
    declarations in Pegasus.
    Note that the Declarations
    are not the same as CIM Qualiifers as seen on Classes, properties, etc.
    These are the original declarations of qualifiers (i.e. input from the compiler
    qualifier Declarations).
*/
class PEGASUS_COMMON_LINKAGE CIMQualifierDecl
{
public:
    /// Constructor.
    CIMQualifierDecl();

    /// Constructor - Creates a CIMQualifierDecl from another CIMQualifierDecl.
    CIMQualifierDecl(const CIMQualifierDecl& x);

    /** Constructor - Constructs a single CIMQualifierDecl object.
        @param name - CIMName containing the name of the Qualifier being created.
        @param value - CIMValue for the qualifier.
        @param scope - CIMScope containing the scope of the qualifier.
        @param flavor - Optional definition of the flavor for the qualifier.  
        CIMFlavor::DEFAULTS is used if no value supplied.  
        @param arraySize - Optional integer defining the arraysize if the
        qualifier is an array type with fixed value array. The default is
        zero indicating that the qualifier declaration is not a fixed size
        array.
    */
    CIMQualifierDecl(
        const CIMName& name, 
        const CIMValue& value, 
        const CIMScope & scope,
        const CIMFlavor & flavor = CIMFlavor (CIMFlavor::DEFAULTS),
        Uint32 arraySize = 0);

    /// Destructor.
    ~CIMQualifierDecl();

    /// Operator.
    CIMQualifierDecl& operator=(const CIMQualifierDecl& x);

    /** Get the name of the CIMQualifierDecl object.
        @return CIMName containing the name of the CIMQualifierDecl object.
    */
    const CIMName& getName() const;

    /** Sets the name in the CIMQualifierDecl object.
        @param name CIMName containing name to be set on this qualifier.
    */
    void setName(const CIMName& name);

    /** Gets the Qualifier Declaration type which is the
        value type (boolean, etc. for this qualifier).
        @return the type as CIMType.
    */
    CIMType getType() const;

    /** Determines if this qualifier declaration is an array type.
        @return true if this is an array type, false otherwise.
    */
    Boolean isArray() const;

    ///
    const CIMValue& getValue() const;

    ///
    void setValue(const CIMValue& value);

    ///
    const CIMScope & getScope() const;

    /** Gets the Flavor definition from the qualifier declaration.
        @return CIMFlavor object containing the flavor flags.  The 
        CIMFlavor hasFlavor method can be used to test against the flavor 
        constants defined in CIMFlavor.
        @See CIMFlavor
    */
    const CIMFlavor & getFlavor() const;

    ///
    Uint32 getArraySize() const;

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized,
                 false otherwise.
    */
    Boolean isUninitialized() const;
    
    /** Compares two qualifier declarations.
        @return true if they are identical, false otherwise.
    */ 
    Boolean identical(const CIMConstQualifierDecl& x) const;

    ///
    CIMQualifierDecl clone() const;

private:

    CIMQualifierDecl(CIMQualifierDeclRep* rep);

    void _checkRep() const;

    CIMQualifierDeclRep* _rep;

    friend class CIMConstQualifierDecl;
    friend class CIMClassRep;
    friend class XmlWriter;
    friend class MofWriter;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstQualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

///
class PEGASUS_COMMON_LINKAGE CIMConstQualifierDecl
{
public:
    ///
    CIMConstQualifierDecl();

    ///
    CIMConstQualifierDecl(const CIMConstQualifierDecl& x);

    ///
    CIMConstQualifierDecl(const CIMQualifierDecl& x);

    /** Constructor - Creates a CIMConstQualiferDecl.
        @param name - CIMName containing the name of the Qualifier declaration.
        @param value - CIMValue for the qualifier.
        @param scope - CIMScope containing the scope of the qualifier.
        @param flavor - Optional definition of the flavor for the qualifier.  
        CIMFlavor::DEFAULTS is used if no value supplied.  
        @param arraySize - Optional integer defining the arraysize if the
        qualifier is an array type with fixed value array. The default is
        zero indicating that the qualifier declaration is not a fixed size
        array.
    */
    CIMConstQualifierDecl(
        const CIMName& name, 
        const CIMValue& value, 
        const CIMScope & scope,
        const CIMFlavor & flavor = CIMFlavor (CIMFlavor::DEFAULTS),
        Uint32 arraySize = 0);

    ///
    ~CIMConstQualifierDecl();

    ///
    CIMConstQualifierDecl& operator=(const CIMConstQualifierDecl& x);

    ///
    CIMConstQualifierDecl& operator=(const CIMQualifierDecl& x);

    ///
    const CIMName& getName() const;

    ///
    CIMType getType() const;

    ///
    Boolean isArray() const;

    ///
    const CIMValue& getValue() const;

    ///
    const CIMScope & getScope() const;

    ///
    const CIMFlavor & getFlavor() const;

    ///
    Uint32 getArraySize() const;

    ///
    Boolean isUninitialized() const;

    ///
    Boolean identical(const CIMConstQualifierDecl& x) const;

    ///
    CIMQualifierDecl clone() const;

private:

    void _checkRep() const;

    CIMQualifierDeclRep* _rep;

    friend class CIMQualifierDecl;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMQualifierDecl
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_QualifierDecl_h */
