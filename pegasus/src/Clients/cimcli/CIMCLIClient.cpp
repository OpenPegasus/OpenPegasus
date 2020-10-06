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

/*
    This file defines the cicml operations(action functions).
    Each function is called from a specific cimcli input parameter opcode.
    The parameters for each operation are defined in the.
    options structure.
*/
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StringConversion.h>

#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/General/MofWriter.h>
#include <Pegasus/Common/Print.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Pegasus_inl.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/General/Stopwatch.h>

#include "CIMCLIClient.h"

#include "ObjectBuilder.h"
#include "CIMCLIOutput.h"
#include "CIMCLIHelp.h"
#include "CIMCLIOptions.h"
#include "CIMCLICommon.h"
#include "CIMCLIOperations.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

const String DEFAULT_NAMESPACE = "root/cimv2";


/************************************************************************
*
*   Start and Stop timer to provide execution times for
*   operation action functions.
*
*************************************************************************/

/* Common function for all command action functions to start the
   elapsed timer that will time command execution
*/
void _startCommandTimer(Options& opts)
{
    if (opts.time)
    {
        opts.elapsedTime.reset();
        opts.elapsedTime.start();
    }
}

/* Common function for all command action functions to
   Stop and save the command timer if it was started
*/
void _stopCommandTimer(Options& opts)
{
    if (opts.time)
    {
        opts.elapsedTime.stop();
        opts.saveElapsedTime = opts.elapsedTime.getElapsed();
    }
}

double _getElapsedTime(Options& opts)
{
    if (opts.time)
    {
        opts.elapsedTime.stop();
        double rtn =  opts.elapsedTime.getElapsed();
        opts.elapsedTime.start();
        return rtn;
    }
    return 0;
}

// get a String that is the elapsed time to hundredths of a
// second (2 decimal places)
String _getElapsedTimeFmtd(Options& opts)
{
    String str;
    if (opts.time)
    {
        opts.elapsedTime.stop();
        double rtn =  opts.elapsedTime.getElapsed();
        opts.elapsedTime.start();
        str.appendPrintf("%1.2f sec ", rtn);

        return str;
    }
    return "0 sec ";
}


/************************************************************************
*
*   Display functions to support the verbose display of input parameters
*
*************************************************************************/

void _showValueParameters(const Options& opts)
{
    for (Uint32 i = 0; i < opts.valueParams.size(); i++)
    {
        cout << opts.valueParams[i] << " ";
    }
    cout << endl;
}

//KS_PULL_BEGIN
// Map Uint32 to a Pegasus string
String _toString(Uint32 i)
{
    char scratchBuffer[22];
    Uint32 n;
    String s = Uint32ToString(scratchBuffer,i,n);
    return(s);
}

// display for Uint32 arguments.  Displays the value
// or NULL
String _toString(Uint32Arg x)
{
    char scratchBuffer[22];
    Uint32 n;
    String s = ((x.isNull()?
        "NULL"
        :
        //_toString(x.getValue()) )
        Uint32ToString(scratchBuffer,x.getValue(),n) )
        );

    return(s);
}

/*
    Test the received response element count against the
    maximum allowed for this response.
    @return true if too many
*/
Boolean _testRcvTooManyElements(
    const Uint32Arg& maxObjectCount,
    Uint32 returnedElementCount, Boolean verbose)
{
    if (!maxObjectCount.isNull())
    {
        if ((maxObjectCount.getValue() < returnedElementCount) && verbose)
        {
             cout << "Error. Operation"
                    " returned more than requested elements."
                << " Returned " << returnedElementCount << " elements. "
                << "maxObjectCount = " << maxObjectCount.getValue() << endl;
             return(true);
        }
    }
    // We can only return true if we sent Null
    return(false);
}
//KS_PULL_END

// Map the keybinding values from any CIMObjectPath that was input into
// our standard input form for use by objectBuilder.
void mapKeyBindingsToInputParameters(Options& opts)
{
    Array<CIMKeyBinding> keys = opts.getTargetObjectName().getKeyBindings();
    for (Uint32 i = 0 ; i < keys.size() ; i++)
    {
        String param = keys[i].getName().getString();
        param.append("=");
        param.append(keys[i].getValue());
        opts.valueParams.append(param);
    }
}

/*************************************************************
*
*  Functions for interactive selection from the console
*
*************************************************************/
/** Select one item from an array of items presented to
    the user. This prints the list and requests user input for
    the response.
    @param selectList Array<String> list of items from which the
    user has to select one.  Each item should be a printable string.
    @param what String that defines for the output string what type
    of items the select is based on (ex: "Instance Names");
    @return Uint32 representing the item to be selected.
*/
Uint32 _selectStringItem(const Array<String>& selectList, const String& what)
{
    Uint32 rtn = 0;
    Uint32 listSize = selectList.size();

    for (Uint32 i = 0 ; i < listSize; i++)
    {
        cout << i + 1 << ": " << selectList[i].getCString() << endl;
    }

    while (rtn < 1 || rtn > listSize)
    {
        cout << "Select " << what
             << " (1.." << listSize << ")? " << flush;

        // if input is not a valid integer, cin will be set to fail status.
        // and rtn will retain its previous value, so the loop could continue.
        cin >> rtn;

        if (cin.fail())
        {
            cin.clear();
            cin.ignore(0x7fffffff, '\n');
        }
    }

    return rtn-1;
}

/** Allow user to select one instance name. Do server
    EnumerateNames for input className and ask user to select a
    singe result from the enumerates returned.
    @param className CIMName for the class to enumerate.
    @param instancePath CIMObjectPath of instance selected
    @return True if at least one instance returned by server.
    Else False and there is nothing in the instancePath

    NOTE: There is no clean way for the user to respond "none of
    the above" to the request to select a single item from the
    list.  They must select one or execute a program kill (ex.
    Ctrl C)
*/
Boolean _selectInstance(Options& opts,
    const CIMName& className,
    CIMObjectPath & instancePath)
{
    // Enumerate instance Names based on input class to get list
    Array<CIMObjectPath> instanceNames =
        opts.client.enumerateInstanceNames(opts.nameSpace,
                                      className);
    // create a corresponding String list
    Array<String> list;
    for (Uint32 i = 0 ; i < instanceNames.size() ; i++)
    {
        list.append(instanceNames[i].toString());
    }

    // return false if nothing in list
    if (list.size() == 0)
    {
        if (opts.verboseTest)
        {
            cout << "No instances exist for class " << className.getString()
                 << endl;
        }
        return false;
    }

    // ask user to select a single entry
    Uint32 rtn = _selectStringItem(list, "an Instance");

    instancePath = instanceNames[rtn];

    return true;
}

/** Use the interactive selection mechanism to get the instance if
    the input object is a class AND if opts.interactive flag is
    set.  This function is used by the associator/reference
    functions because just the existence of the object as class
    is insufficient since these functions accept both class and
    instance input for processing. If the tests are passed this
    function calls the server to enumerate the instance names
    possible and displays them for the user to select one.
    @param opts the context structure for this operaiton
    @param instancePath CIMObjectPath of instance selected if return
    is true.  Else, unchanged.
    @return Boolean True if an instance path is to be returned. If nothing
    is selected, returns False.
*/
Boolean _conditionalSelectInstance(Options& opts,
    CIMObjectPath & instancePath)
{
    // if class level and interactive set.
    if ((instancePath.getKeyBindings().size() == 0) && opts.interactive)
    {
        // Ask the user to select an instance. returns instancePath
        // with selected path

        return _selectInstance(opts, opts.getTargetObjectNameClassName(),
                               instancePath);
    }

    return true;
}

// Display detailed differences between two properties. They are assumed
// to be the same property with the same name. Displays the xml definition
// of the property and details about which characteristics differ.
// If testDetails true, attributes other than simply the value are tested
// for differences.
Boolean _compareProperty(CIMProperty& propTest,
                         CIMProperty& propRtnd,
                         const Options& opts,
                         Boolean detailedTest = false,
                         Boolean display = false)
{
    bool rtn = true;

    if (propTest.getName() != propRtnd.getName())
    {
        if (display)
        {
            cout << "Names differ. "
                 << propTest.getName().getString()
                 << " vs. "
                 << propRtnd.getName().getString()
                 << endl;
        }
        rtn = false;
    }

    if (propTest.getType() != propRtnd.getType())
    {
        if (display)
        {
            cout << "Types differ. "
                 << propTest.getType()
                 << " vs. "
                 << propRtnd.getType()
                 << endl;
        }
        rtn = false;
    }

    if (propTest.getValue() != propRtnd.getValue())
    {
        if (display)
        {
            cout << "Values differ" << endl;
            cout << ". Test Instance ";
            CIMCLIOutput::displayProperty(opts, propTest);
            cout << endl <<"Returned instance ";
            CIMCLIOutput::displayProperty(opts ,propRtnd);
            cout << endl;
        }
        rtn = false;
    }
    if (propTest.isArray() != propRtnd.isArray())
    {
        if (display)
        {
            cout << "isArray Attributes differ differ. "
                 << boolToString(propTest.isArray())
                 << " vs. "
                 << boolToString(propRtnd.isArray())
                 << endl;
        }
        rtn = false;
    }

    // if detailed test specified, we test arraysize, classOrigin,
    // propagated, and qualifiers also
    if (detailedTest)
    {
        if (propTest.getArraySize() !=
            propRtnd.getArraySize())
        {
            if (display)
            {
                cout << "ArraySize Attributes differ differ. "
                     << propTest.getArraySize()
                     << " vs. "
                     << propRtnd.getArraySize()
                     << endl;
            }
            rtn = false;
        }

        if (propTest.getClassOrigin() !=
             propRtnd.getClassOrigin())
        {
            if (display)
            {
                cout << "ClassOrigin values differ.  "
                     << propTest.getClassOrigin().getString()
                     << " vs. "
                     << propRtnd.getClassOrigin().getString()
                     << endl;
            }
            rtn = false;
        }
        if (propTest.getPropagated() !=
             propRtnd.getPropagated())
        {
            if (display)
            {
                cout << "getPropagated values differ.  "
                     << boolToString(propTest.getPropagated())
                     << " vs. "
                     << boolToString(propRtnd.getPropagated())
                     << endl;
            }
            rtn = false;
        }

        if (propTest.getQualifierCount() !=
             propRtnd.getQualifierCount())
        {
            if (display)
            {
                cout << "ClassOrigin values differ. "
                     << propTest.getQualifierCount()
                     << " vs. "
                     << propRtnd.getQualifierCount()
                     << endl;
            }
            rtn = false;
        }
    }
    return rtn;
}
/*
    Compare two instances for equality in terms of number and names of
    properties and property values
    ASSUMPTION: Firstproperty is test, second is returned instance. We use
    this assumption in outputs
*/
Boolean _compareInstances(CIMInstance& inst1,
                          CIMInstance& inst2,
                          Options& opts,
                          Boolean detailedTest,
                          Boolean verbose)
{
    Boolean returnValue = true;


    // If the number of properties not the same in the two instances
    // rtnd instance  must have more than test instance.
    if (inst1.getPropertyCount() != inst2.getPropertyCount())
    {
        returnValue = false;
        if (verbose)
        {
            for (Uint32 i = 0 ; i < inst2.getPropertyCount() ; i++)
            {
                CIMProperty inst2Property = inst2.getProperty(i);
                CIMName testName = inst2Property.getName();
                if (inst1.findProperty(testName) == PEG_NOT_FOUND)
                {
                    cout << "Error: property " << testName.getString()
                        << " not found in test instance" << endl;
                }
            }
            for (Uint32 i = 0 ; i < inst1.getPropertyCount() ; i++)
            {
                CIMProperty inst1Property = inst1.getProperty(i);
                CIMName testName = inst1Property.getName();
                if (inst2.findProperty(testName) == PEG_NOT_FOUND)
                {
                    cout << "Error: property " << testName.getString()
                        << " not found in returned instance" << endl;
                }
            }
        }
        return returnValue;
    }
    // for each property in the test instance.
    // If there are extra properties in the returned instance we do not not
    // that here.  See next set of tests.
    for (Uint32 i = 0 ; i < inst1.getPropertyCount(); i++)
    {
        CIMProperty inst1Property = inst1.getProperty(i);
        CIMName testName = inst1Property.getName();
        Uint32 pos;

        // test for property in returned instance
        if ((pos = inst2.findProperty(testName)) != PEG_NOT_FOUND)
        {
            CIMProperty inst2Property = inst2.getProperty(pos);

            // if the instances are identical pass the test
            // else we will compare in detail
            if (!inst1Property.identical(inst2Property))
            {
                // compare the properties.  Normally we test primarily
                // on value but there is a detailed test for all of the
                // attributes.
                returnValue = _compareProperty(inst1Property,
                                               inst2Property,
                                               opts,
                                               detailedTest,
                                               verbose);
            }
        }

        else   // Property not found in second instance
        {
            returnValue = false;
            if (verbose)
            {
                cout << "Error: Property " << testName.getString()
                    << "not found in returned instance" << endl;
            }
            return returnValue;
        }

    }
    for (Uint32 i = 0; i < inst2.getPropertyCount(); i++ )
    {
        CIMProperty inst2Property = inst2.getProperty(i);
        CIMName testName = inst2Property.getName();
        Uint32 pos;

        // test for property in returned instance
        if ((pos = inst1.findProperty(testName)) != PEG_NOT_FOUND)
        {
            CIMProperty inst1Property = inst1.getProperty(pos);

            // if the instances are identical pass the test
            // else we will compare in detail
            if (!inst2Property.identical(inst1Property))
            {
                // compare the properties.  Normally we test primarily
                // on value but there is a detailed test for all of the
                // attributes.
                returnValue = _compareProperty(inst2Property,
                                 inst1Property,
                                 opts,
                                 detailedTest,
                                 verbose);
            }
        }
        else   // Property not found in second instance
        {
            returnValue = false;
            if (verbose)
            {
                cout << "Error: Property " << testName.getString()
                     << "not found in test instance" << endl;
            }
            return returnValue;
        }
    }
    return returnValue;
}
/******************************************************************************
//
//  Functions to get the interop namespace and the namespaces in the
//  target cimserver.
//
******************************************************************************/
/*
    Find the most likely candidate for the interop namespace using the class
    CIM_Namespace which should exist in the Interop namespace.  This function
    tests the standard expected inputs and appends namespaces input in the
    nsList input. It returns the namespace found and the instances of the
    CIM_Namespace class in that namespace.
    If the interop namespace found, the instances of CIM_Namespace are
    returned in the instances parameter.
    FUTURE: Determine a more complete algorithm for determining the
    interop namespace.  Simply the existence of this class may not always
    be sufficient.
*/
Boolean _findInteropNamespace(Options& opts,
                              const Array<CIMNamespaceName> & nsList,
                              Array<CIMInstance>& instances,
                              CIMNamespaceName& nsSelected)
{
    CIMName className = PEGASUS_CLASSNAME_CIMNAMESPACE;
    Array<CIMNamespaceName> interopNs;

    interopNs.appendArray(nsList);
    interopNs.append(PEGASUS_NAMESPACENAME_INTEROP);
    interopNs.append("interop");
    interopNs.append("root/interop");
    Boolean nsFound = false;

    for (Uint32 i = 0 ; i < interopNs.size() ; i++)
    {
        try
        {
            instances = opts.client.enumerateInstances(interopNs[i],
                                                       className);

            nsFound = true;

            if (opts.verboseTest)
            {
                cout << "Found CIM_NamespaceName in namespace "
                    << interopNs[i].getString()
                    << " with " << instances.size() << " instances "
                    << endl;
            }
            nsSelected = interopNs[i];
            break;
        }
        catch(CIMException & e)
        {
            /* If exceptions caught here for all namespaces tested assume that
               target CIMOM does not support CIM_Namespace class.
               Therefore we have to revert to the __namespaces class to
               get namespace information. (Which may only retrun a subset of
               namspaces.
               NOTE: Possible exceptions include namespace does not exist
                     and class does not exist.
            */
            cerr << "Info: CIMException return to CIM_NamespaceName enumerate"
                " request. "
                << e.getMessage() << endl;
        }
    }
    return nsFound;
}
/*
    Use the __namespace class to attempt to get namespace names.  Returns
    an array containing namespaces found. Used by _getNameSpaceNames(...)
*/
Array<CIMNamespaceName> _getNameSpacesWith__namespace(Options& opts)
{
    Array<CIMNamespaceName> namespaceNames;
    CIMName nsClassName = CIMName("__namespace");

    // TODO Determine if we really need this statement
    opts.nameSpace = PEGASUS_NAMESPACENAME_INTEROP.getString();

    // Build the namespaces incrementally starting at the root
    // ATTN: 20030319 KS today we start with the "root" directory but
    // this is wrong. We should be
    // starting with null (no directory) but today we get an xml error
    // return in Pegasus
    // returned for this call. Note that the specification requires
    // that the root namespace be used
    // when __namespace is defined but does not require that it be
    // the root for all namespaces. That  is a hole is the spec,
    // not in our code.

    // Determine why we need the following statement
    namespaceNames.append(opts.nameSpace);

    Uint32 start = 0;
    Uint32 end = namespaceNames.size();

    do
    {
        // for all new elements in the output array
        for (Uint32 range = start; range < end; range ++)
        {
            // Get the next increment in naming for all a name element
            // in the array
            Array<CIMInstance> instances = opts.client.enumerateInstances(
                namespaceNames[range],nsClassName);
            for (Uint32 i = 0 ; i < instances.size(); i++)
            {
                Uint32 pos;
                // if we find the property and it is a string, use it.
                if ((pos = instances[i].findProperty("name"))
                        != PEG_NOT_FOUND)
                {
                    CIMValue value;
                    String namespaceComponent;
                    value = instances[i].getProperty(pos).getValue();
                    if (value.getType() == CIMTYPE_STRING)
                    {
                        value.get(namespaceComponent);

                        String ns = namespaceNames[range].getString();
                        ns.append("/");
                        ns.append(namespaceComponent);
                        namespaceNames.append(ns);
                    }
                }
            }
            start = end;
            end = namespaceNames.size();
        }
    }
    while (start != end);

    return namespaceNames;
}

