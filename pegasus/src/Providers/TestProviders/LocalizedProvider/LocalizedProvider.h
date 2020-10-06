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

#ifndef LocalizedProvider_h
#define LocalizedProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
//#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
//#include <Pegasus/Provider/CIMQueryProvider.h>
#include <Pegasus/Provider/CIMIndicationProvider.h>
#include <Pegasus/Provider/CIMIndicationConsumerProvider.h>

#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/ContentLanguageList.h>

PEGASUS_USING_PEGASUS;

class LocalizedProvider :
    public PEGASUS_NAMESPACE(CIMInstanceProvider),
//  public CIMAssociationProvider,
    public PEGASUS_NAMESPACE(CIMMethodProvider),
//  public CIMQueryProvider,
    public PEGASUS_NAMESPACE(CIMIndicationProvider),
    public PEGASUS_NAMESPACE(CIMIndicationConsumerProvider)
{
public:

    LocalizedProvider();
    virtual ~LocalizedProvider();

    // CIMProvider interface
    virtual void initialize(PEGASUS_NAMESPACE(CIMOMHandle)& cimom);
    virtual void terminate();

    // CIMInstanceProvider interface
    virtual void getInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& instanceReference,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(Boolean) includeClassOrigin,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(InstanceResponseHandler)& handler);

    virtual void enumerateInstances(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& classxReference,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(Boolean) includeClassOrigin,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(InstanceResponseHandler)& handler);

    virtual void enumerateInstanceNames(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& classReference,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);

    virtual void modifyInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& instanceReference,
        const PEGASUS_NAMESPACE(CIMInstance)& instanceObject,
        const PEGASUS_NAMESPACE(Boolean) includeQualifiers,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        PEGASUS_NAMESPACE(ResponseHandler)& handler);

    virtual void createInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& instanceReference,
        const PEGASUS_NAMESPACE(CIMInstance)& instanceObject,
        PEGASUS_NAMESPACE(ObjectPathResponseHandler)& handler);

    virtual void deleteInstance(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& instanceReference,
        PEGASUS_NAMESPACE(ResponseHandler)& handler);

/*
    // CIMAssociationProvider interface
    virtual void associators(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler);

    virtual void associatorNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        ObjectPathResponseHandler& handler);

    virtual void references(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler);

    virtual void referenceNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        ObjectPathResponseHandler& handler);
*/

    // CIMMethodProviderFacade
    virtual void invokeMethod(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& objectReference,
        const PEGASUS_NAMESPACE(CIMName)& methodName,
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMParamValue)>&
            inParameters,
        PEGASUS_NAMESPACE(MethodResultResponseHandler)& handler);


    // CIMQueryProvider interface
/*
    virtual void executeQuery(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query,
        ObjectResponseHandler& handler);
*/

    // CIMIndicationProvider interface
    virtual void enableIndications(
        PEGASUS_NAMESPACE(IndicationResponseHandler)& handler);

    virtual void disableIndications();

    virtual void createSubscription(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& subscriptionName,
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMObjectPath)>&
            classNames,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        const PEGASUS_NAMESPACE(Uint16) repeatNotificationPolicy);

    virtual void modifySubscription(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& subscriptionName,
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMObjectPath)>&
            classNames,
        const PEGASUS_NAMESPACE(CIMPropertyList)& propertyList,
        const PEGASUS_NAMESPACE(Uint16) repeatNotificationPolicy);

    virtual void deleteSubscription(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMObjectPath)& subscriptionName,
        const PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMObjectPath)>&
            classNames);

    virtual void consumeIndication(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(String)& url,
        const PEGASUS_NAMESPACE(CIMInstance)& indicationInstance);

private:

    CIMOMHandle _cimom;

    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMObjectPath)> _instanceNames;
    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(CIMInstance)> _instances;
    PEGASUS_NAMESPACE(Array)<PEGASUS_NAMESPACE(ContentLanguageList)>
        _instanceLangs;

    PEGASUS_NAMESPACE(MessageLoaderParms) msgParms;
    PEGASUS_NAMESPACE(MessageLoaderParms) notSupportedErrorParms;
    PEGASUS_NAMESPACE(MessageLoaderParms) contentLangParms;
    PEGASUS_NAMESPACE(MessageLoaderParms) roundTripErrorParms;

    PEGASUS_NAMESPACE(String) roundTripString;

    void _checkRoundTripString(
        const PEGASUS_NAMESPACE(OperationContext)& context,
        const PEGASUS_NAMESPACE(CIMInstance)& instanceObject);

    PEGASUS_NAMESPACE(AcceptLanguageList) getRequestAcceptLanguages(
        const PEGASUS_NAMESPACE(OperationContext)& context);

    PEGASUS_NAMESPACE(ContentLanguageList) getRequestContentLanguages(
        const PEGASUS_NAMESPACE(OperationContext)& context);

    PEGASUS_NAMESPACE(CIMObjectPath) buildRefFromInstance(
        const PEGASUS_NAMESPACE(CIMInstance)& instanceObject);

    PEGASUS_NAMESPACE(ContentLanguageList) _loadLocalizedProps(
        PEGASUS_NAMESPACE(AcceptLanguageList)& acceptLangs,
        PEGASUS_NAMESPACE(ContentLanguageList)& contentLangs,
        PEGASUS_NAMESPACE(CIMInstance)& instance);

    PEGASUS_NAMESPACE(ContentLanguageList) _addResourceBundleProp(
        PEGASUS_NAMESPACE(AcceptLanguageList)& acceptLangs,
        PEGASUS_NAMESPACE(CIMInstance)& instance);

    PEGASUS_NAMESPACE(ContentLanguageList) _addResourceBundleProp(
        PEGASUS_NAMESPACE(CIMInstance)& instance);

    void _replaceRBProperty(
        PEGASUS_NAMESPACE(CIMInstance)& instance,
        const PEGASUS_NAMESPACE(String)& newProp);

    PEGASUS_NAMESPACE(ContentLanguageList) _addContentLanguagesProp(
        PEGASUS_NAMESPACE(CIMInstance)& instance);

    void _setHandlerLanguages(
        PEGASUS_NAMESPACE(ResponseHandler)& handler,
        PEGASUS_NAMESPACE(ContentLanguageList)& langs);

    void _testCIMOMHandle();

    void _validateCIMOMHandleResponse(
        const PEGASUS_NAMESPACE(String)& expectedLang);

    void _generateIndication();
};

#endif
