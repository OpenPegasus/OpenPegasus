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
    This module contains main() for cimcli.
    It executes the setup,
                input parameter analysis
                    directly for the object definition (second parameter)
                    other parameters through CIMCLIOptions
                 connect
                calling the proper action function per the input operation
                    parameter
                operation repeats
                close of the connect
                output of summary information
                output of timing information
    Legal operation are defined in CIMCLIOperations.  Legal input parameters
    are defined in CIMCLIOptions.
*/
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Threads.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/HostLocator.h>
#include <Pegasus/Common/FileSystem.h>

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/General/Stopwatch.h>

#include <Clients/cimcli/CIMCLIClient.h>
#include <Clients/cimcli/CIMCLIHelp.h>
#include <Clients/cimcli/CIMCLIOptions.h>
#include <Clients/cimcli/CIMCLIOperations.h>
#include <Clients/cimcli/CIMCLICommon.h>
#include <Pegasus/Common/Message.h>

#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String CIMCLI = "cimcli";
/////////////////////////////////////////////////////////////////////////
//
// The following functions process the target object parameter for
// particular action functions.
//
//////////////////////////////////////////////////////////////////////////

/** getClassNameInput - gets the classname object
 * and puts in into the opts.className holder
 * If rqd, parameter must exist and error generated if it does
 * not exist. Otherwise it subsitutes NULL for the string. Also
 * puts the arguement in inputObjectName for possible error
 * display
 * @param argc number of input arguments. Used to determine if
 *             required argument exits
 * @param argv List of input arguments. Argument for conversion
 *             to classname is in this list.
 * @param opts Options Structure reference. Puts results into
 *             this structure
 * @param- rqd - true if parameter required
 * @return True if parameter found. False if parameter not
 *         required and required.
*/
Boolean _getClassNameInput(int argc, char** argv, Options& opts, Boolean rqd)
{
    if (argc > 2)
    {
        opts.inputObjectName = argv[2];
        try
        {
            opts.className = CIMName(argv[2]);
        }

        catch(Exception& e)
        {
            cerr << "Error: Input class name invalid."
                 << endl << e.getMessage() << endl
                 << "Must be the class Name as defined by DMTF Spec."
                 << " Input Probably contains invalid character. "
                 << endl;
            return false;
        }
    }
    else // Parameter does not exist
    {
        opts.inputObjectName = "";
        if (rqd)
        {
                cerr << "Error: Class Name Required. ex. gc CIM_Door" << endl;
                return false;
        }
        else   // set the opts properties to indicate no classname input
        {
                opts.className = CIMName();
        }
    }
    return true ;
}
/** getObjectName - gets the objectname object
 * and puts in into the opts.className holder
 * If rqd, parameter must exist. Otherwise
 * it subsitutes NULL for the string.
 * Also puts the argument in inputObjectName for possible error
 * display
 * @param argc number of input arguments. Used to determine if
 *             required argument exits
 * @param argv List of input arguments. Argument for conversion
 *             to classname is in this list.
 * @param opts Options Structure reference. Puts results into
 *             this structure
 * @param- rqd - true if parameter required
 * @return True if parameter found.
*/
Boolean _getObjectNameInput(int argc, char** argv, Options& opts, Boolean rqd)
{
    if (argc > 2)
    {
        opts.inputObjectName = argv[2];  // save for possible eror report
        try
        {
            opts.targetObjectName = argv[2];
        }
        catch(Exception& e)
        {
            cerr << "Error: Input ObjectPath formatted incorrectly."
                 << endl << e.getMessage() << endl
                 << "Must be model path defined by DMTF Spec."
                 << endl;
            return false;
        }
    }
    else
    {
        opts.inputObjectName = "";
        if (rqd)
        {
            cerr << "Error: Object Name Required" << endl;
            return false ;
        }
        else
        {
            opts.targetObjectName = CIMObjectPath();
        }
    }
    return true;
}

/** _getQualifierNameInput - Gets a single parameter for
 * qualifier. Puts input into inputObjectName for possible error
 * display
 * @return true if parameter found
*/
Boolean _getQualifierNameInput(int argc, char** argv, Options& opts)
{
    if (argc > 2)
    {
        opts.qualifierName = argv[2];
        opts.inputObjectName = argv[2];
    }
    else
    {
        cerr << "Qualifier Name Required" << endl;
        return false ;
    }
    return true ;
}
// static version of opts.verboseTest.  Used because some functions
// in this code are not part of the classes that reference the opts instance
static bool _localVerboseTest = false;

// Test if a file path exists
Boolean _testFileExists(const String& path)
{
    return FileSystem::exists(path);
}
/*
    Function to handle Client Operation Performance data if the
    server returns this data.
    FUTURE - This code still caries some of the tests that were
    initally implemented when the callback was first created.  We should
    explore removing or modifying these tests. At least partly fixed by
    making the displays conditional on verboseTest
*/

ClientOpPerformanceData returnedPerformanceData;
class ClientStatistics : public ClientOpPerformanceDataHandler
{
public:

    virtual void handleClientOpPerformanceData (
            const ClientOpPerformanceData & item)
    {
        // NOTE: We do not use this value so testing it is only a
        // diagnostic function.
        // FUTURE - Should test against operation we are expecting
        if (_localVerboseTest &&
            ((item.operationType <= 0) || item.operationType >
                CIM_ENUMERATION_COUNT_REQUEST_MESSAGE))
        {
           cerr << "Error:Operation type " << item.operationType
                << " out of expected range in ClientOpPerformanceData"
               << endl;

        }
        returnedPerformanceData.operationType =  item.operationType;
        if ((item.roundTripTime == 0) && _localVerboseTest)
        {
           cerr << "WARNING: roundTripTime incorrect (0) in"
                   " ClientOpPerformanceData. " << endl;
        }
        returnedPerformanceData.roundTripTime =  item.roundTripTime;

        if ((item.requestSize == 0) && _localVerboseTest)
        {
            cerr << "WARNING: requestSize incorrect (0) in"
                    " ClientOpPerformanceData " << endl;
        }
        returnedPerformanceData.requestSize =  item.requestSize;

        if ((item.responseSize == 0) && _localVerboseTest)
        {
            cerr << "WARNING:responseSize incorrect (0)"
                    " in ClientOpPerformanceData " << endl;
        }
        returnedPerformanceData.responseSize =  item.responseSize;

        if (item.serverTimeKnown)
        {
            /* Bypass this because we are getting server times zero
            KS_ISSUE There is a server issue returning 0 server time
            This was a test for that issue and we would like to leave
            it documented here until server issue resolved. KS
            if (item.serverTime == 0)
            {
                cerr << "WARNING: serverTime is incorrect in "
                     << " ClientOpPerformanceData." << endl;
            }
            */
            returnedPerformanceData.serverTime =  item.serverTime;
            returnedPerformanceData.serverTimeKnown =  item.serverTimeKnown;
            returnedPerformanceData.roundTripTime =  item.roundTripTime;
        }
   }
};

//////////////////////////////////////////////////////////////////////
//
//      SSL certification function
//
//////////////////////////////////////////////////////////////////////

Boolean SSLCertVerifyCallback(SSLCertificateInfo& certInfo)
{
    if (_localVerboseTest)
    {
        cout << "Returned certificate:\n"
             << certInfo.toString() << endl;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////
//
//            Main
//
///////////////////////////////////////////////////////////////////////
// Returns from this function should use cimcliExitRtn so that the
// return function is used to exit cimcli
//
int main(int argc, char** argv)
{
    //****** Show the args diagnostic display *******
    // This is developer tool to sort out issues of incoming parameters
    // Activated by making the last argument the keyword "_displaycliargs_".
    // It displays all args and then eliminates argv[argc] so the proces
    // can continue normally.
    if (strcmp(argv[argc - 1],"displaycliargs") == 0)
    {
        cout << "argc = " << --argc << endl;
        for (int i = 0; i < argc; i++)
        {
            cout << "argv[" << i << "] = " << argv[i] << endl;
        }
    }

#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

#ifdef PEGASUS_OS_PASE
    // Allow user group name larger than 8 chars in PASE environemnt
    setenv("PASE_USRGRP_LIMITED","N",1);
#endif

    // If no arguments, simply print usage message and terminate.
    MessageLoader::_useProcessLocale = true;

    // If no arguments, just display usage and exit.
    if (argc == 1)
    {
        showUsage();
        return 0;
    }

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments from the command
    // line.

    OptionManager om;

    // Define the options structure. Common structure used for all action
    // functions.
    Options opts;

    // Execute the following in a try block since there are several
    // possibilities of exception and we want to separate these very basic
    // exceptions from the action function exceptions. Exceptions in this
    // block are probably due to programming errors or very strange things
    // in input.
    try
    {
        // Set the path for the config file.
        //assume that the config file is local to directory where called.

        String testHome = ".";
        om.setMessagePath("pegasus/pegasusCLI");

        // Build the options table based on the options defined in the
        // function, a configuration file if it exists, and the command
        // line input of options.
        BuildOptionsTable(om, argc, argv, testHome);

        // Parse and validate input Options based on the options table
        // Exits if this function returns false.  Note that this may be
        // either an error or the option is for a help function and
        // cimcli terminates immediatly after help displays.
        if (!CheckCommonOptionValues(om, argv, opts))
        {
            return cimcliExitRtn(opts.termCondition);
        }

        // move any other input parameters left to the valueParams List
        //
        /* FUTURE: note that this is in too limited since it assumes a fixed
           number of parameters will be used for all of the commands
           It should be expanded to allow for a variable minimum
           number of commands before it picks up any extras.
        */
        if (argc > 2)
        {
            for (int i = 2 ; i < argc ; i++ )
            {
                    opts.valueParams.append(argv[i]);
            }
        }
    }
    catch(CIMException& e)
    {
        cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.INIT_CIMEXCEPT_ERR",
                "CIMException during initialization.\n $0.",
                e.getMessage()));
        //#N substitution {0} is a string with the exception message
        //#N @version 2.14
        //#P 60
        //#T INIT_CIMEXCEPT_ERR
        //#S CIMException during initialization.\n {0}.


        showUsage();
        return cimcliExitRtn(e.getCode());
    }

    catch (Exception& e)
    {
        cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.INIT_EXCEPT_ERR",
                "Exception during initialization.\n $0.",
                e.getMessage()));