/*
    List the namespaces in the target host CIMObjectManager.
    This function tries several options to generate a list of the
    namespaces in the target CIMOM including:
    1. Try to list a target Class in the interop namespace.  Note that
    it tries several different guesses to get the target namespace
    2. If a class name is provided as opts.Classname (i.e typically as
    argv2 in the direct call operation, that class is substituted for
    the CIM Namespace class.
    3. If a namespace is provided in opts.namespace (typically through
    the -n input option) that namespace is used as the target namespace
    4. If an asterick "*" is found in opts.namespace, a selection of
    possible namespaces is used including the pegasus default, interop,
    and root/interop.
    5. Finally, if no namespace can be found with the namespace class
    an attempt is made to get the namespace with the __namespace class
    and its incremental descent algorithm.
*/

Array<CIMNamespaceName> _getNameSpaceNames(Options& opts)
{
    //CIMName className = PEGASUS_CLASSNAME_CIMNAMESPACE;

    Array<CIMNamespaceName> namespaceNames;
    Array<CIMInstance> instances;
    Array<CIMNamespaceName> interopNs;

    // if there is a name in the input namespace, use it.
    if (opts.nameSpace != "*" && opts.nameSpace.size() != 0)
    {
        interopNs.append(opts.nameSpace);
    }

    // if a namespace with the CIM_Namespace class is found and instances are
    // returned, we can simply list information from the instances.
    // Assumption: all namespaces containing this class will return the
    // same information.

    CIMNamespaceName interopNamespaceFnd;
    if (_findInteropNamespace(opts, interopNs, instances, interopNamespaceFnd))
    {
        for (Uint32 i = 0 ; i < instances.size(); i++)
        {
            Uint32 pos;
            // If we find the property and it is a string, use it.
            if ((pos = instances[i].findProperty("name")) != PEG_NOT_FOUND)
            {
                CIMValue value;
                String namespaceComponent;
                value = instances[i].getProperty(pos).getValue();
                if (value.getType() == CIMTYPE_STRING)
                {
                    value.get(namespaceComponent);
                    namespaceNames.append(CIMNamespaceName(
                        namespaceComponent));
                }
            }
        }
    }
    else  // No  namespace with CIM_Namespace class found.
    {
        if (opts.verboseTest)
        {
            cout << "Using __namespace class to find namespaces"
                << endl;
        }
        namespaceNames = _getNameSpacesWith__namespace(opts);
    }

    // Validate that all of the returned entities are really namespaces.
    // It is legal for us to have a name component that is really not a
    // namespace (ex. root/fred/john is a namespace  but root/fred is not.
    // There is no clearly defined test for this so we will simply try to
    // get something, in this case a well known assoication

    Array<CIMNamespaceName> rtns;

    for (Uint32 i = 0 ; i < namespaceNames.size() ; i++)
    {
        try
        {
            CIMQualifierDecl cimQualifierDecl;
            cimQualifierDecl = opts.client.getQualifier(namespaceNames[i],
                                           "Association");

            rtns.append(namespaceNames[i]);
        }
        catch(CIMException& e)
        {
            if (e.getCode() != CIM_ERR_INVALID_NAMESPACE)
            {
                rtns.append(namespaceNames[i]);
            }
            else
            {
                cerr << "Warning: " << namespaceNames[i].getString()
                     << " Apparently not a real namespace. Ignored"
                     << endl;
            }
        }
    }
    return rtns;
}

/*
    Determine whether cimcli sets includequalifiers true or false for
    operation based on default (i.e. what is default for this operation)
    and the includeQualifiersRequest, notIncludeQualifiersRequest
    parameters.
    Result put into incudeQualifiers and returned as String for
    display.  This required because some operations have default true
    and other false.
    */
void _resolveIncludeQualifiers(Options& opts, Boolean defaultValue)
{
    // Assure niq and iq not both supplied. They are incompatible
    if (opts.includeQualifiersRequested && opts.notIncludeQualifiersRequested)
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR, MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.INC_QUAL_PARM_ERR",
            "Error: -niq and -iq parameters cannot be used together."));
        //#N -niq and -iq are literals. Do not translate
        //#N @version 2.14
        //#P 50
        //#T INC_QUAL_PARM_ERR
        //#S Error: -niq and -iq parameters cannot be used together.
    }
    // if default is true (ex class operations), we test for -niq received
    // depend only on the -niq input.
    if (defaultValue)
    {
        opts.includeQualifiers = opts.notIncludeQualifiersRequested ?
                                    false : true;
    }
    else
    {
        opts.includeQualifiers = opts.includeQualifiersRequested ?
                                    true : false;
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//     The following code section defines the action functions             //
//     These functions are a combination of                                //
//     a. the CIM Operations as defined by the DMTF specification          //
//       ex. getInstance implemented for cimcli.                           //
//     b. Other operations such as ns for get namespaces that              //
//        might be useful to CIMOM testers                                 //
//     Input parameters are defined in the                                 //
//     opts structure.  There are no exception catches.                    //
//     exception handling is in the main path.                             //
/////////////////////////////////////////////////////////////////////////////

/*********************** enumerateAllInstanceNames ***************************/

/* This command searches an entire namespace and displays names of
   all instances.
   It is in effect enumerate classes followed by enumerate instances.
   The user may either provide a starting class or not, in which case
   it enumerates instance names for the complete namespace, not simply the
   defined class.

   It normally returns all instances of all classes below the input class name
   (a single enumerateInstances)
   unless no class was provided with the command.  Then it enumerates
   all classes in the namespace.

   If the --sum option is defined it returns only the count of instances
   for classes that return instances.  NOTE: This is the count for each
   class where an instance is returned. Thus inputting a single class name
   may return a list of instances of multiple classes.  The summary enumerates
   the number of instances of each class in the enumerateInstancesNames
   response.

   This operation also allows processing all namespaces in a Server with
   a single request by defining the input namespace as "*".  In that case,
   it gets a list of all namespaces and process them all. Summary results
   are returned for each namespace.
*/

int enumerateAllInstanceNames(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "enumerateAllInstanceNames (niall) "
            << "Namespace = " << opts.nameSpace
            << ", Class = " << opts.className.getString()
            << endl;
    }

    // This operation allows * as namespace value which means get
    // from all namespaces.
    Array<CIMNamespaceName> nsList;
    if (opts.nameSpace != "*")
    {
        nsList.append(opts.nameSpace);
    }
    else
    {
        nsList = _getNameSpaceNames(opts);
    }

    if (opts.verboseTest)
    {
        cout << "Namespaces List for niall: ";
        for (Uint32 i = 0 ; i < nsList.size() ; i++)
        {
            cout << ((i > 0)? ", " : "")
                 << nsList[i].getString() << endl;
        }
        cout << endl;
    }

    CIMName saveClassName = opts.className;

    // loop to process for each namespace
    for (Uint32 i = 0 ; i < nsList.size() ; i++)
    {
        opts.nameSpace = nsList[i].getString();
        Array<CIMName> classNames;

        // The timer really has no meaning for this operation since
        // we merge output and acquisition over multiple operations
        _startCommandTimer(opts);

        // If className is null, assume that user wants to start at
        // class hiearchy root and we get top level class names. Else
        // we will enumerate just the classname provided.
        opts.className = saveClassName;
        if (opts.className.isNull())
        {
            if (opts.verboseTest)
            {
                cout << "EnumerateClassNames for namespace "
                    << opts.nameSpace << endl;
            }
            try
            {
                classNames = opts.client.enumerateClassNames(opts.nameSpace,
                                                    opts.className,
                                                    false);
            }
            catch(CIMException& e)
            {
                if (e.getCode() == CIM_ERR_INVALID_CLASS)
                {
                    cerr << "Class " << opts.className.getString()
                         << " does not exist in namespace "
                         << opts.nameSpace << endl;
                    continue;
                }
            }

            _stopCommandTimer(opts);
        }
        else
        {
            classNames.append(opts.className);
        }

        if (opts.verboseTest)
        {
            cout << "Evaluate for following list of classes:" << endl;

            for (Uint32 iClass = 0; iClass < classNames.size(); iClass++)
            {
                cout << ((iClass > 0)? ", " : "")
                     << classNames[iClass].getString();
            }
            cout << endl;
        }

        // Create associative array to count instances of each
        // class in returned instance names list. Value function is
        // Uint32 to count instances of each class in array
        typedef HashTable<String, Uint32, EqualFunc<String>,
            HashFunc<String> > InstCounter;
        InstCounter instCounter;

        // Enumerate instance names for all classes in list. This is
        // tree of all classes below defined classname input or just
        // the input classname if one was supplied with request
        Uint32 totalInstances = 0;
        for (Uint32 iClass = 0; iClass < classNames.size(); iClass++)
        {
            if (opts.verboseTest)
            {
                cout << "EnumerateInstanceNames "
                    << "Namespace = " << opts.nameSpace
                    << ", Class = " << classNames[iClass].getString()
                    << endl;
            }

            Array<CIMObjectPath> instanceNames;
            try
            {
                instanceNames =
                    opts.client.enumerateInstanceNames(opts.nameSpace,
                                                       classNames[iClass]);
                totalInstances += instanceNames.size();
            }
            catch(CIMException& e )
            {
                cerr << "Warning: Exception in niall for"
                        " enumerateInstanceNames "
                     << " Namespace=" << opts.nameSpace
                     << " Class=" << classNames[iClass].getString()
                     << ".  " << e.getMessage() << ". Continuing." << endl;
                continue;
            }

            String s = "instances of class";
            opts.className = classNames[iClass];
            if (!opts.summary)
            {
                CIMCLIOutput::displayPaths(opts, instanceNames, s);
            }

            // Insert new classnames in instCounter table and
            // increment count for existing names.
            for (Uint32 i = 0 ; i < instanceNames.size(); i++)
            {
                String className = instanceNames[i].getClassName().getString();

                // Insert new entry in hash table or increment
                // current entry counter
                if (!instCounter.insert(className, 1))
                {
                    Uint32* value=0;
                    instCounter.lookupReference(className, value);
                    *value = *value + 1;
                }
            }
        }

        // Get max size of key property to justify output columns
        size_t maxSize = 0;
        for (InstCounter::Iterator i = instCounter.start(); i; i++)
            if (i.key().size() > maxSize)
                maxSize = i.key().size();

        // Output namespace, className, instance count for all classes that
        // have nonzero instance count (i.e. all entries in the hash table
        for (InstCounter::Iterator i = instCounter.start(); i; i++)
        {
            String key = i.key();
            while (key.size() < maxSize)
            {
                key.append(" ");
            }

            cout << opts.nameSpace << " "
                 << key
                 << " " << i.value() << endl;
        }

        // get list of all classes in namespace for summary info.
        Array<CIMName> classesNamesTotal = opts.client.enumerateClassNames(
                                    opts.nameSpace,
                                    CIMName(),
                                    true);
        // get list of all classes below specified class
        String enumCount;
        String enumTxt;
        if (classNames.size() == 1)
        {
            try
            {
                Array<CIMName> classNamesEnum =
                    opts.client.enumerateClassNames(
                        opts.nameSpace,
                        opts.className,
                        true);
                enumTxt = " Enumerated=";
                char buf[22];
                Uint32 sz;
                enumTxt.append(Uint32ToString(buf,classNamesEnum.size(), sz));
            }
            catch(CIMException& e )
            {
                cerr << "Warning: Exception in niall for"
                        " enumerateClassNames "
                     << " Namespace=" << opts.nameSpace
                     << " Class=" << opts.className.getString()
                     << ".  " << e.getMessage() << ". Continuing." << endl;
                continue;
            }
        }

        // for this namespace, print number in each class
        cout << opts.nameSpace
             << " Total Classes=" << classesNamesTotal.size() + 1
             << enumTxt
             << ", with Instances="
             << instCounter.size()
             << ", Instances=" << totalInstances
             << endl;
    }  // end processing namespace for loop

    return CIMCLI_RTN_CODE_OK;
}


