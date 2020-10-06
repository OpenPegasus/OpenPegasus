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

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/InternalException.h>

#include <Pegasus/Handler/IndicationFormatter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose = false;

CIMInstance _createSubscriptionInstance()
{
    CIMInstance subscriptionInstance (
            PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION);

    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
    CIMValue((String) "filter1")));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
    CIMValue((String) "handler1")));
    return (subscriptionInstance);
}

CIMInstance _createSubscriptionInstance
    (const String & textFormat,
     const Array<String> & textFormatParams)
{
    CIMInstance subscriptionInstance (
            PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION);

    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
    CIMValue((String) "filter1")));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
    CIMValue((String) "handler1")));
    subscriptionInstance.addProperty (CIMProperty
    (CIMName ("TextFormat"), textFormat));
    subscriptionInstance.addProperty (CIMProperty
    (CIMName ("TextFormatParameters"), textFormatParams));

    return (subscriptionInstance);

}

CIMInstance _createIndicationInstance1()
{
    CIMInstance indicationInstance("Test_IndicationProviderClass");

    indicationInstance.addProperty
    (CIMProperty (CIMName ("IndicationTime"), CIMValue(
    CIMDateTime("20050510143211.000000-420"))));
    indicationInstance.addProperty
    (CIMProperty (CIMName ("IndicationIdentifier"), CIMValue(Uint32(1))));
    indicationInstance.addProperty
    (CIMProperty ("MethodName", CIMValue(String("testIndicationFormat"))));
    indicationInstance.addProperty
    (CIMProperty (CIMName ("TestBoolean"), CIMValue(Boolean(true))));

    return (indicationInstance);
}

CIMInstance _createIndicationInstance2(CIMType type)
{
    CIMInstance indicationInstance("Test_IndicationProviderClass");

    indicationInstance.addProperty
    (CIMProperty (CIMName ("IndicationTime"), CIMValue(
    CIMDateTime("20050510143211.000000-420"))));
    indicationInstance.addProperty
    (CIMProperty (CIMName ("TestBoolean"), CIMValue(Boolean(true))));

    switch (type)
    {
        case CIMTYPE_UINT8:
        {
            Array<Uint8> uint8_action;
            uint8_action.append(1);
            uint8_action.append(2);
            uint8_action.append(3);
            uint8_action.append(4);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), uint8_action));

            break;
        }
        case CIMTYPE_UINT16:
        {
            Array<Uint16> uint16_action;
            uint16_action.append(16);
            uint16_action.append(26);
            uint16_action.append(36);
            uint16_action.append(46);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), uint16_action));

            break;
        }
        case CIMTYPE_UINT32:
        {
            Array<Uint32> uint32_action;
            uint32_action.append(32);
            uint32_action.append(42);
            uint32_action.append(52);
            uint32_action.append(62);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), uint32_action));

            break;
        }
        case CIMTYPE_UINT64:
        {
            Array<Uint64> uint64_action;
            uint64_action.append(64);
            uint64_action.append(74);
            uint64_action.append(84);
            uint64_action.append(94);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), uint64_action));

            break;
        }
        case CIMTYPE_SINT8:
        {
            Array<Sint8> sint8_action;
            sint8_action.append(-1);
            sint8_action.append(-2);
            sint8_action.append(-3);
            sint8_action.append(-4);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), sint8_action));

            break;
        }
        case CIMTYPE_SINT16:
        {
            Array<Sint16> sint16_action;
            sint16_action.append(-16);
            sint16_action.append(-26);
            sint16_action.append(-36);
            sint16_action.append(-46);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), sint16_action));

            break;
        }
        case CIMTYPE_SINT32:
        {
            Array<Sint32> sint32_action;
            sint32_action.append(-32);
            sint32_action.append(-42);
            sint32_action.append(-52);
            sint32_action.append(-62);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), sint32_action));

            break;
        }
        case CIMTYPE_SINT64:
        {
            Array<Sint64> sint64_action;
            sint64_action.append(-64);
            sint64_action.append(-74);
            sint64_action.append(-84);
            sint64_action.append(-94);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), sint64_action));

            break;
        }
    case CIMTYPE_STRING:
        {
            Array<String> string_action;
            string_action.append("string_action1");
            string_action.append("string_action2");
            string_action.append("string_action3");
            string_action.append("string_action4");

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), string_action));

            break;
        }
    case CIMTYPE_CHAR16:
        {
            Array<Char16> char16_action;
            char16_action.append('a');
            char16_action.append('b');
            char16_action.append('c');
            char16_action.append('d');

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), char16_action));

            break;
        }
    case CIMTYPE_BOOLEAN:
        {
            Array<Boolean> boolean_action;
            boolean_action.append(true);
            boolean_action.append(false);
            boolean_action.append(true);
            boolean_action.append(false);

            indicationInstance.addProperty (CIMProperty
                (CIMName ("Action"), boolean_action));

            break;
        }

    case CIMTYPE_REAL32:
    case CIMTYPE_REAL64:
    case CIMTYPE_DATETIME:
    case CIMTYPE_REFERENCE:
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        {
            break;
        }

    }
    return (indicationInstance);
}

