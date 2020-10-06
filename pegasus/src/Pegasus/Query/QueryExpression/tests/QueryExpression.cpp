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
#include <Pegasus/Common/FileSystem.h>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Query/QueryExpression/QueryExpression.h>
#include <Pegasus/Query/QueryCommon/SelectStatement.h>
#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/Repository/RepositoryQueryContext.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
//FILE *CQL_in;
//int CQL_parse();

//PEGASUS_NAMESPACE_BEGIN
//CQLParserState* globalParserState = 0;
//PEGASUS_NAMESPACE_END

Boolean _applyProjection(
    QueryExpression& qe,
    Array<CIMInstance>& _instances,
    String testOption,
    String lang)
{
    if (testOption == String::EMPTY || testOption == "2")
    {
        cout << endl << lang << " ========Apply Projection Results========"
             << endl;
        cout << qe.getQuery() << endl;

        for(Uint32 j = 0; j < _instances.size(); j++)
        {
            cout << "Instance of class "
                 << _instances[j].getClassName().getString() << endl;

            try
            {
                CIMInstance projInst = _instances[j].clone();

                Boolean gotPropExc = false;
                try
                {
                    qe.applyProjection(projInst, false);
                }
                catch (QueryRuntimePropertyException & qrpe)
                {
                    // Got a missing property exception.
                    cout << "-----" << qrpe.getMessage() << endl;
                    gotPropExc = true;
                }

                if (gotPropExc)
                {
                    // Got a missing property exception.
                    // Try again, allowing missing properties.
                    // Need to use a cloned instance because the original
                    // instance was partially projected.
                    cout << "Instance of class "
                         << _instances[j].getClassName().getString()
                         << ".  Allow missing properties." << endl;
                    projInst = _instances[j].clone();
                    qe.applyProjection(projInst, true);
                }

                Uint32 cnt = projInst.getPropertyCount();
                if (cnt == 0)
                {
                    cout << "-----No properties left after projection" << endl;
                }

                if (cnt > 10)
                {
                    // If more than 10 props, just print the count to keep
                    // the output file short
                    cout << "Instance has " << cnt << " properties" << endl;
                }
                else
                {
                    for (Uint32 n = 0; n < cnt; n++)
                    {
                        CIMProperty prop = projInst.getProperty(n);
                        CIMValue val = prop.getValue();
                        cout << "-----Prop #" << n << " Name = "
                             << prop.getName().getString();
                        if (val.isNull())
                        {
                            cout << " Value = NULL" << endl;
                        }
                        else
                        {
                            cout << " Value = " << val.toString() << endl;
                        }
                    }
                }
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }
        }
    }

    return true;
}

void _printPropertyList(CIMPropertyList& propList)
{
    if (propList.isNull())
    {
        cout << "-----all properties required" << endl;
    }
    else if (propList.size() == 0)
    {
        cout << "-----no properties required" << endl;
    }
    else
    {
        for (Uint32 n = 0; n < propList.size(); n++)
        {
            cout << "-----Required property " << propList[n].getString()
                 << endl;
        }
    }
}

Boolean _getPropertyList(
    QueryExpression& qe,
    Array<CIMInstance>& _instances,
    CIMNamespaceName ns,
    String testOption,
    String lang)
{
    if (testOption == String::EMPTY || testOption == "3")
    {
        for (Uint32 j = 0; j < _instances.size(); j++)
        {
            cout << endl << lang << " ========Get Property List Results======="
                 << endl;
            cout << qe.getQuery() << endl;

            try
            {
                cout << endl << "Get Class Path List" << endl;
                Array<CIMObjectPath> fromPaths = qe.getClassPathList();
                for (Uint32 k = 0; k < fromPaths.size(); k++)
                {
                    cout << "-----" << fromPaths[k].toString() << endl;
                }
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }

            CIMName className = _instances[j].getClassName();
            CIMObjectPath classPath (String::EMPTY, ns, className);

            CIMPropertyList propList;
            try
            {
                cout << "Property List for the FROM class" << endl;
                propList = qe.getPropertyList();
                _printPropertyList(propList);
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }

            try
            {
                cout << "Property List for " << className.getString() << endl;
                propList = qe.getPropertyList(classPath);
                _printPropertyList(propList);
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }

            try
            {
                propList.clear();
                cout << "SELECT Property List for the FROM class" << endl;
                propList = qe.getSelectPropertyList();
                _printPropertyList(propList);
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }

            try
            {
                propList.clear();
                cout << "SELECT Property List for " << className.getString()
                     << endl;
                propList = qe.getSelectPropertyList(classPath);
                _printPropertyList(propList);
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }

            try
            {
                propList.clear();
                cout << "WHERE Property List for the FROM class" << endl;
                propList = qe.getWherePropertyList();
                _printPropertyList(propList);
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }

            try
            {
                propList.clear();
                cout << "WHERE Property List for " << className.getString()
                     << endl;
                propList = qe.getWherePropertyList(classPath);
                _printPropertyList(propList);
            }
            catch (Exception& e)
            {
                cout << "-----" << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }
        }
    }

    return true;
}

