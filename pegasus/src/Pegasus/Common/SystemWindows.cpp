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

#include "System.h"

#include "Network.h"
#include "Mutex.h"
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include <io.h>
#include <conio.h>
#include <direct.h>
#include <sys/types.h>
#include <windows.h>
#include <process.h>
#include <lm.h>
#include <Pegasus/Common/Tracer.h>

#define SECURITY_WIN32
#include <security.h>

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ACCESS_EXISTS 0
#define PEGASUS_ACCESS_WRITE 2
#define PEGASUS_ACCESS_READ 4
#define PEGASUS_ACCESS_READ_AND_WRITE 6

#define PW_BUFF_LEN 65

void System::getCurrentTime(Uint32& seconds, Uint32& milliseconds)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER largeInt = { ft.dwLowDateTime, ft.dwHighDateTime };
    largeInt.QuadPart -= 0x19db1ded53e8000;
    seconds = long(largeInt.QuadPart / (10000 * 1000));
    milliseconds = long((largeInt.QuadPart % (10000 * 1000)) / 10);
    // This is a real hack. Added the following line after timevalue was
    // corrected and this apparently wrong. ks 7 apri 2002
    milliseconds = milliseconds / 1000;
}

void System::getCurrentTimeUsec(Uint32& seconds, Uint32& microseconds)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER largeInt = { ft.dwLowDateTime, ft.dwHighDateTime };
    largeInt.QuadPart -= 0x19db1ded53e8000;
    seconds = long(largeInt.QuadPart / (10000 * 1000));
    microseconds = long((largeInt.QuadPart % (10000 * 1000)) / 10);
}

Uint64 System::getCurrentTimeUsec()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER largeInt = { ft.dwLowDateTime, ft.dwHighDateTime };
    largeInt.QuadPart -= 0x19db1ded53e8000;

    return Uint64(largeInt.QuadPart / 10);
}

String System::getCurrentASCIITime()
{
    char tmpbuf[128];
    _strdate( tmpbuf );
    String date = tmpbuf;
    _strtime( tmpbuf );
    date.append("-");
    date.append(tmpbuf);
    return date;
}

void System::sleep(Uint32 seconds)
{
    Sleep(seconds * 1000);
}

Boolean System::exists(const char* path)
{
    return _access(path, PEGASUS_ACCESS_EXISTS) == 0;
}

Boolean System::canRead(const char* path)
{
    return _access(path, PEGASUS_ACCESS_READ) == 0;
}

Boolean System::canWrite(const char* path)
{
    return _access(path, PEGASUS_ACCESS_WRITE) == 0;
}

Boolean System::getCurrentDirectory(char* path, Uint32 size)
{
    return GetCurrentDirectory(size, path) != 0;
}

Boolean System::isDirectory(const char* path)
{
    struct stat st;

    if (stat(path, &st) != 0)
        return false;

    return (st.st_mode & _S_IFDIR) != 0;
}

Boolean System::changeDirectory(const char* path)
{
    return chdir(path) == 0;
}

Boolean System::makeDirectory(const char* path)
{
    return _mkdir(path) == 0;
}

Boolean System::getFileSize(const char* path, Uint32& size)
{
    struct stat st;

    if (stat(path, &st) != 0)
        return false;

    size = st.st_size;
    return true;
}

Boolean System::removeDirectory(const char* path)
{
    return rmdir(path) == 0;
}

Boolean System::removeFile(const char* path)
{
    return unlink(path) == 0;
}

Boolean System::renameFile(const char* oldPath, const char* newPath)
{
    if (exists(oldPath))
    {
        removeFile(newPath);
    }
    return rename(oldPath, newPath) == 0;
}

String System::getSystemCreationClassName ()
{
    return "CIM_ComputerSystem";
}

Uint32 System::lookupPort(
    const char * serviceName,
    Uint32 defaultPort)
{
    Uint32 localPort;

    struct servent *serv;

    //
    // Get the port number.
    //
    if ( (serv = getservbyname(serviceName, TCP)) != NULL )
    {
        localPort = ntohs(serv->s_port);
    }
    else
    {
        localPort = defaultPort;
    }

    return localPort;
}

