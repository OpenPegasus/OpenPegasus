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

#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Pegasus_inl.h>
#include <Service/PidFile.h>
#include <Service/ServerRunStatus.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(PEGASUS_OS_HPUX)
# include <sys/pstat.h>
# include <libgen.h>
#endif

#if defined(PEGASUS_OS_ZOS)
# include <sys/ps.h>
#endif

#if defined(PEGASUS_OS_AIX) || defined(PEGASUS_OS_PASE)
# include <procinfo.h>
# define PROCSIZE sizeof(struct procsinfo)
# if PEGASUS_AIX_VERSION <= 5
// AIX version 5 does not define getprocs() in procinfo.h
extern "C"
{
    extern int getprocs(
        struct procsinfo *, int, struct fdsinfo *, int,pid_t *,int);
}
# endif
#endif

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_OS_TYPE_WINDOWS

//////////////////////////////////////
//
// Windows implementation
//
//////////////////////////////////////

ServerRunStatus::ServerRunStatus(
    const char* serverName,
    const char* pidFilePath)
    : _serverName(serverName),
      _pidFilePath(pidFilePath),
      _isRunningServerInstance(false),
      _parentPid(0),
      _event(NULL),
      _wasAlreadyRunning(false)
{
}

ServerRunStatus::~ServerRunStatus()
{
    setServerNotRunning();
}

Boolean ServerRunStatus::isServerRunning()
{
    return _wasAlreadyRunning;
}

void ServerRunStatus::setServerNotRunning()
{
    if (_event != NULL)
    {
        CloseHandle(_event);
        _event = NULL;
    }
}

void ServerRunStatus::setServerRunning()
{
    if (_event == NULL)
    {
        _event = CreateEvent(NULL, TRUE, TRUE, _serverName);
        if (_event == NULL)
        {
            throw Exception(MessageLoaderParms(
                "src.Server.cimserver_windows.EVENT_CREATION_FAILED",
                "Event Creation Failed : $0.",
                PEGASUS_SYSTEM_ERRORMSG_NLS));
        }
        if ((_event != NULL) && (GetLastError() != ERROR_ALREADY_EXISTS))
        {
            _wasAlreadyRunning = false;
        }
    }
}

void ServerRunStatus::setParentPid(PEGASUS_PID_T parentPid)
{
}

Boolean ServerRunStatus::kill()
{
    return true;
}

#elif defined(PEGASUS_OS_TYPE_UNIX) || defined(PEGASUS_OS_VMS)

//////////////////////////////////////
//
// Unix and OpenVMS implementation
//
//////////////////////////////////////

ServerRunStatus::ServerRunStatus(
    const char* serverName,
    const char* pidFilePath)
    : _serverName(serverName),
      _pidFilePath(pidFilePath),
      _isRunningServerInstance(false),
      _parentPid(0)
{
}

ServerRunStatus::~ServerRunStatus()
{
    setServerNotRunning();
}

void ServerRunStatus::setServerNotRunning()
{
    if (_isRunningServerInstance)
    {
        PidFile pidFile(_pidFilePath);
        pidFile.remove();
        _isRunningServerInstance = false;
    }
}

Boolean ServerRunStatus::isServerRunning()
{
    PidFile pidFile(_pidFilePath);
    PEGASUS_PID_T pid = (PEGASUS_PID_T) pidFile.getPid();

    if (pid == 0)
    {
        return false;
    }

    return (pid != (PEGASUS_PID_T) System::getPID()) &&
           (pid != _parentPid) &&
           _isServerProcess(pid);
}

void ServerRunStatus::setServerRunning()
{
    PidFile pidFile(_pidFilePath);
    pidFile.setPid(System::getPID());
    _isRunningServerInstance = true;
}

void ServerRunStatus::setParentPid(PEGASUS_PID_T parentPid)
{
    _parentPid = parentPid;
}

Boolean ServerRunStatus::kill()
{
    PidFile pidFile(_pidFilePath);
    PEGASUS_PID_T pid = (PEGASUS_PID_T) pidFile.getPid();

    if ((pid == 0) ||
        (pid == (PEGASUS_PID_T) System::getPID()) ||
        (pid == _parentPid) ||
        !_isServerProcess(pid))
    {
        pidFile.remove();
        return false;
    }

#if defined(PEGASUS_OS_HPUX) || \
    defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) || \
    defined(PEGASUS_OS_SOLARIS) || \
    defined(PEGASUS_OS_ZOS) || \
    defined(PEGASUS_OS_AIX) || defined(PEGASUS_OS_PASE)

    ::kill(pid, SIGKILL);

#endif

    pidFile.remove();
    return true;
}

# if defined(PEGASUS_OS_ZOS)

///////////////////////////////////////////////////////
// z/OS implementation of _isServerProcess
///////////////////////////////////////////////////////
Boolean ServerRunStatus::_isServerProcess(PEGASUS_PID_T pid)
{
    W_PSPROC buf;
    int token = 0;
    memset(&buf, 0x00, sizeof(buf));
    buf.ps_conttyptr =(char *) malloc(buf.ps_conttylen =PS_CONTTYBLEN);
    buf.ps_pathptr   =(char *) malloc(buf.ps_pathlen   =PS_PATHBLEN);
    buf.ps_cmdptr    =(char *) malloc(buf.ps_cmdlen    =PS_CMDBLEN);
    Boolean returnValue = false;

    while ((token = w_getpsent(token, &buf, sizeof(buf))) > 0)
    {
        if (buf.ps_pid == pid)
        {
            // If the process id is associated with the server program,
            // then a server is still running.
            if (strstr(buf.ps_pathptr, _serverName) != NULL)
            {
                returnValue = true;
            }
            // else the pid was not associated with the server
            break;
        }
    }

    free(buf.ps_conttyptr);
    free(buf.ps_pathptr);
    free(buf.ps_cmdptr);
    return returnValue;
}