//#N substitution {0} is a string with the exception message
//#T INIT_EXCEPT_ERR
//#S Exception during initialization.\n {0}.
        showUsage();
        return cimcliExitRtn(CIMCLI_RTN_CODE_PEGASUS_EXCEPTION);
    }
    catch(...)
    {
        cimcliMsg::errmsg(MessageLoaderParms(
                "Clients.cimcli.CIMCLIClient.INIT_GENEXCEPT_ERR",
                "General Exception during initialization."));
        //#T INIT_GENEXCEPT_ERR
        //#S General Exception during initialization.
        showUsage();
        return cimcliExitRtn(GENERAL_CLI_ERROR_CODE);
    }

    // if there is an arg1, assume it is the command name.

    if (argc > 1)
    {
        opts.cimCmd = argv[1];
    }
    else
    {
        cimcliMsg::errmsg(MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.OPMISSING_ERR",
            "ERROR: Command name or shortcut must be first parameter."
            " \n  ex. $0 enumerateclasses or \n    $0 ec."));
        //#N Substitution {0} is name of the executable
        //#N @version 2.14
        //#P 61
        //#T OPMISSING_ERR
        //#S ERROR: Command name or shortcut must be first parameter.
        //#S \n  ex. {0} enumerateclasses or \n {0} ec.

        showUsage();

        return cimcliExitRtn(CIMCLI_INPUT_ERR);
    }

    // if the trace option was set initialize the trace function.
    // for cimcli
    if (opts.trace != 0)
    {
        const char* tmpDir = getenv ("PEGASUS_TMP");
            if (tmpDir == NULL)
            {
                tmpDir = ".";
            }
            String traceFile (tmpDir);
            traceFile.append("/cliTrace.trc");
            Tracer::setTraceFile (traceFile.getCString());
            Tracer::setTraceComponents("ALL");
            Tracer::setTraceLevel(opts.trace);
    }

    if (opts.verboseTest && opts.debug)
    {
        cout << "Operation = " << opts.cimCmd << endl;
    }

    // Find the command or the shortcut name input.

    Operations thisOperation;

    if (!thisOperation.find(opts.cimCmd))
    {
        cimcliMsg::errmsg(MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.INVALID_OP_NAME_ERR",
            "Error: Invalid cimcli operation name $0. "
                "Operation name must be first parameter"
                " or --c parameter."
                " \n  ex. cli enumerateclasses\n"
                "Enter $1 -h for help.",
            opts.cimCmd, CIMCLI));
        //#N substitution {0} is a string with invalid name
        //#N substitution {1} name of executable
        //#N @version 2.14
        //#P 62
        //#T INVALID_OP_NAME_ERR
        //#S Error: Invalid cimcli operation name {0}.
        //#S Operation name must be first parmeter or --c parameter.
        //#S .ex. cli enumerateclasses.
        //#S Enter {1} -h for help.

        return cimcliExitRtn(GENERAL_CLI_ERROR_CODE);
    }

    // Start the time for total elapsed time for the command
    Stopwatch totalElapsedExecutionTime;
    totalElapsedExecutionTime.start();

    //
    // Try to open the connection to the cim server
    //

    try
    {
        if (thisOperation.get().ID_Operation != ID_ShowOptions)
        {
            String host;
            HostLocator addr;

            if (opts.location.size() != 0)
            {
                addr.setHostLocator(opts.location);
                if (!addr.isValid())
                {
                    throw InvalidLocatorException(opts.location);
                }
                host = addr.getHost();
            }

            Uint32 portNumber = System::lookupPort( WBEM_HTTP_SERVICE_NAME,
                              WBEM_DEFAULT_HTTP_PORT );

            // Set up SSL port and flag for verbose display
            // if SSL included in build
            String useSSL;
#ifdef PEGASUS_HAS_SSL
            if (opts.ssl)
            {
                // Default port from system
                portNumber = System::lookupPort( WBEM_HTTPS_SERVICE_NAME,
                              WBEM_DEFAULT_HTTPS_PORT );
            }
            useSSL = " ssl=";
            useSSL.append((opts.ssl)? "true" : "false");
#endif

            if (host.size() != 0 && addr.isPortSpecified())
            {
                portNumber = addr.getPort();
            }

            //check whether we should use connect() or connectLocal()
            //an empty location option indicates requires using connectLocal()
            //Use localhost, etc. as option to do connect to same machine
            //with connect(...)
            if (host.size() == 0)
            {
                if (opts.verboseTest)
                {
                    cout << "Connect with connectLocal" << endl;
                }
                opts.client.connectLocal();
            }
            else
            {
                if (opts.verboseTest)
                {
                    cout << "Connect to " << host
                        << " port=" << portNumber
                        << useSSL
                         << " for User=" << opts.user
                         << endl;
                }


                // Connect with SSL api only if SSL compile enabled
#ifdef PEGASUS_HAS_SSL
                String sslRndFilePath = "sss.rnd";

                // Make connection using the SSL connection and ssl options
                // of key, cert, truststore.
                if (opts.ssl)
                {
                    // Test that both cert and key either exist or do not exist.
                    // We only warn so that tests can be used to test for
                    // this issue.

                    if ((opts.clientCert.size() == 0
                            && opts.clientKey.size() != 0)
                        ||
                        (opts.clientCert.size() != 0
                            && opts.clientKey.size() == 0))
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.KEYPATH_ERR",
                            "WARNING: Both client key and path required."));
                        //#T KEYPATH_ERR
                        //#S WARNING: Both client key and path required.
                    }
                    if ((opts.clientKey.size() != 0) &&
                        !_testFileExists(opts.clientKey))
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.KEY_ERR",
                            "WARNING: Key File $0 does not exist.",
                            opts.clientKey));
                        //#N Substitution {0} is a file name
                        //#N @version 2.14
                        //#P 64
                        //#T KEY_ERR
                        //#S WARNING: Key File {0} does not exist.
                    }

                    if ((opts.clientCert.size() != 0) &&
                        !_testFileExists(opts.clientCert))
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.CERT_ERR",
                            "WARNING: Certificate File $0 does not exist.",
                            opts.clientCert));
                        //#N Substitution {0} is a file name
                        //#N @version 2.14
                        //#P 65
                        //#T CERT_ERR
                        //#S WARNING: Certificate File {0} does not exist.
                    }

                    if ((opts.clientTruststore.size() != 0) &&
                        !_testFileExists(opts.clientTruststore))
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.TRUSTFILE_ERR",
                            "WARNING: Truststore directory/file $0 does"
                            " not exist."));
                        //#N Substitution {0} is a file name
                        //#N @version 2.14
                        //#P 66
                        //#T TRUSTFILE_ERR
                        //#S WARNING: Truststore directory/file {0} does
                        //#S not exist.
                    }

                    if(opts.clientCert.size() != 0
                       || opts.clientKey.size() != 0)
                    {
                        if (opts.verboseTest)
                        {
                            cout << "SSL options: "
                                << "CertPath(--cert) = " << opts.clientCert
                                << "clientKeyPath(--key) = " << opts.clientKey;

                            if (opts.clientTruststore.size() != 0)
                            {
                                cout << "clientTruststore(--truststore) = "
                                     << opts.clientTruststore;
                            }
                            else
                            {
                                cout << "No clientTrustStore(--truststore)";
                            }
                            cout << endl;
                        }
                        if (opts.clientTruststore.size() == 0)
                        {
                            // Use clientCert and clientKey for connect.
                            // Optional truststore to check server
                            // cert against.
                            opts.client.connect(host,
                                           portNumber,
                                           SSLContext(opts.clientTruststore,
                                               opts.clientCert,
                                               opts.clientKey,
                                               SSLCertVerifyCallback,
                                               sslRndFilePath),
                                           opts.user,
                                           opts.password);
                        }

                    } // End if cert and key exist.

                    else
                    {
                        // No client key or cert. Truststore is
                        // still an option.
                        if (opts.verboseTest)
                        {
                            cout << "SSL options: No key or cert. ";
                            if (opts.clientTruststore.size() != 0)
                            {
                                cout << "clientTruststore(--truststore) = "
                                     << opts.clientTruststore;
                            }
                            else
                            {
                                cout << " No clientTruststore(--truststore)";
                            }
                            cout << endl;
                        }
                        // Connect over SSL but without client trusted cert
                        // Client checks server cert against truststore
                        opts.client.connect(host,
                                       portNumber,
                                       SSLContext(opts.clientTruststore,
                                                  SSLCertVerifyCallback,
                                                  sslRndFilePath),
                                       opts.user,
                                       opts.password);
                    }
                } // end is SSL call

                else  // not ssl request
                {
                    // warn if any of the ssl parameters set and then
                    // ignore
                    if((opts.clientCert.size() != 0)
                       || (opts.clientKey.size() != 0)
                       || (opts.clientTruststore.size() != 0))
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.SSL_NOT_USED_ERR",
                            "WARNING: --key,--cert, or --truststore "
                             "parameters input  but not used."));
                        //#N Substitution {0} is a file name
                        //#N @version 2.14
                        //#P 67
                        //#T SSL_NOT_USED_ERR
                        //#S WARNING: --key,--cert, or --truststore
                        //#S parameters input  but not used.
                    }
                    // Connect to server only with name and password.
                    opts.client.connect(host, portNumber, opts.user,
                                        opts.password);
                }
