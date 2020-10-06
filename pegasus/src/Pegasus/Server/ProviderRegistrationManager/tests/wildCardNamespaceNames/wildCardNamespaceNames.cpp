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
#include <Pegasus/Client/CIMClient.h>
#include \
    <Pegasus/Server/ProviderRegistrationManager/ProviderRegistrationManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

const CIMNamespaceName NAMESPACE = CIMNamespaceName ("root/cimv2");


int main(int, char** argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    if (verbose) cout << argv[0] << ": started" << endl;

    // Setup the repository path and create a repository
    String repositoryRootPath;
        const char* tmpDir = getenv ("PEGASUS_TMP");

    if (tmpDir == NULL)
        repositoryRootPath = ".";
    else
        repositoryRootPath = tmpDir;

    repositoryRootPath.append("/repository");

    FileSystem::removeDirectoryHier(repositoryRootPath);

    try
    {
       CIMRepository _repository(repositoryRootPath);

       CIMNamespaceName ok1(WildCardNamespaceNames::add(String("root/ci*")));
       if (verbose) cout<<argv[0]<<" --- ok1: "<<ok1.getString()<<endl;

       CIMNamespaceName ok0(WildCardNamespaceNames::add(String("root/cimv2")));
       if (verbose) cout<<argv[0]<<" --- ok0: "<<ok0.getString()<<endl;

       CIMNamespaceName ok2(WildCardNamespaceNames::add(String("root/cim*")));
       if (verbose) cout<<argv[0]<<" --- ok2: "<<ok2.getString()<<endl;

       CIMNamespaceName ok3(WildCardNamespaceNames::add(String("root/ci*")));
       if (verbose) cout<<argv[0]<<" --- ok3: "<<ok1.getString()<<endl;

       CIMNamespaceName ok11(WildCardNamespaceNames::check(
           String("root/ci33")));
       if (verbose) cout<<argv[0]<<" --- ok11: "<<ok11.getString()<<endl;

       CIMNamespaceName ok12(WildCardNamespaceNames::check(
           String("root/cim3")));
       if (verbose) cout<<argv[0]<<" --- ok12: "<<ok12.getString()<<endl;

       const Array<String> ar=WildCardNamespaceNames::getArray();
       for ( int i=0,m=ar.size(); i<m; i++ )
       {
           if (verbose) cout<<argv[0]<<" --- "<<i<<" "<<ar[i]<<endl;
       }

       Array<CIMNamespaceName> nss=_repository.enumerateNameSpaces();
       for (int i=0,m=nss.size(); i<m; i++ )
       {
          if (verbose) cout << argv[0]<<" --- "<<i<<" "<<nss[i].getString()
                            <<endl;
       }

    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
        PEGASUS_STD (cout) << argv[0] << " +++++ modify instances failed"
                           << PEGASUS_STD (endl);
        exit(-1);
    }

    FileSystem::removeDirectoryHier(repositoryRootPath);

    PEGASUS_STD(cout) << argv[0] <<  " +++++ passed all tests"
                      << PEGASUS_STD(endl);

    return 0;
}
