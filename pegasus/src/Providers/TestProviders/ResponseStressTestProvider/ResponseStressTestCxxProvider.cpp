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

#include <cstring>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>

#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/Mutex.h>

#include "ResponseStressTestCxxProvider.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// String pattern used to build up size in response instances
String pattern = "abcdefghighjklmnopqrstuvwxyz01234567890";

// Name of superClass used for all classes supported by this provider.
// Any class supported by this provider must be a trivial subclass of
// the class defined by TestClass below.
// TODO add test to be sure any operation is for class that is
// direct subclass of this class.
//// CIMName superClass = "TST_ResponseStressTest";

// Count of instances to be delivered before executing a delay defined by
// the _delay parameter. Ignored if _delay = 0.
#define INSTANCE_FOR_DELAY_COUNT 150

///////////////////////////////////////////////////////////////////
//
//      Config variable controller
//   Defines a table where the config Parameters are kept by
//   ObjectPath of the request.  Therefore, we can have different
//   behavior for different classes or the same class in different
//   namespaces.
//   Note that this provider does NOT check to see if the class in the
//   request has is the same as TST_ResponseStressTest.
//   It will build responses based on the properties in
//   TST_ResponseStessTest in any case.
//   The reason for the multiple class support is to be able to conduct
//   tests with different behavior simultaneously so that the
//   behavior changes (ex. setting delay) for one test does not impact
//   other tests.
//
////////////////////////////////////////////////////////////////////

// The set of configuration variables for this provider
// Structure of behavior controlling parameters. These are set
// by invoke methods and used by the provider to create responses//
//
// The providerParms structure defines the provider behavior parameters that
// are controllable and define a set of default values for these
// parameters
//
typedef struct providerParms
{
    Uint64 _instanceSize;
    Uint64 _responseCount;
    Uint64 _countToFail;
    Uint32 _failureStatusCode;
    Uint32 _delay;
    Boolean _continue;

    // Set the default values
    void defaultValues()
    {
        _responseCount = 5;
        _instanceSize = 100;
        _continue = true;
        _countToFail = 0;
        _failureStatusCode = 0;
        _delay = 0;
    }
    // Diagnostic to display the values of the parameters grouped in
    // this structure.
    String toString()
    {
        String rtn;
        rtn.appendPrintf("instSize %llu respCnt %llu delay %u continue %s"
                         " failStatusCode %u",
                   _instanceSize,
                   _responseCount,
                   _delay,
                   boolToString(_continue),
                   _failureStatusCode);
        return rtn;
    }
} providerParms;

//
//  Structure to link the CIMObjectPath to a particular set of
//  providerParms.  For simplicity we save just the classname
//  and namespace out of the objectPath.
//  Assignment and copy use the default mechanisms
typedef struct providerParmSet
{
    CIMName cl;
    CIMNamespaceName ns;
    providerParms thisProviderParms;

    // Constructor for a providerParmSet that creates a new one based on
    // a provided path
    providerParmSet(const CIMObjectPath& path)
    {
        cl = path.getClassName();
        ns = path.getNameSpace();
        thisProviderParms.defaultValues();
    }
    // Return true if path argument matches this providerParmSet
    Boolean matches(const CIMObjectPath& path)
    {
        return((cl == path.getClassName()) && (ns == path.getNameSpace()));
    }
private:
    // Private constructor
    providerParmSet();

} providerParmSet;

class ConfigVariables
{
public:

    // Array of struct that maps className and namespace
    // to a particular set of configuration variables and
    // possibly a class
    Array<providerParmSet> configVariablesArray;
    // The default value for parameters for constructor and when the
    // provider invoke method resets.
    // KS_TODO this is unnecessary.
    providerParms localDefaultParams;

    // Construct with default values. There is nothing in path at
    // this point
    ConfigVariables()
    {
        localDefaultParams.defaultValues();
    }

    // set Default values and keep current path
    void setDefault()
    {
        localDefaultParams.defaultValues();
    }