/*********************** enumerateInstanceNames  ***************************/
/*
    This action function executes the client enumerateInstanceNames
    client operation.  Inputs are the namespace and classname
*/
int enumerateInstanceNames(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "EnumerateInstanceNames "
            << "Namespace= " << opts.nameSpace
            << ", Class= " << opts.className.getString()
            << endl;
    }

    _startCommandTimer(opts);

    Array<CIMObjectPath> instanceNames =
        opts.client.enumerateInstanceNames(opts.nameSpace,
                                      opts.className);

    _stopCommandTimer(opts);

    CIMCLIOutput::displayPaths(opts,instanceNames);

    return CIMCLI_RTN_CODE_OK;
}


/************************** enumerateInstances  ***************************/
/*
    This action function executes the enumerateInstances
    client operation. Inputs are the parameters for the CIMCLient call
*/

int enumerateInstances(Options& opts)
{
    // Resolve the IncludeQualifiers -iq vs -niq qualifiers default is true.
    _resolveIncludeQualifiers(opts, false);

    if (opts.verboseTest)
    {
        cout << "EnumerateInstances "
            << "Namespace = " << opts.nameSpace
            << ", Class = " << opts.className.getString()
            << ", deepInheritance = " << boolToString(opts.deepInheritance)
            << ", localOnly = " << boolToString(opts.localOnly)
            << ", includeQualifiers = "
                << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin = "
                << boolToString(opts.includeClassOrigin)
            << ", PropertyList = " << opts.propertyList.toString()
            << endl;
    }

    Array<CIMInstance> instances;

    _startCommandTimer(opts);

    instances = opts.client.enumerateInstances( opts.nameSpace,
                                           opts.className,
                                           opts.deepInheritance,
                                           opts.localOnly,
                                           opts.includeQualifiers,
                                           opts.includeClassOrigin,
                                           opts.propertyList );

    _stopCommandTimer(opts);

    CIMCLIOutput::displayInstances(opts, instances);

    return CIMCLI_RTN_CODE_OK;
}

//KS_PULL_BEGIN
/************************** Pull Common functions  ***************************/

/*
    These common functions serve for pulling all of the opened operations.
    They provide a loop to pull either instances or CIMObjectPaths until one
    of the following end conditions is met:
        endOfSequence found
        maxObjToReceive hit (if maxObjToReceive != 0)
        Exception or CIMException - Handled outside these functions.

    These functions also execute an inter-pull-operation delay if the
    pullDelay parameter is set.

    The complete set of instances or paths received is returned.
    KS_TODO - Keep more detailed statistics on pull, how many received,
    timing, etc.

*/

/**
 * _pullInstancesWithPath executes the proper pullInstances function in
 * a loop to get everything until either endOfSequence received
 * or the size limit defined on input is received.
 * @param  instances Array<CIMInstance> pulled
 * @param  enumerationContext CIMEnumerationContext for this
 *                            operation
 * @param withPath Boolean that defines whether the
 *                 pullInstancesWithPath or pullInstances should
 *                 be executed.
 */

int _pullInstancesWithPath(
    Options& opts,
    Array<CIMInstance>& instances,
    CIMEnumerationContext& enumerationContext,
    Boolean withPath = true)
{
    Boolean endOfSequence = false;
    Uint32 maxPullObj = opts.maxObjectCount;
    Uint32 maxObjToReceive = opts.maxObjectsToReceive;
    Boolean rtn = true;
    Boolean limitSize = (maxObjToReceive == 0)? false: true;

    while (!endOfSequence)
    {
        // If max objects limit defined on input, end and return true
        if (limitSize && (instances.size() >= maxObjToReceive))
        {
            if (opts.verboseTest)
            {
                cout <<_getElapsedTimeFmtd(opts)
                    << "Issuing closeEnumeration after "
                     << instances.size() << " received because hit"
                     << " limit of " << limitSize << endl;
            }
            opts.client.closeEnumeration(enumerationContext);
            endOfSequence = true;
        }
        else
        {
            Uint32 remainder = maxObjToReceive - instances.size();
            if (limitSize && ((remainder) < maxPullObj))
            {
                if (opts.verboseTest)
                {
                    cout << "Remainder to request for limit = "
                         << remainder << endl;
                }
                maxPullObj = remainder;
            }
            if (opts.pullDelay != 0)
            {
                System::sleep(opts.pullDelay);
            }
            Array<CIMInstance> cimInstancesPulled;
            if (withPath)
            {
                if (opts.verboseTest)
                {
                    cout << _getElapsedTimeFmtd(opts)
                        << "pullInstancesWithPath. maxObj= "
                         << maxPullObj << endl;
                }
                cimInstancesPulled =
                    opts.client.pullInstancesWithPath(
                        enumerationContext,
                        endOfSequence,
                        maxPullObj);
                if (opts.verboseTest)
                {
                    cout << _getElapsedTimeFmtd(opts)
                        << "pullInstancesWithPath returned. "
                        << cimInstancesPulled.size() << " instances" << endl;
                }
            }
            else
            {
                if (opts.verboseTest)
                {
                    cout << _getElapsedTimeFmtd(opts)
                         << "pullInstances with path. maxObj= "
                         << maxPullObj << endl;
                }
                cimInstancesPulled =
                    opts.client.pullInstances(
                        enumerationContext,
                        endOfSequence,
                        maxPullObj);
                if (opts.verboseTest)
                {
                    cout << _getElapsedTimeFmtd(opts)
                        << "pullInstances returned. "
                         << cimInstancesPulled.size() << " instances" << endl;
                }
            }

            rtn = !_testRcvTooManyElements(maxPullObj,
                cimInstancesPulled.size(), opts.verboseTest);

            instances.appendArray(cimInstancesPulled);
        }
    }
    return rtn;
}


/*
    Common function for the loop to pull instance paths until
    EndOfSequence reached.
    @return - returns true unless some error occured. Note executing
    the close does not constitute error.
*/
Boolean _pullInstancePaths(
    Options& opts,
    Array<CIMObjectPath>& paths,
    CIMEnumerationContext& enumerationContext)
{
    Boolean endOfSequence = false;
    Uint32 maxPullObj = opts.maxObjectCount;
    Uint32 maxObjToReceive = opts.maxObjectsToReceive;

    Boolean rtn = true;
    Boolean limitSize = (maxObjToReceive == 0)? false: true;

    while (!endOfSequence)
    {
        // if we have exceeded the input maxObjectsToReceive
        // close the connection.
        if (limitSize && (paths.size() >= maxObjToReceive))
        {
            if (opts.verboseTest)
            {
                cout << _getElapsedTimeFmtd(opts)
                    << "pullInstancePaths close. " << endl;
            }
            opts.client.closeEnumeration(enumerationContext);
            endOfSequence = true;
        }
        else
        {
            Uint32 remainder = maxObjToReceive - paths.size();
            if (limitSize && ((remainder) < maxPullObj))
            {
                maxPullObj = remainder;
            }
            // if the delay options set, delay between operations by
            // defined amount
            if (opts.pullDelay != 0)
            {
                System::sleep(opts.pullDelay);
            }
            if (opts.verboseTest)
            {
                cout << _getElapsedTimeFmtd(opts) << " sec "
                     << "pullInstancepaths. maxObj= "
                     << maxPullObj << endl;
            }
            Array<CIMObjectPath> pathsPulled = opts.client.pullInstancePaths(
                enumerationContext,
                endOfSequence,
                maxPullObj);

            if (opts.verboseTest)
                {
                    cout << _getElapsedTimeFmtd(opts)
                        << "pullInstancePaths returned. "
                         << pathsPulled.size() << " paths" << endl;
                }

            rtn = !_testRcvTooManyElements(maxPullObj,
                pathsPulled.size(), opts.verboseTest);

            paths.appendArray(pathsPulled);
        }
    }
    return rtn;
}

