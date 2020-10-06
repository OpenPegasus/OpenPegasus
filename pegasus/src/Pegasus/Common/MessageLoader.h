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

#ifndef Pegasus_MessageLoader_h
#define Pegasus_MessageLoader_h

#include <cstdlib>
#include <cctype>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Formatter.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>

PEGASUS_NAMESPACE_BEGIN

/**
    MessageLoaderParms class is basically a stuct class containing public
    variables that control the way MessageLoader behaves. MessageLoader uses
    the fields in this class to decide where and how to load messages from
    the message resources.
 */
class PEGASUS_COMMON_LINKAGE MessageLoaderParms
{
public:

    /**
        Unique message identifier for a particular
        message in a message resource
     */
    const char* msg_id;

    /**
        String default_msg: the default message to use if a message
        cannot be loaded from a message resource
     */
    String default_msg;

    /**
        String msg_src_path: this path tells MessageLoader where to
        find message resources.
        It can be empty, fully qualified or relative to $PEGASUS_HOME
     */
    String msg_src_path;

    /**
        AcceptLanguageList acceptlanguages: This contains the languages
        that are acceptable by the caller of MessageLoader::getMessage()
        or openMessageFile(). That is, MessageLoader will do its best to
        return a message in a language that was specified in this container.
        This container is naturally ordered using the quality values
        attached to the languages and MessageLoader iterates through this
        container in its natural ordering.  This container is used by
        MessageLoader to load messages if it is not empty.
     */
    AcceptLanguageList acceptlanguages;

    /**
        ContentLanguageList contentlanguages: This is set by
        MessageLoader::getMessage() and after a message has been loaded
        from either a message resource or the default message, or by
        MessageLoader::openMessageFile() after it has identified and
        opened a message resource. After the call to
        MessageLoader::getMessage() or MessageLoader::openMessageFile(),
        the caller can check the MessageLoaderParms.contentlanguages
        object to see what MessageLoader set it to. In all cases where a
        message is returned from MessageLoader::getMessage() or will be
        returned from MessageLoader::getMessage2(), this field will be
        set to match the language that the message was (or will be)
        found in.
     */
    ContentLanguageList contentlanguages;

    /**
        Boolean useProcessLocale: Default is false, if true, MessageLoader
        uses the system default language to loads messages from.
     */
    Boolean useProcessLocale;

    /**
        Boolean useThreadLocale: Default is true, this tells
        MessageLoader to use the AcceptLanguageList container
        from the current Pegasus thread.
     */
    Boolean useThreadLocale;

    /**
        const Formatter::Arg&0-9: These are assigned the various
        substitutions necessary to properly format the message being
        extracted.  MessageLoader substitutes these in the correct
        places in the message being returned from
        MessageLoader::getMessage()
     */
    Formatter::Arg arg0;
    Formatter::Arg arg1;
    Formatter::Arg arg2;
    Formatter::Arg arg3;
    Formatter::Arg arg4;
    Formatter::Arg arg5;
    Formatter::Arg arg6;
    Formatter::Arg arg7;
    Formatter::Arg arg8;
    Formatter::Arg arg9;

    /** Constructor */
    MessageLoaderParms();

    /** Constructor */
    MessageLoaderParms(
        const char* id,
        const char* msg,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2,
        const Formatter::Arg& arg3,
        const Formatter::Arg& arg4,
        const Formatter::Arg& arg5 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg6 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg7 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg8 = Formatter::DEFAULT_ARG,
        const Formatter::Arg& arg9 = Formatter::DEFAULT_ARG);

    /** Constructor */
    MessageLoaderParms(
        const char* id,
        const char* msg);

    /**
        Constructor with a String default message argument and no
        substitutions.  This form is handy for cases like command usage
        strings where the message is built up dynamically from component parts.
    */
    MessageLoaderParms(
        const char* id,
        const String& msg);

    /** Constructor */
    MessageLoaderParms(
        const char* id,
        const char* msg,
        const Formatter::Arg& arg0);

    /** Constructor */
    MessageLoaderParms(
        const char* id,
        const char* msg,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1);

    /** Constructor */
    MessageLoaderParms(
        const char* id,
        const char* msg,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2);

    /** Constructor */
    MessageLoaderParms(
        const char* id,
        const char* msg,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2,
        const Formatter::Arg& arg3);

#ifdef PEGASUS_DEBUG
    /** Converts to string. */
    String toString();
#endif

    ~MessageLoaderParms();

private:

    void* _resbundl;

    void _init();

    friend class MessageLoader;
    friend class MessageLoaderICU;
};


/**
    MessageLoader is a static class resposible for looking up messages in
    message resources.
    For specific behaviour details of this class see the Globalization HOWTO.
 */
class PEGASUS_COMMON_LINKAGE MessageLoader
{
public:

    /**
        Retrieves a message from a message resource
        @param parms MessageLoaderParms - controls the behaviour of how a
        message is retrieved
        @return String - the formatted message
     */
    static String getMessage(MessageLoaderParms& parms);

    /**
        Opens a message resource bundle.  If unsuccessful, a subsequent call
        to getMessage2() with the specified MessageLoaderParms object will
        result in the default message being formatted.
        ATTN: Do we want *real* error codes for this?
        @param parms MessageLoaderParms - controls the behaviour of how a
            message is retrieved, this parameter should be used *ONLY* on
            subsequent calls to getMessage2() and closeMessageFile().
     */
    static void openMessageFile(MessageLoaderParms& parms);

    /**
        Closes a message resource bundle.
        @param parms MessageLoaderParms - identifies a previously opened
            resource bundle returned from openMessageFile().
     */
    static void closeMessageFile(MessageLoaderParms& parms);

    /**
        Retrieves a message from a message resource previously opened by
        openMessageFile()
        @param parms MessageLoaderParms - controls the behaviour of how a
            message is retrieved, and is the same MessageLoaderParms
            parameter that was passed to openMessageFile().
        @return String - the formatted message
     */
    static String getMessage2(MessageLoaderParms& parms);

    static void setPegasusMsgHome(String home);

    static void setPegasusMsgHomeRelative(const String& argv0);

    static String getQualifiedMsgPath(const String& path);

    static Boolean _useProcessLocale;

    static Boolean _useDefaultMsg;

    static AcceptLanguageList _acceptlanguages;

private:

    static String formatDefaultMessage(MessageLoaderParms& parms);

    static void initPegasusMsgHome(const String& startDir);

    static void checkDefaultMsgLoading();

    static String pegasus_MSG_HOME;
};

PEGASUS_NAMESPACE_END

#endif
