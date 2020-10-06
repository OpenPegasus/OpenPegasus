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
#include <Pegasus/Common/FileSystem.h>
#include "peg_slp_agent.h"

#ifdef PEGASUS_USE_EXTERNAL_SLP_TYPE
#include <slp.h>
#endif  /* PEGASUS_USE_EXTERNAL_SLP_TYPE */

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// The Solaris version of  external SLP requires the locale parameter to be set.
// OpenSLP allows NULL so that it can select the locale.

#ifdef PEGASUS_USE_EXTERNAL_SLP_TYPE
// language typing for slp call only applies to SOLARIS
#ifdef PEGASUS_OS_SOLARIS
        // If 2 (i.e. solarisslp). Set language.
#if     PEGASUS_USE_EXTERNAL_SLP_TYPE == 2
        const char* slp_service_agent::slp_lang = "en";
        // if 1, openslp and let slp set language
#elif PEGASUS_USE_EXTERNAL_SLP_TYPE == 1
    _   const char* slp_service_agent::slp_lang = NULL;
#else   // Some other implementation
        const char* slp_service_agent::slp_lang = NULL;
#endif  // End PEGASUS_USE_EXTERNAL_SLP_TYPE
#else   // NOT PEGASUS_OS_SOLARIS
    const char* slp_service_agent::slp_lang = NULL;
#endif
#endif  // PEGASUS_USE_EXTERNAL_SLP_TYPE

class sa_reg_params
{
public:

    sa_reg_params(
        const char*,
        const char*,
        const char*,
        const char*,
        unsigned short);
    ~sa_reg_params();

    char* url;
    char* attrs;
    char* type;
    char* scopes;
    Uint32 lifetime;
    Uint32 expire;


private:
    sa_reg_params();
    sa_reg_params(const sa_reg_params&);
    sa_reg_params& operator=(const sa_reg_params&);
};

sa_reg_params::sa_reg_params(
    const char*  _url,
    const char*  _attrs,
    const char*  _type,
    const char*  _scopes,
    unsigned short _lifetime)
{
    if (_url)
    {
        url = strdup(_url);
    }
    if (_attrs)
    {
        attrs = strdup(_attrs);
    }
    if (_type)
    {
        type = strdup(_type);
    }
    if (_scopes)
    {
        scopes = strdup(_scopes);
    }

    lifetime = _lifetime;
    Uint32 msec, now;
    System::getCurrentTime(now, msec);
    expire = 0;
}

sa_reg_params::~sa_reg_params()
{
    if (url)
    {
        free(url);
    }
    if (attrs)
    {
        free(attrs);
    }
    if (type)
    {
        free(type);
    }
    if (scopes)
    {
        free(scopes);
    }
}

#ifdef PEGASUS_USE_EXTERNAL_SLP_TYPE
void SLPRegCallback(SLPHandle slp_handle, SLPError errcode, void* cookie)
{
    /* return the error code in the cookie */
    *(SLPError*)cookie = errcode;

    /*
        You could do something else here like print out
        the errcode, etc.  Remember, as a general rule,
        do not try to do too much in a callback because
        it is being executed by the same thread that is
        reading slp packets from the wire.
    */
}
#endif /* PEGASUS_USE_EXTERNAL_SLP_TYPE */

uint16 slp_service_agent::_getSLPPort()
{
    uint16  localPort;

    struct servent  *serv;
    struct servent  serv_result;

#define SERV_BUFF_SIZE  1024

    char buf[SERV_BUFF_SIZE];

    // Get the port number.

#if defined(PEGASUS_OS_SOLARIS)

    if ((serv = getservbyname_r(
        SLP_SERVICE_NAME, "udp", &serv_result, buf, SERV_BUF_SIZE)) != NULL)
#elif defined(PEGASUS_OS_LINUX)

    int ret = getservbyname_r(
        SLP_SERVICE_NAME,
        "udp",
        &serv_result,
        buf,
        SERV_BUFF_SIZE,
        &serv);

    if (ret == 0 && serv != NULL)
#else
    if ((serv = getservbyname(SLP_SERVICE_NAME, "udp")) != NULL)
#endif
    {
#if defined (PEGASUS_OS_WINDOWS)
        localPort = ntohs(serv->s_port);
#else
        localPort = htons(serv->s_port);
#endif
    }
    else
    {
        localPort = DEFAULT_SLP_PORT;
    }

    return localPort;
}

