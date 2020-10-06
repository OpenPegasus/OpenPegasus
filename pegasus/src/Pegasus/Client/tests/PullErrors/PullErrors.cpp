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
//%////////////////////////////////////////////////////////////////////////////

/*
    These tests are part of a test program rather than a fixed test because
    a great number of them depend on sequences of operations in the server
    rather than single operations and cannot easily be tested with fixed
    comparison tests.

    Tests many error responses for pull operations.  This tests the various
    requests that will generate exception errors including:

    1. Inclusion of Filter and Filter Language parameters - We do not expect
       use of these parameters today and so block them in the server
    2. Execute Pull or Close operations when there is no Open context.
       Because of the way the client code operates we cannot perform this
       test. It generates a namespace error in the client code. Note that
       we can do some of this with item 3 below.
    4. Incorrect pull type after open.
    3. Bad context on Pull after an open enumeration.
       We cannot do this test in this program because we do not have access to
       the context to change ID.  This may have to be done with static tests
       and wbex.  Note: One way to do it here would be to establish a context
       and then time it out leaving a context object that should be invalid.
       Then use this one for the pull, producing a bad context response.
       KS_FUTURE build the above
    4. Null max object count exception. Exececute repeated pulls with
       0 objects until the exception is returned.
    5. Continue on error set - Pegasus does not allow this today so should
       always generate an exception
    6. Not closing or finishing an enumeration sequence. We have
       no way to confirm that the timeout, etc. really occurs because client
       does not have access to the server context.
       KS_FUTURE - Proposal.  Establish max number of contexts in the server.
       Start new ones until we get to the max.  Wait for the timeout and
       try again, indicating that the timeout cleaned out the open sequences.

   FUTURE SUGGESTIONS:
   1  Execute an operation on a closed enumeration context.
   2. EnumerationContextError on close - Execute Close after open
      with changed context.
   3. More error on input of invalid interoperation times and
      maxobjectCounts.
   4. Invalid roles, etc. on association requests.
*/

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>

#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/System.h>     // required for sleep function
#include <stdio.h>
#include <stdlib.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;
#define VCOUT if (verbose) cout

#define ENTRY VCOUT << "Enter "

/*  glob pattern matcher.
 *  The pattern expressions are:
 *    *             match zero or more characters
 *    ?             match any single character
 *    [charSet]     match any character in the character set
 *    [^charSet]    match any character NOT in the character set
 *                   A character set is a group of characters or ranges.
 *                   A range is written as two characters seperated with a
 *                   hyphen:
 *                      a-z denotes all characters between a to z inclusive.
 *    [-charSet]    set matches a literal hypen and any character in the set
 *    []charSet]    match a literal close bracket and any character in the set
 *
 *    char          match itself except where char is '*' or '?' or '['
 *    \char         match char, including any pattern character
 *
 * examples:
 *    a*c        ac abc abbc ...
 *    a?c        acc abc aXc ...
 *    a[a-z]c        aac abc acc ...
 *    a[-a-z]c    a-c aac abc ...
 */

bool _globMatch(const char* pattern, const char* str)
{
    bool negate;
    //bool match;
    char patChar;

    while (*pattern)
    {
        // if not end of str and pattern not any, close false
        if (!*str && *pattern != '*')
        {
            return false;
        }

        // Switch for current character in the pattern
        switch (patChar = *pattern++)
        {
            // "Any" patterh character found
            case '*':
                // ignore repeated * characters in pattern
                while (*pattern == '*')
                {
                    pattern++;
                }
                // close successful if end of pattern
                if (!*pattern)
                {
                    return true;
                }

                // if no escape characters match char by char
                if ((*pattern != '?')
                    && (*pattern != '[')
                    && (*pattern != '\\'))
                {
                    while (*str && *pattern != *str)
                    {
                        str++;
                    }
                }
                // recursive call to find shortest match
                while (*str)
                {
                    if (_globMatch(str, pattern))
                    {
                        return true;
                    }
                    str++;
                }
                return false;

            // Single char escape found. Test only that there is
            // a character in the string
            case '?':
                if (*str)
                {
                    break;
                }
                return false;

            // process character set. Character set is inclusive
            // and includes possibility of negation for first character
            case '[':
            {
                // test for character set negate flag (first after '['
                if (*pattern != '^')
                {
                    negate = false;
                }
                else
                {
                    negate = true;
                    pattern++;
                }

                bool match = false;
                while (!match && (patChar = *pattern++))
                {
                    if (!*pattern)
                    {
                        return false;
                    }
                    // if found char-range char (-)
                    if (*pattern == '-')
                    {
                        if (!*++pattern)
                        {
                            return false;
                        }

                        // if not end of pattern
                        if (*pattern != ']')
                        {
                            if (*str == patChar || *str == *pattern ||
                                (*str > patChar && *str < *pattern))
                            {
                                match = true;
                            }
                        }
                        else
                        {
                            if (*str >= patChar)
                            {
                                match = true;
                            }
                            break;
                        }
                    }
                    else
                    {
                        if (patChar == *str)
                        {
                            match = true;
                        }
                        if (*pattern != ']')
                        {
                            if (*pattern == *str)
                            {
                                match = true;
                            }
                        }
                        else
                            break;
                    }
                }

                if (negate == match)
                {
                    return false;
                }

                // If there is a match, skip past the character set and
                // continue
                while (*pattern && *pattern != ']')
                {
                    pattern++;
                }
                if (!*pattern++)
                {
                    return false;
                }
                break;
            }
            // escape character found
            case '\\':
                if (*pattern)
                {
                    patChar = *pattern++;
                }
            // default, test current character
            default:
                if (patChar != *str)
                {
                    return false;
                }
                break;
        }
        str++;
    }
    return !*str;
}

