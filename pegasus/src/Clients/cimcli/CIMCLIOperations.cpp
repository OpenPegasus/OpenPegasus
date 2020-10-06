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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include "CIMCLIOperations.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

/******************************************************************************
**
**             Operation Table.  Defines all of the
**             Operations available to cimcli including:
**             Type Enum, String definition, Min number of arguments on
**             Input, shortcut name, Help Command internationalization text
**             and Short Usage statement
**
******************************************************************************/
OPERATION_TABLE_ENTRY OperationTable[] =
{
    {ID_EnumerateInstanceNames,  "enumerateInstanceNames", 2 ,"ni",
    "Clients.cimcli.CIMCLIClient.NI_COMMAND_HELP",
    "Enumerate instancenames of <classname>"},

    {ID_EnumerateAllInstanceNames,"enumallInstanceNames", 2 , "niall",
    "Clients.cimcli.CIMCLIClient.NIALL_COMMAND_HELP" ,
    "Enumerate all instance names and count of"
        " instances of [ <classname> ] in <namespace>."
        " Classname optional and default is to enumerate instances of"
        " entire namespace"},

    {ID_EnumerateInstances,      "enumerateInstances",2 ,   "ei",
    "Clients.cimcli.CIMCLIClient.EI_COMMAND_HELP" ,
    "Enumerate instances of <classname>"},

    {ID_EnumerateClassNames,     "enumerateClassNames",2 ,  "nc",
    "Clients.cimcli.CIMCLIClient.NC_COMMAND_HELP",
    "Enumerate Class Names of [ <classname> ]. Classname optional"},

    {ID_EnumerateClasses,        "enumerateClasses",2 ,     "ec",
    "Clients.cimcli.CIMCLIClient.EC_COMMAND_HELP",
    "Enumerate classes of [ <classname> ]. Classname optional"},

    {ID_GetClass,                "getClass",2 ,             "gc",
    "Clients.cimcli.CIMCLIClient.GC_COMMAND_HELP",
    "Get class of <classname>"},

    {ID_GetInstance,             "getInstance",   2 ,       "gi",
    "Clients.cimcli.CIMCLIClient.GI_COMMAND_HELP",
    "Get instance of <objectname> | <classname> (interactive) |"
    " <classname>  *<PropertyName=value> defining key property values."},

    {ID_CreateInstance,          "createInstance",2 ,       "ci",
    "Clients.cimcli.CIMCLIClient.CI_COMMAND_HELP",
                        "Create instance of a class. Input Format is:\n"
                        "   <className> [<propertyDef>]*\n"
                        "where propertyDef is:\n"
                        "  | <propName>=<value>      //property with value\n"
                        "  | <propName>=\"<value>\"    //string with space\n"
                        "                            //in string\n"
                        "  | <propName>=             //NULL value\n"
                        "  | <propName>!             //empty string value\n"
                        "  | <propName>=<arrayVal>   //Arrayinput\n"
                        "      where: arrayVal=<value>[,<value>]*\n"
                        "\n"
                        "  | <propName>{<classname> [<propertyDef>]* }\n"
                        "  | <propName>={<classname> [<propertyDef>]* }\n"
                        "     defines embedded instance or \n"
                        "              embedded objectPath  where:\n"
                        "       -classname - embedded class\n"
                        "       -propertyDef - embedded instance property"},

    {ID_TestInstance,             "testInstance",   2 ,      "ti",
    "Clients.cimcli.CIMCLIClient.TI_COMMAND_HELP",
                        "Test instance  of <objectname>  [<propertyDef>]* |"
                        " <classname>  [<propertyDef>]* for equal property"
                             " values."
                        "where propertyDef is:\n"
                        "  | <propName>=<value>      //property with value\n"
                        "  | <propName>=\"<value>\"    //string with space\n"
                        "                            //in string\n"
                        "  | <propName>=             //NULL value\n"
                        "  | <propName>!             //empty string value\n"
                        "  | <propName>=<arrayVal>   //Arrayinput\n"
                        "      where: arrayVal=<value>[,<value>]*\n"
                        "\n"
                        "  | <propName>{<classname> [<propertyDef>]* }\n"
                        "  | <propName>={<classname> [<propertyDef>]* }\n"
                        "     defines embedded instance or \n"
                        "              embedded objectPath  where:\n"
                        "       -classname - embedded class\n"
                        "       -propertyDef - embedded instance property"},

    {ID_DeleteInstance,          "deleteInstance",2 ,       "di",
    "Clients.cimcli.CIMCLIClient.DI_COMMAND_HELP",
    "Delete Instance of <objectname> | "
    "<classname> *<keyPropertyName=value> | "
    "interactive of <className>"},

    {ID_CreateClass   ,          "createClass",   2 ,       "cc",
    "Clients.cimcli.CIMCLIClient.CC_COMMAND_HELP",
    "Not supported"},

    {ID_ModifyInstance,          "modifyInstance",2 ,       "mi",
    "Clients.cimcli.CIMCLIClient.MI_COMMAND_HELP",
                        "Modify Instance of a class. Input formats are:\n"
                        "<objectname> [<propertyDef>]*\n"
                        "| <classname> [<propertyDef>]*\n"
                        "where propertyDef is:\n"
                        "  | <propName>=<value>      //property with value\n"
                        "  | <propName>=\"<value>\"    //string with space\n"
                        "                            //in string\n"
                        "  | <propName>=             //NULL value\n"
                        "  | <propName>!             //empty string value\n"
                        "  | <propName>=<arrayVal>   //Arrayinput\n"
                        "      where: arrayVal=<value>[,<value>]*\n"
                        "\n"
                        "  | <propName>{<classname> [<propertyDef>]* }\n"
                        "  | <propName>={<classname> [<propertyDef>]* }\n"
                        "     defines embedded instance or \n"
                        "              embedded objectPath  where:\n"
                        "       -classname - embedded class\n"
                        "       -propertyDef - embedded instance property"},

    {ID_DeleteClass,             "deleteClass",   2 ,       "dc",
    "Clients.cimcli.CIMCLIClient.DC_COMMAND_HELP",
    "Delete Class of <classname>"},

    {ID_GetProperty,             "getProperty",   2 ,       "gp",
    "Clients.cimcli.CIMCLIClient.GP_COMMAND_HELP",
    "get single property of <object name>"},

    {ID_SetProperty,             "setProperty",   2 ,       "sp",
    "Clients.cimcli.CIMCLIClient.SP_COMMAND_HELP",
    "set a single property of <object-name>"},

    {ID_GetQualifier,            "getQualifier",  2 ,       "gq",
    "Clients.cimcli.CIMCLIClient.GQ_COMMAND_HELP",
    "Get Qualifier of <qualifiername>"},

    {ID_SetQualifier,            "setQualifier",  2 ,       "sq",
    "Clients.cimcli.CIMCLIClient.SQ_COMMAND_HELP",
    "Not suported"},

    {ID_EnumerateQualifiers,     "enumerateQualifiers",2 ,  "eq",
    "Clients.cimcli.CIMCLIClient.EQ_COMMAND_HELP",
    "Enumerate all Qualifiers"},

    {ID_DeleteQualifier,         "deleteQualifier",  2 ,    "dq",
    "Clients.cimcli.CIMCLIClient.DQ_COMMAND_HELP",
    "Delete qualifer of <qualifiername>"},

    {ID_Associators,             "associators",   2 ,       "a" ,
    "Clients.cimcli.CIMCLIClient.A_COMMAND_HELP",
    "Enumerate Associators of <classname> | <instancename>."},

    {ID_AssociatorNames,         "associatorNames", 2 ,     "an",
    "Clients.cimcli.CIMCLIClient.AN_COMMAND_HELP",
    "Enumerate Associator Names of <classname> | <instancename>"},

    {ID_References,              "references",      2,      "r",
    "Clients.cimcli.CIMCLIClient.R_COMMAND_HELP",
    "Enumerate References of <classname> | <instancename>"},

    {ID_ReferenceNames,          "referenceNames",2 ,       "rn",
    "Clients.cimcli.CIMCLIClient.RN_COMMAND_HELP",
    "Enumerate Reference Names <classname> | <instancename>"},

    {ID_InvokeMethod,            "invokeMethod",  2 ,       "im",
    "Clients.cimcli.CIMCLIClient.IM_COMMAND_HELP",
                    "Invoke Method for <object> <method> {*<inputParam>} "
                    "where inputParms can be defined as:\n"
                    "  <paramName>=<value>\n"
                    "  <paramName>{<embeddedClassName> *[<paramName> }"},

    {ID_ExecQuery,               "execQuery",     2 ,       "xq",
    "Clients.cimcli.CIMCLIClient.XQ_COMMAND_HELP",
    "ExecQuery <query-expresssion> [<query-language>]"},

    {ID_EnumerateNamespaces,     "enumerateNamespaces",2 ,  "ns",
    "Clients.cimcli.CIMCLIClient.NS_COMMAND_HELP",
    "Enumerate all namespaces on the server."},

    {ID_StatisticsOn,            "StatisticsOn",2 ,   "son",
    "Clients.cimcli.CIMCLIClient.SON_COMMAND_HELP",
    "Turn on CIM Server Statistics Gathering"},

    {ID_StatisticsOff,           "StatisticsOff",2 ,  "soff",
    "Clients.cimcli.CIMCLIClient.SOFF_COMMAND_HELP",
    "Turn off CIM Server Statistics Gathering"},

    {ID_CountInstances,          "countInstances",2, "cci",
    "Clients.cimcli.CIMCLIClient.CCI_COMMAND_HELP",
        "Count all Instances in Namespace and sort by ClassName.\n"
        " Warning: Could request large number of instances from Server"},

    {ID_ClassTree,               "classTree",2, "ct",
    "Clients.cimcli.CIMCLIClient.CT_COMMAND_HELP",
        "Display graphic tree of classes in a namespace"},

    {ID_ShowOptions,             "showOptions",2 ,  "?",
    "Clients.cimcli.CIMCLIClient.?_COMMAND_HELP",
    "Show List of Commands"},

//KS_PULL_BEGIN
    {ID_PullEnumerateInstances,   "pullEnumerateInstances",2, "pei",
    "Clients.cimcli.CIMCLIClient.PEI_COMMAND_HELP",
        "Execute EnumerateInstances using the Pull Operations"},

    {ID_PullEnumerateInstancePaths,
        "pullEunumerateInstancePaths",2, "pni",
    "Clients.cimcli.CIMCLIClient.PEIP_COMMAND_HELP",
        "Execute EnumerateInstances Paths using the Pull Operations"},

    {ID_PullReferenceInstances,   "pullReferenceInstances",2, "pr",
    "Clients.cimcli.CIMCLIClient.PRI_COMMAND_HELP",
        "Execute Open and Pull References"},

    {ID_PullReferenceInstancePaths,
        "pullReferencesInstancePaths",2, "prn",
    "Clients.cimcli.CIMCLIClient.PRIP_COMMAND_HELP",
        "Execute Open,Pull ReferenceInstancePaths using the Pull Operations"},

    {ID_PullAssociatorInstances,   "pullAssociatorInstances",2, "pa",
    "Clients.cimcli.CIMCLIClient.PAK_COMMAND_HELP",
        "Execute Open and Pull Associations"},

    {ID_PullAssociatorInstancePaths,
        "pullAssociatorInstancePaths",2, "pan",
    "Clients.cimcli.CIMCLIClient.PAIP_COMMAND_HELP",
        "Execute Open,Pull AssociatorInstancePaths using the Pull Operations"},

    {ID_PullQueryInstances,
        "pullExecQuery",2, "pxq",
    "Clients.cimcli.CIMCLIClient.PXQIP_COMMAND_HELP",
        "Execute query operation using pull Operations\n"
        "pzq <querylanguage> <query> <pull parameters>"},
//KS_PULL_END
};