    void resetParams(const CIMObjectPath& path)
    {
        AutoMutex autoMut(_mutex);
        int i = find(path);
        PEGASUS_ASSERT(i != -1);
        configVariablesArray[i].thisProviderParms.defaultValues();
    }

    // get an entry that matches or create a new one and return
    // the providerParms from that entry
    providerParms get(const CIMObjectPath& path)
    {
        int i;
        if ((i = find(path)) != -1)
        {
            return configVariablesArray[(Uint32)i].thisProviderParms;
        }
        return append(path);
    }

    // Entry must exist. set the providerParms argument into that entry
    void set(const CIMObjectPath& path, providerParms p)
    {
        AutoMutex autoMut(_mutex);
        int i = find(path);
        PEGASUS_ASSERT(i != -1);
        configVariablesArray[i].thisProviderParms = p;
    }
    void print()
    {
        AutoMutex autoMut(_mutex);
        for (Uint32 i = 0; i < configVariablesArray.size(); i++)
        {
            cout << "Item " << i
                 << " class " <<  configVariablesArray[i].cl.getString()
                 << "ns " << configVariablesArray[i].ns.getString()
                 << " " << configVariablesArray[i].thisProviderParms.toString()
                 << endl;
        }
    }

private:
    providerParms append(const CIMObjectPath& path)
    {
        AutoMutex autoMut(_mutex);
        providerParmSet m(path);
        configVariablesArray.append(m);

        Uint32 pos = configVariablesArray.size() -1;
        return configVariablesArray[pos].thisProviderParms;
    }

    // Mutex the callers to this funciton
    int find(const CIMObjectPath& path)
    {
        for (Uint32 i = 0; i < configVariablesArray.size(); i++)
        {
            if (configVariablesArray[i].matches(path))
            {
                return (int)i;
            }
        }
        return -1;
    }
    // Mutex selected operations to protect against concurrent modification
    // of the array.
    Mutex _mutex;
};

// Static instantiation of the configVariables class
ConfigVariables myConfigVariables;

////////////////////////////////////////////////////////////////////
//
//  Local Methods
//
////////////////////////////////////////////////////////////////////
// Build a String of size defined by size parameter * size of pattern
// parameter
String _buildString(Uint64 size, const String& pattern)
{
    String rtn;
    Uint32 repCount = 1;
    if (size > pattern.size())
    {
        repCount = size / pattern.size();
    }

    // Append the pattern to the String
    for (Uint32 i = 0; i <= repCount; i++)
    {
        rtn.append(pattern);
    }
    return(rtn);
}

