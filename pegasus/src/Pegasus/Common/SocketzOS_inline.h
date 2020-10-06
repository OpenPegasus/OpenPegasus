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

#ifndef SocketzOS_inline_h
#define SocketzOS_inline_h

#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/AuthenticationInfo.h>
#include <sys/ioctl.h>
#include <net/rtrouteh.h>
#include <net/if.h>
#include "//'SYS1.SEZANMAC(EZBZTLSC)'"


PEGASUS_NAMESPACE_BEGIN


MP_Socket::MP_Socket(SocketHandle socket)
    : _socket(socket), _isSecure(false),
      _userAuthenticated(false),
      _socketWriteTimeout(PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS)
{
    _username[0]=0;
}

MP_Socket::MP_Socket(
    SocketHandle socket,
    SSLContext * sslcontext,
    ReadWriteSem * sslContextObjectLock,
    const String& ipAddress)
    : _socket(socket),
      _userAuthenticated(false),
      _socketWriteTimeout(PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS)
{
    PEG_METHOD_ENTER(TRC_SSL, "MP_Socket::MP_Socket()");
    _username[0]=0;
    if (sslcontext != NULL)
    {
        _isSecure = true;
    }
    else
    {
        _isSecure = false;
    }
    PEG_METHOD_EXIT();
}