// Create an output string of the common parameters of the pull operations.
// This just reduces code generated. Used for display of input element
// if verbose set.
String _displayPullCommonParam(Options& opts)
{
    String rtn = ", maxObjectCount = ";
    rtn.append(_toString(opts.maxObjectCount));
    rtn.append(", operationTimeout = ");
    rtn.append(", maxObjectsToReceive= ");
    rtn.append(opts.maxObjectsToReceive);
    rtn.append(", pullDelay = ");
    rtn.append(opts.pullDelay);
    return rtn;
}
/************************** PullEnumerateInstances  ***************************/
/*
    This action function executes the enumerateInstances
    client operation. Inputs are the parameters for the CIMCLient call
*/
int pullEnumerateInstances(Options& opts)
{
    // hide the opts defaultfor WQL query language
    if ((opts.queryLanguage == "WQL")  && (opts.query == ""))
    {
        opts.queryLanguage = "";
    }

    if (opts.verboseTest)
    {
        cout << "PullEnumerateInstances "
            << "Namespace = " << opts.nameSpace
            << ", Class = " << opts.className.getString()
            << ", deepInheritance = " << boolToString(opts.deepInheritance)
            << ", localOnly = " << boolToString(opts.localOnly)
            << ", includeQualifiers = " << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin = "
                << boolToString(opts.includeClassOrigin)
            << ", PropertyList = " << opts.propertyList.toString()
            << ", QueryLanguage = " << opts.queryLanguage
            << ", Query = " << opts.query
            << _displayPullCommonParam(opts)
            << endl;
    }

    Array<CIMInstance> instances;

    _startCommandTimer(opts);

    Boolean endOfSequence = false;

    // insure that we do not request more than limit on open
    if (opts.maxObjectsToReceive != 0)
    {
        // if ask for more than max we want, reset max count.
        if (opts.maxObjectCount > opts.maxObjectsToReceive)
        {
            opts.maxObjectCount = opts.maxObjectsToReceive;
        }
    }

    CIMEnumerationContext enumerationContext;
    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open maxObj "
            << opts.maxObjectCount << " instances " << endl;
    }

    instances = opts.client.openEnumerateInstances(
        enumerationContext,
        endOfSequence,
        opts.nameSpace,
        opts.className,
        opts.deepInheritance,
        opts.includeClassOrigin,
        opts.propertyList,
        opts.queryLanguage,
        opts.query,
        opts.pullOperationTimeout,
        opts.continueOnError,
        opts.maxObjectCount);

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open rtned "
            << instances.size()
            << " instances " << endl;
    }

    _testRcvTooManyElements(opts.maxObjectCount,
        instances.size(), opts.verboseTest);

    if (!endOfSequence)
    {
        if (!_pullInstancesWithPath(opts, instances, enumerationContext ))
        {
            // some error return from the pull loop.
            cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.PULL_RESP_ERR",
                "Pull Loop returned error, ex. too many response objects."));
            //#T PULL_RESP_ERR
            //#S Pull Loop returned error, ex. too many response objects.
        }
    }


    _stopCommandTimer(opts);

    CIMCLIOutput::displayInstances(opts, instances);

    return(CIMCLI_RTN_CODE_OK);
}

int pullEnumerateInstancePaths(Options& opts)
{
    // hide the opts defaultfor WQL query language
    if ((opts.queryLanguage == "WQL")  && (opts.query == ""))
    {
        opts.queryLanguage = "";
    }

    if (opts.verboseTest)
    {
        cout << "PullEnumerateInstances "
            << "Namespace = " << opts.nameSpace
            << ", Class = " << opts.className.getString()
            << ", QueryLanguage = " << opts.queryLanguage
            << ", Query = " << opts.query
            << _displayPullCommonParam(opts)
            << endl;
    }

    Array<CIMObjectPath> paths;

    _startCommandTimer(opts);

    Boolean endOfSequence = false;

    // insure that we do not request more than limit on open
    if (opts.maxObjectsToReceive != 0)
    {
        // if ask for more than max we want, reset max count.
        if (opts.maxObjectCount > opts.maxObjectsToReceive)
        {
            opts.maxObjectCount = opts.maxObjectsToReceive;
        }
    }

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open maxObj "
            << opts.maxObjectCount << " paths " << endl;
    }

    CIMEnumerationContext enumerationContext;
    paths = opts.client.openEnumerateInstancePaths( enumerationContext,
        endOfSequence,
        opts.nameSpace,
        opts.className,
        opts.queryLanguage,
        opts.query,
        opts.pullOperationTimeout,
        opts.continueOnError,
        opts.maxObjectCount);

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) <<"Open rtned " << paths.size()
            << " paths " << endl;
    }

    _testRcvTooManyElements(opts.maxObjectCount,
        paths.size(), opts.verboseTest);

    if (!endOfSequence)
    {
        if (!_pullInstancePaths(opts, paths, enumerationContext ))
        {
            // some error return from the pull loop.
            cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.PULL_RESP_ERR",
                "Pull Loop returned error, for example, too many responses."));
        }
    }

    _stopCommandTimer(opts);

    CIMCLIOutput::displayPaths(opts,paths);

    return(CIMCLI_RTN_CODE_OK);
}

int pullReferenceInstancePaths(Options& opts)
{
        // hide the opts defaultfor WQL query language
    if ((opts.queryLanguage == "WQL")  && (opts.query == ""))
    {
        opts.queryLanguage = "";
    }

    if (opts.verboseTest)
    {
        cout << "PullReferenceInstanceNames "
            << "Namespace= " << opts.nameSpace
            << ", ObjectPath= " << opts.getTargetObjectNameStr()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << ", QueryLanguage = " << opts.queryLanguage
            << ", Query = " << opts.query
            << _displayPullCommonParam(opts)
            << endl;
    }
    // do conditional select of instance if params properly set.
    CIMObjectPath thisObjectPath(opts.getTargetObjectName());
    if (!_conditionalSelectInstance(opts, thisObjectPath))
    {
        return(CIMCLI_RTN_CODE_OK);
    }

    _startCommandTimer(opts);

    Boolean endOfSequence = false;

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open maxObj "
            << opts.maxObjectCount << " paths " << endl;
    }

    CIMEnumerationContext enumerationContext;
    Array<CIMObjectPath> paths =
        opts.client.openReferenceInstancePaths( enumerationContext,
                                endOfSequence,
                                opts.nameSpace,
                                thisObjectPath,
                                opts.resultClass,
                                opts.role,
                                opts.queryLanguage,
                                opts.query,
                                opts.pullOperationTimeout,
                                opts.continueOnError,
                                opts.maxObjectCount
                                );

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) <<"Open rtned " << paths.size()
            << " paths " << endl;
    }
    _testRcvTooManyElements(opts.maxObjectCount,
        paths.size(), opts.verboseTest);

    if (!endOfSequence)
    {
        if (!_pullInstancePaths(opts, paths, enumerationContext ))
        {
            // some error return from the pull loop.
            cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.PULL_RESP_ERR",
                "Pull Loop returned error, for example, too many responses."));
        }
    }

    _stopCommandTimer(opts);

    String s = "pull referenceNames";
    opts.className = thisObjectPath.getClassName();
    CIMCLIOutput::displayPaths(opts, paths, s);

    return(CIMCLI_RTN_CODE_OK);}

/*
  Initiate a OpenReferenceInstances call and pull the instances.
*/
int pullReferenceInstances(Options& opts)
{
    // hide the opts defaultfor WQL query language
    if ((opts.queryLanguage == "WQL")  && (opts.query == ""))
    {
        opts.queryLanguage = "";
    }

    if (opts.verboseTest)
    {
        cout << "PullReferenceInstances "
            << "Namespace= " << opts.nameSpace
            << ", Object= " << opts.getTargetObjectNameStr()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << ", includeQualifiers= " << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin= " << boolToString(opts.includeClassOrigin)
            << ", CIMPropertyList= " << opts.propertyList.toString()
            << ", QueryLanguage= " << opts.queryLanguage
            << ", Query= " << opts.query
            << _displayPullCommonParam(opts)
            << endl;
    }

    // do conditional select of instance if params properly set.
    CIMObjectPath thisObjectPath(opts.getTargetObjectName());
    if (!_conditionalSelectInstance(opts, thisObjectPath))
        return(0);

    _startCommandTimer(opts);

    Boolean endOfSequence = false;

    CIMEnumerationContext enumerationContext;

    Array<CIMInstance> instances;

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open maxObj "
            << opts.maxObjectCount << " instances " << endl;
    }

    instances = opts.client.openReferenceInstances(
        enumerationContext,
        endOfSequence,
        opts.nameSpace,
        thisObjectPath,
        opts.resultClass,
        opts.role,
        opts.includeClassOrigin,
        opts.propertyList,
        opts.queryLanguage,
        opts.query,
        opts.pullOperationTimeout,
        opts.continueOnError,
        opts.maxObjectCount);

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) <<"Open rtned " << instances.size()
            << " instances " << endl;
    }

    _testRcvTooManyElements(opts.maxObjectCount,
        instances.size(), opts.verboseTest);

    if (!endOfSequence)
    {
        if (!_pullInstancesWithPath(opts, instances, enumerationContext ))
        {
            // some error return from the pull loop.
            cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.PULL_RESP_ERR",
                "Pull Loop returned error, for example, too many responses."));
        }
    }
    _stopCommandTimer(opts);

    String s = "pull references";
    //// KS_TBDCIMCLIOutput::displayInstances(opts,instances,s);

    CIMCLIOutput::displayInstances(opts,instances);
    return(CIMCLI_RTN_CODE_OK);
}


int pullAssociatorInstancePaths(Options& opts)
{
    // hide the opts defaultfor WQL query language
    if ((opts.queryLanguage == "WQL")  && (opts.query == ""))
    {
        opts.queryLanguage = "";
    }

    if (opts.verboseTest)
    {
        cout << "PullReferenceInstanceNames "
            << "Namespace= " << opts.nameSpace
            << ", ObjectPath= " << opts.getTargetObjectNameStr()
            << ", assocClass= " << opts.assocClass.getString()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << ", QueryLanguage = " << opts.queryLanguage
            << ", Query = " << opts.query
            << _displayPullCommonParam(opts)
            << endl;
    }
    // do conditional select of instance if params properly set.
    CIMObjectPath thisObjectPath(opts.getTargetObjectName());
    if (!_conditionalSelectInstance(opts, thisObjectPath))
        return(0);

    _startCommandTimer(opts);

    Boolean endOfSequence = false;
    String filterQueryLanguage;
    String filterQuery;

    CIMEnumerationContext enumerationContext;

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open maxObj "
            << opts.maxObjectCount << " paths " << endl;
    }

    Array<CIMObjectPath> paths =
        opts.client.openAssociatorInstancePaths( enumerationContext,
            endOfSequence,
            opts.nameSpace,
            thisObjectPath,
            opts.assocClass,
            opts.resultClass,
            opts.role,
            opts.resultRole,
            filterQueryLanguage,
            filterQuery,
            opts.pullOperationTimeout,
            opts.continueOnError,
            opts.maxObjectCount
            );

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd (opts) << "Open rtned " << paths.size()
            << " paths " << endl;
    }

    _testRcvTooManyElements(opts.maxObjectCount,
        paths.size(), opts.verboseTest);

    if (!endOfSequence)
    {
        if (!_pullInstancePaths(opts, paths, enumerationContext ))
        {
            // some error return from the pull loop.
            cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.PULL_RESP_ERR",
                "Pull Loop returned error, for example, too many responses."));
        }
    }

    _stopCommandTimer(opts);

    String s = "pullInstancePaths";
    opts.className = thisObjectPath.getClassName();
    CIMCLIOutput::displayPaths(opts, paths, s);

    return(CIMCLI_RTN_CODE_OK);
}

int pullAssociatorInstances(Options& opts)
{
    // hide the opts defaultfor WQL query language
    if ((opts.queryLanguage == "WQL")  && (opts.query == ""))
    {
        opts.queryLanguage = "";
    }

    if (opts.verboseTest)
    {
        cout << "PullAssoociatorInstances "
            << "Namespace= " << opts.nameSpace
            << ", Object= " << opts.getTargetObjectNameStr()
            << ", assocClass= " << opts.assocClass.getString()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << ", resultRole= " << opts.resultRole
            << ", includeQualifiers= " << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin= " << boolToString(opts.includeClassOrigin)
            << ", CIMPropertyList= " << opts.propertyList.toString()
            << ", QueryLanguage = " << opts.queryLanguage
            << ", Query = " << opts.query
            << _displayPullCommonParam(opts)
            << endl;
    }

    // do conditional select of instance if params properly set.
    CIMObjectPath thisObjectPath(opts.getTargetObjectName());

    if (!_conditionalSelectInstance(opts, thisObjectPath))
        return(CIMCLI_RTN_CODE_OK);

    _startCommandTimer(opts);

    Boolean endOfSequence = false;

    CIMEnumerationContext enumerationContext;

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open maxObj "
            << opts.maxObjectCount << " instances " << endl;
    }

    Array<CIMInstance> instances;
    instances = opts.client.openAssociatorInstances(
        enumerationContext,
        endOfSequence,
        opts.nameSpace,
        thisObjectPath,
        opts.assocClass,
        opts.resultClass,
        opts.role,
        opts.resultRole,
        opts.includeClassOrigin,
        opts.propertyList,
        opts.queryLanguage,
        opts.query,
        opts.pullOperationTimeout,
        opts.continueOnError,
        opts.maxObjectCount);

    if (opts.verboseTest)
    {
        cout << _getElapsedTimeFmtd(opts) << "Open rtned " << instances.size()
            << " instances " << endl;
    }

    _testRcvTooManyElements(opts.maxObjectCount,
        instances.size(), opts.verboseTest);

    if (!endOfSequence)
    {
        if (!_pullInstancesWithPath(opts, instances, enumerationContext ))
        {
            // some error return from the pull loop.
            cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.PULL_RESP_ERR",
                "Pull Loop returned error, for example, too many responses."));
        }
    }

    _stopCommandTimer(opts);

    String s = "Pull Associations";
    /// KS_TBD CIMCLIOutput::displayInstances(opts,instances,s);
    CIMCLIOutput::displayInstances(opts,instances);
    return(CIMCLI_RTN_CODE_OK);
}

