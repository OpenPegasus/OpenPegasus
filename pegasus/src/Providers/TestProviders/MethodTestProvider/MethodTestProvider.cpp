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

#include "MethodTestProvider.h"

PEGASUS_NAMESPACE_BEGIN

MethodTestProvider::MethodTestProvider()
{
}

MethodTestProvider::~MethodTestProvider()
{
}

void MethodTestProvider::initialize(CIMOMHandle& cimom)
{
    _cimom = cimom;
}

void MethodTestProvider::terminate()
{
    delete this;
}

void MethodTestProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    if (!objectReference.getClassName().equal("Test_MethodProviderClass"))
    {
        throw CIMNotSupportedException(
            objectReference.getClassName().getString());
    }

    handler.processing();
    if (methodName.equal("getIdentity"))
    {
        IdentityContainer container(context.get(IdentityContainer::NAME));
        String userName(container.getUserName());
        handler.deliver(CIMValue(userName));
    }

    /* Test1 method recalls the provider with test2 as the name and
       returns the result from test2
    */
    else if (methodName.equal("test1"))
    {
        CIMValue returnValue;
        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        // set the input values for the test2 invoke method
        inParams.append(CIMParamValue("InParam1", CIMValue(Uint32(1))));
        inParams.append(CIMParamValue("InParam2", CIMValue(Uint32(2))));

        // Make cimom handle invokementod request with input parameters.
        CIMObjectPath localReference = CIMObjectPath(
            String(),
            CIMNamespaceName(),
            objectReference.getClassName(),
            objectReference.getKeyBindings());

        returnValue = _cimom.invokeMethod(
            context,
            objectReference.getNameSpace(),
            localReference,
            CIMName("test2"),
            inParams,
            outParams);

        Uint32 rc;
        returnValue.get(rc);
        if (rc != 10)
        {
            throw CIMOperationFailedException("Incorrect return value");
        }

        if (outParams.size() != 2)
        {
            throw CIMOperationFailedException("Incorrect out parameters");
        }

        Uint32 outParam1 = 0;
        Uint32 outParam2 = 0;

        for (Uint32 i = 0; i < 2; i++)
        {
            if (outParams[i].getParameterName() == "OutParam1")
            {
                outParams[i].getValue().get(outParam1);
            }
            else if (outParams[i].getParameterName() == "OutParam2")
            {
                outParams[i].getValue().get(outParam2);
            }
            else
            {
                throw CIMOperationFailedException("Incorrect out parameters");
            }
        }

        if ((outParam1 != 21) || (outParam2 != 32))
        {
            throw CIMOperationFailedException("Incorrect out parameters");
        }

        handler.deliverParamValue(outParams[0]);
        handler.deliverParamValue(outParams[1]);
        handler.deliver(returnValue);
    }
    /* Test2 returns the value in the two inParams put into the two outparams
    */
    else if (methodName.equal("test2"))
    {
        if (inParameters.size() != 2)
        {
            throw CIMOperationFailedException("Incorrect in parameters");
        }

        Uint32 inParam1 = 0;
        Uint32 inParam2 = 0;

        for (Uint32 i = 0; i < 2; i++)
        {
            if (inParameters[i].getParameterName() == "InParam1")
            {
                inParameters[i].getValue().get(inParam1);
            }
            else if (inParameters[i].getParameterName() == "InParam2")
            {
                inParameters[i].getValue().get(inParam2);
            }
            else
            {
                throw CIMOperationFailedException("Incorrect in parameters");
            }
        }

        if ((inParam1 != 1) || (inParam2 != 2))
        {
            throw CIMOperationFailedException("Incorrect in parameters");
        }

        handler.deliverParamValue(CIMParamValue("OutParam1", Uint32(21)));
        handler.deliverParamValue(CIMParamValue("OutParam2", Uint32(32)));
        handler.deliver(Uint32(10));
    }

    /* Test 3 is a test of inputting and outputting single ref parameters.
       We receive the parameters in InParam1 and 2 and expect to return
       exactly the same references in the corresponding outParams
    */
    else if (methodName.equal("test3"))
    {

        CIMObjectPath on1(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Patrick\""));

        CIMObjectPath on2(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Rafter\""));

        if (inParameters.size() != 3)
        {
            throw CIMOperationFailedException("Incorrect in parameters");
        }

        CIMObjectPath inParam1;
        CIMObjectPath inParam2;
        Boolean recursed = false;

        for (Uint32 i = 0; i < 3; i++)
        {
            if (inParameters[i].getParameterName() == "InParam1")
            {
                inParameters[i].getValue().get(inParam1);
            }
            else if (inParameters[i].getParameterName() == "InParam2")
            {
                inParameters[i].getValue().get(inParam2);
            }
            else if (inParameters[i].getParameterName() == "Recursed")
            {
                inParameters[i].getValue().get(recursed);
            }
            else
            {

                throw CIMOperationFailedException("Incorrect in parameters");
            }

        }

        if (recursed == false)
        {
            recursed = true;
            Array<CIMParamValue> recursedInParams;
            Array<CIMParamValue> recursedOutParams;
            CIMValue returnValue;

            // set the input parameters for the test2 invoke method
            // Marks recursed true
            recursedInParams.append(CIMParamValue("InParam1", inParam1));
            recursedInParams.append(CIMParamValue("InParam2", inParam2));
            // Indicate that this is recursed call.
            recursedInParams.append(CIMParamValue("Recursed",
                                                  CIMValue(Boolean(true))));

            if ((inParam1 != on1) || (inParam2 != on2))
            {
                throw CIMOperationFailedException("Incorrect in parameters");
            }

            // reconstruct object path for cimom_handle call
            CIMObjectPath localReference = CIMObjectPath(
                String(),
                CIMNamespaceName(),
                objectReference.getClassName(),
                objectReference.getKeyBindings());

            // Make cimom handle invokemented request with input parameters.
            returnValue = _cimom.invokeMethod(
                context,
                objectReference.getNameSpace(),
                localReference,
                CIMName("test3"),
                recursedInParams,
                recursedOutParams);

            Uint32 rc;
            returnValue.get(rc);
            if (rc != 10)
            {
                throw CIMOperationFailedException("Incorrect return value");
            }

            if (recursedOutParams.size() != 3)
            {
                throw CIMOperationFailedException("Incorrect  out parameters");
            }
            // Scan the returned  parameters from the recursive call and
            // set them up in place of the original parameters.
            for (Uint32 i = 0; i < recursedOutParams.size(); i++)
            {
                if (recursedOutParams[i].getParameterName() == "OutParam1")
                {
                    recursedOutParams[i].getValue().get(inParam1);
                }
                else if (recursedOutParams[i].getParameterName() == "OutParam2")
                {
                    recursedOutParams[i].getValue().get(inParam2);
                }
                else if (recursedOutParams[i].getParameterName() == "Recursed")
                {
                    recursedOutParams[i].getValue().get(recursed);
                }
                else
                {
                    throw CIMOperationFailedException("Incorrect Rtnd params");
                }
            }
        }
        if ((inParam1 != on1) || (inParam2 != on2))
        {
            throw CIMOperationFailedException("Incorrect in parameters");
        }

        // Return exactly the same cimobjects we received.
        handler.deliverParamValue(CIMParamValue("OutParam1", inParam1));
        handler.deliverParamValue(CIMParamValue("OutParam2", inParam2));
        handler.deliverParamValue(CIMParamValue("Recursed", recursed));
        handler.deliver(Uint32(10));
    }

    /* Test 4 is a test of inputting and outputting arrays of reference
       parameters.
       We receive the parameters in InParam1 and 2 and expect to return
       exactly the same references in the corresponding outParams
    */
    else if (methodName.equal("test4"))
    {

        CIMObjectPath inParamA1(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Patrick\""));

        CIMObjectPath inParamA2(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Rafter\""));

        CIMObjectPath inParamB1(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Fred\""));

        CIMObjectPath inParamB2(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"John\""));

        Array<CIMObjectPath> InParamAArray;
        InParamAArray.append(inParamA1);
        InParamAArray.append(inParamA2);
        CIMValue inParam1Test(InParamAArray);

        Array<CIMObjectPath> InParamBArray;
        InParamBArray.append(inParamB1);
        InParamBArray.append(inParamB2);
        CIMValue inParam2Test(InParamBArray);

        if (inParameters.size() != 2)
        {
            throw CIMOperationFailedException("Incorrect in parameters");
        }

        Array<CIMObjectPath> inParam1;
        Array<CIMObjectPath> inParam2;

        for (Uint32 i = 0; i < 2; i++)
        {
            if (inParameters[i].getParameterName() == "InParam1")
            {
                inParameters[i].getValue().get(inParam1);
            }
            else if (inParameters[i].getParameterName() == "InParam2")
            {
                inParameters[i].getValue().get(inParam2);
            }
            else
            {
                throw CIMOperationFailedException("Incorrect in parameters");
            }
        }

        if ((inParam1 != inParam1Test) || (inParam2 != inParam2Test))
        {
            throw CIMOperationFailedException("Incorrect in parameters");
        }

        // Return exactly the same cimobjects we received.
        handler.deliverParamValue(CIMParamValue("OutParam1", inParam1));
        handler.deliverParamValue(CIMParamValue("OutParam2", inParam2));

        handler.deliver(Uint32(10));
    }
    /* Test 5 is for testing the out parameters whose size exceeds object
       response threshold count (default 100).
    */
    else if (methodName.equal("test5"))
    {
        for (Uint32 i = 0; i <  101; ++i)
        {
            handler.deliverParamValue(
                CIMParamValue("OutParam", String("OutParamTest")));
            handler.deliver(Uint32(0));
        }
    }
    else
    {
        throw CIMNotSupportedException(methodName.getString());
    }

    handler.complete();
}

PEGASUS_NAMESPACE_END
