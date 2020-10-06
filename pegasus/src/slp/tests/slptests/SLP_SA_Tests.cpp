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

#define SLP_LIB_IMPORT
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/PegasusAssert.h>
#include "slp/slp_client/src/cmd-utils/slp_client/slp_client.h"
#include <Pegasus/Client/CIMClient.h>
#define SLP_PORT 427
#define LOCALHOST_IP "127.0.0.1"

PEGASUS_USING_PEGASUS;
//Global variables used for SLP registrations and registration tests.
static char *predicate;
const char *scopes = "DEFAULT";
#ifndef PEGASUS_SLP_REG_TIMEOUT
int life = 0XFFFF;
#else
int life = PEGASUS_SLP_REG_TIMEOUT * 60;
#endif
int port=SLP_PORT;
const char *addr = LOCALHOST_IP;
const char *iface = NULL;
static Boolean verbose;

//httpAttrs1 is test data used for registration of  http port.
const char *httpAttrs1 = "(template-url-syntax=service:wbemtest:http://127.0.0"
    ".1:5988),(service-hi-name=Pegasus),(service-hi-description=Pegasus "
    "CIM Server Version 2.6.0 Development),"
    "(service-id=PG:1161621217468-127-0-0-1),(Namespace=interop,root/"
    "benchmark,root/SampleProvider,root/PG_Internal,root/test/A,root/test/B,"
    "test/EmbeddedInstance/Static,test/TestProvider,test/EmbeddedInstance/"
    "Dynamic,root/cimv2,root,test/cimv2,test/static),(Classinfo=Unknown,"
    "Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,"
    "Unknown,Unknown,Unknown),(CommunicationMechanism=CIM-XML),"
    "(OtherCommunicationMechanismDescription=),"
    "(InteropSchemaNamespace=interop),"
    "(ProtocolVersion=1.0),(FunctionalProfilesSupported=Basic "
    "Read,Basic Write,Schema Manipulation,Instance Manipulation,Association "
    "Traversal, Qualifier Declaration,Indications),"
    " (FunctionalProfileDescriptions=), (MultipleOperationsSupported=FALSE),"
    " (AuthenticationMechanismsSupported=Basic)"
    ",(AuthenticationMechanismDescriptions=)";

const char *type = "service:wbemtest";
const char *type1 = "service:wbem";
const char *httpUrl1 = "service:wbemtest:http://127.0.0.1:5988";

//httpAttrs2 is test data used for registration of  https port.
const char *httpAttrs2 ="(template-url-syntax=service:wbemtest:https://127.0.0"
    ".1:5989),(service-hi-name=Pegasus),(service-hi-description=Pegasus CIM "
    "Server Version 2.6.0 Development),"
    "(service-id=PG:1161621217468-127-0-0-1),(Namespace=interop,root/"
    "benchmark,root/SampleProvider,root/PG_Internal,root/test/A,root/test/B,"
    "test/EmbeddedInstance/Static,test/TestProvider,test/EmbeddedInstance/"
    "Dynamic,root/cimv2,root,test/cimv2,test/static),(Classinfo=Unknown,"
    "Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,"
    "Unknown,Unknown,Unknown),(CommunicationMechanism=CIM-XML),"
    "(OtherCommunicationMechanismDescription=),"
    "(InteropSchemaNamespace=interop),"
    "(ProtocolVersion=1.0), (FunctionalProfilesSupported=Basic "
    "Read,Basic Write,Schema Manipulation,Instance Manipulation,Association "
    "Traversal,Qualifier Declaration,Indications),"
    "(FunctionalProfileDescriptions=),(MultipleOperationsSupported=FALSE),"
    "(AuthenticationMechanismsSupported=Basic),"
    "(AuthenticationMechanismDescriptions=)";

const char *httpUrl2 = "service:wbemtest:https://127.0.0.1:5989";

int find (char *str, const char *t)
{

    int len = strlen(str);
    int tlen = strlen(t)-1;
    int i=0,index=-1,j=0;
    int found=1;
    int save=0;
    while (len>i)
    {
        save=i;
        if (str[i]==t[0])
        {
            index = i;
            while (j<tlen && i<len)
            {
                if (str[i]==t[j])
                {
                    i++;
                    j++;
                }
                else
                {
                    found=0;
                    break;
                }
            }
            if (found)
            {
                return(index);
            }
            i = save + 1;
        }
        else
        {
            i+=1;
            if (len==i)
            {
                return(-1);
            }
        }
    }
    return(-1);
}

char* replace (char *s, const char *t, const char *substitute)
{
    if (s == NULL)
    {
        return(NULL);
    }

    int lendiff = 0;
    int len = strlen(t);
    lendiff = strlen(substitute) - len;
    char *substr = (char *)malloc(sizeof(char) * (strlen(s) + 1 + lendiff));
    int index = find(s,t);
    strncpy(substr,s,index);
    substr[index]='\0';
    strcat(substr,substitute);
    char *finalstr = s + index + len;
    strcat(substr,finalstr);
    return(substr);
}