int pullQueryInstances(Options& opts)
{
    // Currently we do not allow the query return class in OpenPegasus.
    Boolean returnQueryResultClass = false;

    if (opts.verboseTest)
    {
        cout << "OpenQueryInstances "
            << "Namespace = " << opts.nameSpace
            << ", queryLanguage = " << opts.queryLanguage
            << ", query = " << opts.query
            << ", returnQueryResultClass = "
            << boolToString(returnQueryResultClass)
            << _displayPullCommonParam(opts)
            << endl;
    }

    if (opts.verboseTest)
    {
        cout << "PullQueryInstances "
            << "Namespace = " << opts.nameSpace
            << ", queryLanguage = " << opts.queryLanguage
            << ", query = " << opts.query
            << endl;
    }

    Array<CIMInstance> instances;

    _startCommandTimer(opts);

    Boolean endOfSequence = false;

    // insure that we do not request more than limit on open
    if (opts.maxObjectsToReceive != 0)
    {
        // if ask for more than max we want, reset max count.
        if (opts.maxObjectCount > opts.maxObjectsToReceive)
        {
            opts.maxObjectCount = opts.maxObjectsToReceive;
        }
    }

    CIMEnumerationContext enumerationContext;
    CIMClass queryResultClass;

    instances = opts.client.openQueryInstances(
        enumerationContext,
        endOfSequence,
        opts.nameSpace,
        opts.queryLanguage,
        opts.query,
        queryResultClass,
        returnQueryResultClass,
        opts.pullOperationTimeout,
        opts.continueOnError,
        opts.maxObjectCount);

    _testRcvTooManyElements(opts.maxObjectCount,
        instances.size(), opts.verboseTest);

    if (!endOfSequence)
    {
        // The false argument indicates that we pull instances only, no path
        if (!_pullInstancesWithPath(opts, instances, enumerationContext, false))
        {
            // some error return from the pull loop.
            cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.PULL_RESP_ERR",
                "Pull Loop returned error, for example, too many responses."));
        }
    }


    _stopCommandTimer(opts);

    // The false indicates that we do not display the path component
    CIMCLIOutput::displayInstances(opts, instances, false);

    return(CIMCLI_RTN_CODE_OK);
}
//KS_PULL_END

/************************** executeQuery  ***************************/
/*
    Execute the client ExecQuery function. The parameters are:
    namespace, queryLanguage, and the query string
*/
int execQuery(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "ExecQuery "
            << "Namespace = " << opts.nameSpace
            << ", queryLanguage = " << opts.queryLanguage
            << ", query = " << opts.query
            << endl;
    }

    Array<CIMObject> objects;

    _startCommandTimer(opts);

    objects = opts.client.execQuery(opts.nameSpace,
                                opts.queryLanguage,
                                opts.query );

    _stopCommandTimer(opts);

    String s = "instances of class";
    CIMCLIOutput::displayObjects(opts, objects, s);

    return CIMCLI_RTN_CODE_OK;
}

/* local function to get the object path for the target defined by input.
    The path is built or acquired from information provided by input as
    follows:
    The InstanceName/Class parameter is special in that it has several options:
       - objectPath(Class plus keys) - Use the object path directly
       - Class only (No keys) -  cimcli uses interactive mode to list instances
         of class for selection
       - Class only in objectName plus entries in extra parameters - cimcli
         builds instance from extra parameters and then builds path from
         instance to retrieve.
    This function is used by all of the action functions that require
    cimObjectPath input BUT do not utilize the -i (interactive option) to
    make the decision.
    @param opts -  Input arg, options specified by the user
    @param thisPath - Output arg,  CIMObjectPath which either contains the path
    to be used or an empty CIMObjectPath if there is no path for the operation.
    @return Returns true if CIMObjectPath returned have keybindings else false.
*/

Boolean _getObjectPath(Options& opts, CIMObjectPath &thisPath)
{
    // try to build path from input objectName property
    // Uses try block because this input generates an exception based on
    // input syntax and we can use this to more clearly tell the user
    // what the issue is than the text of the standard malformed object
    // exception

    thisPath = opts.getTargetObjectName();

    // If there are no keybindings and there are extra input parameters,
    // build path from input arguments. If there are no keybindings
    // and no extra parameters do the select instance.
    if (opts.targetObjectName.getKeyBindings().size() == 0)
    {
        if (opts.valueParams.size() > 1)
        {
            ObjectBuilder ob(
                opts.valueParams,
                opts.client,
                opts.nameSpace,
                opts.targetObjectName.getClassName(),
                CIMPropertyList(),
                opts.verboseTest);

            thisPath = ob.buildCIMObjectPath();
            if (opts.verboseTest && thisPath.getKeyBindings().size() == 0)
            {
                cout << "No valid object path defined. "
                     << thisPath.toString()
                     << endl;
            }
        }
        else  // no extra parameters.
        {
            // get the instance from a console request
            if (!_selectInstance(opts, opts.getTargetObjectNameClassName(),
                                thisPath))
            {
                thisPath = CIMObjectPath();
            }
        }
    }

    return thisPath.getKeyBindings().size() > 0;
}


/************************** deleteInstance  ***************************/
/*
    Execute the client operation deleteInstance with the parameters
    namespace and object or classname.  If only the classname is provided
    an interactive operation is executed and the user is presented with
    a list of instances in the namespace/class from which they can select
    an instance to delete.
*/
int deleteInstance(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "deleteInstance "
            << "Namespace = " << opts.nameSpace
            << ", ObjectName/ClassName = " << opts.getTargetObjectNameStr()
            << endl;
        _showValueParameters(opts);
    }

    // Build or get path based in info in opts. If function returns false
    // (valid object path not provided), return OK without executing
    // CIM Operation
    CIMObjectPath thisPath;
    if (_getObjectPath(opts, thisPath))
    {
        _startCommandTimer(opts);
        opts.client.deleteInstance(opts.nameSpace, thisPath);
        _stopCommandTimer(opts);
    }

    return CIMCLI_RTN_CODE_OK;
}


/***************************** getInstance  ******************************/
/*  Execute the CIMCLient getInstance function with the parameters provided.
    The majority of the parameters are a direct interpretation of the
    client getInstance input parameters
    The InstanceName/Class parameter is special in that it has several options:
       - objectPath form - Use the object path directly
       - Class only -  cimcli uses interactive mode to list instances of class
         for selection
       - Class only in objectName plus entries in extra parameters - cimcli
         builds instance from extra parameters and then builds path from
         instance to retrieve.
         FUTURE: Test if properties suppled include all key properties. At best
         we would issue a warning since we do not want to eliminate
         ability to make error calls
*/
int getInstance(Options& opts)
{
    // Resolve the IncludeQualifiers -iq vs -niq qualifiers default is true.
    _resolveIncludeQualifiers(opts, false);
    if (opts.verboseTest)
    {
        cout << "getInstance "
            << "Namespace = " << opts.nameSpace
            << ", InstanceName/class = " << opts.getTargetObjectNameStr()
            << ", localOnly = " << boolToString(opts.localOnly)
            << ", includeQualifiers = "
                << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin = "
                << boolToString(opts.includeClassOrigin)
            << ", PropertyList = " << opts.propertyList.toString()
            << endl;
        _showValueParameters(opts);
    }

    // Build or get path based in info in opts. If function returns false
    // (valid object path not provided), return OK without executing
    // CIM Operation
    CIMObjectPath thisPath;
    if (_getObjectPath(opts, thisPath))
    {
        _startCommandTimer(opts);
        CIMInstance cimInstance = opts.client.getInstance(opts.nameSpace,
            thisPath,
            opts.localOnly,
            opts.includeQualifiers,
            opts.includeClassOrigin,
            opts.propertyList);
        _stopCommandTimer(opts);

        CIMCLIOutput::displayInstance(opts, cimInstance);
    }

    return CIMCLI_RTN_CODE_OK;
}

/***************************** createInstance  ******************************/
/****
    This action function executes a create instance.

    The CIM Client operation is:
        CIMObjectPath createInstance(
            const CIMNamespaceName& nameSpace,
            const CIMInstance& newInstance
        );

    The input parameters are the classname and the name/value pairs
    that are used to build properties of the instance.
***/
int createInstance(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "createInstance "
            << "Namespace = " << opts.nameSpace
            << ", ClassName = " << opts.className.getString()
            << endl;
        _showValueParameters(opts);
    }

    ObjectBuilder ob(opts.valueParams,
            opts.client,
            opts.nameSpace,
            opts.className,
            CIMPropertyList(),
            opts.verboseTest);

    // create the instance with the defined properties
    CIMInstance newInstance = ob.buildInstance(
        opts.includeQualifiers,
        opts.includeClassOrigin,
        CIMPropertyList());

    if (opts.verboseTest)
    {
        CIMCLIOutput::displayInstance(opts, newInstance);
    }

    _startCommandTimer(opts);

    CIMObjectPath rtnPath = opts.client.createInstance(opts.nameSpace,
                                                 newInstance);

    _stopCommandTimer(opts);

    // Check Output Format to print results
    String description = "Returned Path ";
    CIMCLIOutput::displayPath(opts, rtnPath, description);

    return CIMCLI_RTN_CODE_OK;
}


/***************************** testInstance  ******************************/
/*  Test the instance defined by the input parameters
    against the same instance in the target system.
    1. Get class from classname input
    2. Build the test instance from input parameters
    3. Build path from input and class
    4. getInstance from system using property list from test instance unless
       there is a list provided with the input.
    5. Compare properties in testInstance against the
       same named properties in returned instance
    6 If there is an error, display differences (if verbose set)
    returns 0 if all properties are the same. Else returns
    CIMCLI_RTN_CODE_ERR_COMPARE_FAILED as an error
    NOTE: Only does exact property compare. Today this function DOES NOT
    have ability to do logical compares such as < >, etc.  Also cannot
    test parameters against an input object name.  MUST BE class name
    on input.
*/
int testInstance(Options& opts)
{
    // Resolve the IncludeQualifiers -iq vs -niq qualifiers default is true.
    _resolveIncludeQualifiers(opts, false);

    if (opts.verboseTest)
    {
        cout << "testInstance "
            << "Namespace = " << opts.nameSpace
            << ", InstanceName/ClassName = " << opts.getTargetObjectNameStr()
            << ", includeQualifiers = " << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin = "
                << boolToString(opts.includeClassOrigin)
            << ", PropertyList = " << opts.propertyList.toString()
            << endl;
        _showValueParameters(opts);
    }

    // build the instance from all input properties. It is allowable
    // to build an instance with no properties.
    ObjectBuilder ob(
        opts.valueParams,
        opts.client,
        opts.nameSpace,
        opts.getTargetObjectNameClassName(),
        CIMPropertyList(),
        opts.verboseTest);

    CIMInstance testInstance = ob.buildInstance(
        opts.includeQualifiers,
        opts.includeClassOrigin,
        CIMPropertyList());

    // If the objectName keybindings are zero create the path from the
    // built instance unless the interactive bit is set. Then ask the
    // select from existing instances.
    // Else use the path built above from the objectName

    if (opts.targetObjectNameClassOnly())
    {
        if (!_conditionalSelectInstance(opts, opts.targetObjectName) ||
            !opts.interactive)
        {
            CIMClass thisClass =
            opts.client.getClass(opts.nameSpace,
                                 opts.getTargetObjectNameClassName(),
                                 false,true,true,CIMPropertyList());
            opts.targetObjectName = testInstance.buildPath(
                thisClass);
        }
    }

    // If there is no input property list substitute a list created from
    // the test instance. This means we acquire only the properties that were
    // defined on input as part of the test instance. Note that this may
    // not work since not all providers honor the propertylist but we test for
    // correct response later.
    if (opts.propertyList.size() == 0)
    {
        opts.propertyList = _buildPropertyList(testInstance);
    }

    _startCommandTimer(opts);

    CIMInstance rtndInstance = opts.client.getInstance(opts.nameSpace,
                                        opts.targetObjectName,
                                        opts.localOnly,
                                        opts.includeQualifiers,
                                        opts.includeClassOrigin,
                                        opts.propertyList);

    // Compare the property count of the request and response.
    // Put out a warning if they do not have the same property count.
    // If they do not match filter the response so that we actually
    // test the properties defined as of interest by the parameters in
    // the request. The warning is simply a flag for the user.
    if (rtndInstance.getPropertyCount() != opts.propertyList.size())
    {
        cimcliMsg::errmsg(MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.DIFF_PROP_RTND_ERR",
            "WARNING: Response returned different property set than requested."
            "\nRequested = $0"
            "\nReturned = $1"
            "\nContinuing and testing against requested property list",
            opts.propertyList.toString(),
            _buildPropertyList(rtndInstance).toString() ));
        //#N substitution {0} is a string with the classname in error
        //#N @version 2.14
        //#P 51
        //#T DIFF_PROP_RTND_ERR
        //#S Warning: Response returned different property set than requested.
        //#S \nRequested = {0}
        //#S \nReturned = {1}
        //#S \nContinuing and testing against requested property list.

        rtndInstance.instanceFilter(opts.includeQualifiers,
            opts.includeClassOrigin,
            opts.propertyList);
    }

    // Compare created and returned (possibly modified) instances
    Boolean detailedTest = false;

    // This test compares and if there are differences displays the difference
    // depending on opt.verbose.  It also conducts either a detailed test
    // or a value only test depending on the detailedTest parameter
    if (!_compareInstances(testInstance, rtndInstance, opts, detailedTest,
                           opts.verboseTest))
    {
        cerr << "Error: Test Instance differs from Server returned Instance."
            << "Rtn Code " << CIMCLI_RTN_CODE_ERR_COMPARE_FAILED << endl;

        // optional display of all the instances if you really have problems
        // finding differences.
        if (opts.verboseTest && opts.debug)
        {
            cout << "Test Instance =" << endl;
            CIMCLIOutput::displayInstance(opts, testInstance);
            cout << "Returned Instance =" << endl;
            CIMCLIOutput::displayInstance(opts, rtndInstance);
        }
        return CIMCLI_RTN_CODE_ERR_COMPARE_FAILED;
    }
    else
        cout << "Test instance " << opts.targetObjectName.toString()
             << " OK" << endl;

    _stopCommandTimer(opts);

    return CIMCLI_RTN_CODE_OK;
}


