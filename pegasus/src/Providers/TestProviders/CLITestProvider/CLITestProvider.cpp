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

/*
Test Provider for cimcli. This provider is intended to be used as a
test driver only for cimcli.  It provides implementation of classes and
operations that allow cimcli to exercise all of it operations and options
with known results.

This provider is based on the Test_CLITestProviderClass class the
Test_CLITestProviderLinkClass and the Test_CLITestEmbeddedClass.
The Test_CLITestProviderClass class includes all of the CIM Data types in both
scalar and array form. The Test_CLITestProviderLinkClass allows testing
association operations and the TestCLITestEmbeddedClass allows testing
operations involving embedded instances.

The provider handles the following CIM operations:

1. Initialize - Creates a single instance of the class with all of the
properties initialized. this is placed in an array.

2. EnumerateInstances - Returns all instances in the instances array

3. GetInstance - Returns the single instance found in the array with the
input object path if found

4. Create instance - Creates a new instance of any of the classes supported
by the provider and puts it into the a local cache (array if instances).

5. ModifyInstance - Attempts to modify an instance of the target class if one
is found in the instances array. Supports all of the classes defined for
the provider.

6. DeleteInstance - Delete an instance of the test class if found in the
instances array.

6. Invoke method: Includes several methods as follows:
   -InOutParamTest - Returns all input parameters on the method to the caller
    With return value of zero.
   -setProviderParameters - Single method to allow setting provider attributes
    parameters that modify the provider capabilities for testing.
    Parameters:
      substituteHostName - String parameter that provides an alternate name
      to be used as host name on all responses that include host name.
   -debugMode - method that enables or disables debug mode for
    the provider.  If the boolean parameter newState is set, the provider
    provider internal debug flag is set to that state. The return code

   -resetProviderParameters - resets all parameters set by the
      set the provider parameters and clears the cache restoring
      the provider to it original initialized state. This does not change
      the debug mode.
   -ReferenceParamTest - Generates a specific set of Out Parameters

The Test_CLITestProviderLinkClassProvides a means to test the reference and
associator operations.  Note that the instances do not really represent
understandable associations, simply syntatically correct associations.

The first operation after the provider is initialized builds a set of instances
that can be used as the basis for tests.  These instances may be deleted.

LIMITATIONS:
1. The provider is intended to be used in a single namespace and so
does not include the namespace in the instances placed in the local cache.
Therefore if it is enabled for multiple namespaces, a user from some
namespace could remove, get, or enumerate an instance created in another
namespace.

2. It does not check the validity of any association create or modify instance
requests received (i.e. the references can be to non-existent instances.

3. modifyInstance rejects if includeQualifier parameter == true.

4. createInstance is weak and does not completely validate instances input.
The user can add non-existent properties and change property types without
the provider figuring it out.

5. Provider uses a mixture of Provider exceptions and the general Server
exceptions (CIMException) rather than all provider exceptions.

It is intended to run a set of tests fairly rapidly so does not hold itself in
memory.  Therefore, if instances are put into the array after the normal
Pegasus timeout of providers for unload, they will be discarded.

*/

#include "CLITestProvider.h"
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Tracer.h>
// Required because of some malignent link betweeh MessageLoader and Thread.h
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMQualifierNames.h>
#include <Pegasus/Common/Print.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

String _toString(Boolean x)
{
    return((x)?"true" : "false");
}

static String _toString(const CIMPropertyList& pl)
{
    String s;

    if (pl.isNull())
    {
        s = "NULL";
    }
    else if (pl.size() == 0)
    {
        s = "EMPTY";
    }
    else
    {
        for (Uint32 i = 0; i < pl.size(); i++)
        {
            if (i > 0)
            {
                s.append(", ");
            }
            s.append(pl[i].getString());
        }
    }
    return s;
}

/*
    Add the name value pair to the String target.  The result is the pair
    added to target in the form
    [; ]name "=" value
*/
void _addParam(String& target, const String& name, const String& value)
{
    if (target.size() != 0)
    {
        target.append("; ");
    }
    target.append(name);
    target.append("=");
    target.append(value);
}

/*
    Complete the host and namespace fields of an object path if there are
    empty.
*/
void _completePath(const String& hostName,
                            const CIMNamespaceName& nameSpace,
                            CIMObjectPath& objectPath)
{
    if (objectPath.getHost().size() == 0)
        objectPath.setHost(hostName);

    if (objectPath.getNameSpace().isNull())
        objectPath.setNameSpace(nameSpace);
}

// convert a fully qualified reference into a local reference
// (class name and keys only).
CIMObjectPath _localPath(const CIMObjectPath& inputPath)
{
    CIMObjectPath localPath = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        inputPath.getClassName(),
        inputPath.getKeyBindings());
    return localPath;
}

// Gets a host name, either the real host name or a substitute for testing.
// The substitute name  option allows CLI test program to get a dependable
// host name for result comparisons.
String CLITestProvider::_getHostName()
{
    if (!_useSubstituteHostName)
    {
        return System::getHostName();
    }
    else
    {
        return _substituteHostName;
    }
}

// Serializes access to the instances array during the CIM requests
static Mutex instanceArrayMutex;

CLITestProvider::CLITestProvider()
{
    _initialized = false;
    _debugMode = false;
    _useSubstituteHostName = false;
    _propertyList = CIMPropertyList();
}

CLITestProvider::~CLITestProvider()
{
}

void CLITestProvider::initialize(CIMOMHandle & cimom)
{
    _cimom = cimom;
}

void CLITestProvider::terminate()
{
    _instances.clear();
    _classes.clear();
    delete this;
}

