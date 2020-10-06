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

#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/LanguageParser.h>
#include <iostream>

#ifdef PEGASUS_HAS_ICU
# include <unicode/uloc.h>
# include <unicode/ures.h>
# include <unicode/umsg.h>
# include <unicode/ucnv.h>
# include <unicode/fmtable.h>
# include <unicode/msgfmt.h>
#endif

#define NO_ICU_MAGIC (void*)0xDEADBEEF

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_HAS_ICU

///////////////////////////////////////////////////////////////////////////////
//
// MessageLoaderICU
//
///////////////////////////////////////////////////////////////////////////////

/**
    Contains ICU-specific message loading logic.
*/
class MessageLoaderICU
{
public:

    static void openICUMessageFile(MessageLoaderParms& parms);

    static String extractICUMessage(
        UResourceBundle* resbundl,
        MessageLoaderParms& parms);

private:

    static void _openICUDefaultLocaleMessageFile(
        const char* resbundl_path_ICU,
        MessageLoaderParms& parms);

    static String _formatICUMessage(
        UResourceBundle* resbundl,
        const UChar* msg,
        int msg_len,
        MessageLoaderParms& parms);

    static void _xferFormattable(
        Formatter::Arg& arg,
        Formattable& formattable);
};

void MessageLoaderICU::openICUMessageFile(MessageLoaderParms& parms)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoaderICU::openICUMessageFile");

    PEGASUS_ASSERT(parms._resbundl == NO_ICU_MAGIC);

    // Get the correct path to the resource bundles
    CString resbundl_path_ICU =
        MessageLoader::getQualifiedMsgPath(parms.msg_src_path).getCString();
    PEG_TRACE((TRC_L10N, Tracer::LEVEL4, "Using resource bundle path: %s",
        (const char *)resbundl_path_ICU));

    // The MessageLoader accept language list, _acceptlanguages, takes
    // precedence over the accept language list passed in parms.
    AcceptLanguageList acceptlanguages;
    acceptlanguages = (MessageLoader::_acceptlanguages.size() > 0) ?
        MessageLoader::_acceptlanguages : parms.acceptlanguages;

    if (MessageLoader::_useProcessLocale)
    {
        _openICUDefaultLocaleMessageFile(resbundl_path_ICU, parms);
        PEG_METHOD_EXIT();
        return;
    }

    // The values of useThreadLocale and useProcessLocale
    // are ignored if acceptlanguages is not empty.

    if (acceptlanguages.size() == 0)
    {
        if (parms.useProcessLocale)
        {
            _openICUDefaultLocaleMessageFile(resbundl_path_ICU, parms);
            PEG_METHOD_EXIT();
            return;
        }

        if (parms.useThreadLocale)
        {
            // get AcceptLanguageList from the current Thread
            AcceptLanguageList *al = Thread::getLanguages();
            if (al != NULL)
            {
                acceptlanguages = *al;
                PEG_TRACE_CSTRING(TRC_L10N, Tracer::LEVEL4,
                    "Using thread accept language list.");
             }
             else
             {
                 PEG_TRACE_CSTRING(TRC_L10N, Tracer::LEVEL4,
                    "Thread accept language list requested, but empty. "
                        "Attempting to load default message file.");
                 _openICUDefaultLocaleMessageFile(resbundl_path_ICU, parms);
                 PEG_METHOD_EXIT();
                 return;
             }
         }
         else
         {
             _openICUDefaultLocaleMessageFile(resbundl_path_ICU, parms);
             PEG_METHOD_EXIT();
             return;
         }
    }

    const int size_locale_ICU = 50;
    char locale_ICU[size_locale_ICU];
    LanguageTag languageTag;
    UErrorCode status = U_ZERO_ERROR;

    // Iterate through AcceptLanguageList looking for an exact match.
    for (Uint32 index = 0; index < acceptlanguages.size(); index++)
    {
         languageTag = acceptlanguages.getLanguageTag(index);
         uloc_getName(
             (const char*)(languageTag.toString()).getCString(),
                 locale_ICU, size_locale_ICU, &status);

         PEG_TRACE((TRC_L10N, Tracer::LEVEL4,
             "Attempting to find resource bundle for accept language "
                 "\"%s\" with locale \"%s\".",
                 (const char*)(languageTag.toString().getCString()),
                 locale_ICU));

         status = U_ZERO_ERROR;
         parms._resbundl = ures_open((const char*)resbundl_path_ICU,
             locale_ICU, &status);

         if (U_SUCCESS(status))
         {
             if (status == U_ZERO_ERROR)
             {
                 PEG_TRACE((TRC_L10N, Tracer::LEVEL4,
                     "Exact match message file FOUND. "
                         "Resource bundle for accept language \"%s\" opened.",
                         (const char *)(languageTag.toString()).getCString()));
                 parms.contentlanguages.append(
                     LanguageTag(languageTag.toString()));
                 PEG_METHOD_EXIT();
                 return;
             }
             else
             {
                 // An exact match message file was not found.  ICU returned
                 // either a fallback match, status == U_USING_FALLBACK_WARNING,
                 // or a default match, status == U_USING_DEFAULT_WARNING.
                 // Need to close this message file and continue the search
                 // for exact match.

                 ures_close(
                     reinterpret_cast<UResourceBundle*>(parms._resbundl));
                 parms._resbundl = NO_ICU_MAGIC;
             }
         }
         else
         {
             parms._resbundl = NO_ICU_MAGIC;
         }
    }

    PEG_TRACE_CSTRING(
        TRC_L10N,
        Tracer::LEVEL3,
        "Exact match message file NOT FOUND. "
             "Attempting to open default message file bundle.");
    _openICUDefaultLocaleMessageFile(resbundl_path_ICU, parms);
    PEG_METHOD_EXIT();
    return;
}

