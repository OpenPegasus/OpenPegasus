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
    Test associator operations against the repository:
    references, referenceNames, Associators, associatorNames
    This tests both the basic capabilities and the algorithms
    for classes and subclasses.
    TODO: Add tests for role, resultClass, and super/subclasses.
          Add more tests on returned classes. Today we simply check
          for the correct name and name case.
*/
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

const String NAMESPACE = "aa";
const String targetClass = "MY_targetClass";
const String associatedClass = "MY_associatedClass";
const String associationClass = "MY_associationClass";
String saveCTPath;


// Build Asociations into the repository. This consists of one
// association class, the target class, and the associated Class
void test01BuildClasses(CIMRepository& repository)
{
    repository.createNameSpace(NAMESPACE);
    repository.setQualifier(
    NAMESPACE, CIMQualifierDecl(CIMName ("key"), true,
        (CIMScope::PROPERTY + CIMScope::REFERENCE)));

    repository.setQualifier(
    NAMESPACE, CIMQualifierDecl(CIMName ("association"), true,
        (CIMScope::ASSOCIATION  + CIMScope::CLASS)));

    // create the target Class
    CIMClass ct(targetClass);

    ct.addProperty(
    CIMProperty(CIMName ("key"), Uint32(0))
        .addQualifier(CIMQualifier(CIMName ("key"), true)))
    .addProperty(CIMProperty(CIMName ("ratio"), Real32(1.5)))
    .addProperty(CIMProperty(CIMName ("message"), String("Hello World")));

    repository.createClass(NAMESPACE, ct);

    // Create the associated class
    CIMClass ce(associatedClass);

    ce.addProperty(
    CIMProperty(CIMName ("key"), Uint32(0))
        .addQualifier(CIMQualifier(CIMName ("key"), true)))
    .addProperty(CIMProperty(CIMName ("ratio"), Real32(1.5)))
    .addProperty(CIMProperty(CIMName ("message"), String("Hello World")));

    repository.createClass(NAMESPACE, ce);

    // Create associationClass
    CIMClass ca(associationClass);

    ca
        .addQualifier(CIMQualifier(CIMName ("Association"), Boolean(true)))

        .addProperty(
            CIMProperty(CIMName ("ref1"),
                CIMObjectPath("targetClass"),0,
                CIMName (targetClass))
                    .addQualifier(CIMQualifier(CIMName ("key"), true)))

        .addProperty(
            CIMProperty(CIMName ("ref2"),
                CIMObjectPath("associatedClass"), 0,
                CIMName (associatedClass))
                    .addQualifier(CIMQualifier(CIMName ("key"), true)))
                ;

    repository.createClass(NAMESPACE, ca);
}

// test all 4 of the associator operations against the classes
void test02TestClassOperations(CIMRepository& repository)
{
    Array<CIMObjectPath> paths = repository.referenceNames(NAMESPACE,
        CIMObjectPath(String(),
        CIMNamespaceName(),
        targetClass));

    PEGASUS_TEST_ASSERT(paths.size() == 1);
    if (verbose)
    {
        cout << paths[0].toString() << " " << associationClass
            << " " << paths[0].getClassName().getString() << endl;
    }

    // Test if the string value of the class name is the same.  This insures
    // that the case is also the same.
    PEGASUS_TEST_ASSERT(
        paths[0].getClassName().getString() == associationClass);

    Array<CIMObject> objects = repository.references(NAMESPACE,
        CIMObjectPath(String(),CIMNamespaceName(), targetClass));

    PEGASUS_TEST_ASSERT(objects.size() == 1);
    PEGASUS_TEST_ASSERT(objects[0].getClassName() == associationClass);

    Array<CIMObjectPath> paths2 = repository.associatorNames(NAMESPACE,
        CIMObjectPath(String(),CIMNamespaceName(), targetClass));

    PEGASUS_TEST_ASSERT(paths2.size() == 1);
    PEGASUS_TEST_ASSERT(
        paths2[0].getClassName().getString() == associatedClass);


    Array<CIMObject> objects2 = repository.associators(NAMESPACE,
        CIMObjectPath(String(),CIMNamespaceName(), targetClass));

    PEGASUS_TEST_ASSERT(objects2.size() == 1);
    PEGASUS_TEST_ASSERT(
            objects2[0].getClassName().getString() == associatedClass);
}