void _checkSyntax(const String& textFormat)
{
    Array<String> textFormatParams;
    textFormatParams.append("IndicationTime");
    textFormatParams.append("IndicationIdentifier");
    textFormatParams.append("MethodName");

    if (verbose)
    {
        cout << "\n+++++ Test textFormat syntax: " << textFormat.getCString()
            << endl;
    }


    Array<String> actions;
    actions.append("action1");
    actions.append("action2");
    actions.append("action3");

    CIMClass indicationClass("Test_IndicationProviderClass");

    indicationClass.addProperty (CIMProperty (CIMName ("IndicationTime"),
    CIMValue(CIMDateTime("20050510143211.000000-420"))));
    indicationClass.addProperty (CIMProperty (CIMName ("IndicationIdentifier"),
    CIMValue((Uint32) 1)));
    indicationClass.addProperty (CIMProperty (CIMName ("MethodName"),
    CIMValue((String) "generateIndication")));
    indicationClass.addProperty (CIMProperty (CIMName ("Action"), actions));

    Boolean gotException=false;

    // tests an invalid text format string
    try
    {
        IndicationFormatter::validateTextFormat(textFormat, indicationClass,
                                                textFormatParams);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_INVALID_PARAMETER)
        {
            gotException = true;
            if (verbose)
            {
                cout << e.getMessage()  << endl;
            }
        }
        else
        {
            throw;
        }
    }
    PEGASUS_TEST_ASSERT(gotException);

}

void _validateTextFormat()
{
    Array<String> textFormatParams;
    textFormatParams.append("IndicationTime");
    textFormatParams.append("IndicationIdentifier");
    textFormatParams.append("MethodName");
    textFormatParams.append("Action");

    CIMClass indicationClass("Test_IndicationProviderClass");

    // tests a valid text format string
    String textFormat =
    "The indication which is generated by {2,  string   } occurred at"
        " { 0 , datetime} with Indication ID { 1 }";

    if (verbose)
    {
        cout << "\n+++++ Test textFormat syntax: " << textFormat.getCString()
            << endl;
    }

    IndicationFormatter::validateTextFormat(
        textFormat, indicationClass, textFormatParams);

    // tests invalid text format strings
    textFormat =
    "The indication which is generated by {2, string occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by 2, string} occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {this is a test, string} occurred"
        " at {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {xxx1 , string} occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {1  xxx , string} occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {1 , xxxstring} occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {1 , xxx  string} occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {1 , string  xxx} occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {1[1 , string } occurred at"
        " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    // tests property index out of bounds
    textFormat.clear();
    textFormat =
    "The indication which is generated by {4 , string  } occurred at"
    " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    textFormat.clear();
    textFormat =
    "The indication which is generated by {-1 , string  } occurred at"
    " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    // Invalid property type
    textFormat.clear();
    textFormat =
    "The indication which is generated by {1 , newtype} occurred at"
    " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    // Incorrect property type
    textFormat.clear();
    textFormat =
    "The indication which is generated by {1 , int32} occurred at"
    " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    // Incorrect property type: array format, non-array property
    textFormat.clear();
    textFormat =
    "The indication which is generated by {2[] , string} occurred at"
    " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    // Incorrect property type: non-array format, array property
    textFormat.clear();
    textFormat =
    "The indication which is generated by {3 , string} occurred at"
    " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

    // Invalid index
    textFormat.clear();
    textFormat =
    "The indication which is generated by {3[x] , string} occurred at"
    " {0, datetime} with Indication ID {1, uint32}";
    _checkSyntax(textFormat);

}