void CLITestProvider::invokeMethod(
    const OperationContext & context,
    const CIMObjectPath & objectReference,
    const CIMName & methodName,
    const Array<CIMParamValue> & inParameters,
    MethodResultResponseHandler & handler)
{
    if (_debugMode)
    {
        cout << "invokeMethod Method Name = " << methodName.getString()
             << " Input parameters = ";
        for (Uint32 i = 0 ; i < inParameters.size(); i++)
        {
            CIMParamValue param = inParameters[i];
            CIMValue paramVal = param.getValue();
            String paramName = param.getParameterName();
            cout << paramName << " ";
        }
        cout << endl;
    }

    initializeProvider(objectReference.getNameSpace());

    // convert a fully qualified reference into a local reference
    // (class name and keys only).

    CIMObjectPath localReference = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectReference.getClassName(),
        objectReference.getKeyBindings());

    handler.processing();

    String outString = "CLITestProvider Tests : ";

    if (objectReference.getClassName().equal("Test_CLITestProviderClass"))
    {
        if (methodName.equal("ReferenceParamTest"))
        {
            if (inParameters.size() > 0)
            {
                for(Uint32 i = 0; i < inParameters.size(); ++i)
                {
                    CIMParamValue param = inParameters[i];
                    CIMValue paramVal = param.getValue();
                    String paramName = param.getParameterName();
                    String outParamName;
                    if(String::equalNoCase(paramName,"InParam1"))
                    {
                        outParamName = "OutParam1";
                        PEGASUS_TEST_ASSERT(
                            paramVal.getType() == CIMTYPE_REFERENCE
                            && !paramVal.isArray());

                        param.setParameterName(outParamName);
                        handler.deliverParamValue(param);
                    }
                    else if(String::equalNoCase(paramName,"InParam2"))
                    {
                        outParamName = "OutParam2";
                        PEGASUS_TEST_ASSERT(
                            paramVal.getType() == CIMTYPE_REFERENCE
                            && paramVal.isArray());

                        param.setParameterName(outParamName);
                        handler.deliverParamValue(param);
                    }
                }

                handler.deliver(CIMValue(outString));
            }
            else
            {
                outString.append("Empty Parameters");
                handler.deliver(CIMValue(Uint32(0xFFffFFff)));
            }

            handler.deliver(Uint32(0));
        }

        // This simply returns all parameters and
        // sets return value set to zero. This should provide a complete
        // test of all input and output parameter types for cimcli
        else if(methodName.equal("InOutParamTest"))
        {
            if (inParameters.size() > 0)
            {
                //Returns all input parameters
                handler.deliverParamValue(inParameters);
            }
            handler.deliver(Uint32(0));
        }

        // This simply returns all parameters and
        // sets return value set to zero. This should provide a complete
        // test of all input and output parameter types for cimcli
        else if(methodName.equal("setProviderParameters"))
        {
            Uint32 rtnCode = 0;
            if (inParameters.size() > 0)
            {
                for(Uint32 i = 0; i < inParameters.size(); ++i)
                {
                    String paramName = inParameters[i].getParameterName();
                    CIMValue paramVal = inParameters[i].getValue();

                    if (paramName == "substituteHostName")
                    {
                        if (paramVal.getType() == CIMTYPE_STRING)
                        {
                            String sTmp = _substituteHostName;
                            // set the capability from the input parameter
                            paramVal.get(_substituteHostName);
                            _useSubstituteHostName = true;

                            // return the parameter with the original value
                            CIMParamValue outParam = inParameters[i].clone();
                            CIMValue v = outParam.getValue();

                            v.set(sTmp);
                            outParam.setValue(v);

                            handler.deliverParamValue(outParam);
                        }
                        else
                        {
                            String errMsg = "Incorrect in parameter type ";
                            errMsg.append(paramName);
                            errMsg.append( " for method ");
                            errMsg.append(methodName.getString());
                            throw CIMOperationFailedException(errMsg);
                        }
                    }
                    // NOTE: Add new parameters here with else if
                    // not a valid parameter. Return error
                    else
                    {
                            String errMsg = "Incorrect in parameter ";
                            errMsg.append(paramName);
                            errMsg.append( " for method ");
                            errMsg.append(methodName.getString());
                            throw CIMOperationFailedException(errMsg);
                    }
                }
            }
            handler.deliver(rtnCode);
        }

        // Reset the parameters used to modify operations.  Note that
        // this does not reset the debugMode parameter
        else if(methodName.equal("resetProviderParameters"))
        {
            _useSubstituteHostName = false;
            _substituteHostName = String();
            handler.deliver(0);
        }

        // Restore provider to its post initialized state.  remvoes
        // any instances in cache, sets cache to initialized instances,
        // and sets parameters to initial state. Does not reset the
        // debugMode parameter.
        else if (methodName.equal("reset"))
        {
            // If the provider has been initialized (at least - one operation
            // call clear and reinit the instance repository.
            if (_initialized)
            {
                AutoMutex autoMut(instanceArrayMutex);
                _instances.clear();
                _initialized = false;
            }
            _useSubstituteHostName = false;
            _substituteHostName = String();
            handler.deliver(0);
        }
        else if (methodName.equal("debugMode"))
        {
            Boolean oldMode = _debugMode;
            Uint32 rtnCode = 0;
            if (inParameters.size() > 0)
            {
                for(Uint32 i = 0; i < inParameters.size(); ++i)
                {
                    String paramName = inParameters[i].getParameterName();
                    CIMValue paramVal = inParameters[i].getValue();

                    if (paramName == "newState")
                    {
                        if (paramVal.getType() == CIMTYPE_BOOLEAN)
                        {
                            paramVal.get(_debugMode);
                        }
                        else
                        {
                            // return error. Incorrect type on parameter
                            String errMsg = "Incorrect in parameter type ";
                            errMsg.append(paramName);
                            errMsg.append( " for method ");
                            errMsg.append(methodName.getString());

                            throw CIMOperationFailedException(errMsg);
                        }
                    }
                    // invalid parameter for this method. Throw error
                    else
                    {
                        String errMsg = "Incorrect in parameter ";
                        errMsg.append(paramName);
                        errMsg.append( " for method ");
                        errMsg.append(methodName.getString());

                        throw CIMOperationFailedException(errMsg);
                    }
                }
            }
            if (rtnCode == 0)
            {
                rtnCode = (oldMode != _debugMode)? 0: 1;
            }
            handler.deliver(rtnCode);
        }

        else
        {
            String errMsg = "Incorrect method name ";
            errMsg.append(methodName.getString());

            throw CIMOperationFailedException(errMsg);
        }
    }

    else if (objectReference.getClassName().equal("Test_CLITestEmbeddedClass"))
    {
        if (methodName.equal("embeddedInstParamMethod"))
        {
            if (inParameters.size() > 0)
            {
                //Returns all input parameters
                handler.deliverParamValue(inParameters);
            }
            handler.deliver(Uint32(0));
        }
        else
        {
            cout << "Invalid method name= "
                << methodName.getString()
                << " ignored" << endl;
        }
    }
    else
    {
            String errMsg = "Incorrect className on invokeMethod ";
            errMsg.append(objectReference.getClassName().getString());

            throw CIMOperationFailedException(errMsg);
    }
    handler.complete();
}

void CLITestProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    if (_debugMode)
    {
        cout << "getInstance ref=" << instanceReference.toString() << endl;
    }
    initializeProvider(instanceReference.getNameSpace());

    handler.processing();

    AutoMutex autoMut(instanceArrayMutex);

    Uint32 index;
    if ((index = findInstance(instanceReference)) != PEG_NOT_FOUND)
    {
        // Put input parameters into the requestInputParameters property so
        // they can be tested on by the client.
        String text;
        _addParam(text, "propertyList", _toString(propertyList));
        _addParam(text, "includeQualifiers", _toString(includeQualifiers));
        _addParam(text, "includeClassOrigin", _toString(includeClassOrigin));

        try
        {
            CIMInstance temp = _instances[index].clone();
            // Required because not assured all instances will have this
            // property
            if (temp.findProperty("requestInputParameters") == PEG_NOT_FOUND)
            {
                CIMClass tmpClass = _getClass(CIMName(
                    "Test_CLITestProviderClass"),
                    instanceReference.getNameSpace());

                Uint32 pos = tmpClass.findProperty("requestInputParameters");

                temp.addProperty(tmpClass.getProperty(pos));
            }

            temp.getProperty(temp.findProperty("requestInputParameters"))
                .setValue(text);

            handler.deliver(temp);
        }
        catch(CIMException& e)
        {
            cerr << "CIMCLITestProvider: Exception Occured on deliver : "
                << e.getMessage() << endl;
            throw CIMException(e);
        }
    }
    else
    {
        throw CIMException(CIM_ERR_NOT_FOUND);
    }

    handler.complete();
}

void CLITestProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    if (_debugMode)
    {
        cout << "Enter EnumerateInstances " << ref.toString()
             << " includeQualifiers= " << (includeQualifiers? "true":"false")
             << " includeClassOrigin= " << (includeClassOrigin? "true":"false")
             << " propertyList= " << propertyList.toString() << endl;
    }
    initializeProvider(ref.getNameSpace());

    handler.processing();

    AutoMutex autoMut(instanceArrayMutex);

    CIMName reqClassName = ref.getClassName();

    // Puts input parameters into the requestInputParameters property so
    // they can be tested on by the client.
    String text;
    _addParam(text, "propertyList", _toString(propertyList));
    _addParam(text, "includeQualifiers", _toString(includeQualifiers));
    _addParam(text, "includeClassOrigin", _toString(includeClassOrigin));

    // Loop through cache for all instances with defined className
    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if (reqClassName == _instances[i].getClassName())
        {
            try
            {
                CIMInstance temp = _instances[i].clone();
                if (temp.findProperty(
                    "requestInputParameters") == PEG_NOT_FOUND)
                {
                    CIMClass tmpClass = _getClass(CIMName(
                        "Test_CLITestProviderClass"),
                        ref.getNameSpace());

                    Uint32 pos = tmpClass.findProperty(
                        "requestInputParameters");

                    temp.addProperty(tmpClass.getProperty(pos));
                }

                temp.getProperty(
                    temp.findProperty("requestInputParameters"))
                    .setValue(text);

                handler.deliver(temp);
            }
            catch(CIMException& e)
            {
                cerr << "CIMCLITestProvider:Exception Occured : "
                    << e.getMessage() << endl;
                throw CIMException(e);
            }
        }
    }
    handler.complete();
}

void CLITestProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    if (_debugMode)
    {
        cout << "Enter EnumerateInstanceNames "
             << classReference.toString() << endl;
    }
    initializeProvider(classReference.getNameSpace());

    handler.processing();

    AutoMutex autoMut(instanceArrayMutex);

    CIMName reqClassName = classReference.getClassName();

    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if (reqClassName == _instances[i].getClassName())
        {
            try
            {
                handler.deliver(_instances[i].getPath());
            }
            catch(CIMException& e)
            {
                cerr << "CIMCLITestProvider:Exception Occured : "
                    << e.getMessage() << endl;
                throw CIMException(e);
            }
        }
    }
    handler.complete();
}

