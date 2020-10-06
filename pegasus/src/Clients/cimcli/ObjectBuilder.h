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
#ifndef _CLI_ObjectBuilder_h
#define _CLI_ObjectBuilder_h

#include <Pegasus/Common/Config.h>
#include <Clients/cimcli/Linkage.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>

/*
    Classes and functions to build instances and parameters from
    input parameter (<name><tokenType><value>) string input.
    This class supports building instances for CIM operations like
    createInstance and modifyInstance and CIMParameters for invokeMethod
    from the command line.
    The name entity defines a cim named element, normally a property or
    parameter name.
    The tokenType separates the name from the value and defines the processing
    to be performed
    This function supports the following types:
        "=" defines a string value for the name
        "{" defines an embedded definition such that the parameters between
            the parameter with } and an input parameter containing "}" only is
            the set of parameters that will be used to construct an embedded
            instance.
    The constructor sets up the parameter list,separates out
    names and values and get any required metadata (i.e. Classes)

    The build methods use this to build the objects.
*/

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// Enumeration of the valuePair parsing return types (the token types)
enum TokenType { UNKNOWN, ILLEGAL, VALUE, NO_VALUE, EXCLAM, NAME_ONLY,
                 EMBED, END_EMBED, EMBED_NEXT_ARRAY_ITEM };

class ObjectBuilder;

// Internal object builder class references in private constructor
class iterateArray;

// The tokenItem class is a container for tokens that define each
// parameter/property to be build and is the output of the analysis of
// the input parameter list. This is a class private to the ObjectBuilder
// files except as a property in ObjectBuilder
// Define a single token after syntax analysis.  A token is defined as the
// name, tokenType, and value defined by a single input string or an embedded
// instance definition.  The input parameter is the
// commandline input for a single property or parameter definition.

class PEGASUS_CLI_LINKAGE tokenItem
{
public:
    String tokenInput;            // Original string input for this token
    CIMName tokenName;            // Token name (property or param name)
    String tokenValue;            // Value Component string
    TokenType tokenType;          // Type determined by separator
    Boolean duplicate;            // if true, tokenName is not unique in array
                                  // Used to determine multiple definitions.

    // Instances produced by analyzing embedded token type. Saved in token
    // to be inserted in parent instance/property/parameter.
    Array<CIMInstance> _instances;
    CIMClass _class;

    // Construct a single token item with
    // @param inputParam - The input parameter that was the source of this
    // token.  This is for diagnostics only.
    // @param name String name of the token
    // @param value String with the value definition
    // @param type TokenType enumerating the type for this token

    tokenItem (const String& inputParam, String& name, String& value,
        TokenType& type);

    // Diagnostics used in debugging these functions.
    // print a single tokenItem
    void print();

    // create a String output for a single tokenItem. Diagnostic only
    String toString();

    // return the string for each tokenType. Diagnostic only
    static String typeToString(TokenType t);

    // required by Array template
    tokenItem() {}
private:

};

// Define the class that builds instances and parameters from command line
// input.
class PEGASUS_CLI_LINKAGE ObjectBuilder
{
public:
    /** create the array of tokens for the defined parameters and
        set up the object so that an instance, parameters, etc. can
        be built.
        @param inputs Name/value pairs representing the input
        properties from the command line
        @param client CIMClient so that function can get class info.
        @param className the CIMClass for which we are creating something
        @param cimPropertyList - If specified used to limit the list
        properties retrieved with getClas on the className
        parameter.
        @param verbose true if output of detailed information
    */
    ObjectBuilder(Array<String>& inputPairs,
        CIMClient& client,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMPropertyList& cimPropertyList,
        Boolean verbose);

    ~ObjectBuilder();

    /** return the list of properrties defined in this instance
        input.*/
    Array<CIMName> getPropertyList();

    /** Create an instance with the properties defined in the
        constructor.
        @param includeQualifiers
        @param includeClassOrigin
        @param propertyList
        @return CIMInstance built from the class and input
                parameters.
    */
    CIMInstance buildInstance(Boolean includeQualifiers,
                              Boolean includeClassOrigin,
                              const CIMPropertyList& propertyList);

    /** Create a CIMvalue for a specify property
     *  Create a CIMValue from the provided input parameters
     *  @param - Name of the property. Used to find property in
     *         class
     *  @param value String defining value that will be parsed and
     *               set in returned CIMValue
     *  @param returned CIMInstance if successful.
    */
    CIMValue buildPropertyValue(const CIMName& name,
        const String& value);

    /**
       Create an array of  CIMParamValues for the defined parameter
       name from input parameters defined in the constructor
       @param method name for which parameters will be built.
       */
    Array<CIMParamValue> buildMethodParameters(CIMName& name);

    // Return the class definition that was acquired during the
    // construction.  This is provided so that other functions do
    // not have to repeat getting the class.
    const CIMClass getTargetClass();

    /** Build a CIMObjectPath from the information provided with the
        constructor.
        @return CIMObjectPath based on the parameters supplied with
                the constructor if build of the path was successful
     */
    CIMObjectPath buildCIMObjectPath();

    // diagnostic to print out information about this ObjectBuilder
    void print();


private:
    // empty constructor not used
    ObjectBuilder();

    /**
     * Build an embedded instance from a subset of the input
     * parameters. Only used from within ObjectBuilder
     *
     * @param ia class iterateArray.
     * @param client
     * @param nameSpace
     * @param ti tokenItem containing className ClassName for the
     *           embedded class and the feature(property/parameter)
     *           name
     * @param cimPropertyList - TBD
     * @param verbose - Flag to define if verbose output
     * @param rtnInstance - Instance to be returned
     * @param recurseLevel - Diagnostic indicating level of
     *                     objectBuilder recursion(i.e  embedded
     *                     objects)
     */
    ObjectBuilder(iterateArray& ia,
        CIMClient& client,
        const CIMNamespaceName& nameSpace,
        tokenItem& ti,
        const CIMPropertyList& cimPropertyList,
        Boolean verbose,
        CIMInstance& rtnInstance,
        Uint32& recurseLevel);

    void scanInputList(CIMClient& client,
        const CIMNamespaceName& nameSpace,
        const CIMPropertyList& cimPropertyList,
        iterateArray& ia, Uint32& recurseLevel,
        const CIMName& featureName);

    // Diagnostic to print all tokens created
    void printTokens(String message = String());

    // Append a new tokenItem to the array for this instance. if the
    // name already exists, mark it as duplicate. NOTE: duplicate is
    // name only test.
    void appendToken(tokenItem ti);

    // Internal data for object builder
    // Array of tokens created
    Array<tokenItem> _tokens;     // Array of tokens created by constructor
    CIMInstance _instance;  // Instance created when embedded inst encountered
    CIMClass _thisClass;         // Class accessed to build object
    CIMName _className;          // ClassName supplied with call
    Boolean _verbose;            // verbose output
};

PEGASUS_NAMESPACE_END
#endif
