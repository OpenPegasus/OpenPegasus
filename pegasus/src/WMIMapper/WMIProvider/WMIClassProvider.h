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
// Modified By:    Adriano Zanuz (adriano.zanuz@hp.com)
//              Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIClassProvider_h
#define Pegasus_WMIClassProvider_h

#include "Stdafx.h"
#include <WMIMapper/WMIProvider/WMIBaseProvider.h>
#include <WMIMapper/WMIProvider/WMIQualifier.h>

PEGASUS_NAMESPACE_BEGIN


class WMICollector;

class PEGASUS_WMIPROVIDER_LINKAGE WMIClassProvider : public WMIBaseProvider
{
public:
    WMIClassProvider(void);
    virtual ~WMIClassProvider(void);

    /// virtual class CIMClass. From the operations class
    virtual CIMClass getClass(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& className,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList());

    // deleteClass
    virtual void deleteClass(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& className);

    // createClass
    virtual void createClass(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMClass& newClass,
        Boolean updateClass = false);

    // modifyClass
    virtual void modifyClass(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMClass& modifiedClass);

    /// enumerateClasses
    virtual Array<CIMClass> enumerateClasses(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& className = String::EMPTY,
        Boolean deepInheritance = false,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false);

    /// enumerateClassNames
    virtual Array<CIMName> enumerateClassNames(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& className = String::EMPTY,
        Boolean deepInheritance = false);

protected:

    // verifies if the class already exists into the wmi
    void performInitialCheck(const CIMClass& newClass,
                             Boolean updateClass = false);

    // do the initial consistences defined by the CIM model,
    // this is a step of create class
    Boolean classAlreadyExists(const String& className);


private:
    // create the properties for the new class, this is a step of create class
    // keys are a special kind of property
    void createProperties(const CIMClass& newClass,
                          IWbemServices *pServices,
                          IWbemClassObject *pNewClass);
    // create the class name and the class qualifiers, this is a step of create
    // classif the function could create the name and qualifiers, it returns a
    // valid pNewClass.
    void createClassNameAndClassQualifiers(const CIMClass& newClass,
                                           IWbemServices *pServices,
                                           IWbemClassObject **pNewClass,
                                            const bool hasSuperClass);
    // create the methods of a class
    void createMethods (const CIMClass& newClass, IWbemServices *pServices,
        IWbemClassObject *pNewClass);

    // create one property
    void createProperty (const CIMProperty &keyProp,
        IWbemClassObject *pNewClass);
    // create one qualifier
    void createQualifier (const WMIQualifier &qualifier,
        IWbemQualifierSet *pQual);
    // create a method
    void createMethod (CIMConstMethod &method,
                       IWbemServices *pServices,
                       IWbemClassObject *pNewClass);
    // create a parameter
    void createParam (const CIMConstParameter &param,
        IWbemClassObject *pNewClass);
};


PEGASUS_NAMESPACE_END

#endif // ifndef Pegasus_WMIClassProvider_h
