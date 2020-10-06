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

#include <Pegasus/Common/AtomicInt.h>
#include <cstring>
#include "IndicationStressTestConsumer.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

static AtomicInt _indicationCount(0);
static AtomicInt _indicationCountFromExpectedIdentity(0);
static String _expectedSenderIdentity;

IndicationStressTestConsumer::IndicationStressTestConsumer()
{
}

IndicationStressTestConsumer::~IndicationStressTestConsumer()
{
}

void IndicationStressTestConsumer::initialize(CIMOMHandle& handle)
{
}

void IndicationStressTestConsumer::terminate()
{
    delete this;
}

void IndicationStressTestConsumer::invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & objectReference,
        const CIMName & methodName,
        const Array<CIMParamValue> & inParameters,
        MethodResultResponseHandler & handler)
{
    handler.processing();

    if (methodName.equal ("setupTestConfiguration"))
    {
        _indicationCount = 0;
        _indicationCountFromExpectedIdentity = 0;
        if (inParameters.size() > 1)
        {
            throw CIMInvalidParameterException("Too many parameters.");
        }
        else if (inParameters.size() == 0)
        {
            _expectedSenderIdentity = String::EMPTY;
        }
        else
        {
            CIMValue paramVal = inParameters[0].getValue();
            paramVal.get(_expectedSenderIdentity);
        }
        handler.deliver(CIMValue((Uint32)0));
    }
    else if (methodName.equal("getTestResults"))
    {
        handler.deliverParamValue(CIMParamValue("indicationsReceived",
                CIMValue(_indicationCount.get())));
        handler.deliverParamValue(CIMParamValue(
                "indicationsReceivedFromExpectedIdentity",
                CIMValue(_indicationCountFromExpectedIdentity.get())));
        handler.deliver(CIMValue((Uint32)0));
    }
    else
    {
        throw CIMNotSupportedException(String::EMPTY);
    }

     handler.complete();
}

//
// Writes all the properties of any indications it receives to
// file indicationLog. The indicationLog file is located in a
// directory specified by the environment variable
// PEGASUS_DISPLAYCONSUMER_DIR (defaults to PEGASUS_HOME).
//
// If the PEGASUS_DISPLAYCONSUMER_DIR is set to "console", the
// output will be written to stdout.
//
// If the PEGASUS_DISPLAYCONSUMER_DIR is set to "nooutput", then
// no output will be produced.
//
void IndicationStressTestConsumer::consumeIndication(
    const OperationContext & context,
    const String& url,
    const CIMInstance& indicationInstance)
{
    String userName;
    IdentityContainer container(context.get(IdentityContainer::NAME));
    userName.assign(container.getUserName());
    if (String::equal(userName,_expectedSenderIdentity))
    {
       _indicationCountFromExpectedIdentity++;
    }
    _indicationCount++;

    String indicationFile = INDICATION_DIR;
    if (strcmp(indicationFile.getCString(), "nooutput") == 0)
    {
        return;
    }

    indicationFile.append("/IndicationStressTestLog");
    FILE *_indicationLogHandle = fopen(indicationFile.getCString(), "a+");
    if (_indicationLogHandle == NULL)
    {
        return;
    }

    fprintf(_indicationLogHandle,
        "+++++ Received Indication +++++\n");
    fprintf(_indicationLogHandle,
            "userName = %s\n", (const char *)userName.getCString());

    for (Uint8 i=0; i < indicationInstance.getPropertyCount(); i++)
    {
        CIMConstProperty property = indicationInstance.getProperty(i);

        String _propertyName = property.getName().getString();
        _propertyName.append(" = ");
        fprintf(_indicationLogHandle, "%s",
                (const char *)_propertyName.getCString());

        CIMValue propertyValue = property.getValue();
        if (!propertyValue.isNull())
        {
            switch (propertyValue.getType())
            {
                case CIMTYPE_UINT8:
                    Uint8 propertyValueUint8;
                    propertyValue.get(propertyValueUint8);
                    fprintf(_indicationLogHandle, "%u\n",
                            propertyValueUint8);
                    break;

                case CIMTYPE_UINT16:
                    Uint16 propertyValueUint16;
                    propertyValue.get(propertyValueUint16);
                    fprintf(_indicationLogHandle, "%u\n",
                            propertyValueUint16);
                    break;

                case CIMTYPE_UINT32:
                    Uint32 propertyValueUint32;
                    propertyValue.get(propertyValueUint32);
                    fprintf(_indicationLogHandle, "%u\n",
                            propertyValueUint32);
                    break;

                case CIMTYPE_UINT64:
                    Uint64 propertyValueUint64;
                    propertyValue.get(propertyValueUint64);
                    fprintf(
                            _indicationLogHandle,
                            "%" PEGASUS_64BIT_CONVERSION_WIDTH "u\n",
                            propertyValueUint64);
                    break;

                case CIMTYPE_SINT8:
                    Sint8 propertyValueSint8;
                    propertyValue.get(propertyValueSint8);
                    fprintf(_indicationLogHandle, "%d\n",
                            propertyValueSint8);
                    break;

                case CIMTYPE_SINT16:
                    Sint16 propertyValueSint16;
                    propertyValue.get(propertyValueSint16);
                    fprintf(_indicationLogHandle, "%d\n",
                            propertyValueSint16);
                    break;

                case CIMTYPE_SINT32:
                    Sint32 propertyValueSint32;
                    propertyValue.get(propertyValueSint32);
                        fprintf(_indicationLogHandle, "%d\n",
                            propertyValueSint32);
                    break;

                case CIMTYPE_SINT64:
                    Sint64 propertyValueSint64;
                    propertyValue.get(propertyValueSint64);
                        fprintf(
                            _indicationLogHandle,
                            "%" PEGASUS_64BIT_CONVERSION_WIDTH "d\n",
                            propertyValueSint64);
                    break;

                case CIMTYPE_REAL32:
                    Real32 propertyValueReal32;
                    propertyValue.get(propertyValueReal32);
                        fprintf(_indicationLogHandle, "%f\n",
                            propertyValueReal32);
                    break;

                case CIMTYPE_REAL64:
                    Real64 propertyValueReal64;
                    propertyValue.get(propertyValueReal64);
                    fprintf(_indicationLogHandle, "%f\n", propertyValueReal64);
                    break;

                case CIMTYPE_BOOLEAN :
                    Boolean booleanValue;
                    propertyValue.get(booleanValue);
                    fprintf(_indicationLogHandle, "%d\n", booleanValue);
                    break;

                case CIMTYPE_CHAR16:
                case CIMTYPE_STRING :
                    fprintf(_indicationLogHandle, "%s\n",
                        (const char *)
                            propertyValue.toString().getCString());
                    break;

                case CIMTYPE_DATETIME :
                    {
                        CIMDateTime propertyValueDateTime;
                        propertyValue.get(propertyValueDateTime);
                        fprintf(_indicationLogHandle, "%s\n",
                            (const char *)propertyValueDateTime.
                                toString().getCString());
                        break;
                    }

                case CIMTYPE_OBJECT :
                    {
                        CIMObject propertyValueObject;
                        propertyValue.get(propertyValueObject);
                        fprintf(_indicationLogHandle, "%s\n",
                            (const char *) propertyValueObject.toString().
                                getCString());
                        break;
                    }

                default:
                    break;
            }
        }
    }

    fprintf(_indicationLogHandle,
        "++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
    fclose(_indicationLogHandle);
}

PEGASUS_NAMESPACE_END
