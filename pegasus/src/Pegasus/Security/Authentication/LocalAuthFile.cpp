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

#include "LocalAuthFile.h"

#include <fstream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef PEGASUS_OS_TYPE_WINDOWS
# include <sys/time.h>
# include <unistd.h>
#endif

#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Config/ConfigManager.h>

#include "LocalAuthFile.h"

#if defined(PEGASUS_OS_TYPE_WINDOWS)
// srandom() and random() are srand() and rand() on OS/400 and windows
# define srandom(x) srand(x)
# define random() rand()
#endif


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//
// Constant representing the random seed for srandom function
//
const Uint32 RANDOM_SEED     =  100;

//
// File path used to create temporary random file
//
const char TMP_AUTH_FILE_NAME []  =  "/cimclient_";

//
// Constant representing the int buffer size
//
const Uint32 INT_BUFFER_SIZE = 16;

//
// Constant representing the random device name
//
const char DEV_URANDOM []    =  "/dev/urandom";

//
// Constant representing the size of random entropy needed.
// Require minimum 160 bits = 20 bytes of randomness.
//
const Uint32 RANDOM_BYTES_NEEDED = 20;

//
// A unique sequence number used in random file name creation.
//
static Uint32 sequenceCount = 1;
static Mutex sequenceCountLock;



LocalAuthFile::LocalAuthFile(const String& userName)
    : _userName(userName)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "LocalAuthFile::LocalAuthFile()");

    srandom(RANDOM_SEED);

    //
    // get the configured temporary authentication file path
    //
    _authFilePath = PEGASUS_LOCAL_AUTH_DIR;

#if defined(PEGASUS_OS_TYPE_WINDOWS)
    System::makeDirectory((const char *)_authFilePath.getCString());
#endif
    _authFilePath.append(TMP_AUTH_FILE_NAME);

    PEG_METHOD_EXIT();
}

LocalAuthFile::~LocalAuthFile()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "LocalAuthFile::~LocalAuthFile()");

    PEG_METHOD_EXIT();
}

//
// create a file and make a random token data entry to it.
// send the file name back to caller
//
String LocalAuthFile::create()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "LocalAuthFile::create()");

    Uint32 secs, milliSecs;
    Uint32 mySequenceNumber;

    System::getCurrentTime(secs, milliSecs);

    //
    // extension size is username plus the sequence count
    //
    {
        AutoMutex autoMut(sequenceCountLock);
        mySequenceNumber = sequenceCount++;
    } // mutex unlocks here

    char extension[2*INT_BUFFER_SIZE];
    sprintf(extension,"_%u_%u", mySequenceNumber, milliSecs);
    extension[strlen(extension)] = 0;

    String filePath;

    // Check to see whether a domain was specified. If so, strip the domain
    // from the local auth file name, since the backslash and '@' are invalid
    // filename characters.  Store this in another variable, since we need to
    // keep the domain around for the rest of the operations.
    String fileUserName = _userName;
    Uint32 index = _userName.find('\\');
    if (index != PEG_NOT_FOUND)
    {
        fileUserName = _userName.subString(index + 1);
    }
    else
    {
        index = _userName.find('@');
        if (index != PEG_NOT_FOUND)
        {
            fileUserName = _userName.subString(0, index);
        }
    }

    filePath.append(_authFilePath);
    filePath.append(fileUserName);//_userName);
    filePath.append(extension);
    CString filePathCString = filePath.getCString();

    //
    // 1. Create a file name for authentication.
    //
    ofstream outfs(filePathCString);

    if (!outfs)
    {
        // unable to create file
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Security.Authentication.LocalAuthFile.NO_CREATE",
                "Creation of the local authentication security file"
                    " $0 failed: $1",
                filePath, strerror(errno)));
        PEG_METHOD_EXIT();
        throw CannotOpenFile (filePath);
    }
    outfs.clear();

    //
    // 2. Set file permission to read/write by the owner only.
    //

#if defined(PEGASUS_OS_TYPE_WINDOWS)
    Boolean success =
        FileSystem::changeFilePermissions(filePath, _S_IREAD | _S_IWRITE);
#else
    Boolean success =
        FileSystem::changeFilePermissions(filePath, S_IRUSR | S_IWUSR);