slp_service_agent::slp_service_agent()
   : _listen_thread(service_listener, this, false),
   _initialized(0)
#ifdef PEGASUS_SLP_REG_TIMEOUT
   ,_update_reg_semaphore(0)
#endif
{
    try
    {
        _init();
    }
    catch(...)
    {
    }
    if(_initialized.get() && _library.isLoaded() && _create_client)
    {
        _rep = _create_client(
            0,
            0,
            _getSLPPort(),
            "DSA",
            "DEFAULT",
// Dont create listener sockets if REG_TIMEOUT or EXTERNAL_SLP_TYPE is set
#if defined(PEGASUS_SLP_REG_TIMEOUT) || defined(PEGASUS_USE_EXTERNAL_SLP_TYPE)
            FALSE,
#else
            TRUE,
#endif
            FALSE,
            "service:wbem");
    }
}

slp_service_agent::slp_service_agent(
    const char *local_interface,
    unsigned short target_port,
    const char *scopes,
    Boolean listen,
    Boolean use_da,
    const char* srv_type)
    : _listen_thread(service_listener, this, false),
    _initialized(0)
#ifdef PEGASUS_SLP_REG_TIMEOUT
   ,_update_reg_semaphore(0)
#endif

{

    try
    {
        _init();
    }
    catch(...)
    {
    }

    if(_initialized.get() && _library.isLoaded() && _create_client)
    {
        _rep = _create_client(
            "239.255.255.253",
            local_interface,
            target_port,
            "DSA",
            "scopes",
            listen,
            use_da,
            srv_type);
    }
}

slp_service_agent::~slp_service_agent()
{
    try
    {
        _de_init();
    }
    catch(...)
    {
    }
}


void slp_service_agent::_init()
{
    _initialized = 0;


    String libraryFileName;

#if defined(PEGASUS_OS_ZOS)
    char * pegasusHome;
    if (pegasusHome = getenv("PEGASUS_HOME"))
    {
        libraryFileName.append(pegasusHome);
        libraryFileName.append("/lib/");
    }
#endif

    libraryFileName.append(FileSystem::buildLibraryFileName("pegslp_client"));
    _library = DynamicLibrary(libraryFileName);

    if (_library.load())
    {
        _create_client = (slp_client * (*)(
            const char*,
            const char*,
            uint16,
            const char*,
            const char*,
            BOOL,
            BOOL,
            const char*))
            _library.getSymbol("create_slp_client");

        _destroy_client = (void (*)(struct slp_client *))
            _library.getSymbol("destroy_slp_client");

        _find_das = (int (*)(struct slp_client *, const char *, const char *))
            _library.getSymbol("find_das");

        _test_reg = (uint32 (*)(char*, char*, char*, char*))
            _library.getSymbol("test_srv_reg");

        _initialized = 1;

        if (_create_client == 0 ||
            _destroy_client == 0 ||
            _find_das == 0 ||
            _test_reg == 0)
        {
            _initialized = 0;
            String symbol;
            if (_create_client == 0)
            {
                symbol = "create_slp_client";
            }
            if (_destroy_client == 0)
            {
                symbol = "destroy_slp_client";
            }
            if (_find_das == 0)
            {
                symbol = "find_das";
            }
            if (_test_reg == 0)
            {
                symbol = "test_srv_reg";
            }

            Logger::put(Logger::ERROR_LOG,
                "slp_agent",
                Logger::SEVERE,
                "Link Error to library: $0, symbol: $1",
                _library.getFileName(),
                symbol);

            _library.unload();
        }
    }
}

void slp_service_agent::_de_init()
{
    if (_initialized.get() && _library.isLoaded())
    {
        if (_rep)
        {
            _destroy_client(_rep);
            _rep = 0;
        }

        try
        {
            _library.unload();
        }
        catch(...)
        {
        }
    }
}

