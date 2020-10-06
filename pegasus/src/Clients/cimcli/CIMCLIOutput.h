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

/*******************************************************************************

    Defines the display functions used by cimcli.  The are largely CIM entities
    and arrays of CIMEntities.  The output formats are controlled by
    data in the Options structure provider.

    The goal is that all output from the action operation in cimcli be
    routed through the methods in this display class.

*******************************************************************************/
#ifndef _CLI_OUTPUT_H
#define _CLI_OUTPUT_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/General/OptionManager.h>
#include <Clients/cimcli/Linkage.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

class CIMCLIOutput
{
public:

     /**
     * Display a single CIMInstance with output format determined by
     * parameters in opts.
     * @param opts Options structure with parameters for output
     *   display
     * @param path CIMInstance to be displayed.
     */
    static void PEGASUS_CLI_LINKAGE displayInstance(Options& opts,
        CIMInstance& instance);
    /**
     * Display an array of CIMInstances with output format dependent
     * on the parameters in opts.
     * @param opts Options structure with parameters for output
     * display
     * @param paths Array<CIMInstances> array containing the
     * CIMObjectOaths to be displayed. This array may be modified by
     * the output functions, specifically to sort the array if the
     * sort command line option is specified.
     * @param displayPathComponent bool. If true, displays the
     * path component.  Default = true
     */
    static void PEGASUS_CLI_LINKAGE displayInstances(Options& opts,
        Array<CIMInstance>& instances, bool displayPathComponent = true);


    /**
     * Display a single CIMClass with output format determined
     * by parameters in opts.
     * @param opts Options structure with parameters for output
     *   display
     * @param path CIMClass to be displayed.
     */
    static void PEGASUS_CLI_LINKAGE displayClass(Options& opts,
        const CIMClass& cimClass);
    /**
     * Display an array of CIMClasses with output format
     * dependent on the parameters in opts.
     * @param opts Options structure with parameters for output
     * display
     * @param paths Array<CIMClass> array containing the
     * CIMObjectOaths to be displayed. This array may be modified by
     * the output functions, specifically to sort the array if the
     * sort command line option is specified.
     */
    static void PEGASUS_CLI_LINKAGE displayClasses(Options& opts,
        Array<CIMClass>& classes);

    /**
     * Display a single CIMObjectPath with output format determined
     * by parameters in opts.
     * @param opts Options structure with parameters for output
     *   display
     * @param path CIMObjectPath to be displayed.
     * @param description String (optional) which may be displayed
     * with the output. If the String is empty, nothing will be
     * displayed.
     */
    static void PEGASUS_CLI_LINKAGE displayPath(Options& opts,
        CIMObjectPath& path,
        const String& description = String());

    /**
     * Display an array of CIMOCIMObjectPaths with output format
     * dependent on the parameters in opts.
     * @param opts Options structure with parameters for output
     * display
     * @param paths Array<CIMObjectPath> array containing the
     * CIMObjectOaths to be displayed. This array may be modified by
     * the output functions, specifically to sort the array if the
     * sort command line option is specified.
     */
    static void PEGASUS_CLI_LINKAGE displayPaths(Options& opts,
        Array<CIMObjectPath>& paths,
        const String& description = String());


    /**
     * Display an array of CIMObjects with output format dependent
     * on the parameters in opts.
     * @param opts Options structure with parameters for output
     * display
     * @param objects Array<CIMObject> array contaning the
     * CIMObjects to be displayed. This array may be modified by the
     * output functions, specifically to sort the array if the sort
     * command line option is specified.
     */
    static void PEGASUS_CLI_LINKAGE displayObjects(Options& opts,
        Array<CIMObject>& objects,
        const String& description);

    // display a single property
    static void PEGASUS_CLI_LINKAGE displayProperty(const Options& opts,
        const CIMProperty& property);