#else   // Not SSL
                opts.client.connect(host, portNumber, opts.user,
                                    opts.password);
#endif
            }
        }  // end if if not show options.
    }
    catch(Exception &e)
    {
        cimcliMsg::errmsg(MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.CONNECT_EXCEPTION_ERR",
            "Pegasus Exception: $0 trying to connect to $1",
                e.getMessage(), opts.location ));
        //#N Substitution {0} is an exception message Text
        //#N substitution {1} is host name or IP address
        //#N @version 2.14
        //#P 68
        //#T CONNECT_EXCEPTION_ERR
        //#S Pegasus Exception: {0} trying to connect to {1}.
        return cimcliExitRtn(CIMCLI_CONNECTION_FAILED);
    }

    // Register for Client statistics which might be returned with
    // the response
    ClientStatistics statistics = ClientStatistics();
    _localVerboseTest = opts.verboseTest;
    opts.client.registerClientOpPerformanceDataHandler(statistics);

    // if delay option set, execute this delay between the connect and the
    // execution of the command.
    // NOTE: This was a test option for some specific tests
    // FUTURE: Consider removing this option.
    if (opts.delay != 0)
    {
        Threads::sleep(opts.delay * 1000);
    }

    // If the timeout is not zero, set the timeout for this connection.
    if (opts.connectionTimeout != 0)
    {
        opts.client.setTimeout(opts.connectionTimeout * 1000);
    }

    // Setup the other timers.
    double totalTime = 0;
    Uint32 repeatCount = opts.repeat;
    double maxTime = 0;
    double minTime = 10000000;

    Uint64 serverTotalTime = 0;
    Uint64 maxServerTime = 0;
    Uint64 minServerTime = 10000000;

    Uint64 rtTotalTime = 0;
    Uint64 maxRtTime = 0;
    Uint64 minRtTime = 10000000;

    //
    //  Process the requested operation action function
    //  Process the function within a try block to catch all operation
    //  command exceptions
    //
    try
    {
        // Loop to repeat the command a number of times.
        // while opts.repeat > 0
        do
        {
            // or exit with error through default of case logic
            switch(thisOperation.get().ID_Operation)
            {
                case ID_EnumerateInstanceNames :
                    if (!_getClassNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = enumerateInstanceNames(opts);
                    break;

                case ID_EnumerateAllInstanceNames :
                    if (!_getClassNameInput(argc, argv, opts, false))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = enumerateAllInstanceNames(opts);
                    break;

                case ID_EnumerateInstances :
                    if (!_getClassNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = enumerateInstances(opts);
                    break;
                case ID_GetInstance :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = getInstance(opts);
                    break;

                case ID_EnumerateClassNames :
                    if (!_getClassNameInput(argc, argv, opts, false))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = enumerateClassNames(opts);
                    break;

                case ID_EnumerateClasses :
                    if (!_getClassNameInput(argc, argv, opts, false))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = enumerateClasses(opts);
                    break;

                case ID_GetClass :
                    if (!_getClassNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = getClass(opts);
                    break;

                case ID_CreateInstance :
                    if (!_getClassNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = createInstance(opts);
                    break;

                case ID_TestInstance :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = testInstance(opts);
                    break;

                case ID_ModifyInstance :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = modifyInstance(opts);
                    break;

                case ID_DeleteInstance :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = deleteInstance(opts);
                    break;

                case ID_CreateClass :
                    cerr << "CreateClass not implemented" << endl;
                    return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    break;

                case ID_DeleteClass :
                    if (!_getClassNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = deleteClass(opts);
                    break;

                case ID_GetProperty :
                    if (argc < 4)
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.GETPROP_ERR",
                            "Usage: $0 getproperty <instancename>"
                                        "<propertyname> or"
                                        " $0 getproperty <classname> "
                                        "<propertyname> <keypropery=value>*.",
                            CIMCLI));
        //#N Substitution {0} is the name of the executable
        //#N getproperty is keyword.  Do not translate
        //#N classname keyproperty instancename propertyname are words
        //#N representing particular input (ex. property name
        //#N is the name of a property.
        //#N @version 2.14
        //#P 69
        //#T GETPROP_ERR
        //#S Usage: {0} getproperty <instancename> <propertyname> or
        //#S {0} getproperty <classname> <propertyname> <keyproperty=value>*.

                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }

                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }

                    // The current CLI operation
                    //  cimcli gp Test_CLITestProviderClass.Id=\"Mike\"  Name
                    // could be input as
                    //  cimcli gp Test_CLITestProviderClass Id="Mike"  Name
                    //So we assume the last paramter is for propertyName
                    opts.propertyName = argv[argc-1];
                    opts.valueParams.remove(opts.valueParams.size()-1);

                    opts.termCondition = getProperty(opts);
                    break;

                case ID_SetProperty :
                    if (argc < 4)
                    {
                        // KS_TODO this probably in error. Does not id where
                        // the new value is defined.
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.SETPROP_ERR",
                            "Usage: $0 setproperty <instancename>"
                                        "<propertyname> or"
                                        " $0 setproperty <classname> "
                                        "<propertyname> <keyproperty=value>*.",
                                        CIMCLI ));
//#N Substitution {0} is the name of the executable
//#N setproperty is keyword.  Do not translate
//#N classname keyproperty instancename propertyname are words
//#N representing particular input (ex. property name
//#N is the name of a property.
//#N @version 2.14
//#P 670
//#T SETPROP_ERR
//#S Usage: {0} setproperty <instancename> <propertyname> or
//#S {0} setproperty <classname> <propertyname> <keyproperty=value>*.
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    else
                    // The current CLI operation
                    //  cimcli sp
                    //    Test_CLITestProviderClass.Id=\"PropertyOpTest\"
                    //    scalSint8=-99
                    // could be input as
                    //  cimcli sp Test_CLITestProviderClass Id="PropertyOpTest"
                    //    scalSint8=-99
                    //So we assume the last paramter is for propertyName=value
                    {
                        String property = argv[argc-1];
                        Uint32 pos = property.find('=');
                        if (PEG_NOT_FOUND == pos)
                        {
                            cout <<
                               "Usage: cli setproperty instancename "
                               "propertyname=value "
                               << endl;
                            return cimcliExitRtn(CIMCLI_INPUT_ERR);
                        }

                        opts.propertyName = property.subString(0,pos);
                        opts.newValue =
                            property.subString(opts.propertyName.size() + 1,
                                               PEG_NOT_FOUND);
                        opts.valueParams.remove(opts.valueParams.size()-1);

                        opts.termCondition = setProperty(opts);
                    }
                    break;

                case ID_EnumerateQualifiers :
                    opts.termCondition = enumerateQualifiers(opts);
                    break;

                case ID_SetQualifier :
                    cerr << "SetQualifer not implemented" << endl;
                    return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    break;

                case ID_GetQualifier :
                    if (!_getQualifierNameInput(argc, argv, opts))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = getQualifier(opts);
                    break;

                case ID_DeleteQualifier :
                    if (!_getQualifierNameInput(argc, argv, opts))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = deleteQualifier(opts);
                    break;

                case ID_References  :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = references(opts);
                    break;

                case ID_ReferenceNames :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = referenceNames(opts);
                    break;

                case ID_Associators :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = associators(opts);
                    break;

                case ID_AssociatorNames :
                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    opts.termCondition = associatorNames(opts);
                    break;

                case ID_EnumerateNamespaces :
                    // Note that the following constants are fixed here.  We
                    // should be getting them from the environment to assure
                    // that others know that we are using them.
                    //opts.className = PEGASUS_CLASSNAME_CIMNAMESPACE;

                    if (argc > 2)
                    {
                        opts.nameSpace = argv[2];
                        opts.inputObjectName = argv[2];
                    }
                    else
                    {
                        // set nameSpace to interop namespace name
                        opts.nameSpace =
                            PEGASUS_NAMESPACENAME_INTEROP.getString();
                    }

                    opts.termCondition = enumerateNamespaceNames(opts);
                    break;

                case ID_InvokeMethod :
                    if (argc < 4)
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.INVOKEMETHOD_ERR",
                            "Usage: invokemethod(im) requires that object and"
                                " method names be specified.\n"
                                "Input parameters are optional and can be"
                                " specified as additional parameters to"
                                " this call. "
                                "Enter each input parameter as name=value"
                                " (no spaces around equal sign)." ));
                        //#N invokemethod is a keyword. im is a keyword
                        //#N @version 2.14
                        //#P 71
                        //#T INVOKEMETHOD_ERR
                        //#S Usage: invokemethod(im) requires that object and
                        //#S method names be specified.\n
                        //#S Input parameters are optional and can be
                        //#S specified as additional parameters to
                        //#S this call.
                        //#S Enter each input parameter as name=value
                        //#S (no spaces around equal sign).

                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }

                    if (!_getObjectNameInput(argc, argv, opts, true))
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }

                    opts.methodName = CIMName(argv[3]);
                    // remove the method name argument
                    opts.valueParams.remove(0);

                    // If there are any extra arguments they must be parameters
                    // These parameters  can be used in addtion to parameters
                    // ifrom the -ip option setting. Parameters found here must
                    // be key=value pairs or they will generate an exception.

                    opts.termCondition = invokeMethod(opts);
                    break;

                case ID_ShowOptions :
                    showUsage();
                    break;

                case ID_ExecQuery:
                    if (argc <= 2 && opts.query.size() == 0)
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.EXECQUERY_ERR",
                            "Usage: execquery(xq) requires a query"
                                "filter definition:\n"
                                "   - supplied directly as a parameter\n"
                                "   - OR supplied with the -f option\n"
                                "   The filterLanguage may be supplied\n"
                                "   - as the second argument\n"
                                "   - OR as the -ql option\n" ));
                        //#N execquery is a keyword. im is a keyword
                        //#T EXECQUERY_ERR
                        //#S Usage: execquery(xq) requires a query
                        //#S filter definition:\n
                        //#S    - supplied directly as a parameter\n
                        //#S    - OR supplied with the -f option\n
                        //#S    The filterLanguage may be supplied\n
                        //#S    - as the second argument\n
                        //#S    - OR as the -ql option.
                    }
                    opts.query = argv[2];
                    if (argc==4)
                    {
                        opts.queryLanguage = argv[3];
                    }
                    opts.termCondition = execQuery(opts);
                    break;

                case ID_StatisticsOn:
                    Boolean rtndState;
                    setObjectManagerStatistics(opts, true, rtndState);
                    break;

                case ID_StatisticsOff:
                    opts.termCondition = setObjectManagerStatistics(
                        opts, false, rtndState);
                    break;