/*
    Builds a single path of the TST_ResponseTest class
*/
CIMObjectPath _buildPath(Uint64 sequenceNumber, CIMName className)
{
    Array<CIMKeyBinding> keyBindings;

    char namebuf[60];
    sprintf(namebuf, "%lu", (unsigned long)sequenceNumber);

    keyBindings.append(CIMKeyBinding("Id",
        namebuf, CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        className, keyBindings);
    return(path);
}

/*
    Builds a single instance with the defined sequence number, instanceSize,
    and timeDiff parameters.
*/
void _buildInstance(CIMName className, CIMInstance instance,
    Uint64 sequenceNumber,
    Uint64 timeDiff,
    const CIMPropertyList& propertyList,
    const String s1)
{
    char namebuf[60];
    sprintf(namebuf, "%lu", (unsigned long)sequenceNumber);

    instance.addProperty(CIMProperty("Id", String(namebuf)));
    instance.addProperty(CIMProperty("SequenceNumber", sequenceNumber));
    instance.addProperty(CIMProperty("Pattern", pattern));
    instance.addProperty(CIMProperty("interval", timeDiff));
    instance.addProperty(CIMProperty("S1", s1));

    instance.setPath(_buildPath(sequenceNumber,className));

    // filter out unwanted properties. It would be cheaper to not add
    // the unwanted properties. Note that this function is not really
    // required here since the real solution should be to
    // a) not build the properties that are not to be sent
    // b) let the server eliminate them if you do build everything.
    // However, it created a nasty bug in the pull operations tests
    // bug 9953) and so we leave it here to assure that the
    // bug is fixed.

    instance.instanceFilter(true, true, propertyList);

    return;
}

//
//      ResponseStressTestCxxProvider Class Methods
//

ResponseStressTestCxxProvider::ResponseStressTestCxxProvider()
{
}

ResponseStressTestCxxProvider::~ResponseStressTestCxxProvider()
{
}

// Reset all of the behavior parameters to a defined default.
void ResponseStressTestCxxProvider::resetParameters()
{
    myConfigVariables.setDefault();

}
void ResponseStressTestCxxProvider::initialize(CIMOMHandle& cimom)
{
    // save cimom handle
    _cimom = cimom;
    resetParameters();
}

void ResponseStressTestCxxProvider::terminate()
{
    delete this;
}

/*
    Minimal implementation since the goal of this provider is to return
    enumerates.
*/
void ResponseStressTestCxxProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();
    providerParms myParams = myConfigVariables.get(instanceReference);
    CIMName thisClass = instanceReference.getClassName();

    // Build the big string that sets the instance size
    String s1 = _buildString(myParams._instanceSize, pattern);

    // NOTE: For getInstance ignore the timeDiff property
    // and set to zero.
    CIMInstance instance(thisClass);
    _buildInstance(thisClass,instance,
        0,
        0,
        propertyList,
        s1);
    handler.deliver(instance);

    throw CIMException(CIM_ERR_NOT_FOUND);

    handler.complete();
}

/*
    Build and return a set of instances based on the provider parameters
    of _responseCount and _instanceSixe
*/
void ResponseStressTestCxxProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();

    // get the parameter set for this objectPath
    providerParms myParams = myConfigVariables.get(ref);

    CIMName thisClass = ref.getClassName();

    // Build the big string that sets the instance size
    String s1 = _buildString(myParams._instanceSize, pattern);

    // if _countToFail != 0 count number of objects to deliver before
    // we issue the defined CIMException status code
    Uint32 countToFail = myParams._countToFail;

    Uint64 prevTime = TimeValue::getCurrentTime().toMicroseconds();

    for (Uint32 i = 0, n = myParams._responseCount ; i < n; i++)
    {
        // Each instance reflects time difference from previous instance
        // creation.
        Uint64 newTime = TimeValue::getCurrentTime().toMicroseconds();

        try
        {
            CIMInstance instance(thisClass);
            _buildInstance(thisClass,instance,
                i,
                (newTime - prevTime),
                propertyList,
                s1);

            prevTime = newTime;
            handler.deliver(instance);
        }
        catch (CIMException&)
        {
            // suppress error
        }

        if (myParams._countToFail != 0)
        {
            if ((--countToFail) == 0)
            {
                throw CIMException((CIMStatusCode)myParams._failureStatusCode,
                    "Reached failure count");
            }
        }
        // If _delay parameter not equal zero, execute a delay at regular
        // interval defined by INSTANCE_FOR_DELAY_COUNT
        if (myParams._delay != 0 && i > 0)
        {
            if ((i % INSTANCE_FOR_DELAY_COUNT) == 0)
            {
                System::sleep(myParams._delay);
            }
        }
    }

    handler.complete();
}

void ResponseStressTestCxxProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    handler.processing();

    providerParms myParams = myConfigVariables.get(classReference);
    CIMName thisClass = classReference.getClassName();

    Uint32 countToFail = myParams._countToFail;

    for (Uint32 i = 0, n = myParams._responseCount ; i < n; i++)
    {
        try
        {
            CIMObjectPath path = _buildPath(i, thisClass);
            handler.deliver(path);
        }
        catch (CIMException&)
        {
            // suppress error
        }
        if (myParams._countToFail != 0)
        {
            if ((--countToFail) == 0)
            {
                throw CIMException((CIMStatusCode)myParams._failureStatusCode,
                    "Reached failure count Limit");
            }
        }
        // If _delay parameter not equal zero, execute a delay at regular
        // interval defined by INSTANCE_FOR_DELAY_COUNT
        if ((myParams._delay != 0) && (i > 0))
        {
            if ((i % INSTANCE_FOR_DELAY_COUNT) == 0)
            {
                System::sleep(myParams._delay);
            }
        }
    }

    handler.complete();
}

void ResponseStressTestCxxProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void ResponseStressTestCxxProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void ResponseStressTestCxxProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void ResponseStressTestCxxProvider::invokeMethod(
    const OperationContext & context,
    const CIMObjectPath & objectReference,
    const CIMName & methodName,
    const Array<CIMParamValue> & inParameters,
    MethodResultResponseHandler & handler)
{
    // get the config Variables for this objectPath and
    // possibly build new entry in table
    providerParms myParams = myConfigVariables.get(objectReference);

    CIMName thisClass = objectReference.getClassName();

    // convert a fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectReference.getClassName(),
        objectReference.getKeyBindings());

    handler.processing();
    Uint32 rtnCode = 0;

    // NOTE: Following test pretty worthless
    if (objectReference.getClassName().equal(thisClass))
    {
        // set method sets the _responseCount and instanceSize based on
        // input parameters.
        if (methodName.equal("set"))
        {
            for(Uint32 i = 0; i < inParameters.size(); ++i)
            {
                if (_continue == false)
                {
                    _continue = true;
                }

                String paramName = inParameters[i].getParameterName();
                CIMValue v = inParameters[i].getValue();

                if(paramName =="ResponseCount")
                {
                    Uint64 responseCount;
                    v.get(responseCount);
                    myParams._responseCount = responseCount;
                }
                else if(paramName =="Size")
                {
                    Uint64 instanceSize;
                    v.get(instanceSize);
                    myParams._instanceSize = instanceSize;
                }
                else if(paramName =="CountToFail")
                {
                    Uint64 countToFail;
                    v.get(countToFail);
                    myParams._countToFail = countToFail;
                }
                else if(paramName =="FailureStatusCode")
                {
                    Uint32 failureStatusCode;
                    v.get(failureStatusCode);
                    myParams._failureStatusCode = failureStatusCode;
                }
                else if(paramName =="Delay")
                {
                    Uint32 delay;
                    v.get(delay);
                    myParams._delay = delay;
                }
                else
                {
                    rtnCode = 1;
                }
            }

            // set new parameters back into the configVariables
            myConfigVariables.set(objectReference, myParams);
        }

        // get method returns current _responseCount and instanceSize
        // parameters.
        else if (methodName.equal("get"))
        {
            Array<CIMParamValue> OutParams;

            OutParams.append(CIMParamValue("ResponseCount",
                myParams._responseCount));
            OutParams.append(CIMParamValue("Size",
                (Uint64)myParams._instanceSize));
            OutParams.append(CIMParamValue("CountToFail",
                (Uint64)myParams._countToFail));
            OutParams.append(CIMParamValue("FailureStatusCode",
                (Uint32)myParams._failureStatusCode));
            OutParams.append(CIMParamValue("Delay", myParams._delay));
            handler.deliverParamValue(OutParams);
            handler.deliver(Uint32(0));
        }
        // Resets all of the method behavior parameters and returns OK
        else if (methodName.equal("reset"))
        {
            myConfigVariables.resetParams(objectReference);
            handler.deliver(Uint32(0));
        }

        // stop method set stop flag for the provider.  Hopefully this
        // would stop any existing generation of responses.  Note that
        // this flag remains set until provider is reloaded.
        // FUTURE: We do not use the method today.
        else if (methodName.equal("stop"))
        {
            Uint32 rtn = (_continue)? 0 : 1;
            _continue = false;
            handler.deliver(rtn);
        }
        else
        {
            throw CIMException(CIM_ERR_METHOD_NOT_AVAILABLE);
        }
    }
    handler.deliver(rtnCode);
    handler.complete();
}
PEGASUS_NAMESPACE_END
