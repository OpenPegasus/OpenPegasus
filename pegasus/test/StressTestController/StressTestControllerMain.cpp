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

#include <iostream>
#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "StressTestController.h"
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/FileSystem.h>

//#define DEBUG
#include <time.h>

//
// For Windows
//
#ifdef PEGASUS_OS_TYPE_WINDOWS
 // for DWORD etc.
# include <windows.h>
 // getpid() and others
 typedef DWORD pid_t;
# include <process.h>
#else
# include <unistd.h>
#endif


/**
   Signal handler set up SIGALARM.
*/
static Boolean useDefaults = false;


/**
   Message resource name
 */

static const char ERR_OPTION_NOT_SUPPORTED [] =
    "Invalid option. Use '--help' to obtain command syntax.";

static const char ERR_USAGE [] =
    "Incorrect usage. Use '--help' to obtain command syntax.";


//
// exclude main from the Pegasus Namespace
//
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


int main (int argc, char* argv [])
{
    char strTime[256];
    struct tm tmTime;
    int rc;
    String fileName;
    ofstream log_file;


    tmTime = getCurrentActualTime();
    strftime(strTime,256,"%d%m%Y%H%M%S.",&tmTime);

    StressTestControllerCommand command;

    //
    // Generate log files and PID files
    //
    if (!command.generateRequiredFileNames(strTime))
    {
        cout<<StressTestControllerCommand::COMMAND_NAME\
            <<"::Failed to generate required files for tests. "<<endl;
        command.removeUnusedFiles();
        return Command::RC_ERROR;
    }

    //
    // open the log file
    //
    OpenAppend(log_file,command.getStressTestLogFile());

    if (!log_file)
    {
       log_file.close();
       cout<<"Cannot get file "<<command.getStressTestLogFile()<<endl;
       command.removeUnusedFiles();
       return Command::RC_ERROR;
    }
    strftime(strTime,256,"%d/%m/%Y at %H:%M:%S\n",&tmTime);
    log_file<<StressTestControllerCommand::COMMAND_NAME\
        <<"::Initiated on "<<strTime<<endl;
    log_file<<StressTestControllerCommand::COMMAND_NAME\
        <<"::Process ID: "<<getpid()<<endl;

    try
    {
        log_file<<StressTestControllerCommand::COMMAND_NAME;
        log_file<<"::Geting Command Options."<<endl;
        if(verboseEnabled)
        {
           cout<<StressTestControllerCommand::COMMAND_NAME;
           cout<<"::Getting Command options."<<endl;
        }
        //
        // validate and set command arguments
        //
        command.setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        String msg(cfe.getMessage());

        log_file<< StressTestControllerCommand::COMMAND_NAME << "::" \
            << msg <<  endl;
        cerr<< StressTestControllerCommand::COMMAND_NAME << "::"\
            << msg <<  endl;

        if (msg.find(String("Unknown flag")) != PEG_NOT_FOUND)
        {

            cerr<< StressTestControllerCommand::COMMAND_NAME <<
                "::" << ERR_OPTION_NOT_SUPPORTED << endl;
            log_file<< StressTestControllerCommand::COMMAND_NAME <<
                "::" << ERR_OPTION_NOT_SUPPORTED << endl;
        }
        else
        {
            cerr<< StressTestControllerCommand::COMMAND_NAME <<
                "::" << ERR_USAGE << endl;
            log_file<< StressTestControllerCommand::COMMAND_NAME <<
                "::" << ERR_USAGE << endl;
        }

        log_file.close();
        command.removeUnusedFiles();
        return Command::RC_ERROR;
    }
    catch (...)
    {
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Unknown exception caught when setting commands."<<endl;
        cerr<<StressTestControllerCommand::COMMAND_NAME<<
            "::Unknown exception caught when setting commands."<<endl;
        log_file.close();
        command.removeUnusedFiles();
        return Command::RC_ERROR;
    }

    //
    // For help or version options execute usage/version and
    // exit
    //
    if ((command.getOperationType() == OPERATION_TYPE_HELP)
       ||(command.getOperationType() == OPERATION_TYPE_VERSION))
    {
        rc = command.execute (cout, cerr);
        log_file.close();
        //
        // Log file not required when help or verbose is opted.
        //
        FileSystem::removeFile(command.getStressTestLogFile());
        return rc;
    }

    String filename;

    //
    // If a configuration file is specified then:
    //    Check if it exists as indicated, if not
    //    also look for it in the default config dir.
    //
    if (command.IsConfigFilePathSpecified())
    {
        filename = command.getConfigFilePath();
        FileSystem::translateSlashes(filename);
        //
        // Check whether the file exists or not
        //
        if (!FileSystem::exists(filename))
        {
            //
            // Check for file in default directory as well
            //
            fileName = StressTestControllerCommand::DEFAULT_CFGDIR;
            fileName.append(filename);

            if (!FileSystem::exists(fileName))
            {
                cerr << StressTestControllerCommand::COMMAND_NAME ;
                cerr << "::Specified Configuration file \""<<filename;
                cerr << "\" does not exist."<<endl;
                log_file.close();
                command.removeUnusedFiles();
                return Command::RC_ERROR;
            }
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                "::Using config file: "<<fileName<<endl;
        }
        else
        {
            fileName = filename;
        }

        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Using config file: "<<fileName<<endl;
        cout<<StressTestControllerCommand::COMMAND_NAME<<
           "::Using config file: "<<fileName<<endl;
    }
    else
    {
        //
        // Use default file in default dir.
        //
        fileName = StressTestControllerCommand::DEFAULT_CFGDIR;
        fileName.append(StressTestControllerCommand::FILENAME);
        //
        // Use hard coded default configuration values if default conf. file
        // was not found.
        if (!FileSystem::exists(fileName))
        {
            //
            // Use Hard-coded default values
            //
            useDefaults = true;
        }
        else
        {
            log_file << StressTestControllerCommand::COMMAND_NAME <<
                "::Using default file: " << fileName<<endl;
            cout << StressTestControllerCommand::COMMAND_NAME <<
                "::Using default file: " << fileName<<endl;
        }
    }

    //
    // Read the contents of the file
    //
    try
    {
        //
        // Use Hard-coded default values
        //
        if (useDefaults)
        {
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                "::Using hard coded default config values."<<endl;
            cout<<StressTestControllerCommand::COMMAND_NAME<<
                "::Using hard coded default config values."<<endl;
            command.getDefaultClients(log_file);
        }
        else
        {
            log_file << StressTestControllerCommand::COMMAND_NAME <<
                "::Reading config file: " << fileName<<endl;
            if (verboseEnabled)
            {
                cout<< StressTestControllerCommand::COMMAND_NAME <<
                    "::Reading config file: " << fileName<<endl;
            }
            command.getFileContent(fileName,log_file);
        }
    }
    catch (NoSuchFile& e)
    {
        String msg(e.getMessage());

        log_file << StressTestControllerCommand::COMMAND_NAME <<
            ": " << msg <<  endl;
        cerr << StressTestControllerCommand::COMMAND_NAME <<
            ": " << msg <<  endl;
        log_file.close();
        command.removeUnusedFiles();
        return Command::RC_ERROR;
    }
    catch (Exception& e )
    {
        String msg(e.getMessage());
        log_file << StressTestControllerCommand::COMMAND_NAME <<
            "::" << msg <<  endl;
        cerr << StressTestControllerCommand::COMMAND_NAME <<
            "::Invalid Configuration ";
        cerr << "in File: " << fileName <<  endl;
        cerr << msg <<  endl;
        log_file.close();
        command.removeUnusedFiles();
        return Command::RC_ERROR;
    }
    catch (...)
    {
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Unknown exception caught when acquiring configuration."<<endl;
        cerr<<StressTestControllerCommand::COMMAND_NAME<<
            "::Unknown exception caught when acquiring configuration."<<endl;
        log_file.close();
        command.removeUnusedFiles();
        return Command::RC_ERROR;
    }

    log_file << StressTestControllerCommand::COMMAND_NAME <<
        "::Generating Client Commands"<<  endl;
    if(verboseEnabled)
    {
        cout << StressTestControllerCommand::COMMAND_NAME <<
            "::Generating Client Commands"<<  endl;
    }

    //
    // TimeStamp
    //
    log_file<<StressTestControllerCommand::COMMAND_NAME<<
        "::Initiated on "<<strTime<<endl;
    log_file<<StressTestControllerCommand::COMMAND_NAME<<
        "::Process ID: "<<getpid()<<endl;
    cout<<StressTestControllerCommand::COMMAND_NAME<<
        "::Initiated on "<<strTime<<endl;
    cout<<StressTestControllerCommand::COMMAND_NAME<<
        "::Process ID: "<<getpid()<<endl;

    if(!command.generateClientCommands(log_file))
    {
        cerr << StressTestControllerCommand::COMMAND_NAME <<
            "::Failed to Generate Client Commands."<<  endl;
        log_file << StressTestControllerCommand::COMMAND_NAME <<
            "::Failed to Generate Client Commands."<<  endl;
        log_file.close();
        command.removeUnusedFiles();
        return Command::RC_ERROR;
    }

    //
    // Getting current time
    //
    tmTime = getCurrentActualTime();
    strftime(strTime,256,"%d/%m/%Y at %H:%M:%S\n",&tmTime);
    log_file << StressTestControllerCommand::COMMAND_NAME <<endl;
    log_file << "   Preparing to execute Clients on "<<strTime<<endl;

    //
    // Begin to run stress Tests
    //
    rc = command.execute (cout, cerr);

    //
    // Getting current time after stress Tests are completed
    //
    tmTime = getCurrentActualTime();

    strftime(strTime,256,"%d/%m/%Y at %H:%M:%S\n",&tmTime);

    //
    // Check overall status of tests
    //
    if(rc)
    {
       log_file << StressTestControllerCommand::COMMAND_NAME;
       log_file << "::execution interrupted on "<<strTime<<endl;
       cout << StressTestControllerCommand::COMMAND_NAME;
       cout << "::execution interrupted on "<<strTime<<endl;
    } else {
       log_file << StressTestControllerCommand::COMMAND_NAME;
       log_file << "::successfully completed on "<<strTime<<endl;
       cout << StressTestControllerCommand::COMMAND_NAME;
       cout << "::successfully completed on "<<strTime<<endl;
    }
    cout <<"IMPORTANT: ";
    cout <<"Please check the Controller log file for additional info and the"
         << endl;
    cout <<"           Client log file for individual errors which may or may "
         <<"not have "<< endl;
    cout <<"           caused a stress test failure. "<< endl;
    cout <<" Log Directory:"<<endl;
    cout <<"     "<<
        FileSystem::extractFilePath(command.getStressTestLogFile())<<endl;
    cout <<"\n Controller log: "<<
        FileSystem::extractFileName(command.getStressTestLogFile())<<endl;
    cout <<" Client log:     "<<
        FileSystem::extractFileName(command.getStressClientLogFile())<<endl;
    log_file.close();
    return rc;
} /* main */
