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
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>

#include <Pegasus/getoopt/getoopt.h>

#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

#include <iostream>

#ifdef PEGASUS_OS_TYPE_UNIX
# include <fcntl.h>
# include <errno.h>
# include <sys/types.h>
# include <sys/wait.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    The CLI message resource name
 */
static const char MSG_PATH[] = "pegasus/pegasusCLI";

/**
    The command name.
*/
static const char COMMAND_NAME[] = "cimreparchive";

/**
    The usage string for this command.  This string is displayed
    when an error occurs in parsing or validating the command line.
*/
static const char USAGE[] = "Usage: ";

/*
    These constants represent the operation modes supported by the CLI.
    Any new operation should be added here.
*/

/**
    This constant signifies that an operation option has not been recorded
*/
static const Uint32 OPERATION_TYPE_UNINITIALIZED  = 0;

/**
    This constant represents an archive operation
*/
static const Uint32 OPERATION_TYPE_ARCHIVE        = 1;

/**
    This constant represents a help operation
*/
static const Uint32 OPERATION_TYPE_HELP           = 2;

/**
    This constant represents a version display operation
*/
static const Uint32 OPERATION_TYPE_VERSION        = 3;


//
// cimreparchive exit status values
//
static const Uint32 EXIT_STATUS_SUCCESS             = 0;
static const Uint32 EXIT_STATUS_GENERAL_ERROR       = 1;
static const Uint32 EXIT_STATUS_SYSTEM_CALL_FAILED  = 2;
static const Uint32 EXIT_STATUS_ARCHIVE_FAILED      = 3;


static const char LONG_HELP[] = "help";

static const char LONG_VERSION[] = "version";


/**
    This command is used to archive a copy of the CIM Server's repository.
*/
class CIMRepositoryArchiveCommand : public Command
{
public:

    /**
        Constructs a CIMRepositoryArchiveCommand.
    */
    CIMRepositoryArchiveCommand();

    /**
        Parses the command line, validates the options, and sets member
        variables based on the option arguments.

        @param  argc       An integer number of arguments.
        @param  argv       A string array of command line arguments

        @exception CommandFormatException If an error is encountered in parsing
            the command line.
    */
    void setCommand(Uint32 argc, char* argv[]);

    /**
        Executes the command and writes the results to the output streams.

        @param ostream The stream to which command output is written.
        @param ostream The stream to which command errors are written.
        @return 0 if the command is successful,
                1 if an error occurs in executing the command.
    */
    Uint32 execute(
        ostream& outPrintWriter,
        ostream& errPrintWriter);

private:

    /**
        The destination file for the repository archive.
    */
    String _archiveFileName;

    /**
        The type of operation specified on the command line.
    */
    Uint32 _operationType;

    String _usage;
};

/**
    Constructs a CIMRepositoryArchiveCommand and initializes instance variables.
*/
CIMRepositoryArchiveCommand::CIMRepositoryArchiveCommand()
{
    //
    // Build the command usage string.
    //

    _usage.reserveCapacity(250);
    _usage.append(USAGE);
    _usage.append(COMMAND_NAME);
    _usage.append(" archive_file\n");
    _usage.append("       cimreparchive --").append(LONG_HELP).append("\n");
    _usage.append("       cimreparchive --").append(LONG_VERSION).append("\n");

    _usage.append("Options:\n");
    _usage.append("    --help     - Display this help message\n");
    _usage.append("    --version  - Display CIM Server version number\n");

#ifdef PEGASUS_HAS_ICU

    MessageLoaderParms menuparms(
        "Clients.cimreparchive.CIMRepositoryArchiveCommand.MENU.STANDARD",
        _usage);
    menuparms.msg_src_path = MSG_PATH;
    _usage = MessageLoader::getMessage(menuparms);

#endif

    setUsage(_usage);
}


/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.
*/
void CIMRepositoryArchiveCommand::setCommand(
    Uint32 argc,
    char* argv[])
{
    //
    //  Initialize and parse options
    //
    getoopt options("");
    options.addFlagspec("");    // No (short) options are defined
    options.addLongFlagspec(LONG_HELP,getoopt::NOARG);
    options.addLongFlagspec(LONG_VERSION,getoopt::NOARG);

    options.parse(argc, argv);

    if (options.hasErrors())
    {
        throw CommandFormatException(options.getErrorStrings()[0]);
    }

    _operationType = OPERATION_TYPE_UNINITIALIZED;


    //
    //  Get options and arguments from the command line
    //
    for (Uint32 i = options.first(); i < options.last(); i++)
    {
        if (options[i].getType() == Optarg::LONGFLAG)
        {
            if (options[i].getopt() == LONG_HELP)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    String param = String(options[i].getopt());
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(param);
                }

               _operationType = OPERATION_TYPE_HELP;
            }
            else if (options[i].getopt() == LONG_VERSION)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    String param = String(options[i].getopt());
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(param);
                }

               _operationType = OPERATION_TYPE_VERSION;
            }
            else
            {
                throw UnexpectedOptionException(String(options[i].getopt()));
            }
        }
        else if (options[i].getType() == Optarg::REGULAR)
        {
            if (_operationType != OPERATION_TYPE_UNINITIALIZED)
            {
                throw UnexpectedArgumentException(options[i].Value());
            }

            _archiveFileName = options[i].Value();
            _operationType = OPERATION_TYPE_ARCHIVE;
        }

        // No short form options are defined for this command.
        PEGASUS_ASSERT(options[i].getType() != Optarg::FLAG);
    }

    //
    // Some more validations
    //
    if (_operationType == OPERATION_TYPE_UNINITIALIZED)
    {
        throw CommandFormatException(localizeMessage(
            MSG_PATH,
            "Clients.cimreparchive.CIMRepositoryArchiveCommand."
                "REQUIRED_ARGS_MISSING",
            "Required arguments missing."));
    }
}