/***************************** modifyInstance  ******************************/
/****
    The function executes the CIM Operation modify instance.
    CIMObjectPath modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers = true,
        const CIMPropertyList& propertyList = CIMPropertyList());

    NOTE: We do not support the includequalifiers option so this
    is always set to false.
    This command is similar to create instance but more complex in that
    it is based on an existing instance name and the creation of a
    possibly incomplete instance.

    Therefore, it takes as input an object name which may be just a
    class name and the extra parameters to build an instance.

    This operation differes from the create instance in that the CIM Operation
    input requires a namedInstance rather than simply an instance.  It is the
    name that is used to identify the instance to be modified.  Therefore
    the operation must allow for the name component of the instance to
    be created independently from the input instance

    If the input object name is just a class name, the parameters are used to
    build an instance which MUST include all of the key properties.  Then the
    instance is used to build a path which becomes the path in the
    input instance.

    If the input includes the keys component of a
    cim object path, the logic uses that as the instance name and the
    extra parameters to build the instance.

    If only the classname is provided or not provide any key property,
    an interactive operation is executed
***/

int modifyInstance(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "modifyInstance "
            << "Namespace = " << opts.nameSpace
            << ", InstanceName/ClassName = " << opts.getTargetObjectNameStr()
            << ", Property List = " << opts.propertyList.toString()
            << endl;
        _showValueParameters(opts);
    }

    // Determine if the input form was with an object path or with individual
    // properties listed including the key properties or specifically
    // interactive (-i) where an enumerateinstance names will be used tl
    // determine object path.
    Array<CIMKeyBinding> keys = opts.getTargetObjectName().getKeyBindings();
    if (!opts.targetObjectNameClassOnly())
    {
        mapKeyBindingsToInputParameters(opts);
    }
    else
    {
        if (opts.interactive)
        {
            if (_conditionalSelectInstance(opts, opts.targetObjectName))
            {
                mapKeyBindingsToInputParameters(opts);
            }
            else
            {
                cimcliMsg::exit(CIMCLI_INPUT_ERR, MessageLoaderParms(
                    "Clients.cimcli.CIMCLIClient.NO_PATH_ERR",
                    "No path for instance set."));
                //#T NO_PATH_ERR
                //#S No path for instance set.
            }
        }
    }

    // build the instance from all input properties. It is allowable
    // to build an instance with no properties.
    ObjectBuilder ob(
        opts.valueParams,
        opts.client,
        opts.nameSpace,
        opts.getTargetObjectNameClassName(),
        CIMPropertyList(),
        opts.verboseTest);

    CIMInstance modifiedInstance = ob.buildInstance(
        opts.includeQualifiers,
        opts.includeClassOrigin,
        CIMPropertyList());

    opts.targetObjectName = ob.buildCIMObjectPath();

    // put the path into the modifiedInstance
    modifiedInstance.setPath(opts.targetObjectName);

    _startCommandTimer(opts);

    opts.client.modifyInstance(opts.nameSpace,
                         modifiedInstance,
                         false,
                         opts.propertyList);

    // Need to put values into the parameters.
    _stopCommandTimer(opts);

    CIMCLIOutput::display(opts, "modified");

    return CIMCLI_RTN_CODE_OK;
}

/***************************** enumerateClassNames  **************************/
/*
    Execute the client operation enumerateClassNames with the input parameters
    Namespace, ClassName, and the DeepInheritance option.
*/
int enumerateClassNames(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "EnumerateClasseNames "
            << "Namespace= " << opts.nameSpace
            << ", Class= " << opts.className.getString()
            << ", deepInheritance= " << boolToString(opts.deepInheritance)
            << endl;
    }
    Array<CIMName> classNames;

    _startCommandTimer(opts);

    classNames = opts.client.enumerateClassNames(opts.nameSpace,
                                        opts.className,
                                        opts.deepInheritance);

    _stopCommandTimer(opts);

    CIMCLIOutput::displayClassNames(opts, classNames);

    return CIMCLI_RTN_CODE_OK;
}

/***************************** enumerateClasses  ******************************/
/*
    Execute the client operation enumerateClasses with the input parameters
    Namespace, ClassName, and the DeepInheritance localOnly,
    includeQualifiers, and includeClassOrigin option.
*/
int enumerateClasses(Options& opts)
{
    // Resolve the IncludeQualifiers -iq vs -niq qualifiers default is true.
    _resolveIncludeQualifiers(opts, true);
    if (opts.verboseTest)
    {
        cout << "EnumerateClasses "
            << "Namespace= " << opts.nameSpace
            << ", Class= " << opts.className.getString()
            << ", deepInheritance= " << boolToString(opts.deepInheritance)
            << ", localOnly= " << boolToString(opts.localOnly)
            << ", includeQualifiers= "
                << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin= "
                << boolToString(opts.includeClassOrigin)
            << endl;
    }

    _startCommandTimer(opts);

    Array<CIMClass> classes = opts.client.enumerateClasses(opts.nameSpace,
                                        opts.className,
                                        opts.deepInheritance,
                                        opts.localOnly,
                                        opts.includeQualifiers,
                                        opts.includeClassOrigin);

    _stopCommandTimer(opts);

    CIMCLIOutput::displayClasses(opts, classes);

    return CIMCLI_RTN_CODE_OK;
}

/***************************** deleteClass  ******************************/
/*
    Execute the client operation deleteClass with the input parameters
    Namespace and ClassName.
*/
int deleteClass(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "deleteClasses "
            << "Namespace = " << opts.nameSpace
            << ", Class = " << opts.className.getString()
            << endl;
    }

    _startCommandTimer(opts);

    opts.client.deleteClass(opts.nameSpace, opts.className);

    _stopCommandTimer(opts);

    return CIMCLI_RTN_CODE_OK;
}

/***************************** getClass  ******************************/
/*
    Execute the client operation getClass with the input parameters
    Namespace and ClassName and the options deepInheritance, localOnly,
    includeQualifiers, includeClassOrigin, and a possible propertyList
*/
int getClass(Options& opts)
{
    // Resolve the IncludeQualifiers -iq vs -niq qualifiers default is true.
    _resolveIncludeQualifiers(opts, true);

    if (opts.verboseTest)
    {
        cout << "getClass "
            << "Namespace= " << opts.nameSpace
            << ", Class= " << opts.className.getString()
            << ", deepInheritance= " << boolToString(opts.deepInheritance)
            << ", localOnly= " << boolToString(opts.localOnly)
            << ", includeQualifiers= " << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin= " << boolToString(opts.includeClassOrigin)
            << ", PropertyList= " << opts.propertyList.toString()
            << endl;
    }

    _startCommandTimer(opts);

    CIMClass cimClass = opts.client.getClass(opts.nameSpace,
                                        opts.className,
                                        opts.localOnly,
                                        opts.includeQualifiers,
                                        opts.includeClassOrigin,
                                        opts.propertyList);

    _stopCommandTimer(opts);

    CIMCLIOutput::displayClass(opts, cimClass);

    return CIMCLI_RTN_CODE_OK;
}

/***************************** getProperty  ******************************/
/*
    Execute the client operation getProperty with the input parameters
    Namespace, InstanceName, and propertyName
*/
int getProperty(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "getProperty "
            << "Namespace= " << opts.nameSpace
            << ", InstanceName= " << opts.getTargetObjectNameStr()
            << ", propertyName= " << opts.propertyName
            << endl;
        _showValueParameters(opts);
    }

    // Build or get path based in info in opts. If function returns false
    // (valid object path not provided), return OK without executing
    // CIM Operation
    CIMObjectPath thisPath;

    if (_getObjectPath(opts, thisPath))
    {
        CIMValue cimValue;
        _startCommandTimer(opts);
        cimValue = opts.client.getProperty(
            opts.nameSpace,
            thisPath,
            opts.propertyName);
        _stopCommandTimer(opts);
        if (opts.summary)
        {
            if (opts.time)
            {
                cout << opts.saveElapsedTime << endl;
            }
        }
        else
        {
            cout << opts.propertyName << " = " << cimValue.toString() << endl;
        }
    }

    return CIMCLI_RTN_CODE_OK;
}

/***************************** setProperty  ******************************/
/*
    Execute the client operation setProperty with the input parameters
    Namespace, InstanceName, propertyName and new property value
*/
int setProperty(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "setProperty "
            << "Namespace= " << opts.nameSpace
            << ", InstanceName= " << opts.getTargetObjectNameStr()
            << ", propertyName= " << opts.propertyName
            << ", newValue= " << opts.newValue
            << endl;
        _showValueParameters(opts);
    }

    // Build or get path based in info in opts. If function returns false
    // (valid object path not provided), return OK without executing
    // CIM Operation
    CIMObjectPath thisPath;
    if (_getObjectPath(opts, thisPath))
    {
        ObjectBuilder ob(opts.valueParams,
                opts.client,
                opts.nameSpace,
                thisPath.getClassName(),
                CIMPropertyList(),
                opts.verboseTest);

        CIMValue cimValue =
            ob.buildPropertyValue(opts.propertyName,opts.newValue);

        _startCommandTimer(opts);

        opts.client.setProperty( opts.nameSpace,
                                       thisPath,
                                       opts.propertyName,
                                       cimValue);

        _stopCommandTimer(opts);
    }

    return CIMCLI_RTN_CODE_OK;
}

/***************************** getQualifier  ******************************/
/*
    Execute the client operation getQualifier with the input parameters
    Namespace and qualifierName.
*/
int getQualifier(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "getQualifier "
            << "Namespace= " << opts.nameSpace
            << ", Qualifier= " << opts.qualifierName
            << endl;
    }

    CIMQualifierDecl cimQualifierDecl;

    _startCommandTimer(opts);

    cimQualifierDecl = opts.client.getQualifier( opts.nameSpace,
                                   opts.qualifierName);

    _stopCommandTimer(opts);

    // display received qualifier

    CIMCLIOutput::displayQualDecl(opts, cimQualifierDecl);

    return CIMCLI_RTN_CODE_OK;
}

int setQualifier(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "setQualifiers "
            << "Namespace= " << opts.nameSpace
            // KS add the qualifier decl here.
            << endl;
    }

    _startCommandTimer(opts);

    opts.client.setQualifier(opts.nameSpace, opts.qualifierDeclaration);

    _stopCommandTimer(opts);

    return CIMCLI_RTN_CODE_OK;
}

/***************************** deleteQualifier  ******************************/
/*
    Execute the client operation deleteQualifier with the input parameters
    Namespace and qualifierName.
*/
int deleteQualifier(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "deleteQualifiers "
            << "Namespace= " << opts.nameSpace
            << " QualifierName= " << opts.qualifierName
            << endl;
    }

    _startCommandTimer(opts);

    opts.client.deleteQualifier(opts.nameSpace, opts.qualifierName);

    _stopCommandTimer(opts);

    return CIMCLI_RTN_CODE_OK;
}

/***************************** enumerateQualifiers  **************************/
/*
    Execute the client operation enumerateQualifiers with the input parameters
    Namespace.
*/
int enumerateQualifiers(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "enumerateQualifiers "
            << "Namespace= " << opts.nameSpace
            << endl;
    }

    Array<CIMQualifierDecl> qualifierDecls;

    _startCommandTimer(opts);

    qualifierDecls = opts.client.enumerateQualifiers(opts.nameSpace);

    _stopCommandTimer(opts);

    CIMCLIOutput::displayQualDecls(opts, qualifierDecls);

    return CIMCLI_RTN_CODE_OK;
}


