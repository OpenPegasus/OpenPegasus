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

#include <Pegasus/Common/ObjectNormalizer.h>
#include <Pegasus/Common/XmlWriter.h>

#include <Pegasus/General/Stopwatch.h>

#include "LocalRepository.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

static LocalRepository * repository = 0;
static Stopwatch _stopwatch;

#define PRINT(x) if(verbose) cout << x << endl;

//
// Basic ObjectNormalizer tests
//

// test improper ObjectNormalizer seeding
void Test001a(void)
{
    PRINT("Test001a");

    CIMClass cimClass;

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        false,
        false,
        CIMNamespaceName(String("root")),
        nullContext);

    normalizer.processClassObjectPath(CIMObjectPath());

    normalizer.processInstanceObjectPath(CIMObjectPath());

    normalizer.processInstance(CIMInstance());
}

// test bad arguments
void Test001b(void)
{
    PRINT("Test001b");

    PRINT("Explicity disabled.");

    /*
    // ATTN: the following test has been disabled because the initialized object
    // check was moved to the response handlers.
    ObjectNormalizer normalizer(
        CIMClass("CIM_ManagedElement"),
        false,
        false);

    try
    {
        normalizer.processClassObjectPath(CIMObjectPath());

        throw Exception("Failed to detect null class object path.");
    }
    catch(CIMException & e)
    {
        PRINT("expected CIMException: " << e.getMessage());
    }

    try
    {
        normalizer.processInstanceObjectPath(CIMObjectPath());

        throw Exception("Failed to detect null instance object path.");
    }
    catch(CIMException & e)
    {
        PRINT("expected CIMException: " << e.getMessage());
    }

    try
    {
        normalizer.processInstance(CIMInstance());

        throw Exception("Failed to detect null instance.");
    }
    catch(CIMException & e)
    {
        PRINT("expected CIMException: " << e.getMessage());
    }
    */
}

//
// object path failures
//