static int _globMatch(const String& pattern, const String& str)
{
    return _globMatch(pattern.getCString(), str.getCString());
}

////// Original match.  However, this one appears to have problems
////static int _match(const char* pattern, const char* str)
////{
////    const char* p;
////    const char* q;
////
////    /* Now match expression to str. */
////
////    for (p = pattern, q = str; *p && *q; )
////    {
////        if (*p == '*')
////        {
////            const char* r;
////
////            p++;
////
////            /* Recursively call to find the shortest match. */
////
////            for (r = q; *r; r++)
////            {
////                if (_match(p, r) == 0)
////                    break;
////            }
////
////            q = r;
////
////        }
////        else if (*p == *q)
////        {
////            p++;
////            q++;
////        }
////        else
////            return -1;
////    }
////
////    /* If src was exhausted but pattern has a single '*'remaining charcters,
////     * then match the result.
////     */
////
////    if (p[0] == '*' && p[1] == '\0')
////        return 0;
////
////    /* If anything left over, then they do not match. */
////
////    if (*p || *q)
////        return -1;
////
////    return 0;
////}
////
////static int _Match(const String& pattern, const String& str)
////{
////    return _match(pattern.getCString(), str.getCString());
////}

/***************************************************************************
    Class to test all of the different operations with various
    error scenarios.  The parameters are supplied once and each of the
    various opens can then be tested with the same set of parmeters.
    This avoids the duplicated test code for each open operation and
    each error scenario.
    There are several types of function:
       - set functions that set parameters for the tests
       - functions that execute particular operations (enumerateInstanceNames)
       - Functions that display information
       - functions that test results (test...())
*****************************************************************************/
class testEnumSequence{
public:
    //
    // The following are the arguments for calls
    CIMEnumerationContext _enumerationContext;
    CIMNamespaceName _nameSpace;
    CIMName _className;
    CIMObjectPath _cimObjectName;
    Boolean _deepInheritance;
    Boolean _includeClassOrigin;

    // Standard open... Arguments
    Boolean _endOfSequence;
    Uint32 _operationTimeout;
    Boolean _continueOnError;
    Uint32 _maxObjectCount;

    // the query parameters for open calls. Note that for
    // the openQueryInstances, this is wql
    String _filterQueryLanguage;
    String _filterQuery;

    // Argument for openQueryInstances
    CIMClass _queryResultClass;

    // Arguments for associator and reference calls
    String _role;
    String _resultRole;
    CIMName _assocClass;
    CIMName _resultClass;

    CIMPropertyList _cimPropertyList;

    CIMClient& _client;

    // Flags on tests for returns

    // set true if expect good return on open/pull. Default in constructor
    // is true.  Must set to false if expect error response on execution
    // of any operation.
    Boolean _expectGoodReturn;

    // Expected error code on response. Used in conjuction with the
    // binary messages above to evaluate responses
    CIMStatusCode _expectedCIMExceptionCode;
    String _expectedCIMExceptionMessage;

    // The status code returned by the last request operation if
    // it resulted in an exception
    CIMStatusCode _returnedCIMExceptionCode;

    // Names for the test and the operation being tested
    String _testName;
    String _operationName;

    //  Constructor sets up defaults for all parameters.
    testEnumSequence(CIMClient& client, const char*nameSpace);

    void setClientParams(CIMObjectPath& path,
                    Boolean deepInheritance,
                    Boolean includeClassOrigin)
    {
        _cimObjectName = path;
        _deepInheritance = deepInheritance;
        _includeClassOrigin = includeClassOrigin;
    }

    void setTestName(String testName)
    {
        _testName = testName;
    }