int MP_Socket::ATTLS_zOS_query()
{
    // ioctl data structure
    struct TTLS_IOCTL ioc;
    int rcIoctl;
    int errnoIoctl;
    int errno2Ioctl;

    PEG_METHOD_ENTER(TRC_SSL, "ATTLS_zOS_query()");

    // clean the structure
    memset(&ioc,0,sizeof(ioc));
    // set used version of structure
    ioc.TTLSi_Ver = TTLS_VERSION1;
    // initialize for query only
    ioc.TTLSi_Req_Type = TTLS_QUERY_ONLY;
    // no buffer for the certificate
    ioc.TTLSi_BufferPtr = NULL;
    // will not use it
    ioc.TTLSi_BufferLen = 0;

    rcIoctl = ioctl(_socket,SIOCTTLSCTL,(char *)&ioc);
    errnoIoctl = errno;
    errno2Ioctl =__errno2();

    if (rcIoctl < 0)
    {
       switch(errnoIoctl)
       {
           case(EINPROGRESS):
           case(EWOULDBLOCK):
           {
               PEG_TRACE((TRC_SSL, Tracer::LEVEL4,
                   "Accept pending: %s (error code %d, reason code 0x%08X).",
                   strerror(errnoIoctl),
                   errnoIoctl,
                   errno2Ioctl));
               PEG_METHOD_EXIT();
               // accept pending
               return 0;
           }
           case(ECONNRESET):
           {
               Logger::put_l(
                   Logger::STANDARD_LOG, System::CIMSERVER,
                   Logger::INFORMATION,
                   MessageLoaderParms(
                       "Pegasus.Common.SocketzOS_inline.CONNECTION_RESET_ERROR",
                           "ATTLS reset the connection due to handshake "
                           "failure. Connection closed."));
               PEG_METHOD_EXIT();
               // close socket
               return -1;
           }
           case(ENOTCONN):
           {
               int socket_errno;
               SocketLength optlen = sizeof(int);
               getsockopt(_socket, SOL_SOCKET, SO_ERROR,
                   (char*)&socket_errno, &optlen);
               PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                   "Client not connected to ATTLS. Closing socket %d : "
                       "%s (error code %d)",
                   _socket,strerror(socket_errno),socket_errno));
               PEG_METHOD_EXIT();
               // close socket
               return -1;
           }
           default:
           {
               char str_errno2[10];
               sprintf(str_errno2,"%08X",errno2Ioctl);
               Logger::put_l(
                   Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                   MessageLoaderParms(
                       "Pegasus.Common.SocketzOS_inline.UNEXPECTED_ERROR",
                       "An unexpected error occurs: $0 ( errno $1, reason code "
                          "0x$2 ). Connection closed.",
                       strerror(errnoIoctl),
                       errnoIoctl,
                       str_errno2));
               PEG_METHOD_EXIT();
               // close socket
               return -1;
           }
       } // end switch(errnoIoctl)
    } // -1 ioctl()

    // this should be a secured connection so
    // check the configuration of ATTLS policy.
    switch(ioc.TTLSi_Stat_Policy)
    {
        case(TTLS_POL_OFF):
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Pegasus.Common.SocketzOS_inline.POLICY_OFF",
                    "ATTLS is not active for TCP-IP stack the CIM server "
                        "is using for HTTPS connections. "
                        "Communication not secured. Connection closed."));
            PEG_METHOD_EXIT();
            // close socket
            return -1;
        }
        case(TTLS_POL_NO_POLICY):
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Pegasus.Common.SocketzOS_inline.NO_POLICY",
                    "There is no ATTLS policy found for the CIM server "
                        "HTTPS connections. "
                        "Communication not secured. Connection closed."));
            PEG_METHOD_EXIT();
            // close socket
            return -1;
        }
        case(TTLS_POL_NOT_ENABLED):
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Pegasus.Common.SocketzOS_inline.POLICY_NOT_ENABLED",
                    "ATTLS policy is not active for the CIM Server HTTPS port. "
                        "Communication not secured. Connection closed."));
            PEG_METHOD_EXIT();
            // close socket
            return -1;
        }
        case(TTLS_POL_ENABLED):
        {
            // a policy exists so it is ensured that a secured connection will
            // be established
            break;
        }
        case(TTLS_POL_APPLCNTRL):
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Pegasus.Common.SocketzOS_inline.APPLCNTRL",
                    "ATTLS policy not valid for CIM Server. Set "
                        "ApplicationControlled to OFF. Connection closed."));
            PEG_METHOD_EXIT();
            // close socket
            return -1;
        }

    } // end switch(ioc.TTLSi_Stat_Policy)

    // check status of connection, configuration is ok for the CIM Server
    switch(ioc.TTLSi_Stat_Conn)
    {
        case(TTLS_CONN_NOTSECURE):
        case(TTLS_CONN_HS_INPROGRESS):
        {
            // the SSL handshake has not been finished yet, try late again.
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                 "ATTLS reports SSL handshake pending.");
            // accept pending
            return 0;
        }
        case(TTLS_CONN_SECURE):
        {
            // the connection is secure
            break;
        }


    } // end switch(ioc.TTLSi_Stat_Conn)

    //
    switch(ioc.TTLSi_Sec_Type)
    {
        case(TTLS_SEC_UNKNOWN):
        case(TTLS_SEC_CLIENT):
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Pegasus.Common.SocketzOS_inline.WRONG_ROLE",
                    "ATTLS policy specifies the wrong HandshakeRole for the "
                        "CIM Server HTTPS port. Communication not secured. "
                        "Connection closed."));
            PEG_METHOD_EXIT();
            // close connection
            return -1;

        }

        case(TTLS_SEC_SERVER):
        case(TTLS_SEC_SRV_CA_PASS):
        case(TTLS_SEC_SRV_CA_FULL):
        case(TTLS_SEC_SRV_CA_REQD):
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                "ATTLS Security Type is valid but no SAFCHK.");
            PEG_METHOD_EXIT();
            // successfull return
            return 1;
        }

        case(TTLS_SEC_SRV_CA_SAFCHK):
        {
            _userAuthenticated=true;
            _authType=AuthenticationInfoRep::AUTH_TYPE_ZOS_ATTLS;
            memcpy(_username,ioc.TTLSi_UserID,ioc.TTLSi_UserID_Len);
            _username[ioc.TTLSi_UserID_Len]=0;   // null terminated string
            __e2a_s(_username);                  // the user name is in EBCDIC!
            PEG_TRACE((TRC_SSL, Tracer::LEVEL2,
                "ATTLS Security Type is SAFCHK. Resolved user ID \'%s\'",
                _username));
            PEG_METHOD_EXIT();
            // successfull return
            return 1;

        }
    } // end switch(ioc.TTLSi_Sec_Type)
    // This should never be reached
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
        "Received unexpected return value of ioctl(SIOCTTLSCTL).");
    PEG_METHOD_EXIT();
    return -1;
} // end ATTLS_zOS_Query

