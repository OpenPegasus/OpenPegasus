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
#include "SimpleDisplayConsumer.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;


SimpleDisplayConsumer::SimpleDisplayConsumer()
{
}

SimpleDisplayConsumer::~SimpleDisplayConsumer()
{
}

void SimpleDisplayConsumer::initialize(CIMOMHandle& handle)
{
}

void SimpleDisplayConsumer::terminate()
{
    delete this;
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
void SimpleDisplayConsumer::consumeIndication(
    const OperationContext& context,
    const String& url,
    const CIMInstance& indicationInstance)
{
    String indicationFile = INDICATION_DIR;
    FILE* indicationLogHandle = 0;

    Boolean printOnConsole;

    if (strcmp(indicationFile.getCString(), "nooutput") == 0)
    {
        return;
    }
    if (strcmp(indicationFile.getCString(), "console") == 0)
    {
        printOnConsole = true;
    }
    else
    {
        printOnConsole = false;
    }

    if (!printOnConsole)
    {
        indicationFile.append("/indicationLog");
        indicationLogHandle = fopen(indicationFile.getCString(), "a+");
    }

    if (printOnConsole)
    {
        cout << endl;
        cout << "++++++++++++++ Received Indication +++++++++++++++++" << endl;
    }
    else
    {
        if (indicationLogHandle == NULL)
        {
            fprintf(
                stderr,
                "Failed to open file %s \n",
                (const char *) indicationFile.getCString());
            return;
        }
        else
        {
            fprintf(
                indicationLogHandle,
                "++++++++++++++ Received Indication +++++++++++++++++\n");
        }
    }

    CIMInstance indication = indicationInstance.clone();

    for (Uint8 i = 0; i < indicationInstance.getPropertyCount(); i++)
    {
        CIMProperty property = indication.getProperty(i);
        CIMValue propertyValue = property.getValue();
        CIMType type = propertyValue.getType ();
        Boolean valueIsNull = propertyValue.isNull();

        String _propertyName = property.getName().getString();
        _propertyName.append(" = ");
        if (printOnConsole)
        {
            cout << _propertyName;
        }
        else if (indicationLogHandle != NULL)
        {
            fprintf(
                indicationLogHandle,
                "%s", (const char*)_propertyName.getCString());
        }

        if (!valueIsNull)
        {
            switch (type)
            {
                case CIMTYPE_UINT8:
                    Uint8 propertyValueUint8;
                    propertyValue.get(propertyValueUint8);
                    if (printOnConsole)
                    {
                        cout << propertyValueUint8 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%u\n", propertyValueUint8);
                    }
                    break;

                case CIMTYPE_UINT16:
                    Uint16 propertyValueUint16;
                    propertyValue.get(propertyValueUint16);
                    if (printOnConsole)
                    {
                        cout << propertyValueUint16 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%u\n", propertyValueUint16);
                    }
                    break;

                case CIMTYPE_UINT32:
                    Uint32 propertyValueUint32;
                    propertyValue.get(propertyValueUint32);
                    if (printOnConsole)
                    {
                        cout << propertyValueUint32 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%u\n", propertyValueUint32);
                    }
                    break;

                case CIMTYPE_UINT64:
                    Uint64 propertyValueUint64;
                    propertyValue.get(propertyValueUint64);
                    if (printOnConsole)
                    {
                        cout << (unsigned short)propertyValueUint64 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%" PEGASUS_64BIT_CONVERSION_WIDTH "u\n",
                            propertyValueUint64);
                    }
                    break;

                case CIMTYPE_SINT8:
                    Sint8 propertyValueSint8;
                    propertyValue.get(propertyValueSint8);
                    if (printOnConsole)
                    {
                        cout << propertyValueSint8 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%d\n", propertyValueSint8);
                    }
                    break;

                case CIMTYPE_SINT16:
                    Sint16 propertyValueSint16;
                    propertyValue.get(propertyValueSint16);
                    if (printOnConsole)
                    {
                        cout << propertyValueSint16 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%d\n", propertyValueSint16);
                    }
                    break;

                case CIMTYPE_SINT32:
                    Sint32 propertyValueSint32;
                    propertyValue.get(propertyValueSint32);
                    if (printOnConsole)
                    {
                        cout << propertyValueSint32 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%d\n", propertyValueSint32);
                    }
                    break;

                case CIMTYPE_SINT64:
                    Sint64 propertyValueSint64;
                    propertyValue.get(propertyValueSint64);
                    if (printOnConsole)
                    {
                        cout << (unsigned short)propertyValueSint64 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%" PEGASUS_64BIT_CONVERSION_WIDTH "d\n",
                            propertyValueSint64);
                    }
                    break;

                case CIMTYPE_REAL32:
                    Real32 propertyValueReal32;
                    propertyValue.get(propertyValueReal32);
                    if (printOnConsole)
                    {
                        cout << propertyValueReal32 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%f\n", propertyValueReal32);
                    }
                    break;

                case CIMTYPE_REAL64:
                    Real64 propertyValueReal64;
                    propertyValue.get(propertyValueReal64);
                    if (printOnConsole)
                    {
                        cout << propertyValueReal64 << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%f\n", propertyValueReal64);
                    }
                    break;

                case CIMTYPE_BOOLEAN:
                    Boolean booleanValue;
                    propertyValue.get(booleanValue);
                    if (printOnConsole)
                    {
                        cout << booleanValue << endl;
                    }
                    else
                    {
                        fprintf(indicationLogHandle, "%d\n", booleanValue);
                    }
                    break;

                case CIMTYPE_CHAR16:
                case CIMTYPE_STRING:
                    if (printOnConsole)
                    {
                        cout << propertyValue.toString() << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%s\n",
                            (const char*)propertyValue.toString().getCString());
                    }
                    break;

                case CIMTYPE_DATETIME:
                {
                    CIMDateTime propertyValueDateTime;
                    propertyValue.get(propertyValueDateTime);
                    if (printOnConsole)
                    {
                        cout << propertyValueDateTime.toString () << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%s\n",
                            (const char*)propertyValueDateTime.toString().
                                getCString());
                    }
                    break;
                }

                case CIMTYPE_OBJECT:
                {
                    CIMObject propertyValueObject;
                    propertyValue.get(propertyValueObject);
                    if (printOnConsole)
                    {
                        cout << propertyValueObject.toString () << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%s\n",
                            (const char*)propertyValueObject.toString().
                                getCString());
                    }
                    break;
                }
                case CIMTYPE_INSTANCE:
                {
                    CIMInstance propertyValueInstance;
                    propertyValue.get(propertyValueInstance);
                    CIMObject printableObject(propertyValueInstance);
                    if (printOnConsole)
                    {
                        cout << printableObject.toString () << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%s\n",
                            (const char*)printableObject.toString().
                                getCString());
                    }
                    break;
                }
                case CIMTYPE_REFERENCE:
                {
                    //
                    //  ATTN: reference properties are not allowed in
                    //        indications; print to log or console
                    //
                    CIMObjectPath propertyValueReference;
                    propertyValue.get(propertyValueReference);
                    if (printOnConsole)
                    {
                        cout << propertyValueReference.toString() << endl;
                    }
                    else
                    {
                        fprintf(
                            indicationLogHandle,
                            "%s\n",
                            (const char*)propertyValueReference.toString().
                                getCString());
                    }
                    break;
                }
            }
        }
    }

    if (printOnConsole)
    {
        cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    }
    else
    {
        fprintf(
            indicationLogHandle,
            "++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
        fclose(indicationLogHandle);
    }
}

PEGASUS_NAMESPACE_END
