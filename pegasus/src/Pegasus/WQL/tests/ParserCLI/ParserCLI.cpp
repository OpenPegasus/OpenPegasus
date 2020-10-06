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
#include <iostream>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/WQL/WQLSimplePropertySource.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean containsProperty(const CIMName& name, const Array<CIMName>& props)
{
  for (Uint32 i = 0; i < props.size(); i++)
  {
    if (props[i] == name)
    {
      return true;
    }
  }

  return false;
}


int main(int argc, char** argv)
{
    Boolean verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;

    //
    // Check arguments:
    //

    if (argc < 2)
    {
    cerr << "Usage: " << argv[0] << " <returnOptions> wql-text..." << endl;
    cerr << "return Options are keywords parseError,"
        " noEvaluate or evalulateError" << endl;
    cerr << "They tell the call to reverse the error"
        " on the parse or evalutate tests" << endl;
    return(1);
    }
    Boolean evaluateErrorTest = false;
    Boolean parseErrorTest = false;
    Boolean ignoreEvaluation = false;

    if (!strcmp(argv[1],"evaluateError"))
    {
        if (verbose)
            {cout << "Negative  evaluate test" << endl;}
        evaluateErrorTest = true;
    }

    if (!strcmp(argv[1], "parseError"))
    {
        if (verbose)
            {cout << "Negative  parse test" << endl;}
        parseErrorTest = true;
    }


    if (!strcmp(argv[1], "noEvaluate"))
    {
        if (verbose)
            {cout << "Negative  parse test" << endl;}
        ignoreEvaluation = true;
    }
    //
    // Append all arguments together to from a single string:
    //

    Buffer text;
    int startArray = 1;
    if (parseErrorTest || evaluateErrorTest || ignoreEvaluation)
        startArray++;

    for (int i = startArray; i < argc; i++)
    {
        text.append(argv[i], strlen(argv[i]));

        if (i + 1 !=  argc)
            text.append("\n", 1);
    }

    if (verbose)
        cout << text.getData() << endl;

    //
    // Parse the text:
    //

    WQLSelectStatement statement;

    try
    {
        WQLParser::parse(text, statement);
    }
    catch (Exception& e)
    {
        if (parseErrorTest)
        {
            cout << argv[0] <<" +++++ passed all tests" << endl;
            return 0;
        }
        else
        {
            cerr << "Parse Exception: " << e.getMessage() << endl;
            return(1);
        }
    }

    if (verbose)
    {
        statement.print();
    }

    //
    // Create a property source:
    //

    WQLSimplePropertySource source;
    PEGASUS_TEST_ASSERT(source.addValue("v", WQLOperand()));
    PEGASUS_TEST_ASSERT(source.addValue("w",
        WQLOperand(true, WQL_BOOLEAN_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("x", WQLOperand(10,
        WQL_INTEGER_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("y",
        WQLOperand(10.10, WQL_DOUBLE_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("z",
        WQLOperand("Ten", WQL_STRING_VALUE_TAG)));

    PEGASUS_TEST_ASSERT(source.addValue("SourceInstance",
        WQLOperand("SourceInstance", WQL_STRING_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("PreviousInstance",
        WQLOperand("PreviousInstance", WQL_STRING_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("OperationalStatus",
        WQLOperand("OperationalStatus", WQL_STRING_VALUE_TAG)));

    //
    // Create an instance
    //

    CIMInstance inst("ClassName");
    inst.addProperty(CIMProperty(CIMName("v"), CIMValue()));
    inst.addProperty(CIMProperty(CIMName("w"), CIMValue(true)));
    inst.addProperty(CIMProperty(CIMName("x"), CIMValue(10)));
    inst.addProperty(CIMProperty(CIMName("y"), CIMValue(10.10)));
    inst.addProperty(CIMProperty(CIMName("z"), CIMValue(String("Ten"))));
    inst.addProperty(CIMProperty(CIMName("SourceInstance"),
        CIMValue(String("SourceInstance"))));
    inst.addProperty(CIMProperty(CIMName("PreviousInstance"),
        CIMValue(String("PreviousInstance"))));
    inst.addProperty(CIMProperty(CIMName("OperationalStatus"),
        CIMValue(String("OperationalStatus"))));

    //
    // Create an empty instance
    //
    CIMInstance emptyInst("ClassName");

    //
    // Create an empty object
    //
    CIMObject emptyObj = emptyInst;

    //
    // Apply projection (both overloads) with populated instance or object.
    // Tests removal of extra properties.
    //

    try
    {
        Array<CIMName> selectList =
            statement.getSelectPropertyList().getPropertyNameArray();

        CIMInstance projInst = inst.clone();
        statement.applyProjection(projInst, false); // don't allow missing props
        if (statement.getAllProperties())
        {
            PEGASUS_TEST_ASSERT(projInst.getPropertyCount()
                    == inst.getPropertyCount());
        }
        else
        {
            PEGASUS_TEST_ASSERT(projInst.getPropertyCount()
                    == statement.getSelectPropertyNameCount());
            for (Uint32 i = 0; i < projInst.getPropertyCount(); i++)
            {
                PEGASUS_TEST_ASSERT(
                    containsProperty(projInst.getProperty(i).getName(),
                        selectList));
            }
        }

        CIMObject projObj = inst.clone();
        statement.applyProjection(projObj, false);  // don't allow missing props
        if (statement.getAllProperties())
        {
            PEGASUS_TEST_ASSERT(projObj.getPropertyCount()
                    == inst.getPropertyCount());
        }
        else
        {
            PEGASUS_TEST_ASSERT(projObj.getPropertyCount()
                    == statement.getSelectPropertyNameCount());
            for (Uint32 i = 0; i < projObj.getPropertyCount(); i++)
            {
                PEGASUS_TEST_ASSERT(
                        containsProperty(projObj.getProperty(i).getName(),
                            selectList));
            }
        }
    }
    catch (Exception& e)
    {
        cerr << "ApplyProjection Exception: " << e.getMessage() << endl;
        return(1);
    }

    //
    // Apply projection (both overloads) with empty instance or object.
    // Tests checking for missing properties (allowMissing parameter).
    //
    if (!statement.getAllProperties())
    {
        Boolean gotExc = false;
        try
        {
            statement.applyProjection(emptyInst, false);//disallow missing props
            PEGASUS_TEST_ASSERT(false);
        }
        catch (QueryRuntimePropertyException&)
        {
            gotExc = true;  // this is expected due to missing props
        }
        catch (Exception& e)
        {
            cerr << "ApplyProjection Exception with empty instance,"
                " do not allow missing props: "
                 << e.getMessage() << endl;
            return(1);
        }
        PEGASUS_TEST_ASSERT(gotExc);

        try
        {
            statement.applyProjection(emptyInst, true);  // allow missing props
        }
        catch (Exception& e)
        {
            cerr << "ApplyProjection Exception with empty instance,"
                    " allow missing props: "
                 << e.getMessage() << endl;
            return(1);
        }

        gotExc = false;
        try
        {
            statement.applyProjection(emptyObj, false);// disallow missing props
            PEGASUS_TEST_ASSERT(false);
        }
        catch (QueryRuntimePropertyException&)
        {
            gotExc = true;  // this is expected due to missing props
        }
        catch (Exception& e)
        {
            cerr << "ApplyProjection Exception with empty object,"
                " do not allow missing props: "
                 << e.getMessage() << endl;
            return(1);
        }
        PEGASUS_TEST_ASSERT(gotExc);

        try
        {
            statement.applyProjection(emptyObj, true);  // allow missing props
        }
        catch (Exception& e)
        {
            cerr << "ApplyProjection Exception with empty object,"
                    " allow missing props: "
                 << e.getMessage() << endl;
            return(1);
        }
    }

    //
    // Evaluate the where clause:
    //
    if (!ignoreEvaluation)
    {
        try
        {
            Boolean returnValue;
            returnValue = (statement.evaluateWhereClause(&source) == true);
            if (evaluateErrorTest)
            {
                PEGASUS_TEST_ASSERT(!returnValue);
            }
            else
            {
                PEGASUS_TEST_ASSERT(returnValue);
            }
        }
        catch (Exception& e)
        {
            if (evaluateErrorTest)
            {
                cerr << "EvaluateWhereClause Exception: "
                     << e.getMessage() << endl;
                exit(1);
            }
            else
            {
                cout << argv[0] <<" +++++ passed all tests" << endl;
                return 0;
            }
        }
    }

    cout << argv[0] <<" +++++ passed all tests" << endl;
    return 0;
}
