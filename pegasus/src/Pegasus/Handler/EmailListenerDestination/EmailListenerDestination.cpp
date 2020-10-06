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
#include <Pegasus/Common/PegasusVersion.h>

#include <iostream>
#include <Pegasus/Handler/CIMHandler.h>
#include <Pegasus/Handler/IndicationFormatter.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/IndicationService/IndicationConstants.h>



#if defined(PEGASUS_OS_VMS)
#include <unistd>
#include <stdio>
#include <descrip>
#include <ssdef>
#include <maildef>
#include <mail$routines>
#include <nam>
#include <starlet>
#endif

#if !defined(PEGASUS_OS_HPUX) && !defined(PEGASUS_OS_LINUX) && \
    !defined(PEGASUS_OS_VMS)
#error "Unsupported Platform"
#endif

#include "EmailListenerDestination.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

void EmailListenerDestination::initialize(CIMRepository* repository)
{
}

void EmailListenerDestination::handleIndication(
    const OperationContext& context,
    const String nameSpace,
    CIMInstance& indication,
    CIMInstance& handler,
    CIMInstance& subscription,
    ContentLanguageList& contentLanguages)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "EmailListenerDestination::handleIndication");

    String indicationText;

    try
    {
        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
            "EmailListenerDestination %s:%s.%s processing %s Indication",
           (const char*)(nameSpace.getCString()),
           (const char*)(handler.getClassName().getString().getCString()),
           (const char*)(handler.getProperty(
           handler.findProperty(PEGASUS_PROPERTYNAME_NAME)).
           getValue().toString().getCString()),
           (const char*)(indication.getClassName().getString().
           getCString())));

        // gets formatted indication message
        indicationText = IndicationFormatter::getFormattedIndText(
            subscription, indication, contentLanguages);

        // get MailTo from handler instance
        Array < String > mailTo;
        handler.getProperty(handler.findProperty(
            PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO)).getValue().get(mailTo);

        // get MailSubject from handler instance
        String mailSubject;
        handler.getProperty(handler.findProperty(
            PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT)).getValue().get(
                mailSubject);

        // get MailCc from handler instance
        CIMValue mailCcValue;
        Array<String> mailCc;

        Uint32 posMailCc = handler.findProperty(
            PEGASUS_PROPERTYNAME_LSTNRDST_MAILCC);

        if (posMailCc != PEG_NOT_FOUND)
        {
            mailCcValue = handler.getProperty(posMailCc).getValue();
        }

        if (!mailCcValue.isNull())
        {
            if ((mailCcValue.getType() == CIMTYPE_STRING) &&
                (mailCcValue.isArray()))
            {
                mailCcValue.get(mailCc);
            }
        }

        // Sends the formatted indication to the specified recipients
        _sendViaEmail(mailTo, mailCc, mailSubject, indicationText);
    }
    catch (CIMException& c)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1, "CIMException: %s",
            (const char*)c.getMessage().getCString()));
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, c.getMessage());
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1, "Exception: %s",
            (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "Failed to deliver indication via e-mail.");
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "FAILED_TO_DELIVER_INDICATION_VIA_EMAIL",
            "Failed to deliver indication via e-mail."));
    }

    PEG_METHOD_EXIT();
}

void EmailListenerDestination::_sendViaEmail(
    const Array<String>& mailTo,
    const Array<String>& mailCc,
    const String& mailSubject,
    const String& formattedText)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "EmailListenerDestination::_sendViaEmail");

#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX) || \
    defined(PEGASUS_OS_VMS)

    String exceptionStr;
    FILE* filePtr;
    char mailFile[TEMP_NAME_LEN];

#ifndef PEGASUS_OS_VMS
    // Check for proper execute permissions for sendmail
    if (access(SENDMAIL_CMD, X_OK) < 0)
    {
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "Handler.EmailListenerDestination.EmailListenerDestination."
                    "_MSG_EXECUTE_ACCESS_FAILED",
                "Cannot execute $0: $1",
                SENDMAIL_CMD,
                strerror(errno)));

        PEG_METHOD_EXIT();
        return;
    }