    /**
     * Display a single class Name in a format determined by the
     * opts structure
     * @param opts Options structure with parameters for output
     * display
     * @param className CIMName of className to be displayed
     */
    static void PEGASUS_CLI_LINKAGE displayClassName(const Options& opts,
        const CIMName& className);
    /**
     * Display an array of classnames with format determined from
     * the opts structure.
     * @param opts Options structure with parameters for output
     * display
     * @param classNames Array<CIMName of class names.  The array
     * may be modified by the display function.  It will be sorted
     * if the sort command line option is set.
     */
    static void PEGASUS_CLI_LINKAGE displayClassNames(Options& opts,
        Array<CIMName>& classNames);

    /**
     * Display an array of CIMNamespaceName with output format
     * determined by the opts structure
     * @param opts Options structure with parameters for output
     * display
     * @param ns Array<CIMNamespaceName> array with the namespace
     * names.  Note that the array may be modified by the display
     * function.  It may be sorted if the sort command line option
     * is set.
     */
    static void displayNamespaceNames(Options& opts,
        Array<CIMNamespaceName>& ns,
        const String& description = String());

    /**
     * Display a single CIMValue
     * @param opts Options structure with parameters for output
     * display
     * @param value CIMValue to display
     */
    static void PEGASUS_CLI_LINKAGE displayValue(Options& opts,
        const CIMValue& value);

    /**
     * Display an array of CIMParamValues with format defined by the
     * opts parameter
     * @param opts Options structure with parameters for output
     * display
     * @param parmvalues Array<CIMParamValue> with the parameter
     * values to be displayed.  Note that the array may be modified
     * by the display function. In particular it may be sorted.
     */
    static void PEGASUS_CLI_LINKAGE displayParamValues(Options& opts,
        Array<CIMParamValue>& parmValuess);

    static void PEGASUS_CLI_LINKAGE displayOperationSummary(Options& opts,
        Uint32 count,
        const String& description,
        const String& item);

    /**
     * Display a String with parameters defined by the opts
     * structure.
     * @param opts Options structure with parameters for output
     * display
     * @param s String to display
     */
    static void PEGASUS_CLI_LINKAGE display(Options& opts, const String& s);

    /**
     * Display a char * with parameters defined by the opts
     * structure.
     * @param opts Options structure with parameters for output
     * display
     * @param s char * to display
     */
    static void PEGASUS_CLI_LINKAGE display(Options& opts, const char* s);

    /** Display a Qualifier Declaration or Array of Qualifier with
     *  output formatting determined from the opts parameter
     *
     * @param opts Options structure with parameters for output
     * display
     * @param qualifierDecl QualifierDecl to be displayed.
     */
    static void PEGASUS_CLI_LINKAGE displayQualDecl(const Options& opts,
        const CIMQualifierDecl& qualifierDecl);

    /**
     * Display an array of qualifier decls.
     * @param opts Options structure reference
     * @param qualifierDecls Array<CIMQualifierDecl> Array of
     * qualifer declaractions to be displayed.  This array will be
     * modified by the function if the sort command line option is
     * set since the array itself is sorted.
     */
    static void PEGASUS_CLI_LINKAGE displayQualDecls(Options& opts,
        Array<CIMQualifierDecl>& qualifierDecls);

    /**
     * Test count of returned objects against input -count option parameter
     * and if not equal set error exit code and output message. While this
     * is done within the Output functions for standard object displays
     * other action functions may need it. This function executes
     * the compare test if opt.executeCountTest  is true. It
     * compares the rcvdCount value to opts.expectedCount.
     * @param opts Options structure with parameters for output
     * display
     * @param rcvdCount Uint32 count to compare to value in opt
     * @param description String that will be displayed with message
     * if the test fails
     * NOTE: Today this function exits cimcli with a specific error
     * code if the test fails.  It returns only if the rcvdCount
     * equals the expected count in the opts structure (defined in
     * the opts.expectedCount and executeCountTest is true.
     */
    static void testReturnCount(Options& opts,
        Uint32 rcvdCount,
        const String& description);

private:

};

PEGASUS_NAMESPACE_END

#endif
