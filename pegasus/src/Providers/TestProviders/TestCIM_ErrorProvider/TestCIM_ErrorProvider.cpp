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
    This is a test provider defined specifically to test the generation
    of CIM_Error objects in exception responses.  It returns responses
    from getInstance and enumerateInstances only.   It has one built in
    instance to be returned from getInstance and otherwise returns an
    error with a CIM_Error object.

    enumerateInstances always returns an error with 2 CIM_Error objects.

    NOTE: If Pegasus ever builds an overall test tool for determining
    consistency of providers, the above strategy is flawed in that
    the enumerateInstances function should also return an object rather
    than the errors.

    TODO: Generalize so that the basic provider returns the instances defined
    unless a setError method and provided instance(s) of CIM_Error to be
    returned.  In that case, both getInstance and enumerateInstances would
    return the errors.

    This provider includes a debugMode that generates some diagnostic mode
    but this can only be enabled by recompiling today.
*/

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/General/CIMError.h>

#include <Pegasus/Common/CIMDateTime.h>

#include "TestCIM_ErrorProvider.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;



const char* PegasusOwningEntityName = "OpenPegasus";

// Generate a set of fixed errors that are the default CIM_Error objects
// returned.
Array<CIMInstance> generateDefaultErrors(Boolean debugMode)
{
    Array<CIMInstance> errors;

    CIMError err(PegasusOwningEntityName,
              "MSG0001",
              "TestErrorProvider::enumerateInstances(). First Error",
              CIMError::PERCEIVED_SEVERITY_FATAL,
              CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
              CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);
    errors.append(err.getInstance());

    CIMError err1(PegasusOwningEntityName,
              "MSG0002",
              "TestErrorProvider::enumerateInstances(). Second Error",
              CIMError::PERCEIVED_SEVERITY_FATAL,
              CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
              CIMError::CIM_STATUS_CODE_CIM_ERR_NOT_FOUND);

    errors.append(err1.getInstance());

    if (debugMode)
    {
        cout << "Generating 2 CIM_Errors as enumerateInstance response."
            << endl;
        err.print();
        err1.print();
    }
    return errors;
}

TestCIM_ErrorProvider::TestCIM_ErrorProvider()
    :
    _debugMode(false), _instancesToDeliverCounter(0)
{
}

TestCIM_ErrorProvider::~TestCIM_ErrorProvider()
{
}

void createInstances(Array<CIMInstance>& instances)
{
    // create default instances
    {
        CIMInstance instance("Test_CIM_ErrorResponse");
        instance.addProperty(CIMProperty("Id", String("1")));

        instance.addProperty(CIMProperty("name", String("SomeName")));

        instance.setPath(CIMObjectPath("Test_CIM_ErrorResponse.Id=\"One\""));
        instances.append(instance);
    }
}

void TestCIM_ErrorProvider::initialize(CIMOMHandle& cimom)
{
    // create default instances
    if (_debugMode)
    {
        cout << "TestCIM_ErrorProvider::initialize" << endl;
    }
    createInstances(_instances);
}

void TestCIM_ErrorProvider::terminate()
{
    delete this;
}

void TestCIM_ErrorProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    if (_debugMode)
    {
        cout << "TestCIM_ErrorProvider::getInstance "
              << " instanceReference = "
              << instanceReference.toString() << endl;
    }
    handler.processing();

    // create relative object path for comparison
    CIMObjectPath cimObjectPath(instanceReference);

    cimObjectPath.setHost(String());
    cimObjectPath.setNameSpace(CIMNamespaceName());

    // getInstance operation delivers an instance if one exists or
    // CIM_Error if no instance exists.
    Boolean delivered = false;

    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if (cimObjectPath == _instances[i].getPath())
        {
            try
            {
                delivered = true;
                handler.deliver(_instances[i]);
            }
            catch (CIMException&)
            {
                // suppress error
            }

            break;
        }
    }

    // How can we deliver an instance and also an error and how can be
    // tell the provider to do this
    if (!delivered)
    {
        CIMError err(PegasusOwningEntityName,
                  "MSG0001",
                  "TestErrorProvider::getInstances()",
                  CIMError::PERCEIVED_SEVERITY_FATAL,
                  CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
                  CIMError::CIM_STATUS_CODE_CIM_ERR_NOT_FOUND);

        if (_debugMode)
        {
            cout << "Generating CIM_Error as getInstance response." << endl;
            err.print();
        }
        throw CIMException(CIM_ERR_NOT_FOUND,
            instanceReference.toString(),
            err.getInstance());
    }
    else
    {
            handler.complete();
    }
}

