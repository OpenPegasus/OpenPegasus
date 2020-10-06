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


#include <cstdlib>
#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifier.h>
#include <Pegasus/Provider/CIMOMHandleQueryContext.h>
#include <Pegasus/Repository/RepositoryQueryContext.h>
#include <Pegasus/CQL/CQLIdentifier.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void drive_FromList(QueryContext& _query)
{
    Array<String> alias;
    Array<CQLIdentifier> classes;
    alias.append("A");
    alias.append("B");
    alias.append("C");
    alias.append("D");  // alias == identifier, ignore alias
    alias.append("A");  // dup, should not be inserted
    classes.append(CQLIdentifier("APPLE"));
    classes.append(CQLIdentifier("BONGO"));
    classes.append(CQLIdentifier("CLAVE"));
    classes.append(CQLIdentifier("D"));   // alias == identifier, ignore alias
    classes.append(CQLIdentifier("APPLE"));  // dup, should not be inserted

    for (Uint32 i = 0; i < alias.size(); i++)
    {
        _query.insertClassPath(classes[i],alias[i]);
    }

    //
    // Error inserts.  Keep before the from list test below
    //

    // empty identifier
    try
    {
        _query.insertClassPath(QueryIdentifier());
        PEGASUS_TEST_ASSERT(false);
    }
    catch (QueryParseException&)
    {
    }

    // identifier is already an alias
    try
    {
       _query.insertClassPath(CQLIdentifier("A"));
       PEGASUS_TEST_ASSERT(false);
    }
    catch (QueryParseException&)
    {
    }

    // alias is already in the from list
    try
    {
        _query.insertClassPath(CQLIdentifier("NEW"),String("BONGO"));
        PEGASUS_TEST_ASSERT(false);
    }
    catch (QueryParseException&)
    {
    }

    // alias is already used for another from list entry
    try
    {
        _query.insertClassPath(CQLIdentifier("NEW"),String("B"));
        PEGASUS_TEST_ASSERT(false);
    }
    catch (QueryParseException&)
    {
    }


    // check the from list
    Array<QueryIdentifier> fromList = _query.getFromList();
    PEGASUS_TEST_ASSERT(fromList.size() == 4);
    PEGASUS_TEST_ASSERT(fromList[0].getName() == "APPLE");
    PEGASUS_TEST_ASSERT(fromList[1].getName() == "BONGO");
    PEGASUS_TEST_ASSERT(fromList[2].getName() == "CLAVE");
    PEGASUS_TEST_ASSERT(fromList[3].getName() == "D");

    // check the from string
    String fromString = _query.getFromString();
    PEGASUS_TEST_ASSERT(
        fromString == "FROM APPLE AS A , BONGO AS B , CLAVE AS C , D ");

    // identifier and alias lookup
    QueryIdentifier lookup = _query.findClass(String("C"));
    PEGASUS_TEST_ASSERT(lookup.getName() == "CLAVE");
    lookup = _query.findClass(String("BONGO"));
    PEGASUS_TEST_ASSERT(lookup.getName() == "BONGO");
    lookup = _query.findClass(String("D"));
    PEGASUS_TEST_ASSERT(lookup.getName() == "D");
    lookup = _query.findClass(String("notthere"));
    PEGASUS_TEST_ASSERT(lookup.getName() == CIMName());
}