// test all 4 of the associator operations against the classes
void test03AddInstances(CIMRepository& repository)
{
    // Get the three classes from the Repository
    CIMClass ct = repository.getClass(NAMESPACE,targetClass);
    CIMClass ce = repository.getClass(NAMESPACE,associatedClass);

    // Create an instance for each class
    //
    CIMInstance it = ct.buildInstance(true,true, CIMPropertyList());
    CIMInstance ie = ce.buildInstance(true,true, CIMPropertyList());

    // Put data into target instance key property
    CIMObjectPath ref1Path;
    {
        Uint32 i = it.findProperty("key");
        CIMProperty p = it.getProperty(i);
        p.setValue(Uint32(1));
        CIMObjectPath path = it.buildPath(ct);
        saveCTPath = path.toString();
        it.setPath(path);

        ref1Path = repository.createInstance(NAMESPACE,it);
    }
    // Put data into associated instance key property
    CIMObjectPath ref2Path;
    {
        Uint32 i = ie.findProperty("key");
        CIMProperty p = ie.getProperty(i);
        p.setValue(Uint32(1));
        CIMObjectPath path = ie.buildPath(ce);
        ie.setPath(path);

        ref2Path = repository.createInstance(NAMESPACE,ie);
    }
    // Put references into the association instance
    CIMObjectPath assocPath;
    {
        CIMInstance ia(associationClass);

        ia.addProperty(CIMProperty(CIMName("ref1"),
            CIMObjectPath(ref1Path),0,CIMName(targetClass)));
        ia.addProperty(CIMProperty(CIMName("ref2"),
            CIMObjectPath(ref2Path),0, CIMName(associatedClass)));

        assocPath = repository.createInstance(NAMESPACE,ia);
    }
}

// test all 4 of the associator operations instance operations
void test04TestAssociationInstances(CIMRepository& repository)
{

    // Test referenceNames
    CIMObjectPath targetInstancePath(saveCTPath);
    Array<CIMObjectPath> paths = repository.referenceNames(NAMESPACE,
                                                  targetInstancePath);

    if (verbose)
        cout << targetInstancePath.toString() << " " << paths.size() << endl;
    PEGASUS_TEST_ASSERT(paths.size() == 1);
    if (verbose)
        cout << paths[0].toString() << endl;
    PEGASUS_TEST_ASSERT(paths[0].getClassName() == associationClass);

    // Test references.

    Array<CIMObject> objects =
        repository.references(NAMESPACE,targetInstancePath);

    PEGASUS_TEST_ASSERT(objects.size() == 1);

    PEGASUS_TEST_ASSERT(
        objects[0].getClassName().getString() == associationClass);

    // Test Associator Names

    paths = repository.associatorNames(NAMESPACE,targetInstancePath);
    PEGASUS_TEST_ASSERT(paths.size() == 1);
    PEGASUS_TEST_ASSERT(paths[0].getClassName().getString() == associatedClass);

    // Test associators

    objects = repository.associators(NAMESPACE, targetInstancePath);

    PEGASUS_TEST_ASSERT(objects.size() == 1);
    PEGASUS_TEST_ASSERT(
        objects[0].getClassName().getString() == associatedClass);
}
int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " XML | BIN" << endl;
        return 1;
    }

    String repositoryRoot;
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        repositoryRoot = ".";
    }
    else
    {
        repositoryRoot = tmpDir;
    }
    repositoryRoot.append("/repository");

    FileSystem::removeDirectoryHier(repositoryRoot);

    Uint32 mode;
    try
    {
        if (!strcmp(argv[1],"XML") )
        {
            mode = CIMRepository::MODE_XML;
            if (verbose) cout << argv[0]<< ": using XML mode repository"
                << endl;
        }
        else if (!strcmp(argv[1],"BIN") )
        {
            mode = CIMRepository::MODE_BIN;
            if (verbose) cout << argv[0]<< ": using BIN mode repository"
                << endl;
        }
        else
        {
            cout << argv[0] << ": invalid argument: " << argv[1] << endl;
            return 1;
        }
        CIMRepository r (repositoryRoot, mode);

        test01BuildClasses(r);

        test02TestClassOperations(r);
        test03AddInstances(r);
        test04TestAssociationInstances(r);
    }
    catch (Exception& e)
    {
    cout << argv[0] << " " << argv[1] << " " << e.getMessage() << endl;
    exit(1);
    }

    FileSystem::removeDirectoryHier(repositoryRoot);

    cout << argv[0] << " " << argv[1] << " +++++ passed all tests" << endl;

    return 0;
}
