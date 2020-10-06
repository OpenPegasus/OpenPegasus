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
// Author: Barbara Packard (barbara_packard@hp.com)
//
// Modified By:
//
//%////////////////////////////////////////////////////////////////////////////
// WMIReferenceProvider.cpp: implementation of the WMIReferenceProvider class.
//
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// WMIReferenceProvider::
//
// ///////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include "WMICollector.h"
#include "WMIBaseProvider.h"
#include "WMIClassProvider.h"
#include "WMIReferenceProvider.h"

#include "WMIProperty.h"
#include "WMIString.h"
#include "WMIValue.h"
#include "WMIQualifier.h"
#include "WMIQualifierSet.h"
#include "WMIType.h"
#include "WMIException.h"

PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WMIReferenceProvider::WMIReferenceProvider()
{
    _collector = NULL;
    m_bInitialized = false;

}

WMIReferenceProvider::~WMIReferenceProvider()
{
    cleanup();

}

///////////////////////////////////////////////////////////////////////////////
// ATTN:
// The  following public methods are not yet implemented
//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// WMIReferenceProvider::references
//
// ///////////////////////////////////////////////////////////////////////////
Array<CIMObject> WMIReferenceProvider::references(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMObjectPath& objectName,
        const String& resultClass,
        const String& role,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList)
{
    String sQuery;
    String sQueryLanguage;

    Array<CIMObject> objects;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIReferenceProvider::references()");

    sQueryLanguage = qString(Q_WQL);
    sQuery = getReferenceQueryString(objectName,
                resultClass,
                role);

    objects = execCIMQuery(nameSpace,
                userName,
                password,
                sQueryLanguage,
                sQuery,
                propertyList,
                includeQualifiers,
                includeClassOrigin);

    PEG_METHOD_EXIT();

    return objects;
}


/////////////////////////////////////////////////////////////////////////////
// WMIReferenceProvider::referenceNames
//
// ///////////////////////////////////////////////////////////////////////////
Array<CIMObjectPath> WMIReferenceProvider::referenceNames(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMObjectPath& objectName,
        const String& resultClass,
        const String& role)
{
    Array<CIMObject> objects;
    Array<CIMObjectPath> objectNames;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIReferenceProvider::referenceNames()");

    // create an empty property list to save time...
    Array<CIMName> propNames;
    CIMPropertyList propertyList(propNames);

    // now get the objects
    objects = references(    nameSpace,
                            userName,
                            password,
                            objectName,
                            resultClass,
                            role,
                            false,
                            false,
                            propertyList);

    // now get the names from the object
    Uint32 size = objects.size();
    Uint32 i;

    //check if namespace is remote
    CIMNamespaceName oNamespace(nameSpace);
    String strNamespace = oNamespace.getString();
    String strNamespaceLower = strNamespace;
    strNamespaceLower.toLower();
    String strRemotePrefix = "";

    if (strNamespaceLower.subString(0, 4) != "root")
    {
        Uint32 uiPos = strNamespaceLower.find("root");
        if (uiPos == PEG_NOT_FOUND)
            throw CIMException(CIM_ERR_FAILED);

        strRemotePrefix = strNamespace.subString(0, uiPos);
    }

    for (i=0; i<size; i++)
    {
        CIMObjectPath oObjectPath = objects[i].getPath();

        if (strRemotePrefix != "")
        {
            strNamespace = strRemotePrefix;
            oNamespace = strNamespace.append(
                oObjectPath.getNameSpace().getString());
            oObjectPath.setNameSpace(oNamespace);
        }

        objectNames.append(oObjectPath);
    }

    PEG_METHOD_EXIT();

    return objectNames;
}


//////////////////////////////////////////////////////////////////////////////
// WMIReferenceProvider
//        private methods
//
// ///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// WMIReferenceProvider::getReferenceQueryString - calls the BaseProvider
//          method to build the query string from the input parameters
//
// ///////////////////////////////////////////////////////////////////////////
String WMIReferenceProvider::getReferenceQueryString(
        const CIMObjectPath &objectName,
        const String &resultClass,
        const String &role)
{
    String sQuery;

    sQuery = qString(Q_REFERENCES);

    return getQueryString(objectName, sQuery,
        String::EMPTY, resultClass, role);

}

PEGASUS_NAMESPACE_END