# elif defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) || \
    defined(PEGASUS_OS_SOLARIS)

///////////////////////////////////////////////////////
// Linux and Solaris implementation of _isServerProcess
///////////////////////////////////////////////////////
/*
   Opens the 'stat' file in the /proc/<pid> directory to
   verify that the process name is that of the server.
*/
static Boolean _isServerPidDir(
    const char *directory,
    const char* serverProcessName)
{
    static char filename[80];
    static char buffer[512];
    int fd, bytesRead;

    // generate the name of the stat file in the process's /proc directory,
    // and open it
    sprintf(filename, "%s/%s", directory, "stat");
    if ( (fd = open(filename, O_RDONLY, 0)) == -1 )
    {
        return false;
    }

    // read the contents
    if ( (bytesRead = read( fd, buffer, (sizeof buffer) - 1 )) <= 0 )
    {
        close(fd);
        return false;
    }

    // null terminate the file contents
    buffer[bytesRead] = 0;

    close(fd);

    // the process name is the second element of the file contents and
    // is surrounded by parentheses.
    //
    // find the positions of the parentheses in the file contents
    const char* openParen;
    const char* closeParen;

    openParen = strchr(buffer, '(');
    closeParen = strchr(buffer, ')');
    if (openParen == NULL || closeParen == NULL || closeParen < openParen)
    {
        return false;
    }

    // allocate memory for the result
    AutoArrayPtr<char> processName(new char[closeParen - openParen]);

    // copy the process name into the result
    strncpy(processName.get(), openParen + 1, closeParen - openParen -1);

    // strncpy doesn't NULL-terminate the result, so do it here
    processName[closeParen - openParen -1] = '\0';

    return strcmp(processName.get(), serverProcessName) == 0;
}

Boolean ServerRunStatus::_isServerProcess(PEGASUS_PID_T pid)
{
    // This method makes a stat() system call on the directory in /proc
    // with a name that matches the pid of the server.  It returns true
    // if it successfully located the process dir and verified that the
    // process name matches that of the server.

    static char path[32];
    static struct stat statBuffer;

    sprintf(path, "/proc/%d", pid);
    if (stat(path, &statBuffer) == -1)          // process stopped running
    {
        return false;
    }

    // get the process name to make sure it is the cimserver process
// ATTN: skip verify for Solaris
#  if !defined(PEGASUS_OS_SOLARIS)
    if (!_isServerPidDir(path, _serverName))
    {
        return false;
    }
#  endif

    return true;
}

# elif defined(PEGASUS_OS_AIX) || defined(PEGASUS_OS_PASE)

///////////////////////////////////////////////////////
// AIX implementation of _isServerProcess
///////////////////////////////////////////////////////
/*
   Calls subroutine getprocs() to get information about all processes.
   If successful, an array of procsinfo structures filled with process table
   entries is returned.  Otherwise, a null pointer is returned.
   The output parameter cnt specifies the number of the processes in the
   returned table.
*/
static struct procsinfo* _getProcessData(int& cnt)
{
    struct procsinfo* proctable = NULL;
    struct procsinfo* rtnp = NULL;
    int count = 1048576;
    int rtncnt;
    int repeat = 1;
    int nextp = 0;

    cnt = 0;
    while (repeat &&
           (rtncnt = getprocs(rtnp, PROCSIZE, 0, 0, &nextp, count) > 0))
    {
        if (!rtnp)
        {
            count=rtncnt;
            proctable = (struct procsinfo*) malloc((size_t) PROCSIZE*count);
            if (!proctable)
            {
                return NULL;
            }
            rtnp = proctable;
            nextp = 0;
        }
        else
        {
            cnt += rtncnt;
            if (rtncnt >= count)
            {
                proctable = (struct procsinfo *)peg_inln_realloc(
                    (void*)proctable, (size_t) (PROCSIZE*(cnt+count)));
                if (!proctable)
                {
                    return NULL;
                }
                rtnp = proctable+(cnt);
            }
            else
            {
                repeat = 0;
            }
        } // end of if(!rtnp)
    } //end of while
    return proctable;
}

Boolean ServerRunStatus::_isServerProcess(PEGASUS_PID_T pid)
{
    int count;
    struct procsinfo* proctable;

    proctable = _getProcessData(count);
    if (proctable == NULL)
    {
        return false;
    }

    for (int i=0; i < count; i++)
    {
        if (!strcmp(proctable[i].pi_comm, _serverName) && \
            proctable[i].pi_pid == pid)
        {
            free(proctable);
            return true;
        }
    }

    free(proctable);
    return false;
}

# elif defined(PEGASUS_OS_HPUX)

///////////////////////////////////////////////////////
// HP-UX implementation of _isServerProcess
///////////////////////////////////////////////////////
Boolean ServerRunStatus::_isServerProcess(PEGASUS_PID_T pid)
{
    struct pst_status pstru;

    if (pstat_getproc(&pstru, sizeof(struct pst_status), (size_t)0, pid) != -1)
    {
        //
        // Gets the command basename disregarding the command parameters
        //
        char *execName = strchr(pstru.pst_cmd,' ');
        if (execName)
        {
            *execName = '\0';
        }
        execName = basename(pstru.pst_cmd);

        if (strcmp(execName, _serverName) == 0)
        {
            return true;
        }
    }

    return false;
}

# else

///////////////////////////////////////////////////////
// Generic implementation of _isServerProcess
///////////////////////////////////////////////////////
Boolean ServerRunStatus::_isServerProcess(PEGASUS_PID_T pid)
{
    return false;
}

# endif

#endif

PEGASUS_NAMESPACE_END