/***************************** referenceNames  ******************************/
/*
    Execute CIM Operation referencenames.  The signature of the
    client CIM Operation is:

    Array<CIMObjectPath> referenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String()
*/
int referenceNames(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "ReferenceNames "
            << "Namespace= " << opts.nameSpace
            << ", ObjectPath= " << opts.getTargetObjectNameStr()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << endl;
    }
    // do conditional select of instance if params properly set.

    CIMObjectPath thisObjectPath(opts.getTargetObjectName());

    if (!_conditionalSelectInstance(opts, thisObjectPath))
    {
        return CIMCLI_RTN_CODE_OK;
    }

    _startCommandTimer(opts);

    Array<CIMObjectPath> referenceNames =
        opts.client.referenceNames( opts.nameSpace,
                               thisObjectPath,
                               opts.resultClass,
                               opts.role);

    _stopCommandTimer(opts);

    String s = "referenceNames";
    opts.className = thisObjectPath.getClassName();
    CIMCLIOutput::displayPaths(opts, referenceNames, s);

    return CIMCLI_RTN_CODE_OK;
}


/***************************** references  ******************************/
/****
  get references for the target input object using the client references
  operation. This operation uses the interactive option to determine if
  the input target is to be selected interactively.  This is required
  because the reference operation can have either a class or path as
  input (returns either classes or instances depending on this input).
  The interactive option requests that the instance interactive selector
  be used to allow the user to select instances for a particular class
  input.
     Array<CIMObject> references(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );
*/
int references(Options& opts)
{
    // Resolve the IncludeQualifiers -iq vs -niq qualifiers default is true.
    _resolveIncludeQualifiers(opts, false);
    if (opts.verboseTest)
    {
        cout << "References "
            << "Namespace= " << opts.nameSpace
            << ", ObjectName = " << opts.getTargetObjectNameStr()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << ", includeQualifiers= " << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin= " << boolToString(opts.includeClassOrigin)
            << ", CIMPropertyList= " << opts.propertyList.toString()
            << endl;
    }

    // do conditional select of instance if params properly set.
    CIMObjectPath thisObjectPath(opts.getTargetObjectName());

    if (!_conditionalSelectInstance(opts, thisObjectPath))
    {
        return CIMCLI_RTN_CODE_OK;
    }

    _startCommandTimer(opts);

    Array<CIMObject> objects =
        opts.client.references(  opts.nameSpace,
                            thisObjectPath,
                            opts.resultClass,
                            opts.role,
                            opts.includeQualifiers,
                            opts.includeClassOrigin,
                            opts.propertyList);

    _stopCommandTimer(opts);

    String s = "references";
    CIMCLIOutput::displayObjects(opts,objects,s);

    return CIMCLI_RTN_CODE_OK;
}


/***************************** associatorNames  ******************************/
/*
    Use the client associatorNames operation to return associated classes
    or instances for the target inputs. Note that this operation uses the
    interactive option.

    Array<CIMObjectPath> associatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY
    );
*/
int associatorNames(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "associatorNames "
            << "Namespace= " << opts.nameSpace
            << ", ObjectName= " << opts.getTargetObjectNameStr()
            << ", assocClass= " << opts.assocClass.getString()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << ", resultRole= " << opts.resultRole
            << endl;
    }

    // do conditional select of instance if params properly set.
    CIMObjectPath thisObjectPath(opts.getTargetObjectName());

    if (!_conditionalSelectInstance(opts, thisObjectPath))
    {
        return CIMCLI_RTN_CODE_OK;
    }

    _startCommandTimer(opts);

    Array<CIMObjectPath> associatorNames =
        opts.client.associatorNames( opts.nameSpace,
                                    thisObjectPath,
                                    opts.assocClass,
                                    opts.resultClass,
                                    opts.role,
                                    opts.resultRole);

    _stopCommandTimer(opts);

    String s = "associatorNames";
    opts.className = thisObjectPath.getClassName();
    CIMCLIOutput::displayPaths(opts, associatorNames, s);

    return CIMCLI_RTN_CODE_OK;
}


/***************************** associators  ******************************/
/*
    Execute the CIM Client Operation associators. Note that this function
    uses the interactive operation to allow the user to select the
    object to be the target objecName. The signature of
    the function is:

    Array<CIMObject> associators(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );
 */
int associators(Options& opts)
{
    // Resolve the IncludeQualifiers -iq vs -niq qualifiers default is true.
    _resolveIncludeQualifiers(opts, false);

    if (opts.verboseTest)
    {
        cout << "Associators "
            << "Namespace= " << opts.nameSpace
            << ", Object= " << opts.getTargetObjectNameStr()
            << ", assocClass= " << opts.assocClass.getString()
            << ", resultClass= " << opts.resultClass.getString()
            << ", role= " << opts.role
            << ", resultRole= " << opts.resultRole
            << ", includeQualifiers= " << boolToString(opts.includeQualifiers)
            << ", includeClassOrigin= " << boolToString(opts.includeClassOrigin)
            << ", propertyList= " << opts.propertyList.toString()
            << endl;
    }

    // do conditional select of instance if params properly set.
    CIMObjectPath thisObjectPath(opts.getTargetObjectName());

    if (!_conditionalSelectInstance(opts, thisObjectPath))
    {
        return CIMCLI_RTN_CODE_OK;
    }

    _startCommandTimer(opts);

    Array<CIMObject> objects =
        opts.client.associators( opts.nameSpace,
                            thisObjectPath,
                            opts.assocClass,
                            opts.resultClass,
                            opts.role,
                            opts.resultRole,
                            opts.includeQualifiers,
                            opts.includeClassOrigin,
                            opts.propertyList);

    _stopCommandTimer(opts);

    String s = "associators";
    CIMCLIOutput::displayObjects(opts,objects,s);

    return CIMCLI_RTN_CODE_OK;
}

/***************************** invokeMethod  ******************************/
/*
    CIMValue invokeMethod(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters
*/
/***************************** invokeMethod  ******************************/
 int invokeMethod(Options& opts)
 {
     {
         if (opts.verboseTest)
         {
             cout << "invokeMethod"
                 << " Namespace= " << opts.nameSpace
                 << ", ObjectName= " << opts.getTargetObjectNameStr()
                 << ", methodName= " << opts.methodName.getString()
                 << ", inParams Count= " << opts.inParams.size()
                 << endl;

             CIMCLIOutput::displayParamValues(opts, opts.inParams);

             _showValueParameters(opts);
        }

        ObjectBuilder ob(
            opts.valueParams,
            opts.client,
            opts.nameSpace,
            opts.getTargetObjectNameClassName(),
            CIMPropertyList(),
            opts.verboseTest);

        Array<CIMParamValue> params = ob.buildMethodParameters(opts.methodName);

         // Create array for output parameters
        CIMValue retValue;
        Array<CIMParamValue> outParams;

        _startCommandTimer(opts);

        // Call invoke method with the parameters
        retValue = opts.client.invokeMethod(opts.nameSpace,
                                            opts.getTargetObjectName(),
                                            opts.methodName,
                                            params,
                                            outParams);

        _stopCommandTimer(opts);

        // Display the return value CIMValue
        cout << "Return Value= ";

        CIMCLIOutput::displayValue(opts, retValue);
        cout << endl;

        // Display any outparms

        CIMCLIOutput::displayParamValues(opts, outParams);
     }

    return CIMCLI_RTN_CODE_OK;
 }

/************************ enumerateNamespace names **********************/
/* Enumerate the Namespace names.  This function is based on using either
    the CIM_Namespace class or if this does not exist the
    __Namespace class and either returns all namespaces or simply the ones
    starting at the namespace input as the namespace variable.
    It assumes that the input classname is __Namespace.
*/

int enumerateNamespaceNames(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "EnumerateNamespaces "
            << "Namespace= " << opts.nameSpace
            << ", Class= " << opts.className.getString()
            << endl;
    }

    _startCommandTimer(opts);

    Array<CIMNamespaceName> ns = _getNameSpaceNames(opts);

    _stopCommandTimer(opts);

    CIMCLIOutput::displayNamespaceNames(opts, ns);

    return CIMCLI_RTN_CODE_OK;
}


/************************ setObjectManagerStatistics **********************/
/*
    Set the statistics on/off flag in the objectmanager Class.  This should
    be considered temporary code pending a more general solution for
    setting many of these attributes. Do not count on this being in
    future versions of cimcli.
    DEPRECATED - This should be replaced with a special function but since
    the whole use of the statistics setting functions is in question in
    the DMTF we left it for now.
*/
int setObjectManagerStatistics(Options& opts, Boolean newState,
                                   Boolean& stateAfterMod)
{
    CIMName gathStatName ("GatherStatisticalData");

    Array<CIMInstance> instancesObjectManager;
    CIMInstance instObjectManager;
    Uint32 prop_num;
    Array<CIMName> plA;
    plA.append(gathStatName);
    CIMPropertyList statPropertyList(plA);

    // Create property list that represents correct request
    // get instance.  Get only the gatherstatitistics property
    instancesObjectManager  =
        opts.client.enumerateInstances(PEGASUS_NAMESPACENAME_INTEROP,
            "CIM_ObjectManager",
            true, false, false, false, statPropertyList);
    PEGASUS_TEST_ASSERT(instancesObjectManager.size() == 1);
    instObjectManager = instancesObjectManager[0];

    // set correct path into instance
    instObjectManager.setPath(instancesObjectManager[0].getPath());

    prop_num = instObjectManager.findProperty(gathStatName);
    PEGASUS_TEST_ASSERT(prop_num != PEG_NOT_FOUND);

    instObjectManager.getProperty(prop_num).setValue(CIMValue(newState));

    opts.client.modifyInstance(PEGASUS_NAMESPACENAME_INTEROP, instObjectManager,
         false, statPropertyList);

    // get updated instance to confirm change made
    CIMInstance updatedInstance =
        opts.client.getInstance(PEGASUS_NAMESPACENAME_INTEROP,
        instObjectManager.getPath(),
        false, false, false, statPropertyList);

    prop_num = updatedInstance.findProperty(gathStatName);
    PEGASUS_TEST_ASSERT(prop_num != PEG_NOT_FOUND);
    CIMProperty p = updatedInstance.getProperty(prop_num);
    CIMValue v = p.getValue();
    v.get(stateAfterMod);

    // It is our intention to deprecate this function so did not
    // internationalize
    cout << "Updated Status= " << boolToString(stateAfterMod) << endl;

    if (stateAfterMod != newState)
    {
        cerr << "Error: State change error. Expected: "
            << boolToString(newState)
            << " Rcvd: " << boolToString(stateAfterMod) << endl;
    }

    return CIMCLI_RTN_CODE_OK;
}

/*****************************************************************************
*
*        Show tree view of classes
*        Options include:
*           ClassName - Starting place in the hiearchy for the display
*           -nlo - Reverse and do the superclasses of this class as tree
*           -di  - Append info on reference properties to association classes
*
******************************************************************************/

/*
    Return an array of CIMName containing the property names of any
    Reference properties in the Class
*/

Array<CIMName> getAssocRefs(const CIMClass& c)
{
    Array<CIMName> rtn;
    if(c.isAssociation())
    {
        for (Uint32 i = 0; i < c.getPropertyCount(); i++)
        {
            CIMConstProperty p = c.getProperty(i);
            if (p.getType() == CIMTYPE_REFERENCE)
            {
                  rtn.append(p.getReferenceClassName());
            }
        }
    }
    return rtn;
}

// Structure defines a single class and subclass set
struct classTreeEntry
{
    CIMName _class;
    Array<CIMName> _subclasses;

    Array<CIMName> _assocRefs;
    Boolean _isAssociation;

    classTreeEntry(){}
    classTreeEntry(const CIMName& className, Array<CIMName>& subclasses,
        Options& opts)
        : _class(className), _subclasses(subclasses), _isAssociation(false)
    {
        // Null CIMName indicates top level request and there is no
        // classname. Assumes this is not an association
        if (!className.isNull())
        {
            CIMClass c = opts.client.getClass(opts.nameSpace, className,
                true, true);
            if (opts.deepInheritance)
            {
                _assocRefs = getAssocRefs(c);
            }
            _isAssociation = c.isAssociation();
        }
    }

    classTreeEntry(const CIMClass& c, Array<CIMName>& subclasses, Options& opts)
    {
        _class = c.getClassName();
        _subclasses = subclasses;
        if (opts.deepInheritance)
        {
            _assocRefs = getAssocRefs(c);
        }
        _isAssociation = c.isAssociation();
    }

    // Diagnostic Display of a single classTreeEntry struct
    void show(Uint32 i)
    {
        cout << "Entry " << i << " class= "
             << _class.getString()
             << "  subclassesCnt=" << _subclasses.size() << " "
             << " subclasses=" << _toString(_subclasses)
             << " assocRefs=" << _toString(_assocRefs) << " "
             << " isAssociation=" << boolToString(_isAssociation)
             << endl;
    }
};

/*
    Class defining the container for classTreeEntry items.  As classes are
    analyzed, classTreeEntry items are put into this container and at the
    end the displayTree displays the complete tree.
*/
class classTreeList
{
public:
    classTreeList() {}

    // Constructor for the class tree container
    classTreeList(Options& opts)
    {
        _topLevelClassName = opts.className;
        _opts = &opts;
    }