String System::getPassword(const char* prompt)
{
    char password[PW_BUFF_LEN] = {0};
    int num_chars = 0;
    int ch;

    fputs(prompt, stderr);

    while ((ch = _getch()) != '\r' &&
            num_chars < PW_BUFF_LEN)
        {
        // EOF
        if (ch == EOF)
        {
            fputs("[EOF]\n", stderr);
            return String::EMPTY;
        }
        // Backspace or Delete
        else if ((ch == '\b' || ch == 127) &&
                num_chars > 0)
        {
            password[--num_chars] = '\0';
            fputs("\b \b", stderr);
        }
        // CTRL+C
        else if (ch == 3)
        {
            // _getch() does not catch CTRL+C
            fputs("^C\n", stderr);
            exit(-1);
        }
        // CTRL+Z
        else if (ch == 26)
        {
            fputs("^Z\n", stderr);
            return String::EMPTY;
        }
        // Esc
        else if (ch == 27)
        {
            fputc('\n', stderr);
            fputs(prompt, stderr);
            num_chars = 0;
        }
        // Function keys (0 or E0) are a guards for a Function key codes
        else if (ch == 0 || ch == 0xE0)
        {
            ch = (ch << 4) | _getch();
            // Handle DELETE, left arrow, keypad DEL, and keypad left arrow
            if ((ch == 0xE53 || ch == 0xE4B || ch == 0x053 || ch == 0x04b) &&
                    num_chars > 0)
            {
                password[--num_chars] = '\0';
                fputs("\b \b", stderr);
            }
            else
            {
                fputc('\a', stderr);
            }
        }
        else if ((num_chars < sizeof(password) - 1) &&
                    !iscntrl(((unsigned char)(ch))))
        {
            password[num_chars++] = ch;
            fputc('*', stderr);
        }
        else
        {
            fputc('\a', stderr);
        }
    }

    fputc('\n', stderr);
    password[num_chars] = '\0';

    return String(password);
}

String System::getEffectiveUserName()
{
#if (_MSC_VER >= 1300) || defined(PEGASUS_WINDOWS_SDK_HOME)

    //Bug 3076 fix
    wchar_t fullUserName[UNLEN+1];
    DWORD userNameSize = sizeof(fullUserName)/sizeof(fullUserName[0]);
    wchar_t computerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD computerNameSize = sizeof(computerName)/sizeof(computerName[0]);
    wchar_t userName[UNLEN+1];
    wchar_t userDomain[UNLEN+1];
    String userId;

    if (!GetUserNameExW(NameSamCompatible, fullUserName, &userNameSize))
    {
        return String();
    }

    wchar_t* index = wcschr(fullUserName, '\\');
    *index = 0;
    wcscpy(userDomain, fullUserName);
    wcscpy(userName, index + 1);

    //The above function will return the system name as the domain if
    //the user is not on a real domain.  Strip this out so that the rest of
    //our windows user functions work.  What if the system name and the domain
    //name are the same?
    GetComputerNameW(computerName, &computerNameSize);

    if (wcscmp(computerName, userDomain) != 0)
    {
        //userId.append(userDomain);
        Uint32 n = (Uint32)wcslen(userDomain);
        for (unsigned long i = 0; i < n; i++)
        {
            userId.append(Char16(userDomain[i]));
        }
        userId.append("\\");
        //userId.append(userName);
        n = (Uint32)wcslen(userName);
        for (unsigned long i = 0; i < n; i++)
        {
            userId.append(Char16(userName[i]));
        }

    }
    else
    {
        //userId.append(userName);
        Uint32 n = (Uint32)wcslen(userName);
        for (unsigned long i = 0; i < n; i++)
        {
            userId.append(Char16(userName[i]));
        }

    }

    return userId;

#else //original getEffectiveUserName function

    int retcode = 0;

    // UNLEN (256) is the limit, not including null
    wchar_t pUserName[256+1] = {0};
    DWORD nSize = sizeof(pUserName)/sizeof(pUserName[0]);

    retcode = GetUserNameW(pUserName, &nSize);
    if (retcode == 0)
    {
        // zero is failure
        return String();
    }
    String userId;
    Uint32 n = wcslen(pUserName);
    for (unsigned long i = 0; i < n; i++)
    {
        userId.append(Char16(pUserName[i]));
    }

    return userId;
#endif
}