#endif

    if (!success)
    {
        // Unable to change the local auth file permissions, remove the file
        // and throw CannotOpenFile error.
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Security.Authentication.LocalAuthFile.NO_CHMOD",
                "Changing permissions of the local authentication security "
                    "file $0 failed: $1",
                filePath, strerror(errno)));

        if (filePath.size())
        {
            if (FileSystem::exists(filePath))
            {
                FileSystem::removeFile(filePath);
            }
        }

        PEG_METHOD_EXIT();
        throw CannotOpenFile (filePath);
    }

    //
    // 3. Generate random token and write the token to the file.
    //
    String randomToken = _generateRandomTokenString();
    outfs << randomToken;

    // test if the write was successful
    if (outfs.fail())
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Security.Authentication.LocalAuthFile.NO_WRITE",
                "Cannot write security token to the local authentication "
                    "security file $0.",
                filePath));

        if (filePath.size())
        {
            if (FileSystem::exists(filePath))
            {
                FileSystem::removeFile(filePath);
            }
        }

        PEG_METHOD_EXIT();
        throw CannotOpenFile (filePath);

    }

    outfs.close();

    //
    // 4. Set file permission to read only by the owner.
    //
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    success = FileSystem::changeFilePermissions(filePath, _S_IREAD);
#else
    success = FileSystem::changeFilePermissions(filePath, S_IRUSR);
#endif

    if (!success)
    {
        // Unable to change the local auth file permissions, remove the file
        // and throw CannotOpenFile error.
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Security.Authentication.LocalAuthFile.NO_CHMOD",
                "Changing permissions of the local authentication security "
                    "file $0 failed: $1",
                filePath, strerror(errno)));

        if (filePath.size())
        {
            if (FileSystem::exists(filePath))
            {
                FileSystem::removeFile(filePath);
            }
        }

        PEG_METHOD_EXIT();
        throw CannotOpenFile (filePath);
    }

    //
    // 5. Change the file owner to the requesting user.
    //

    if (!FileSystem::changeFileOwner(filePath,_userName))
    {
        // Unable to change owner on local auth file, remove the file
        // and throw CannotOpenFile error.

        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Security.Authentication.LocalAuthFile.NO_CHOWN_REQUSER",
                "Changing ownership of the local authentication "
                    "security file $0 to the requesting user failed: $1",
                filePath, strerror(errno)));

        if (filePath.size())
        {
            if (FileSystem::exists(filePath))
            {
                FileSystem::removeFile(filePath);
            }
        }

        PEG_METHOD_EXIT();
        throw CannotOpenFile (filePath);
    }

    _secret = randomToken;

    _filePathName = filePath;

    PEG_METHOD_EXIT();

    return _filePathName;
}

//
//  Removes the file that was created
//
Boolean LocalAuthFile::remove()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "LocalAuthFile::remove()");

    Boolean retVal = true;

    //
    // remove the file
    //
    if (FileSystem::exists(_filePathName))
    {

#if !defined(PEGASUS_OS_TYPE_WINDOWS)
        // change ownership back to the CIMOM
        int rc = chown(
            (const char *)_filePathName.getCString(), geteuid(), getegid());

        if (rc == -1)
        {
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
                MessageLoaderParms(
                    "Security.Authentication.LocalAuthFile.NO_CHOWN",
                    "Changing ownership of the local authentication "
                        "security file back to the CIMServer UserId failed."));
        }
#endif

        retVal = FileSystem::removeFile(_filePathName);
    }

    PEG_METHOD_EXIT();

    return retVal;
}

//
// Get the string that was created as a secret string
//
String LocalAuthFile::getSecretString()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "LocalAuthFile::getSecretString()");

    PEG_METHOD_EXIT();

    return _secret;
}


//
// Generate  random token string
//
String LocalAuthFile::_generateRandomTokenString()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "LocalAuthFile::_generateRandomTokenString()");

    String randomToken;

    String randFile = String(DEV_URANDOM);
    FILE *fh;

    //
    // If /dev/urandom exists then read random token from /dev/urandom
    //
    if ((fh = fopen( (const char *)randFile.getCString(), "r")) != NULL)
    {
        char token[RANDOM_BYTES_NEEDED+1];

        setvbuf(fh, NULL, _IONBF, 0);   // need unbuffered input
        Uint32 n = (Uint32)fread( (unsigned char *)token, 1,
            RANDOM_BYTES_NEEDED, fh );
        fclose(fh);
        token[n]=0;

        randomToken.clear();
        char hexChar[10];

        for (Uint32 i=0; i < n; i++)
        {
            sprintf(hexChar, "%X", (unsigned char)token[i]);
            randomToken.append(String(hexChar));
            memset(hexChar, 0x00, sizeof(hexChar));
        }
    }

    char token[2*INT_BUFFER_SIZE];
    Uint32 seconds, milliseconds;

    System::getCurrentTime(seconds, milliseconds);

    //
    // generate a random token
    //
    char randnum[] = {char('0'+(random() % 10)),char('0'+(random() % 10)),'\0'};
    long randomNum = atol(randnum);

    sprintf (token,"%ld%u", randomNum, seconds + milliseconds );
    token[strlen(token)] = 0;

    randomToken.append(_userName);
    randomToken.append(token);

    PEG_METHOD_EXIT();

    return randomToken;
}

PEGASUS_NAMESPACE_END