#endif

    // open a temporary file to hold the indication mail message
    _openFile(&filePtr, mailFile);

    try
    {
        _buildMailHeader(mailTo, mailCc, mailSubject, filePtr);

        // write indication text to the file
        _writeStrToFile(formattedText, filePtr);

        fclose(filePtr);
    }
    catch (CIMException&)
    {
        fclose(filePtr);
        unlink(mailFile);

        PEG_METHOD_EXIT();
        return;
    }

    try
    {
#ifdef PEGASUS_OS_VMS
        //
        // Start mail send process
        //
        status = mail$send_begin(&send_context, &nulllist, &nulllist);
        if (status != SS$_NORMAL)
        {
            PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
                "Routine mail$send_begin failed.");
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "Handler.EmailListenerDestination.EmailListenerDestination."
                    "ROUTINE_MAIL_SEND_BEGIN_FAILED.PEGASUS_OS_VMS",
                "Routine mail$send_begin failed."));
        }
#endif
        // send the message
        _sendMsg(mailFile);
    }
    catch (CIMException&)
    {
        unlink(mailFile);

        PEG_METHOD_EXIT();
        return;
    }

    unlink(mailFile);

#else

    PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
                         "sendmail is not supported.");
    PEG_METHOD_EXIT();

    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED, MessageLoaderParms(
        "Handler.EmailListenerDestination.EmailListenerDestination."
            "UNSUPPORTED_OPERATION",
        "sendmail is not supported."));
#endif

    PEG_METHOD_EXIT();
}

void EmailListenerDestination::_buildMailHeader(
    const Array<String>& mailTo,
    const Array<String>& mailCc,
    const String& mailSubject,
    FILE* filePtr)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "EmailListenerDestination::_buildMailHeader");

    String exceptionStr;

    String mailToStr = _buildMailAddrStr(mailTo);

    if (mailToStr == String::EMPTY)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
             "Handler.EmailListenerDestination.EmailListenerDestination."
                 "DO_NOT_HAVE_EMAIL_ADDRESS",
             "Do not have an e-mail address."));
    }
    PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL3,
       "EmailListenerDestination sending Indication via email to %s",
       (const char*)(mailToStr.getCString())));

#ifdef PEGASUS_OS_VMS

    //
    // Add cc destination to message.
    //

    String mailCcStr = _buildMailAddrCcStr(mailCc);

    //
    // Write the mailSubject string
    //

    String mailSubjectStr;
    mailSubjectStr.append(mailSubject);
    CString foo = mailSubjectStr.getCString();

    attribute_itmlst[0].buffer_length = strlen(foo);
    attribute_itmlst[0].buffer_address = (long &)foo;

    status = mail$send_add_attribute(
        &send_context,
        attribute_itmlst,
        &nulllist);
    if (status != SS$_NORMAL)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "Routine mail$send_add_attribute failed.");
        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "ROUTINE_MAIL_SEND_ADD_ATTRIBUTE_FAILED.PEGASUS_OS_VMS",
            "Routine mail$send_add_attribute failed."));
    }

    //
    // Add filename to bodypart of the message
    //

    bodypart_itmlst[0].buffer_length = strlen(mailFileVms);
    bodypart_itmlst[0].buffer_address = (long &)mailFileVms;

    status = mail$send_add_bodypart(&send_context, bodypart_itmlst, 0);
    if (status != SS$_NORMAL)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "Routine mail$send_add_bodypart failed..");
        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "ROUTINE_MAIL_SEND_ADD_BODYPART_FAILED.PEGASUS_OS_VMS",
            "Routine mail$send_add_bodypart failed."));
    }

