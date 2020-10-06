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


#ifndef Pegasus_slp_agent_h
#define Pegasus_slp_agent_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/General/DynamicLibrary.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Semaphore.h>
#include "Linkage.h"
#include "../slp_client/src/cmd-utils/slp_client/slp_client.h"
#include "../slp_client/src/cmd-utils/slp_client/slp_utils.h"

PEGASUS_NAMESPACE_BEGIN


class sa_reg_params;

struct slpTableEqual
{
    static Boolean equal(const String & x, const String & y)
    {
        if (0 == String::compareNoCase(x, y))
        {
            return true;
        }
        return false;
    }
};

struct slpTableHash
{
    static Uint32 hash(const String & str)
    {
        String cpy(str);
        cpy.toLower();
        Uint32 h = 0;
        for (Uint32 i = 0, n = cpy.size(); i < n; i++)
        {
            h = 5 * h + cpy[i];
        }
        return h;
    }
};

typedef HashTable<String, sa_reg_params*, slpTableEqual, slpTableHash >
    slp_reg_table;

class PEGASUS_SLP_LINKAGE slp_service_agent
{
public:
    slp_service_agent();
    slp_service_agent(
        const char *local_interface,
        unsigned short target_port,
        const char *scopes,
        Boolean listen,
        Boolean use_da,
        const char *srv_type);
    ~slp_service_agent();

    /* svr_register registers one url.  The parameters must exist.
       This function should be used in conjunction with the
        test_svr_register function
       @param url char* parameter that represents the service url
       @param attributes char* string containing all of the attribute
       key/value pairs to be registered.
       @param type char* service names
       @param scopes char* TBD
       @param lifetime unsigned short TBD
       @return Boolean returns Fails if the url, attributes, or type
       parameters do not exist.
       @exception throws the UnitializedObjectException if the agent
       was unable to initialize.
    */
    Boolean srv_register(
        const char *url,
        const char *attributes,
        const char *type,
        const char *scopes,
        unsigned short lifetime);

    /* Tests the parameters for registration of a service.
       @param url char* parameter that represents the service url
       @param attributes char* string containing all of the attribute
       key/value pairs to be registered.
       @param type char* service names
       @param scopes char* TBD
       @param lifetime unsigned short TBD
       @return Uint32 returns 0 for success and non-zero following codes for
       failures.  Code 1, type empty or the service name is not complete
       (no service: component). Code to url empty. Code 3 TBD. Code 4 TBD.
       parameters do not exist.
       @exception throws the UnitializedObjectException if the agent
       was unable to initialize.

      */
    Uint32 test_registration(
        const char *url,
        const char *attrs,
        const char *type,
        const char *scopes);

    static ThreadReturnType PEGASUS_THREAD_CDECL service_listener(void *);

    void start_listener();
    void unregister(Boolean stopListener = true);

    void set_registration_callback(void (*ptr)());

#ifdef PEGASUS_SLP_REG_TIMEOUT
    Semaphore& get_update_reg_semaphore();
#else
    void update_reg_count();
#endif

private:

    slp_service_agent(const slp_service_agent &);
    slp_service_agent & operator= (const slp_service_agent &);

    struct slp_client* _rep;

    slp_client * (*_create_client )(
        const char *,
        const char *,
        uint16,
        const char *,
        const char *,
        BOOL,
        BOOL,
        const char *);
    void (*_destroy_client)(struct slp_client *);


    int (*_find_das)(
        struct slp_client *client,
        const char *predicate,
        const char *scopes);

    uint32 (*_test_reg)(
        char *,
        char *,
        char *,
        char *);

    void (*update_registrations)();

    uint16 _getSLPPort();

    DynamicLibrary _library;

    void _init();
    void _de_init();
    Thread _listen_thread;
    AtomicInt _should_listen;
    AtomicInt _initialized;
    AtomicInt _using_das;
    slp_reg_table _internal_regs;

#ifdef PEGASUS_SLP_REG_TIMEOUT
    Semaphore _update_reg_semaphore;
#else
    AtomicInt _update_reg_count;
#endif
   static const char* slp_lang;
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_slp_agent_h