// modifyInstance - Helper function for modifyInstance operation.
// Modify the property in existingInst with the value of modifiedProperty
// if they have different values. If modifiedProperty does not exist in
// existingInstance, add it from propertyInClass.

void modifyProperty(CIMInstance& existingInst,
    CIMConstProperty& modifiedProperty,
    CIMConstProperty& propertyInClass)
{
    CIMName pName = modifiedProperty.getName();
    try
    {
        Uint32 pos;
        if ((pos = existingInst.findProperty(pName)) != PEG_NOT_FOUND)
        {
            // If values are different, move value to existing property
            CIMProperty existingProp = existingInst.getProperty(pos);
            CIMValue vExisting = existingProp.getValue();
            if (vExisting != modifiedProperty.getValue())
            {
                existingProp.setValue(modifiedProperty.getValue());
            }
        }
        else   // add the property since not in instance
        {
            // Get property from Class
            CIMProperty addProp = propertyInClass.clone();

            // Do we have to do these tests if property NULL??
            if ((modifiedProperty.getValue().getType()
                != addProp.getValue().getType()) ||
                (modifiedProperty.getValue().isArray() !=
                addProp.getValue().isArray())
                )
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER,
                      "modifyInstance inputInstance Property Value type"
                      " mismatch: " +
                      pName.getString());
            }

            addProp.setValue(modifiedProperty.getValue());
            existingInst.addProperty(addProp);
        }
    }
    catch(CIMException& e)
    {
        throw CIMException(CIM_ERR_FAILED,
            " modify Instance " + e.getMessage());
    }
}

Uint32 testPropertyInClass(CIMName& name, CIMConstClass& mc)
{
    Uint32 pos;
    if ((pos = mc.findProperty(name)) ==  PEG_NOT_FOUND)
    {
        throw CIMException(CIM_ERR_INVALID_PARAMETER,
              "modifyInstance: Property"
              " Not in class: " +
             name.getString());
    }
    return pos;
}

void CLITestProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    if (_debugMode)
    {
        cout << "Enter modify Instance. instanceRef = "
            << instanceReference.toString()
            << " includeQualifiers = "
            << (includeQualifiers? "true" : "false")
            << " property List = " << _toString(propertyList) << endl;
        // This only displays instance in Pegasus debug mode.
        PrintInstance(cout, instanceObject);
    }

    initializeProvider(instanceReference.getNameSpace());

    handler.processing();

    // Reject includeQualifiers = true
    if (includeQualifiers)
    {
        throw CIMException(CIM_ERR_FAILED,
            " modifyInstance rejected by provider."
            " includeQualifiers=true not allowed");
    }

    // get the class for this operation.  Used several times in process.
    CIMConstClass mc = _getClass(instanceReference.getClassName(),
                            instanceReference.getNameSpace());

    // convert a fully qualified reference into a local reference
    // (class name and keys only).

    // Ref must be correct
    CIMObjectPath localRef = _localPath(instanceReference);

    AutoMutex autoMut(instanceArrayMutex);

    // Find the proper instance in the cache
    Uint32 index;
    if ((index = findInstance(localRef)) != PEG_NOT_FOUND)
    {
        // Clone the instance to be modified so we do not modify unless
        // completely successful.
        CIMInstance instToModify = _instances[index].clone();

        // If the PropertyList is NULL, the set of properties to be modified
        // consists of those of ModifiedInstance with values different from
        // the current values in the instance to be modified.
        if (propertyList.isNull())
        {
            // for all properties in the input instance
            Uint32 propertyCount = instanceObject.getPropertyCount();
            for (Uint32 j = 0 ;  j < propertyCount ; j++)
            {
                CIMConstProperty inputProperty = instanceObject.getProperty(j);
                CIMName pName = inputProperty.getName();

                // reject if input property not in class
                Uint32 pos = testPropertyInClass(pName, mc);

                CIMConstProperty propertyInClass = mc.getProperty(pos);

                modifyProperty(instToModify,
                    inputProperty,
                    propertyInClass);
            }  // for loop processing properties if propertyList isNull
        }  // end propertyList.isNull()

        else if (propertyList.size() != 0)
        {
            /*
            If the PropertyList input parameter is not NULL, the members of the
            array define one or more property names.  Only properties specified
            in the PropertyList are modified.  Properties of the
            ModifiedInstance that are missing from the PropertyList are
            ignored.

            If a property is not specified in the ModifiedInstance but is
            specified in the PropertyList, then the class-defined default
            value (or NULL if none is defined) becomes its new value in
            the instance to be modified.
            */
            for (Uint32 i = 0 ; i < propertyList.size() ; i++)
            {
                CIMName plName = propertyList[i];

                // test if property in propertyList is in class
                Uint32 posInClass = testPropertyInClass(plName, mc);

                // If property not found in input instance, get from the class
                // including value
                Uint32 pos;
                if ((pos = instanceObject.findProperty(plName)) ==
                     PEG_NOT_FOUND)
                {
                    CIMProperty addProp = mc.getProperty(posInClass).clone();
                    instToModify.addProperty(addProp);
                }

                else  // else modify existing property
                {
                    CIMConstProperty inputProperty =
                        instanceObject.getProperty(pos);

                    CIMConstProperty propertyInClass =
                         mc.getProperty(posInClass);

                    modifyProperty(instToModify,
                        inputProperty,
                        propertyInClass);
                }
            }  // for propertyList[] loop
        }   // end propertyList.size() != 0

        // if propertyList empty exit without updating existingInst
        else
        {
            handler.complete();
            return;
        }

        // Test for no modifications to key. This allows key properties
        // in instanceObject if they do not change the CIMValue.
        CIMObjectPath p = instToModify.buildPath(mc);
        if (p != localRef)
        {
            throw CIMException(CIM_ERR_INVALID_PARAMETER,
                  "modifyInstance input instance modifies keys.");
        }

        // Complete successful, replace the existing instance.
        _instances.remove(index);
        _instances.append(instToModify);

        handler.complete();
    } // end if found

    else // Instance not found in cache
    {
        throw CIMObjectNotFoundException(
            "Instance not found: " + instanceReference.toString());    }
}