#else

    String mailHdrStr;

    // Write the mailToStr to file
    mailHdrStr.append("To: ");
    mailHdrStr.append(mailToStr);
    _writeStrToFile(mailHdrStr, filePtr);

    String mailCcStr = _buildMailAddrStr(mailCc);

    // Write the mailCcStr to file
    mailHdrStr = String::EMPTY;

    mailHdrStr.append("Cc: ");
    mailHdrStr.append(mailCcStr);
    _writeStrToFile(mailHdrStr, filePtr);

    // build from string
    String fromStr;
    fromStr.append("From: ");
    fromStr.append(System::getEffectiveUserName());
    fromStr.append("@");
    fromStr.append(System::getFullyQualifiedHostName());

    // Write the fromStr to file
    _writeStrToFile(fromStr, filePtr);

    // Write the mailSubject string to file
    String mailSubjectStr;
    mailSubjectStr.append("Subject: ");
    mailSubjectStr.append(mailSubject);
    _writeStrToFile(mailSubjectStr, filePtr);
#endif

    PEG_METHOD_EXIT();
}

String EmailListenerDestination::_buildMailAddrStr(
    const Array<String>& mailAddr)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "EmailListenerDestination::_buildMailAddrStr");

    String mailAddrStr;
    Uint32 mailAddrSize = mailAddr.size();

    for (Uint32 i = 0; i < mailAddrSize; i++)
    {
#if defined(PEGASUS_OS_VMS)

        CString mailAddrVms = mailAddr[i].getCString();
        //
        // Add destination to message
        //
        address_itmlst[0].buffer_length = strlen(mailAddrVms);
        address_itmlst[0].buffer_address = (long &)mailAddrVms;

        status = mail$send_add_address(
            &send_context,
            address_itmlst,
            &nulllist);
        if (status != SS$_NORMAL)
        {
            PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
                "Routine mail$send_add_address failed.");
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "Handler.EmailListenerDestination.EmailListenerDestination."
                    "ROUTINE_MAIL_SEND_ADD_ADDRESS_FAILED.PEGASUS_OS_VMS",
                "Routine mail$send_add_address failed."));
        }
        mailAddrStr = "VMS";
#else
        mailAddrStr.append(mailAddr[i]);

        if (i < (mailAddrSize - 1))
        {
            mailAddrStr.append(",");
        }
#endif
    }

    PEG_METHOD_EXIT();
    return  mailAddrStr;
}

#ifdef PEGASUS_OS_VMS
String EmailListenerDestination::_buildMailAddrCcStr(
    const Array<String>& mailAddr)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "EmailListenerDestination::_buildMailAddrCcStr");

    String mailAddrStr;
    Uint32 mailAddrSize = mailAddr.size();

    for (Uint32 i = 0; i < mailAddrSize; i++)
    {
        CString mailAddrCcVms = mailAddr[i].getCString();
        //
        // Add cc destination to message
        //
        address_cc_itmlst[0].buffer_length = strlen(mailAddrCcVms);
        address_cc_itmlst[0].buffer_address = (long &)mailAddrCcVms;

        status = mail$send_add_address(
            &send_context,
            address_cc_itmlst,
            &nulllist);
        if (status != SS$_NORMAL)
        {
            PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
                "Routine mail$send_add_address failed (cc).");
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "Handler.EmailListenerDestination.EmailListenerDestination."
                    "ROUTINE_MAIL_SEND_ADD_ADDRESS_FAILED_CC.PEGASUS_OS_VMS",
                "Routine mail$send_add_address failed (cc)."));
        }
    }

    PEG_METHOD_EXIT();
    return mailAddrStr;
}

#endif

void EmailListenerDestination::_writeStrToFile(
    const String& mailHdrStr,
    FILE* filePtr)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "EmailListenerDestination::_writeStrToFile");

    String exceptionStr;

    if (fprintf(filePtr, "%s\n", (const char *) mailHdrStr.getCString()) < 0)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1,
            "Failed to write the %s to the file: %s.",
            (const char *) mailHdrStr.getCString(),
            strerror(errno)));

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_WRITE_TO_THE_FILE_FAILED",
            "Failed to write the $0 to the file: $1.",
            mailHdrStr,
            strerror(errno));

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }

    PEG_METHOD_EXIT();
}

