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

#ifndef Pegasus_ParamValue_h
#define Pegasus_ParamValue_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class CIMParamValueRep;

////////////////////////////////////////////////////////////////////////////////
//
// CIMParamValue
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMParamValue class represents an extrinsic method parameter value,
    as defined in the DMTF Specification for CIM Operations over HTTP.

    <p>The CIMParamValue class uses a shared representation model, such that
    multiple CIMParamValue objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMParamValue object affects all the CIMParamValue
    objects that refer to the same data copy.  The data remains valid until
    all the CIMParamValue objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMParamValue
{
public:

    /**
        Constructs an uninitialized CIMParamValue object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMParamValue();

    /**
        Constructs a CIMParamValue object from the value of a specified
        CIMParamValue object, so that both objects refer to the same data
        copy.
        @param x The CIMParamValue object from which to construct a new
            CIMParamValue object.
    */
    CIMParamValue(const CIMParamValue& x);

    /**
        Assigns the value of the specified CIMParamValue object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMParamValue object from which to assign this
            CIMParamValue object.
        @return A reference to this CIMParamValue object.
    */
    CIMParamValue& operator=(const CIMParamValue& x);

    /**
        Constructs a parameter value with the specified attributes.
        @param parameterName A String containing the name of this parameter.
        @param value A CIMValue containing the value of this parameter.
        @param isTyped A Boolean indicating whether the type indicated in
            the value is correct.  This is needed because the Specification
            for the Representation of CIM in XML does not require type
            information to be specified with a parameter value.
        @exception UninitializedObjectException If the parameter name is an
            empty String.
    */
    CIMParamValue(
	String parameterName,
	CIMValue value,
	Boolean isTyped=true);

    /**
        Destructs the CIMParamValue object.
    */
    ~CIMParamValue();

    /**
        Gets the parameter name for the parameter value.
        @return A String containing the parameter name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    String getParameterName() const;

    /**
        Gets the value for the parameter.
        @return A CIMValue containing the parameter value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMValue getValue() const;

    /**
        Checks whether the parameter value has the correct type.
        @return True if the parameter value is known to have the correct
            type, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isTyped() const;

    /**
        Sets the parameter name for the parameter value.
        @param parameterName A String containing the parameter name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setParameterName(String& parameterName);

    /**
        Sets the value for the parameter.
        @param value A CIMValue containing the parameter value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setValue(CIMValue& value);

    /**
        Sets a flag indicating whether the parameter value has the correct
        type.
        @param isTyped A Boolean indicating whether the parameter value is
            known to have the correct type.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setIsTyped(Boolean isTyped);

    /**
        Makes a deep copy of the object.  This creates a new copy of all
        the object attributes.
        @return A new copy of the CIMParamValue object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMParamValue clone() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

private:

    CIMParamValue(CIMParamValueRep* rep);

    void _checkRep() const;

    CIMParamValueRep* _rep;

    friend class XmlWriter;
};

#define PEGASUS_ARRAY_T CIMParamValue
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ParamValue_h */
