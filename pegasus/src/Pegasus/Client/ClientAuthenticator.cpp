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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Base64.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Constants.h>
#include "ClientAuthenticator.h"

#include <ctype.h>

//
// Constants used to parse the authentication challenge header
//
#define CHAR_BLANK     ' '

#define CHAR_QUOTE     '"'


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    Constant representing the authentication challenge header.
*/
static const char* WWW_AUTHENTICATE = "WWW-Authenticate";

/**
    Constant representing the Basic authentication header.
*/
static const String BASIC_AUTH_HEADER = "Authorization: Basic ";

/**
    Constant representing the Digest authentication header.
*/
static const String DIGEST_AUTH_HEADER = "Authorization: Digest ";

/**
    Constant representing the Negotiate authentication header.
*/
static const String NEGOTIATE_AUTH_HEADER = "Authorization: Negotiate ";

/**
    Constant representing the local authentication header.
*/
static const String LOCAL_AUTH_HEADER = "PegasusAuthorization: Local";


ClientAuthenticator::ClientAuthenticator()
{
    clear();
}

ClientAuthenticator::~ClientAuthenticator()
{
}

void ClientAuthenticator::clear()
{
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    _session.reset(new NegotiateClientSession(String::EMPTY));
#endif
    _requestMessage.reset();
    _userName.clear();
    _password.clear();
    _localAuthFile.clear();
    _localAuthFileContent.clear();
    _challengeReceived = false;
    _authType = ClientAuthenticator::NONE;
    _cookie.clear();
}

Boolean ClientAuthenticator::checkResponseHeaderForChallenge(
    Array<HTTPHeader> headers)
{
    //
    // Search for "WWW-Authenticate" header:
    //
    const char* authHeader;
    String authType;
    String authChallenge;
    String authRealm;

    if (!HTTPMessage::lookupHeader(
            headers, WWW_AUTHENTICATE, authHeader, false))
    {
        return false;
    }

   //
   // Parse the authentication challenge header
   //
   if (!_parseAuthHeader(authHeader, authType, authChallenge))
   {
       throw InvalidAuthHeader();
   }

   if (String::equal(authType, "Local"))
   {
       _authType = ClientAuthenticator::LOCAL;
       authRealm = _parseBasicRealm(authChallenge);
       if (authRealm.size() == 0)
           return false;
   }
   else if ( String::equal(authType, "Basic"))
   {
       _authType = ClientAuthenticator::BASIC;
       authRealm = _parseBasicRealm(authChallenge);
       if (authRealm.size() == 0)
           return false;
   }
   else if ( String::equal(authType, "Digest"))
   {
       _authType = ClientAuthenticator::DIGEST;
   }
   else if ( String::equal(authType, "Negotiate"))
   {
       _authType = ClientAuthenticator::NEGOTIATE;
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
       _session->parseChallenge(authChallenge);
#endif
   }
   else
   {
       throw InvalidAuthHeader();
   }

   if (_challengeReceived)
   {
       // Do not respond to a challenge more than once.
       // Only Negotiate authentication can take multiple roundtrips,
       // but stop it when the server returns empty challenge.
       if (_authType != ClientAuthenticator::NEGOTIATE
               || authChallenge.size() == 0)
       {
           return false;
       }
   }

   _challengeReceived = true;

   if (_authType == ClientAuthenticator::LOCAL)
   {
       String filePath = authRealm;
       FileSystem::translateSlashes(filePath);

       // Check whether the directory is a valid pre-defined directory.
       //
       Uint32 index = filePath.reverseFind('/');

       if (index != PEG_NOT_FOUND)
       {
           String dirName = filePath.subString(0,index);

           if (!String::equal(dirName, String(PEGASUS_LOCAL_AUTH_DIR)))
           {
               // Refuse to respond to the challenge when the file is
               // not in the expected directory
               return false;
           }
       }

       _localAuthFile = authRealm;
   }

   return true;
}


