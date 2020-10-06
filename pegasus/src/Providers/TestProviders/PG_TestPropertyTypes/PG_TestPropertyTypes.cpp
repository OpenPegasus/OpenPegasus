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
#include <Pegasus/Provider/CIMInstanceProvider.h>

#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMPropertyList.h>

#include "PG_TestPropertyTypes.h"

PEGASUS_NAMESPACE_BEGIN

extern "C" PEGASUS_EXPORT CIMProvider* PegasusCreateProvider(
    const String& providerName)
{
    const String PROVIDER_NAME = "PG_TestPropertyTypes";

    if (String::equalNoCase(providerName, PROVIDER_NAME))
    {
        return new PG_TestPropertyTypes();
    }

    return 0;
}


// Search for path in array and return array index (or PEG_NOT_FOUND)
static Uint32 findObjectPath(
    const Array<CIMObjectPath>& array,
    const CIMObjectPath& path)
{
    for (Uint32 i = 0; i < array.size(); i++)
    {
        if (array[i] == path)
            return i;
    }

    return PEG_NOT_FOUND;
}


PG_TestPropertyTypes::PG_TestPropertyTypes()
{
}

PG_TestPropertyTypes::~PG_TestPropertyTypes()
{
}

void PG_TestPropertyTypes::initialize(CIMOMHandle& cimom)
{
    // save cimom handle
    _cimom = cimom;

    // create default instances
    CIMInstance instance1("PG_TestPropertyTypes");

    instance1.addProperty(CIMProperty(
        "CreationClassName", String("PG_TestPropertyTypes")));   // key
    instance1.addProperty(CIMProperty("InstanceId", Uint64(1))); //key
    instance1.addProperty(CIMProperty(
        "PropertyString", String("PG_TestPropertyTypes_Instance1")));
    instance1.addProperty(CIMProperty("PropertyUint8", Uint8(120)));
    instance1.addProperty(CIMProperty("PropertyUint16", Uint16(1600)));
    instance1.addProperty(CIMProperty("PropertyUint32", Uint32(3200)));
    instance1.addProperty(CIMProperty("PropertyUint64", Uint64(6400)));
    instance1.addProperty(CIMProperty("PropertySint8", Sint8(-120)));
    instance1.addProperty(CIMProperty("PropertySint16", Sint16(-1600)));
    instance1.addProperty(CIMProperty("PropertySint32", Sint32(-3200)));
    instance1.addProperty(CIMProperty("PropertySint64", Sint64(-6400)));
    instance1.addProperty(CIMProperty("PropertyBoolean", Boolean(1)));
    instance1.addProperty(CIMProperty("PropertyReal32", Real32(1.12345670123)));
    instance1.addProperty(CIMProperty(
        "PropertyReal64", Real64(1.12345678906543210123)));
    instance1.addProperty(CIMProperty(
        "PropertyDatetime", CIMDateTime("20010515104354.000000:000")));

    // update object path
    {
        CIMObjectPath objectPath = instance1.getPath();

        Array<CIMKeyBinding> keys;

        {
            CIMProperty keyProperty = instance1.getProperty(
                instance1.findProperty("CreationClassName"));

            keys.append(
                CIMKeyBinding(keyProperty.getName(), keyProperty.getValue()));
        }

        {
            CIMProperty keyProperty = instance1.getProperty(
                instance1.findProperty("InstanceId"));

            keys.append(
                CIMKeyBinding(keyProperty.getName(), keyProperty.getValue()));
        }

        objectPath.setKeyBindings(keys);

        instance1.setPath(objectPath);
    }

    _instances.append(instance1);

    CIMInstance instance2("PG_TestPropertyTypes");

    instance2.addProperty(CIMProperty(
        "CreationClassName", String("PG_TestPropertyTypes")));   // key
    instance2.addProperty(CIMProperty("InstanceId", Uint64(2))); //key
    instance2.addProperty(CIMProperty(
        "PropertyString", String("PG_TestPropertyTypes_Instance2")));

    instance2.addProperty(CIMProperty("PropertyUint8", Uint8(122)));
    instance2.addProperty(CIMProperty("PropertyUint16", Uint16(1602)));
    instance2.addProperty(CIMProperty("PropertyUint32", Uint32(3202)));
    instance2.addProperty(CIMProperty("PropertyUint64", Uint64(6402)));
    instance2.addProperty(CIMProperty("PropertySint8", Sint8(-122)));
    instance2.addProperty(CIMProperty("PropertySint16", Sint16(-1602)));
    instance2.addProperty(CIMProperty("PropertySint32", Sint32(-3202)));
    instance2.addProperty(CIMProperty("PropertySint64", Sint64(-6402)));
    instance2.addProperty(CIMProperty("PropertyBoolean", Boolean(0)));
    instance2.addProperty(CIMProperty("PropertyReal32", Real32(2.12345670123)));
    instance2.addProperty(CIMProperty(
        "PropertyReal64", Real64(2.12345678906543210123)));

    instance2.addProperty(CIMProperty(
        "PropertyDatetime", CIMDateTime("20010515104354.000000:000")));

    _instances.append(instance2);

    // update object path
    {
        CIMObjectPath objectPath = instance2.getPath();

        Array<CIMKeyBinding> keys;

        {
            CIMProperty keyProperty = instance2.getProperty(
                instance2.findProperty("CreationClassName"));

            keys.append(
                CIMKeyBinding(keyProperty.getName(), keyProperty.getValue()));
        }

        {
            CIMProperty keyProperty = instance2.getProperty(
                instance2.findProperty("InstanceId"));

            keys.append(
                CIMKeyBinding(keyProperty.getName(), keyProperty.getValue()));
        }

        objectPath.setKeyBindings(keys);

        instance2.setPath(objectPath);
    }

    // instance3 to check for negative floating point and exponents
    CIMInstance instance3("PG_TestPropertyTypes");

    instance3.addProperty(CIMProperty(
        "CreationClassName", String("PG_TestPropertyTypes")));   // key
    instance3.addProperty(CIMProperty("InstanceId", Uint64(3))); //key
    instance3.addProperty(CIMProperty(
        "PropertyString", String("PG_TestPropertyTypes_Instance3")));
    instance3.addProperty(CIMProperty("PropertyUint8", Uint8(120)));
    instance3.addProperty(CIMProperty("PropertyUint16", Uint16(1600)));
    instance3.addProperty(CIMProperty("PropertyUint32", Uint32(3200)));
    instance3.addProperty(CIMProperty("PropertyUint64", Uint64(6400)));
    instance3.addProperty(CIMProperty("PropertySint8", Sint8(-120)));
    instance3.addProperty(CIMProperty("PropertySint16", Sint16(-1600)));
    instance3.addProperty(CIMProperty("PropertySint32", Sint32(-3200)));
    instance3.addProperty(CIMProperty("PropertySint64", Sint64(-6400)));
    instance3.addProperty(CIMProperty("PropertyBoolean", Boolean(0)));
    instance3.addProperty(CIMProperty(
        "PropertyReal32", Real32(-1.12345670123)));
    instance3.addProperty(CIMProperty(
        "PropertyReal64", Real64(0.000000012345)));
    instance3.addProperty(CIMProperty(
        "PropertyDatetime", CIMDateTime("20060301104354.000000:000")));

    // update object path
    {
        CIMObjectPath objectPath = instance3.getPath();

        Array<CIMKeyBinding> keys;

        {
            CIMProperty keyProperty = instance3.getProperty(
                instance3.findProperty("CreationClassName"));

            keys.append(
                CIMKeyBinding(keyProperty.getName(), keyProperty.getValue()));
        }

        {
            CIMProperty keyProperty =
                instance3.getProperty(instance3.findProperty("InstanceId"));

            keys.append(
                CIMKeyBinding(keyProperty.getName(), keyProperty.getValue()));
        }

        objectPath.setKeyBindings(keys);

        instance3.setPath(objectPath);
    }
    realValueTestInstance = instance3;
}