String System::encryptPassword(const char* password, const char* salt)
{
    BYTE pbBuffer[PW_BUFF_LEN] = {0};
    DWORD dwByteCount;
    char pcSalt[3] = {0};

    strncpy(pcSalt, salt, 2);
    dwByteCount = (DWORD)strlen(password);
    memcpy(pbBuffer, password, dwByteCount);
    for (DWORD i=0; (i<dwByteCount) || (i>=PW_BUFF_LEN); i++)
            (i%2 == 0) ? pbBuffer[i] ^= pcSalt[1] : pbBuffer[i] ^= pcSalt[0];

    return String(pcSalt) + String((char *)pbBuffer);
}

String processUserName;
Mutex processUserNameMut;

Boolean System::isSystemUser(const char* userName)
{
    if (processUserName.size() == 0)
    {
        // Lock and recheck the processUserName length in case two threads
        // enter this block simultaneously
        AutoMutex mut(processUserNameMut);
        if (processUserName.size() == 0)
        {
            processUserName = getEffectiveUserName();
        }
    }
    if (processUserName == userName)
    {
      return true;
    }

    Boolean isSystemUser = false;

    char mUserName[UNLEN+1];
    char mDomainName[UNLEN+1];
    char tUserName[UNLEN+1];
    wchar_t wUserName[UNLEN+1];
    wchar_t wDomainName[UNLEN+1];
    char* pbs;
    bool usingDomain = false;

    LPBYTE pComputerName=NULL;
    DWORD dwLevel = 1;
    LPUSER_INFO_1 pUserInfo = NULL;
    NET_API_STATUS nStatus = NULL;

    // Make a copy of the specified username, it cannot be used directly
    // because it's declared as const and strchr() may modify the string.
    strncpy(tUserName, userName, sizeof(tUserName) - 1);
    tUserName[sizeof(tUserName)- 1] = '\0';

    //separate the domain and user name if both are present.
    if (NULL != (pbs = strchr(tUserName, '\\')))
    {
        *pbs = '\0';
        strcpy(mDomainName, tUserName);
        strcpy(mUserName, pbs+1);
        usingDomain = true;

    }
    else if ((NULL != (pbs = (strchr(tUserName, '@')))) ||
             (NULL != (pbs = (strchr(tUserName, '.')))))
    {
        *pbs = '\0';
        strcpy(mDomainName, pbs+1);
        strcpy(mUserName, tUserName);
        usingDomain = true;

    }
    else
    {
        strcpy(mDomainName, ".");
        strcpy(mUserName, tUserName);
    }

    //convert domain name to unicode
    if (!MultiByteToWideChar(
            CP_ACP, 0, mDomainName, -1, wDomainName,
            (int)(strlen(mDomainName) + 1)))
    {
        return false;
    }

    //convert username to unicode
    if (!MultiByteToWideChar(
            CP_ACP, 0, mUserName, -1, wUserName, (int)(strlen(mUserName) + 1)))
    {
        return false;
    }

    if (usingDomain)
    {
        //get domain controller
        DWORD rc = NetGetDCName(NULL, wDomainName, &pComputerName);
        if (rc == NERR_Success)
        {
            // this is automatically prefixed with "\\"
            wcscpy(wDomainName, (LPWSTR) pComputerName);
        }
        /*
        else
        {
            // failover
            // ATTN: This is commented out until there is resolution on
            // Bugzilla 2236. -hns 2/2005
            // This needs to be more thoroughly tested when we uncomment it out.

            PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;

            //this function does not take wide strings
            rc = DsGetDcName(NULL,
                             mDomainName,
                             NULL,
                             NULL,
                             //not sure what flags we want here
                             DS_DIRECTORY_SERVICE_REQUIRED,
                             &DomainControllerInfo);

            if (rc == ERROR_SUCCESS && DomainControllerInfo)
            {
                strcpy(mDomainName, DomainControllerInfo->DomainName);
                NetApiBufferFree(DomainControllerInfo);

                if (!MultiByteToWideChar(
                        CP_ACP, 0, mDomainName, -1, wDomainName,
                        strlen(mDomainName) + 1))
                {
                    return false;
                }
            }
        }
        */
    }

    //get user info
    nStatus = NetUserGetInfo(wDomainName,
                             wUserName,
                             dwLevel,
                             (LPBYTE *)&pUserInfo);

    if (nStatus == NERR_Success)
    {
        isSystemUser = true;
    }

    if (pComputerName != NULL)
    {
        NetApiBufferFree(pComputerName);
    }

    if (pUserInfo != NULL)
    {
        NetApiBufferFree(pUserInfo);
    }

    return isSystemUser;
}