int MP_Socket::LocalSocket_zOS_query()
{
// This function is only available in z/OS 1.8 and greater
#if (__TARGET_LIB__ >= 0x41080000)

    struct __sect_s ioSec;
    int rcIoctl;
    int errnoIoctl;
    int errno2Ioctl;

    PEG_METHOD_ENTER(TRC_SSL, "LocalSocket_zOS_query()");


    // clean the structure
    memset(&ioSec,0,sizeof(ioSec));

    // Query the local socket for security information.
    rcIoctl = ioctl(_socket,SECIGET_T,(char *)&ioSec);
    errnoIoctl = errno;
    errno2Ioctl =__errno2();

    // If an error occures, the error is written to the trace
    // but the function will successfully return.

    if (rcIoctl < 0)
    {
        switch (errnoIoctl)
        {
        case(EBADF):
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
                "Not a valid socket descriptor for "
                    "query local authentication.");
            break;
        }
        case(EINVAL):
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
                "The local authentication request is not valid"
                    " or not supported on this socket.");
            break;
        }
        case(ENODEV):
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL2,
                "Not a local socket descriptor.");
            break;
        }
        default:
        {
            PEG_TRACE((TRC_SSL, Tracer::LEVEL1,
                "An unexpected error occurs: %s ( errno $d, reason code "
                    "0x%08X ). ",
                strerror(errnoIoctl),
                errnoIoctl,
                errno2Ioctl));
            break;
        } // end default
        } // end switch errnoIoctl

        PEG_METHOD_EXIT();
        return 1;

    } // if rcIoctl < 0

    // Is client task level security information available ?
    if (ioSec.__sectt_useridlen != 0)
    {
        _userAuthenticated=true;
        _authType=AuthenticationInfoRep::AUTH_TYPE_ZOS_LOCAL_DOMIAN_SOCKET;
        memcpy(_username,ioSec.__sectt_userid,ioSec.__sectt_useridlen);
        // null terminated string
        _username[ioSec.__sectt_useridlen]=0;
        // the user name is in EBCDIC!
        __e2a_s(_username);
        PEG_TRACE((TRC_SSL, Tracer::LEVEL3,
            "Local Socket authentication. Resolved task level user ID \'%s\'",
            _username));
        PEG_METHOD_EXIT();
        return 1;

    }

    // Is client process level security information available ?
    if (ioSec.__sectp_useridlen != 0)
    {
        _userAuthenticated=true;
        _authType=AuthenticationInfoRep::AUTH_TYPE_ZOS_LOCAL_DOMIAN_SOCKET;
        memcpy(_username,ioSec.__sectp_userid,ioSec.__sectp_useridlen);
        // null terminated string
        _username[ioSec.__sectp_useridlen]=0;
        // the user name is in EBCDIC!
        __e2a_s(_username);
        PEG_TRACE((TRC_SSL, Tracer::LEVEL3,
            "Local Socket authentication. "
                "Resolved process level user ID \'%s\'",
            _username));
        PEG_METHOD_EXIT();
        return 1;

    }

    // This should never be reached
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
        "Received unexpected return value of ioctl(SECIGET_T).");
    PEG_METHOD_EXIT();
    return -1;
#else
    return 1;
#endif //  (__TARGET_LIB__ >= 0x41080000)
}


PEGASUS_NAMESPACE_END
#endif

