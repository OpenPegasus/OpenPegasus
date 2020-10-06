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

#ifndef Pegasus_System_h
#define Pegasus_System_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Network.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Pegasus_inl.h>
#include <sys/stat.h>


#if defined(PEGASUS_OS_TYPE_WINDOWS)
#ifndef mode_t
typedef unsigned long mode_t;
#endif
#include <windows.h>
#endif

#if defined (PEGASUS_OS_TYPE_UNIX) || \
    defined (PEGASUS_OS_VMS)
# include <unistd.h>
# include <fcntl.h>  // File locking
# define PEGASUS_UID_T uid_t
# define PEGASUS_GID_T gid_t
#else
# define PEGASUS_UID_T Uint32
# define PEGASUS_GID_T Uint32
#endif

#if defined(PEGASUS_OS_TYPE_WINDOWS)
#  define PEGASUS_SYSTEM_ERRORMSG_NLS \
      System::getErrorMSG_NLS(GetLastError(),0)
#  define PEGASUS_SYSTEM_NETWORK_ERRORMSG_NLS \
      System::getErrorMSG_NLS(WSAGetLastError(),0)
#  define PEGASUS_SYSTEM_ERRORMSG \
      System::getErrorMSG(GetLastError(),0)
#  define PEGASUS_SYSTEM_NETWORK_ERRORMSG \
      System::getErrorMSG(WSAGetLastError(),0)
#elif defined(PEGASUS_OS_ZOS)
#  define PEGASUS_SYSTEM_ERRORMSG_NLS \
      System::getErrorMSG_NLS(errno,__errno2())
#  define PEGASUS_SYSTEM_NETWORK_ERRORMSG_NLS \
      System::getErrorMSG_NLS(errno,__errno2())
#  define PEGASUS_SYSTEM_ERRORMSG \
      System::getErrorMSG(errno,__errno2())
#  define PEGASUS_SYSTEM_NETWORK_ERRORMSG \
      System::getErrorMSG(errno,__errno2())
#else
#  define PEGASUS_SYSTEM_ERRORMSG_NLS \
      System::getErrorMSG_NLS(errno,0)
#  define PEGASUS_SYSTEM_NETWORK_ERRORMSG_NLS \
      System::getErrorMSG_NLS(errno,0)
#  define PEGASUS_SYSTEM_ERRORMSG \
      System::getErrorMSG(errno,0)
#  define PEGASUS_SYSTEM_NETWORK_ERRORMSG \
      System::getErrorMSG(errno,0)
#endif

//
// Protocal Type
//
#define TCP                        "tcp"

PEGASUS_NAMESPACE_BEGIN

/** The System class defines wrappers for operating system related calls.
    These are only placed here if they are extremely light. These are
    usually just direct wrappers which map more or less one to one to the
    underlying function.
*/
class PEGASUS_COMMON_LINKAGE System
{
public:

    /* Creates a String object containing the system message
       from  the errno and if supported from a second level error
       number. The _NLS Method is looking up an internationalized version of
       the message.
        @param errorCode  The system errno.
        @param errorCode2 The secondary error number like errno2 on z/OS
    */
    static String getErrorMSG_NLS(int errorCode,int errorCode2);
    static String getErrorMSG(int errorCode,int errorCode2);

    /** getCurrentTime - Gets the current time as seconds and milliseconds
    into the provided variables using system functions.
    @param seconds Return for the seconds component of the time.
    @param milliseconds Return for the milliseconds component of the time.
    @return The value is returned in the parameters.
    The time returned is as defined in number of seconds and milliseconds
    since 00:00 Coordinated Universal Time (UTC), January 1, 1970,

    */
    static void getCurrentTime(Uint32& seconds, Uint32& milliseconds);

    /** Similar to getCurrentTime() above but get microseconds (rather than
        milliseconds).
    */
    static void getCurrentTimeUsec(Uint32& seconds, Uint32& microseconds);