Boolean System::isPrivilegedUser(const String& userName)
{
    Boolean isPrivileged = false;

    char mUserName[UNLEN+1];
    char mDomainName[UNLEN+1];
    wchar_t wUserName[UNLEN+1];
    wchar_t wDomainName[UNLEN+1];
    char* pbs;
    char userStr[UNLEN+1];
    bool usingDomain = false;

    LPBYTE pComputerName=NULL;
    DWORD dwLevel = 1;
    LPUSER_INFO_1 pUserInfo = NULL;
    NET_API_STATUS nStatus = NULL;

    //get the username in the correct format
    strcpy(userStr, (const char*)userName.getCString());

    //separate the domain and user name if both are present.
    if (NULL != (pbs = strchr(userStr, '\\')))
    {
        *pbs = '\0';
        strcpy(mDomainName, userStr);
        strcpy(mUserName, pbs+1);
        usingDomain = true;

    }
    else if ((NULL != (pbs = (strchr(userStr, '@')))) ||
             (NULL != (pbs = (strchr(userStr, '.')))))
    {
        *pbs = '\0';
        strcpy(mDomainName, pbs+1);
        strcpy(mUserName, userStr);
        usingDomain = true;

    }
    else
    {
        strcpy(mDomainName, ".");
        strcpy(mUserName, userStr);
    }

    //convert domain name to unicode
    if (!MultiByteToWideChar(
            CP_ACP, 0, mDomainName, -1, wDomainName,
            (int)(strlen(mDomainName) + 1)))
    {
        return false;
    }

    //convert username to unicode
    if (!MultiByteToWideChar(
            CP_ACP, 0, mUserName, -1, wUserName, (int)(strlen(mUserName) + 1)))
    {
        return false;
    }

    if (usingDomain)
    {
        //get domain controller
        DWORD rc = NetGetDCName(NULL, wDomainName, &pComputerName);
        if (rc == NERR_Success)
        {
            // this is automatically prefixed with "\\"
            wcscpy(wDomainName, (LPWSTR) pComputerName);
        }
        /*
        else
        {
            // failover
            // ATTN: This is commented out until there is resolution on
            // Bugzilla 2236. -hns 2/2005
            // This needs to be more thoroughly tested when we uncomment it out.

            PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;

            //this function does not take wide strings
            rc = DsGetDcName(NULL,
                             mDomainName,
                             NULL,
                             NULL,
                             // not sure what flags we want here
                             DS_DIRECTORY_SERVICE_REQUIRED,
                             &DomainControllerInfo);

            if (rc == ERROR_SUCCESS && DomainControllerInfo)
            {
                strcpy(mDomainName, DomainControllerInfo->DomainName);
                NetApiBufferFree(DomainControllerInfo);

                if (!MultiByteToWideChar(
                        CP_ACP, 0, mDomainName, -1, wDomainName,
                        strlen(mDomainName) + 1))
                {
                    return false;
                }
            }
        }
        */
    }

    //get privileges
    nStatus = NetUserGetInfo(wDomainName,
                             wUserName,
                             dwLevel,
                             (LPBYTE *)&pUserInfo);

    if ((nStatus == NERR_Success) &&
        (pUserInfo != NULL) &&
        (pUserInfo->usri1_priv == USER_PRIV_ADMIN))
    {
        isPrivileged = true;
    }

    if (pComputerName != NULL)
    {
        NetApiBufferFree(pComputerName);
    }

    if (pUserInfo != NULL)
    {
        NetApiBufferFree(pUserInfo);
    }

    return isPrivileged;
}

String System::getPrivilegedUserName()
{
    // ATTN-NB-03-20000304: Implement better way to get the privileged
    // user on the system.

    return String("Administrator");
}