const Uint32 NUM_OPERATIONS =
    sizeof(OperationTable) / sizeof(OperationTable[0]);

/*
    The following table defines the Example strings and supported options
    strings for all cimcli commands defined in the operations table
    along with the strings that define the corresponding text bundle strings.
    NOTE: This table MUST BE ordered the same as the CommandTable since the
    primary user assume that the index to a command is the same as the
    index into this table.
*/

OperationExampleEntry OperationExamples[] = {
    {"Clients.cimcli.CIMCLIClient.NI_COMMAND_EXAMPLE",
    "cimcli ni -n test/TestProvider TST_Person\n"
        "    -- Enumerate Instance Names of class TST_Person\n",
    "Clients.cimcli.CIMCLIClient.NI_COMMAND_OPTIONS",
    "    -n\n"},

    {"Clients.cimcli.CIMCLIClient.NIALL_COMMAND_EXAMPLE",
    "cimcli niall\n"
        "    -- Enumerate Instance Names and count instances of all classes\n"
        "       in the default namespace\n"
    "cimcli niall CIM_ComputerSystem -n root/interop\n"
        "    -- Enumerate Instance Names and count instances of all classes\n"
        "       in namespace root/interop starting at CIM_ComputerSystem",
    "Clients.cimcli.CIMCLIClient.NIALL_COMMAND_OPTIONS",
    "    -n --sum (count only, no instanceNames output) classname\n"},

    {"Clients.cimcli.CIMCLIClient.EI_COMMAND_EXAMPLE",
    "cimcli ei PG_ComputerSystem   -- Enumerate Instances of class\n",
    "Clients.cimcli.CIMCLIClient.EI_COMMAND_OPTIONS",
    "    -n, -di, -lo, -iq, -pl -ic\n"},


    {"Clients.cimcli.CIMCLIClient.NC_COMMAND_EXAMPLE",
    "cimcli nc -- Enumerate class names from root/cimv2.\n",
    "Clients.cimcli.CIMCLIClient.NC_COMMAND_OPTIONS",
    "    -n, -di\n"},


    {"Clients.cimcli.CIMCLIClient.EC_COMMAND_EXAMPLE",
    "cimcli ec -n root/cimv2\n"
        "    -- Enumerate classes from namespace root/cimv2.\n",
    "Clients.cimcli.CIMCLIClient.EC_COMMAND_OPTIONS",
    "    -n, -di, -lo, -niq -ic\n"},


    {"Clients.cimcli.CIMCLIClient.GC_COMMAND_EXAMPLE",
    "cimcli gc CIM_door -u guest -p guest\n"
        "    -- Get class user = guest and password = guest.\n",
    "Clients.cimcli.CIMCLIClient.GC_COMMAND_OPTIONS",
    "    -n, -lo, -niq, -pl -ic\n"},


    {"Clients.cimcli.CIMCLIClient.GI_COMMAND_EXAMPLE",
    "cimcli gi -n test/TestProvider TST_Person\n"
        "    -- Get Instance of class\n",
    "Clients.cimcli.CIMCLIClient.GI_COMMAND_OPTIONS",
    "    -n, -lo, -iq, -pl -ic\n"},


    {"Clients.cimcli.CIMCLIClient.CI_COMMAND_EXAMPLE",
    "cimcli ci -n test/TestProvider TST_Person Name=Mike SSN=333\n"
        "    -- Create Instance of  class TST_Person with properties:\n"
        "       Name = Mike\n"
        "       SSN = 333\n\n"
    "cimcli ci -n test/TestProvider C1 id=3 name=fred \\\n"
        "    embed{C2 p1=3 p2=\"ab cd\" }\n\n"
        "    -- Create instance of class C1 with properties:\n"
        "       id with value 3\n"
        "       name with string value \"fred\"\n"
        "       embed embedded instance property with instance of class\n",
        "        C2 with properties:\n"
        "           p1 embedded instance property with value 3\n"
        "           p2 embedded instance string property with value \"ab cd\""
    "cimcli ci -n test/TestProviderAssoc assoc1\\\n"
        "    child={{C2 id=3 } parent={C2.id=4 }\n\n"
        "    -- Create instance of class assoc1with properties:\n"
        "       child - reference to class C2 with key property = 3\n"
        "       parent ref to class C2 with key property id = 4\n"

    "Clients.cimcli.CIMCLIClient.CI_COMMAND_OPTIONS",
    "    -n\n"},


    {"Clients.cimcli.CIMCLIClient.TI_COMMAND_EXAMPLE",
    "cimcli ti -n test/TestProvider TST_Person Name=Mike SSN=333\n"
        "    -- Test Instance of  class for equality of Name and SSN\n"
        "       properties. Returns error code if instance in server\n"
        "       does not have same properties and values as the\n"
        "       instance built from input\n",
    "Clients.cimcli.CIMCLIClient.TI_COMMAND_OPTIONS",
    "    -n -nlo -iq -pl\n"},


    {"Clients.cimcli.CIMCLIClient.DI_COMMAND_EXAMPLE",
    "cimcli di -n test/TestProvider TST_Person\n"
        "    -- Delete Instance of class TST_Person interactively\n"
        "or\n"
    " cimcli di -n test/TestProvider TST_Person name=Mike\n"
        "    -- Delete Instance of class TEST_Persion with key property\n"
        "       name equal Mike using instance builder format to\n"
        "       define object path\n"
        "or\n"
    " cimcli di -n test/TestProvider TST_Person.name=\\\\\"Mike\\\\\"\n"
        "    -- Delete Instance of class TST_Persion with object path\n"
        "       TST_Person.name=\"Mike\" using object path input format\n",
    "Clients.cimcli.CIMCLIClient.DI_COMMAND_OPTIONS",
    "    -n\n"},

    // Create Class Not Supported
    {"Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
    "Operation Not supported..\n",
    "Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
    "\n"},

    // Modify Instance
    {"Clients.cimcli.CIMCLIClient.MI_COMMAND_EXAMPLE",
    "cimcli mi -n test/TestProvider TST_Person.Id=\\\\\"Mike\\\\\" SSN=444\n"
    "    -- Modifies the Instance if it exists using rules of DMTF \n"
    "       modifyInstance operation.\n",
    "Clients.cimcli.CIMCLIClient.MI_COMMAND_OPTIONS",
    "-n -pl(define properties to be modified) -i (interactive)\n "},


    {"Clients.cimcli.CIMCLIClient.DC_COMMAND_EXAMPLE",
    "cimcli dc -n test/TestProvider TST_Person\n"
        "    -- Deletes the Class when there are no instances and\n"
        "       sub-classes for this class\n",
    "Clients.cimcli.CIMCLIClient.DC_COMMAND_OPTIONS",
    "    -n\n"},


    // getProperty
    {"Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
        "cimcli gp -n test/TestProvider TST_Person.Id=\\\\\"Mike\\\\\" SSN \n"
            "    -- Get the property named SSN from the instance\n"
            "       defined by TST_Person.Id=\"Mike\"",
    "Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
    "\n"},


    // setProperty
    {"Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
        "cimcli sp -n test/TestProvider TST_Person.Id=\\\\\"Mike\\\\\""
        " SSN=333 \n"
            "    -- Set the property named SSS in the instance\n"
            "       defined by TST_Person.Id=\"Mike\")",
    "Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
    "\n"},


    {"Clients.cimcli.CIMCLIClient.GQ_COMMAND_EXAMPLE",
    "cimcli gq Association\n"
        "    -- Get the qualifier named Association in mof output\n"
        "       in the default namespace (normally root/cimv2)",
    "Clients.cimcli.CIMCLIClient.GQ_COMMAND_OPTIONS",
    "    -n\n"},

    // setQualifier
    {"Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
    "Operation Not supported..\n",
    "Clients.cimcli.CIMCLIClient.OPERATION_NOT_SUPPORTED",
    "\n"},


    {"Clients.cimcli.CIMCLIClient.EQ_COMMAND_EXAMPLE",
    "cimcli eq -n test/TestProvider\n"
        "    -- Enumerate Qualifiers of namespace test/TestProvider\n",
    "Clients.cimcli.CIMCLIClient.EQ_COMMAND_OPTIONS",
    "    -n\n"},

    {"Clients.cimcli.CIMCLIClient.DQ_COMMAND_EXAMPLE",
    "cimcli dq -n test/TestProvider ASSOCIATION\n"
        "    -- Delete Qualifier named ASSOCIATION in namespace\n"
        "       test/TestProvider\n",
    "Clients.cimcli.CIMCLIClient.DQ_COMMAND_OPTIONS",
    "    -n, qualifierName\n"},


    {"Clients.cimcli.CIMCLIClient.A_COMMAND_EXAMPLE",
    "cimcli a TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider"
        "  -ac TST_Lineage\n -rc TST_Person\n"
        "    -- get associations for instances of association class\n"
        "       TST_Lineage and result class TST_Person\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
        " cimcli a TST_Person -n test/TestProvider -ac TST_Lineage\n"
        "    -- get associated classes for all instances of classes\n"
        "       associated with class TST_person and with\n"
        "       association class TST_Lineage\n"
        "or\n"
        " cimcli a TST_Person -n test/TestProvider -ac TST_Lineage -i\n"
        "    -- get associated instances by selecting interactively for\n"
        "       classes  associated with class TST_person and with\n"
        "       association class TST_Lineage\n",
    "Clients.cimcli.CIMCLIClient.A_COMMAND_OPTIONS",
    "    -n, -ac, -rc, -r, -rr, -iq, -pl, -ic, -i\n"},

    {"Clients.cimcli.CIMCLIClient.AN_COMMAND_EXAMPLE",
    "cimcli an TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider "
        "-ac TST_Lineage\n"
        "or\n"
        " cimcli an TST_Person -n test/TestProvider -ac TST_Lineage\n",
    "Clients.cimcli.CIMCLIClient.AN_COMMAND_OPTIONS",
    "    -n, -ac, -rc, -r, -rr, -i\n"},

    {"Clients.cimcli.CIMCLIClient.R_COMMAND_EXAMPLE",
    "cimcli r TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider"
        " -rc TST_Lineage\n"
        "    -- get references for instances association class TST_Lineage\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
    " cimcli r TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider\n"
        "    -- get references for all instances of classes\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
        " cimcli r TST_Person -n test/TestProvider -rc TST_Lineage\n"
        "    -- get reference classes for TST_Lineage association class and\n"
        "       subclasses in namespace test/TestProvider\n",
    "Clients.cimcli.CIMCLIClient.R_COMMAND_OPTIONS",
    "    -n, -rc, -r, -iq, -pl, -i -ic\n"},

    {"Clients.cimcli.CIMCLIClient.RN_COMMAND_EXAMPLE",
    "cimcli rn TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider "
        "-rc TST_Lineage\n"
        "    -- get referenceNames for instances association class\n"
        "       TST_Lineage associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
    " cimcli rn TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider\n"
        "    -- get referenceNames for all instances of classes\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
        " cimcli rn TST_Person -n test/TestProvider -rc TST_Lineage\n"
        "    -- get referenceNames for Class TST_Lineage in namespace\n"
        "       test/TestProvider\n",
    "Clients.cimcli.CIMCLIClient.RN_COMMAND_OPTIONS",
    "    -n, -rc, -r, -i\n"},

    {"Clients.cimcli.CIMCLIClient.IM_COMMAND_EXAMPLE",
    "cimcli im Sample_MethodProviderClass.Name=\\\\\"mooo\\\\\" SayHello\n"
        "    -n root/SampleProvider -ip p1=fred\n",
    "Clients.cimcli.CIMCLIClient.IM_COMMAND_OPTIONS",
        "    -n, -ip\n"},

    {"Clients.cimcli.CIMCLIClient.XQ_COMMAND_EXAMPLE",
    "cimcli xq \"select * from CIM_ComputerSystem\"\n"
        "    -- This command will work when the code is compiled with\n"
        "       ExecQuery enabled\n",
    "Clients.cimcli.CIMCLIClient.XQ_COMMAND_OPTIONS",
    "    -n, queryLanguage, query\n"},

    {"Clients.cimcli.CIMCLIClient.NS_COMMAND_EXAMPLE",
    "cimcli ns  -- Enumerate all Namespaces in repository\n",
    "Clients.cimcli.CIMCLIClient.NO_OPTIONS_REQUIRED",
    " No options Required\n"},

    {"Clients.cimcli.CIMCLIClient.SON_COMMAND_EXAMPLE",
    "cimcli son  -- Switch On's the statistical information on CIMServer\n",
    "Clients.cimcli.CIMCLIClient.NO_OPTIONS_REQUIRED",
    " No options Required\n"},

    {"Clients.cimcli.CIMCLIClient.SOFF_COMMAND_EXAMPLE",
    "cimcli soff  -- Switch OFF's the statistical information on CIMServer\n",
    "Clients.cimcli.CIMCLIClient.NO_OPTIONS_REQUIRED",
    " No options Required\n"},

    {"Clients.cimcli.CIMCLIClient.CCI_COMMAND_EXAMPLE",
    "cimcli cci  -- Display count of instances in namespace\n",
    "Clients.cimcli.CIMCLIClient.NO_OPTIONS_REQUIRED",
    "    No options Required\n"
    "    -count and output options( -o and -x) do not apply"},

    {"Clients.cimcli.CIMCLIClient.CT_COMMAND_EXAMPLE",
    "cimcli ct CIM_ManagedElement -- Displays tree of class hiearchy\n"
    "     Starting at CIM_ManagedElement.\n",
    "Clients.cimcli.CIMCLIClient.CT_COMMAND_OPTIONS",
    "    -nlo displays tree of superclasses.\n"
    "    -di displays association information\n"
    "    -count and output options( -o and -x) do not apply"},

    {"Clients.cimcli.CIMCLIClient.?_COMMAND_EXAMPLE",
    "cimcli ?  -- Displays help command\n",
    "Clients.cimcli.CIMCLIClient.NO_OPTIONS_REQUIRED",
    "    No options Required"},

//KS_PULL_BEGIN
    {"Clients.cimcli.CIMCLIClient.PEI_COMMAND_EXAMPLE",
    "cimcli pei PG_ComputerSystem   -- Pull Enumerate Instances of class\n",
    "Clients.cimcli.CIMCLIClient.PEI_COMMAND_OPTIONS",
    "    -n, -di, -lo, -iq, -pl\n"},

    {"Clients.cimcli.CIMCLIClient.PNI_COMMAND_EXAMPLE",
    "cimcli pni -n test/TestProvider TST_Person\n"
        "    -- Pull Enumerate Instance Names of class TST_Person\n",
    "Clients.cimcli.CIMCLIClient.PNI_COMMAND_OPTIONS",
    "    -n\n"},

    {"Clients.cimcli.CIMCLIClient.PR_COMMAND_EXAMPLE",
    "cimcli pr TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider"
        " -rc TST_Lineage\n"
        "    -- get references for instances association class TST_Lineage\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
    " cimcli pr TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider\n"
        "    -- get references for all instances of classes\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
        " cimcli pr TST_Person -n test/TestProvider -rc TST_Lineage\n"
        "    -- get reference classes for TST_Lineage association class and\n"
        "       subclasses in namespace test/TestProvider\n",
    "Clients.cimcli.CIMCLIClient.PR_COMMAND_OPTIONS",
    "    -n, -rc, -r, -iq, -pl, -i\n"},

    {"Clients.cimcli.CIMCLIClient.PRN_COMMAND_EXAMPLE",
    "cimcli prn TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider "
        "-rc TST_Lineage\n"
        "    -- get referenceNames for instances association class\n"
        "       TST_Lineage associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
    " cimcli prn TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider\n"
        "    -- get referenceNames for all instances of classes\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
        " cimcli prn TST_Person -n test/TestProvider -rc TST_Lineage\n"
        "    -- get referenceNames for Class TST_Lineage in namespace\n"
        "       test/TestProvider\n",
    "Clients.cimcli.CIMCLIClient.PRN_COMMAND_OPTIONS",
    "    -n, -rc, -r, -i\n"},

    {"Clients.cimcli.CIMCLIClient.PA_COMMAND_EXAMPLE",
    "cimcli pa TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider"
        "  -ac TST_Lineage\n -rc TST_Person\n"
        "    -- get associations for instances of association class\n"
        "       TST_Lineage and result class TST_Person\n"
        "       associated with instance of TST_person with key\n"
        "       \"Mike\" in namespace test/TestProvider\n"
        "or\n"
        " cimcli pa TST_Person -n test/TestProvider -ac TST_Lineage\n"
        "    -- get associated classes for all instances of classes\n"
        "       associated with class TST_person and with\n"
        "       association class TST_Lineage\n"
        "or\n"
        " cimcli pa TST_Person -n test/TestProvider -ac TST_Lineage -i\n"
        "    -- get associated instances by selecting interactively for\n"
        "       classes  associated with class TST_person and with\n"
        "       association class TST_Lineage\n",
    "Clients.cimcli.CIMCLIClient.PA_COMMAND_OPTIONS",
    "    -n, -ac, -rc, -r, -rr, -iq, -pl, includeClassOrigin, -i\n"},

    {"Clients.cimcli.CIMCLIClient.PAN_COMMAND_EXAMPLE",
    "cimcli an TST_Person.name=\\\\\"Mike\\\\\" -n test/TestProvider "
        "-ac TST_Lineage\n"
        "or\n"
        " cimcli an TST_Person -n test/TestProvider -ac TST_Lineage\n",
    "Clients.cimcli.CIMCLIClient.PAN_COMMAND_OPTIONS",
    "    -n, -ac, -rc, -r, -rr, -i\n"},
    {"Clients.cimcli.CIMCLIClient.PXQ_COMMAND_EXAMPLE",
    "cimcli pxq \"select * from CIM_ComputerSystem\"\n"
        "    -- This command will work when the code is compiled with\n"
        "       ExecQuery enabled\n",
    "Clients.cimcli.CIMCLIClient.PXQ_COMMAND_OPTIONS",
    "    -n, queryLanguage, query, pull parameters\n"},
//KS_PULL_END
};


/*
   Operations Class implements the operations on the OperationTable and
   Examples table above.
*/
Operations::Operations()
{
    _index = 0;
}

Boolean Operations::find(const String& opString)
{
    for( _index = 0 ; _index < NUM_OPERATIONS; _index++ )
    {
        if ((String::equalNoCase(opString,
                OperationTable[_index].OperationName))
                ||
                (opString == OperationTable[_index].ShortCut))
        {
            return true;
        }
    }
    return false;
}

OperationTableEntry Operations::get()
{
    return OperationTable[_index];
}

Uint32 Operations::getIndex()
{
    return _index;
}

Boolean Operations::more()
{
    return (_index < NUM_OPERATIONS)? true : false;
}

// Note that this method has NO range check so MUST BE used
// with more()
OperationTableEntry Operations::next()
{
    return OperationTable[_index++];
}

OperationExampleEntry Operations::getExampleEntry()
{
    return OperationExamples[_index];
}


PEGASUS_NAMESPACE_END
// END_OF_FILE