String MessageLoaderICU::extractICUMessage(
    UResourceBundle* resbundl,
    MessageLoaderParms& parms)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t msgLen = 0;

    const UChar* msg = ures_getStringByKey(
        resbundl, parms.msg_id, &msgLen, &status);

    if (U_FAILURE(status))
    {
        return String::EMPTY;
    }

    return _formatICUMessage(resbundl, msg, msgLen, parms);
}

void MessageLoaderICU::_openICUDefaultLocaleMessageFile(
    const char* resbundl_path_ICU,
    MessageLoaderParms& parms)
{
    PEGASUS_ASSERT(parms._resbundl == NO_ICU_MAGIC);

    // UResourceBundle* ures_open(const char* packageName,
    //     const char* locale, UErrorCode* status)
    // If locale is NULL, the default locale will be used.  If
    // strlen(locale) == 0, the root locale will be used.

    // Open the resource bundle for default locale.
    UErrorCode status = U_ZERO_ERROR;
    parms._resbundl = ures_open((const char*)resbundl_path_ICU, NULL, &status);

    if (U_SUCCESS(status))
    {
        const char* rbLocale = ures_getLocale(
            reinterpret_cast<UResourceBundle*>(parms._resbundl), &status);
        PEG_TRACE((TRC_L10N, Tracer::LEVEL4,
            "Requested default locale, using locale \"%s\"", rbLocale));

        // The "root" locale indicates that an ICU message bundle is not
        // present for the current locale setting.

        String localeStr(rbLocale);
        if (localeStr != "root")
        {
            parms.contentlanguages.append(LanguageTag(
                LanguageParser::convertLocaleIdToLanguageTag(localeStr)));
        }
    }
    else
    {
        PEG_TRACE((TRC_L10N, Tracer::LEVEL2,
            "Failed to open resource bundle for default locale or root bundle, "
                "ICU error = %d", status));
        parms._resbundl = NO_ICU_MAGIC;
    }
    return;
}

String MessageLoaderICU::_formatICUMessage(
    UResourceBundle* resbundl,
    const UChar* msg,
    int msg_len,
    MessageLoaderParms& parms)
{
    // format the message
    UnicodeString msg_pattern(msg, msg_len);
    UnicodeString msg_formatted;
    UErrorCode status = U_ZERO_ERROR;
    const int arg_count = 10;
    const char* locale;
    if (resbundl == NULL)
        locale = ULOC_US;
    else
        locale = ures_getLocale(resbundl, &status);

    char lang[4];
    char cc[4];
    char var[arg_count];
    uloc_getLanguage(locale, lang, 4, &status);
    uloc_getCountry(locale, cc, 4, &status);
    uloc_getVariant(locale, var, 10, &status);
    Locale localeID(lang,cc,var);

    status = U_ZERO_ERROR;
    MessageFormat formatter(msg_pattern, localeID, status);

    Formattable args[arg_count];

    if (parms.arg0._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg0, args[0]);
    if (parms.arg1._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg1, args[1]);
    if (parms.arg2._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg2, args[2]);
    if (parms.arg3._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg3, args[3]);
    if (parms.arg4._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg4, args[4]);
    if (parms.arg5._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg5, args[5]);
    if (parms.arg6._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg6, args[6]);
    if (parms.arg7._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg7, args[7]);
    if (parms.arg8._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg8, args[8]);
    if (parms.arg9._type != Formatter::Arg::VOIDT)
        _xferFormattable(parms.arg9, args[9]);

    Formattable args_obj(args, arg_count);
    status = U_ZERO_ERROR;
    msg_formatted = formatter.format(args_obj, msg_formatted, status);

    return String(
        (const Char16*)msg_formatted.getBuffer(),  msg_formatted.length());
}