    // find a single entry with name. It is a programming error if
    // the name does not exist.
    classTreeEntry findEntry(CIMName& name)
    {
        for (Uint32 i = 0 ; i < _classTreeList.size() ; i++)
        {
            if (_classTreeList[i]._class == name)
            {
                return _classTreeList[i];
            }
        }

        cimcliMsg::exit(CIMCLI_INTERNAL_ERR, MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.CLASS_NAME_NOT_FOUND_ERR",
            "Class $0 not found in classTreeArray",
            (const char *)name.getString().getCString()));
        //#N substitution {0} is a string with the class name in error
        //#N @version 2.14
        //#P 52
        //#T CLASS_NAME_NOT_FOUND_ERR
        //#S Class {0} not found in classTreeArray

        PEGASUS_UNREACHABLE(return classTreeEntry();)
    }

    // Display a single tree entry
    Boolean displayTreeEntry(CIMName& className, Uint32 level, bool isLast);

    // Add an entry to the list
    void add(classTreeEntry& entry)
    {
        _classTreeList.append(entry);
    }

    // create a new entry in the list with just the classname
    void add(const CIMName& className, Array<CIMName>& subclasses)
    {
        classTreeEntry entry(className, subclasses, *_opts);
        add(entry);
    }

    // Create a new entry in the list with the provider class
    void add(const CIMClass& c, Array<CIMName>& subclasses)
    {
        classTreeEntry entry(c, subclasses, *_opts);
        add(entry);
    }

    void displayTree()
    {
        Uint32 level = 0;
        displayTreeEntry(_topLevelClassName,level, false);
    }

    // Set the starting point of the tree
    void setTopLevel(const CIMName& name)
    {
        _topLevelClassName = name;
    }
    String generateLevelGraphic(Uint32 index, bool last);

private:
    Array<classTreeEntry> _classTreeList;
    CIMName _topLevelClassName;
    Options * _opts;
};

/*
    Create String representing level.  This is a string that uses
    |, _, + to show the place in the hiearchy for a class.
    Asterick at end indicates an association class
    Ex.
    CIM_System
    |__CIM_ApplicationSystem
    |  |__CIM_J2eeServer
    |  |__CIM_DatabaseSystem
    |  |__CIM_J2eeApplication
*/

String classTreeList::generateLevelGraphic(Uint32 index, bool last)
{
    // Array controls use of the | mark. Do not want it set if on last
    // class for this level.
    static Array<Boolean> lastForLevel;
    static Uint32 prevLevel = 0;

    // append new entry if array too small
    if (lastForLevel.size() <= index)
    {
        lastForLevel.append(false);
    }

    Uint32 indent = 2;
    Uint32 gap = indent-1;
    String rtn;

    for (Uint32 i = 0 ; i < index ; i++)
    {
        // set vertical line (becomes blank after last entry)
        rtn.append((lastForLevel[i]) ? " " : "|");

        // Set horizontal line for last indent "_" or " "
        rtn.append(fillString(gap,((i == (index-1)) ? '_' : ' ')));
    }

    // if uplevel, reset vertical line on all lower levels.
    if (index < prevLevel)
    {
        for (Uint32 i = index ; i < lastForLevel.size() ; i++)
        {
            lastForLevel[i] = false;
        }
    }
    prevLevel = index;

    // Set indicator for level display of '|' or ' '
    if (last)
    {
        lastForLevel[index-1] = true;
    }

    return rtn;
}

/*
    Display a single classTreeEntry as a part of a tree
    @param array of classTreeEntry items where each item defines a
    single class/subclasses relationship
    @return true if subclasses exist
*/
bool classTreeList::displayTreeEntry(CIMName& className,
    Uint32 level, bool isLast)
{
    classTreeEntry entry = findEntry(className);

    String assocInfo;

    // if is association try to build the assoc info
    if (entry._isAssociation)
    {
        assocInfo.append("(");
        for (Uint32 i = 0; i < entry._assocRefs.size() ; i++)
        {
            if (i > 0)
            {
                assocInfo.append(" ");
            }
            assocInfo.append(entry._assocRefs[i].getString());
        }
        assocInfo.append(")");
    }

    // Display entry for this class including linking lines
    cout << generateLevelGraphic(level, isLast)
         << className.getString()
         << (entry._isAssociation? "*" : "")
         << assocInfo
         << endl;

    // return false if no more subclasses
    if (entry._subclasses.size() == 0)
    {
        return false;
    }

    // recall for subclasses for the next level
    level++;
    for (Uint32 i = 0; i < entry._subclasses.size() ; i++)
    {
        bool isLast = ((i+1 == entry._subclasses.size())? true: false);
        displayTreeEntry(entry._subclasses[i], level, isLast);
    }
    return true;
}

// CIMCLI operation to display class tree on the console.
int classTree(Options& opts)
{
    // Set the options for this output
    // This option shows reference property information for assciation
    // properties
    Boolean showAssocRefs = false;

    // flag to show superclasses vs subclasses based on localOnly option flag
    Boolean showSuperClasses = !opts.localOnly;

    // Today, we use deepInheritance as flag for showAssocRefs
    if (opts.deepInheritance)
    {
        showAssocRefs = true;
    }

    if (opts.verboseTest)
    {
        cout << "classTree. className=" << opts.className.getString()
             << " namespace=" << opts.nameSpace << endl
             << "Output (deepInheritance flag)"
             << (showAssocRefs? "with Assoc Info" : "No assoc Info") << endl
             << "Show  (localOnly flag)"
             << (!showSuperClasses? "SuperClasses" : "Subclasses") << endl;
    }

    classTreeList classTree(opts);

    if (showSuperClasses) // build tree of superclasses
    {
        // Must have a class name for tail of tree if showing superclasses
        if (opts.className.isNull())
        {
            cimcliMsg::exit(CIMCLI_INPUT_ERR, MessageLoaderParms(
                        "Clients.cimcli.CIMCLIClient.NEED_CLASS_NAME_ERR",
                        "Class name input required"));
//#T NEED_CLASS_NAME_ERR
//#S Class name input required
        }

        CIMName cn = opts.className;
        CIMClass c;
        CIMName prevClassName;

        // Loop to get superclasses starting at the input class
        do
        {
            c = opts.client.getClass(opts.nameSpace, cn);

            // Put this class into the classTree container.
            Array<CIMName> subclasses;
            if (!prevClassName.isNull())
            {
                subclasses.append(prevClassName);
            }

            classTree.add(c, subclasses);

            prevClassName = c.getClassName();
            cn = c.getSuperClassName();
        }
        while (!cn.isNull());

        // set the classname for the top of tree
        classTree.setTopLevel(c.getClassName());
    }

    else // build tree of subclasses
    {
        // get all classnames in namespace starting at the classname
        // provided with input
        Array<CIMName> classes = opts.client.enumerateClassNames(
            opts.nameSpace,
            opts.className,
            true);

        // get first subclass at current level with shallow request
        // This class is not part of the classes array
        Array<CIMName> subclasses = opts.client.enumerateClassNames(
            opts.nameSpace,
            opts.className,
            false);

        // put top level entry into array
        classTree.add(opts.className, subclasses);

        // for each classname, do shallow enumClassNames to get first level
        // subclasses
        for (size_t i = 0 ; i < classes.size() ; i++)
        {
            Array<CIMName> subclasses = opts.client.enumerateClassNames(
                opts.nameSpace,
                classes[i],
                false);

            // Create table entry for this class
            classTree.add(classes[i], subclasses);
        }
    }

    classTree.displayTree();

    return 0;
}

/************************ countInstances ***********************************
*
*    CountInstances
*    parameters:
*        optional - classname
*    Count the number of instances for each class that contains instances
*    in a namespace. This is different than enumerate instance in that it
*    actually reports the number of instances by class, not by class and
*    subclass.
*    If classname option provided it counts only for that class
*
***************************************************************************/
/*
    Determine if class defined by name is an association.  Does lookup
    of classname.
*/
Boolean isAssociation(Options& opts,const CIMName& className)
{
    CIMClass c = opts.client.getClass(opts.nameSpace, className,true, true);
    return(c.isAssociation());
}

// Structure to keep count of number of instances
struct instanceCounter
{
    String _name;
    Uint32 _count;

    instanceCounter(String& name) :
        _name(name),
        _count(1){}

    instanceCounter(){}

    const char* name()
    {
        return (const char*)_name.getCString();
    }
    void inc()
    {
        _count++;
    }
};

// find an instance in array of instanceCounter. Returns true if found
static bool _findInstance(Array<instanceCounter>& x, String& name, Uint32& pos)
{
    for (pos = 0; pos < x.size(); pos++)
    {
        if (x[pos]._name == name)
            return true;
    }
    return false;
}

//
// Sort the array of instanceCounter
//
// Compare  function for qsort. Compares two instanceCounter items
static int _compareInstanceCounterNames (const void* p1, const void* p2)
{
    const  instanceCounter * pa = (const instanceCounter*)p1;
    const  instanceCounter *pb =  (const instanceCounter*)p2;
    // use the string compare w or w/o  case sensitiveas as required
    return String::compareNoCase( pa->_name,   pb->_name);
}

// sort the array of instanceCounter items in an array
static void sort(Array<instanceCounter>& x)
{
    instanceCounter* data = (instanceCounter*)x.getData();

    Uint32 size = x.size();

    if (size > 1)
    {
        qsort((void*)data, size, sizeof(instanceCounter),
              _compareInstanceCounterNames);
    }
}

/*
    CIMCLI operation to count instances and display result
*/
int countInstances(Options& opts)
{
    if (opts.verboseTest)
    {
        cout << "countInstances. ";
        if (opts.className.isNull())
        {
            cout << "Count all instances in ";
        }
        else
        {
            cout << "Count Instance starting at "
                 << opts.className.getString()
                 << " in class hiearchy in ";
        }
        cout << opts.nameSpace << endl;
    }

    Array<CIMName> classNames;

    // Since this operation involves multiple calls to the server the
    // operation time does not mean much.
    _startCommandTimer(opts);

    // Get list of classes to explore.  This is just the input
    // classname if operationTarget is supplied. Otherwise it is all top
    // level class names.
    if (opts.className.isNull())
    {
        classNames = opts.client.enumerateClassNames(opts.nameSpace,
            opts.className,
            true);
    }
    else
    {
        classNames.append(opts.className);
    }

    Array<instanceCounter> instCtrArray;
    Uint32 maxNameLen = 0;

    // loop to enumerateInstanceNames for all classes in list
    for (Uint32 iClass = 0; iClass < classNames.size(); iClass++)
    {
        Array<CIMObjectPath> instanceNames;
        try
        {
            instanceNames = opts.client.enumerateInstanceNames(
                opts.nameSpace, classNames[iClass]);

            // verbose diagnostic to show actual
            if (instanceNames.size() != 0 && opts.verboseTest)
            {
                cout << "Class  " << classNames[iClass].getString()
                     << " enumeration returned " << instanceNames.size()
                     << " instances" << endl;
            }
        }

        // Continue after exceptions to the enumerateInstanceNames operation.
        // Results will probably enoreous but may give more info than
        // just stopping
        catch(CIMException& e)
        {
            cerr << "CIMException: " << classNames[iClass].getString() << " "
                 << e.getMessage() << " Code " << e.getCode()
                 << ". Continuing processing."<< endl;
            continue;
        }
        catch(Exception& e)
        {
            cerr << " Pegasus Exception: " << e.getMessage() << " "
                 << classNames[iClass].getString()
                 << ". Continuing processing."<< endl;
            continue;
        }

        // insert counter of instances into table
        for (Uint32 j = 0; j < instanceNames.size(); j++)
        {
            String name = instanceNames[j].getClassName().getString();

            maxNameLen = LOCAL_MAX(name.size(), maxNameLen);

            // Search table for this entry and if not there insert
            // otherwise, increment counter
            Uint32 pos;
            if (_findInstance(instCtrArray, name, pos))
            {
                instCtrArray[pos].inc();
            }

            else
            {
                instanceCounter item(name);
                instCtrArray.append(item);
            }
        }
    }
    _stopCommandTimer(opts);

    // Display result. There is no summary option for this output. The first
    // line summarizes the call parameters and number of classes with
    // instances
    cout << instCtrArray.size()
         << ((instCtrArray.size() == 1) ? " class" : " classes")
         << " with instances. " << opts.nameSpace;

    if (!opts.className.isNull())
    {
        cout << " Start " << opts.className.getString();
    }

    cout << " (*)association" << endl;

    // sort the array by name for display
    sort(instCtrArray);

    // Display complete array of
    for (Uint32 i = 0; i < instCtrArray.size(); i++)
    {
        // display Name with assoc indicator and count
        String name = instCtrArray[i]._name;
        cout << name << ((isAssociation(opts,
                            instCtrArray[i]._name))? "*" : " ")
             << fillString(maxNameLen + 1 - name.size())
             << instCtrArray[i]._count << endl;
    }

    return(0);
}

PEGASUS_NAMESPACE_END
// END_OF_FILE

