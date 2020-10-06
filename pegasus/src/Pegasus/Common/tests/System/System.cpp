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


#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/HostAddress.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#ifdef PEGASUS_ENABLE_IPV6
static void _testIPv6()
{
    String hostName;
    String hostIP;
    int af;
    char binAddr[PEGASUS_IN6_ADDR_SIZE];
    char binAddr2[PEGASUS_IN6_ADDR_SIZE];

    hostName = System::getHostName();

    // Node may not be in network or not configured.
    if (hostName == String::EMPTY || !System::getHostIP(hostName, &af, hostIP))
    {
        return;
    }

    PEGASUS_TEST_ASSERT(System::isLocalHost(hostName));
    PEGASUS_TEST_ASSERT(System::isLocalHost(
        System::getFullyQualifiedHostName()));
    PEGASUS_TEST_ASSERT(System::isLocalHost(hostIP));
    PEGASUS_TEST_ASSERT(System::isLocalHost("127.0.0.1"));
    PEGASUS_TEST_ASSERT(System::isLocalHost("::1"));


    System::acquireIP(hostName.getCString(), &af, binAddr);
    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(af,
        hostIP.getCString(), binAddr2) == 1);
    PEGASUS_TEST_ASSERT(HostAddress::equal(af, binAddr, binAddr2));

    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV4,
        "127.0.0.1", binAddr) == 1);
    PEGASUS_TEST_ASSERT(System::isLoopBack(HostAddress::AT_IPV4, binAddr));

    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV4,
        "127.0.0.8", binAddr) == 1);
    PEGASUS_TEST_ASSERT(System::isLoopBack(HostAddress::AT_IPV4, binAddr));

    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV4,
        "127.10.120.18", binAddr) == 1);
    PEGASUS_TEST_ASSERT(System::isLoopBack(HostAddress::AT_IPV4, binAddr));

    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV4,
        "127.255.255.255", binAddr) == 1);
    PEGASUS_TEST_ASSERT(System::isLoopBack(HostAddress::AT_IPV4, binAddr));

    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV4,
        "127.0.0.0", binAddr) == 1);
    PEGASUS_TEST_ASSERT(System::isLoopBack(HostAddress::AT_IPV4, binAddr));

    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV6,
        "::1", binAddr) == 1);
    PEGASUS_TEST_ASSERT(System::isLoopBack(HostAddress::AT_IPV6, binAddr));


    // Test fail cases
    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV4,
        "192.127.1.1", binAddr) == 1);
    PEGASUS_TEST_ASSERT(!System::isLoopBack(HostAddress::AT_IPV4, binAddr));

    PEGASUS_TEST_ASSERT(HostAddress::convertTextToBinary(HostAddress::AT_IPV6,
        "fffe:1234:3455:cbcd:1123:345e:abcd:ef12", binAddr) == 1);
    PEGASUS_TEST_ASSERT(!System::isLoopBack(HostAddress::AT_IPV6, binAddr));
}
#endif

int main(int, char** argv)
{
    PEGASUS_TEST_ASSERT(System::strcasecmp("","") == 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("a","A") == 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("A","a") == 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("A","aa") < 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("AA","a") > 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("a","AA") < 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("aa","A") > 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("","A") < 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("A","") > 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("aa","ab") < 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("ab","aa") > 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("AA","ab") < 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("AB","aa") > 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("aa","AB") < 0);
    PEGASUS_TEST_ASSERT(System::strcasecmp("ab","AA") > 0);

    char full_path_buff[4096];
    char extracted_dir_name[10000];
    char large_path[6000];
    for( int i=0; i< 4096; i++ )
    {
        full_path_buff[i] = '/';
    }
    full_path_buff[sizeof(full_path_buff)-1]='\0';
    System::extract_file_path( full_path_buff, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp(full_path_buff,extracted_dir_name)
        == 0);

    for( int i=0; i< 6000; i++ )
    {
        large_path[i] = '/';
    }
    large_path[sizeof(large_path)-1]='\0';
    System::extract_file_path( large_path, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp(full_path_buff,extracted_dir_name)
        == 0);

    for( int i=0; i< 4096; i++ )
    {
        full_path_buff[i] = 'a';
    }
    full_path_buff[sizeof(full_path_buff)-1]='\0';
    System::extract_file_path( full_path_buff, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp(full_path_buff,extracted_dir_name)
        == 0);

    full_path_buff[0]='\0';
    System::extract_file_path( full_path_buff, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp(full_path_buff,extracted_dir_name)
        == 0);

    full_path_buff[0]='1';
    full_path_buff[1]='\0';
    System::extract_file_path( full_path_buff, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp(full_path_buff,extracted_dir_name)
        == 0);

    strcpy( full_path_buff, "single_file" );
    System::extract_file_path( full_path_buff, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp(full_path_buff,extracted_dir_name)
        == 0);

    strcpy( full_path_buff, "/path1/path2/file1" );
    System::extract_file_path( full_path_buff, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp("/path1/path2/", extracted_dir_name)
        == 0);

    strcpy( full_path_buff, "\\path1\\path2\\file1" );
    System::extract_file_path( full_path_buff, extracted_dir_name );
    PEGASUS_TEST_ASSERT(System::strcasecmp(
        "\\path1\\path2\\", extracted_dir_name) == 0);

    // userSuccess variable contains the username part of the userGroup      //
    // userFailure variable contains the username not part of the userGroup  //
    // By default  this test case uses root user,root group for success      //
    // condition under non-windows platforms. Administrator user and         //
    // Administrator group for success condition under Windows platform.     //
    // and guest user to check for failure condition.                        //
    // However the existance of user names above are not verified            //
    // If the user wishes to provide different values he can specify through //
    // the environment variables PEGASUS_TEST_ISGROUP_USER_SUCCESS,          //
    // PEGASUS_TEST_ISGROUP_USER_FAILURE, PEGASUS_TEST_ISGROUP_GROUP         //

#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION

    const char* envValue;
    char userSuccess[20],userFailure[20],userGroup[20];

#ifndef PEGASUS_OS_TYPE_WINDOWS
    strcpy(userSuccess,"root");
    strcpy(userGroup,"root");
    strcpy(userFailure,"guest");
#else
    strcpy(userSuccess,"Administrator");
    strcpy(userGroup,"Administrators");
    strcpy(userFailure,"Guest");
#endif

    if ((envValue=getenv("PEGASUS_TEST_ISGROUP_USER_SUCCESS"))!=NULL)
    {
        strcpy(userSuccess,envValue);
    }

    if ((envValue = getenv("PEGASUS_TEST_ISGROUP_GROUP"))!=NULL)
    {
        strcpy(userGroup,envValue);
    }

    PEGASUS_TEST_ASSERT(System::isGroupMember(userSuccess,userGroup)==true);

    if ((envValue=getenv("PEGASUS_TEST_ISGROUP_USER_FAIL"))!=NULL)
    {
        strcpy(userFailure,envValue);
    }

    PEGASUS_TEST_ASSERT(System::isGroupMember(userFailure,userGroup)==false);

#endif

#ifdef PEGASUS_ENABLE_IPV6
    _testIPv6 ();
#endif

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