void MessageLoaderICU::_xferFormattable(
    Formatter::Arg &arg,
    Formattable& formattable)
{
    switch (arg._type)
    {
        case Formatter::Arg::INTEGER:
            formattable = (int32_t)arg._integer;
            break;
        case Formatter::Arg::UINTEGER:
            // Note: the ICU Formattable class doesn't support
            // unsigned 32.  Cast to signed 64.
            formattable = (int64_t)arg._uinteger;
            break;
        case Formatter::Arg::BOOLEAN:
            // Note: the ICU formattable class doesn't support
            // boolean.  Turn it into a string.
            if (!arg._boolean)
                formattable = Formattable("false");
            else
                formattable = Formattable("true");
            break;
        case Formatter::Arg::REAL:
            formattable = (double)arg._real;
            break;
        case Formatter::Arg::LINTEGER:
            // Note: this uses a Formattable constructor that is
            // labelled ICU 2.8 draft.  Assumes that Pegasus uses
            // at least ICU 2.8.
            formattable = (int64_t)arg._lInteger;
            break;
        case Formatter::Arg::ULINTEGER:
            // Note: the ICU Formattable class doesn't support
            // unsigned 64.  If the number is too big for signed 64
            // then turn it into a string.  This string will
            // not be formatted for the locale, but at least the number
            // will appear in the message.
            if (arg._lUInteger >  PEGASUS_UINT64_LITERAL(0x7FFFFFFFFFFFFFFF))
            {
                char buffer[32];  // Should need 21 chars max
                sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u",
                    arg._lUInteger);
                formattable = Formattable(buffer);
            }
            else
            {
                formattable = (int64_t)arg._lUInteger;
            }
            break;
        case Formatter::Arg::STRING:
            formattable = Formattable((UChar*)arg._string.getChar16Data());
            break;
        case Formatter::Arg::VOIDT:
        default:
            formattable = "";
            break;
    }
}

#endif


///////////////////////////////////////////////////////////////////////////////
//
// MessageLoader
//
///////////////////////////////////////////////////////////////////////////////

static const String server_resbundl_name = "pegasus/pegasusServer";
String MessageLoader::pegasus_MSG_HOME;
Boolean MessageLoader::_useProcessLocale = false;
Boolean MessageLoader::_useDefaultMsg = false;
AcceptLanguageList MessageLoader::_acceptlanguages;

String MessageLoader::getMessage(MessageLoaderParms& parms)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoader::getMessage");
    PEG_TRACE((TRC_L10N, Tracer::LEVEL4, "Message ID = %s", parms.msg_id));

    String msg;

    try
    {
        openMessageFile(parms);
        msg = getMessage2(parms);
        closeMessageFile(parms);
    }
    catch (Exception&)
    {
        msg = String("AN INTERNAL ERROR OCCURED IN MESSAGELOADER: ").append(
            parms.default_msg);
    }
    PEG_METHOD_EXIT();
    return msg;
}

String MessageLoader::getMessage2(MessageLoaderParms& parms)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoader::getMessage2");
    String msg;

    try
    {
#if defined (PEGASUS_HAS_MESSAGES) && defined (PEGASUS_HAS_ICU)
        if (!_useDefaultMsg && (parms._resbundl != NO_ICU_MAGIC))
        {
            msg = MessageLoaderICU::extractICUMessage(
                reinterpret_cast<UResourceBundle*>(parms._resbundl), parms);
            if (msg.size() > 0)
            {
                PEG_METHOD_EXIT();
                return msg;
            }
        }
#endif
        // NOTE: the default message is returned if:
        // 1) The previous call to openMessageFile() set _useDefaultMsg
        // 2) parms._resbundl is set to NO_ICU_MAGIC from a previous
        //    call to openMessageFile() indicating an error
        //    (including InitializeICU::initICUSuccessful() failed)
        // 3) Message loading is DISABLED
        // 4) Did not get a message from ICU

        msg = formatDefaultMessage(parms);
    }
    catch (Exception&)
    {
        msg = String("AN INTERNAL ERROR OCCURED IN MESSAGELOADER: ").append(
            parms.default_msg);
    }

    PEG_METHOD_EXIT();
    return msg;
}