// Determine if a property is a key property
static Boolean isKey(const CIMConstProperty& p)
{
    Uint32 index = p.findQualifier(CIMQualifierNames::KEY);

    if (index == PEG_NOT_FOUND)
    {
        return false;
    }

    Boolean flag;
    const CIMValue& value = p.getQualifier(index).getValue();

    if (value.getType() != CIMTYPE_BOOLEAN)
    {
        return false;
    }

    value.get(flag);
    return flag;
}

void CLITestProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    if (_debugMode)
    {
        cout << "Enter createInstance "
             << instanceReference.toString() << endl;
        // Only displays instance in Pegasus debug build
        PrintInstance(cout, instanceObject);
    }
    initializeProvider(instanceReference.getNameSpace());

    handler.processing();

    CIMObjectPath newInstanceRef = _localPath(instanceReference);

    AutoMutex autoMut(instanceArrayMutex);

    CIMClass c = _getClass(instanceObject.getClassName(),
        instanceReference.getNameSpace());

    // Test the instance against class for key Properties existence
    for (Uint32 i = 0 ; i < c.getPropertyCount() ; i++)
    {
        CIMConstProperty p = c.getProperty(i);
        if (isKey(p))
        {
            if (instanceObject.findProperty(p.getName()) == PEG_NOT_FOUND)
            {
                throw CIMOperationFailedException(
                    "Key Property(s) Required: " + p.getName().getString());
            }
        }
    }

    // Test to assure that all properties in instance are in the class
    for (Uint32 i = 0 ; i < instanceObject.getPropertyCount() ; i++)
    {
        CIMConstProperty p = instanceObject.getProperty(i);
        if (c.findProperty(p.getName()) == PEG_NOT_FOUND)
        {
            throw CIMPropertyNotFoundException(
                "Property " + p.getName().getString());
        }
        // To be completely valid we should also check type, etc.
    }

    // determine that key properties exist in the instance.  Do we
    // really need to do this or simply take whatever is there?

    newInstanceRef = instanceObject.buildPath(c);

    if (newInstanceRef.getKeyBindings().size() == 0)
    {
        throw CIMOperationFailedException("Key Property(s) Required");
    }

    // If the instance exists, throw already_exists exception

    Uint32 index;
    if ((index = findInstance(newInstanceRef)) == PEG_NOT_FOUND)
    {
        // add the instance to the set of instances saved in the cache.
        CIMInstance myInstance = instanceObject.clone();
        myInstance.setPath(newInstanceRef);
        _instances.append(myInstance);

        // Deliver path of new instance
        handler.deliver(newInstanceRef);
        handler.complete();
    }

    else
    {
        throw CIMException(CIM_ERR_ALREADY_EXISTS);
    }
}

void CLITestProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    if (_debugMode)
    {
        cout << "Enter deleteInstance "
             << instanceReference.toString() << endl;
    }
    initializeProvider(instanceReference.getNameSpace());

    handler.processing();

    // convert a fully qualified reference into a local reference
    // (class name and keys only).

    AutoMutex autoMut(instanceArrayMutex);

    Uint32 index;
    if ((index = findInstance(instanceReference)) != PEG_NOT_FOUND)
    {
        _instances.remove(index);
    }
    else
    {
        throw CIMObjectNotFoundException(
            "Instance not found: " + instanceReference.toString());
    }

    handler.complete();
    return;
}

/*
    Processing of associator/Reference Operation Requests

    NOTE: This association code is not based on any definition of the
    relationship between objects but simply returning information
    on instances that exist in the repository. Thus typically it returns
    the target instance itself (i.e. association of an instance with
    itself).  This works since the only goal of this provider
    is a syntatic test of cimcli, not any association structure. So our
    association is that every instance is associated with itself. Note that
    this removes any meaning from the role and assoc/result class parameters
    but we test the validity of these by returning a property in the
    returned instances containing all of these values so that the client
    can test to determine if the provider received what was input.
*/
void CLITestProvider::associators(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& associationClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    ObjectResponseHandler& handler)
{
    if (_debugMode)
    {
        cout << "Enter associators "
             << objectName.toString()
             << "associationClass " << associationClass.getString()
             << "resultClass " << resultClass.getString()
             << "role " << role
             << "resultRole " << resultRole << endl;
    }

    initializeProvider(objectName.getNameSpace());

    // Get the namespace and host names to create the CIMObjectPath
    CIMNamespaceName nameSpace = objectName.getNameSpace();
    String host = _getHostName();

    handler.processing();
    // complete processing the request
    // Puts input parameters into the requestInputParameters property so that
    // they can be tested by the client.
    String text;
    _addParam(text, "role", role);
    _addParam(text, "resultRole", resultRole);
    _addParam(text, "associationClass", associationClass.getString());
    _addParam(text, "resultClass", resultClass.getString());
    _addParam(text, "includeQualifiers", _toString(includeQualifiers));
    _addParam(text, "includeClassOrigin", _toString(includeClassOrigin));
    _addParam(text, "propertyList", _toString(propertyList));

    // Return an instance of the associated class for every instance
    // currently in the local list.
    Uint32 index;
    if ((index = findInstance(objectName)) != PEG_NOT_FOUND)
    {
        try
        {
            CIMInstance temp = _instances[index].clone();
            if (temp.findProperty("requestInputParameters") == PEG_NOT_FOUND)
            {
                CIMClass tmpClass = _getClass(CIMName(
                    "Test_CLITestProviderClass"),
                    nameSpace);
                Uint32 pos = tmpClass.findProperty("requestInputParameters");
                temp.addProperty(tmpClass.getProperty(pos));
            }

            temp.getProperty(temp.findProperty("requestInputParameters"))
                .setValue(text);

            CIMObjectPath rtnObjectName = objectName;
            if (_useSubstituteHostName)
            {
                rtnObjectName.setHost(_getHostName());
            }
            temp.setPath(rtnObjectName);
            handler.deliver(temp);
        }
        catch(CIMException& e)
        {
            cerr << "CIMCLITestProvider:Exception Occured : "
                << e.getMessage() << endl;
            throw CIMException(e);
        }
    }
    handler.complete();
}

