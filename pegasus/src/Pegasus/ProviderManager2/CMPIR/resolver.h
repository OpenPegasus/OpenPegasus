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
    \file resolver.h
    \brief Resolver header file.

    Defines the interface for a resolver component.

*/

#ifndef _REMOTE_CMPI_RESOLVER_H
#define _REMOTE_CMPI_RESOLVER_H

typedef struct provider_address provider_address;


#ifndef CMPI_VER_100
# define CMPI_VER_100
#endif

#ifndef CONST
# ifdef CMPI_VER_100
#  define CONST const
# else
#  define CONST
# endif
#endif

//! Address of a remote provider.
/*!
    This holds the address of one remote provider location in a linked list.
 */
struct  provider_address
{
    char * comm_layer_id;     /*!< the communication layer responsible for
                   connecting to the remote provider */
    char * dst_address;       /*!< the destination address of the remote
                   provider in comm-layer specific format */
    char * provider_module;   /*!< the provider module containing the
                   remote provider on the remote host */
    struct provider_address * next; /*!< pointer to the next address */

    void (*destructor)(struct provider_address*); /*!< address of destructor */
};


/****************************************************************************/

//! Resolves a list of remote provider addresses for an enumerative request.

provider_address * resolve_class (
    CONST CMPIBroker *,
    CONST CMPIContext *,
    CONST CMPIObjectPath *,
    const char *,
    CMPIStatus *);

//! Resolves a single address of a remote provider.
provider_address * resolve_instance (
    CONST CMPIBroker *,
    CONST CMPIContext *,
    CONST CMPIObjectPath *,
    const char *,
    CMPIStatus *);

//! Frees previously resolved address(es).
//void free_addresses ( provider_address * );


#endif