void MessageLoader::openMessageFile(MessageLoaderParms& parms)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoader::openMessageFile");

    try
    {
        parms.contentlanguages.clear();

#if defined (PEGASUS_HAS_MESSAGES) && defined (PEGASUS_HAS_ICU)
        if (InitializeICU::initICUSuccessful())
        {
            MessageLoaderICU::openICUMessageFile(parms);
        }
        // If initICUSuccessful() has returned false, then
        // _resbundl is still set to NO_ICU_MAGIC and
        // subsequent calls to getMessage2 will call
        // formatDefaultMessage() instead of using ICU.
#else
        // Set _resbundl to the "magic number" indicating there is no ICU
        // resource bundle to use
        parms._resbundl = NO_ICU_MAGIC;
#endif
    }
    catch (Exception&)
    {
        // Set _resbundl to the "magic number" indicating there was trouble
        // during openICUMessageFile(), so there is no ICU resource bundle
        // to use.
        parms._resbundl = NO_ICU_MAGIC;
    }

    PEG_METHOD_EXIT();
    return;
}

#if defined (PEGASUS_HAS_MESSAGES) && defined (PEGASUS_HAS_ICU)
void MessageLoader::closeMessageFile(MessageLoaderParms& parms)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoader::closeMessageFile");

    if (parms._resbundl != NO_ICU_MAGIC)
    {
        ures_close(reinterpret_cast<UResourceBundle*>(parms._resbundl));
        parms._resbundl = NO_ICU_MAGIC;
    }

    PEG_METHOD_EXIT();
}
#else
void MessageLoader::closeMessageFile(MessageLoaderParms&)
{
    // Do nothing dummy function
}
#endif

String MessageLoader::formatDefaultMessage(MessageLoaderParms& parms)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoader::formatDefaultMessage");

    // NOTE TO PROGRAMMERS: using native substitution functions
    // ie. calling Formatter::format()
    // can result in incorrect locale handling of substitutions

    // locale INSENSITIVE formatting code

    // this could have previously been set by ICU
    parms.contentlanguages.clear();

    PEG_METHOD_EXIT();
    return Formatter::format(
        parms.default_msg,
        parms.arg0,
        parms.arg1,
        parms.arg2,
        parms.arg3,
        parms.arg4,
        parms.arg5,
        parms.arg6,
        parms.arg7,
        parms.arg8,
        parms.arg9);
}

String MessageLoader::getQualifiedMsgPath(const String& path)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoader::getQualifiedMsgPath");

    if (pegasus_MSG_HOME.size() == 0)
        initPegasusMsgHome(String::EMPTY);

    if (path.size() == 0)
    {
        PEG_METHOD_EXIT();
        return pegasus_MSG_HOME + server_resbundl_name;
    }

    if (System::is_absolute_path(path.getCString()))
    {
        PEG_METHOD_EXIT();
        return path;
    }

    PEG_METHOD_EXIT();
    return pegasus_MSG_HOME + path;  // relative path and package name
}

void MessageLoader::setPegasusMsgHome(String home)
{
    PEG_METHOD_ENTER(TRC_L10N, "MessageLoader::setPegasusMsgHome");
    pegasus_MSG_HOME = home + "/";

    // TODO: remove the next call once test cases are compatible with ICU
    // messages
    checkDefaultMsgLoading();
    PEG_METHOD_EXIT();
}