Boolean slp_service_agent::srv_register(
    const char* url,
    const char* attributes,
    const char* type,
    const char* scopes,
    unsigned short lifetime)
{
    if (_initialized.get() == 0 )
    {
        throw UninitializedObjectException();
    }
    if(url == 0 || attributes == 0 || type == 0)
    {
        return false;
    }
    sa_reg_params* rp = 0;
    _internal_regs.lookup(url, rp);

    if (rp)
    {
        _internal_regs.remove(url);
        delete rp;
    }

    rp = new sa_reg_params(url, attributes, type, scopes, lifetime);

    _internal_regs.insert(url, rp);

#if defined(PEGASUS_USE_EXTERNAL_SLP_TYPE) && !defined(PEGASUS_SLP_REG_TIMEOUT)
    SLPHandle slp_handle = 0;
    SLPError  slpErr = SLP_OK;
    SLPError  callbackErr = SLP_OK;
    if ((slpErr = SLPOpen(slp_lang, SLP_FALSE, &slp_handle)) == SLP_OK)
    {
        slpErr = SLPReg(
            slp_handle,
            url,
            lifetime,
            type,
            attributes,
            SLP_TRUE,
            SLPRegCallback,
            &callbackErr);
        if ((slpErr != SLP_OK) || (callbackErr != SLP_OK))
        {
            SLPClose(slp_handle);
            return false;
        }
        SLPClose(slp_handle);
    }
    else
    {
        return false;
    }
#endif /* PEGASUS_USE_EXTERNAL_SLP_TYPE */

    return true;
}

#ifdef PEGASUS_SLP_REG_TIMEOUT
Semaphore& slp_service_agent::get_update_reg_semaphore()
{
    return _update_reg_semaphore;
}
#else
void slp_service_agent::update_reg_count()
{
    _update_reg_count++;
}
#endif

void slp_service_agent::unregister(Boolean stopListener)
{
    if (_initialized.get() == 0 )
    {
        throw UninitializedObjectException();
    }

    if (stopListener && _should_listen.get())
    {
        _should_listen = 0;
#ifdef PEGASUS_SLP_REG_TIMEOUT
        _update_reg_semaphore.signal();
#endif
        _listen_thread.join();
    }

    while (slp_reg_table::Iterator i = _internal_regs.start())
    {
        sa_reg_params *rp = i.value();
#ifdef PEGASUS_USE_EXTERNAL_SLP_TYPE
        SLPHandle slp_handle = 0;
        SLPError slpErr = SLP_OK;
        SLPError callbackErr=SLP_OK;

        if ((slpErr = SLPOpen(slp_lang, SLP_FALSE, &slp_handle)) == SLP_OK)
        {
            slpErr = SLPDereg(
                slp_handle,
                rp->url,
                SLPRegCallback,
                &callbackErr);
            SLPClose(slp_handle);
        }
#else
        // Unregister with external SLP SA.
        sa_reg_params *p=0;

        _internal_regs.lookup(rp->url, p);
        _rep->srv_reg_local(_rep,
                            (const char*)p->url,
                            (const char*)p->attrs,
                            (const char*)p->type,
                            p->scopes,
                            0);
#endif

        _internal_regs.remove(rp->url);
        delete rp;
    }
}

Uint32 slp_service_agent::test_registration(
    const char *url,
    const char *attrs,
    const char *type,
    const char *scopes)
{

    if (_initialized.get() == 0 )
    {
        throw UninitializedObjectException();
    }
    //cout << "test_registration. type= " << type << endl;
    if (type ==  0)
    {
        return 1;
    }
    if (url == 0)
    {
        return 2;
    }
    if (attrs == 0)
    {
        return 3;
    }
    if(scopes == 0)
    {
        return 4;
    }
    char* _type = strdup(type);
    char* _url = strdup(url);
    char* _attrs = strdup(attrs);
    char* _scopes = strdup(scopes);

    Uint32 ccode = _test_reg(_type, _url, _attrs, _scopes);

    //cout << "rtn from _tst_reg: " << ccode << endl;

    free(_type);
    free(_url);
    free(_attrs);
    free(_scopes);
    return ccode;
}


void slp_service_agent::start_listener()
{
    // see if we need to use an slp directory agent
    if(_initialized.get() == 0 )
    {
        throw UninitializedObjectException();
    }
    _should_listen = 0;
    _listen_thread.run();
}


void slp_service_agent::set_registration_callback(void (*ptr)())
{
    update_registrations = ptr;
}