void PG_TestPropertyTypes::terminate()
{
    delete this;
}

void PG_TestPropertyTypes::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    // synchronously get references
    //Get extra instance created for testing negative realValues.
    Array<CIMObjectPath> references =
        _enumerateInstanceNames(context, instanceReference);

    // ensure the InstanceId key is valid
    Array<CIMKeyBinding> keys = instanceReference.getKeyBindings();
    Uint32 i;
    for (i=0; i<keys.size() && !keys[i].getName().equal("InstanceId"); i++)
    {
    }

    if (i==keys.size())
    {
        throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }

    // ensure the Namespace is valid
    if (!instanceReference.getNameSpace().equal ("test/static"))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // ensure the class existing in the specified namespace
    if (!instanceReference.getClassName().equal ("PG_TestPropertyTypes"))
    {
        throw CIMException(CIM_ERR_INVALID_CLASS);
    }
        Boolean testRealValue = false;
    // ensure the request object exists
    Uint32 index = findObjectPath(references, instanceReference);
    if (index == PEG_NOT_FOUND)
    {
        CIMClass cimclass = _cimom.getClass(
            context,
            instanceReference.getNameSpace(),
            instanceReference.getClassName(),
            false,
            true,
            true,
            CIMPropertyList());
        CIMObjectPath tempRef = realValueTestInstance.buildPath(cimclass);
        tempRef.setHost(instanceReference.getHost());
        tempRef.setNameSpace(instanceReference.getNameSpace());
        if (instanceReference != tempRef)
        {
            throw CIMException(CIM_ERR_NOT_FOUND);
        }
        testRealValue = true;
    }

    // begin processing the request
    handler.processing();
    if (testRealValue)
    {
        handler.deliver(realValueTestInstance);
    }
    else
    {
        // instance index corresponds to reference index
        handler.deliver(_instances[index]);
    }
    // complete processing the request
    handler.complete();
}