    void setExpectErrorResponseOnly()
    {
        _expectGoodReturn = false;
    }

    void setExpectGoodResponse()
    {
        _expectGoodReturn = true;
    }

    void setCIMException(CIMStatusCode code)
    {
        _expectedCIMExceptionCode = code;
    }
    void setCIMExceptionMessage(const char * msg)
    {
        _expectedCIMExceptionMessage = msg;
    }

    // Execute Client Operation functions


    // Execution of  defined Client request Operations
    // Returns true if successful or matches defined CIMException
    Boolean openEnumerateInstances();
    Boolean openEnumerateInstancePaths();
    Boolean openReferenceInstances();
    Boolean openReferenceInstancePaths();
    Boolean openAssociatorInstances();
    Boolean openAssociatorInstancePaths();
    Boolean openQueryInstances();

    Boolean pullInstancesWithPath();
    Boolean pullInstancePaths();
    Boolean pullInstances();

    // execute the open functions for both enumerateInstances and
    // enumerate instance names. These call simply aggregate
    // the above opens into a larger group to allow executing
    // a test on multiple open message types in a single call.
    Boolean executeEnumerateCalls();
    Boolean executeAssociationCalls();
    Boolean executeAllOpenCalls();
    Boolean executeAllOpenInstanceCalls();
    Boolean executeAllOpenPathCalls();

private:
    // default constructor and copy
    testEnumSequence();
    testEnumSequence(const testEnumSequence& that);

    // Test the execution for defined exceptions
    Boolean _testCIMException(CIMException& e);
    // Internal functions to display for the execution methods

    void _startOperationTest(const char* operationName)
    {
        _operationName = operationName;
        VCOUT << "Start " << _testName << " test. Operation "
            << _operationName << endl;
    }

    void _continueOperationTest(const char* operationName)
    {
        _operationName = operationName;
        VCOUT << "Continue " << _testName << " test. Operation "
            << _operationName << endl;
    }

    void _displayEndTest()
    {
        VCOUT << "End " << _testName << " test. Operation"
            << _operationName << endl;
    }
}; // End of Class testEnumSequence

// Constructor
testEnumSequence::testEnumSequence(CIMClient& client, const char*nameSpace):
    _nameSpace(nameSpace),
    _className(),
    _cimObjectName(),
    _deepInheritance(true),
    _includeClassOrigin(false),
    _operationTimeout(25),
    _continueOnError(false),
    _maxObjectCount(100),

    _filterQueryLanguage(""),
    _filterQuery(""),

    _role(""),
    _resultRole(""),
    _assocClass(CIMName()),
    _resultClass(CIMName()),

    _cimPropertyList(CIMPropertyList()),
    _client(client),
    _expectGoodReturn(true),
    _expectedCIMExceptionCode(CIM_ERR_SUCCESS),
    _expectedCIMExceptionMessage(""),
    _returnedCIMExceptionCode(CIM_ERR_SUCCESS),
    _testName("unknown"),
    _operationName("unknown")
{}

// Test against the CIMException defined by the one
// defined for the test. If not equal, generate error
// and return false.  If the message variable is set,
// also test the message against the one defined.
Boolean testEnumSequence::_testCIMException(CIMException& e)
{
    _returnedCIMExceptionCode = e.getCode();
    if (e.getCode() != _expectedCIMExceptionCode)
    {
        cerr << "CIMException Error: " << e.getCode() << " "
            << e.getMessage()
            << " Expected: "
            << _expectedCIMExceptionCode << " "
            << cimStatusCodeToString(_expectedCIMExceptionCode)
            << endl;
        return false;

    }
    else
    {
        VCOUT << "Correct Exception Code received." << _testName
              << " " << _operationName << " Exception "
              << e.getCode() << " " << e.getMessage() << endl;

        // test for correct message if required (i.e. test string not
        // empty

        if (_expectedCIMExceptionMessage != "")
        {
            if (_globMatch(_expectedCIMExceptionMessage,
                       e.getMessage()) != 0 )
            {
                cerr << "Received CIMException Message Error: |"
                     << e.getMessage()
                     << "| does not match expected CIMException |"
                     << _expectedCIMExceptionMessage << "|" << endl;
                return false;
            }
        }
    }
    return true;
}

