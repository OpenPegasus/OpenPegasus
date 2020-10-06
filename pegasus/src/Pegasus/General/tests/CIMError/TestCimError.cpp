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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMInstance.h>

#include <Pegasus/General/CIMError.h>
#include <Pegasus/General/MofWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const char * verbose;
const char* PegasusOwningEntityName = "OpenPegasus";

/* this test just creates and prints an instance using the default constructor.
*/
void test01()
{
    if (verbose)
    {
        cout << "test01 started" << endl;
    }
    try
    {
    CIMError err1;
    CIMError err2;

        CIMInstance myInstance = err1.getInstance();
    //err2.setInstance(err1.getInstance());
        if (verbose)
        {
            cout << "test01: display of err1 CIMError instance" << endl;
            err1.print();
        }
    }
    catch(CIMException &e )
    {
        cout <<"CIMException test01: " << e.getMessage() << endl;
    PEGASUS_TEST_ASSERT(0);
    }
}
/* This test creates an instance with the default constructor and tests
 * the properties of the created instance.
*/
void test02()
{
    if (verbose)
    {
        cout << "test02 started" << endl;
    }
    try
    {
    CIMError err1;
        //Set all of the required fields from the default constructor

        err1.setOwningEntity(PegasusOwningEntityName);
        err1.setMessageID("MessageId");
        err1.setMessage("Text Of Message");
        err1.setPerceivedSeverity(CIMError::PERCEIVED_SEVERITY_LOW);
        err1.setProbableCause(CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION);
        err1.setCIMStatusCode(CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);
        if (verbose)
        {
            cout << "test02: display of err1 CIMError Instance" << endl;
            err1.print();
        }
        String rtnString;
        PEGASUS_TEST_ASSERT(err1.getOwningEntity(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == PegasusOwningEntityName);

        PEGASUS_TEST_ASSERT(err1.getMessageID(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == "MessageId");

        PEGASUS_TEST_ASSERT(err1.getMessage(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == "Text Of Message");

        CIMError::PerceivedSeverityEnum tmp1;
        PEGASUS_TEST_ASSERT(err1.getPerceivedSeverity(tmp1));
        PEGASUS_TEST_ASSERT(tmp1 == CIMError::PERCEIVED_SEVERITY_LOW);

        CIMError::ProbableCauseEnum tmp2;
        PEGASUS_TEST_ASSERT(err1.getProbableCause(tmp2));
        PEGASUS_TEST_ASSERT(
                tmp2 == CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION);

        CIMError::CIMStatusCodeEnum tmp3;
        PEGASUS_TEST_ASSERT(err1.getCIMStatusCode(tmp3));
        PEGASUS_TEST_ASSERT(tmp3 == CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);

        // Test remaining properties for NULL
        CIMError::ErrorTypeEnum errorType;
        PEGASUS_TEST_ASSERT(!err1.getErrorType(errorType));
        PEGASUS_TEST_ASSERT(!err1.getOtherErrorType(rtnString));
        Array<String> messageArguments;
        PEGASUS_TEST_ASSERT(!err1.getMessageArguments(messageArguments));

        PEGASUS_TEST_ASSERT(!err1.getProbableCauseDescription(rtnString));
        Array<String> recommendedActions;
        PEGASUS_TEST_ASSERT(!err1.getRecommendedActions(recommendedActions));
        PEGASUS_TEST_ASSERT(!err1.getErrorSource(rtnString));

        CIMError::ErrorSourceFormatEnum errorSourceFormat;
        PEGASUS_TEST_ASSERT(!err1.getErrorSourceFormat(errorSourceFormat));
        PEGASUS_TEST_ASSERT(!err1.getOtherErrorSourceFormat(rtnString));
        PEGASUS_TEST_ASSERT(!err1.getCIMStatusCodeDescription(rtnString));
        // Test the set and get with the completed instance
    CIMError err2;

    err2.setInstance(err1.getInstance());

        if (verbose)
        {
            cout << "Error instance err1:" << endl;
            err1.print();
            cout << "Error instance err2:" << endl;
            err2.print();
        }
    }
    catch(CIMException &e )
    {
        cout <<"CIMException Test02: " << e.getMessage() << endl;
    PEGASUS_TEST_ASSERT(0);
    }
}
/* Create a complete instance using the normal constructor and test
 * the properties
*/
void test03()
{
    if (verbose)
    {
        cout << "test03 started" << endl;
    }
    try
    {
    CIMError err1(PegasusOwningEntityName,
                  "MessageIDString",
                  "Text Of Message",
                  CIMError::PERCEIVED_SEVERITY_LOW,
                  CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
                  CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);
    {
        // Test the properties that should have values
        String rtnString;
        PEGASUS_TEST_ASSERT(err1.getOwningEntity(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == PegasusOwningEntityName);

        PEGASUS_TEST_ASSERT(err1.getMessageID(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == "MessageIDString");

        PEGASUS_TEST_ASSERT(err1.getMessage(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == "Text Of Message");

        CIMError::PerceivedSeverityEnum tmp1;
        PEGASUS_TEST_ASSERT(err1.getPerceivedSeverity(tmp1));
        PEGASUS_TEST_ASSERT(tmp1 == CIMError::PERCEIVED_SEVERITY_LOW);

        CIMError::ProbableCauseEnum tmp2;
        PEGASUS_TEST_ASSERT(err1.getProbableCause(tmp2));
        PEGASUS_TEST_ASSERT(
                tmp2 == CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION);

        CIMError::CIMStatusCodeEnum tmp3;
        PEGASUS_TEST_ASSERT(err1.getCIMStatusCode(tmp3));
        PEGASUS_TEST_ASSERT(tmp3 == CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);

        // Test remaining properties for NULL
        CIMError::ErrorTypeEnum errorType;
        PEGASUS_TEST_ASSERT(!err1.getErrorType(errorType));
        PEGASUS_TEST_ASSERT(!err1.getOtherErrorType(rtnString));
        Array<String> messageArguments;
        PEGASUS_TEST_ASSERT(!err1.getMessageArguments(messageArguments));

        PEGASUS_TEST_ASSERT(!err1.getProbableCauseDescription(rtnString));
        Array<String> recommendedActions;
        PEGASUS_TEST_ASSERT(!err1.getRecommendedActions(recommendedActions));
        PEGASUS_TEST_ASSERT(!err1.getErrorSource(rtnString));

        CIMError::ErrorSourceFormatEnum errorSourceFormat;
        PEGASUS_TEST_ASSERT(!err1.getErrorSourceFormat(errorSourceFormat));
        PEGASUS_TEST_ASSERT(!err1.getOtherErrorSourceFormat(rtnString));
        PEGASUS_TEST_ASSERT(!err1.getCIMStatusCodeDescription(rtnString));

        if (verbose)
        {
            cout << "CIM_Error instance err1:" << endl;
            err1.print();
        }

    }

    // Copy and retest the new error instance for
    // same property characteristics.
    CIMError err2;
    err2.setInstance(err1.getInstance());

    {
        // Test the properties that should have values
        String rtnString;
        PEGASUS_TEST_ASSERT(err2.getOwningEntity(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == PegasusOwningEntityName);

        PEGASUS_TEST_ASSERT(err2.getMessageID(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == "MessageIDString");

        PEGASUS_TEST_ASSERT(err2.getMessage(rtnString));
        PEGASUS_TEST_ASSERT(rtnString == "Text Of Message");

        CIMError::PerceivedSeverityEnum tmp1;
        PEGASUS_TEST_ASSERT(err2.getPerceivedSeverity(tmp1));
        PEGASUS_TEST_ASSERT(tmp1 == CIMError::PERCEIVED_SEVERITY_LOW);

        CIMError::ProbableCauseEnum tmp2;
        PEGASUS_TEST_ASSERT(err2.getProbableCause(tmp2));
        PEGASUS_TEST_ASSERT(
                tmp2 == CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION);

        CIMError::CIMStatusCodeEnum tmp3;
        PEGASUS_TEST_ASSERT(err2.getCIMStatusCode(tmp3));
        PEGASUS_TEST_ASSERT(tmp3 == CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);

        // Test remaining properties for NULL
        CIMError::ErrorTypeEnum errorType;
        PEGASUS_TEST_ASSERT(!err2.getErrorType(errorType));
        PEGASUS_TEST_ASSERT(!err2.getOtherErrorType(rtnString));
        Array<String> messageArguments;
        PEGASUS_TEST_ASSERT(!err2.getMessageArguments(messageArguments));

        PEGASUS_TEST_ASSERT(!err2.getProbableCauseDescription(rtnString));
        Array<String> recommendedActions;
        PEGASUS_TEST_ASSERT(!err2.getRecommendedActions(recommendedActions));
        PEGASUS_TEST_ASSERT(!err2.getErrorSource(rtnString));

        CIMError::ErrorSourceFormatEnum errorSourceFormat;
        PEGASUS_TEST_ASSERT(!err2.getErrorSourceFormat(errorSourceFormat));
        PEGASUS_TEST_ASSERT(!err2.getOtherErrorSourceFormat(rtnString));
        PEGASUS_TEST_ASSERT(!err2.getCIMStatusCodeDescription(rtnString));
    }


    }
    catch(CIMException &e )
    {
        cout <<"Unexpected CIMException test03 err2: " << e.getMessage()
            << endl;
    PEGASUS_TEST_ASSERT(0);
    }


    catch(Exception &e )
    {
        cout <<"Unexpected Exception test03: " << e.getMessage() << endl;
    PEGASUS_TEST_ASSERT(0);
    }
    catch(...)
    {
        cout << "Unexpected Exception test03: " << endl;
    }
}

//test04 Build a complete instance with all properties set and confirm that
// properties are correct. Then copy it and retest for valid properties. Tests
// the setting and getting of additional properties and the maintenance of
// property values through the copy.
// Finally reNull the extra properties and confirm that they get set back to
// Null.
void test04()
{
    if (verbose)
    {
        cout << "test03 started" << endl;
    }
    try
    {
        CIMError err1(PegasusOwningEntityName,
                      "MessageIDString",
                      "Text Of Message",
                      CIMError::PERCEIVED_SEVERITY_LOW,
                      CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
                      CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);

        // Set the additional properties in err1 to nonNull values.
        String rtnString;
        err1.setErrorType(CIMError::ERROR_TYPE_COMMUNICATIONS_ERROR);
        err1.setOtherErrorType("Some Other Type string");

        Array<String> setMessageArguments;
        setMessageArguments.append("one");
        setMessageArguments.append("two");
        err1.setMessageArguments(setMessageArguments);
        err1.setProbableCauseDescription("Probable Cause String");
        err1.setErrorSource("Error Source String");
        err1.setErrorSourceFormat(CIMError::ERROR_SOURCE_FORMAT_UNKNOWN);
        err1.setOtherErrorSourceFormat("Other Source Format String");
        err1.setCIMStatusCodeDescription("Error Description String");

        Array<String> recommendedActionsIn;
        recommendedActionsIn.append("jump over cliff");
        recommendedActionsIn.append("float out to sea");
        err1.setRecommendedActions(recommendedActionsIn);
        {
            if (verbose)
            {
                cout << "test04 err1 CIMError Instance:" << endl;
                err1.print();
            }

            // Test Extra properties for valid returns
            CIMError::ErrorTypeEnum errorType;
            PEGASUS_TEST_ASSERT(err1.getErrorType(errorType));
            PEGASUS_TEST_ASSERT(
                    errorType == CIMError::ERROR_TYPE_COMMUNICATIONS_ERROR);
            PEGASUS_TEST_ASSERT(err1.getOtherErrorType(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Some Other Type string");

            Array<String> messageArgumentsRtn;
            PEGASUS_TEST_ASSERT(err1.getMessageArguments(messageArgumentsRtn));
            PEGASUS_TEST_ASSERT(setMessageArguments == messageArgumentsRtn);

            PEGASUS_TEST_ASSERT(err1.getProbableCauseDescription(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Probable Cause String");

            Array<String> recommendedActionsRtn;
            PEGASUS_TEST_ASSERT(
                    err1.getRecommendedActions(recommendedActionsRtn));
            PEGASUS_TEST_ASSERT(recommendedActionsRtn == recommendedActionsIn);

            PEGASUS_TEST_ASSERT(err1.getErrorSource(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Error Source String");

            CIMError::ErrorSourceFormatEnum errorSourceFormatRtn;
            PEGASUS_TEST_ASSERT(
                    err1.getErrorSourceFormat(errorSourceFormatRtn));
            PEGASUS_TEST_ASSERT(
                errorSourceFormatRtn == CIMError::ERROR_SOURCE_FORMAT_UNKNOWN);


            PEGASUS_TEST_ASSERT(err1.getOtherErrorSourceFormat(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Other Source Format String");

            PEGASUS_TEST_ASSERT(err1.getCIMStatusCodeDescription(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Error Description String");

            // Test the required properties that should have values
            PEGASUS_TEST_ASSERT(err1.getOwningEntity(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == PegasusOwningEntityName);

            PEGASUS_TEST_ASSERT(err1.getMessageID(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "MessageIDString");

            PEGASUS_TEST_ASSERT(err1.getMessage(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Text Of Message");

            CIMError::PerceivedSeverityEnum tmp1;
            PEGASUS_TEST_ASSERT(err1.getPerceivedSeverity(tmp1));
            PEGASUS_TEST_ASSERT(tmp1 == CIMError::PERCEIVED_SEVERITY_LOW);

            CIMError::ProbableCauseEnum tmp2;
            PEGASUS_TEST_ASSERT(err1.getProbableCause(tmp2));
            PEGASUS_TEST_ASSERT(
                    tmp2 == CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION);

            CIMError::CIMStatusCodeEnum tmp3;
            PEGASUS_TEST_ASSERT(err1.getCIMStatusCode(tmp3));
            PEGASUS_TEST_ASSERT(
                    tmp3 == CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);
        }

        // copy to new CIMError and retest the properties.

        CIMError err2;
        err2.setInstance(err1.getInstance());
        {
            // Test Extra properties for valid returns
            CIMError::ErrorTypeEnum errorType;
            PEGASUS_TEST_ASSERT(err2.getErrorType(errorType));
            PEGASUS_TEST_ASSERT(
                    errorType == CIMError::ERROR_TYPE_COMMUNICATIONS_ERROR);
            PEGASUS_TEST_ASSERT(err2.getOtherErrorType(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Some Other Type string");

            Array<String> messageArgumentsRtn;
            PEGASUS_TEST_ASSERT(err2.getMessageArguments(messageArgumentsRtn));
            PEGASUS_TEST_ASSERT(setMessageArguments == messageArgumentsRtn);

            PEGASUS_TEST_ASSERT(err2.getProbableCauseDescription(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Probable Cause String");

            Array<String> recommendedActionsRtn;
            PEGASUS_TEST_ASSERT(
                    err2.getRecommendedActions(recommendedActionsRtn));
            PEGASUS_TEST_ASSERT(recommendedActionsRtn == recommendedActionsIn);

            PEGASUS_TEST_ASSERT(err2.getErrorSource(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Error Source String");

            CIMError::ErrorSourceFormatEnum errorSourceFormatRtn;
            PEGASUS_TEST_ASSERT(
                    err2.getErrorSourceFormat(errorSourceFormatRtn));
            PEGASUS_TEST_ASSERT(
                errorSourceFormatRtn == CIMError::ERROR_SOURCE_FORMAT_UNKNOWN);


            PEGASUS_TEST_ASSERT(err2.getOtherErrorSourceFormat(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Other Source Format String");

            PEGASUS_TEST_ASSERT(err2.getCIMStatusCodeDescription(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Error Description String");

            // Test the required properties that should have values
            PEGASUS_TEST_ASSERT(err2.getOwningEntity(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == PegasusOwningEntityName);

            PEGASUS_TEST_ASSERT(err2.getMessageID(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "MessageIDString");

            PEGASUS_TEST_ASSERT(err2.getMessage(rtnString));
            PEGASUS_TEST_ASSERT(rtnString == "Text Of Message");

            CIMError::PerceivedSeverityEnum tmp1;
            PEGASUS_TEST_ASSERT(err2.getPerceivedSeverity(tmp1));
            PEGASUS_TEST_ASSERT(tmp1 == CIMError::PERCEIVED_SEVERITY_LOW);

            CIMError::ProbableCauseEnum tmp2;
            PEGASUS_TEST_ASSERT(err2.getProbableCause(tmp2));
            PEGASUS_TEST_ASSERT(
                    tmp2 == CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION);

            CIMError::CIMStatusCodeEnum tmp3;
            PEGASUS_TEST_ASSERT(err2.getCIMStatusCode(tmp3));
            PEGASUS_TEST_ASSERT(
                    tmp3 == CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);
        }

        // Set the extraProperties back to Null and confirm that they
        // are Null and that the
        // others are not.

        err1.setErrorType(CIMError::ERROR_TYPE_COMMUNICATIONS_ERROR, true);
        err1.setOtherErrorType("Some Other Type string", true);

        err1.setMessageArguments(setMessageArguments, true);
        err1.setProbableCauseDescription("Probable Cause String", true);
        err1.setErrorSource("Error Source String", true);
        err1.setErrorSourceFormat(CIMError::ERROR_SOURCE_FORMAT_UNKNOWN, true);
        err1.setOtherErrorSourceFormat("Other Source Format String", true);
        err1.setCIMStatusCodeDescription("Error Description String", true);

        err1.setRecommendedActions(recommendedActionsIn, true);
        {
            // Test Extra properties for Null. They should now be null
            CIMError::ErrorTypeEnum errorType;
            PEGASUS_TEST_ASSERT(!err1.getErrorType(errorType));
            PEGASUS_TEST_ASSERT(!err1.getOtherErrorType(rtnString));

            Array<String> messageArgumentsRtn;
            PEGASUS_TEST_ASSERT(
                    !err1.getMessageArguments(messageArgumentsRtn));
            PEGASUS_TEST_ASSERT(!err1.getProbableCauseDescription(rtnString));

            Array<String> recommendedActionsRtn;
            PEGASUS_TEST_ASSERT(
                    !err1.getRecommendedActions(recommendedActionsRtn));
            PEGASUS_TEST_ASSERT(!err1.getErrorSource(rtnString));

            CIMError::ErrorSourceFormatEnum errorSourceFormatRtn;
            PEGASUS_TEST_ASSERT(
                    !err1.getErrorSourceFormat(errorSourceFormatRtn));

            PEGASUS_TEST_ASSERT(!err1.getOtherErrorSourceFormat(rtnString));
            PEGASUS_TEST_ASSERT(!err1.getCIMStatusCodeDescription(rtnString));
        }
    }
    catch(CIMException &e )
    {
        cout <<"Unexpected CIMException test04: " << e.getMessage() << endl;
    PEGASUS_TEST_ASSERT(0);
    }

}
int main()
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    test01();
    test02();
    test03();
    test04();
    printf("+++++ passed all tests\n");

    return 0;
}