Boolean _evaluate(
    QueryExpression& qe,
    Array<CIMInstance>& _instances,
    String testOption,
    String lang)
{
    if (testOption == String::EMPTY || testOption == "1")
    {
        cout << endl << lang << " ========Evaluating QueryExpression======="
             << endl;
        cout << qe.getQuery() << endl << endl;

        for (Uint32 j = 0; j < _instances.size(); j++)
        {
            try
            {
                Boolean result = qe.evaluate(_instances[j]);
                cout << qe.getQuery() << " = ";
                if (result)
                    cout << "TRUE" << endl << endl;
                else
                    cout << "FALSE" << endl << endl;
            }
            catch (Exception& e)
            {
                cout << e.getMessage() << endl;
            }
            catch (...)
            {
                cout << "Unknown Exception" << endl;
            }
        }
    }

    return true;
}

void _validate(
    QueryExpression& qe,
    String lang,
    String testOption)
{
    if (testOption == String::EMPTY || testOption == "4")
    {
        cout << endl << lang << " ========Validating QueryExpression======="
             << endl;
        cout << qe.getQuery() << endl;
        try
        {
            qe.validate();
            cout << "validate ok" << endl;
        }
        catch (Exception& e)
        {
            cout << e.getMessage() << endl;
        }
        catch (...)
        {
            cout << "Unknown Exception" << endl;
        }
    }
}

void help(const char* command)
{
    cout << command << " queryFile [option]" << endl;
    cout << " options:" << endl;
    cout << " -test: ";
    cout << "1 = evaluate" << endl
         << "        2 = apply projection" << endl
         << "        3 = get property list and get class path list" << endl;
    cout << "        4 = validate properties" << endl;
    cout << " -className class" << endl;
    cout << " -nameSpace namespace (Example: root/SampleProvider)"
         << endl << endl;
}

