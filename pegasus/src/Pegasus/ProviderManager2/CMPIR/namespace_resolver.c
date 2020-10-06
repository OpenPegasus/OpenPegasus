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
    \file resolver.c
    \brief Sample resolver.

    This is a sample resolver component, showing the general functionality
    of a resolver. The results returned upon requests issued by the proxy
    provider are hard-coded, no real lookup is yet done.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include "resolver.h"
#include "debug.h"
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

/****************************************************************************/

static void _free_addresses ( provider_address * addr )
{
    if (addr)
    {
        _free_addresses ( addr->next );
        free ( addr->dst_address );
        free ( addr->provider_module );
        free ( addr );
    }
}

static provider_address * outofprocess_resolver (
    CONST CMPIBroker * broker,
    const char * provider)
{
    provider_address * addr;
    char *module;

    module=broker->xft->resolveFileName(provider);

    addr = (provider_address *) calloc ( 1, sizeof ( provider_address ) );

    addr->comm_layer_id   = "CMPIROutOfProcessComm";
    addr->dst_address     = strdup ( "0,0" ); /* uid/gid */
    addr->provider_module = strdup ( module );
    addr->destructor = _free_addresses;

    return addr;
}

static provider_address * namespace_resolver (
    CONST CMPIBroker * broker,
    const char * provider,
    const char *hostname)
{
    provider_address * addr;
    char *pnp;
    char * in_between;
    in_between = strdup(provider);

    addr = (provider_address *) calloc ( 1, sizeof ( provider_address ) );

    addr->comm_layer_id   = "CMPIRTCPComm";
    addr->dst_address     = strdup ( hostname );


    if ((pnp=strchr(in_between,':')))
    {
        *pnp=0;
        addr->provider_module = strdup ( in_between );
    }
    else addr->provider_module = strdup ( in_between );

    addr->destructor = _free_addresses;
    free( in_between );
    return addr;
}

provider_address * resolve_instance (
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop,
    const char * provider,
    CMPIStatus * rc)
{
    CMPIStatus irc;
    const char *ip;
    provider_address *a = NULL;

    CMPIData info = CMGetContextEntry(ctx,"CMPIRRemoteInfo",&irc);

    if (irc.rc == CMPI_RC_OK)
    {
        ip = CMGetCharsPtr(info.value.string,&irc);
        switch (ip[1])
        {
            case '0':
                a = outofprocess_resolver(broker,provider);
                break;
            case '1':
                a = namespace_resolver(broker,provider,ip+3);
                break;
        }
    }

    TRACE_NORMAL(("Resolve requested for provider: %s", provider ));
    if (rc)
    {
        *rc = irc;
    }
    return a;
}

provider_address * resolve_class (
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop,
    const char * provider,
    CMPIStatus * rc)
{
    TRACE_NORMAL(("Resolve requested for provider: %s", provider ));
    return resolve_instance ( broker, ctx, cop, provider, rc );
}




/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/