//KS_PULL_BEGIN
                case (ID_PullEnumerateInstances):
                    if (!_getClassNameInput(argc, argv, opts, true))
                        exit(CIMCLI_INPUT_ERR);
                    opts.termCondition = pullEnumerateInstances(opts);
                    break;

                case (ID_PullEnumerateInstancePaths):
                    if (!_getClassNameInput(argc, argv, opts, true))
                        exit(CIMCLI_INPUT_ERR);
                    opts.termCondition = pullEnumerateInstancePaths(opts);
                    break;
                case (ID_PullReferenceInstances):
                    if (!_getObjectNameInput(argc, argv, opts, true))
                        exit(CIMCLI_INPUT_ERR);
                    opts.termCondition = pullReferenceInstances(opts);
                    break;

                case (ID_PullReferenceInstancePaths):
                    if (!_getObjectNameInput(argc, argv, opts, true))
                        exit(CIMCLI_INPUT_ERR);
                    opts.termCondition = pullReferenceInstancePaths(opts);
                    break;

                case (ID_PullAssociatorInstances):
                    if (!_getObjectNameInput(argc, argv, opts, true))
                        exit(CIMCLI_INPUT_ERR);
                    opts.termCondition = pullAssociatorInstances(opts);
                    break;

                case (ID_PullAssociatorInstancePaths):
                    if (!_getObjectNameInput(argc, argv, opts, true))
                        exit(CIMCLI_INPUT_ERR);
                    opts.termCondition = pullAssociatorInstancePaths(opts);
                    break;
                case ID_PullQueryInstances:
                    if (argc <= 2 && opts.query.size() == 0)
                    {
                        cimcliMsg::errmsg(MessageLoaderParms(
                            "Clients.cimcli.CIMCLIClient.PULLQUERY_ERR",
                            "Usage: pullqueryInstances(pxq) requires a "
                                "query filter definition:\n"
                                "   - supplied directly as a parameter\n"
                                "   - OR supplied with the -f option\n"
                                "   The filterLanguage may be supplied:\n"
                                "   - as the second argument\n"
                                "   - OR as the -ql option\n" ));
                        //#N execquery is a keyword. im is a keyword
                        //#T PULLQUERY_ERR
                        //#S Usage: pullQueryInstances requires a query
                        //#S filter definition:\n
                        //#S    - supplied directly as a parameter\n
                        //#S    - OR supplied with the -f option\n
                        //#S    The filterLanguage may be supplied\n
                        //#S    - as the second argument\n
                        //#S    - OR as the -ql option.
                    }
                    opts.query = argv[2];
                    if (argc==4)
                    {
                        opts.queryLanguage = argv[3];
                    }
                    opts.termCondition = pullQueryInstances(opts);
                    break;

