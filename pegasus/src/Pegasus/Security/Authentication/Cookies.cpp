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
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/Base64.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/ArrayIterator.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/StringConversion.h>

#include "Cookies.h"


#include <openssl/rand.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/** Number of random characters in a session ID, before encoding to base64. */
const int SESSION_ID_SIZE = 32;


HTTPSession::HTTPSession(const String &userName, const String &ip)
    : _userName(userName), _ip(ip)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSession::HTTPSession");

    Time::gettimeofday(&_created);

    PEG_METHOD_EXIT();
}

HTTPSession::~HTTPSession()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSession::~HTTPSession");
    PEG_METHOD_EXIT();
}

String HTTPSession::getUserName()
{
    return _userName;
}

String HTTPSession::getIp()
{
    return _ip;
}

bool HTTPSession::expired(int timeout)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSession::expired");

    timeval now, delta;
    Time::gettimeofday(&now);
    Time::subtract(&delta, &now, &_created);

    PEG_METHOD_EXIT();
    return delta.tv_sec >= timeout;
}


HTTPSessionList::HTTPSessionList()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSessionList::HTTPSessionList");

    PEG_METHOD_EXIT();
}

HTTPSessionList::~HTTPSessionList()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSessionList::~HTTPSessionList");

    AutoMutex lock(_sessionsMutex);
    for (SessionTable::Iterator i = _sessions.start(); i; i++)
    {
        delete i.value();
    }

    PEG_METHOD_EXIT();
}

String HTTPSessionList::addNewSession(const String &userName, const String &ip)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSessionList::addNewSession");

    unsigned char raw_id[SESSION_ID_SIZE];

    HTTPSession *session = new HTTPSession(userName, ip);

    String sessionID;
    bool unique;
    // Generate unique and random sessionID.
    do
    {
        // This code assumes that OpenSSL was properly seeded.
        // Now it happens in SSLContext constructor, called by CIMServer
        // constructor.
        int ret = RAND_bytes(raw_id, sizeof(raw_id));
        if (ret != 1)
        {
            // TODO: report error and throw something useful.
        }

        // Convert to a string, that can be transported in HTTP headers
        // (using base64 here).
        Buffer raw_buf((const char*) raw_id, sizeof(raw_id));
        Buffer base64_buf = Base64::encode(raw_buf);
        sessionID.assign(base64_buf.getData(), base64_buf.size());

        {
            AutoMutex lock(_sessionsMutex);
            unique = _sessions.insert(sessionID, session);
        }
    } while (!unique);

    PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
            "Created session %s for user %s@%s",
            (const char *) sessionID.getCString(),
            (const char *) userName.getCString(),
            (const char *) ip.getCString()));


    PEG_METHOD_EXIT();
    return sessionID;
}

bool HTTPSessionList::isAuthenticated(const String &sessionID, const String &ip,
        String &userName)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSessionList::isAuthenticated");

    HTTPSession *data;

    AutoMutex lock(_sessionsMutex);

    if (!cookiesEnabled())
    {
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
                "Rejected session %s from %s, sessions are disabled "
                "by configuration",
                (const char *) sessionID.getCString(),
                (const char *) ip.getCString()));
        PEG_METHOD_EXIT();
        return false;
    }

    if (!_sessions.lookup(sessionID, data))
    {
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL2,
                "Rejected unknown session %s from %s",
                (const char *) sessionID.getCString(),
                (const char *) ip.getCString()));
        PEG_METHOD_EXIT();
        return false;
    }

    int timeout = _getSessionTimeout();

    if (data->expired(timeout))
    {
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL2,
                "Rejected expired session %s from %s",
                (const char *) sessionID.getCString(),
                (const char *) ip.getCString()));
        delete data;
        _sessions.remove(sessionID);
        PEG_METHOD_EXIT();
        return false;
    }

    if (data->getIp() != ip)
    {
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL2,
                "Rejected session %s from wrong IP address %s (expected %s)",
                (const char *) sessionID.getCString(),
                (const char *) ip.getCString(),
                (const char *) data->getIp().getCString()));
        PEG_METHOD_EXIT();
        return false;
    }

    userName = data->getUserName();
    PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
            "Accepted session %s from %s@%s",
            (const char *) sessionID.getCString(),
            (const char *) userName.getCString(),
            (const char *) ip.getCString()));

    PEG_METHOD_EXIT();
    return true;
}

void HTTPSessionList::clearExpired()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSessionList::clearExpired");

    AutoMutex lock(_sessionsMutex);

    Array<HTTPSession*> expiredSessions;
    Array<String> expiredIDs;

    int timeout = _getSessionTimeout();

    // We cannot delete items during HashTable iteration, store expired
    // sessions in an array and delete them later.
    for (SessionTable::Iterator i = _sessions.start(); i; i++)
    {
        if (i.value()->expired(timeout))
        {
            expiredIDs.append(i.key());
            expiredSessions.append(i.value());

            PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
                    "Removing expired session %s",
                    (const char *) i.key().getCString()));
        }
    }

    // Delete expired sessions
    ArrayIterator<HTTPSession*> iter1(expiredSessions);
    for (unsigned i=0; i<iter1.size(); i++)
    {
        delete iter1[i];
    }

    ArrayIterator<String> iter2(expiredIDs);
    for (unsigned i=0; i<iter2.size(); i++)
    {
        _sessions.remove(iter2[i]);
    }
    PEG_METHOD_EXIT();
}

bool HTTPSessionList::cookiesEnabled()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSessionList::cookiesEnabled");
    PEG_METHOD_EXIT();
    return _getSessionTimeout() > 0;
}

int HTTPSessionList::_getSessionTimeout()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION, "HTTPSessionList::_getSessionTimeout");
    // load httpSessionTimeout configuration value
    ConfigManager* configManager = ConfigManager::getInstance();
    String strTimeout = configManager->getCurrentValue("httpSessionTimeout");


    Uint64 timeout;
    StringConversion::decimalStringToUint64(strTimeout.getCString(),
            timeout, false);

    PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
            "Session timeout is %d", (int)timeout));

    PEG_METHOD_EXIT();
    return timeout;
}

PEGASUS_NAMESPACE_END
