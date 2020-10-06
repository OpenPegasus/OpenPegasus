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

#ifndef Pegasus_CIMSubCommand_h
#define Pegasus_CIMSubCommand_h


#include <iostream>

#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>


PEGASUS_NAMESPACE_BEGIN

/**

This is a CLI used to manage indication subscriptions.  This command supports
operations to list, enable, disable, and remove subscriptions.

 */

//
//  String entries for each column in the output
//
typedef Array <String> ListColumnEntry;

class CIMSubCommand : public Command
{

public:

    /**
        Constructs a CIMSubCommand and initializes instance variables.
    */
    CIMSubCommand ();

    //
    // Overrides the virtual function setCommand from Command class
    // This is defined as an empty function.
    //
    void setCommand (Uint32, char**)
    {
        // Empty function
    };

    /**
        Parses the command line, validates the options, and sets instance
        variables based on the option arguments. This implementation of
        setCommand includes the parameters for output and error stream.

        @param  outPrintWriter    The stream to which command output is written.

        @param  errPrintWriter    The stream to which command errors are
                                  written.

        @param  argc       The int containing the arguments count

        @param  argv       The string array containing the command line
                            arguments

        @throws  CommandFormatException  if an error is encountered in parsing
                                     the command line
    */
    void setCommand (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter,
        Uint32 argc,
        char* argv []);