    /** Similar to getCurrentTime() above but get the full time in microseconds
    */
    static Uint64  getCurrentTimeUsec();

    /** getCurrentASCIITime Gets time/date in a fixed format. The format is
        YY MM DD-HH:MM:SS
        @return Returns String with the ASCII time date.
    */
    static String getCurrentASCIITime();

    static void sleep(Uint32 seconds);

    static Boolean exists(const char* path);

    static Boolean canRead(const char* path);

    static Boolean canWrite(const char* path);

    static Boolean getCurrentDirectory(char* path, Uint32 size);

    static Boolean isDirectory(const char* path);

    static Boolean changeDirectory(const char* path);

    static Boolean makeDirectory(const char* path);

    static Boolean getFileSize(const char* path, Uint32& size);

    static Boolean removeDirectory(const char* path);

    static Boolean removeFile(const char* path);

    /**
        Renames a file.  If the new name refers to an existing file, it is
        removed and replaced with the renamed file.  The rename operation is
        performed atomically.
        @param oldPath A character string containing the name of the file to
            rename.
        @param newPath A character string containing the name to which to
            rename the file.
        @return A Boolean indicating whether the rename operation was
            successful.
    */
    static Boolean renameFile(const char* oldPath, const char* newPath);

    static Boolean copyFile(const char* fromPath, const char* toPath);

    static String getHostName();
    static String getFullyQualifiedHostName ();
    static String getSystemCreationClassName ();

    // The following 2 methods are wrappers around system functions
    // gethostbyname/gethostbyaddr or gethostbyname_r/gethostbyaddr_r.
    // In addition to calling corresponding system functions, these
    // methods introduce re-tries when errno is set to TRY_AGAIN.
    // Optional parameters are required to cover systems which use '_r'
    // versions of the system functions.
    static struct hostent* getHostByName(
        const char* name,
        struct hostent* he = 0,
        char* buf = 0,
        size_t len = 0);
    static struct hostent* getHostByAddr(
        const char *addr,
        int len,
        int type,
        struct hostent* he = 0,
        char* buf = 0,
        size_t buflen = 0);


    // The following 2 methods are wrappers around system functions
    // getaddrinfo/getnameinfo.
    // In addition to calling corresponding system functions, these
    // methods introduce re-tries on EAI_AGAIN error returns.
    //
    // Callers are expected to call freeaddrinfo when using System::getAddrInfo
    static int getAddrInfo(
        const char *hostname,
        const char *servname,
        const struct addrinfo *hints,
        struct addrinfo **res);
    static int getNameInfo(
        const struct sockaddr *sa,
        size_t salen,
        char *host,
        size_t hostlen,
        char *serv,
        size_t servlen,
        int flags);

    // Gets IP address assosiated with hostName. af indicates the
    // type of address (ipv4 or ipv6) returned.
    static Boolean getHostIP(const String &hostName, int *af, String &hostIP);

    // Gets IP address in binary form. af indicates the type of
    // address (ipv4 or ipv6) returned. Address will be copied to dst.
    static Boolean acquireIP(const char* hostname, int *af, void *dst);

    /**
        Returns true if IPv6 stack is active by checking return code from
        Socket::createSocket() and getSocketError() calls.

        ATTN: We return true if some error other than
        PEGASUS_INVALID_ADDRESS_FAMILY is returned while creating the socket
        because we will not be sure whether the IPv6 stack is active or not
        from the returned error code. Return value of "true" from this method
        should not be trusted absolutely.
    */
#ifdef PEGASUS_ENABLE_IPV6
    static Boolean isIPv6StackActive();
#endif

    /**
        Returns all interface addresses. Both ip4 and ip6 interface addresses
        will be returned.
    */
    static Array<String> getInterfaceAddrs();

    static Uint32 lookupPort(
        const char * serviceName,
        Uint32 defaultPort);