ThreadReturnType
PEGASUS_THREAD_CDECL slp_service_agent::service_listener(void *parm)
{
#if !defined(PEGASUS_USE_EXTERNAL_SLP_TYPE) ||  \
    (defined(PEGASUS_USE_EXTERNAL_SLP_TYPE) && defined(PEGASUS_SLP_REG_TIMEOUT))
    Thread *myself = (Thread *)parm;
    if (myself == 0)
    {
        throw NullPointer();
    }
    slp_service_agent *agent = (slp_service_agent *)myself->get_parm();


#if defined(PEGASUS_SLP_REG_TIMEOUT)
    Uint16 life = PEGASUS_SLP_REG_TIMEOUT * 60;
#elif !defined(PEGASUS_USE_EXTERNAL_SLP_TYPE)
    agent->_using_das = agent->_find_das(agent->_rep, NULL, "DEFAULT");
#endif

    agent->_should_listen = 1;
#ifndef PEGASUS_SLP_REG_TIMEOUT
    lslpMsg msg_list;
#endif

    while (agent->_should_listen.get())
    {
        Uint32 now, msec;
        System::getCurrentTime(now, msec);

        // now register everything
        for (slp_reg_table::Iterator i = agent->_internal_regs.start();
            i ; i++)
        {
            sa_reg_params *rp = i.value();

#ifdef PEGASUS_USE_EXTERNAL_SLP_TYPE
            SLPHandle slp_handle = 0;
            SLPError slpErr = SLP_OK;
            SLPError callbackErr = SLP_OK;
            if ((slpErr = SLPOpen(slp_lang, SLP_FALSE, &slp_handle))
                == SLP_OK)
            {
                slpErr = SLPReg(
                    slp_handle,
                    rp->url,
                    life,
                    rp->type,
                    rp->attrs,
                    SLP_TRUE,
                    SLPRegCallback,
                    &callbackErr);
                SLPClose(slp_handle);
            }
            else
            {
                // ATTN: Could not get SLP handle,
                // we try again when lifetime expires.
            }
#elif PEGASUS_SLP_REG_TIMEOUT
            agent->_rep->srv_reg_local(agent->_rep,
                                       rp->url,
                                       rp->attrs,
                                       rp->type,
                                       rp->scopes,
                                       life);
#else
            if(rp->expire == 0 || rp->expire < now - 1)
            {
                rp->expire = now + rp->lifetime;

                if(agent->_using_das.get())
                {
                    agent->_rep->srv_reg_all(
                        agent->_rep,
                        rp->url,
                        rp->attrs,
                        rp->type,
                        rp->scopes,
                        rp->lifetime);
                }
                else
                {
                    agent->_rep->srv_reg_local(
                        agent->_rep,
                        rp->url,
                        rp->attrs,
                        rp->type,
                        rp->scopes,
                        rp->lifetime);
                }
            }
#endif
        }

#ifdef PEGASUS_SLP_REG_TIMEOUT
        Uint32 waitTime = life * 1000;
        try
        {
            if (!agent->_update_reg_semaphore.time_wait(waitTime))
            {
                // PEGASUS_SLP_REG_TIMEOUT expired, re-register with
                // external SLP SA.
            }
            // semaphore is signalled means we have to update registrations.
            else if (agent->_should_listen.get())
            {
                agent->unregister(false);
                agent->update_registrations();
            }
        }
        catch(...)
        {
        }
#else
         agent->_rep->service_listener(agent->_rep, 0, &msg_list);
        _LSLP_SLEEP(1);
        if (agent->_update_reg_count.get() && agent->_should_listen.get())
        {
            agent->unregister(false);
            agent->update_registrations();
            agent->_update_reg_count--;
        }
#endif
    }

#ifndef PEGASUS_SLP_REG_TIMEOUT
    //Reaching here means listening is stopped,
    //Free the memories used by thread
    //
    //Free the replies
    lslpMsg *temp;
    while(! _LSLP_IS_EMPTY( &msg_list))
    {
        temp = msg_list.next;
        _LSLP_UNLINK(temp);

        //safe to do free here as the lslpMsg is dynamically destructed
        free(temp);
    }
#endif


#endif /* PEGASUS_USE_EXTERNAL_SLP_TYPE */
    return ThreadReturnType(0);
}

PEGASUS_NAMESPACE_END