    /**
        Executes the command and writes the results to the output streams.

        @param  outPrintWriter    The stream to which command output is written.

        @param  errPrintWriter    The stream to which command errors are
                                  written.

        @return  0        if the command is successful
                 1        if an error occurs in executing the command
    */
    Uint32 execute (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);


private:
    /**
        list all matching subscriptions in the specified namespace(s)

        @param  namespaceNames          The namespaces to search

        @param  filterName              The name of the filter to list

        @param  filterNamespace         The namespace of the filter to list

        @param  handlerName             The handler name to find

        @param  handlerNamespace        The handler namespace to find

        @param  handlerCreationClass    The handler creation class to find

        @param  verbose                 A true value denotes a verbose listing
                                        A false value denotes a columnar list

        @param  outPrintWriter          The stream to which command output
                                        is written.

        @param  errPrintWriter          The stream to which command errors are
                                        written.
    */
    void _listSubscriptions(
        const Array<CIMNamespaceName>& namespaceNames,
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        const Boolean verbose,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        create specify filter instance

        @param  filterName              The name of filter to create

        @param  filterNamespace         The filter namespace where create

        @param  filterQuery             The query expression of filter to create

        @param  filterQueryLanguage     The query language of filter to create

        @param  filterSourceNamespace   The source namespace of filter to create

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return 0       if the filter instance was created
                Other   if the filter instance was not created
    */
    Uint32 _createFilter(
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        const String& filterQuery,
        const String& filterQueryLanguage,
        const Array<String>& filterSourceNamespaces,
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    /**
        create specify CIMXML handler instance

        @param  handlerName             The name of handler to create

        @param  handlerNamespace        The handler namespace where create

        @param  handlerCreationClass    The creation class of handler to create

        @param  handlerDestination      The destination of filter to create

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return 0       if the filter instance was created
                Other   if the filter instance was not created
    */
    Uint32 _createCimXmlHandler(
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        const String& handlerDestination,
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    /**
        create specify syslog handler instance

        @param  handlerName             The name of handler to create

        @param  handlerNamespace        The handler namespace where create

        @param  handlerCreationClass    The creation class of handler to create

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return 0       if the filter instance was created
                Other   if the filter instance was not created
    */
    Uint32 _createSystemLogHandler(
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    /**
        create specify e-mail handler instance

        @param  handlerName             The name of handler to create

        @param  handlerNamespace        The handler namespace where create

        @param  handlerCreationClass    The creation class of handler to create

        @param  mailTo                  The mail to list of mial handler

        @param  mailCc                  The mail cc list of mial handler

        @param  mailSubject             The mail subject of mial handler

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return 0       if the filter instance was created
                Other   if the filter instance was not created
    */
    Uint32 _createEmailHandler(
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        const Array<String>& mailTo,
        const Array<String>& mailCc,
        const String& mailSubject,
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    /**
        create specify syslog handler instance

        @param  handlerName             The name of handler to create

        @param  handlerNamespace        The handler namespace where create

        @param  handlerCreationClass    The creation class of handler to create

        @param  targetHost              The target host of snmp handler

        @param  snmpPort                The port number of snmp handler

        @param  snmpVersion             The snmp version of snmp handler

        @param  securityName            The security name of snmp handler

        @param  engineId                The engine id of snmp handler

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return 0       if the filter instance was created
                Other   if the filter instance was not created
    */
    Uint32 _createSnmpMapperHandler(
            const String& handlerName,
            const CIMNamespaceName& handlerNamespace,
            const String& handlerCreationClass,
            const String& targetHost,
            Uint32 snmpPort,
            Uint32 snmpVersion,
            const String& securityName,
            const String& engineId,
            ostream& outPrintWriter,
            ostream& errPrintWriter);

    /**
        create specify subscription instance

        @param  subscriptionNamespace   The subscription namespace to create

        @param  filterName              The name of the filter to create

        @param  filterNamespace         The namespace of the filter to create

        @param  handlerName             The handler name to create

        @param  handlerNamespace        The handler namespace to create

        @param  handlerCreationClass    The handler creation class to create

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return true    if the subscription instance was created
                false   if the subscription instance was not created
    */
    Uint32 _createSubscription(
        const CIMNamespaceName& subscriptionNamespace,
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    /**
        remove matching subscription instance

        @param  subscriptionNamespace   The subscription namespace to search

        @param  filterName              The name of the filter to find

        @param  filterNamespace         The namespace of the filter to find

        @param  handlerName             The handler name to find

        @param  handlerNamespace        The handler namespace to find

        @param  handlerCreationClass    The handler creation class to find

        @param  removeAll               If true remove subscription,filter,
                                        handler
                                        If false remove only subscription

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return true    if the subscription instance was found
                false   if the subscription instance was not found
    */
    Uint32 _removeSubscription (
        const CIMNamespaceName& subscriptionNamespace,
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        const Boolean removeAll,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        remove matching filter instance

        @param  filterName              The name of the filter to find

        @param  filterNamespace         The namespace of the filter to find

        @param  outPrintWriter          The stream to which command output is
                                        written.

        @param  errPrintWriter          The stream to which command errors
                                        are written.

        @return true    if the filter instance was found
                false   if the filter instance was not found
    */
    Uint32 _removeFilter (
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        remove matching handler instance

        @param  handlerName           The handler name to find

        @param  handlerNamespace      The handler namespace to find

        @param  handlerCreationClass  The handler creation class to find

        @param  outPrintWriter        The stream to which command output is
                                      written.

        @param  errPrintWriter        The stream to which command errors are
                                      written.

        @return true    if the handler instance was found
                false   if the handler instance was not found
    */
    Uint32 _removeHandler (
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        Get a list of subscriptions in the specifed namespace

        @param  subscriptionNSin        The namespace to search

        @param  filterName              The name of the filter to list

        @param  filterNSIn              The namespace of the filter to list

        @param  handlerName             The handler name to match

        @param  handlerNSIn             The handler namespace to match

        @param  handlerCreationClass    The handler creation class to match

        @param  verbose                 A true value denotes a verbose listing
                                        A false value denotes a columnar list

        @param  handlerInstancesFound   The corresponding handler for this
                                        subscription

        @param  handlerTypesFound       The type of handler found. This is an
                                        enumerated type of the handler
                                        creation class.

        @param  querysFound             The subscription's filter's query
                                        string

        @param  maxColumnWidth          The array of column widths. This is
                                        the fixed width for each column. This
                                        is used to lineup all the data in
                                        each column.

        @param  listOutputTable         The array of output strings to print.
                                        It is an array of columns. The first
                                        element in each column is the column
                                        title.
    */
    void _getSubscriptionList (
        const CIMNamespaceName& subscriptionNSIn,
        const String& filterName,
        const CIMNamespaceName& filterNSIn,
        const String& handlerName,
        const CIMNamespaceName& handlerNSIn,
        const String& handlerCreationClass,
        const Boolean verbose,
        Array<CIMInstance>& handlerInstancesFound,
        Array<Uint32>& handlerTypesFound,
        Array<String>& querysFound,
        Array <Uint32>& maxColumnWidth,
        Array <ListColumnEntry>& listOutputTable);

    /**
        List matching filters in the specified namespace(s)

        @param  filterName        The name of the filter to list

        @param  verbose           A true value denotes a verbose listing
                                  A false value denotes a columnar list

        @param  namespaceNames    The namespaces to search

        @param  outPrintWriter    The stream to which command output is
                                  written.

        @param  errPrintWriter    The stream to which command errors are
                                  written.
    */
  void _listFilters (
        const String& filterName,
        const Boolean verbose,
        const Array<CIMNamespaceName>& namespaceNames,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        Get matching filters in the specified namespace

        @param  filterName        The name of the filter to list

        @param  filterNamespace   The namespace of the filter to list

        @param  verbose           A true value denotes a verbose listing
                                  A false value denotes a columnar list

        @param  maxColumnWidth    The array of column widths. This is the fixed
                                  width for each column. This is used to lineup
                                  all the data in each column.

        @param  listOutputTable   The array of output strings to print. It is
                                  an array of columns. The first element in
                                  each column is the column title.

        @param  queryLangsFound   The array of query language strings for each
                                  filter found

        @param filterSourceNamespaces The array of source namcespace strings
                                  for each filter found

        @param  outPrintWriter    The stream to which command output is
                                  written.

        @param  errPrintWriter    The stream to which command errors are
                                  written.
    */
    void _getFilterList (
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        const Boolean verbose,
        Array <Uint32>& maxColumnWidth,
        Array <ListColumnEntry>& listOutputTable,
        Array <String>& queryLangsFound,
        Array<String>& filterSourceNamespaces,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        List matching handlers in the specified namespace(s)

        @param  handlerName       The handler name to find

        @param  namespaceNames    The namespaces to search

        @param  handlerCreationClass  The handler creation class to match

        @param  verbose           A true value denotes a verbose listing
                                  A false value denotes a columnar list

        @param  outPrintWriter    The stream to which command output is
                                  written.

        @param  errPrintWriter    The stream to which command errors are
                                  written.
    */
    void _listHandlers (
        const String& handlerName,
        const Array<CIMNamespaceName>& namespaceNames,
        const String& handlerCreationClass,
        const Boolean verbose,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);


    /**
        get a list of all handlers in a specified namespace

        @param  handlerName       The handler name to find

        @param  handlerNamespace  The handler namespace to search

        @param  handlerCreationClass  The handler creation class to match

        @param  verbose           A true value denotes a verbose listing
                                  A false value denotes a columnar list

        @param  instancesFound    The array of matching CIMInstances

        @param  handlerTypesFound The type of handler found. This is an
                                  enumerated type of the handler
                                  creation class.

        @param  maxColumnWidth    The array of column widths. This is the fixed
                                  width for each column. This is used to lineup
                                  all the data in each column.

        @param  listOutputTable   The array of output strings to print. It is
                                  an array of columns. The first element in
                                  each column is the column title.

        @param  outPrintWriter    The stream to which command output is
                                  written.

        @param  errPrintWriter    The stream to which command errors are
                                  written.
     */
    void _getHandlerList (
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        const Boolean verbose,
        Array<CIMInstance>& instancesFound,
        Array<Uint32>& handlerTypesFound,
        Array <Uint32>& maxColumnWidth,
        Array <ListColumnEntry>& listOutputTable,
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter
        );

    /**
        Get the name from a CIMObjectPath in string form

        @param r     The string of key to scan

        @return      The string value of the name property
    */
    String _getNameInKey (
       const CIMObjectPath& r);

    /**
        Find all namespaces in repository

        @param  namespaceNames The array of all namespaces found
    */
    void _getAllNamespaces (
        Array<CIMNamespaceName>& namespaceNames);

    /**
        Change the subscription state

        @param  subscriptionNS          The subscription's namespace

        @param  targetPath              The path of the subscription to modify

        @param  newState                The new state for the subscription

        @param  outPrintWriter          The stream to which command errors are
                                        written.
    */
    void _modifySubscriptionState (
        const CIMNamespaceName& subscriptionNS,
        const CIMObjectPath& targetPath,
        const Uint16 newState,
        PEGASUS_STD(ostream)& outPrintWriter);

    /**
        Find a matching subscription

        @param  subscriptionNamespace   The subscription namespace to search

        @param  filterName              The name of the filter to match

        @param  filterNamespace         The namespace of the filter to match

        @param  handlerName             The handler name to match

        @param  handlerNamespace        The handler namespace to match

        @param  handlerCC               The creation class of the handler to
                                        match

        @param  subscriptionFound       The OjectPath of the subscription found

        @return true    if the subscription instance was found
                false   if the subscription instance was not found
    */
    Boolean _findSubscription (
        const CIMNamespaceName& subscriptionNamespace,
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCC,
        Array<CIMObjectPath>& subscriptionFound);

    /**
        Find a subscription and modify it's state

        @param  newState          The new state for the subscription

        @param  subscriptionNamespace The subscription namespace to search

        @param  filterName        The name of the filter

        @param  filterNamespace   The namespace of the filter

        @param  handlerName       The handler name to find

        @param  handlerNamespace  The handler namespace to search

        @param  handlerCreationClass The creation class of the handler

        @param  outPrintWriter    The stream to which command errors are
                                  written.

        @return RC_SUCCESS          if the subscription instance was found
                RC_OBJECT_NOT_FOUND if the subscription instance was not found
    */
    Uint32 _findAndModifyState (
        const Uint16 newState,
        const CIMNamespaceName& subscriptionNamespace,
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        PEGASUS_STD(ostream)& outPrintWriter);

    /**
        find a filter

        @param  filterName        The name of the filter

        @param  filterNamespace   The namespace of the filter

        @param  errPrintWriter    The stream to which command errors are
                                  written.

        @param  filterpath        The reference of the filter found

        @return true    if the subscription instance was found
                false   if the subscription instance was not found
     */
    Boolean _findFilter (
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        PEGASUS_STD(ostream)& errPrintWriter,
        CIMObjectPath& filterPath);

    /**
        find matching handler

        @param  handlerName           The handler name to find

        @param  handlerNamespace      The handler namespace to search

        @param  handlerCreationClass  The handler creation class to match

        @param  errPrintWriter        The stream to which command errors are
                                      written.

        @param  handlerPath           The handler reference found

        @return true    if the handler instance was found
                false   if the handler instance was not found
    */
    Boolean _findHandler (
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        PEGASUS_STD(ostream)& errPrintWriter,
        CIMObjectPath& handlerPath);

    /**
        check a subscription for a filter match

        @param  subPath          The ObjectPath of the subscription to check

        @param  subscriptionNS   The namespace of the subscription

        @param  filterName       The name of the filter

        @param  filterNamespace  The namespace of the filter

        @param  filterNS         The namespace of the filter reference

        @param  filterRef        The path for the filter reference

        @return true    if the subscription filter ref matches the filter
                        criteria
                false   if the subscription filter ref does not match
     */
    Boolean _filterMatches (
        const CIMObjectPath& subPath,
        const CIMNamespaceName& subscriptionNS,
        const String& filterName,
        const CIMNamespaceName& filterNamespace,
        CIMNamespaceName& filterNS,
        CIMObjectPath& filterRef);

    /**
        check a subscription for a handler match

        @param  subPath          The ObjectPath of the subscription to check

        @param  subscriptionNS   The namespace of the subscription

        @param  handlerName      The name of the handler

        @param  handlerNamespace The namespace of the handler

        @param  handlerCreationClass The creation class of the handler

        @param  handlerNS        The namespace of the handler reference

        @param  handlerRef       The path for the handler reference

        @return true    if the subscription handler ref matches the handler
                        criteria
                false   if the subscription handler ref does not match
     */
    Boolean _handlerMatches (
        const CIMObjectPath& subPath,
        const CIMNamespaceName& subscriptionNS,
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        CIMNamespaceName& handlerNS,
        CIMObjectPath& handlerRef);

    /**
        Get the email properties from a handler instance

        @param  handlerInstance    The handler instance to query

        @param  ccString           The value of the MailCc property

        @param  toString           The value of the MailTo property

        @param  subjectString      The value of the MailSubject property
    */
    void _getEmailInfo (
        const CIMInstance& handlerInstance,
        String& ccString,
        String& toString,
        String& subjectString);

    /**
        Get the persistence value from the handler instance

        @param  handlerInstance    The handler instance to query

        @return the text string equivalent of the persistence type
    */
    String _getPersistenceType (
        const CIMInstance& handlerInstance);

    /**
        Get the subscription state string from a subscription instance

        @param  subscriptionNameSpace The subscription namespace

        @param  subPath               The ObjectPath of the subscription to
                                      query

        @return the text string equivalent of the subscription state
    */
    String _getSubscriptionState (
        const CIMNamespaceName& subscriptionNamespace,
        const CIMObjectPath& subPath);

    /**
        Get the SNMP version string from a handler instance

        @param  handlerInstance  The snmp handler instance to query

        @return the text string equivalent of the snmp version
    */
    String _getSnmpVersion (
        const CIMInstance& handlerInstance);

    /**
        Print data in columns.

        @param  maxColumnWidth    The array of column widths. This is the fixed
                                  width for each column. This is used to lineup
                                  all the data in each column.

        @param  listOutputTable   The array of output strings to print. It is
                                  an array of columns. The first element in
                                  each column is the column title.

        @param  outPrintWriter    The stream to which command output is
                                  written.
    */
    void _printColumns (
        const Array <Uint32>& maxColumnWidth,
        const Array <ListColumnEntry>& listOutputTable,
        PEGASUS_STD(ostream)& outPrintWriter);

    /**

        Sort a string array by indexes

        @param  x                 The array of strings to be sorted

        @param  startIndex        Starting index for the sort

        @param  index             The array of indexes into the x array

    */
    void _bubbleIndexSort (
        const Array<String>& x,
        const Uint32 startIndex,
        Array<Uint32>& index);

    /**
        print a verbose listing of Filters

        @param  listOutputTable   The array of output strings to print.
                                  It is an array of columns. The first
                                  element in each column is the column
                                  title.

        @param  queryLangs        The array of query language strings to print.

        @param  filterSourceNamespaces The array of source namespace strings
                                       to print.

        @param  outPrintWriter    The stream to which command output is
                                  written.
     */

    void _printFiltersVerbose (
        const Array <ListColumnEntry>& listOutputTable,
        const Array <String>& queryLangs,
        const Array<String>& filterSourceNamespaces,
        PEGASUS_STD(ostream)& outPrintWriter);

    /**
        print a verbose listing of Handlers

        @param  instancesFound    The array of matching handler instances

        @param  handlerTypesFound The type of handler found. This is an
                                  enumerated type of the handler
                                  creation class.

        @param  listOutputTable   The array of output strings to print.
                                  It is an array of columns. The first
                                  element in each column is the column
                                  title.

        @param  outPrintWriter    The stream to which command output is
                                  written.
     */

    void _printHandlersVerbose (
        const Array<CIMInstance>& instancesFound,
        const Array<Uint32>& handlerTypesFound,
        const Array <ListColumnEntry>& listOutputTable,
        PEGASUS_STD(ostream)& outPrintWriter);

    /**
        get the handler destination and type

        @param  handlerInstance    The handler instance to query

        @param  creationClassName  Return value of creation class for this
                                   handler

        @param  handlerTypesFound  The type of handler found. This is an
                                   enumerated type of the handler
                                   creation class.

        @param  destination        Return string value of destination for
                                   this handler
     */

    void _getHandlerDestination (
        const CIMInstance& handlerInstance,
        String& creationClassName,
        Uint32&  handlerTypeFound,
        String& destination);

    /**
        get the filter information for a filter

        @param filterNamespace The filter namespace
        @param filterPath The filter's object path
        @param queryString The text string equivalent of the query string
        @param queryLangString The text string equivalent of the querylanguage
                   string
        @param filterSourceNamespace The text string equivalent of the
                   source namespace string
    */
    void _getFilterInfo (
        const CIMNamespaceName& filterNamespace,
        const CIMObjectPath& filterPath,
        String& queryString,
        String& queryLangString,
        Array <String>& filterSourceNamespace);

    /**
        print a verbose listing of subscriptions

            @param  handlerInstancesFound The array of matching handler
                                          instances


            @param  handlerTypesFound     The type of handler found. This is an
                                          enumerated type of the handler
                                          creation class.

            @param  querysFound           The array of handler query strings

            @param  listOutputTable       The array of output strings to
                                          print. It is an array of columns.
                                          The first element in each column
                                          is the column title.

            @param  outPrintWriter        The stream to which command output
                                          is written.
    */
    void _printSubscriptionsVerbose (
        const Array<CIMInstance>& handlerInstancesFound,
        const Array<Uint32>& handlerTypesFound,
        const Array<String>& querysFound,
        const Array <ListColumnEntry>& listOutputTable,
        PEGASUS_STD(ostream)& outPrintWriter);

    /**
        parse the filter option string

         @param  filterNameString  The value of the filter commandline
                                   option

         @param  filterName        Return value of parsed filter name

         @param  filterNameSpace   Return value of parsed filter namespace
    */
    void _parseFilterName(
        const String& filterNameString,
        String& filterName,
        String& filterNamespace);

    /**
        parse the handler option string

        @param  handlerString          The value of the handler commandline
                                       option

        @param  handlerName            Return value of parsed handler name

        @param  handlerNameSpace       Return value of parsed handler
                                       namespace

        @param  handlerCreadionClass   Return value of parsed handler
                                       creation class
    */
    void _parseHandlerName(
        const String& handlerString,
        String& handlerName,
        String& handlerNamespace,
        String& handlerCreationClass);

    /**
        Values for the creation class type
     */
    enum _handlerCreationClassType {_HANDLER_CIMXML = 0,
        _HANDLER_EMAIL = 1, _HANDLER_SYSLOG = 2,
        _HANDLER_SNMP = 3};

    //
    // The CIM Client reference
    //
    AutoPtr<CIMClient> _client;

    //
    // The type of operation specified on the command line.
    //
    Uint32 _operationType;

    //
    // The subtype of operation to be done
    //
    String _operationArg;

    //
    // The Filter Name
    //
    String _filterName;

    //
    // The Filter Namespace
    //
    String _filterNamespace;

    //
    // The Filter Query
    //
    String _filterQuery;

    //
    // The Filter Query Language
    //
    String _filterQueryLanguage;

    //
    // The Filter SourceNamespaces
    //
    Array<String> _filterSourceNamespaces;

    //
    //The sourceNamespaces Property flag
    //

    Boolean _filterNSFlag;

    //
    // The Handler Name
    //
    String _handlerName;

    //
    // The Handler Namespace
    //
    String _handlerNamespace;

    //
    // The Handler's creation class
    //
    String _handlerCreationClass;

    //
    // The Handler's destination
    //
    String _handlerDestination;

    //
    // The Handler's maillto
    //
    String _handlerMailTo;

    //
    // The Handler's maillcc
    //
    String _handlerMailCc;

    //
    // The Handler's maill subject
    //
    String _handlerMailSubject;

    //
    // The Handler's SNMP Tartget Host
    //
    String _handlerSNMPTartgetHost;

    //
    // The Handler's SNMP Port Number
    //
    Uint64 _handlerSNMPPortNumber;

    //
    // The Handler's SNMP Version
    //
    Uint64 _handlerSNMPVersion;

    //
    // The Handler's SNMP Security Name
    //
    String _handlerSNMPSecurityName;

    //
    // The Handler's SNMP Engine ID
    //
    String _handlerSNMPEngineID;

    //
    // The namespace of the subscription
    //
    String _subscriptionNamespace;

    //
    //The Batch file Name for Batch execution
    //

    String _batchFileName;

    //
    // The Namespace specified for batch file.
    //

    String _batchNamespace;

    //
    // The flag for batch namespace.
    //

    Boolean _isBatchNamespace;

    //
    // The flag to indicate a verbose listing
    //
    Boolean _verbose;


    String usage;

    //
    //  These constants represent the operation modes supported by the CLI.
    //  Any new operation should be added here.
    //

    /**
        This constant signifies that an operation option has not been recorded
     */
    static const Uint32 OPERATION_TYPE_UNINITIALIZED;

    /**
        This constant represents a disable subscription operation
     */
    static const Uint32 OPERATION_TYPE_DISABLE;

    /**
        This constant represents a enable subscription operation
     */
    static const Uint32 OPERATION_TYPE_ENABLE;

    /**
        This constant represents a list operation
     */
    static const Uint32 OPERATION_TYPE_LIST;

    /**
        This constant represents a remove operation
     */
    static const Uint32 OPERATION_TYPE_REMOVE;

    /**
        This constant represents a help operation
     */
    static const Uint32 OPERATION_TYPE_HELP;

    /**
        This constant represents a verbose list operation
     */
    static const Uint32 OPERATION_TYPE_VERBOSE;

    /**
        This constant represents a version display operation
     */
    static const Uint32 OPERATION_TYPE_VERSION;

    /**
        This constant represents a create operation
     */
    static const Uint32 OPERATION_TYPE_CREATE;

    /**
        This constant represents a Batch Execution Operation
     */

    static const Uint32 OPERATION_TYPE_BATCH;

};

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_CIMSubCommand_h */
