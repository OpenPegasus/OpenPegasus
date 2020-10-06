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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/ProviderException.h>
#include "DNSService.h"
#include "DNSServiceProvider.h"

PEGASUS_USING_STD;

/* ==========================================================================
   Miscellaneous Defines
   ========================================================================== */
static const int MAX_KEYS = 4;  // The number of keys for the class
static const String CLASS_PG_DNS_SERVICE("PG_DNSService");
static const String CLASS_CIM_UNITARY_COMPUTER_SYSTEM(
    "CIM_UnitaryComputerSystem");

/* ==========================================================================
   WBEM MOF property names.  These values are returned by the provider as
   the property names.
   ========================================================================== */
static const String PROPERTY_CAPTION("Caption");
static const String PROPERTY_DESCRIPTION("Description");
static const String PROPERTY_SYSTEM_CREATION_CLASS_NAME(
    "SystemCreationClassName");
static const String PROPERTY_SYSTEM_NAME("SystemName");
static const String PROPERTY_CREATION_CLASS_NAME("CreationClassName");
static const String PROPERTY_NAME("Name");
static const String PROPERTY_SEARCH_LIST("SearchList");
static const String PROPERTY_ADDRESSES("Addresses");

DNSServiceProvider::DNSServiceProvider()
{
}

DNSServiceProvider::~DNSServiceProvider()
{
}

void DNSServiceProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    CIMName className;
    String dName, systemName;
    className = instanceReference.getClassName();
    Array<CIMKeyBinding> keys;
    CIMInstance instance;

    //-- make sure we're working on the right class
    className = instanceReference.getClassName();
    if (!className.equal (CLASS_PG_DNS_SERVICE))
        throw CIMNotSupportedException(
            "DNSServiceProvider does not support class " +
                className.getString());

    DNSService dns;

    if (!dns.AccessOk(context))
       throw CIMAccessDeniedException("Access denied by DNSProvider");

    //-- make sure we're the right instance
    int keyCount;
    CIMName keyName;
    String keyValue;

    keyCount = MAX_KEYS;
    keys = instanceReference.getKeyBindings();

    if ((unsigned int)keys.size() != (unsigned int)keyCount)
        throw CIMInvalidParameterException("Wrong number of keys");

    // doesn't seem as though this code will handle duplicate keys,
    // but it appears as though the CIMOM strips those out for us.
    // Despite test cases, don't get invoked with 2 keys of the same
    // name.

    if(!dns.getSystemName(systemName))
        throw CIMOperationFailedException("DNSProvider "
                  "can't determine system name");

    if(!dns.getDNSName(dName))
        throw CIMOperationFailedException("DNSProvider "
                  "can't determine domain name");

    for (unsigned int ii = 0; ii < keys.size(); ii++) {
         keyName = keys[ii].getName();
         keyValue = keys[ii].getValue();

        if (keyName.equal (PROPERTY_CREATION_CLASS_NAME) &&
            (String::equalNoCase(keyValue, CLASS_NAME.getString()) ||
             keyValue.size() == 0))
            keyCount--;
        else if (keyName.equal (PROPERTY_NAME) &&
                 String::equalNoCase(keyValue, dName))
            keyCount--;
        else if (keyName.equal (PROPERTY_SYSTEM_CREATION_CLASS_NAME) &&
                 ((keyValue.size() == 0) || (String::equalNoCase(keyValue,
                         SYSTEM_CREATION_CLASS_NAME.getString()))))
            keyCount--;
        else if (keyName.equal (PROPERTY_SYSTEM_NAME) &&
                 ((keyValue.size() == 0) ||
                  (String::equalNoCase(keyValue, systemName))))
            keyCount--;

    }

    if (keyCount)
    {
        throw CIMInvalidParameterException("Wrong keys");
    }
    handler.processing();

    //-- fill 'er up...
    instance = _build_instance(
        className,
        instanceReference.getNameSpace(),
        instanceReference.getKeyBindings(),
        dns);
    handler.deliver(instance);
    handler.complete();
}

void DNSServiceProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
#ifdef DNSPROVIDER_DEBUG
    cout << "DNSServiceProvider::enumerateInstances()" << endl;
#endif

    CIMName className;
    CIMInstance instance;
    CIMObjectPath newref;

    className = classReference.getClassName();

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups
    if (className.equal (CLASS_PG_DNS_SERVICE))
    {
        handler.processing();

        DNSService dns;

        if (!dns.AccessOk(context))
            throw CIMAccessDeniedException("Access denied by DNSProvider");

        newref = _fill_reference(classReference.getNameSpace(), className, dns);
        instance = _build_instance(
            className,
            classReference.getNameSpace(),
            classReference.getKeyBindings(),
            dns);
        instance.setPath(newref);
        handler.deliver(instance);
        handler.complete();
    }
    else
    {
        throw CIMNotSupportedException("DNSServiceProvider "
                "does not support class " + className.getString());
    }
}

void DNSServiceProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    CIMObjectPath newref;
    CIMName className;

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups
    className = classReference.getClassName();
    if (!className.equal (CLASS_PG_DNS_SERVICE))
    {
        throw CIMNotSupportedException("DNSServiceProvider "
                           "does not support class " + className.getString());
    }

    DNSService dns;

    if (!dns.AccessOk(context))
       throw CIMAccessDeniedException("Access denied by DNSProvider");

    handler.processing();
    // in terms of the class we use, want to set to what was requested
    newref = _fill_reference(classReference.getNameSpace(), className, dns);
    handler.deliver(newref);
    handler.complete();
}

void DNSServiceProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    throw CIMNotSupportedException("DNSServiceProvider "
                       "does not support modifyInstance");
}

void DNSServiceProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    throw CIMNotSupportedException("DNSServiceProvider "
                       "does not support createInstance");
}

void DNSServiceProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    throw CIMNotSupportedException("DNSServiceProvider "
                       "does not support deleteInstance");
}

void DNSServiceProvider::initialize(CIMOMHandle & cimom)
{
}

void DNSServiceProvider::terminate()
{
    delete this;
}

/***********************************************************************
 Create properties to provider
***********************************************************************/

CIMInstance DNSServiceProvider::_build_instance(
    const CIMName & className,
    const CIMNamespaceName & nameSpace,
    const Array<CIMKeyBinding> keys,
    DNSService dns)
{
#ifdef DNSPROVIDER_DEBUG
    cout << "DNSServiceProvider::_build_instance()" << endl;
#endif

    CIMInstance instance(className);
    String strValue;
    Array<String> strArr;

    if (dns.getSystemName(strValue))
    {
        instance.setPath(CIMObjectPath(strValue, nameSpace, className, keys));
        instance.addProperty(CIMProperty(PROPERTY_SYSTEM_NAME, strValue));
    }
    else
        throw CIMOperationFailedException(
            "DNSProvider can't determine System Name property");

#ifdef DNSPROVIDER_DEBUG
    cout << "DNSServiceProvider::_build_instance() SystemName = `" <<
        strValue << "'" << endl;
#endif

    if (dns.getDNSName(strValue))
        instance.addProperty(CIMProperty(PROPERTY_NAME, strValue));
    else
        throw CIMOperationFailedException("DNSProvider "
                  "can't determine Name property");

#ifdef DNSPROVIDER_DEBUG
    cout << "DNSServiceProvider::_build_instance() Name = `" <<
        strValue << "'" << endl;
#endif

    instance.addProperty(CIMProperty(PROPERTY_CREATION_CLASS_NAME,
                                     CLASS_PG_DNS_SERVICE));

    instance.addProperty(CIMProperty(PROPERTY_SYSTEM_CREATION_CLASS_NAME,
                                     CLASS_CIM_UNITARY_COMPUTER_SYSTEM));

    if (dns.getCaption(strValue))
        instance.addProperty(CIMProperty(PROPERTY_CAPTION, strValue));

    if (dns.getDescription(strValue))
        instance.addProperty(CIMProperty(PROPERTY_DESCRIPTION, strValue));

    if (dns.getSearchList(strArr))
        instance.addProperty(CIMProperty(PROPERTY_SEARCH_LIST, strArr));

    if (dns.getAddresses(strArr))
        instance.addProperty(CIMProperty(PROPERTY_ADDRESSES, strArr));

#ifdef DNSPROVIDER_DEBUG
    cout << "DNSServiceProvider::_build_instance() - done" << endl;
#endif

    return instance;
}

/***********************************************************************
    Return CIMObjectPath instance of class valid keys
***********************************************************************/

CIMObjectPath DNSServiceProvider::_fill_reference(
    const CIMNamespaceName &nameSpace,
    const CIMName &className,
    DNSService dns)
{
#ifdef DNSPROVIDER_DEBUG
    cout << "DNSServiceProvider::_fill_reference()" << endl;
#endif

    Array<CIMKeyBinding> keys;
    String strValue;

    keys.append(CIMKeyBinding(
        PROPERTY_SYSTEM_CREATION_CLASS_NAME,
        CLASS_CIM_UNITARY_COMPUTER_SYSTEM,
        CIMKeyBinding::STRING));

    keys.append(CIMKeyBinding(
        PROPERTY_CREATION_CLASS_NAME,
        CLASS_PG_DNS_SERVICE,
        CIMKeyBinding::STRING));

    if (dns.getDNSName(strValue))
        keys.append(
            CIMKeyBinding(PROPERTY_NAME, strValue, CIMKeyBinding::STRING));
    else
        throw CIMOperationFailedException("DNSProvider "
                  "can't determine Name property");

    if (!dns.getSystemName(strValue))
        throw CIMOperationFailedException(
            "DNSProvider can't determine System Name property");

    keys.append(CIMKeyBinding(
        PROPERTY_SYSTEM_NAME,
        strValue,
        CIMKeyBinding::STRING));

    return CIMObjectPath(strValue, nameSpace, className, keys);
}