// Parse the list and check for the required data.
// Returns 'true' if the required data is found otherwise
// returns 'false'
BOOL parseFind(lslpMsg *responses,  const char* httpAttr)
{
    BOOL found = false;
    lslpURL *url_list;
    lslpMsg *temp;

    temp = responses->next;
    while (1)
    {
        if ( (temp->type == srvRply) && (NULL != temp->msg.srvRply.urlList)
            && (! _LSLP_IS_EMPTY(temp->msg.srvRply.urlList)))
        {/*start of url if*/
            url_list = temp->msg.srvRply.urlList->next;
            while (! _LSLP_IS_HEAD(url_list) && !found)
            {/*start of url while */
                if (NULL != url_list->attrs &&
                    ! _LSLP_IS_HEAD(url_list->attrs->next))
                {
                    lslpAtomList *attrs = url_list->attrs->next;
                    //while traversing attr list
                    while (! _LSLP_IS_HEAD (attrs) && attrs->str
                        && strlen (attrs->str) && !found)
                    {
                        if (!String::compare (attrs->str,httpAttr))
                        {
                            found = true;
                            break;
                        }
                        attrs = attrs->next;
                    }  //while traversing attr list
                }
                url_list = url_list->next;
                // if there is another url, print a record separator
            } // while traversing url list
            if (found)
            {
                break;
            }
        } // if there are urls to print
        if (temp->next == responses)
        {
            break;
        }
        temp = temp->next;
    }

    return found;
}

/* Registration and verification for http
   This testcase register cimserver with http port and  checks if the
   registration is succesful or not. If the registration fails tests are
   terminated. It will also check whether the data registered with SLP is
   same as the input data used for registration or not. If not test will be
   terminated.
*/

void test1 ()
{
    struct slp_client *client;
    lslpMsg responses,*temp;

    //Create slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    /* Register with SLP using http. This assert would fail if SLP SA is not
       running */
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes,life ));

    // discover all responses
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;

    // retrieve the response head.
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    // parse the response list and check for the required response.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs1));

    destroy_slp_client(client);
    return;
}

/* This testcase register cimserver with http and https port and  checks if
   the registration is succesful or not. If the registration fails tests are
   terminated. It will also check whether the data registered with SLP is same
   as the input data used for registration or not. If not test will be
   terminated. It will also deregister the registration for both http port and
   https port. If the registration fails testcase will be terminated.
*/

void test2 ()
{
    struct slp_client *client;
    lslpMsg responses,*temp,*temp1;

    // Creates slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",scopes,
        FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    //Register http  with SLP. This assert would fail if SLP SA is not running
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs1));

    //Register http with SLP. This assert would fail if SLP SA is not running
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl2, httpAttrs2,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs2));

    // Unregister http
    PEGASUS_TEST_ASSERT(client->srv_reg_local (client, httpUrl1, httpAttrs1,
        type, scopes,0));
    System::sleep(1);  //This gives time to SA to clean up it datastructures

    client->converge_srv_req (client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    client->get_response (client, &responses);
    temp1 = &responses;
    // Check if unregister service can be found
    PEGASUS_TEST_ASSERT(!parseFind(temp1, httpAttrs1));

    //Unregister https
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl2, httpAttrs2,
        type, scopes,0));

    System::sleep(1);  //This gives time to SA to clean up it datastructures

    client->converge_srv_req (client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    client->get_response (client, &responses);
    temp1 = &responses;

    // Check if unregister service can be found
    PEGASUS_TEST_ASSERT(!parseFind(temp1, httpAttrs2));

    destroy_slp_client(client);
    return;
}

/* This testcase register CIMServer with http port and validates the
   registration. Deregisters the registered service and check if we can
   find the unregistered service. If found testcase would fail else passed.
*/

void test3 ()
{
    struct slp_client *client;
    lslpMsg responses,*temp;

    // Creates slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    /* Register with SLP using http. This assert would fail if SLP SA is not
       running */
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs1));

    //Unregister http
    PEGASUS_TEST_ASSERT(client->srv_reg_local (client, httpUrl1, httpAttrs1,
        type, scopes, 0));

    System::sleep(1);  //This gives time to SA to clean up it datastructures

    client->converge_srv_req (client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    client->get_response (client, &responses);
    temp = &responses;

    // Check if unregister service can be found
    PEGASUS_TEST_ASSERT(!parseFind(temp, httpAttrs1));

    destroy_slp_client(client);
    return;
}

// To check if a registration is done correctrly or not.
// This would validate service registration and registerdata for the service.
// Also tests for a non-existent service.

int test4()
{
    struct slp_client *client;
    lslpMsg responses,*temp;

    // Creates slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    /* Register with SLP using http. This assert would fail if SLP SA is not
       running */
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs1));

    temp = &responses;
    //Check if unregistered data can can be found
    PEGASUS_TEST_ASSERT (!parseFind(temp, httpAttrs2));

    destroy_slp_client(client);
    return( 32 );
}

// This testcase would test registration of http and https ports
// together.