String ClientAuthenticator::buildRequestAuthHeader()
{
    String challengeResponse;

    switch (_authType)
    {
        case ClientAuthenticator::BASIC:

            if (_challengeReceived)
            {
                challengeResponse = BASIC_AUTH_HEADER;

                //
                // build the credentials string using the
                // user name and password
                //
                String userPass =  _userName;

                userPass.append(":");

                userPass.append(_password);

                //
                // copy userPass string content to Uint8 array for encoding
                //
                Buffer userPassArray;

                Uint32 userPassLength = userPass.size();

                userPassArray.reserveCapacity(userPassLength);
                userPassArray.clear();

                for (Uint32 i = 0; i < userPassLength; i++)
                {
                    userPassArray.append((char)userPass[i]);
                }

                //
                // base64 encode the user name and password
                //
                Buffer encodedArray;

                encodedArray = Base64::encode(userPassArray);

                challengeResponse.append(
                    String(encodedArray.getData(), encodedArray.size()));
            }
            break;

        //
        //ATTN: Implement Digest Auth challenge handling code here
        //
        case ClientAuthenticator::DIGEST:
        //    if (_challengeReceived)
        //    {
        //        challengeResponse = DIGEST_AUTH_HEADER;
        //    }
            break;

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
        case ClientAuthenticator::NEGOTIATE:
            challengeResponse = NEGOTIATE_AUTH_HEADER;
            challengeResponse.append(_session->buildRequestAuthData());
            break;
#endif

        case ClientAuthenticator::LOCAL:

            challengeResponse = LOCAL_AUTH_HEADER;
            challengeResponse.append(" \"");

            if (_userName.size())
            {
                 challengeResponse.append(_userName);
            }
            else
            {
                //
                // Get the current login user name
                //
                challengeResponse.append(System::getEffectiveUserName());
            }

            challengeResponse.append(_buildLocalAuthResponse());

            break;

        case ClientAuthenticator::NONE:
            //
            // Gets here only when no authType was set.
            //
            challengeResponse.clear();
            break;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    return challengeResponse;
}

void ClientAuthenticator::setRequestMessage(Message* message)
{
    _requestMessage.reset(message);
}

Message* ClientAuthenticator::getRequestMessage()
{
    return _requestMessage.get();
}

void ClientAuthenticator::resetChallengeStatus()
{
    _challengeReceived = false;
    _localAuthFile.clear();
    _localAuthFileContent.clear();
}

Message* ClientAuthenticator::releaseRequestMessage()
{
    return _requestMessage.release();
}

void ClientAuthenticator::setUserName(const String& userName)
{
    _userName = userName;
}

String ClientAuthenticator::getUserName()
{
    return (_userName);
}

void ClientAuthenticator::setPassword(const String& password)
{
    _password = password;
}

void ClientAuthenticator::setHost(const String& host)
{
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    _session.reset(new NegotiateClientSession(host));
#endif
}

void ClientAuthenticator::setAuthType(ClientAuthenticator::AuthType type)
{
    PEGASUS_ASSERT( (type == ClientAuthenticator::BASIC) ||
         (type == ClientAuthenticator::DIGEST) ||
         (type == ClientAuthenticator::LOCAL) ||
         (type == ClientAuthenticator::NEGOTIATE) ||
         (type == ClientAuthenticator::NONE) );

    _authType = type;
}

ClientAuthenticator::AuthType ClientAuthenticator::getAuthType()
{
    return (_authType);
}

String ClientAuthenticator::_getFileContent(const String& filePath)
{
    String translatedFilePath = filePath;
    FileSystem::translateSlashes(translatedFilePath);

    //
    // Check whether the file exists or not
    //
    if (!FileSystem::exists(translatedFilePath))
    {
        throw NoSuchFile(translatedFilePath);
    }

    //
    // Open the challenge file and read the challenge data
    //
    ifstream ifs(translatedFilePath.getCString());
    if (!ifs)
    {
        //ATTN: Log error message
        return String::EMPTY;
    }

    String fileContent;
    String line;

    while (GetLine(ifs, line))
    {
        fileContent.append(line);
    }

    ifs.close();

    return fileContent;
}

String ClientAuthenticator::_buildLocalAuthResponse()
{
    String authResponse;

    if (_challengeReceived)
    {
        authResponse.append(":");

        //
        // Append the file path that is in the realm sent by the server
        //
        authResponse.append(_localAuthFile);

        authResponse.append(":");

        if (_localAuthFileContent.size() == 0)
        {
            //
            // Read the challenge file content
            //
            try
            {
                _localAuthFileContent = _getFileContent(_localAuthFile);
            }
            catch (NoSuchFile&)
            {
                //ATTN-NB-04-20000305: Log error message to log file
            }
        }

        authResponse.append(_localAuthFileContent);
    }

    authResponse.append("\"");

    return authResponse;
}

Boolean ClientAuthenticator::_parseAuthHeader(
    const char* authHeader,
    String& authType,
    String& authChallenge)
{
    //
    // Skip the white spaces in the begining of the header
    //
    while (*authHeader && isspace(*authHeader))
    {
        ++authHeader;
    }

    //
    // Get the authentication type
    //
    String type = _getSubStringUptoMarker(&authHeader, CHAR_BLANK);

    if (!type.size())
    {
        return false;
    }

    // skip any spaces between authentication type and data
    while (*authHeader && isspace(*authHeader))
    {
        ++authHeader;
    }

    // the rest is challenge
    String challenge(authHeader);

    // There must be challenge in the header.
    // Except Negotiate authentication, where the first 401 Unauthorized
    // has no challenge.
    if (!challenge.size() && !String::equal(type, "Negotiate"))
    {
        return false;
    }

    authType = type;

    authChallenge= challenge;

    return true;
}

String ClientAuthenticator::_parseBasicRealm(const String &challenge)
{
    CString str = challenge.getCString();
    const char *challengeStr = str;
    //
    // Ignore everything up to the start quote
    //
    _getSubStringUptoMarker(&challengeStr, CHAR_QUOTE);
    String realm = _getSubStringUptoMarker(&challengeStr, CHAR_QUOTE);

    return realm;
}

String ClientAuthenticator::_getSubStringUptoMarker(
    const char** line,
    char marker)
{
    String result;

    if (*line)
    {
        //
        // Look for the marker
        //
        const char *pos = strchr(*line, marker);

        if (pos)
        {
            if (*line)
            {
                Uint32 length = (Uint32)(pos - *line);
                result.assign(*line, length);
            }

            while (*pos == marker)
            {
                ++pos;
            }

            *line = pos;
        }
        else
        {
            result.assign(*line);

            *line += strlen(*line);
        }
    }

    return result;
}

void ClientAuthenticator::parseCookie(Array<HTTPHeader> headers)
{
    // This code assumes there is at maximum one Set-Cookie header in a
    // response. In real HTTP, multiple Set-Cookie headers may be present.
    const char* cookieHeader;
    if (!HTTPMessage::lookupHeader(
            headers, "Set-Cookie", cookieHeader, false))
    {
        return;
    }

    // Skip initial whitespaces
    while (*cookieHeader && isspace(*cookieHeader))
    {
        ++cookieHeader;
    }

    _cookie = _getSubStringUptoMarker(&cookieHeader, ';');
}

String ClientAuthenticator::getCookie()
{
    return _cookie;
}

void ClientAuthenticator::clearCookie()
{
    _cookie.clear();
}

PEGASUS_NAMESPACE_END
