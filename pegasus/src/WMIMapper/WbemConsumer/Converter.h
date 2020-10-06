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

////////////////////////////////////////////////////////////////////////////
// Converter.h: interface for the CConverter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONVERTER_H__676FDAFD_AB1A_46B8_8CF8_F2AEEBD865BB__INCLUDED_)
#define AFX_CONVERTER_H__676FDAFD_AB1A_46B8_8CF8_F2AEEBD865BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMInstance.h>

#include <wbemcli.h>
#include <wbemidl.h>

class CConverter  
{
public:
    CConverter();
    virtual ~CConverter();

    /**
        getProp
        Get all properties from as WMI Indication Class and copy it to
        CIMInstance Object 
    */ 
    bool getProp(IWbemClassObject *pClass,
        long lFlags,
        bool includeQualifiers,
        bool includeClassOrigin,
        Pegasus::CIMInstance *pInstance);

    /**
        convertIndicationWMI2Peg
        Converts a WMI Event instance to a CIM Pegasus Event instance
    */
    bool convertIndicationWMI2Peg(IWbemClassObject *pConsumer, 
        IWbemClassObject *pWbemObj, 
        Pegasus::CIMInstance *pCIMObj);

    /**
        convertHandlerWMI2Peg 
        Converts a WMI Handler instance to a CIM Pegasus Event instance
    */  
    bool convertHandlerWMI2Peg(IWbemClassObject *pConsumer,
        Pegasus::CIMInstance *pCIMObj);

private:
    /**      
        CConverter::getStringProperty - helper function to retrieve specific
        string properties from a class or instance object
    */                                            
    Pegasus::String _getStringProperty(IWbemClassObject *pObject, 
        const CComBSTR &bsPropertyName);
};

#endif // !defined(
       // AFX_CONVERTER_H__676FDAFD_AB1A_46B8_8CF8_F2AEEBD865BB__INCLUDED_)
