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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const char* tmpDir;
static Boolean verbose;
String repositoryRoot;

void test(Uint32 mode)
{

  CIMRepository r (repositoryRoot, mode);

    // Create a namespace:

    const String NAMESPACE = "/zzz";
    const String ABSTRACT = "abstract";
    r.createNameSpace(NAMESPACE);

    // Create a qualifier declaration:

    CIMQualifierDecl q1(ABSTRACT, Boolean(true), CIMScope::CLASS);
    r.setQualifier(NAMESPACE, q1);

    // Get it back and check to see if it is identical:

    CIMConstQualifierDecl q2 = r.getQualifier(NAMESPACE, ABSTRACT);

    PEGASUS_TEST_ASSERT(q1.identical(q2));

    // Remove it now:

    r.deleteQualifier(NAMESPACE, ABSTRACT);

    // Try to get it again (this should fail with a not-found error):

    try
    {
    q2 = r.getQualifier(NAMESPACE, ABSTRACT);
    }
    catch (CIMException& e)
    {
    PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
    }

    // Create two qualifiers:

    CIMQualifierDecl q3(CIMName ("q3"), Uint32(66), CIMScope::CLASS);
    CIMQualifierDecl q4(CIMName ("q4"), String("Hello World"), CIMScope::CLASS);

    r.setQualifier(NAMESPACE, q3);
    r.setQualifier(NAMESPACE, q4);

    // Enumerate the qualifier names:

    Array<CIMQualifierDecl> qualifiers = r.enumerateQualifiers(NAMESPACE);
    PEGASUS_TEST_ASSERT(qualifiers.size() == 2);

    for (Uint32 i = 0, n = qualifiers.size(); i < n; i++)
    {
    // qualifiers[i].print();
    PEGASUS_TEST_ASSERT(
        qualifiers[i].identical(q3) || qualifiers[i].identical(q4));
    }
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " XML | BIN" << endl;
        return 1;
    }

    tmpDir = getenv ("PEGASUS_TMP");
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

    try
    {
      Uint32 mode;
      if (!strcmp(argv[1],"XML") )
    {
      mode = CIMRepository::MODE_XML;
      if (verbose) cout << argv[0]<< ": using XML mode repository" << endl;
    }
      else if (!strcmp(argv[1],"BIN") )
    {
      mode = CIMRepository::MODE_BIN;
      if (verbose) cout << argv[0]<< ": using BIN mode repository" << endl;
    }
      else
    {
      cout << argv[0] << ": invalid argument: " << argv[1] << endl;
      return 1;
    }

      test(mode);
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