// class object path (null object path)
void Test002a(void)
{
    PRINT("Test002a");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    try
    {
        _stopwatch.start();

        CIMObjectPath normalizedObjectPath =
             normalizer.processClassObjectPath(cimObjectPath);

        _stopwatch.stop();

        throw Exception("Failed to detect a null class object path.");
    }
    catch(CIMException & e)
    {
        _stopwatch.stop();

        PRINT("expected CIMException: " << e.getMessage());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// class object path (incorrect class name)
void Test002b(void)
{
    PRINT("Test002b");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    // use different case to test that too
    cimObjectPath.setClassName("ClassBAD");

    try
    {
        _stopwatch.start();

        CIMObjectPath normalizedObjectPath =
             normalizer.processClassObjectPath(cimObjectPath);

        _stopwatch.stop();

        throw Exception("Failed to detect class object path with"
                        " incorrect class name.");
    }
    catch(CIMException & e)
    {
        _stopwatch.stop();

        PRINT("expected CIMException: " << e.getMessage());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.")
}

// instance object path (incorrect class name)
void Test002c(void)
{
    PRINT("Test002c");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    cimObjectPath.setClassName("ClassBAD");

    // no keys

    try
    {
        _stopwatch.start();

        CIMObjectPath normalizedObjectPath =
             normalizer.processInstanceObjectPath(cimObjectPath);

        _stopwatch.stop();

        throw Exception("Failed to detect instance object path with"
                        " incorrect class name.");
    }
    catch(CIMException & e)
    {
        _stopwatch.stop();

        PRINT("expected CIMException: " << e.getMessage());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// instance object path (no key properties in instance, no keys in object path)
void Test002d(void)
{
    PRINT("Test002d");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    cimObjectPath.setClassName(cimClass.getClassName());

    // no keys

    try
    {
        _stopwatch.start();

        CIMObjectPath normalizedObjectPath =
             normalizer.processInstanceObjectPath(cimObjectPath);

        _stopwatch.stop();

        throw Exception("Failed to detect instance object path with no key"
                        " properties and no keys.");
    }
    catch(CIMException & e)
    {
        _stopwatch.stop();

        PRINT("expected CIMException: " << e.getMessage());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

//
// object path successes
//

// class object path (normal)
void Test003a(void)
{
    PRINT("Test003a");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    cimObjectPath.setClassName(cimClass.getClassName());

    _stopwatch.start();

    CIMObjectPath normalizedObjectPath =
         normalizer.processClassObjectPath(cimObjectPath);

    _stopwatch.stop();

    if(verbose)
    {
        cout << normalizedObjectPath.toString() << endl;
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// class object path (with erroneous and extra information)
void Test003b(void)
{
    PRINT("Test003b");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    // use lowercase. normalization should fix case
    cimObjectPath.setClassName("classc");

    // fake keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("FakeProperty1",
                               CIMValue(String("junk"))));
    keys.append(CIMKeyBinding("FakeProperty2",
                               CIMValue(String("more junk"))));

    cimObjectPath.setKeyBindings(keys);

    _stopwatch.start();

    CIMObjectPath normalizedObjectPath =
         normalizer.processClassObjectPath(cimObjectPath);

    _stopwatch.stop();

    PRINT(normalizedObjectPath.toString());

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// instance object path (normal)
void Test003c(void)
{
    PRINT("Test003c");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    // use lowercase. normalization should fix case
    cimObjectPath.setClassName("classc");

    // simple keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding(
        "property1", CIMValue(Uint32(1))));
    keys.append(CIMKeyBinding(
        "property2", CIMValue(String("Test Instance #003c"))));
    keys.append(CIMKeyBinding(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    cimObjectPath.setKeyBindings(keys);

    _stopwatch.start();

    CIMObjectPath normalizedObjectPath =
         normalizer.processInstanceObjectPath(cimObjectPath);

    _stopwatch.stop();

    PRINT(normalizedObjectPath.toString());

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// instance object path (with missing information)
void Test003d(void)
{
    PRINT("Test003d");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    // use lowercase. normalization should fix case
    cimObjectPath.setClassName("classc");

    // keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("property1",
                               CIMValue(Uint32(1))));
    //keys.append(CIMKeyBinding("property2",
    //  CIMValue(String("Test Instance #003d"))));
    keys.append(CIMKeyBinding("property3",
                               CIMValue(CIMDateTime::getCurrentDateTime())));

    cimObjectPath.setKeyBindings(keys);

    try
    {
        _stopwatch.start();

        CIMObjectPath normalizedObjectPath =
            normalizer.processInstanceObjectPath(cimObjectPath);

        _stopwatch.stop();

        throw Exception("Failed to dected instance with missing keys.");
    }
    catch(CIMException & e)
    {
        _stopwatch.stop();

        PRINT("expected CIMException: " << e.getMessage());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// instance object path (with extra information)
void Test003e(void)
{
    PRINT("Test003e");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMObjectPath cimObjectPath;

    // use lowercase. normalization should fix case
    cimObjectPath.setClassName("classc");

    // simple keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding(
        "property1", CIMValue(Uint32(1))));
    keys.append(CIMKeyBinding(
        "property2", CIMValue(String("Test Instance #003e"))));
    keys.append(CIMKeyBinding(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    // fake keys
    keys.append(CIMKeyBinding("FakeProperty1", CIMValue(String("junk"))));
    keys.append(CIMKeyBinding("FakeProperty2", CIMValue(String("more junk"))));

    cimObjectPath.setKeyBindings(keys);

    _stopwatch.start();

    CIMObjectPath normalizedObjectPath =
        normalizer.processInstanceObjectPath(cimObjectPath);

    _stopwatch.stop();

    PRINT(normalizedObjectPath.toString());

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

//
// instance object failures
//

// instance object with no properties and no object path
void Test004a(void)
{
    PRINT("Test004a");

    _stopwatch.reset();

    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMInstance cimInstance(cimClass.getClassName());

    // no properties specified

    // no object path specified

    try
    {
        _stopwatch.start();

        CIMInstance normalizedInstance =
            normalizer.processInstance(cimInstance);

        _stopwatch.stop();

        throw Exception("Failed to dected instance with no properties"
                        " and no object path.");
    }
    catch(CIMException & e)
    {
        _stopwatch.stop();

        PRINT("expected CIMException: " << e.getMessage());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// instance object with incorrect property type
void Test004b(void)
{
    PRINT("Test004b");

    _stopwatch.reset();

    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMInstance cimInstance(cimClass.getClassName());

    // only populate keys, let the normalizer do the rest
    cimInstance.addProperty(CIMProperty(
        "property1", CIMValue(Uint32(1))));
    cimInstance.addProperty(CIMProperty(
        "property2", CIMValue(String("Test Instance #004b"))));
    cimInstance.addProperty(CIMProperty(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    // incorrect property type
    cimInstance.addProperty(CIMProperty(
        "property4", CIMValue(Uint32(0))));   // should be String

    // no object path specified

    try
    {
        _stopwatch.start();

        CIMInstance normalizedInstance =
             normalizer.processInstance(cimInstance);

        _stopwatch.stop();

        throw Exception("Failed to detect incorrect property type.");
    }
    catch(CIMException & e)
    {
        _stopwatch.stop();

        PRINT("expected CIMException: " << e.getMessage());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

//
// instance object successes
//

// instance with class origin
void Test005a(void)
{
    PRINT("Test005a");

    _stopwatch.reset();

    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = true;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMInstance cimInstance(cimClass.getClassName());

    // only populate keys, let the normalizer do the rest
    cimInstance.addProperty(CIMProperty(
        "property1", CIMValue(Uint32(1))));
    cimInstance.addProperty(CIMProperty(
        "property2", CIMValue(String("Test Instance #005a"))));
    cimInstance.addProperty(CIMProperty(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));
    cimInstance.addProperty(CIMProperty(
        "property4", CIMValue(String("Pegasus TestObjectNormalizer"))));

    // complete object path
    CIMObjectPath cimObjectPath;

    cimObjectPath.setClassName(cimInstance.getClassName());

    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding(
        "property1", CIMValue(Uint32(1))));
    keys.append(CIMKeyBinding(
        "property2", CIMValue(String("Test Instance #005b"))));
    // slightly differnt value than property. who wins?
    keys.append(CIMKeyBinding(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    cimObjectPath.setKeyBindings(keys);

    cimInstance.setPath(cimObjectPath);

    _stopwatch.start();

    CIMInstance normalizedInstance = normalizer.processInstance(cimInstance);

    _stopwatch.stop();

    if(verbose)
    {
        cout << normalizedInstance.getPath().toString() << endl;

        XmlWriter::printInstanceElement(normalizedInstance);
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// instance with object path but no properties
void Test005b(void)
{
    PRINT("Test005b");

    _stopwatch.reset();

    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMInstance cimInstance(cimClass.getClassName());

    // all properties
    cimInstance.addProperty(CIMProperty(
        "property1", CIMValue(Uint32(1))));
    cimInstance.addProperty(CIMProperty(
        "property2", CIMValue(String("Test Instance #005b"))));
    cimInstance.addProperty(CIMProperty(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));
    cimInstance.addProperty(CIMProperty(
        "property4", CIMValue(String("Pegasus TestObjectNormalizer"))));

    // complete object path
    CIMObjectPath cimObjectPath;

    cimObjectPath.setClassName(cimInstance.getClassName());

    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding(
        "property1", CIMValue(Uint32(1))));
    keys.append(CIMKeyBinding(
        "property2", CIMValue(String("Test Instance #005b"))));
    // slightly differnt value than property. who wins?
    keys.append(CIMKeyBinding(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    cimObjectPath.setKeyBindings(keys);

    cimInstance.setPath(cimObjectPath);

    _stopwatch.start();

    CIMInstance normalizedInstance =
        normalizer.processInstance(cimInstance);

    _stopwatch.stop();

    if(verbose)
    {
        cout << normalizedInstance.getPath().toString() << endl;

        XmlWriter::printInstanceElement(normalizedInstance);
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// instance with qualifiers
void Test005c(void)
{
    PRINT("Test005c");

    _stopwatch.reset();

    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMInstance cimInstance(cimClass.getClassName());

    cimInstance.addQualifier(
        CIMQualifier("Description",
             String("This object qualifier value comes from the instance")));

    // all properties
    cimInstance.addProperty(CIMProperty(
        "property1", CIMValue(Uint32(1))));
    cimInstance.addProperty(CIMProperty(
        "property2", CIMValue(String("Test Instance #005c"))));
    cimInstance.addProperty(CIMProperty(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    CIMProperty property4("property4",
                          CIMValue(String("Pegasus TestObjectNormalizer")));

    property4.addQualifier(CIMQualifier(
        "Description",
        String("This property qualifier value comes from the instance")));

    cimInstance.addProperty(property4);

    // complete object path
    CIMObjectPath cimObjectPath;

    cimObjectPath.setClassName(cimInstance.getClassName());

    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding(
        "property1", CIMValue(Uint32(1))));
    keys.append(CIMKeyBinding(
        "property2", CIMValue(String("Test Instance #005b"))));
    // slightly differnt value than property. who wins?
    keys.append(CIMKeyBinding(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    cimObjectPath.setKeyBindings(keys);

    cimInstance.setPath(cimObjectPath);

    _stopwatch.start();

    CIMInstance normalizedInstance =
        normalizer.processInstance(cimInstance);

    _stopwatch.stop();

    if(verbose)
    {
        cout << normalizedInstance.getPath().toString() << endl;

        XmlWriter::printInstanceElement(normalizedInstance);
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// include with properties containing class origin
void Test005d(void)
{
    PRINT("Test005d");

    _stopwatch.reset();

    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    CIMInstance cimInstance(cimClass.getClassName());

    // all properties
    cimInstance.addProperty(CIMProperty(
        "property1", CIMValue(Uint32(1))));
    cimInstance.addProperty(CIMProperty(
        "property2", CIMValue(String("Test Instance #005c"))));
    cimInstance.addProperty(CIMProperty(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));
    cimInstance.addProperty(CIMProperty(
        "property4", CIMValue(String("Pegasus TestObjectNormalizer"))));

    // complete object path
    CIMObjectPath cimObjectPath;

    cimObjectPath.setClassName(cimInstance.getClassName());

    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding(
        "property1", CIMValue(Uint32(1))));
    keys.append(CIMKeyBinding(
        "property2", CIMValue(String("Test Instance #005b"))));
    // slightly differnt value than property. who wins?
    keys.append(CIMKeyBinding(
        "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

    cimObjectPath.setKeyBindings(keys);

    cimInstance.setPath(cimObjectPath);

    _stopwatch.start();

    CIMInstance normalizedInstance =
        normalizer.processInstance(cimInstance);

    _stopwatch.stop();

    if(verbose)
    {
        cout << normalizedInstance.getPath().toString()
            << endl;

        XmlWriter::printInstanceElement(normalizedInstance);
    }

    PRINT("*** " << _stopwatch.getElapsed()
          << " milliseconds.");
}

//
// Test100a()
//
void Test100a(void)
{
    PRINT("Test100a");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    for(Uint32 i = 0, n = 100; i < n; i++)
    {
        CIMObjectPath cimObjectPath;

        cimObjectPath.setClassName(cimClass.getClassName());

        // simple keys
        Array<CIMKeyBinding> keys;

        keys.append(CIMKeyBinding(
            "property1", CIMValue(Uint32(1))));
        keys.append(CIMKeyBinding(
            "property2", CIMValue(String("Test Instance #005b"))));
        keys.append(CIMKeyBinding(
            "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

        cimObjectPath.setKeyBindings(keys);

        _stopwatch.start();

        CIMObjectPath normalizedObjectPath =
            normalizer.processInstanceObjectPath(cimObjectPath);

        _stopwatch.stop();

        PRINT(normalizedObjectPath.toString());
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// large instance set without qualifiers and without class origin
void Test101a(void)
{
    PRINT("Test101a");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    for(Uint32 i = 0, n = 1000; i < n; i++)
    {
        CIMInstance cimInstance(cimClass.getClassName());

        // all properties
        cimInstance.addProperty(
            CIMProperty("property1", CIMValue(Uint32(i))));
        cimInstance.addProperty(
            CIMProperty("property2",
                        CIMValue(String("Test Instance #101a"))));
        cimInstance.addProperty(
            CIMProperty("property3",
                        CIMValue(CIMDateTime::getCurrentDateTime())));
        cimInstance.addProperty(
            CIMProperty("property4",
                        CIMValue(String("Pegasus TestObjectNormalizer"))));

        // complete object path
        CIMObjectPath cimObjectPath;

        cimObjectPath.setClassName(cimInstance.getClassName());

        Array<CIMKeyBinding> keys;

        keys.append(CIMKeyBinding(
            "property1", CIMValue(Uint32(i))));
        keys.append(CIMKeyBinding(
            "property2", CIMValue(String("Test Instance #101a"))));
        // slightly differnt value than property. who wins?
        keys.append(CIMKeyBinding(
            "property3", CIMValue(CIMDateTime::getCurrentDateTime())));

        cimObjectPath.setKeyBindings(keys);

        cimInstance.setPath(cimObjectPath);

        _stopwatch.start();

        CIMInstance normalizedInstance =
            normalizer.processInstance(cimInstance);

        _stopwatch.stop();

        if(verbose)
        {
            cout << normalizedInstance.getPath().toString() << endl;

            XmlWriter::printInstanceElement(normalizedInstance);
        }
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.")
}

// large instance set without qualifiers and with class origin
void Test101b(void)
{
    PRINT("Test101b");

    _stopwatch.reset();

    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = true;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    for(Uint32 i = 0, n = 1000; i < n; i++)
    {
        CIMInstance cimInstance(cimClass.getClassName());

        // only populate keys, let the normalizer do the rest
        cimInstance.addProperty(
            CIMProperty("property1",
                        CIMValue(Uint32(i))));
        cimInstance.addProperty(
            CIMProperty("property2",
                        CIMValue(String("Test Instance #101b"))));
        cimInstance.addProperty(
            CIMProperty("property3",
                        CIMValue(CIMDateTime::getCurrentDateTime())));
        cimInstance.addProperty(
            CIMProperty("property4",
                        CIMValue(String("Pegasus TestObjectNormalizer"))));

        // complete object path
        CIMObjectPath cimObjectPath;

        cimObjectPath.setClassName(cimInstance.getClassName());

        Array<CIMKeyBinding> keys;

        keys.append(CIMKeyBinding("property1",
                  CIMValue(Uint32(i))));
        keys.append(CIMKeyBinding("property2",
                  CIMValue(String("Test Instance #101b"))));
        // slightly differnt value than property. who wins?
        keys.append(CIMKeyBinding("property3",
                  CIMValue(CIMDateTime::getCurrentDateTime())));

        cimObjectPath.setKeyBindings(keys);

        cimInstance.setPath(cimObjectPath);

        _stopwatch.start();

        CIMInstance normalizedInstance =
            normalizer.processInstance(cimInstance);

        _stopwatch.stop();

        if(verbose)
        {
            cout << normalizedInstance.getPath().toString() << endl;

            XmlWriter::printInstanceElement(normalizedInstance);
        }
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

// large instance set with qualifiers and with class origin
void Test101c(void)
{
    PRINT("Test101c");

    _stopwatch.reset();

    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = true;

    CIMClass cimClass =
        repository->getClass(
            "test_namespace",
            "ClassC",
            true,
            includeClassOrigin,
            CIMPropertyList());

    SharedPtr<NormalizerContext> nullContext(0);
    ObjectNormalizer normalizer(
        cimClass,
        includeQualifiers,
        includeClassOrigin,
        CIMNamespaceName("test_namespace"),
        nullContext);

    for(Uint32 i = 0, n = 1000; i < n; i++)
    {
        CIMInstance cimInstance(cimClass.getClassName());

        // only populate keys, let the normalizer do the rest
        cimInstance.addProperty(CIMProperty("property1",
                        CIMValue(Uint32(i))));
        cimInstance.addProperty(CIMProperty("property2",
                        CIMValue(String("Test Instance #101b"))));
        cimInstance.addProperty(CIMProperty("property3",
                        CIMValue(CIMDateTime::getCurrentDateTime())));
        cimInstance.addProperty(CIMProperty("property4",
                        CIMValue(String("Pegasus TestObjectNormalizer"))));

        // complete object path
        CIMObjectPath cimObjectPath;

        cimObjectPath.setClassName(cimInstance.getClassName());

        Array<CIMKeyBinding> keys;

        keys.append(CIMKeyBinding("property1",
                                  CIMValue(Uint32(i))));
        keys.append(CIMKeyBinding("property2",
                                  CIMValue(String("Test Instance #101c"))));
        // slightly differnt value than property. who wins?
        keys.append(CIMKeyBinding("property3",
                                  CIMValue(CIMDateTime::getCurrentDateTime())));

        cimObjectPath.setKeyBindings(keys);

        cimInstance.setPath(cimObjectPath);

        _stopwatch.start();

        CIMInstance normalizedInstance =
            normalizer.processInstance(cimInstance);

        _stopwatch.stop();

        if(verbose)
        {
            cout << normalizedInstance.getPath().toString() << endl;

            XmlWriter::printInstanceElement(normalizedInstance);
        }
    }

    PRINT("*** " << _stopwatch.getElapsed() << " milliseconds.");
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    ObjectNormalizer::setEnableNormalization(true);
    repository = new LocalRepository();

    if(verbose)
    {
        CIMClass cimClass =
            repository->getClass(
                "test_namespace",
                "ClassC",
                true,
                true,
                CIMPropertyList());

        cout << "using the following classes for tests:" << endl;

        XmlWriter::printClassElement(cimClass);
    }

    try
    {
        // basic object tests
        Test001a();
        Test001b();

        // object path failures
        Test002a();
        Test002b();
        Test002c();
        Test002d();

        // object path successes
        Test003a();
        Test003b();
        Test003c();
        Test003d();
        Test003e();

        // instance object failures
        Test004a();
        Test004b();

        // instance object successes
        Test005a();
        Test005b();
        Test005c();
        Test005d();

        // object path processing performance tests
        Test100a();

        // instance processing performance tests
        Test101a();
        Test101b();
        Test101c();
    }
    catch(CIMException & e)
    {
        cout << "CIMException: " << e.getCode() << " " << e.getMessage()
            << endl;

        return(1);
    }
    catch(Exception & e)
    {
        cout << "Exception: " << e.getMessage() << endl;

        return(1);
    }
    catch(...)
    {
        cout << "unknown exception" << endl;

        return(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    delete repository;

    return(0);
}
