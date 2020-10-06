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

/*!
    \file proxy_comm.c
    \brief Proxy Provider communication layer handling.

    This module provides the functionality to load and manage communication
    layers, as requested by the proxy provider. These are required to
    communicate with remote providers.

*/
#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "proxy.h"
#include "tool.h"
#include "debug.h"

#if defined PEGASUS_OS_TYPE_WINDOWS
# include <winsock2.h>
# include <Winbase.h>
# include <Windows.h>
#else
# include <dlfcn.h>
# ifndef PEGASUS_OS_ZOS
#  include <error.h>
# endif
#endif

extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;
#define INIT_LOCK(l) if (l==NULL) l=CMPI_BrokerExt_Ftab->newMutex(0);

static void * _load_lib ( const char * libname )
{
    char filename[255];
    //PEGASUS_CMPIR_LIBTYPE appends libname with the lib extn based on os.
    sprintf ( filename, PEGASUS_CMPIR_LIBTYPE, libname );
    //invoke dlopen under unix and LoadLibrary under windows OS.
    return PEGASUS_CMPIR_LOADLIBRARY( filename, RTLD_LAZY );
}

static void _unload_lib ( void * libhandle )
{
    PEGASUS_CMPIR_FREELIBRARY(libhandle);
}

//! Loads a server-side communication layer library.
/*!
    The function tries to load "lib<id>.so" looking for an entry point
    "<id>_InitCommLayer". The latter one is called to obtain a provider_comm
    structure including the function pointer table for MI calls towards
    remote providers.

    \param id the name of the comm-layer for which the library has to be loaded.
    \param broker broker handle as passed to the init function.
    \param ctx context as passed to the init function.

    \return pointer to the provider_comm structure from the comm-layer, or NULL.
*/
static provider_comm * load_comm_library (
    const char * id,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx )
{
    void * hLibrary;
    char function[255];

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("loading comm-layer library: lib%s.so", id));

    hLibrary = _load_lib ( id );

    if (hLibrary != NULL)
    {
        INIT_COMM_LAYER fp;
        sprintf ( function, "%s_InitCommLayer", id );
        //invokes dlsym on unix and GetProcAddress on windows
        fp = (INIT_COMM_LAYER)PEGASUS_CMPIR_GETPROCADDRESS(hLibrary,function);
        if (fp != NULL)
        {
            provider_comm * result = fp(broker, ctx);
            result->id = strdup ( id );
            result->handle = hLibrary;

            TRACE_INFO(("comm-layer successfully initialized."));
            TRACE_VERBOSE(("leaving function."));
            return result;
        }
        //invokes dlfree call on unix and FreeLibrary on windows
        PEGASUS_CMPIR_FREELIBRARY( hLibrary );
    }
    error_at_line (
        0,
        errno,
        __FILE__,
        __LINE__,
        "Unable to load/init communication-layer library.%s Error",
        id);
    TRACE_VERBOSE(("leaving function."));
    return NULL;
}



//! Unloads a server-side communication layer library.
/*!
    The function unloads "lib<id>.so".

    \param id comm the provider_comm structure that can be unloaded
*/
static void unload_comm_library ( provider_comm * comm )
{
    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("unloading comm-layer library: %s", comm->id));

    _unload_lib ( comm->handle );

    TRACE_VERBOSE(("leaving function."));
}


static provider_comm * __comm_layers = NULL;
static CMPI_MUTEX_TYPE __mutex=NULL;

//! Looks up a server-side communication layer or loads it, if necessary.
/*!
    The function maintains a list of previously loaded comm-layers locally.
    A mutex is used to ensure proper access to this list. If a comm-layer
    cannot be found within the list, it is being loaded and inserted at
    the begininng.

    \param comm_id the name of the communication layer to be looked up.
    \param broker broker handle as passed to the init function.
    \param ctx context as passed to the init function.

    \return the comm-layer matching the id or NULL if it cannot be loaded.
*/
provider_comm * load_provider_comm (
    const char * comm_id,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx )
{
    provider_comm * tmp;

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("loading remote communication layer: %s", comm_id ));

    INIT_LOCK(__mutex);
    CMPI_BrokerExt_Ftab->lockMutex(__mutex);

    for (tmp = __comm_layers; tmp != NULL; tmp = tmp->next)
    {
        if (strcmp ( tmp->id, comm_id ) == 0)
        {
            CMPI_BrokerExt_Ftab->unlockMutex(__mutex);
            TRACE_INFO(("found previously loaded comm-layer."));
            TRACE_VERBOSE(("leaving function."));
            return tmp;
        }
    }

    tmp = load_comm_library ( comm_id, broker, ctx );

    if (tmp != NULL)
    {
        tmp->next     = __comm_layers;
        __comm_layers = tmp;
    }

    CMPI_BrokerExt_Ftab->unlockMutex(__mutex);

    TRACE_VERBOSE(("leaving function."));
    return tmp;
}

//! Unloads communication layers (usually after provider terminsation.)
/*!
    The function maintains a list of previously loaded comm-layers locally.
    A mutex is used to ensure proper access to this list. If a comm-layer
    is found within the list, it is being unloaded.

    \param comm_id the name of the communication layer to be looked up.
    \param broker broker handle as passed to the init function.
    \param ctx context as passed to the cleanup function.

*/
void unload_provider_comms ()
{
    provider_comm * tmp;
    provider_comm * next;
    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("unloading remote communication layers"));

    INIT_LOCK(__mutex);
    CMPI_BrokerExt_Ftab->lockMutex(__mutex);

    for (tmp = __comm_layers; tmp != NULL; tmp = next)
    {
        TRACE_INFO(("unloading comm-layer: %s.", tmp->id));
        next = tmp->next;
        // IBMKR: Call the cleanup function of comm library.
        tmp->terminate();
        // IBMKR: free the library name.
        free (tmp->id); tmp->id = 0;
        unload_comm_library(tmp);
    }

    __comm_layers = NULL;
    CMPI_BrokerExt_Ftab->unlockMutex(__mutex);

    TRACE_VERBOSE(("leaving function."));
}


void cleanup_provider_comms ()
{
    CMPI_BrokerExt_Ftab->destroyMutex(__mutex);
    __mutex = 0;
}
/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
