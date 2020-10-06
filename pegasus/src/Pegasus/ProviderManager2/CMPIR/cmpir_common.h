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

/* This files contains Macro's for platform / os specific system calls and c
functions with different names */

#ifndef _CMPIR_PROXY_PROVIDER_H_
#define _CMPIR_PROXY_PROVIDER_H_

#include <Pegasus/Common/Config.h>
#include <stdlib.h>

#ifdef PEGASUS_OS_TYPE_UNIX

# define lloonngg long long int
# define CMPIR_THREAD_CDECL CMPI_THREAD_CDECL
# define PEGASUS_CMPIR_ATOL(x) atoll(x)
# define PEGASUS_CMPIR_CDECL
# define PEGASUS_CMPIR_IO_BUFPTR_TYPE void
# define PEGASUS_CMPIR_CLOSESOCKET(socket_hdl) close(socket_hdl)
# define PEGASUS_CMPIR_FREELIBRARY(libhandle) dlclose(libhandle)
# define PEGASUS_CMPIR_GETGID() getgid()
# define PEGASUS_CMPIR_GETPID() getpid()
# define PEGASUS_CMPIR_GETUID() getuid()
# define PEGASUS_CMPIR_GETPROCADDRESS(hLibrary, function) \
    dlsym(hLibrary, function)
# define PEGASUS_CMPIR_INVALID_SOCKET (-1)

# ifndef PEGASUS_PLATFORM_HPUX_PARISC_ACC
#  define PEGASUS_CMPIR_LIBTYPE "lib%s.so"
#else
#  define PEGASUS_CMPIR_LIBTYPE "lib%s.sl"
#endif

# define PEGASUS_CMPIR_LOADLIBRARY(filename,mode) dlopen (filename, mode)
# define PEGASUS_CMPIR_RECV(a,b,c,d) read(a,b,c)
# define PEGASUS_CMPIR_SEND(a,b,c,d)  write(a,b,c)
# define PEGASUS_CMPIR_SLEEP(duration) sleep(duration)
# define PEGASUS_CMPIR_STDCALL
# define PEGASUS_CMPIR_STRCASECMP(a,b) strcasecmp(a,b)
# define PEGASUS_CMPIR_VSPRINTF(a,b,c,d) vsnprintf(a,b,c,d)
# define PEGASUS_CMPIR_WSAGETLASTERROR errno
#endif

#ifdef PEGASUS_OS_TYPE_WINDOWS

# define lloonngg __int64
# define ssize_t size_t

# ifndef FD_SETSIZE
#  define FD_SETSIZE 1024
#endif

# define CMPIR_THREAD_CDECL
# define PEGASUS_CMPIR_ATOL(x) _atoi64(x)
# define PEGASUS_CMPIR_CDECL  __cdecl
# define PEGASUS_CMPIR_IO_BUFPTR_TYPE int
# define PEGASUS_CMPIR_CLOSESOCKET(socket_hdl) closesocket(socket_hdl)
# define PEGASUS_CMPIR_LIBTYPE  "%s.dll"
# define PEGASUS_CMPIR_LOADLIBRARY(filename,mode)  LoadLibrary(filename)
# define PEGASUS_CMPIR_FREELIBRARY(libhandle) FreeLibrary(libhandle)
# define PEGASUS_CMPIR_GETGID() 1
# define PEGASUS_CMPIR_GETPID() _getpid()
# define PEGASUS_CMPIR_GETPROCADDRESS(hLibrary,function)  \
    GetProcAddress(hLibrary, function)
# define PEGASUS_CMPIR_GETUID() 1
# define PEGASUS_CMPIR_INVALID_SOCKET INVALID_SOCKET
# define PEGASUS_CMPIR_RECV(a,b,c,d) recv(a,b,c,d)
# define PEGASUS_CMPIR_SEND(a,b,c,d) send(a,b,c,d)
# define PEGASUS_CMPIR_SLEEP(duration) Sleep(duration)
# define PEGASUS_CMPIR_STDCALL __stdcall
# define PEGASUS_CMPIR_STRCASECMP(a,b) _stricmp(a,b)
# define PEGASUS_CMPIR_VSPRINTF(a, b, c,d)  vsprintf(a,c,d)
# define PEGASUS_CMPIR_WSAGETLASTERROR WSAGetLastError()


typedef unsigned char Uint8;
typedef unsigned int  Uint32;
typedef size_t socklen_t;

int gettimeofday (struct timeval *t,void * );
#endif


#ifdef PEGASUS_OS_ZOS
# define atoll(X) strtoll(X, NULL, 10)
#endif

#endif // End of cmpir_comm.h
