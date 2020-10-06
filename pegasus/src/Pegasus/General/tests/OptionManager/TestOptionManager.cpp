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

/**
    This file has testcases that are added to cover the functions in
    Pegasus/General/OptionManager.cpp.
*/

#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/PegasusAssert.h>

#include <Pegasus/General/OptionManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose = false;

void test01()
{
     OptionManager omobj;

    {
        Array<String> inDomainString;
        inDomainString.append("red");
        inDomainString.append("blue");
        inDomainString.append("green");

        Option oobj(String::EMPTY, "blue" , false,
                    Option::STRING, inDomainString);

        Boolean caughtException = false;

        try
        {
            Option oobjbad("BadOption", "blue" , false, Option::INTEGER);
        }
        catch(OMInvalidOptionValue &)
        {
            caughtException = true;
        }
        PEGASUS_TEST_ASSERT(caughtException == true);

        /**
            Added to cover the functions:
            const String& getOptionName() const
            void setOptionName(const String& optionName)
        */
        PEGASUS_TEST_ASSERT(oobj.getOptionName() == String::EMPTY);
        oobj.setOptionName("favoriteColor");
        PEGASUS_TEST_ASSERT(oobj.getOptionName() == "favoriteColor");

        /**
            Added to cover the function
            const String& getDefaultValue() const
            void setDefaultValue(const String& defaultValue)
        */
        PEGASUS_TEST_ASSERT(oobj.getDefaultValue() == "blue");
        oobj.setDefaultValue("green");
        PEGASUS_TEST_ASSERT(oobj.getDefaultValue() == "green");

        /**
            Added to cover the functions:
            const String& getValue() const
            void setValue(const String& value)
            Boolean isResolved() const
        */
        PEGASUS_TEST_ASSERT(oobj.getValue() == "blue");
        PEGASUS_TEST_ASSERT(oobj.isResolved() == false);
        oobj.setValue("red");
        PEGASUS_TEST_ASSERT(oobj.getValue() == "red");
        PEGASUS_TEST_ASSERT(oobj.isResolved() == true);

        /**
            Added to cover the functions:
            Boolean getRequired() const
            void setRequired(Boolean required)
        */
        PEGASUS_TEST_ASSERT(oobj.getRequired() == false);
        oobj.setRequired(true);
        PEGASUS_TEST_ASSERT(oobj.getRequired() == true);

        /**
            Added to cover the functions:
            const Array<String>& getDomain() const
            void setDomain(const Array<String>& domain)
        */
        PEGASUS_TEST_ASSERT(oobj.getDomain().size() == 3);
        PEGASUS_TEST_ASSERT(oobj.isValid("yellow") == false);
        inDomainString.append("yellow");
        PEGASUS_TEST_ASSERT(oobj.getDomain().size() == 3);
        oobj.setDomain(inDomainString);
        PEGASUS_TEST_ASSERT(oobj.getDomain().size() == 4);
        PEGASUS_TEST_ASSERT(oobj.isValid("yellow") == true);

        /**
            Added to cover the function
            const String& getCommandLineOptionName() const
            void setCommandLineOptionName(const String& commandLineOptionName)
        */
        PEGASUS_TEST_ASSERT(oobj.getCommandLineOptionName() == String::EMPTY);
        oobj.setCommandLineOptionName("favColor");
        PEGASUS_TEST_ASSERT(oobj.getCommandLineOptionName() == "favColor");

        /**
            Added to cover the function
            const String& getOptionHelpMessage() const
        */
        PEGASUS_TEST_ASSERT(oobj.getOptionHelpMessage() == String::EMPTY);

        /**
            Added to cover the function
            Option& Option(const Option& x)
            Type getType() const
        */
        Option oobj1(oobj);
        PEGASUS_TEST_ASSERT(oobj1.getType() == Option::STRING);

        /**
            Added to cover the function
            Boolean isValid(const String& value) const
            void setType(Type type)
        */

        oobj1.setType(Option::BOOLEAN);
        oobj1.setDomain(Array<String>());
        PEGASUS_TEST_ASSERT(oobj1.isValid("true") == true);
        PEGASUS_TEST_ASSERT(oobj1.isValid("false") == true);
        PEGASUS_TEST_ASSERT(oobj1.isValid("5") == false);
        oobj1.setType(Option::INTEGER);
        PEGASUS_TEST_ASSERT(oobj1.isValid("7") == true);
        PEGASUS_TEST_ASSERT(oobj1.isValid("7x") == false);
        PEGASUS_TEST_ASSERT(oobj1.isValid("7.2") == false);

        /**
            Added to cover the function
            Option& operator=(const Option& x)
        */
        Option oobj2(oobj);
        oobj2 = oobj1;
        PEGASUS_TEST_ASSERT(oobj2.getType() == Option::INTEGER);
   }

   {
        /**
            Added to cover the function
            void OptionManager::mergeFile(const String& fileName)
        */
        Boolean gotException = false;
        try
        {
            omobj.mergeFile("abc.cpp");
        }
        catch(NoSuchFile&)
        {
            gotException = true;
        }
        PEGASUS_TEST_ASSERT(gotException);
    }

    {
        /**
            Added to cover the function
            Boolean OptionManager::lookupValue(
            const String& name, String& value) const
        */
        Boolean ret_val = false;
        String value1;
        String &value = value1;
        ret_val = omobj.lookupValue("name", value);
        PEGASUS_TEST_ASSERT(ret_val == false);
    }

    {
        /**
            Added to cover the function
            Boolean OptionManager::lookupIntegerValue(
            const String& name, Uint32& value) const
        */
        Boolean ret_val = false;
        Uint32 value1 = 32;
        Uint32 &value = value1;

        ret_val = omobj.lookupIntegerValue("name", value);
        PEGASUS_TEST_ASSERT(ret_val == false);
    }

    {
        /**
            Added to cover the function
            Boolean OptionManager::valueEquals(
            const String& name, const String& value)
        */
        Boolean ret_val = false;
        String value1;
        String &value = value1;

        ret_val = omobj.valueEquals("name", value);
        PEGASUS_TEST_ASSERT(ret_val == false);
    }

    {
        /**
            Added to cover the function
            Boolean OptionManager::isTrue(const String& name) const
        */
        Boolean ret_val = false;
        String value1;
        const String &value = value1;
        ret_val = omobj.isTrue(value);
        PEGASUS_TEST_ASSERT(ret_val == false);
    }
}