// Execute the OpenEnumerateInstances with the arguments defined
Boolean testEnumSequence::openEnumerateInstances()
{
    _startOperationTest("openEnumerateInstances");

    try
    {
        Array<CIMInstance> cimInstances = _client.openEnumerateInstances(
            _enumerationContext,
            _endOfSequence,
            _nameSpace,
            _className,
            _deepInheritance,
            _includeClassOrigin,
            _cimPropertyList,
            _filterQueryLanguage,
            _filterQuery,
            _operationTimeout,
            _continueOnError,
            _maxObjectCount
            );
        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::openEnumerateInstancePaths()
{
    _startOperationTest("openEnumerateInstancePaths");
    CIMEnumerationContext enumerationContext;
    try
    {
        Array<CIMObjectPath> cimPaths =
            _client.openEnumerateInstancePaths(
            _enumerationContext,
            _endOfSequence,
            _nameSpace,
            _className,
            _filterQueryLanguage,
            _filterQuery,
            _operationTimeout,
            _continueOnError,
            _maxObjectCount);
        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::openReferenceInstances()
{
    _startOperationTest("openReferenceInstances");

    Array<CIMInstance> cimInstances;
    try
    {
        cimInstances = _client.openReferenceInstances(
            _enumerationContext,
            _endOfSequence,
            _nameSpace,
            _cimObjectName,
            _resultClass,
            _role,
            _includeClassOrigin,
            _cimPropertyList,
            _filterQueryLanguage,
            _filterQuery,
            _operationTimeout,
            _continueOnError,
            _maxObjectCount);
        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::openReferenceInstancePaths()
{
    _startOperationTest("openReferenceInstancePaths");

    Array<CIMObjectPath> cimInstancePaths;
    try
    {
        cimInstancePaths = _client.openReferenceInstancePaths(
            _enumerationContext,
            _endOfSequence,
            _nameSpace,
            _cimObjectName,
            _resultClass,
            _role,
            _filterQueryLanguage,
            _filterQuery,
            _operationTimeout,
            _continueOnError,
            _maxObjectCount);
        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::openAssociatorInstances()
{
    _startOperationTest("openAssociatorInstances");

    try
    {
        Array<CIMInstance> cimInstances = _client.openAssociatorInstances(
            _enumerationContext,
            _endOfSequence,
            _nameSpace,
            _cimObjectName,
            _assocClass,
            _resultClass,
            _role,
            _resultRole,
            _includeClassOrigin,
            _cimPropertyList,
            _filterQueryLanguage,
            _filterQuery,
            _operationTimeout,
            _continueOnError,
            _maxObjectCount);
            PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::openAssociatorInstancePaths()
{
    _startOperationTest("openAssociatorInstancePaths");

    Array<CIMObjectPath> cimInstancePaths;
    try
    {
        cimInstancePaths = _client.openAssociatorInstancePaths(
            _enumerationContext,
            _endOfSequence,
            _nameSpace,
            _cimObjectName,
            _assocClass,
            _resultClass,
            _role,
            _resultRole,
            _filterQueryLanguage,
            _filterQuery,
            _operationTimeout,
            _continueOnError,
            _maxObjectCount);
        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

// Note that for this operation, the filterQueryLanguage and
// _filterQuery are actually WQL or CQL, not FQL
Boolean testEnumSequence::openQueryInstances()
{
    _startOperationTest("openQueryInstances");

    Array<CIMInstance> cimInstances;
    try
    {
        cimInstances = _client.openQueryInstances(
            _enumerationContext,
            _endOfSequence,
            _nameSpace,
            _filterQueryLanguage,
            _filterQuery,
            _queryResultClass,
            _operationTimeout,
            _continueOnError,
            _maxObjectCount);
        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::pullInstancesWithPath()
{
    _continueOperationTest("pullInstancesWithPath");
    try
    {
        Array<CIMInstance> cimInstances = _client.pullInstancesWithPath(
            _enumerationContext,
            _endOfSequence,
            _maxObjectCount);

        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::pullInstancePaths()
{
    _continueOperationTest("pullInstancePaths");
    try
    {
        Array<CIMObjectPath> cimPaths = _client.pullInstancePaths(
            _enumerationContext,
            _endOfSequence,
            _maxObjectCount);

        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        return _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::pullInstances()
{
    _continueOperationTest("pullInstances");
    try
    {
        Array<CIMInstance> cimInstances = _client.pullInstances(
            _enumerationContext,
            _endOfSequence,
            _maxObjectCount);

        PEGASUS_TEST_ASSERT(_expectGoodReturn);
    }
    catch (CIMException& e)
    {
        _testCIMException(e);
    }
    _displayEndTest();
    return true;
}

Boolean testEnumSequence::executeEnumerateCalls()
{
    VCOUT << "execute tests for EnumerateFunctions" << endl;
    Boolean rtn1 = openEnumerateInstances();
    Boolean rtn2 = openEnumerateInstancePaths();
    return (rtn1 && rtn2)? true:false;
}

Boolean testEnumSequence::executeAssociationCalls()
{
    VCOUT << "execute tests for AssociateFunctions" << endl;
    Boolean rtn1 = openReferenceInstances();
    Boolean rtn2 = openReferenceInstancePaths();
    Boolean rtn3 = openAssociatorInstances();
    Boolean rtn4 = openAssociatorInstancePaths();
    return (rtn1 && rtn2 && rtn3 && rtn4)?true: false;
}

Boolean testEnumSequence::executeAllOpenCalls()
{
    VCOUT << "Execute tests for all open functions" << endl;
    Boolean rtn1 = executeEnumerateCalls();
    Boolean rtn2 = executeAssociationCalls();
    return (rtn1 && rtn2)? true : false;
}

Boolean testEnumSequence::executeAllOpenInstanceCalls()
{
    VCOUT << "Execute tests for all open Instance functions" << endl;

    Boolean rtn1 = openEnumerateInstances();
    Boolean rtn2 = openReferenceInstances();
    Boolean rtn3 = openAssociatorInstances();
    return (rtn1 && rtn2 && rtn3)? true : false;
}
Boolean testEnumSequence::executeAllOpenPathCalls()
{
    VCOUT << "Execute tests for all open Path functions" << endl;
    Boolean rtn1 = openEnumerateInstancePaths();
    Boolean rtn2 = openReferenceInstancePaths();
    Boolean rtn3 = openAssociatorInstancePaths();
    return (rtn1 && rtn2 && rtn3)? true : false;
}


// Parse Hostname input into name and port number
Boolean parseHostName(const String& inputName, String& hostName, Uint32& port)
{
    port = 5988;
    hostName = inputName;

    Uint32 pos;
    if (!((pos = inputName.reverseFind(':')) == PEG_NOT_FOUND))
    {
        Uint64 temp;
        if (StringConversion::decimalStringToUint64(
            hostName.subString(pos+1).getCString(), temp)
            &&
            StringConversion::checkUintBounds(temp,CIMTYPE_UINT32))
        {
            hostName.remove(pos);
            port = (Uint32)temp;
        }
        else
        {
            return false;
        }
    }
    return true;
}

// Create a connection to the host. defined by the
// argument host.  If the argument is empty, do a
// connect local
Boolean connectToHost(CIMClient& client, const String& inputName)
{
    // Try to connect.  If there is an optional argument use it as the
    // hostname, optional(port) to connect.
    try
    {
        if (inputName != "")
        {
            Uint32 port;
            String hostName;
            if (!parseHostName(inputName, hostName, port))
            {
                cerr << "Invalid hostName input " << inputName
                     << "format is hostname[:port]" << endl;
                return 1;
            }

            VCOUT << "Connect hostName = " << hostName << " port = "
                  << port << endl;
            client.connect(hostName, port, "", "");
        }
        else
        {
            VCOUT << "connectLocal" << endl;
            client.connectLocal();
        }
        return true;
    }

    catch (CIMException& e)
    {
            cerr << "CIMException Error: in connect: CIMException "
                 << e.getMessage() << endl;
    }
    catch (Exception& e)
    {
        cerr << "Error: in connect: Exception " << e.getMessage() << endl;
    }
    return false;
}
/* Test for the globMatch function we put into this code.
   For the moment, this is disabled since the function was tested.
   Included the test since this is new code.  If we move it to more general
   location, we will have the test.
   The test is enabled only if the define TEST_GLOBMATCH is enabled
   */
// #define TEST_GLOBMATCH
#ifdef TEST_GLOBMATCH
// test a single glob match call.
void tx(const char * pattern, const char * str, bool matches = true)
{
    VCOUT << "test pattern=" << pattern << " string=" << str
        << " " << (matches? "Should match" : "Should NOT match") << endl;
    if (matches)
    {
        PEGASUS_ASSERT(_globMatch(pattern, str));
    }
    else
    {
        PEGASUS_ASSERT(!_globMatch(pattern, str));
    }

}

// Test the glob match function
void testGlobMatch()
{
    tx("*","a");
    tx("*","abc");
    tx("?","a");
    tx("?ac","aac");
    tx("ac?","acx");
    tx("ac?","acxx", false);
    tx("a?c","abcx", false);
    tx("[","x", false);
    tx("[--z]","-");
    tx("???","abc");
    tx("???","abcd", false);
    tx("[--z]","z");
    tx("[-]abc","-abc");
    tx("[-a-]?","zd");
    tx("[-abc]?","dd", false);
    tx("[a-c][a-c][a-c]","abc");
    tx("[]","a",false);
    tx("[]-z]abc","^abc");
    tx("[]]abc","]abc");
    tx("[]abc[a]","a");
    tx("[]abc[de]","[");
    tx("[]abc[de]","]");
    tx("[]abc[de]","d");
    tx("[^a-z]","a", false);
    tx("[a-z]","A", false);
    tx("[a-z]","a");
    tx("[ab","a", false);
    tx("[abc]??[def]","abca", false);
    tx("[abc]??[def]","abcg",false);
    tx("[abc]??[def]","abcd");
    tx("[b-","a", false);
    tx("[z-a]","a");
    tx("[z-a]","z");
    tx("\\?\\?\\*!\\[abc\\]","??*![abc]");
    tx("ab[a-z]?a*b","abCdaxxxxb", false);
    tx("ab[a-z]?a*b","abc", false);
    tx("ab[a-z]?a*b","abcdab");
    tx("ab[a-z]?a*b","abcdaxxxxb");
    tx("xxxxxx","xxxxxx");
    tx("xxxxxx","xxxxxy", false);
    tx("x","xx", false);
    tx("","abc", false);
    tx("","");
    tx("","");
    cout << "Test of _globMatch passed" << endl;
}
#endif
    /**************************************************************************
    **
    **              Main. Starts client and executes each test.
    **
    ***************************************************************************/
int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");
    CIMClient client;

#ifdef TEST_GLOBMATCH
    testGlobMatch();
#endif

    String hostName = "";

    if (argc == 2)
    {
        hostName = argv[1];
    }

    if (!connectToHost(client, hostName))
    {
        cerr << "Error connection to host " << hostName << " failed." << endl;
        PEGASUS_ASSERT(false);
    }

    /**********************************************************************
    **
    **          Execute each individual test.  Each test is a single
    **          testEnumSequence instance where the paremeters are first
    **          set with calls to particular functions and then
    **          the test is executed with a call for each operation
    **          to be executed.  Multiple steps may be executed by
    **          calling, the various pull operations after the
    **          open has been executed.
    **
    **          As an option, test parameters
    **          can be reset and a testEnumSequence instance reused.
    **
    **********************************************************************/

    testEnumSequence tc(client, "test/TestProvider");
    tc.setTestName("Invalid Classname test");
    tc.setCIMException(CIM_ERR_INVALID_CLASS);
    tc._className =  "junk";
    PEGASUS_ASSERT(tc.executeEnumerateCalls());

    // Test invalid Object Path tests
    tc.setTestName("Invalid Classname in open associator/reference call");
    tc._cimObjectName = "junk.id=1";
    tc.setCIMException(CIM_ERR_INVALID_PARAMETER);
    PEGASUS_ASSERT(tc.executeAssociationCalls());

    // test Filter parameters on all operations
    tc.setTestName("Filter Parameter test- Using incorrect filterQuery");
    tc._className =  "CMPI_TEST_Person";
    tc._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
    tc._filterQuery="abc";
    tc.setCIMException(CIM_ERR_FAILED);
    PEGASUS_ASSERT(tc.executeAllOpenCalls());

#ifdef PEGASUS_ENABLE_FQL
    tc.setCIMException(CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED);
#else
    tc.setCIMException(CIM_ERR_FILTERED_ENUMERATION_NOT_SUPPORTED);
#endif
    tc.setTestName("Filter Instances Parameter test Using filter wth Bad Lang");
    tc._filterQueryLanguage="WQL";
    PEGASUS_ASSERT(tc.executeAllOpenInstanceCalls());

    tc.setTestName("Filter Paths Parameter test- Using filter and Language");
    tc._filterQueryLanguage="WQL";
    tc.setCIMException(CIM_ERR_FILTERED_ENUMERATION_NOT_SUPPORTED);
    PEGASUS_ASSERT(tc.executeAllOpenPathCalls());

    tc.setTestName("Filter Parameter test- Using filterQuery Language Only");
    tc._filterQuery="";
    tc.setCIMException(CIM_ERR_FAILED);
    PEGASUS_ASSERT(tc.executeAllOpenCalls());

    tc.setTestName("MaxObjectCount GT max allowed by Server");
    tc._filterQuery="";
    tc._filterQueryLanguage="";
    tc._maxObjectCount = 1000000;
    tc.setCIMException(CIM_ERR_INVALID_PARAMETER);
    PEGASUS_ASSERT(tc.executeAllOpenCalls());

    // ContinueOnError test. Should always error out
    tc.setTestName("ContinueOnError should always return exception");
    tc._maxObjectCount = 100;
    tc._continueOnError = true;
    tc.setCIMException(CIM_ERR_NOT_SUPPORTED);
    tc.setCIMExceptionMessage("ContinueOnError = true argument not supported");
    PEGASUS_ASSERT(tc.executeAllOpenCalls());
    tc.setCIMExceptionMessage("");

    //
    // Test invalid pull type not in accord with open type
    //
    {
        testEnumSequence tc3(client, "test/TestProvider");
        tc3.setTestName("OpenEnumerate followed by Incorrect Pull operation");
        tc3._className =  "CMPI_TEST_Person";
        tc3._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tc3._maxObjectCount = 0;
        PEGASUS_ASSERT(tc3.openEnumerateInstances());
        tc3.setCIMException(CIM_ERR_FAILED);
        tc3.setExpectErrorResponseOnly();
        PEGASUS_ASSERT(tc3.pullInstancePaths());

        // Now request the corect response
        tc3.setCIMException(CIM_ERR_INVALID_ENUMERATION_CONTEXT);
        tc3.setExpectErrorResponseOnly();
        PEGASUS_ASSERT(tc3.pullInstancesWithPath());
        tc3.setExpectGoodResponse();
    }
    {
        testEnumSequence tc3(client, "test/TestProvider");
        tc3.setTestName(
            "OpenEnumeratePaths followed by Incorrect Pull operation");
        tc3._className =  "CMPI_TEST_Person";
        tc3._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tc3._maxObjectCount = 0;
        PEGASUS_ASSERT(tc3.openEnumerateInstancePaths());
        tc3.setCIMException(CIM_ERR_FAILED);
        tc3.setExpectErrorResponseOnly();
        PEGASUS_ASSERT(tc3.pullInstancesWithPath());

        // Now request the corect response and expect error response
        tc3.setCIMException(CIM_ERR_INVALID_ENUMERATION_CONTEXT);
        tc3.setExpectErrorResponseOnly();
        PEGASUS_ASSERT(tc3.pullInstancePaths());
        tc3.setExpectGoodResponse();
    }

    // Test for a complete sequence that works.
    testEnumSequence tcgood(client, "test/TestProvider");
    tcgood.setTestName("Good complete open/pull response");
    tcgood._className =  "CMPI_TEST_Person";
    tcgood._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
    PEGASUS_ASSERT(tcgood.openEnumerateInstances());
    while(tcgood._endOfSequence == false)
    {
        PEGASUS_ASSERT(tcgood.pullInstancesWithPath());
    }

    /*  Interoperation Timeout tests.  Note that these tests cannot be
        completely confirmed since we have no way to test if the
        EnumerationContext is actually released, just that we get the
        error back
    */
    // test openEnumerateInstances and pull after timeout.
    tc.setTestName("Interoperation Timeout upon Pull");
    tc._className =  "CMPI_TEST_Person";
    tc._maxObjectCount = 1;
    tc._operationTimeout = 7;

    // execute the open call and then wait past timer to test for timeout
    PEGASUS_ASSERT(tc.openEnumerateInstances());
    VCOUT << "Wait for open operation to timeout. This should wait about"
             " 10 Seconds, return error, and then pass test." << endl;
    System::sleep(10);

    tc.setCIMException(CIM_ERR_INVALID_ENUMERATION_CONTEXT);
    PEGASUS_ASSERT(tc.pullInstancesWithPath());
    VCOUT << "Interoperation Timeout test passed. Note that we"
             " cannot do complete test without enum context table inspsection"
          << endl;

    //
    // Test for maximum simultaneous 0 object requests before reject
    //
    // Tests for enumerateInstancesWithPath and enumerateInstancePaths
    {
        testEnumSequence tczerotest(client, "test/TestProvider");
        tczerotest.setTestName("excessive consecutive zero length responses ");
        tczerotest._className =  "CMPI_TEST_Person";
        tczerotest._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tczerotest._maxObjectCount = 0;

        tczerotest.setCIMException(CIM_ERR_SERVER_LIMITS_EXCEEDED);
        PEGASUS_ASSERT(tczerotest.openEnumerateInstances());
        while(tczerotest._endOfSequence == false)
        {
            if (!tczerotest.pullInstancesWithPath())
            {
                break;
            }
            if (tczerotest._returnedCIMExceptionCode != CIM_ERR_SUCCESS)
            {
                break;
            }
        }
    }

    {
        testEnumSequence tczerotest(client, "test/TestProvider");
        tczerotest.setTestName("excessive consecutive zero length responses ");
        tczerotest._className =  "CMPI_TEST_Person";
        tczerotest._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tczerotest._maxObjectCount = 0;

        tczerotest.setCIMException(CIM_ERR_SERVER_LIMITS_EXCEEDED);
        PEGASUS_ASSERT(tczerotest.openEnumerateInstancePaths());
        while(tczerotest._endOfSequence == false)
        {
            if (!tczerotest.pullInstancePaths())
            {
                break;
            }
            if (tczerotest._returnedCIMExceptionCode != CIM_ERR_SUCCESS)
            {
                break;
            }
        }
    }

    // Tests for associatorInstances and associatorPaths
    {
        testEnumSequence tczerotest(client, "test/TestProvider");
        tczerotest.setTestName("excessive consecutive zero length responses ");
        tczerotest._className =  "CMPI_TEST_Person";
        tczerotest._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tczerotest._maxObjectCount = 0;

        tczerotest.setCIMException(CIM_ERR_SERVER_LIMITS_EXCEEDED);
        PEGASUS_ASSERT(tczerotest.openAssociatorInstances());
        while(tczerotest._endOfSequence == false)
        {
            if (!tczerotest.pullInstancesWithPath())
            {
                break;
            }
            if (tczerotest._returnedCIMExceptionCode != CIM_ERR_SUCCESS)
            {
                break;
            }
        }
    }
    {
        testEnumSequence tczerotest(client, "test/TestProvider");
        tczerotest.setTestName("excessive consecutive zero length responses ");
        tczerotest._className =  "CMPI_TEST_Person";
        tczerotest._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tczerotest._maxObjectCount = 0;

        tczerotest.setCIMException(CIM_ERR_SERVER_LIMITS_EXCEEDED);
        PEGASUS_ASSERT(tczerotest.openAssociatorInstancePaths());
        while(tczerotest._endOfSequence == false)
        {
            if (!tczerotest.pullInstancePaths())
            {
                break;
            }
            if (tczerotest._returnedCIMExceptionCode != CIM_ERR_SUCCESS)
            {
                break;
            }
        }
    }

    // Tests for associatorInstances and associatorPaths
    {
        testEnumSequence tczerotest(client, "test/TestProvider");
        tczerotest.setTestName("excessive consecutive zero length responses ");
        tczerotest._className =  "CMPI_TEST_Person";
        tczerotest._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tczerotest._maxObjectCount = 0;

        tczerotest.setCIMException(CIM_ERR_SERVER_LIMITS_EXCEEDED);
        PEGASUS_ASSERT(tczerotest.openReferenceInstances());
        while(tczerotest._endOfSequence == false)
        {
            if (!tczerotest.pullInstancesWithPath())
            {
                break;
            }
            if (tczerotest._returnedCIMExceptionCode != CIM_ERR_SUCCESS)
            {
                break;
            }
        }
    }

    {
        testEnumSequence tczerotest(client, "test/TestProvider");
        tczerotest.setTestName("excessive consecutive zero length responses ");
        tczerotest._className =  "CMPI_TEST_Person";
        tczerotest._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        tczerotest._maxObjectCount = 0;

        tczerotest.setCIMException(CIM_ERR_SERVER_LIMITS_EXCEEDED);
        PEGASUS_ASSERT(tczerotest.openReferenceInstancePaths());
        while(tczerotest._endOfSequence == false)
        {
            if (!tczerotest.pullInstancePaths())
            {
                break;
            }
            if (tczerotest._returnedCIMExceptionCode != CIM_ERR_SUCCESS)
            {
                break;
            }
        }
    }

    for (Uint32 i = 0; i < 10; i++)
    {
        testEnumSequence tc(client, "test/TestProvider");
        tc.setTestName("Interoperation Timeout after Open");
        tc._className =  "CMPI_TEST_Person";
        tc._maxObjectCount = 0;
        tc._operationTimeout = 7;
        // execute the open call and then wait past timer to test for timeout
        PEGASUS_ASSERT(tc.openEnumerateInstances());
    }

    {
        // Just make sure we can receive good responses
        // Test for a complete sequence that works.
        testEnumSequence tcgood(client, "test/TestProvider");
        tcgood.setTestName("Good complete open/pull response");
        tcgood._className =  "TST_ResponseStressTestCxx";
        tcgood._maxObjectCount = 1;
        //tcgood._cimObjectName = "CMPI_TEST_Person.name=\"Melvin\"";
        PEGASUS_ASSERT(tcgood.openEnumerateInstances());
        while(tcgood._endOfSequence == false)
        {
            PEGASUS_ASSERT(tcgood.pullInstancesWithPath());
        }
    }

    {
        // Test for a complete sequence that works.
        testEnumSequence tcgood(client, "test/TestProvider");
        tcgood.setTestName("Good complete open/pull response");
        tcgood._className =  "TST_ResponseStressTestCxx";
        tcgood._maxObjectCount = 1;
        PEGASUS_ASSERT(tcgood.openEnumerateInstancePaths());
        while(tcgood._endOfSequence == false)
        {
            PEGASUS_ASSERT(tcgood.pullInstancePaths());
        }
    }
    // Added to help server clean up all enum contexts before shutdown
    // Used to insure that the TestMakefile test works.
    System::sleep(30);
    cout << argv[0] <<" +++++ passed all tests" << endl;
    return 0;
}
