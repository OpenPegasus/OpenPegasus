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

#if !defined(PEGASUS_OS_ZOS) && \
    !defined(PEGASUS_OS_DARWIN)
# include <crypt.h>
#endif

PEGASUS_NAMESPACE_BEGIN

Boolean System::canRead(const char* path)
{
    return access(path, R_OK) == 0;
}

Boolean System::canWrite(const char* path)
{
    return access(path, W_OK) == 0;
}

String System::getPassword(const char* prompt)
{
#if  defined(PEGASUS_OS_PASE)

    char* umepass = umeGetPass();
    if(NULL == umepass)
    {
        return String::EMPTY;
    }
    else
    {
        return String(umepass);
    }

#else /* default */

    return String(getpass(prompt));

#endif /* default */

}

String System::encryptPassword(const char* password, const char* salt)
{
    return String(crypt(password, salt));
}

Boolean System::isPrivilegedUser(const String& userName)
{
#if defined(PEGASUS_OS_PASE)
    CString user = userName.getCString();
    // this function only can be found in PASE environment
    return umeIsPrivilegedUser((const char *)user);

#else
    struct passwd   pwd;
    struct passwd   *result;
    const unsigned int PWD_BUFF_SIZE = 1024;
    char            pwdBuffer[PWD_BUFF_SIZE];

    if (getpwnam_r(
          userName.getCString(), &pwd, pwdBuffer, PWD_BUFF_SIZE, &result) != 0)
    {
        PEG_TRACE((
            TRC_OS_ABSTRACTION,
            Tracer::LEVEL1,
            "getpwnam_r failure : %s",
            strerror(errno)));
    }

    // Check if the requested entry was found. If not return false.
    if ( result != NULL )
    {
        // Check if the uid is 0.
        if ( pwd.pw_gid == 0 || pwd.pw_uid == 0 )
        {
            return true;
        }
    }
    return false;
#endif
}

#if defined(PEGASUS_ENABLE_USERGROUP_AUTHORIZATION)

static void doFreeIfNeeded( const Boolean freeNeeded, char* ptrToFree)
{
    if (freeNeeded)
    {
        free(ptrToFree);
        ptrToFree = NULL;
    }
}

Boolean System::isGroupMember(const char* userName, const char* groupName)
{
    struct group grp;
    Boolean retVal = false;
    const  Uint32 PWD_BUFF_SIZE = 1024;
    struct passwd pwd;
    struct passwd* result;
    struct group* grpresult;
    char pwdBuffer[PWD_BUFF_SIZE];
    // Search Primary group information.

    // Find the entry that matches "userName"
    Sint32 errCode = 0;
    if((errCode = getpwnam_r(userName, &pwd, pwdBuffer,
        PWD_BUFF_SIZE, &result)) != 0)
    {
        String errorMsg = String("getpwnam_r failure : ") +
                            String(strerror(errCode));
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                                  errorMsg);
        throw InternalSystemError();
    }

    Uint32 grpbuflen = 1024; 

    //tell if we need to free due to dynamically allocated memory
    Boolean isDynamicMemory = false;
    char grpBuf[grpbuflen];
    char* grpBuffer = grpBuf;


    if ( result != NULL )
    {

        // User found, check for group information.
        gid_t group_id;
        group_id = pwd.pw_gid;


        //getgrgid_r may failed with groups with large number of users in group
        //Hence use loop here if ERANGE occurs 
        while((errCode = getgrgid_r(group_id, &grp, grpBuffer,
            grpbuflen, &grpresult) == ERANGE))
        {
           
            grpBuffer = (!isDynamicMemory) ? NULL: grpBuffer; 
            isDynamicMemory = true; 
            grpbuflen *= 2;
            
            grpBuffer = (char*)peg_inln_realloc( grpBuffer, grpbuflen);
        }

        if (errCode != 0 )
        {
            String errorMsg = String("getgrgid_r failure : ") +
                String(strerror(errCode));
        }
  

        // Compare the user's group name to groupName.
        if (strcmp(grp.gr_name, groupName) == 0)
        {
            doFreeIfNeeded(isDynamicMemory, grpBuffer);

             // User is a member of the group.
             return true;
        }
    }

    // Search supplemental groups.
    // Get a user group entry

    // The grpbuflen which has been calculated 
    // succeed getgrgid_r is good enough
    // for getgrnam_r so no need to repeat 
    // above step for getgrnam_r

    errCode = 0;
    if ((errCode = getgrnam_r((char *)groupName, &grp,
        grpBuffer, grpbuflen, &grpresult)) != 0)
    {
        doFreeIfNeeded(isDynamicMemory, grpBuffer);
        String errorMsg = String("getgrnam_r failure : ") +
            String(strerror(errCode));
        Logger::put(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING, errorMsg);
        throw InternalSystemError();
    }

    // Check if the requested group was found.
    if (grpresult == NULL)
    {
        doFreeIfNeeded(isDynamicMemory, grpBuffer);
        return false;
    }


    //
    // Get all the members of the group
    //
    Uint32 j = 0;
    char *member = NULL;
    while ( (member = grp.gr_mem[j++]) )
    {
        //
        // Check if the user is a member of the group
        //
        if ( strcmp(userName, member) == 0 )
        {
            retVal = true;
            break;
        }
    }

    doFreeIfNeeded( isDynamicMemory, grpBuffer);
    return retVal;
}

#endif /* PEGASUS_ENABLE_USERGROUP_AUTHORIZATION */


PEGASUS_NAMESPACE_END