    /**
        Attempts to find the given IP address(32bit) on any of the local defined
        network interfaces
     */
    static Boolean isIpOnNetworkInterface(Uint32 inIP);

    /**
        Attempts to resolve a given hostname
        this function possibly can take some as it can request information
        from the DNS

        @param resolvedNameIP On successful hostname resolution, this output
        parameter contains the IP address that was determined.
        @return true if successful, false if not successful.
     */
    static Boolean resolveHostNameAtDNS(
        const char* hostname,
        Uint32* resolvedNameIP);

    /**
        Attempts to resolve a given IP address
        this function possibly can take some as it can request information
        from the DNS
        @param resolvedIP On successful hostname resolution, this output
        parameter contains the IP address that was determined.
        @return true if successful, false if not successful.
     */
    static Boolean resolveIPAtDNS(Uint32 ip_addr, Uint32 * resolvedIP);

    /**
        Bundling function used to determine if a given hostname or IP address
        belongs to the local host
        this function has the potential to take some time as it will possibly
        use the DNS
     */
    static Boolean isLocalHost(const String& hostName);

    /**
        Checks binIPAddress represented by address family and returns true
        if binary representation matches with loopback ip address. binIPAddress
        must be in host-byte order.
    */
    static Boolean isLoopBack(int af, void *binIPAddress);

    static String getEffectiveUserName();

    /**
        This function is used to input a password with echo disabled.
        The function reads up to a newline and returns a password of at most
        8 characters.

        @param prompt String containing the message prompt to be displayed
        @return password obtained from the user
    */
    static String getPassword(const char* prompt);

    /**
        This function is used to encrypt the user's password.
        The encryption is compatible with Apache's password file (generated
        using the htpasswd command)

        @param password Password to be encrypted.
        @param salt Two character string chosen from the set [a-zA-Z0-9./].

        @return Encrypted password.
    */
    static String encryptPassword(const char* password, const char* salt);

    /**
        This function is used to verify whether specified user is a user
        on the local system.

        @param userName User name to be verified.

        @return true if the username is valid, else false
    */
    static Boolean isSystemUser(const char* userName);

    /**
        Checks whether the given user is a privileged user.

        @param userName User name to be checked.
        @return true if the user is a privileged user, else false
    */
    static Boolean isPrivilegedUser(const String& userName);

    /**
        This function returns the privileged user name on the system.
        @return the privileged user name
    */
    static String getPrivilegedUserName();

    /**
        This function is used to verify whether the specified user is a member
        of the specified user group.

        @param userName User name to be verified.
        @param groupName User group name.

        @return true if the user is a member of the user group, false otherwise.
        @throw InternalSystemError - If there is an error accessing the
        specified user or group information.
    */
    static Boolean isGroupMember(const char* userName, const char* groupName);

    /**
        Gets the user and group IDs associated with the specified user.
        @param userName  User name for which to look up user and group IDs.
        @param uid       User ID for the specified user name.
        @param gid       Group ID for the specified user name.
        @return          True if the user and group IDs were retrieved
                         successfully, false otherwise.
    */
    static Boolean lookupUserId(
        const char* userName,
        PEGASUS_UID_T& uid,
        PEGASUS_GID_T& gid);

    /**
        Changes the process user context to the specified user and group.
        IMPORTANT:  This method is not reentrant and not async signal safe.
        It should only be called in a single-threaded program.
        @param userName  User name to set as the process user context.
        @param uid       User ID to set as the process user context.
        @param gid       Group ID to set as the process group context.
        @return          True if the user context is successfully changed,
                         false otherwise.
    */
    static Boolean changeUserContext_SingleThreaded(
        const char* userName,
        const PEGASUS_UID_T& uid,
        const PEGASUS_GID_T& gid);

    /**
        This function is used to get the process ID of the calling process.
        @return Process ID
    */
    static Uint32 getPID();

    static Boolean truncateFile(const char* path, size_t newSize);