/**
    Executes the command and writes the results to the PrintWriters.
*/
Uint32 CIMRepositoryArchiveCommand::execute(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    if (_operationType == OPERATION_TYPE_HELP)
    {
        errPrintWriter << _usage << endl;
        return EXIT_STATUS_SUCCESS;
    }

    if (_operationType == OPERATION_TYPE_VERSION)
    {
        errPrintWriter << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return EXIT_STATUS_SUCCESS;
    }

    //
    // Perform the requested archive operation
    //

    PEGASUS_ASSERT(_operationType == OPERATION_TYPE_ARCHIVE);

    //
    // Get environment variables
    //
    const char* env = getenv("PEGASUS_HOME");
    String repositoryDir = FileSystem::getAbsolutePath(
        env, PEGASUS_REPOSITORY_DIR);
    String lockFile = FileSystem::getAbsolutePath(
        env, PEGASUS_REPOSITORY_LOCK_FILE);

    String repositoryDirPath = FileSystem::extractFilePath(repositoryDir);
    String repositoryDirName = FileSystem::extractFileName(repositoryDir);

    AutoFileLock repositoryLock(lockFile.getCString());

    // Define the "tar" command location based on the platform
#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_AIX) || \
  defined(PEGASUS_OS_PASE)
    const char TAR_COMMAND[] = "/usr/bin/tar";
#elif defined(PEGASUS_OS_LINUX)
    const char TAR_COMMAND[] = "/bin/tar";
#elif defined(PEGASUS_OS_ZOS)
    const char TAR_COMMAND[] = "/bin/pax";
#else
    const char TAR_COMMAND[] = "tar";
#endif

    // Define the "archive" command based on the platfrom
#if defined(PEGASUS_OS_ZOS)
    String sysCommand =
        String(TAR_COMMAND) + " -o saveext -ppx -wzf "
            + _archiveFileName + " " + repositoryDir;
#else
    String sysCommand =
        String(TAR_COMMAND) + " cf " + _archiveFileName +
            " -C " + repositoryDirPath + " " + repositoryDirName;
#endif

#if defined(PEGASUS_OS_TYPE_UNIX)

    int rc = system(sysCommand.getCString());
    if (rc == -1)
    {
        MessageLoaderParms errorMessage(
            "Clients.cimreparchive.CIMRepositoryArchiveCommand.SYSCALL_FAILED",
            "Failed to initiate archive operation: $0",
            strerror(errno));
        errPrintWriter << MessageLoader::getMessage(errorMessage) << endl;
        return EXIT_STATUS_SYSTEM_CALL_FAILED;
    }
    else if (WIFEXITED(rc) && (WEXITSTATUS(rc) != 0))
    {
        // Archive operation failed.  Write an error and remove
        // the partial archive file.
        MessageLoaderParms errorMessage(
            "Clients.cimreparchive.CIMRepositoryArchiveCommand.ARCHIVE_FAILED",
            "Archive operation failed with status $0.  Removing file \"$1\".",
            WEXITSTATUS(rc), _archiveFileName);
        errPrintWriter << MessageLoader::getMessage(errorMessage) << endl;
        FileSystem::removeFile(_archiveFileName);
        return EXIT_STATUS_ARCHIVE_FAILED;
    }

    MessageLoaderParms errorMessage(
        "Clients.cimreparchive.CIMRepositoryArchiveCommand.ARCHIVE_CREATED",
        "File \"$0\" now contains an archive of directory \"$1\".",
        _archiveFileName, repositoryDir);
    outPrintWriter << MessageLoader::getMessage(errorMessage) << endl;

    return EXIT_STATUS_SUCCESS;

#else

    errPrintWriter << "Archive operation is not available." << endl;
    return EXIT_STATUS_ARCHIVE_FAILED;

#endif
}

PEGASUS_NAMESPACE_END

//
// exclude main from the Pegasus Namespace
//
PEGASUS_USING_PEGASUS;

///////////////////////////////////////////////////////////////////////////////
/**
    Parses the command line, and execute the command.

    @param argc An integer count of the command line arguments
    @param argv A string array containing the command line arguments
*/
///////////////////////////////////////////////////////////////////////////////

int main (int argc, char* argv[])
{
    AutoPtr<CIMRepositoryArchiveCommand> command;
    Uint32 retCode;
#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif


    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

    command.reset(new CIMRepositoryArchiveCommand());

    try
    {
        command->setCommand(argc, argv);
    }
    catch (CommandFormatException& cfe)
    {
        cerr << COMMAND_NAME << ": " << cfe.getMessage() << endl;

        MessageLoaderParms parms(
                "Clients.cimreparchive.CIMRepositoryArchiveCommand.ERR_USAGE",
                "Use '--help' to obtain command syntax.");
        parms.msg_src_path = MSG_PATH;
        cerr << COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;

        return EXIT_STATUS_GENERAL_ERROR;
    }

    retCode = command->execute(cout, cerr);

    return retCode;
}