// Tests a default indication text message
void _testDefaultFormat()
{
    ContentLanguageList contentLangs;

    contentLangs.append(LanguageTag("en-US-mn"));
    String expectedIndicationText = "Indication (default format):";

    expectedIndicationText.append(
    "IndicationTime = 20050510143211.000000-420,");

    if (verbose)
    {
        cout << "\n+++++ Test Default Format Indication Message" << endl;
    }

    expectedIndicationText.append(
    " IndicationIdentifier = 1, MethodName = testIndicationFormat,"
    " TestBoolean = true");

    CIMInstance subscriptionInstance = _createSubscriptionInstance();

    CIMInstance indicationInstance = _createIndicationInstance1();

    String formattedIndText =
    IndicationFormatter::getFormattedIndText(subscriptionInstance,
    indicationInstance, contentLangs);

    PEGASUS_TEST_ASSERT(formattedIndText == expectedIndicationText);
}

// Tests a formatted indication text message.
void _testFormat()
{
    ContentLanguageList contentLangs;

    contentLangs.append(LanguageTag("en-US-mn"));

    if (verbose)
    {
        cout << "\n+++++ Test A Formatted Indication Text Message" << endl;
    }

    String textFormat =
        "The prediction is {3, boolean }. A indication which is generated by"
        " {2, string} occurred at {0, datetime} with Indication ID {1, uint32}";

    Array<String> textFormatParams;
    textFormatParams.append("IndicationTime");
    textFormatParams.append("IndicationIdentifier");
    textFormatParams.append("MethodName");
    textFormatParams.append("TestBoolean");

    String expectedIndicationText =
    "The prediction is true. A indication which is generated by " \
        "testIndicationFormat occurred at 20050510143211.000000-420" \
        " with Indication ID 1";

    CIMInstance subscriptionInstance =
    _createSubscriptionInstance(textFormat, textFormatParams);

    CIMInstance indicationInstance = _createIndicationInstance1();

    String formattedIndText;
    formattedIndText = IndicationFormatter::getFormattedIndText(
    subscriptionInstance, indicationInstance, contentLangs);

    PEGASUS_TEST_ASSERT(formattedIndText == expectedIndicationText);
}

// checks whether the formatted indication text is same as expected
void _checkIndicationText(
    const String & textFormat,
    const Array<String> & textFormatParams,
    const String expectedIndicationText,
    const CIMType type,
    const ContentLanguageList contentLangs)
{
    CIMInstance subscriptionInstance =
    _createSubscriptionInstance(textFormat, textFormatParams);

    CIMInstance indicationInstance = _createIndicationInstance2(type);

    String formattedIndText = IndicationFormatter::getFormattedIndText(
        subscriptionInstance, indicationInstance, contentLangs);

    PEGASUS_TEST_ASSERT(formattedIndText == expectedIndicationText);
}