Boolean System::isGroupMember(const char* userName, const char* groupName)
{
    Boolean retVal = false;

    LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
    DWORD dwLevel = 0;
    DWORD dwFlags = LG_INCLUDE_INDIRECT ;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;
    wchar_t wcUserName[UNLEN+1];
    wchar_t wcGroupName[UNLEN+1];

    //Convert user name to unicode
    if (!MultiByteToWideChar(CP_ACP,0,userName, -1, wcUserName,
        strlen(userName)+1))
    {
        return false;
    }

    //Convert group name to unicode
    if (!MultiByteToWideChar(CP_ACP, 0, groupName, -1, wcGroupName,
        strlen(groupName)+1))
    {
        return false;
    }

    //
    // Call the NetUserGetLocalGroups function
    // specifying information level 0.
    //
    // The LG_INCLUDE_INDIRECT flag specifies that the
    // function should also return the names of the local
    // groups in which the user is indirectly a member.
    //
    nStatus = NetUserGetLocalGroups(
        NULL,
        (LPCWSTR)wcUserName,
        dwLevel,
        dwFlags,
        (LPBYTE *) &pBuf,
        dwPrefMaxLen,
        &dwEntriesRead,
        &dwTotalEntries);

    //
    // If the call succeeds,
    //
    if (nStatus == NERR_Success)
    {
        LPLOCALGROUP_USERS_INFO_0 pTmpBuf;
        DWORD i;
        DWORD dwTotalCount = 0;

        if ((pTmpBuf = pBuf) != NULL)
        {
            //
            // Loop through the local groups that the user belongs
            // and find the matching group name.
            //
            for (i = 0; i < dwEntriesRead; i++)
            {
                //
                // Compare the user's group name to groupName.
                //

                if (wcscmp(pTmpBuf->lgrui0_name, wcGroupName) == 0)
                {
                    // User is a member of the group.
                    retVal = true;
                    break;
                }

                pTmpBuf++;
                dwTotalCount++;
            }
        }
    }

    //
    // Free the allocated memory.
    //
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);

    //
    // If the given user and group are not found in the local group
    // then try on the global groups.
    //
    if (!retVal)
    {
        LPGROUP_USERS_INFO_0 pBuf = NULL;
        dwLevel = 0;
        dwPrefMaxLen = MAX_PREFERRED_LENGTH;
        dwEntriesRead = 0;
        dwTotalEntries = 0;

        //
        // Call the NetUserGetGroups function, specifying level 0.
        //
        nStatus = NetUserGetGroups(
            NULL,
            (LPCWSTR)wcUserName,
            dwLevel,
            (LPBYTE*)&pBuf,
            dwPrefMaxLen,
            &dwEntriesRead,
            &dwTotalEntries);

        //
        // If the call succeeds,
        //
        if (nStatus == NERR_Success)
        {
            LPGROUP_USERS_INFO_0 pTmpBuf;
            DWORD i;
            DWORD dwTotalCount = 0;

            if ((pTmpBuf = pBuf) != NULL)
            {
                //
                // Loop through the global groups to which the user belongs
                // and find the matching group name.
                //
                for (i = 0; i < dwEntriesRead; i++)
                {
                    //
                    // Compare the user's group name to groupName.
                    //
                    if (wcscmp(pTmpBuf->grui0_name, wcGroupName) == 0)
                    {
                        // User is a member of the group.
                        retVal = true;
                        break;
                    }

                    pTmpBuf++;
                    dwTotalCount++;
                }
            }
        }

        //
        // Free the allocated buffer.
        //
        if (pBuf != NULL)
            NetApiBufferFree(pBuf);
    }

    return retVal;
}

Boolean System::lookupUserId(
    const char* userName,
    PEGASUS_UID_T& uid,
    PEGASUS_GID_T& gid)
{
    // ATTN: Implement this method to look up the specified user
    return false;
}

Boolean System::changeUserContext_SingleThreaded(
    const char* userName,
    const PEGASUS_UID_T& uid,
    const PEGASUS_GID_T& gid)
{
    // ATTN: Implement this method to change the process user context to the
    //       specified user
    return false;
}

Uint32 System::getPID()
{
    return _getpid();
}

Boolean System::truncateFile(
    const char* path,
    size_t newSize)
{

    Boolean rv = false;
    int fd = open(path, O_RDWR);
    if (fd != -1)
    {
        if (chsize(fd, (long)newSize) == 0)
        {
            rv = true;
        }

        close(fd);
    }

    return rv;
}

