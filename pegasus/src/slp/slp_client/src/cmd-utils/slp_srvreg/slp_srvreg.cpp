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
/*****************************************************************************
 *  Description: scripting shell for slp_client - generates an slp REGISTRATION
 *
 *  Originated: September 16, 2002
 *  Original Author: Mike Day md@soft-hackle.net
 *                       mdday@us.ibm.com
 *
 *  Copyright (c) 2001 - 2003  IBM
 *  Copyright (c) 2000 - 2003 Michael Day
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/


#define SLP_LIB_IMPORT
#include "../slp_client/slp_client.h"
#include "../slp_client/slp_utils.h"
#if defined(PEGASUS_OS_TYPE_WINDOWS)
#include <time.h>
#endif

char *type, *url, *attrs, *addr, *scopes, *iface, *spi;
uint16 life = 0x0fff;
BOOL should_listen = FALSE, dir_agent = FALSE, test = FALSE;
static uint16 port = DEFAULT_SLP_PORT;

void free_globals()
{
    if (type != NULL)
    {
        free(type);
    }
    if (url != NULL)
    {
        free(url);
    }
    if (attrs != NULL)
    {
        free(attrs);
    }
    if (addr != NULL)
    {
        free(addr);
    }
    if (scopes != NULL)
    {
        free(scopes);
    }
    if (iface != NULL)
    {
        free(iface);
    }
}

void usage()
{
    printf("\nslp_srvreg -- transmit an SLP Service Registration and print"
        " the results.\n");
    printf("--------------------------------------------------------------"
        "-----------\n");
    printf("slp_srvreg  --type=service-type-string\n");
    printf("            --url=url-string\n");
    printf("            --attributes=attribute-string\n");
    printf("           [--address=target-IP]\n");
    printf("           [--port=target-port]\n");
    printf("           [--scopes=scope-string]\n");
    printf("           [--lifetime=seconds]\n");
    printf("           [--interface=host-IP]\n");
    printf("           [--daemon=true]\n");
    printf("           [--use_da=true]\n");
    printf("           [--test]\n");
    printf("           [--spi=security-parameters-index] (not used)\n");
    printf("\n");
    printf("All parameters must be a single string containing no spaces.\n");
    printf("Always use the format of <parameter>=<value>.\n");
    printf("Parameters enclosed in brackets are not optional.\n");

}


BOOL get_options(int argc, char *argv[])
{
    int i;
    char *bptr;

    for(i = 1; i < argc; i++)
    {
        if ((*argv[i] == '-') && (*(argv[i] + 1) == '-'))
        {
            if (TRUE == lslp_pattern_match(argv[i] + 2, "type=*", FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                /* handle case where type is specified twice on the cmd line */
                if (type != NULL)
                {
                    free(type);
                }
                type = strdup(bptr);
            }
            else if (TRUE == lslp_pattern_match(argv[i] + 2, "url=*", FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                if (url != NULL)
                {
                    free(url);
                }
                url = strdup(bptr);
            }
            else if (
                TRUE == lslp_pattern_match(argv[i] + 2,
                "attributes=*",
                FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                if(attrs != NULL)
                {
                    free(attrs);
                }
                attrs = strdup(bptr);
            }
            else if (
                TRUE == lslp_pattern_match(argv[i] + 2,
                "address=*",
                FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                if (addr != NULL)
                {
                    free(addr);
                }
                addr = strdup(bptr);
            }
            else if (
                TRUE == lslp_pattern_match(argv[i] + 2,
                "scopes=*",
                FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                if (scopes != NULL)
                {
                    free(scopes);
                }
                scopes = strdup(bptr);
            }
            else if (
                TRUE == lslp_pattern_match(argv[i] + 2,
                "lifetime*",
                FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                life = (uint16)strtoul(bptr, NULL, 10);
            }
            else if (TRUE == lslp_pattern_match(argv[i] + 2, "port=*", FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                port = (uint16)strtoul(bptr, NULL, 10);
            }
            else if (
                TRUE == lslp_pattern_match(argv[i] + 2,
                "interface=*",
                FALSE))
            {
                bptr = argv[i] + 2;
                while (*bptr != '=')
                {
                    bptr++;
                }
                bptr++;
                if (iface != NULL)
                {
                    free(iface);
                }
                iface = strdup(bptr);
            }
            else if (
                TRUE == lslp_pattern_match(argv[i] + 2,
                "daemon=true*",
                FALSE))
            {
                should_listen = TRUE;
            }
            else if (TRUE == lslp_pattern_match(argv[i] + 2, "test*", FALSE))
            {
                test = TRUE;
            }
            else if (
                TRUE == lslp_pattern_match(argv[i] + 2,
                "use_da=true*",
                FALSE))
            {
                dir_agent = TRUE;
            }
        }
    }

    if (type == NULL || url == NULL || attrs == NULL)
    {
        return FALSE;
    }
    return TRUE;
}


int main(int argc, char **argv)
{
    struct slp_client *client;
    int count = 0;
    time_t now, last;
    lslpMsg msg_list;

    if (FALSE == get_options(argc, argv))
    {
        usage();
    }
    else
    {
        if (scopes == NULL)
        {
            scopes = strdup("DEFAULT");
        }
        if (test == TRUE)
        {
            return test_srv_reg(type, url, attrs, scopes);
        }

        if (NULL != (client = create_slp_client(
            addr,
            iface,
            port,
            "DSA",
            scopes,
            should_listen,
            dir_agent,
            type)))
        {
            now = (last = time(NULL));
            if (slp_is_loop_back_addr(addr))
            {
                count = client->srv_reg_local(
                    client,
                    url,
                    attrs,
                    type,
                    scopes,
                    life);
            }
            else
            {
                count = client->srv_reg_all(
                    client,
                    url,
                    attrs,
                    type,
                    scopes,
                    life);
            }
            while (should_listen == TRUE)
            {
                _LSLP_SLEEP(10);
                client->service_listener(client, 0, &msg_list);
                now = time(NULL);
                if ((now - last) > (life - 1))
                {
                    count = client->srv_reg_local(
                        client,
                        url,
                        attrs,
                        type,
                        scopes,
                        life);
                    last = time(NULL);
                }
            }
            destroy_slp_client(client);
        }
        printf("srvreg: registered %s with %d SLP agents.\n", type, count);
    }
    free_globals();
    return(1);
}