void EmailListenerDestination::_sendMsg(
    char* mailFile)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER, "EmailListenerDestination::_sendMsg");

    String exceptionStr;
    char sendmailCmd[MAX_SENDMAIL_CMD_LEN];
    FILE* sendmailPtr;
    struct stat statBuf;

    // Checks the existence of the temp mail file
    if (!System::exists(mailFile))
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1,
            "File %s does not exist: %s.",
            mailFile,
            strerror(errno)));

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_FILE_DOES_NOT_EXIST",
            "File $0 does not exist: $1.",
            mailFile,
            strerror(errno));

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }

    // Checks the length of the file since a zero length file causes
    // problems for sendmail()
    if (stat(mailFile, &statBuf) != 0)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1,
            "Can not get file %s status: %s.",
            mailFile,
            strerror(errno)));

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_CAN_NOT_GET_FILE_STATUS",
            "Can not get file $0 status: $1.",
            mailFile,
            strerror(errno));

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }

    if (statBuf.st_size == 0)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1,
            "File %s does not contain any data.",
            mailFile));

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_FILE_DOES_NOT_CONTAIN_DATA",
            "File $0 does not contain any data.",
            mailFile);

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }

#ifdef PEGASUS_OS_VMS
    //
    // Send the mail message
    //
    status = mail$send_message(&send_context, nulllist, nulllist);
    if (status != SS$_NORMAL)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "Routine mail$send_message failed.");
        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "ROUTINE_MAIL_SEND_MESSAGE_FAILED.PEGASUS_OS_VMS",
            "Routine mail$send_message failed."));
    }

    //
    // End mail send process
    //
    status = mail$send_end(&send_context, nulllist, nulllist);
    if (status != SS$_NORMAL)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "Routine mail$send_end failed.");
        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "ROUTINE_MAIL_SEND_END_FAILED.PEGASUS_OS_VMS",
            "Routine mail$send_end failed."));
    }

#else
    sprintf(sendmailCmd, "%s %s %s", SENDMAIL_CMD, SENDMAIL_CMD_OPTS, mailFile);

    // Open the pipe to send the message
    if ((sendmailPtr = popen(sendmailCmd, "r")) == NULL)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "popen of sendmail failed.");

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_POPEN_OF_SENDMAIL_FAILED",
            "popen of sendmail failed.");

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }

    // Close the pipe
    Sint32 retCode = pclose(sendmailPtr);
    if (retCode < 0)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "No associated stream with this popen command.");

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_NO_ASSOCIATED_STREAM",
            "No associated stream with this popen command.");

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }
    else if (retCode == SH_EXECUTE_FAILED)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "/usr/bin/sh could not be executed.");

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_SHELL_CAN_NOT_BE_EXECUTED",
            "/usr/bin/sh could not be executed.");

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }
#endif

    PEG_METHOD_EXIT();
}

void EmailListenerDestination::_openFile(
    FILE** filePtr,
    char* mailFile)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "EmailListenerDestination::_openFile");

    String exceptionStr;

    *filePtr = fopen(tmpnam(mailFile), "w");
#ifdef PEGASUS_OS_VMS
    strcpy(mailFileVms, mailFile);
#endif
    if (*filePtr == NULL)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1,
            "fopen of %s failed: %s.", mailFile,
            strerror(errno)));

        MessageLoaderParms parms(
            "Handler.EmailListenerDestination.EmailListenerDestination."
                "_MSG_FAILED_TO_OPEN_THE_FILE",
            "fopen of $0 failed: $1.",
            mailFile,
            strerror(errno));

        exceptionStr.append(MessageLoader::getMessage(parms));

        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

// This is the entry point into this dynamic module.

extern "C" PEGASUS_EXPORT CIMHandler* PegasusCreateHandler(
    const String& handlerName)
{
    if (handlerName == "EmailListenerDestination")
    {
        return new EmailListenerDestination;
    }

    return 0;
}