void PG_TestPropertyTypes::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{

    // ensure the Namespace is valid
    if (!ref.getNameSpace().equal ("test/static"))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // ensure the class existing in the specified namespace
    if (!ref.getClassName().equal ("PG_TestPropertyTypes"))
    {
        throw CIMException(CIM_ERR_INVALID_CLASS);
    }

    // begin processing the request
    handler.processing();

    // NOTE: It would be much more efficient to remember the instance names

    // get class definition from repository
    CIMClass cimclass = _cimom.getClass(
        context,
        ref.getNameSpace(),
        ref.getClassName(),
        false,
        true,
        true,
        CIMPropertyList());

    for (Uint32 i = 0; i < _instances.size(); i++)
    {
        handler.deliver(_instances[i]);
    }

    // complete processing the request
    handler.complete();
}

void PG_TestPropertyTypes::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{

    // ensure the Namespace is valid
    if (!classReference.getNameSpace().equal ("test/static"))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // ensure the class existing in the specified namespace
    if (!classReference.getClassName().equal ("PG_TestPropertyTypes"))
    {
        throw CIMException(CIM_ERR_INVALID_CLASS);
    }

    // begin processing the request
    handler.processing();

    Array<CIMObjectPath> instanceNames;
    instanceNames = _enumerateInstanceNames(context, classReference);

    handler.deliver(instanceNames);

    // complete processing the request
    handler.complete();
}

void PG_TestPropertyTypes::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    // This provider only allows partial instance modification for the
    // PropertyUint8 property.
    if (!(propertyList.isNull() ||
          ((propertyList.size() == 1) &&
           (propertyList[0].equal("PropertyUint8")))))
    {
        throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }

    // synchronously get references
    Array<CIMObjectPath> references =
        _enumerateInstanceNames(context, instanceReference);

    // ensure the Namespace is valid
    if (!instanceReference.getNameSpace().equal("test/static"))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // ensure the class existing in the specified namespace
    if (!instanceReference.getClassName().equal("PG_TestPropertyTypes"))
    {
        throw CIMException(CIM_ERR_INVALID_CLASS);
    }

    // ensure the property values are valid
    _testPropertyTypesValue(instanceObject);

    // ensure the request object exists
    Uint32 index = findObjectPath(references, instanceReference);
    if (index == PEG_NOT_FOUND)
    {
        throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // begin processing the request
    handler.processing();

    // We do nothing here since we like to have static result
    // complete processing the request
    handler.complete();
}

void PG_TestPropertyTypes::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    // synchronously get references
    Array<CIMObjectPath> references =
        _enumerateInstanceNames(context, instanceReference);

    // ensure the Namespace is valid

    if (!instanceReference.getNameSpace().equal("test/static"))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // ensure the class existing in the specified namespace
    if (!instanceReference.getClassName().equal("PG_TestPropertyTypes"))
    {
        throw CIMException(CIM_ERR_INVALID_CLASS);
    }

    // ensure the InstanceId key is valid
    Uint32 propIndex = instanceObject.findProperty("InstanceId");
    if (propIndex == PEG_NOT_FOUND)
    {
        throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }

//    Uint32 i;
//    for (i=0; i<keys.size() && keys[i].getName() != "InstanceId"; i++);

