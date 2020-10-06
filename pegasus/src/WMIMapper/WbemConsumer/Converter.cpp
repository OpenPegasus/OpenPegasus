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

#include "stdafx.h"
#include "Converter.h"

#include "../WMIProvider/WMICollector.h"
#include "../WMIProvider/WMIClassProvider.h"
#include "../WMIProvider/MyString.h"
#include "../WMIProvider/config.h"

#include "../WMIProvider/WMIValue.h"

#include <Pegasus/Common/tracer.h>
#include <Pegasus/Common/CIMObjectRep.h>

#include "ConsumerTracer.h"

typedef LONG     HRESULT;

/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//
/////////////////////////////////////////////////////////////////////////////

CConverter::CConverter()
{

}

CConverter::~CConverter()
{

}

/////////////////////////////////////////////////////////////////////////////
// CConverter::getStringProperty - helper function to retrieve specific
//    string properties from a class or instance object
//
/////////////////////////////////////////////////////////////////////////////
Pegasus::String CConverter::_getStringProperty(IWbemClassObject *pObject, 
    const CComBSTR &bsPropertyName)
{
    CNS_METHOD_ENTER(Pegasus::TRC_WMI_MAPPER_CONSUMER, 
        "CConverter::_getStringProperty");

    HRESULT hr;
    CComVariant vValue;
    CComBSTR bs;
    Pegasus::CMyString s = "";
    Pegasus::CMyString name; name = bsPropertyName;

    // get variant value from WMI Object
    hr = pObject->Get(bsPropertyName, 0, &vValue, NULL, NULL);
    if (SUCCEEDED(hr))
    {
        if (VT_NULL != vValue.vt)
        {
            // there is a value... if not we will return an empty string...
            bs = vValue.bstrVal;

            if (0 != bs.Length())
            {
                s = bs;
            }
        }

        CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
            Pegasus::Tracer::LEVEL3,
            "CConverter::_getStringProperty() - Value for %s is %s", 
            (LPCTSTR)name, (LPCTSTR)s));
    }
    else
    {
        CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
            Pegasus::Tracer::LEVEL3,
            "CConverter::_getStringProperty() - Error getting Property", 
            (LPCTSTR)name, hr));
    }

    CNS_METHOD_EXIT();

    if (VT_NULL == vValue.vt)
    {
        // returns empty string if there's no result
        vValue.Clear();
        return Pegasus::String::EMPTY;
    }
    else
    {
        vValue.Clear();
        return Pegasus::String((LPCTSTR)s);
    }
}