void CLITestProvider::associatorNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& associationClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    ObjectPathResponseHandler& handler)
{
    initializeProvider(objectName.getNameSpace());
    // Get the namespace and host names to create the CIMObjectPath
    CIMNamespaceName nameSpace = objectName.getNameSpace();
    String host = _getHostName();

    handler.processing();

    // Return an instance of the associated class for every instance
    // currently in the local list. Simple since we just return the
    // input path if the instance exists.
    Uint32 index;
    if ((index = findInstance(objectName)) != PEG_NOT_FOUND)
    {
        try
        {
            CIMObjectPath rtnObjectName = objectName;
            if (_useSubstituteHostName)
            {
                rtnObjectName.setHost(_getHostName());
            }
            handler.deliver(rtnObjectName);
        }
        catch(CIMException& e)
        {
            cerr << "CIMCLITestProvider:Exception Occured : "
                << e.getMessage() << endl;
            throw CIMException(e);
        }
    }
    handler.complete();
}

void CLITestProvider::references(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    ObjectResponseHandler& handler)
{
    initializeProvider(objectName.getNameSpace());
    // Get the namespace and host names to create the CIMObjectPath
    CIMNamespaceName nameSpace = objectName.getNameSpace();
    String host = _getHostName();

    handler.processing();

    CIMName objectClassName = objectName.getClassName();

    // if the target instance exists in the local storage, build the
    // association class instance
    AutoMutex autoMut(instanceArrayMutex);
    Uint32 index;
    if ((index = findInstance(objectName)) != PEG_NOT_FOUND)
    {
        Array<CIMName> refClassArray;
        refClassArray.append(CIMName("Test_CLITestProviderLinkClass"));

        // Create a single instance of the Test_CLITestProviderLinkClass
        // This creates a single fixed instance simply to allow the
        // cimcli client to test results.  It also places the input parameters
        // into the text result so that the client can confirm that the
        // input parameters were passed to the provider.
        // Create path for assoc instance.

        CIMClass assocClass = _getClass(CIMName(
            "Test_CLITestProviderLinkClass"),
            nameSpace);

        // Create a single instance with all properties and with path
        // independent of namespace or hostname

        CIMInstance assocInstance = assocClass.buildInstance(
            true, true, CIMPropertyList());

        CIMObjectPath rtnObjectName = objectName;
        if (_useSubstituteHostName)
        {
            rtnObjectName.setHost(_getHostName());
        }

        assocInstance.getProperty(
            assocInstance.findProperty("parent"))
            .setValue(rtnObjectName);

        assocInstance.getProperty(
            assocInstance.findProperty("child"))
            .setValue(rtnObjectName);

        // Put input parameters into the requestInputParameters property so
        // they can be confirmed by the client.
        String text;
        _addParam(text, "role", role);
        _addParam(text, "resultClass", resultClass.getString());
        _addParam(text, "includeQualifiers", _toString(includeQualifiers));
        _addParam(text, "includeClassOrigin", _toString(includeClassOrigin));
        _addParam(text, "propertyList", _toString(propertyList));

        assocInstance.getProperty(
            assocInstance.findProperty("requestInputParameters"))
            .setValue(text);

        CIMObjectPath objectPath =
            assocInstance.buildPath(assocClass);

        _completePath(host, nameSpace, objectPath);

        assocInstance.setPath(objectPath);

        handler.deliver(assocInstance);
    }
    handler.complete();
}

// Return all references (association instance names) in which the given
// object is involved.

void CLITestProvider::referenceNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    ObjectPathResponseHandler& handler)
{
    CIMNamespaceName nameSpace = objectName.getNameSpace();
    initializeProvider(nameSpace);
    // Get the namespace and host names to create the CIMObjectPath

    String host = _getHostName();

    // If the objectName exists in the local list, build the instance
    // of the association and then build the path for this instance.

    AutoMutex autoMut(instanceArrayMutex);
    Uint32 index;

    if ((index = findInstance(objectName)) != PEG_NOT_FOUND)
    {
        Array<CIMName> refClassArray;
        refClassArray.append(CIMName("Test_CLITestProviderLinkClass"));

        CIMInstance assocInstance("Test_CLITestProviderLinkClass");

        CIMObjectPath rtnObjectName = objectName;
        if (_useSubstituteHostName)
        {
            rtnObjectName.setHost(_getHostName());
        }

        assocInstance.addProperty(CIMProperty(CIMName("parent"),
            rtnObjectName,
            0,
            CIMName("Test_CLITestProviderClass")));

        assocInstance.addProperty(CIMProperty(CIMName("child"),
            rtnObjectName,
            0,
            CIMName("Test_CLITestProviderClass")));

        CIMClass assocClass = _getClass(
            CIMName("Test_CLITestProviderLinkClass"),
            nameSpace);

        // build path for this instance
        CIMObjectPath objectPath =
            assocInstance.buildPath(assocClass);

        _completePath(host, nameSpace, objectPath);

        handler.deliver(objectPath);
    }
    // complete processing the request
    handler.complete();
}


