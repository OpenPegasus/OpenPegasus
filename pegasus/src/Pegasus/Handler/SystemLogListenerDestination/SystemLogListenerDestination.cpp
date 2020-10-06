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


#include "SystemLogListenerDestination.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

void SystemLogListenerDestination::initialize(CIMRepository* repository)
{
}

void SystemLogListenerDestination::handleIndication(
    const OperationContext& context,
    const String nameSpace,
    CIMInstance& indication,
    CIMInstance& handler,
    CIMInstance& subscription,
    ContentLanguageList& contentLanguages)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "SystemLogListenerDestination::handleIndication");

    String ident_name = "CIM Indication";
    String indicationText;

    try
    {
        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
            "SystemLogListenerDestination %s:%s.%s processing %s Indication",
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

        // default severity
        Uint32 severity = Logger::INFORMATION;

        // If an indication contains severity information, gets the value
        // and maps it to Pegasus logger severity. Otherwise, default value
        // is used.

        Uint32 severityPos =
            indication.findProperty(CIMName("PerceivedSeverity"));

        if (severityPos != PEG_NOT_FOUND)
        {
            Uint16 perceivedSeverity;
            CIMValue perceivedSeverityValue =
                indication.getProperty(severityPos).getValue();

            if (!perceivedSeverityValue.isNull())
            {
                perceivedSeverityValue.get(perceivedSeverity);

                switch (perceivedSeverity)
                {
                    case (_SEVERITY_FATAL):
                    case (_SEVERITY_CRITICAL):
                    {
                        severity = Logger::FATAL;
                        break;
                    }

                    case (_SEVERITY_MAJOR):
                    {
                        severity = Logger::SEVERE;
                        break;
                    }

                    case (_SEVERITY_MINOR):
                    case (_SEVERITY_WARNING):
                    {
                        severity = Logger::WARNING;
                        break;
                    }

                    case (_SEVERITY_INFORMATION):
                    case (_SEVERITY_OTHER):
                    case (_SEVERITY_UNKNOWN):
                    {
                        severity = Logger::INFORMATION;
                        break;
                    }

                    default:
                    {
                        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL2,
                            "PerceivedSeverity = %d is not a valid value."
                            " Using default severity.", perceivedSeverity));
                        break;
                    }
                }
            }
        }

       PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
           "SystemLogListenerDestination writing %s Indication to system log",
           (const char*)(indication.getClassName().getString().getCString())));
       // writes the formatted indication to a system log file
        _writeToSystemLog(ident_name, severity, indicationText);
       PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
           "%s Indication written to system log successfully",
           (const char*)(indication.getClassName().getString().getCString())));
    }
    catch (CIMException& c)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1, "CIMException: %s",
            (const char*)c.getMessage().getCString()));
        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, c.getMessage());
    }
    catch (Exception&e)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1, "Exception: %s",
            (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
            "Failed to deliver indication to system log file.");
        PEG_METHOD_EXIT();

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "Handler.SystemLogListenerDestination.SystemLogListenerDestination."
                "FAILED_TO_DELIVER_INDICATION_TO_SYSTEM_LOG",
            "Failed to deliver indication to system log file."));
    }

    PEG_METHOD_EXIT();
}

void SystemLogListenerDestination::_writeToSystemLog(
    const String& identifier,
    Uint32 severity,
    const String& formattedText)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "SystemLogListenerDestination::_writeToSystemLog");

#if defined(PEGASUS_USE_SYSLOGS)

    System::syslog(identifier, severity, formattedText.getCString());

#else

    PEG_TRACE_CSTRING(TRC_INDICATION_GENERATION, Tracer::LEVEL3,
       "SystemLogListenerDestination writing to PegasusStandard.log");
    // PEGASUS_USE_SYSLOGS is not defined, writes the formatted
    // indications into PegasusStandard.log file
    Logger::put(Logger::STANDARD_LOG , identifier, severity,
        (const char*)formattedText.getCString());

#endif

    PEG_METHOD_EXIT();

}

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

// This is the entry point into this dynamic module.

extern "C" PEGASUS_EXPORT CIMHandler* PegasusCreateHandler(
    const String& handlerName)
{
    if (handlerName == "SystemLogListenerDestination")
    {
        return new SystemLogListenerDestination;
    }

    return 0;
}
