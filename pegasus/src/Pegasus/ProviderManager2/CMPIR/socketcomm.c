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
    \file socketcomm.c
    \brief Socket-based communication layer common functionality.

    This module provides common functionality for the socket based
    communication layers. This comprises the serialization of entities not
    supported by the serialization module, such as contexts and properties
    for instance, as well as dispatching MI and MB calls respectively.
*/

#include <stdio.h>
#include <stdlib.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include "socketcomm.h"
#include "serialization.h"
#include "debug.h"

char * RCMPI_CTX_ID = "RCMPI_CTX_ID";

/****************************************************************************/

void socketcomm_copy_args ( CMPIArgs * src, CMPIArgs * dst )
{
    unsigned int i,arg_count;

    TRACE_NORMAL(("Copying CMPIArgs."));

    arg_count = CMGetArgCount ( src, NULL );

    TRACE_INFO(("arg count: %d", arg_count ));

    for (i = 0; i < arg_count; i++)
    {

        CMPIString * argName;
        CMPIData data = CMGetArgAt ( src, i, &argName, NULL );

        TRACE_INFO(("arg:\nname: %s\ntype: 0x%x\nstate: 0x%x.",
            CMGetCharsPtr ( argName, NULL ),
            data.type, data.state ));

        if (data.state & CMPI_nullValue)
        {
            CMAddArg ( dst,
                CMGetCharsPtr ( argName, NULL ),
                NULL,
                CMPI_null );
        }
        else CMAddArg ( dst,
                CMGetCharsPtr ( argName, NULL ),
                &data.value,
                data.type );
    }
}

//! Copies array contents into CMPIResult objects.
/*!
    Since results from remote providers cannot be sent as CMPIResult directly,
    they are serialized using arrays, which can be transferred to the result
    using this method.

    \param array the source array.
    \param result the destination result.
*/
void socketcomm_array2result ( CMPIArray * array, CONST CMPIResult * result )
{
    CMPIStatus rc;
    TRACE_VERBOSE(("entered function."));


    if (array != NULL && result != NULL)
    {

        CMPICount size = CMGetArrayCount ( array, NULL );
        CMPICount i;

        TRACE_NORMAL(("Transferring %d array elements to CMPIResult.",
            size));

        for (i = 0; i < size; i++)
        {

            CMPIData data = CMGetArrayElementAt ( array, i, NULL );

            if (data.type == CMPI_instance)
            {
                TRACE_INFO(("transferring instance."));
                // EmbeddedObjects or EmbeddedInstances returned
                // from MethodProviders can not use CMReturnInstance
                // because it is not supported, return
                // these type of data with CMReturnData.
                rc = CMReturnInstance ( result, data.value.inst );
                if (rc.rc  == CMPI_RC_ERR_NOT_SUPPORTED)
                {
                    CMReturnData ( result, &data.value, data.type );
                }

            }
            else if (data.type == CMPI_ref)
            {

                TRACE_INFO(("transferring object path."));
                CMReturnObjectPath ( result, data.value.ref );
            }
            else
            {

                TRACE_INFO(("transferring CMPIData."));
                CMReturnData ( result,
                    &data.value,
                    data.type );
            }
        }
    }

    TRACE_VERBOSE(("leaving function."));
}

/****************************************************************************/

void socketcomm_serialize_props (
    int socket,
    const struct BinarySerializerFT * sft,
    char ** props )
{
    unsigned long int i = 0;

    if (props != NULL)
    {

        while (props[i])
        {
            i++;
        }

        sft->serialize_UINT32 ( socket, i );

        while (i)
        {
            sft->serialize_string ( socket, props[--i] );
        }

    }
    else
    {
        sft->serialize_UINT32 ( socket, i );
    }
}


char ** socketcomm_deserialize_props (
    int socket,
    const struct BinarySerializerFT * sft,
    CONST CMPIBroker * broker )
{
    int i;
    char ** r = NULL;

    i = sft->deserialize_UINT32 ( socket );

    if (i > 0)
    {

        r =  (char **) calloc ( i + 1, sizeof ( char * ) );
        while (i)
        {
            r[--i] = sft->deserialize_string ( socket, broker );
        }
    }

    return r;
}

void socketcomm_serialize_context (
    int socket,
    const struct BinarySerializerFT * sft,
    CONST CMPIContext * ctx )
{
    unsigned int size = CMGetContextEntryCount ( ctx, NULL ), i;
    CMPIBoolean hasContainers = 0;
    CMPIString *entryName = 0;
    // Check for Containers.
    CMPIData data = CMGetContextEntry (ctx, "SnmpTrapOidContainer", NULL);
    if (data.state != CMPI_nullValue)
    {
        size++;
        hasContainers = 1;
        CMGetContextEntryAt (ctx, 0, &entryName, NULL);
    }

    TRACE_NORMAL(("serializing context with %d entries", size ));

    sft->serialize_UINT32 ( socket, size );

    if (hasContainers)
    {
         sft->serialize_CMPIString(socket, entryName);
         sft->serialize_CMPIData(socket, data);
         size--;
    }

    for (i = 0; i < size; i++)
    {
        CMPIString * entryName;
        CMPIData entry = CMGetContextEntryAt (
            ctx,
            i,
            &entryName,
            NULL );

        TRACE_INFO(("serializing entry(%d): %s",
            i, CMGetCharsPtr ( entryName, NULL ) ));

        sft->serialize_CMPIString ( socket, entryName );
        sft->serialize_CMPIData ( socket, entry );
    }
}

void socketcomm_deserialize_context (
    int socket,
    const struct BinarySerializerFT * sft,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx )
{
    unsigned int size;

    size = sft->deserialize_UINT32 ( socket );

    TRACE_NORMAL(("deserializing context with %d entries", size ));

    while (size--)
    {
        CMPIString * entryName =
            sft->deserialize_CMPIString ( socket, broker );
        CMPIData entry =
            sft->deserialize_CMPIData ( socket, broker );
        CMPIType type =
            ( entry.state & CMPI_nullValue )?
            CMPI_null: entry.type;

        TRACE_INFO(("adding entryName: %s", CMGetCharsPtr (entryName, NULL)));

        CMAddContextEntry (
            ctx,
            CMGetCharsPtr ( entryName, NULL ),
            &entry.value,
            type );
    }
}

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
