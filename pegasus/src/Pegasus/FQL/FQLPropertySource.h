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

#ifndef Pegasus_FQLPropertySource_h
#define Pegasus_FQLPropertySource_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/FQL/Linkage.h>
#include <Pegasus/FQL/FQLOperand.h>

PEGASUS_NAMESPACE_BEGIN

/** This class must be implemented to provide a source of properties for the
    FQLSelectStatement::evaluate method which calls methods of
    this class to obtain real values for property names used in the where
    clause.
*/
class PEGASUS_FQL_LINKAGE FQLPropertySource
{
public:

    /** Virtual destructor.
    */
    virtual ~FQLPropertySource();

    /** Returns the value whose property has the given name. The output
    parameter value is populated with the value of that parameter.
    Note that only integer, double, and string types are supported
    (see the FQLOperand class). The implementer of this method must
    perform appropriate conversions to the appropriate type.

    @param propertyName name of property to be gotten.
    @param value  FQLOperand thatholds the value of the property
                  upon return with the FQLType derived from the
                  property CIMType.
    @return true if such a property was found, false otherwise.
    */
    virtual Boolean getValue(
    const String& propertyName,
    FQLOperand& value) const = 0;



    /** determines whether the property defined by Name is an array
        property.
    @param propertyName Name of Property.
    @return true if the property is a name property
    @exception returns exception of property does not exist.
    */
    virtual Boolean isArrayProperty(const CIMName& propertyName) const = 0;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FQLPropertySource_h */