// Is absolute path?
Boolean System::is_absolute_path(const char *path)
{
    char full[_MAX_PATH];
    char path_slash[_MAX_PATH];
    char *p;

    strncpy(path_slash, path, _MAX_PATH);
    path_slash[_MAX_PATH-1] = '\0';

    for (p = path_slash; p < path_slash + strlen(path_slash); p++)
      if (*p == '/')
          *p = '\\';

    return (strcasecmp(
        _fullpath(full, path_slash, _MAX_PATH), path_slash) == 0);
}

// Changes file permissions on the given file.
Boolean System::changeFilePermissions(const char* path, mode_t mode)
{
    // ATTN: File permissions are not currently defined in Windows
    return true;
}

Boolean System::verifyFileOwnership(const char* path)
{
    // ATTN: Implement this to check that the owner of the specified file is
    //       the same as the effective user for this process.
    return true;
}

void System::syslog(const String& ident, Uint32 severity, const char* message)
{
    // Not implemented
}

void System::openlog(const char *ident, int logopt, int facility)
{
    // Not implemented
}

void System::closelog()
{
    // Not implemented
}



// System ID constants for Logger::put and Logger::trace
const String System::CIMSERVER = "cimserver";  // Server system ID

// check if a given IP address is defined on the local network interfaces
Boolean System::isIpOnNetworkInterface(Uint32 inIP)
{
    Socket::initializeInterface();
    SOCKET sock;
    int interfaces = 0;
    int errcode;

    if ( SOCKET_ERROR != ( sock  = WSASocket(AF_INET,
                         SOCK_RAW, 0, NULL, 0, 0) ) )
    {
        unsigned long *bytes_returned=0;
        char *output_buf = (char *)calloc(1, 256);
        int buf_size = 256;

        if ( 0 == (errcode = WSAIoctl(sock,
                                      SIO_ADDRESS_LIST_QUERY,
                                      NULL,
                                      0,
                                      output_buf,
                                      256,
                                      bytes_returned,
                                      NULL,
                                      NULL)) )
        {
            SOCKET_ADDRESS_LIST *addr_list;
            SOCKET_ADDRESS *addr;
            Uint32 ip;
            struct sockaddr_in *sin;

            addr_list = (SOCKET_ADDRESS_LIST *)output_buf;
            addr = addr_list->Address;

            sin = (struct sockaddr_in *)addr->lpSockaddr;

            for ( ; interfaces < addr_list->iAddressCount; interfaces++)
            {
                ip = sin->sin_addr.s_addr;
                addr++;
                sin = (struct sockaddr_in *)addr->lpSockaddr;
                if (ip == inIP)
                {
                    free(output_buf);
                    closesocket(sock);
                    Socket::uninitializeInterface();
                    return true;
                }
            }
        }
        else
        {
            free(output_buf);
            Socket::uninitializeInterface();
            return false;
        }
        free(output_buf);
        closesocket(sock);
        Socket::uninitializeInterface();
    }
    return false;
}

void _getInterfaceAddrs(Array<String> &ips, int af)
{
    SOCKET sock;

    if (INVALID_SOCKET != (sock = WSASocket(af, SOCK_RAW,
        0, NULL, 0, 0)))
    {
        DWORD  bytesReturned;
        char buf[2048];
        int interfaces = 0;
        char str[PEGASUS_INET6_ADDRSTR_LEN];
        void *p = 0;
        if (0 == WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY, NULL, 0,
            buf, 2048, &bytesReturned, NULL,
            NULL))
        {

            SOCKET_ADDRESS_LIST *addr_list;
            SOCKET_ADDRESS *addr;
            struct sockaddr *sin;
            addr_list = (SOCKET_ADDRESS_LIST *)buf;
            addr = addr_list->Address;
            int rc = 0;
            for (sin = (struct sockaddr *) addr->lpSockaddr ;
                interfaces < addr_list->iAddressCount;
                interfaces++)
            {
                if (af == AF_INET)
                {
                    p = &((struct sockaddr_in*)sin)->sin_addr;
                }
#ifdef PEGASUS_ENABLE_IPV6
                else
                {
                    p = &((struct sockaddr_in6*)sin)->sin6_addr;
                }
#endif
                // Don't gather loopback addrs
                if (!System::isLoopBack(af, p))
                {
                    if (af == AF_INET)
                    {
                        if ((rc = ::getnameinfo( 
                            sin, 
                            sizeof(struct sockaddr_in),
                            str,
                            sizeof(str),
                            NULL,
                            0,
                            NI_NUMERICHOST)) == 0)
                        {
                            ips.append(str);
                        }
                        //Error detected in getting name info, 
                        //display the error string
                        else
                        {
                            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
                                "getnameinfo failed: %s", gai_strerror(rc)));
                        }
                    }
#ifdef PEGASUS_ENABLE_IPV6
                    else if (af == AF_INET6)
                    {
                        if((rc = System::getNameInfo( 
                            sin,
                            sizeof(struct sockaddr_in6),
                            str,
                            sizeof(str),
                            NULL,
                            0,
                            NI_NUMERICHOST)) == 0)
                        {
                            ips.append(str);
                        }
                    }
#endif
                }
                ++addr;
                sin = (struct sockaddr*)addr->lpSockaddr;
            }
        }
    }
}