#ifdef PEGASUS_HAS_MESSAGES
void MessageLoader::setPegasusMsgHomeRelative(const String& argv0)
{
    try
    {
        String startingDir, pathDir;

#ifdef PEGASUS_OS_TYPE_WINDOWS
        if (PEG_NOT_FOUND == argv0.find('\\'))
        {
            char exeDir[_MAX_PATH];
            HMODULE module = GetModuleHandle(NULL);
            if (NULL != module )
            {
                DWORD filename =
                    GetModuleFileName(module,exeDir ,sizeof(exeDir));
                if (0 != filename)
                {
                    String path(exeDir);
                    Uint32 command = path.reverseFind('\\');
                    startingDir = path.subString(0, command+1);
                }
            }
        }
        else
        {
            Uint32 command = argv0.reverseFind('\\');
            startingDir = argv0.subString(0, command+1);
        }
#endif

#if defined(PEGASUS_OS_TYPE_UNIX) && !defined(PEGASUS_OS_PASE)
        if (PEG_NOT_FOUND  != argv0.find('/'))
        {
            Uint32 command = argv0.reverseFind('/');
            startingDir = argv0.subString(0, command+1);
        }
        else
        {
            if (FileSystem::existsNoCase(argv0))
            {
                FileSystem::getCurrentDirectory(startingDir);
                startingDir.append("/");
            }
            else
            {
                String path;

                const char* env = getenv("PATH");
                if (env != NULL)
                    path.assign(env);
                String pathDelim = FileSystem::getPathDelimiter();
                
                while (path.size() > 0)
                {
                    try
                    {
                        Uint32 delim = path.find(pathDelim);
                        if (delim != PEG_NOT_FOUND)
                        {
                            pathDir = path.subString(0,delim);
                            path.remove(0,(delim+1));
                        }
                        else
                        {
                            pathDir = path;
                            path = "";
                        }

                        String commandPath = pathDir.append('/');
                        commandPath = commandPath.append(argv0) ;
                        Boolean dirContent =
                            FileSystem::existsNoCase(commandPath);
                        if (dirContent)
                        {
                            startingDir = pathDir;
                            break;
                        }
                    }
                    catch (Exception& e)
                    {
                        // Have to do nothing.
                        // Catching the exception if there is any exception
                        // while searching in the path variable
                    }
                }
            }
        }
#endif

#ifdef PEGASUS_OS_PASE
        // PASE environment have special message path
        startingDir = String(PASE_DEFAULT_MESSAGE_SOURCE);
#endif

        initPegasusMsgHome(startingDir);
    }
    catch (Exception& e)
    {
        // Have to do nothing.
        // Catching the exception if there is any exception while searching
        // in the path variable
    }
}
#else // PEGASUS_HAS_MESSAGES not defined
void MessageLoader::setPegasusMsgHomeRelative(const String&)
{
    // Do nothing function
}
#endif


void MessageLoader::initPegasusMsgHome(const String& startDir)
{
    String startingDir = startDir;
    if (startingDir.size() == 0)
    {
        const char* env = getenv("PEGASUS_MSG_HOME");

        if (env != NULL)
            startingDir.assign(env);
    }

#ifdef PEGASUS_DEFAULT_MESSAGE_SOURCE
    if (System::is_absolute_path(
            (const char *)PEGASUS_DEFAULT_MESSAGE_SOURCE))
    {
        pegasus_MSG_HOME = PEGASUS_DEFAULT_MESSAGE_SOURCE;
        pegasus_MSG_HOME.append('/');
    }
    else
    {
        if (startingDir.size() != 0)
        {
            pegasus_MSG_HOME = startingDir;
            pegasus_MSG_HOME.append('/');
        }
        pegasus_MSG_HOME.append(PEGASUS_DEFAULT_MESSAGE_SOURCE);
        pegasus_MSG_HOME.append('/');
    }
#else
    if (startingDir.size() != 0)
    {
        pegasus_MSG_HOME = startingDir;
        pegasus_MSG_HOME.append("/");
    }
    else
    {
         // Will use current working directory
    }
#endif
    checkDefaultMsgLoading();
}

void MessageLoader::checkDefaultMsgLoading()
{
    // Note: this function is a special hook for the automated tests
    // (poststarttests).  Since the automated tests expect the old hardcoded
    // default messages, an env var will be used to tell this code to ignore
    // ICU and return the default message.
    // This will allow poststarttests to run with ICU installed.
    // TODO: remove this function once test cases are compatible with ICU
    // messages
    const char* env = getenv("PEGASUS_USE_DEFAULT_MESSAGES");
    if (env != NULL)
        _useDefaultMsg = true;
}

MessageLoaderParms::MessageLoaderParms()
{
    useProcessLocale = false;
    useThreadLocale = true;

    acceptlanguages = AcceptLanguageList();
    contentlanguages = ContentLanguageList();

    _resbundl = NO_ICU_MAGIC;
}

MessageLoaderParms::MessageLoaderParms(
    const char* id,
    const char* msg,
    const Formatter::Arg& arg0_,
    const Formatter::Arg& arg1_,
    const Formatter::Arg& arg2_,
    const Formatter::Arg& arg3_,
    const Formatter::Arg& arg4_,
    const Formatter::Arg& arg5_,
    const Formatter::Arg& arg6_,
    const Formatter::Arg& arg7_,
    const Formatter::Arg& arg8_,
    const Formatter::Arg& arg9_)
{
    msg_id = id;
    default_msg = msg;
    _init();
    arg0 = arg0_;
    arg1 = arg1_;
    arg2 = arg2_;
    arg3 = arg3_;
    arg4 = arg4_;
    arg5 = arg5_;
    arg6 = arg6_;
    arg7 = arg7_;
    arg8 = arg8_;
    arg9 = arg9_;
}