void test5()
{
    struct slp_client *client;
    lslpMsg responses,*temp;

    // Creates slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    /* Register with SLP using http. This assert would fail if SLP SA is not
       running */
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs1));

    /* Register with SLP using https. This assert would fail if SLP SA is not
       running */
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl2, httpAttrs2,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs2));

    destroy_slp_client(client);
    return;
}

/* This testcase unregister a service and check for the empty response from
   the SA. */

void  test6()
{
    struct slp_client *client;
    lslpMsg responses, *temp;

    // Creates slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);
    PEGASUS_TEST_ASSERT(client->srv_reg_local (client, httpUrl1, httpAttrs1,
        type, scopes, 0));

    System::sleep(1);  //This gives time to SA to clean up it datastructures

    client->converge_srv_req (client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    client->get_response (client, &responses);
    temp = &responses;

    // Check if unregister service can be found
    PEGASUS_TEST_ASSERT(!parseFind(temp, httpAttrs1));

    destroy_slp_client(client);
    return;
}
// This testcase checks if the SLP allows duplicate registrations are not.
// SLP replaces the existing registration with the new registration. It will
// not maintain duplicate registrations.
void  test7()
{
    struct slp_client *client;
    lslpMsg responses,*temp;

    // Creates slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    /* Register with SLP using http. This assert would fail if SLP SA is
       not running */
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes, life));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs1));

    //Register with same data, as used in earlier registrtion.
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes, life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    //check if the registered data is same ot not.
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs1));

    destroy_slp_client(client);
    return;
}
// This testcase registers cimserver and validates the registration.
// Modifies the registration data and reregisters with modified data and
// validates if this reregistration is ocrrect or not.

void test8()
{
    struct slp_client *client;
    lslpMsg responses,*temp;
    char *changedata = (char *)NULL;

    // Creates slp client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    /* Register with SLP using http. This assert would fail if SLP SA is
       not running */
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, httpAttrs1,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    lslpURL *url_list;
    temp = responses.next;

    /* Parse through the list of registrations, identify the registration
       intended to be modified. Replace the service-hi-name in the
       registration data with "Changed". Register with the modified data and
       check for success case and failure case.
    */
    while(1)
    {
        if ( (temp->type == srvRply) && (NULL != temp->msg.srvRply.urlList) &&
            (! _LSLP_IS_EMPTY(temp->msg.srvRply.urlList)))//start of url if
        {
            url_list = temp->msg.srvRply.urlList->next;

            while (! _LSLP_IS_HEAD(url_list))//start of url while
            {
                if (NULL != url_list->attrs &&
                    ! _LSLP_IS_HEAD(url_list->attrs->next))
                {
                    lslpAtomList *attrs = url_list->attrs->next;
                    while (! _LSLP_IS_HEAD(attrs) && attrs->str
                        && strlen(attrs->str)) //while traversing attr list
                    {
                        if (!String::compare(attrs->str,httpAttrs1))
                        {
                            /* service-hi-name is changed from "Pegasus" to
                               "Changed" */
                            changedata = replace(
                                attrs->str,"Pegasus", "Changed");
                        }
                        attrs = attrs->next;
                    }  //while traversing attr list
                }
                // if there is another url, print a record separator
                url_list = url_list->next;
            } // while traversing url list
        } // if there are urls to print
        if (temp->next == &responses)
        {
            break;
        }
        temp = temp->next;
    }

    // Register with SLP using http with modified data. This assert would fail
    // if SLP SA is not running
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl1, changedata,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;

    // Check for the correctness of the modified data
    PEGASUS_TEST_ASSERT (parseFind(temp, changedata));

    temp = &responses;
    //Failure case -- Check for data prior to modification .
    PEGASUS_TEST_ASSERT (!parseFind(temp, httpAttrs1));

    free(changedata);
    destroy_slp_client(client);
    return;
}

/* Registration and verification of https
   This testcase register cimserver with https port and  checks if the
   registration is succesful or not. If the registration fails tests are
   terminated. It will also check whether the data registered with SLP is
   same as the input data used for registration or not. If not test will be
   terminated.
*/

void test9 ()
{
    struct slp_client *client;
    lslpMsg responses,*temp;

    // Create SLP Client
    client = create_slp_client (addr,iface,SLP_PORT,"DSA",
        scopes,FALSE,FALSE, 0);
    PEGASUS_TEST_ASSERT(NULL != client);

    // Register with SLP using https. This assert would fail if SLP SA is
    // not running
    PEGASUS_TEST_ASSERT(client->srv_reg_local(client, httpUrl2, httpAttrs2,
        type, scopes,life ));
    client->converge_srv_req(client, type, predicate, scopes);
    responses.isHead = TRUE;
    responses.next = responses.prev = &responses;
    PEGASUS_TEST_ASSERT(client->get_response (client, &responses));
    temp = &responses;
    PEGASUS_TEST_ASSERT (parseFind(temp, httpAttrs2));

    destroy_slp_client(client);
    return;
}


int main()
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    if (verbose)
    {
        PEGASUS_STD(cout)<<"+++++  SLP tests passed ++"<<PEGASUS_STD(endl);
    }
    return 0;
}
