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

///
class PEGASUS_COMMON_LINKAGE CIMParamValue
{
public:

    ///
    CIMParamValue();

    ///
    CIMParamValue(const CIMParamValue& x);

    ///
    CIMParamValue& operator=(const CIMParamValue& x);

    ///
    CIMParamValue(
	String parameterName,
	CIMValue value,
	Boolean isTyped=true);

    ///
    ~CIMParamValue();

    ///
    String getParameterName() const;

    ///
    CIMValue getValue() const;

    ///
    Boolean isTyped() const;

    ///
    void setParameterName(String& parameterName);

    ///
    void setValue(CIMValue& value);

    ///
    void setIsTyped(Boolean isTyped);

    ///
    CIMParamValue clone() const;

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized,
                 false otherwise.
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