//KS_PULL_END
                case (ID_CountInstances):
                    if (_getClassNameInput(argc, argv, opts,  false))
                    {
                        opts.termCondition = countInstances(opts);
                    }
                    else
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    break;

                case (ID_ClassTree):
                    if (_getClassNameInput(argc, argv, opts,  false))
                    {
                        opts.termCondition = classTree(opts);
                    }
                    else
                    {
                        return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    }
                    break;

            default:
                    // This should never happen
                    // message international def documented above.
                    cimcliMsg::errmsg(MessageLoaderParms(
                        "Clients.cimcli.CIMCLIClient.OPMISSING_ERR",
                        "ERROR: Command name or shortcut must be first "
                        "parameter.\n  ex. $0 enumerateclasses or \n $0 ec."));

                    return cimcliExitRtn(CIMCLI_INPUT_ERR);
                    break;
            } // switch statement

            // If the repeat option set, do any interim time calculation
            // and output and decrement the repeat count
            if (opts.repeat > 0)
            {
                if (opts.verboseTest)
                {
                    cout << "Repetitition " << opts.repeat << endl;
                }
                opts.repeat--;

                if (opts.time)
                {
                    totalTime += opts.saveElapsedTime;
                    maxTime = LOCAL_MAX(maxTime, opts.saveElapsedTime);
                    minTime = LOCAL_MIN(minTime, opts.saveElapsedTime);
                    rtTotalTime += (returnedPerformanceData.roundTripTime);
                    maxRtTime = LOCAL_MAX(maxRtTime,
                            returnedPerformanceData.roundTripTime);
                    minRtTime = LOCAL_MIN(minRtTime,
                            returnedPerformanceData.roundTripTime);

                    if (returnedPerformanceData.serverTimeKnown)
                    {
                        serverTotalTime += (returnedPerformanceData.serverTime);
                        maxServerTime = LOCAL_MAX(maxServerTime,
                                returnedPerformanceData.serverTime);
                        minServerTime = LOCAL_MIN(minServerTime,
                                returnedPerformanceData.serverTime);
                    }
                }
            }
        } while (opts.repeat > 0  );

        // Command processing complete.  If the time parameter is set,
        // output any total time information

        if (opts.time)
        {
            cout << thisOperation.get().OperationName << " "
                 << opts.inputObjectName;

            if (repeatCount == 0)
            {
                // KS_TODO did not internationalize this one.
                cout << " Time= "
                    << opts.saveElapsedTime
                    << " Sec "
                    << " SvrTime= "
                    << CIMValue(returnedPerformanceData.serverTime).toString()
                    << " us "
                    << " RtTime= "
                    << CIMValue(returnedPerformanceData.roundTripTime).
                           toString()
                    << " us "
                    << "Req size= "
                    << CIMValue(returnedPerformanceData.requestSize).toString()
                    << " b Resp size= "
                    << CIMValue(returnedPerformanceData.responseSize).toString()
                    << " b"
                    << endl;
            }
            else
            {
                cout << " Total Time "
                    << totalTime
                    << " for "
                    << repeatCount
                    << " ops. Avg= "
                    << (totalTime * 1000000)/repeatCount
                    << " us min= "
                    << minTime * 1000000
                    << " us max= "
                    << (maxTime * 1000000)
                    << " us SvrTime avg= "
                    << CIMValue(serverTotalTime/repeatCount).toString()
                    << " us SvrTime min= "
                    << CIMValue(minServerTime).toString()
                    << " us SvrTime max= "
                    << CIMValue(maxServerTime).toString()
                    << " us"
                    << " RtTime avg= "
                    << CIMValue(rtTotalTime/repeatCount).toString()
                    << " us RtTime min= "
                    << CIMValue(minRtTime).toString()
                    << " us RtTime max= "
                    << CIMValue(maxRtTime).toString()
                    << " us"
                    << endl;
            }
        }
    }

    // Exceptions for action operations try block
    // The following exceptions are all routed to cerr
    catch(CIMException& e)
    {
        cerr << CIMCLI << " CIMException: "
             <<" Cmd= " << opts.cimCmd
             << " Object= " << opts.inputObjectName
             << " Code= " << e.getCode()
             << "\n" << e.getMessage()
             << endl;
        opts.termCondition = e.getCode();
    }
    catch(Exception& e)
    {
        cerr << CIMCLI << " Pegasus Exception: " << e.getMessage()
                <<  ". Cmd = " << opts.cimCmd
                << " Object = " << opts.inputObjectName
                << endl;
        opts.termCondition = CIMCLI_RTN_CODE_PEGASUS_EXCEPTION;
    }
    catch(...)
    {
        cerr << CIMCLI << " Caught General Exception:" << endl;
        opts.termCondition = CIMCLI_RTN_CODE_UNKNOWN_EXCEPTION;
    }

    totalElapsedExecutionTime.stop();

    if (opts.time)
    {
        // if abnormal term, dump all times
        if (opts.termCondition != 0)
        {
            // KS_TODO did not internationalize this one since it is
            // and abnormal termination
            cout << "Exception in cimcli " << endl;
            cout << "Prev Elapsed Time " << opts.saveElapsedTime << " Sec"
                 << endl;
            opts.saveElapsedTime = opts.elapsedTime.getElapsed();
            cout << "Last Elaposed Time " << opts.saveElapsedTime << " Sec"
                 << endl;
            cout << "Total Time " << totalTime << " for "
                << repeatCount << " operations. Avg.= "
                << totalTime/repeatCount
                << " min= " << minTime << " max= " << maxTime << endl;
        }
         cimcliMsg::msg(MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.TOTAL_ELAPSED_TIME",
            "Total Elapsed Time= $0 sec. Terminated at $1. ",
            totalElapsedExecutionTime.getElapsed(),
            System::getCurrentASCIITime()));
         //#N Substitution {0} integer representing elapsed time
         //#N Substitution {1} String with current time
         //#T TOTAL_ELAPSED_TIME
         //#S Total Elapsed Time= {0} sec. Terminated at {1}.
    }

    // if delay parameter set, sleep before terminating.
    if (opts.delay != 0)
    {
        Threads::sleep(opts.delay * 1000);
    }

    // Terminate with termination code
    return cimcliExitRtn(opts.termCondition);
}

//PEGASUS_NAMESPACE_END

// END_OF_FILE