void drive_WhereIds(QueryContext& _query)
{
    CQLChainedIdentifier chid1("fromclass.eo.scope1::prop");
    CQLChainedIdentifier chid2("fromclass.eo.scope2::prop");
    CQLChainedIdentifier chid3("fromclass.eo.scope1::prop#'ok'");
    CQLChainedIdentifier chid4("fromclass.eo.scope1::prop[1]");

    _query.addWhereIdentifier(chid1);
    _query.addWhereIdentifier(chid2);
    _query.addWhereIdentifier(chid3);
    _query.addWhereIdentifier(chid4);
    _query.addWhereIdentifier(chid1); // dup, ignored

    Array<QueryChainedIdentifier> qchids = _query.getWhereList();

    PEGASUS_TEST_ASSERT(qchids.size() == 4);
    PEGASUS_TEST_ASSERT(qchids[0].getSubIdentifiers().size() == 3);
    PEGASUS_TEST_ASSERT(qchids[1].getSubIdentifiers().size() == 3);
    PEGASUS_TEST_ASSERT(qchids[2].getSubIdentifiers().size() == 3);
    PEGASUS_TEST_ASSERT(qchids[3].getSubIdentifiers().size() == 3);
}

void drive_Schema(QueryContext& _query)
{
    CIMName base("CQL_TestElement");
    CIMClass _class = _query.getClass(base);
    PEGASUS_TEST_ASSERT(_class.getClassName() == base);

    Array<CIMName> names = _query.enumerateClassNames(base);
    PEGASUS_TEST_ASSERT(names.size() == 2);

    CIMName derived("CQL_TestPropertyTypes");

    PEGASUS_TEST_ASSERT(_query.isSubClass(base, derived));
    PEGASUS_TEST_ASSERT(!_query.isSubClass(derived, base));

    PEGASUS_TEST_ASSERT(
        _query.getClassRelation(base, base) == QueryContext::SAMECLASS);
    PEGASUS_TEST_ASSERT(
        _query.getClassRelation(base, derived) == QueryContext::SUBCLASS);
    PEGASUS_TEST_ASSERT(
        _query.getClassRelation(derived, base) == QueryContext::SUPERCLASS);

    CIMName unrelated("CIM_Process");
    PEGASUS_TEST_ASSERT(
        _query.getClassRelation(base, unrelated) == QueryContext::NOTRELATED);
    PEGASUS_TEST_ASSERT(
        _query.getClassRelation(unrelated, base) == QueryContext::NOTRELATED);
}

void drive_CIMOMHandleQueryContext()
{
    CIMNamespaceName _ns("root/SampleProvider");
    CIMOMHandle _ch;
    CIMOMHandleQueryContext _queryOrig(_ns,_ch);

    CIMOMHandleQueryContext _query = _queryOrig;

    PEGASUS_TEST_ASSERT(_query.getNamespace() == _ns);

    drive_FromList(_query);
    drive_WhereIds(_query);
    drive_Schema(_query);
}

void drive_RepositoryQueryContext()
{
    // get the configuration variable PEGASUS_HOME
    const char* peg_home = getenv("PEGASUS_HOME");
    if (peg_home == NULL)
    {
        cout << "PEGASUS_HOME needs to be set to run this test." << endl;
        exit(-1);
    }
    String repositoryDir(peg_home);
    repositoryDir.append("/");

    // get the makefile build config variable REPOSITORY_NAME
    const char* repo_name = getenv("REPOSITORY_NAME");
    if (repo_name == NULL)
        repositoryDir.append("repository");
    else
        repositoryDir.append(repo_name);

    CIMNamespaceName _ns("root/SampleProvider");
    CIMRepository _rep(repositoryDir);
    RepositoryQueryContext _queryOrig(_ns, &_rep);

    RepositoryQueryContext _query = _queryOrig;

    PEGASUS_TEST_ASSERT(_query.getNamespace() == _ns);

    drive_FromList(_query);
    drive_WhereIds(_query);
    drive_Schema(_query);
}

int main(int, char *argv[])
{
    //
    // NOTE: this test needs to be in poststarttests
    // because the CIMOMHandle uses CIMClient local connect
    //

    //BEGIN TESTS....
    String repositoryDir;

    try
    {
        drive_CIMOMHandleQueryContext();
        drive_RepositoryQueryContext();
    }
    catch (Exception& e)
    {
        cout << "Received exception: " << e.getMessage() << endl;
        cout << argv[0] << "+++++ failed" << endl;
        return -1;
    }

    //END TESTS....

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