// returns two CIM Errors.
void TestCIM_ErrorProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    if (_debugMode)
    {
        cout << "Enter enumerateInstances." << endl;
    }

    handler.processing();


    for (Uint32 i = 0, n = _instances.size(); i < n ; i++)
    {
        // Deliver up to count value.  If we do not reach count value
        // do not generate errors
        if (n == _instancesToDeliverCounter)
        {
            Array<CIMInstance> errors = generateDefaultErrors(_debugMode);
            throw CIMException(CIM_ERR_FAILED,
                "Test of Multiple Error objects returned."
                " Returns 2 CIM_Error objects",
                errors);
        }

        try
        {
            handler.deliver(_instances[i]);
        }
        catch (CIMException&)
        {
            // suppress error
        }
    }

    handler.complete();
}

void TestCIM_ErrorProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    handler.processing();


    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        // Deliver up to count value.  If we do not reach count value
        // do not generate errors
        if (n == _instancesToDeliverCounter)
        {
            Array<CIMInstance> errors = generateDefaultErrors(_debugMode);
            throw CIMException(CIM_ERR_FAILED,
                "Test of Multiple Error objects returned."
                " Returns 2 CIM_Error objects",
                errors);
        }

        try
        {
            handler.deliver(_instances[i].getPath());
        }
        catch (CIMException&)
        {
            // suppress error
        }
    }

    handler.complete();
}

void TestCIM_ErrorProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestCIM_ErrorProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestCIM_ErrorProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestCIM_ErrorProvider::invokeMethod(
    const OperationContext & context,
    const CIMObjectPath & objectReference,
    const CIMName & methodName,
    const Array<CIMParamValue> & inParameters,
    MethodResultResponseHandler & handler)
{
    // display input parameters
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

    handler.processing();

    Array<CIMParamValue> outParameters;
    Uint32 rtnCode = 0;
    if (objectReference.getClassName().equal("Test_CIM_ErrorResponse"))
    {
        if (methodName.equal("testMethodResponse"))
        {
            // if parameters exist, Error.
            if (inParameters.size() != 0)
            {
                String errMsg = "Input parameters not allowed for method "
                    + methodName.getString();

                // throw CIMOperationFailedException(errMsg);
                CIMError err(PegasusOwningEntityName,
                    "MSG0003",
                    "TestErrorProvider::invokeMethod()."
                    " Incorrect in parameter"   ,
                    CIMError::PERCEIVED_SEVERITY_FATAL,
                    CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
                    CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);

                throw CIMException(CIM_ERR_FAILED,
                    errMsg,
                    err.getInstance());
            }
        }

        else if (methodName.equal("setTestParameters"))
        {
            if (inParameters.size() > 0)
            {
                for(Uint32 i = 0; i < inParameters.size(); ++i)
                {
                    String paramName = inParameters[i].getParameterName();
                    CIMValue paramVal = inParameters[i].getValue();

                    if (paramName == "debugMode")
                    {
                        if (paramVal.getType() == CIMTYPE_BOOLEAN)
                        {
                            paramVal.get(_debugMode);
                        }

                        else
                        {
                            // return error. Incorrect type on parameter
                            String errMsg = "Incorrect input parameter type "
                                 + paramName + " for method "
                                 + methodName.getString();

                            throw CIMOperationFailedException(errMsg);
                        }
                    }

                    else if (paramName == "countToDeliver")
                    {
                        if (paramVal.getType() == CIMTYPE_UINT32)
                        {
                            paramVal.get(_instancesToDeliverCounter);
                        }
                        else
                        {
                            // return error. Incorrect type on parameter
                            String errMsg = "Incorrect input parameter type "
                                 + paramName + " for method "
                                 + methodName.getString();

                            throw CIMOperationFailedException(errMsg);
                        }
                    }

                    // invalid parameter for this method. Throw error
                    else
                    {
                        String errMsg = "Incorrect input parameter "+ paramName
                             + " for method " + methodName.getString();

                        throw CIMOperationFailedException(errMsg);
                    }
                }  // end input parameter loop
            }  // end of setTestParameters

            // Set output Parameter values for test Parameters
            outParameters.append(CIMParamValue("debugMode",
                CIMValue(Boolean(_debugMode))));
            outParameters.append(CIMParamValue("countToDeliver",
                CIMValue(Uint32(_instancesToDeliverCounter))));
        }

        else
        {
            // Incorrect method name
            String errMsg = "Incorrect method name " + methodName.getString();

            CIMError err(PegasusOwningEntityName,
                "MSG0004",
                "TestErrorProvider::invokeMethod(). "
                "Incorect method name.",
                CIMError::PERCEIVED_SEVERITY_FATAL,
                CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
                CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);

            throw CIMException(CIM_ERR_FAILED,
                errMsg,
                err.getInstance());
        }
    }

    else
    {
        String errMsg = "Incorrect className on invokeMethod. "
            + objectReference.getClassName().getString();

        throw CIMOperationFailedException(errMsg);
    }
    if (outParameters.size() != 0)
    {
        handler.deliverParamValue(outParameters);
    }
    handler.deliver(rtnCode);
    handler.complete();
}
PEGASUS_NAMESPACE_END