/////////////////////////////////////////////////////////////////////////////
// convertIndicationWMI2Peg
//
// Converts a WMI Event instance to a CIM Pegasus Event instance
//
/////////////////////////////////////////////////////////////////////////////
bool CConverter::convertIndicationWMI2Peg(IWbemClassObject *pConsumer, 
    IWbemClassObject *pWbemObj, 
    Pegasus::CIMInstance  *pInstance)
{
    Pegasus::WMICollector collector(true);
    Pegasus::CIMPropertyList propertyList;

    CNS_METHOD_ENTER(Pegasus::TRC_WMI_MAPPER_CONSUMER,
        "CConverter::convertIndicationWMI2Peg");

    // Get the class name. It's fundamental set the name of new CIMInstance
    // that is going to be created. Otherwise the collector can't converts it.
    Pegasus::String className = collector.getClassName(pWbemObj);        
    Pegasus::CIMInstance newInstance(className);

    // create a new qualifier SourceHost to pass the Source Host Name
    CComBSTR bsServer = "__SERVER";
    Pegasus::String sourceHost = _getStringProperty(pConsumer, bsServer);

    Pegasus::CIMQualifier mQualifier(
        Pegasus::CIMName(Pegasus::String("SourceHost")), 
        Pegasus::CIMValue(sourceHost));             

    newInstance.addQualifier(mQualifier);

    if(!collector.getCIMInstance(pWbemObj, newInstance, false, true, 
        true, propertyList, true)) 
    {
        throw Pegasus::CIMException(Pegasus::CIM_ERR_NOT_FOUND);
    }    

    *pInstance = newInstance;
    CNS_METHOD_EXIT();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// convertHandlerWMI2Peg
//
// Converts a WMI Handler instance to a CIM Pegasus Event instance
//
/////////////////////////////////////////////////////////////////////////////
bool CConverter::convertHandlerWMI2Peg(IWbemClassObject *pConsumer,
    Pegasus::CIMInstance  *pInstance)
{
    Pegasus::WMICollector           collector(true);
    Pegasus::CIMPropertyList        propertyList;

    CNS_METHOD_ENTER(Pegasus::TRC_WMI_MAPPER_CONSUMER,
        "CConverter::convertHandlerWMI2Peg");

    // Get the class name. It's fundamental set the name of new CIMInstance
    // that is going to be created. Otherwise the collector can't converts it.
    Pegasus::String className = collector.getClassName(pConsumer);        
    Pegasus::CIMInstance newInstance(className);

    if(!collector.getCIMInstance(pConsumer, newInstance, false, true, 
        true, propertyList, true)) 
    {
    throw Pegasus::CIMException(Pegasus::CIM_ERR_NOT_FOUND);
    }    

    *pInstance = newInstance;

    CNS_METHOD_EXIT();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// getProp
// Get all properties from as WMI Indication Class and copy it to CIMInstance 
// Object
//
/////////////////////////////////////////////////////////////////////////////
bool CConverter::getProp(IWbemClassObject *pClass, 
    long lFlags,
    bool includeQualifiers,
    bool includeClassOrigin,
    Pegasus::CIMInstance *pInstance)
{
    HRESULT hr;
    Pegasus::String sMessage;

    CComBSTR bsName; // of the property
    CComVariant vValue; // of the property
    long lFlavor; // of the property
    CIMTYPE type;
    Pegasus::CIMProperty property;

    CNS_METHOD_ENTER(Pegasus::TRC_WMI_MAPPER_CONSUMER,
        "CConverter::getProp()");

    bool bFound = false;

    hr = pClass->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    sMessage = "BeginEnumeration()";

    if (SUCCEEDED(hr))
    {
        bsName.Empty();
        vValue.Clear();
        sMessage = "Next()";
        hr = pClass->Next(0, &bsName, &vValue, &type, &lFlavor);
    }

    // process each property
    while (SUCCEEDED(hr))
    {
        if (WBEM_S_NO_MORE_DATA == hr)
        {
            break;
        }

        CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER,
            Pegasus::Tracer::LEVEL4,
            "getProp() - CIMTYPE[%x] - WMITYPE[%x]",
            type, 
            vValue.vt));

        bFound = true;

        Pegasus::CMyString sPropName; sPropName = bsName;

        bool bPropagated = 
            (lFlavor & WBEM_FLAVOR_ORIGIN_PROPAGATED) ? true : false;

        try
        {
            property = Pegasus::WMICollector::getProperty(
                pClass, 
                bsName, 
                vValue, 
                type, 
                includeClassOrigin, 
                includeQualifiers, 
                bPropagated);
        }
        catch( Pegasus::TypeMismatchException & e )
        {
            // ATTN:
            // unsupported for now - do some tracing...
            Pegasus::String sClass = 
                Pegasus::WMICollector::getClassName(pClass);
                
            CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
                Pegasus::Tracer::LEVEL3,
                "getProp() - Ignoring invalid type \
                for %s in %s.  %s, unsupported WMI CIM type is %x",
                (LPCTSTR)sPropName, sClass, e.getMessage(), type));

            bsName.Empty();
            vValue.Clear();

            sMessage = "Next()";
            hr = pClass->Next(0, &bsName, &vValue, &type, &lFlavor);

            continue;
        }

        try
        {
            pInstance->addProperty(property);
        }
        catch( Pegasus::AlreadyExistsException& e )
        {
            // ignore this
            CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
                Pegasus::Tracer::LEVEL4,
                "getProp() - Property %s is already defined", 
                (LPCTSTR)sPropName));
            CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER,
                Pegasus::Tracer::LEVEL4,
                "getProp() - %s", 
                e.getMessage()));

        }
        catch( Pegasus::Exception & e )
        {
            // ignore this
            CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
                Pegasus::Tracer::LEVEL4,
                "getProp() - Ignoring AddedReferenceToClass.  %s", 
                e.getMessage()));

        }
        catch(... ) 
        {
            throw Pegasus::CIMException(Pegasus::CIM_ERR_FAILED, 
                "[getProp] general 1");
        }

        bsName.Empty();
        vValue.Clear();

        sMessage = "Next()";
        hr = pClass->Next(0, &bsName, &vValue, &type, &lFlavor);
    }
    
    pClass->EndEnumeration();
    vValue.Clear();

    if (FAILED(hr))
    {
        CNS_TRACE((Pegasus::TRC_WMI_MAPPER_CONSUMER, 
            Pegasus::Tracer::LEVEL4,
            "getProp() - %s result is %x", sMessage.getCString(), hr));

        throw Pegasus::CIMException(Pegasus::CIM_ERR_FAILED, 
            "[getProp] general 2");
    }

    CNS_METHOD_EXIT();
    
    return bFound;
}
