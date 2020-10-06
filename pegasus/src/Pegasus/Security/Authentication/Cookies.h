#ifndef Pegasus_Cookies_h
#define Pegasus_Cookies_h

#include <Pegasus/Common/Config.h>

#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/Mutex.h>

PEGASUS_NAMESPACE_BEGIN

/* Definition for purpose of this module:

  SessionID = short unique string, identifying a session.
              A client can then present its session ID instead of username
              and password. Any client that shows Pegasus valid session ID
              is authenticated.
              The string consists just of random characters and is base64
              encoded.

  Cookie   = Value of Cookie: or Set-Cookie: HTTP headers.
             Set-Cookie: is used to transport SessionID from Pegasus to a
             client after its successful authentication.
             Cookie: is used in the opposite direction, instead of username
             and password.

  Session = data stored in Pegasus, associated with a session ID. As we do not
            transport user name inside the cookie, we must remember it on
            server side and that's what Session is used for.
            A session is started when a client authenticates. The session
            has a limited lifetime - see httpSessionTimeout configuration
            option. After this time, session is deleted and SessionID is
            unusable. Client will get 401 Unauthorized response and must present
            username+password again (or use other authentication mechanism).
*/

/**
    Representation session data that we store for one HTTP session.
*/
class PEGASUS_COMMON_LINKAGE HTTPSession
{
public:
    /**
        Create new session.
        @param userName  Name of the authenticated user.
        @param ip        IP address of the client.
     */
    HTTPSession(const String &userName, const String &ip);
    ~HTTPSession();
    String getUserName();
    String getIp();

    /**
        Check, that the session is not longer than 'timeout' seconds.

        @param timeout  Maximum age of the session, in seconds.
        @return false   If the session was created less than 'timeout' seconds
                        ago.
     */
    bool expired(int timeout);

private:
    String _userName, _ip;
    timeval _created;
    unsigned _timeout;
};



/**
    Database of active sessions. There should be only one instance of this
    class.

    It should be cleared regularly by calling clearExpired().
*/
class PEGASUS_COMMON_LINKAGE HTTPSessionList
{
public:

    HTTPSessionList();
    ~HTTPSessionList();

    /**
        Create new session and return its session ID. The client has already
        passed proper HTTP authentication.

        @param userName  Name of the authenticated user.
        @param ip        IP address of the client.
        @return session ID to put into Set-Cookie: header.
    */
    String addNewSession(const String &userName, const String &ip);

    /**
        Check the session with given ID is still valid.

        @param sessionID Session ID to check.
        @param ip        Source IP address of HTTP request to check. It must
                         match the IP of the client when it authenticated.
        @param userName  OUT, name of the authenticated user, as stored in the
                         session. It's valid only if this method returns 'true'.
        @return true     If the session ID points to valid session.
        @return false    If the session is expired or does not exist at all or
                         client IP address is different. Pegasus should respond
                         with 401 Unauthorized.
     */
    bool isAuthenticated(const String &sessionID, const String &ip,
            String &userName);

    /** Remove all expired sessions. */
    void clearExpired();

    /** Return 'true' if cookies are enabled. */
    bool cookiesEnabled();

private:
    /** List of sessions. */
    typedef HashTable<String,
                      HTTPSession*,
                      EqualFunc<String>,
                      HashFunc<String> > SessionTable;

    SessionTable _sessions;
    Mutex _sessionsMutex;
    int _getSessionTimeout();
};

PEGASUS_NAMESPACE_END

#endif //Pegasus_Cookies_h