/* get the defined class from the repository. Classes once acquired
   are saved in an array so that future references get the local copy
    @param className CIMName name of the class to get
    @return CIMClass with the class or unitialized if
    there was an error in the getClass
*/
CIMClass CLITestProvider::_getClass(const CIMName& className,
                                    const CIMNamespaceName& ns)
{
    // try to get from array of already acquired classess
    for (Uint32 i = 0; i < _classes.size() ; i++)
    {
        if (_classes[i].getClassName() == className)
        {
            return _classes[i];
        }
    }

    // Not in array yet, get this class from the cim server.
    CIMClass c;
    try
    {
        c = _cimom.getClass(
            OperationContext(),
            ns,
            className,
            false,
            true,
            true,
            CIMPropertyList());

        _classes.append(c);
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "CLITestProvider GetClass operation failed: Class %s. Msg %s",
            (const char*) className.getString().getCString(),
            (const char*) e.getMessage().getCString()));
        throw CIMException(CIM_ERR_FAILED);
    }

    return c;
}

/*
    Find an instance in the instance array with path defined by
    the input parameter. returns the index of the instance or PEG_NOT_FOUND
*/
Uint32 CLITestProvider::findInstance(const CIMObjectPath& path)
{
    CIMObjectPath localPath = _localPath(path);

    for (Uint32 i = 0; i < _instances.size(); i++)
    {
        if(localPath == _instances[i].getPath())
        {
            return i;
        }
    }
    return PEG_NOT_FOUND;
}

// Create the instances that will be considered inherently part of the
// provider for these tests.  This includes one instance of each class
// This was done with namespace input so that we could build association
// instances that require namespace.  It later turned out to be easier
// to build them on the fly so that the namespace parameter and the
// corresponding dynamic initialization of the provider (initializeProvider)
// are not really necessary.
// NOTE: Uses the namespace of the initial operation to determine the
// namespace in which these instances are created and where it looks for
// the class definitions.