// Tests a formatted indication text message and one of the indication
// properties is an array property.
void _testArrayFormat()
{
    ContentLanguageList contentLangs;

    contentLangs.append(LanguageTag("en-US-mn"));

    if (verbose)
    {
        cout << "\n+++++ Test A Formatted Indication Text Message With"
                " An Array Property"
            << endl;
    }

    Array<String> textFormatParams;
    textFormatParams.append("IndicationTime");
    textFormatParams.append("Action");

    String expectedText =
    "A UPS Alert occurred at 20050510143211.000000-420. " \
        "The Following actions are recommended to resolve the alert: ";

    // test case 1: {1[], string}
    String textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], string}";

    String expectedIndicationText;
    String formattedIndText;
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append(
        "[string_action1,string_action2,string_action3,string_action4]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_STRING, contentLangs);

    // test case 2: {1[1], string}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], string}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("string_action2");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_STRING, contentLangs);

    // test case 3: {1[], uint8}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], uint8}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[1,2,3,4]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT8, contentLangs);

    // test case 4: {1[1], uint8}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], uint8}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("2");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT8, contentLangs);

    // test case 5: {1[], uint16}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], uint16}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[16,26,36,46]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT16, contentLangs);

    // test case 6: {1[1], uint16}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], uint16}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("26");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT16, contentLangs);

    // test case 7: {1[], uint32}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], uint32}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[32,42,52,62]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT32, contentLangs);

    // test case 8: {1[1], uint32}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], uint32}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("42");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT32, contentLangs);

    // test case 9: {1[], uint64}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], uint64}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[64,74,84,94]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT64, contentLangs);

    // test case 10: {1[1], uint64}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], uint64}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("74");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_UINT64, contentLangs);

    // test case 11: {1[], sint8}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], sint8}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[-1,-2,-3,-4]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT8, contentLangs);

    // test case 12: {1[1], sint8}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], sint8}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("-2");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT8, contentLangs);

    // test case 13: {1[], sint16}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], sint16}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[-16,-26,-36,-46]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT16, contentLangs);

    // test case 14: {1[1], sint16}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], sint16}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("-26");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT16, contentLangs);

    // test case 15: {1[], sint32}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], sint32}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[-32,-42,-52,-62]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT32, contentLangs);

    // test case 16: {1[1], sint32}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], sint32}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("-42");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT32, contentLangs);

    // test case 17: {1[], sint64}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], sint64}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[-64,-74,-84,-94]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT64, contentLangs);

    // test case 18: {1[1], sint64}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], sint64}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("-74");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_SINT64, contentLangs);

    // test case 19: {1[], boolean}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], boolean}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[true,false,true,false]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_BOOLEAN, contentLangs);

    // test case 20: {1[1], boolean}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], boolean}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("false");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_BOOLEAN, contentLangs);

    // test case 21: {1[], char16}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[], char16}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("[a,b,c,d]");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_CHAR16, contentLangs);

    // test case 22: {1[1], char16}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], char16}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("b");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_CHAR16, contentLangs);

    // test case 20: {1[1], boolean}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[1], boolean}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("false");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_BOOLEAN, contentLangs);
    // test case 21: array index out of bounds {1[-1], string}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[-1], string}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("UNKNOWN");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_STRING, contentLangs);

    // test case 22: array index out of bounds {1[4], string}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[4], string}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("UNKNOWN");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_STRING, contentLangs);

    // test case 23: array index with whitespace {1[ \t \n 0 \r   \n], string}
    textFormat.clear();
    expectedIndicationText.clear();
    textFormat =
        "A UPS Alert occurred at {0, datetime}. The Following actions are"
        " recommended to resolve the alert: {1[ \t \n 0 \r   \n], string}";
    expectedIndicationText.append(expectedText);
    expectedIndicationText.append("string_action1");

    _checkIndicationText(textFormat, textFormatParams, expectedIndicationText,
                         CIMTYPE_STRING, contentLangs);
}

int main(int argc, char** argv)
{
    // Check command line option
    if (argc > 2)
    {
    cerr << "Usage: TestIndicationFormatter [-v]" << endl;
        return 1;
    }

    if (argc == 2)
    {
    const char *opt = argv[1];
        if (strcmp(opt, "-v") == 0)
        {
            verbose = true;
        }
        else
        {
            cerr << "Usage: TestIndicationFormatter [-v]" << endl;
            return 1;
        }
    }

    try
    {
    _validateTextFormat();
        _testDefaultFormat();
        _testFormat();
    _testArrayFormat();
    }
    catch (Exception& e)
    {
    cerr << "Error: " << e.getMessage() << endl;
    return -1;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;

}