int main(int argc, char ** argv)
{
    // process options
    if (argc == 1 || (argc > 1 && strcmp(argv[1],"-h") == 0))
    {
        help(argv[0]);
        exit(0);
    }

    // Since the output of this program will be compared with
    // a master output file, turn off ICU message loading.
    MessageLoader::_useDefaultMsg = true;

    String testOption;
    String className;
    String nameSpace;

    for (int i = 0; i < argc; i++)
    {
        if ((strcmp(argv[i],"-test") == 0) && (i+1 < argc))
            testOption = argv[i+1];
        if ((strcmp(argv[i],"-className") == 0) && (i+1 < argc))
            className = argv[i+1];
        if ((strcmp(argv[i],"-nameSpace") == 0) && (i+1 < argc))
            nameSpace = argv[i+1];
    }

    String lang("WQL");
    Array<CQLSelectStatement> _statements;

    // setup test environment

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

    CIMNamespaceName _ns;
    if (nameSpace != String::EMPTY)
    {
        _ns = nameSpace;
    }
    else
    {
        cout << "Using root/SampleProvider as default namespace." << endl;
        _ns = String("root/SampleProvider");
    }
    CIMRepository* _rep;
    try
    {
        _rep = new CIMRepository(repositoryDir);
    }
    catch(Exception &e)
    {
        cout << endl << endl
             << "Invalid Repository: Exception: " << e.getMessage()
             << endl << endl;
        exit(-1);
    }
    RepositoryQueryContext _ctx(_ns, _rep);

    char text[1024];
    char* _text;

    // setup Test Instances
    Array<CIMInstance> _instances;
    if (className != String::EMPTY)
    {
        try
        {
            const CIMName _testclass(className);
            _instances = _rep->enumerateInstancesForSubtree(
                _ns, _testclass);
        }
        catch (Exception& e)
        {
            cout << endl << endl
                 << "Exception: Invalid namespace/class: "
                 << e.getMessage() << endl << endl;
        }
    }
    else
    { // load default class names
        cout << endl << "Using default class names to test queries. "
             << endl << endl;
        const CIMName _testclass(String("QExpr_TestPropertyTypes"));
        const CIMName _testclass2(String("QExpr_TestElement"));
        try
        {
            _instances = _rep->enumerateInstancesForSubtree(
                _ns, _testclass);
            _instances.appendArray(_rep->enumerateInstancesForSubtree(
                _ns, _testclass2));
        }
        catch (Exception& e)
        {
            cout << endl << endl << "Exception: Invalid namespace/class: "
                 << e.getMessage() << endl << endl;
        }
    }

    // setup input stream
    if (argc >= 2)
    {
        ifstream queryInputSource(argv[1]);
        if (!queryInputSource)
        {
            cout << "Cannot open input file.\n" << endl;
            return 1;
        }
        int cnt = 0;
        while (!queryInputSource.eof())
        {
            queryInputSource.getline(text, 1024);
            char* _ptr = text;
            _text = strcat(_ptr,"\n");
            // check for comments and ignore
            // a comment starts with a # as the first non whitespace character
            // on the line
            char _comment = '#';
            int i = 0;
            while (text[i] == ' ' || text[i] == '\t') i++; // ignore whitespace
            if (text[i] != _comment)
            {
                if (!(strlen(_text) < 2))
                {
                    String query(text);
                    if (query == "WQL:\n")
                        lang = "WQL";
                    else if(query == "CQL:\n")
                        lang = "DMTF:CQL";
                    else
                    {
                        try
                        {
                            QueryExpression qexpr;

                            if(cnt % 2)
                            {
                                qexpr = QueryExpression(lang,query);
                                qexpr.setQueryContext(_ctx);
                            }
                            else
                            {
                                qexpr = QueryExpression(lang,query,_ctx);
                            }

                            cnt++;

                            SelectStatement* ss = qexpr.getSelectStatement();
                            PEGASUS_TEST_ASSERT(0 != ss);

                            String returnQuery = qexpr.getQuery();
                            PEGASUS_TEST_ASSERT(returnQuery == query);

                            String returnLang = qexpr.getQueryLanguage();
                            PEGASUS_TEST_ASSERT(returnLang == lang);

                            QueryExpression copy(qexpr);
                            PEGASUS_TEST_ASSERT(
                                copy.getQuery() == qexpr.getQuery());
                            PEGASUS_TEST_ASSERT(
                                copy.getQueryLanguage() ==
                                    qexpr.getQueryLanguage());

                            _applyProjection(
                                qexpr, _instances, testOption, lang);
                            _getPropertyList(
                                qexpr, _instances, _ns, testOption,lang);
                            _validate(qexpr,lang, testOption);
                            _evaluate(qexpr,_instances, testOption,lang);
                        }
                        catch(Exception& e)
                        {
                            cout << e.getMessage() << endl;
                        }
                        catch(...)
                        {
                            cout << "CAUGHT ... BADNESS HAPPENED!!!" << endl;
                        }
                    }
                }
            }

            // while (!eof) behaves differently on HP-UX, seems
            // like it takes an extra iteration to hit eof.  This
            // leaves "text" with the previous value from
            // getline(..), which causes a duplicate parse of
            // the last select statement in the query file,
            // FIX: we clear text before doing another getline(..)
            text[0] = 0;
        }
        queryInputSource.close();
    }
    else
    {
        cout << "Invalid number of arguments.\n" << endl;
    }
    delete _rep;
    return 0;
}