void CLITestProvider::createInstances(const CIMNamespaceName& ns)
{
    if (_debugMode)
    {
        cout << "Enter createInstances function" << endl;
    }

    // Create instances of Test_CLITestProviderClass class
    CIMClass theClass = _getClass(
        CIMName("Test_CLITestProviderClass"),
        ns);

    // Create a single instance with all properties and with path
    // independent of namespace or hostname

    CIMInstance instance = theClass.buildInstance(
        true, true, CIMPropertyList());

    instance.getProperty(instance.findProperty("Id")).setValue(
        CIMValue(String("Mike")));
    instance.getProperty(instance.findProperty("Name")).setValue(
        CIMValue(String("Bob")));
    instance.getProperty(instance.findProperty("scalBool")).setValue(
        CIMValue(Boolean(true)));
    instance.getProperty(instance.findProperty("scalUint8")).setValue(
        CIMValue(Uint8(220)));
    instance.getProperty(instance.findProperty("scalSint8")).setValue(
        CIMValue(Sint8(124)));
    instance.getProperty(instance.findProperty("scalUint16")).setValue(
        CIMValue(Uint16(100)));
    instance.getProperty(instance.findProperty("scalSint16")).setValue(
        CIMValue(Sint16(100)));
    instance.getProperty(instance.findProperty("scalUint32")).setValue(
        CIMValue(Uint32(100)));
    instance.getProperty(instance.findProperty("scalSint32")).setValue(
        CIMValue(Sint32(100)));
    instance.getProperty(instance.findProperty("scalUint64")).setValue(
        CIMValue(Uint64(100)));
    instance.getProperty(instance.findProperty("scalReal32")).setValue(
        CIMValue(Real32(100)));
    instance.getProperty(instance.findProperty("scalReal64")).setValue(
        CIMValue(Real64(100)));
    instance.getProperty(instance.findProperty("scalString")).setValue(
        CIMValue(String("teststring")));
    instance.getProperty(instance.findProperty("scalDateTime")).setValue(
        CIMValue(CIMDateTime("19991224120000.000000+360")));

    // set Values into the corresponding array properties
    Array<Boolean> ab;
    ab.append(true); ab.append(false); ab.append(true);
    instance.getProperty(instance.findProperty("arrayBool")).setValue(
        CIMValue(ab));

    Array<Uint8> auint8;
    auint8.append(4); auint8.append(128); auint8.append(240);
    instance.getProperty(instance.findProperty("arrayUint8")).setValue(
        CIMValue(auint8));

    Array<Sint8> asint8;
    asint8.append(4); asint8.append(126); asint8.append(-126);
    instance.getProperty(instance.findProperty("arraySint8")).setValue(
        CIMValue(asint8));

    Array<Uint16> auint16;
    auint16.append(4); auint16.append(128); auint16.append(240);
    instance.getProperty(instance.findProperty("arrayUint16")).setValue(
        CIMValue(CIMValue(auint16)));

    Array<Sint16> asint16;
    asint16.append(4); asint16.append(126); asint16.append(-126);
    instance.getProperty(instance.findProperty("arraySint16")).setValue(
        CIMValue(asint16));

    Array<Uint32> auint32;
    auint32.append(4); auint32.append(128); auint32.append(240);
    instance.getProperty(instance.findProperty("arrayUint32")).setValue(
        CIMValue(auint32));

    Array<Sint32> asint32;
    asint32.append(4); asint32.append(126); asint32.append(-126);
    instance.getProperty(instance.findProperty("arraySint32")).setValue(
        CIMValue(asint32));

    Array<Uint64> auint64;
    auint64.append(4); auint64.append(128); auint64.append(240);
    instance.getProperty(instance.findProperty("arrayUint64")).setValue(
         CIMValue(auint64));

    Array<Real32> aReal32;
    aReal32.append(4); aReal32.append(128); aReal32.append(240);
    instance.getProperty(instance.findProperty("arrayReal32")).setValue(
        CIMValue(aReal32));

    Array<Real64> aReal64;
    aReal64.append(4); aReal64.append(128); aReal64.append(240);
    instance.getProperty(instance.findProperty("arrayReal64")).setValue(
        CIMValue(aReal64));

    Array<String> aString;
    aString.append("First"); aString.append("Second"); aString.append("Third");
    instance.getProperty(instance.findProperty("arrayString")).setValue(
        CIMValue(aString));

    Array<CIMDateTime> aCIMDateTime;
    aCIMDateTime.append(CIMDateTime("19991224120000.000000+360"));
    aCIMDateTime.append(CIMDateTime("19991224120000.000000+360"));
    aCIMDateTime.append(CIMDateTime("19991224120000.000000+360"));
    instance.getProperty(instance.findProperty("arrayDateTime")).setValue(
        CIMValue(aCIMDateTime));

    CIMObjectPath p("Test_CLITestProviderClass.Id=\"Mike\"");

    instance.setPath(p);

    _instances.append(instance);

    // Create instance of Test_CLITestEmbedded1 Class

    // Create an embedded instance
    CIMClass embeddedClass1 = _getClass(CIMName("Test_CLITestEmbedded1"), ns);

    CIMInstance embedded1 = embeddedClass1.buildInstance(
        true, true, CIMPropertyList());

    embedded1.getProperty(embedded1.findProperty("Id")).setValue(
        CIMValue(String("100")));

    embedded1.getProperty(embedded1.findProperty("name")).setValue(
        CIMValue(String("Ronald")));

    embedded1.getProperty(embedded1.findProperty("comment")).setValue(
        CIMValue(String("Instance created by provider.")));

    CIMObjectPath p1("Test_CLITestEmbedded1.Id=100");
    embedded1.setPath(p1);

    if (_debugMode)
    {
        cout << "Created Test_CLITestEmbedded1 instance " << endl;
    }

    // Create instance of Test_CLITestEmbedded3 Class

    CIMClass embedded3Class = _getClass(CIMName("Test_CLITestEmbedded3"), ns);

    CIMInstance embedded3 = embedded3Class.buildInstance(
        true, true, CIMPropertyList());

    embedded3.getProperty(embedded3.findProperty("Id")).setValue(
        CIMValue(String("103")));

    embedded3.getProperty(embedded3.findProperty("name")).setValue(
        CIMValue(String("McDonald")));

    embedded3.getProperty(embedded3.findProperty("comment")).setValue(
        CIMValue(String("Instance created by provider.")));

    CIMObjectPath p3("Test_CLITestEmbedded3.Id=100");
    embedded3.setPath(p3);

    if (_debugMode)
    {
        cout << "Created Test_CLITestEmbedded3 instance " << endl;
    }

    // Create instance of Test_CLITestEmbedded2 Class
    CIMClass embedded2Class = _getClass(CIMName("Test_CLITestEmbedded2"), ns);

    CIMInstance embedded2 = embedded2Class.buildInstance(
        true, true, CIMPropertyList());

    embedded2.getProperty(embedded2.findProperty("Id")).setValue(
        CIMValue(String("102")));

    embedded2.getProperty(embedded2.findProperty("name")).setValue(
        CIMValue(String("Ronald")));

    embedded2.getProperty(embedded2.findProperty("comment")).setValue(
        CIMValue(String("Instance created by provider.")));

    embedded2.getProperty(
        embedded2.findProperty("recursiveEmbeddedInst")).setValue(
        CIMValue(CIMInstance(embedded3)));

    CIMObjectPath p2("Test_CLITestEmbedded2.Id=102");
    embedded2.setPath(p2);

    if (_debugMode)
    {
        cout << "Created Test_CLITestEmbedded2 instance " << endl;
    }

    // build the instance that embedds the above instance
    CIMClass embeddingClass = _getClass(
        CIMName("Test_CLITestEmbeddedClass"), ns);

   CIMInstance embeddedingClassInst =
       embeddingClass.buildInstance(true, true, CIMPropertyList());

    embeddedingClassInst.getProperty(
        embeddedingClassInst.findProperty("Id")).setValue(
        CIMValue(String("101")));

    embeddedingClassInst.getProperty(
        embeddedingClassInst.findProperty("embeddedInst")).setValue(
        CIMValue(CIMInstance(embedded1)));

    embeddedingClassInst.getProperty(
        embeddedingClassInst.findProperty("embeddedInst2")).setValue(
        CIMValue(CIMInstance(embedded2)));

    if (_debugMode)
    {
        cout << "Created Embedding instance " << endl;
    }

    CIMObjectPath p4("Test_CLITestEmbeddedClass.Id=101");

    embeddedingClassInst.setPath(p4);

    _instances.append(embeddedingClassInst);
}

// initialze the provider by creating the instances that will exist
// in the instances array at startup.   The namespace parameter is the
// namespace that will be set in any paths built when instances
// are created.
void CLITestProvider::initializeProvider(const CIMNamespaceName& ns)
{
    if (!_initialized)
    {
        AutoMutex autoMut(instanceArrayMutex);
        if (!_initialized)
        {
            createInstances(ns);
            _initialized = true;
        }
    }
}