    /** Compare two strings but ignore any case differences.
        This method is provided only because some platforms lack a fast enough
        strcasecmp function in the standard library.
    */
    static Sint32 strcasecmp(const char* s1, const char* s2);

    /** Compare two strings for equality but ignore any case differences.
        This method should only be used if length of both strings is
        known already. If not, please use System::strcasecmp()
    */
    static bool strncasecmp(
        const char* s1,
        size_t s1_l,
        const char* s2,
        size_t s2_l);

    /** Return just the file or directory name from the path into basename.
        This method returns a file or directory name at the end of a path.
        The path can be relative or absolute. If the path is the root,
        then empty string is returned.
    */
    static char *extract_file_name(const char *fullpath, char *basename);

    /** Return just the pathname into dirname. The fullpath can be relative
        or absolute. This method returns a path minus the file or
        directory name at the end of a supplied path (fullpath).
        If the fullpath is the root, then fullpath is returned.
        The resulting path will contain a trailing slash unless fullpath is
        a file or directory name, in which case, just the file or directory
        name is returned.
    */
    static char *extract_file_path(const char *fullpath, char *dirname);

    // Is absolute path?
    static Boolean is_absolute_path(const char *path);

    /** Changes file permissions on the given file.
        @param path path of the file.
        @param mode the bit-wise inclusive OR of the values for the desired
        permissions.
        @return true on success, false on error and errno is set appropriately.
    */
    static Boolean changeFilePermissions(const char* path, mode_t mode);

    /** Checks whether the specified file is a regular file owned by the
        effective user for the current process.
        @param path Path of the file to check.
        @return True if the file is owned by the effective user for the
        current process, false otherwise.
    */
    static Boolean verifyFileOwnership(const char* path);

    /**
        Flag indicating whether shared libraries are loaded with the
        BIND_VERBOSE option.

        THIS FLAG IS USED ON HP-UX ONLY.
     */
    static Boolean bindVerbose;

    /**
        Writes a message to the system log.  This method encapsulates the
        semantics of opening the system log, writing the specified message,
        and closing the log.

        @param ident An identifier to be prepended to the log messages
        (typically a program name).
        @param severity A severity value to be associated with the message.
        Severity values are defined in Logger.h.
        @param message A message to be written to the system log.
    */
    static void syslog(
        const String& ident,
        Uint32 severity,
        const char* message);

    static void openlog(
        const char *ident,
        int logopt,
        int facility);

    static void closelog();

    /** Function to set hostname and thus override system supplied value */
    static void setHostName(const String & hostName);

    /** Function to set fully qualified hostname and thus override system
        supplied value */
    static void setFullyQualifiedHostName(const String & fullHostName);

    // System ID constants for Logger::put and Logger::trace
    static const String CIMSERVER;

    // System ID constants for Logger::put and Logger::trace
    static const String CIMLISTENER;

    // mutex used for synchronising threads calling getHostName.
    static Mutex _mutexForGetHostName;

    // mutex used for synchronising threads calling getFullyQualifiedHostName
    static Mutex _mutexForGetFQHN;

private:
    // Strings caching hostnames to avoid repeated resolver lookups and to
    // enable setting by configuration
    static String _hostname;
    static String _fullyQualifiedHostname;
};

/**
    The AutoFileLock class uses an advisory file lock to allow access to a
    resource to be controlled.
*/
class PEGASUS_COMMON_LINKAGE AutoFileLock
{
public:

    AutoFileLock(const char* fileName);
    ~AutoFileLock();

private:

    AutoFileLock();
    AutoFileLock(const AutoFileLock&);
    AutoFileLock& operator=(const AutoFileLock&);

#ifdef PEGASUS_OS_TYPE_UNIX
    struct flock _fl;
    int _fd;
#endif
#ifdef PEGASUS_OS_TYPE_WINDOWS
    HANDLE _hFile;
#endif
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_System_h */
