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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By: Marek Szermutzky (mszermutzky@de.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include "MCCA_TestAssocProvider.h"

PEGASUS_NAMESPACE_BEGIN

MCCA_TestAssocProvider::MCCA_TestAssocProvider(void)
{
    nameSpaceA = CIMNamespaceName ("root/MCCAtest/A");
    nameSpaceB = CIMNamespaceName ("root/MCCAtest/B");
    testClassName = CIMName ("MCCA_TestClass");
    assocClassName = CIMName ("MCCA_TestAssocClass");

}

MCCA_TestAssocProvider::~MCCA_TestAssocProvider(void)
{
}

void MCCA_TestAssocProvider::initialize(CIMOMHandle & cimom)
{
}

void MCCA_TestAssocProvider::terminate(void)
{
    delete this;
}

void MCCA_TestAssocProvider::associators(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    CDEBUG("MCCA_TestAssocProvider::associators() called.");
    // create a new CIMInstance based on class with name className
    CIMInstance constructedInstance = CIMInstance(testClassName);
    CIMObjectPath   targetObjectPath = CIMObjectPath();
    Array<CIMKeyBinding> keyBindings = Array<CIMKeyBinding>();
    Uint32 sourceKey = 0;

    CDEBUG("Initialisation ended.");
    handler.processing();
    CDEBUG("handler.processing started.");
    // we do ignore role, resultRole, includeQualifiers, includeClassOrigin,
    // propertyList

    CDEBUG("Next building object path.");
    /////////////////////////////////////////////////////////////////////
    //      BUILD OBJECTPATH
    /////////////////////////////////////////////////////////////////////

    // preparing object path first
    targetObjectPath.setHost("localhost:5988");
    targetObjectPath.setClassName(testClassName);
    CDEBUG("Host and classname set, host=" << objectName.getHost());
    // determine if source namespace is namespace A or B
    // and build respective target namespace ...
    if (objectName.getNameSpace().equal(nameSpaceA))
    {
        targetObjectPath.setNameSpace(nameSpaceB);
    }
    if (objectName.getNameSpace().equal(nameSpaceB))
    {
        targetObjectPath.setNameSpace(nameSpaceA);
    }
    CDEBUG("NameSpace set.");
    // determine key of source object so we can create target object
    Array<CIMKeyBinding> sourceKeyBindings = objectName.getKeyBindings();

    CDEBUG("Determining sourceKey.");
    // only one keyvalue, so we take that first one
    String      keyValueString = String(sourceKeyBindings[0].getValue());
    CDEBUG("keyValueString=" << keyValueString);
    sourceKey = strtoul((const char*) keyValueString.getCString(), NULL, 0);
    CDEBUG("sourceKey=" << sourceKey);


    CIMKeyBinding  testClassKey = CIMKeyBinding(CIMName("theKey"),
                                                    CIMValue(sourceKey) );
    CDEBUG("Created new KeyBinding testClassKey.");
    // testClassKey.setValue(keyValueString);

    CDEBUG("sourceKey = string(set keybinding),int(sourceKey)"
            << testClassKey.getValue()
            << "," << sourceKey);
    keyBindings.append(testClassKey);
    CDEBUG("Appended(testClassKey).");

    targetObjectPath.setKeyBindings(keyBindings);
    /////////////////////////////////////////////////////////////////////
    //      ADD PROPERTIES
    /////////////////////////////////////////////////////////////////////

    // add properties to the CIMInstance object
    constructedInstance.addProperty( CIMProperty( CIMName("theKey"),
                sourceKey));
    constructedInstance.addProperty( CIMProperty( CIMName("theData"),
                20+sourceKey));
    char  buffer[100];
    sprintf(buffer,"ABC-%u",20*sourceKey+sourceKey);
    constructedInstance.addProperty(CIMProperty(CIMName ("theString"),
                String(buffer)));

    CDEBUG("Added properties to the CIMInstance object");

    CIMObject cimObject(constructedInstance);
    cimObject.setPath (targetObjectPath);

    // lets deliver all instances of CIMObjectpaths I do think are okay
    handler.deliver(cimObject);
    // complete processing the request
    handler.complete();
    CDEBUG("Association call conmplete.");
}

void MCCA_TestAssocProvider::associatorNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler)
{
    throw CIMNotSupportedException("MCCA_TestAssocProvider::associatorNames");
}

void MCCA_TestAssocProvider::references(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    throw CIMNotSupportedException("MCCA_TestAssocProvider::references");
}

void MCCA_TestAssocProvider::referenceNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler)
{
    throw CIMNotSupportedException("MCCA_TestAssocProvider::referenceNames");
}

PEGASUS_NAMESPACE_END

