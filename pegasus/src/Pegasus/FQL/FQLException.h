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

#ifndef Pegasus_FQLException_h
#define Pegasus_FQLException_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/FQL/Linkage.h>
#include <Pegasus/Common/Exception.h>

//#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

class PEGASUS_FQL_LINKAGE FQLQueryException : public Exception
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLQueryException(const String& msg)
        : Exception(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLQueryException(const ContentLanguageList& langs, const String& msg)
        : Exception(msg)
    {
        setContentLanguages(langs);
    }

    FQLQueryException(
        const ContentLanguageList& langs,
        const MessageLoaderParms& parms)
        : Exception(parms)
    {
        setContentLanguages(langs);
    }

    FQLQueryException(const MessageLoaderParms& parms)
        : Exception(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE QueryLanguageInvalidException :
    public FQLQueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLQueryLanguageInvalidException(const String& msg)
        : FQLQueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLQueryLanguageInvalidException(const MessageLoaderParms& parms)
        : FQLQueryException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLQueryValidationException :
    public FQLQueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryValidationException(const String& msg)
        : FQLQueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLQueryValidationException(const MessageLoaderParms& parms)
        : FQLQueryException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLQueryMissingPropertyException :
    public FQLQueryValidationException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLQueryMissingPropertyException(const String& msg)
        : FQLQueryValidationException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLQueryMissingPropertyException(const MessageLoaderParms& parms)
        : FQLQueryValidationException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLQueryParseException : public FQLQueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLQueryParseException(const String& msg)
        : FQLQueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryParseException(const MessageLoaderParms& parms)
        : QueryException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLQueryRuntimeException : public FQLQueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLQueryRuntimeException(const String& msg)
        : FQLQueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLQueryRuntimeException(const MessageLoaderParms& parms)
        : FQLQueryException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLQueryRuntimePropertyException :
    public FQLQueryRuntimeException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLQueryRuntimePropertyException(const String& msg)
        : FQLQueryRuntimeException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLQueryRuntimePropertyException(const MessageLoaderParms& parms)
        : FQLQueryRuntimeException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLSyntaxErrorException :
    public FQLQueryParseException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLSyntaxErrorException(
        const String& msg,
        Uint32 token,
        Uint32 position,
        const String& rule)
        : FQLQueryParseException(
              MessageLoaderParms(KEY, MSG, msg, token, position, rule))
    {
    }

    FQLSyntaxErrorException(const MessageLoaderParms& parms)
        : FQLQueryParseException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLIdentifierParseException :
    public FQLQueryParseException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLIdentifierParseException(const String& msg)
        : FQLQueryParseException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLIdentifierParseException(const MessageLoaderParms& parms)
        : FQLQueryParseException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLChainedIdParseException :
    public FQLQueryParseException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLChainedIdParseException(const String& msg)
        : FQLQueryParseException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLChainedIdParseException(const MessageLoaderParms& parms)
        : FQLQueryParseException(parms)
    {
    }
};

class PEGASUS_FQL_LINKAGE FQLRuntimeException :
    public FQLQueryRuntimeException
{
public:
    static const char MSG[];
    static const char KEY[];

    FQLRuntimeException(const String& msg)
        : FQLQueryRuntimeException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    FQLRuntimeException(const MessageLoaderParms& parms)
        : FQLQueryRuntimeException(parms)
    {
    }
};


PEGASUS_NAMESPACE_END

#endif