//    if (i==keys.size())
//    {
//        throw CIMException(CIM_ERR_INVALID_PARAMETER);
//    }

    // create the CIMObjectPath to return
    Array<CIMKeyBinding> keys;
    keys.append(CIMKeyBinding(
        instanceObject.getProperty(propIndex).getName(),
        instanceObject.getProperty(propIndex).getValue().toString(),
        CIMKeyBinding::NUMERIC));
    keys.append(CIMKeyBinding(
        "CreationClassName",
        "PG_TestPropertyTypes",
        CIMKeyBinding::STRING));
    CIMObjectPath returnReference(
        String::EMPTY,
        CIMNamespaceName(),
        instanceReference.getClassName(),
        keys);

    // ensure the property values are valid
    _testPropertyTypesValue(instanceObject);

    // Determine if a property exists in the class
    if (instanceObject.findProperty("PropertyUint8") == PEG_NOT_FOUND)
    {
        throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }

    // ensure the requested object do not exist
    if (findObjectPath(references, instanceReference) != PEG_NOT_FOUND)
    {
        throw CIMException(CIM_ERR_ALREADY_EXISTS);
    }

    // begin processing the request
    handler.processing();

    handler.deliver(returnReference);

    // complete processing request
    handler.complete();
}

void PG_TestPropertyTypes::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    // synchronously get references
    Array<CIMObjectPath> references =
        _enumerateInstanceNames(context, instanceReference);

    // ensure the Namespace is valid
    if (!instanceReference.getNameSpace().equal("test/static"))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // ensure the class existing in the specified namespace
    if (!instanceReference.getClassName().equal("PG_TestPropertyTypes"))
    {
        throw CIMException(CIM_ERR_INVALID_CLASS);
    }

    // ensure the requested object exists
    Uint32 index = findObjectPath(references, instanceReference);
    if (index == PEG_NOT_FOUND)
    {
        throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // begin processing the request
    handler.processing();

    // we do not remove instance
    // complete processing the request
    handler.complete();
}

Array<CIMObjectPath> PG_TestPropertyTypes::_enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference)
{
    Array<CIMObjectPath> instanceNames;

    // get class definition from repository
    CIMClass cimclass = _cimom.getClass(
        context,
        classReference.getNameSpace(),
        classReference.getClassName(),
        false,
        true,
        true,
        CIMPropertyList());

    // convert instances to references;
    for (Uint32 i = 0; i < _instances.size(); i++)
    {
        CIMObjectPath tempRef = _instances[i].buildPath(cimclass);

        // ensure references are fully qualified
        tempRef.setHost(classReference.getHost());
        tempRef.setNameSpace(classReference.getNameSpace());

        instanceNames.append(tempRef);
    }

    return instanceNames;
}

// private method: _testPropertyTypesValue
void PG_TestPropertyTypes::_testPropertyTypesValue(
    const CIMInstance& instanceObject)
{
    Uint32 PropertyCount = instanceObject.getPropertyCount();

    Uint32 j;
    for (j = 0; j < PropertyCount; j++)
    {
        const CIMConstProperty& property = instanceObject.getProperty(j);
        const CIMValue& propertyValue = property.getValue();
        CIMType type = propertyValue.getType();

        switch (type)
        {
        case CIMTYPE_UINT8:
            Uint8 propertyValueUint8;
            propertyValue.get(propertyValueUint8);
            if (propertyValueUint8 >= 255)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

        case CIMTYPE_UINT16:
            Uint16 propertyValueUint16;
            propertyValue.get(propertyValueUint16);
            if (propertyValueUint16 >= 10000)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

        case CIMTYPE_UINT32:
            Uint32 propertyValueUint32;
            propertyValue.get(propertyValueUint32);
            if (propertyValueUint32 >= 10000000)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

        case CIMTYPE_UINT64:
            Uint64 propertyValueUint64;
            propertyValue.get(propertyValueUint64);
            if (propertyValueUint64 >= 1000000000)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

        case CIMTYPE_SINT8:
            Sint8 propertyValueSint8;
            propertyValue.get(propertyValueSint8);
            if (propertyValueSint8 <= (Sint8)-120)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

        case CIMTYPE_SINT16:
            Sint16 propertyValueSint16;
            propertyValue.get(propertyValueSint16);
            if (propertyValueSint16 < -10000)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

        case CIMTYPE_SINT32:
            Sint32 propertyValueSint32;
            propertyValue.get(propertyValueSint32);
            if (propertyValueSint32 <= -10000000)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

        case CIMTYPE_SINT64:
            Sint64 propertyValueSint64;
            propertyValue.get(propertyValueSint64);
            if (propertyValueSint64 <= -1000000000)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

         case CIMTYPE_REAL32:
            Real32 propertyValueReal32;
            propertyValue.get(propertyValueReal32);
            if (propertyValueReal32 >= 10000000.32)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

         case CIMTYPE_REAL64:
            Real64 propertyValueReal64;
            propertyValue.get(propertyValueReal64);
            if (propertyValueReal64 >= 1000000000.64)
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
            }
            break;

         default:
             ;
         }
    } // end for loop
}

PEGASUS_NAMESPACE_END