MessageLoaderParms::MessageLoaderParms(
    const char* id,
    const char* msg)
{
    msg_id = id;
    default_msg = msg;
    _init();
}

MessageLoaderParms::MessageLoaderParms(
    const char* id,
    const String& msg)
{
    msg_id = id;
    default_msg = msg;
    _init();
}

MessageLoaderParms::MessageLoaderParms(
    const char* id,
    const char* msg,
    const Formatter::Arg& arg0_)
{
    msg_id = id;
    default_msg = msg;
    _init();
    arg0 = arg0_;
}

MessageLoaderParms::MessageLoaderParms(
    const char* id,
    const char* msg,
    const Formatter::Arg& arg0_,
    const Formatter::Arg& arg1_)
{
    msg_id = id;
    default_msg = msg;
    _init();
    arg0 = arg0_;
    arg1 = arg1_;
}

MessageLoaderParms::MessageLoaderParms(
    const char* id,
    const char* msg,
    const Formatter::Arg& arg0_,
    const Formatter::Arg& arg1_,
    const Formatter::Arg& arg2_)
{
    msg_id = id;
    default_msg = msg;
    _init();
    arg0 = arg0_;
    arg1 = arg1_;
    arg2 = arg2_;
}

MessageLoaderParms::MessageLoaderParms(
    const char* id,
    const char* msg,
    const Formatter::Arg& arg0_,
    const Formatter::Arg& arg1_,
    const Formatter::Arg& arg2_,
    const Formatter::Arg& arg3_)
{
    msg_id = id;
    default_msg = msg;
    _init();
    arg0 = arg0_;
    arg1 = arg1_;
    arg2 = arg2_;
    arg3 = arg3_;
}

void MessageLoaderParms::_init()
{
    useProcessLocale = false;
    useThreadLocale = true;

    acceptlanguages.clear();
    contentlanguages.clear();

    arg0 = Formatter::DEFAULT_ARG;
    arg1 = Formatter::DEFAULT_ARG;
    arg2 = Formatter::DEFAULT_ARG;
    arg3 = Formatter::DEFAULT_ARG;
    arg4 = Formatter::DEFAULT_ARG;
    arg5 = Formatter::DEFAULT_ARG;
    arg6 = Formatter::DEFAULT_ARG;
    arg7 = Formatter::DEFAULT_ARG;
    arg8 = Formatter::DEFAULT_ARG;
    arg9 = Formatter::DEFAULT_ARG;

    _resbundl = NO_ICU_MAGIC;
}

#ifdef PEGASUS_DEBUG
String MessageLoaderParms::toString()
{
    String s;
    String processLoc,threadLoc;
    processLoc = (useProcessLocale) ? "true" : "false";
    threadLoc = (useThreadLocale) ? "true" : "false";

    s.append("msg_id = ");
    s.append(msg_id);
    s.append("\n");
    s.append("default_msg = " + default_msg + "\n");
    s.append("msg_src_path = " + msg_src_path + "\n");
    s.append("acceptlanguages = " +
        LanguageParser::buildAcceptLanguageHeader(acceptlanguages) + "\n");
    s.append("contentlanguages = " +
        LanguageParser::buildContentLanguageHeader(contentlanguages) + "\n");

    s.append("useProcessLocale = " + processLoc + "\n");
    s.append("useThreadLocale = " + threadLoc + "\n");
    s.append(
        "arg0 = " + arg0.toString() + "\n" +
        "arg1 = " + arg1.toString() + "\n" +
        "arg2 = " + arg2.toString() + "\n" +
        "arg3 = " + arg3.toString() + "\n" +
        "arg4 = " + arg4.toString() + "\n" +
        "arg5 = " + arg5.toString() + "\n" +
        "arg6 = " + arg6.toString() + "\n" +
        "arg7 = " + arg7.toString() + "\n" +
        "arg8 = " + arg8.toString() + "\n" +
        "arg9 = " + arg9.toString() + "\n\n");

    return s;
}
#endif

MessageLoaderParms::~MessageLoaderParms()
{
}

PEGASUS_NAMESPACE_END