// test building options and getting configuration file and command line
void test02(int argc, char** argv)
{
    OptionManager om;

    // Define a set of options
    static OptionRowWithMsg optionsTable[] =
    {
        {"debug", "false", false, Option::BOOLEAN, 0, 0, "d",
        "key1",
        "msg 1"},

        {"delay", "0", false, Option::WHOLE_NUMBER, 0, 0, "delay",
        "key2",
        "msg 2"},

        {"Password", "", false, Option::STRING, 0, 0, "p",
        "key3",
        "msg 3" },

        {"Name", "", true, Option::STRING, 0, 0, "n",
        "key4",
        "msg 4" },

        {"optional", "blah", false, Option::STRING, 0, 0, "o",
        "key6",
        "msg 6" },

        {"optional2", "999", false, Option::INTEGER, 0, 0, "o2",
        "key7",
        "msg 7" },

        {"configfile1", "blah", false, Option::STRING, 0, 0, "cf1",
        "key6",
        "msg 6" },

        {"configfile2", "999", false, Option::INTEGER, 0, 0, "cf2",
        "key7",
        "msg 7" }
    };
    const Uint32 NUM_OPTIONS = sizeof(optionsTable) / sizeof(optionsTable[0]);

    // Register all of the options in the table above
    om.registerOptions(optionsTable, NUM_OPTIONS);

    if (verbose)
    {
        cout << "argc = " << argc << endl;
        for (int i = 0; i < argc; i++)
        {
            cout << "argv[" << i << "] = " << argv[i] << endl;
        }
    }

    // Merge options from the config file if it exists
    String configFile = "test.conf";

    om.mergeFile(configFile);

    // Merge options from the command line
    om.mergeCommandLine(argc, (char**&)argv);

    // One parameter is required, the "Name" parameter so this should
    // not exception out.
    om.checkRequiredOptions();

    PEGASUS_TEST_ASSERT(om.isTrue("debug"));    
    {
        const Option* op = om.lookupOption("debug");
        PEGASUS_TEST_ASSERT(op != 0);
        PEGASUS_TEST_ASSERT(op->isResolved());
    }

    PEGASUS_TEST_ASSERT(om.valueEquals("delay", "3"));    
    {
        const Option* op = om.lookupOption("delay");
        PEGASUS_TEST_ASSERT(op != 0);
        PEGASUS_TEST_ASSERT(op->isResolved());
    }

    String passwordValue;
    om.lookupValue("Password", passwordValue);
    PEGASUS_TEST_ASSERT(passwordValue == "mypassword");    
    PEGASUS_TEST_ASSERT(om.valueEquals("Password", "mypassword"));    
    {
        const Option* op = om.lookupOption("Password");
        PEGASUS_TEST_ASSERT(op != 0);
        PEGASUS_TEST_ASSERT(op->isResolved());
    }

    // confirmed parameter that are not input are
    // marked not used and returns default value
    PEGASUS_TEST_ASSERT(om.valueEquals("optional", "blah"));
    {
        const Option* op = om.lookupOption("optional");
        PEGASUS_TEST_ASSERT(op != 0);
        PEGASUS_TEST_ASSERT(!op->isResolved());
    }
    
    {
        const Option* op = om.lookupOption("optional2");
        PEGASUS_TEST_ASSERT(op != 0);
        PEGASUS_TEST_ASSERT(!op->isResolved());
    }
    PEGASUS_TEST_ASSERT(om.valueEquals("optional2", "999"));
    
    {
        const Option* op = om.lookupOption("configfile1");
        PEGASUS_TEST_ASSERT(op != 0);
        PEGASUS_TEST_ASSERT(op->isResolved());
    }
    PEGASUS_TEST_ASSERT(om.valueEquals("configfile1", "noblah"));   
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
       test01();
       test02(argc, argv);
    }
    catch (Exception& e)
    {
        PEGASUS_STD(cout) << "Exception: " << e.getMessage()
            << PEGASUS_STD(endl);
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
