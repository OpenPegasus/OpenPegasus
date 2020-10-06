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

#include "LocalRepository.h"

PEGASUS_USING_PEGASUS;

LocalRepository::LocalRepository(void) : context(0)
{
    // the SimpleDeclContext object does not handle inheritance
    // so it is necessary to create super-class
    // qualifiers and properties in subclasses.
    context = new SimpleDeclContext;

    // create essential qualifiers
    CIMQualifierDecl abstractQualifier("Abstract", Boolean(true),
                                       CIMScope::CLASS,
                                       CIMFlavor::NONE);
    CIMQualifierDecl keyQualifier("Key", Boolean(true),
                                   (CIMScope::PROPERTY +
                                   CIMScope::REFERENCE),
                                   CIMFlavor::TOSUBCLASS);

    context->addQualifierDecl("test_namespace", abstractQualifier);
    context->addQualifierDecl("test_namespace", keyQualifier);

    // create base class
    {
        CIMClass classA("ClassA");

        classA.addQualifier(CIMQualifier("Abstract", Boolean(true)));

        CIMProperty property1("Property1", CIMValue(CIMTYPE_UINT32, 0, 0), 0,
                               CIMName(), "ClassA");

        classA.addProperty(property1);

        CIMProperty property2("Property2", CIMValue(String("default_value")), 0,
                               CIMName(), "ClassA");

        classA.addProperty(property2);

        CIMProperty property3("Property3", CIMValue(CIMTYPE_DATETIME, 0, 0), 0,
                               CIMName(), "ClassA");

        classA.addProperty(property3);

        // add class to namespace
        context->addClass("test_namespace", classA);
    }

    // create sub-class
    {
        CIMClass classB("ClassB", "ClassA");

        classB.addQualifier(CIMQualifier("Abstract", Boolean(true)));

        CIMProperty property1("Property1", CIMValue(CIMTYPE_UINT32, 0, 0), 0,
                               CIMName(), "ClassA");

        property1.addQualifier(CIMQualifier("Key", Boolean(true)));

        classB.addProperty(property1);

        CIMProperty property2("Property2", CIMValue(String("default_value")), 0,
                               CIMName(), "ClassA");

        property2.addQualifier(CIMQualifier("Key", Boolean(true)));

        classB.addProperty(property2);

        CIMProperty property3("Property3", CIMValue(CIMTYPE_DATETIME, 0, 0), 0,
                               CIMName(), "ClassA");

        property3.addQualifier(CIMQualifier("Key", Boolean(true)));

        classB.addProperty(property3);

        CIMProperty property4("Property4", CIMValue(String("default_value")), 0,
                               CIMName(), "ClassB");

        classB.addProperty(property4);

        // add class to namespace
        context->addClass("test_namespace", classB);
    }

    // create sub-class
    {
        CIMClass classC("ClassC", "ClassB");

        classC.addQualifier(CIMQualifier("Description", String("")));

        CIMProperty property1("Property1", CIMValue(CIMTYPE_UINT32, 0, 0), 0,
                               CIMName(), "ClassA");

        property1.addQualifier(CIMQualifier("Key", Boolean(true)));
        property1.addQualifier(CIMQualifier("Description", String("")));

        classC.addProperty(property1);

        CIMProperty property2("Property2", CIMValue(String("default_value")), 0,
                               CIMName(), "ClassA");

        property2.addQualifier(CIMQualifier("Key", Boolean(true)));
        property2.addQualifier(CIMQualifier("Description", String("")));

        classC.addProperty(property2);

        CIMProperty property3("Property3", CIMValue(CIMTYPE_DATETIME, 0, 0), 0,
                               CIMName(), "ClassA");

        property3.addQualifier(CIMQualifier("Key", Boolean(true)));
        property3.addQualifier(CIMQualifier("Description", String("")));

        classC.addProperty(property3);

        CIMProperty property4("Property4", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                               CIMName(), "ClassB");

        property4.addQualifier(CIMQualifier("Description", String("")));

        classC.addProperty(property4);

        CIMProperty property5("Property5", CIMValue(CIMTYPE_REAL32, 0, 0), 0,
                               CIMName(), "ClassC");

        classC.addProperty(property5);

        CIMProperty property6("Property6", CIMValue(CIMTYPE_REAL64, 0, 0), 0,
                               CIMName(), "ClassC");

        classC.addProperty(property6);

        CIMProperty property7("Property7", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                               CIMName(), "ClassC");

        classC.addProperty(property7);

        CIMProperty property8("Property8", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                               CIMName(), "ClassC");

        classC.addProperty(property8);

        CIMProperty property9("Property9", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                               CIMName(), "ClassC");

        classC.addProperty(property9);

        CIMProperty property10("Property10", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property10);

        CIMProperty property11("Property11", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property11);

        CIMProperty property12("Property12", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property12);

        CIMProperty property13("Property13", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property13);

        CIMProperty property14("Property14", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property14);

        CIMProperty property15("Property15", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property15);

        CIMProperty property16("Property16", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property16);

        CIMProperty property17("Property17", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property17);

        CIMProperty property18("Property18", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property18);

        CIMProperty property19("Property19", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property19);

        CIMProperty property20("Property20", CIMValue(CIMTYPE_STRING, 0, 0), 0,
                                CIMName(), "ClassC");

        classC.addProperty(property20);

        // add class to namespace
        context->addClass("test_namespace", classC);
    }

    // TODO: add association object

    // TODO: add indication object

    // TODO: add object with embedded object
}

LocalRepository::~LocalRepository(void)
{
    delete context;
}

CIMClass LocalRepository::getClass(
    const String & nameSpace,
    const String & className,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList & propertyList)
{
    // create a duplicate object before modifying
    CIMClass cimClass = context->lookupClass("test_namespace",
                                             className).clone();

    if(!includeQualifiers)
    {
        // remove qualifiers from class
        for(Uint32 i = 0, n = cimClass.getQualifierCount(); i < n; i++)
        {
            cimClass.removeQualifier(i);
        }

        // remove qualifiers from properties
        for(Uint32 i = 0, n = cimClass.getPropertyCount(); i < n; i++)
        {
            CIMProperty cimProperty = cimClass.getProperty(i);

            for(Uint32 j = 0, m = cimProperty.getQualifierCount(); j < m; j++)
            {
                cimProperty.removeQualifier(j);
            }
        }
    }

    if(!includeClassOrigin)
    {
        // remove class origin
        for(Uint32 i = 0, n = cimClass.getPropertyCount(); i < n; i++)
        {
            cimClass.getProperty(i).setClassOrigin(CIMName());
        }
    }

    cimClass.setPath(CIMObjectPath("localhost", "test_namespace",
                                   cimClass.getClassName()));

    return(cimClass);
}