Array<String> System::getInterfaceAddrs()
{
    Socket::initializeInterface();
    Array<String> ips;
    _getInterfaceAddrs(ips, AF_INET);
#ifdef PEGASUS_ENABLE_IPV6
    _getInterfaceAddrs(ips, AF_INET6);
#endif
    Socket::uninitializeInterface();
    return ips;
}

String System::getErrorMSG_NLS(int errorCode,int errorCode2)
{
    LPVOID winErrorMsg = NULL;

    if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&winErrorMsg,
            0,
            NULL))
    {
        MessageLoaderParms parms(
            "Common.System.ERROR_MESSAGE.STANDARD",
            "$0 (error code $1)",(char*)winErrorMsg,errorCode);
        LocalFree(winErrorMsg);
        return MessageLoader::getMessage(parms);
    }

    MessageLoaderParms parms(
        "Common.System.ERROR_MESSAGE.STANDARD",
        "$0 (error code $1)","",errorCode);
    return MessageLoader::getMessage(parms);

}

String System::getErrorMSG(int errorCode,int errorCode2)
{

    String buffer;
    LPVOID winErrorMsg = NULL;

    char strErrorCode[32];
    sprintf(strErrorCode, "%d", errorCode);

    if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&winErrorMsg,
            0,
            NULL))
    {
        buffer.append((char*)winErrorMsg);
        LocalFree(winErrorMsg);
    }

    buffer.append(" (error code ");
    buffer.append(strErrorCode);
    buffer.append(")");

    return buffer;
}


///////////////////////////////////////////////////////////////////////////////
// AutoFileLock class
///////////////////////////////////////////////////////////////////////////////

AutoFileLock::AutoFileLock(const char* fileName)
{
   // Repeat createFile, if there is a sharing violation.
   do
   {
       _hFile = CreateFile (fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   }while ((GetLastError() == ERROR_SHARING_VIOLATION));

   // If this conditon succeeds, There is an error opening the file. Hence
   // returning from here  as Lock can not be acquired.
   if((_hFile == INVALID_HANDLE_VALUE)
       && (GetLastError() != ERROR_ALREADY_EXISTS)
       && (GetLastError() != ERROR_SUCCESS))
   {
       PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
          "AutoFileLock: Failed to open lock file '%s', error code %d.",
          fileName, GetLastError()));
       return;
   }

   OVERLAPPED l={0,0,0,0,0};
   if(LockFileEx(_hFile,LOCKFILE_EXCLUSIVE_LOCK, 0, 0, 0, &l) == 0)
   {
       PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
           "AutoFileLock: Failed to Acquire lock on file %s, error code %d.",
           fileName, GetLastError()));
       CloseHandle(_hFile);
       _hFile = INVALID_HANDLE_VALUE;
   }
}

AutoFileLock::~AutoFileLock()
{
    if(_hFile != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED l={0,0,0,0,0};
        if(UnlockFileEx (_hFile, 0, 0, 0, &l) == 0)
        {
           PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
               "AutoFileLock: Failed to unlock file, error code %d.",
                GetLastError()));
        }
        CloseHandle(_hFile);
    }
}


PEGASUS_NAMESPACE_END
