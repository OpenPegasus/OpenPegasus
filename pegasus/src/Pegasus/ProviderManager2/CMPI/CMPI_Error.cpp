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

#include <Pegasus/General/CIMError.h>

#include "CMPI_Version.h"
#include "CMPI_Error.h"
#include "CMPI_Ftabs.h"
#include "CMPI_String.h"


PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{

    CMPIError *newCMPIError(
        const char* owner, const char* msgID,
        const char * msg,
        const CMPIErrorSeverity sev,
        const CMPIErrorProbableCause pc,
        const CMPIrc cimStatusCode)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:newCMPIError()");
        CIMError::PerceivedSeverityEnum pgSev =
            (CIMError::PerceivedSeverityEnum)sev;
        CIMError::ProbableCauseEnum pgPc = (CIMError::ProbableCauseEnum)pc;
        CIMError::CIMStatusCodeEnum pgSc =
            (CIMError::CIMStatusCodeEnum)cimStatusCode;

        CIMError *cer=new CIMError(owner, msgID, msg, pgSev, pgPc, pgSc);
        CMPIError* cmpiError =
            reinterpret_cast<CMPIError*>(new CMPI_Object(cer));
        PEG_METHOD_EXIT();
        return cmpiError;
    }

    static CMPIError* errClone(const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_Error:errClone()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid Handle - cerr...");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }
        CIMError* cErr=new CIMError(*cer);
        CMPI_Object* obj=new CMPI_Object(cErr);
        obj->unlink();
        CMPIError* neErr=reinterpret_cast<CMPIError*>(obj);
        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return neErr;
    }

    static CMPIStatus errRelease(CMPIError* eErr)
    {
        PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_Error:errRelease()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (cer)
        {
            delete cer;
            (reinterpret_cast<CMPI_Object*>(eErr))->unlinkAndDelete();
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIErrorType errGetErrorType(const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetErrorType()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid Handle - cer...");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return UnknownErrorType;
        }

        CMPIBoolean notNull;
        CIMError::ErrorTypeEnum pgErrorType;

        try
        {
            notNull = cer->getErrorType(pgErrorType);
            if (!notNull)
            {
                PEG_TRACE_CSTRING(
                    TRC_CMPIPROVIDERINTERFACE,
                    Tracer::LEVEL1,
                    "Received invalid Parameter...");
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return UnknownErrorType;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return UnknownErrorType;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return(CMPIErrorType)pgErrorType;
    }

    static CMPIString* errGetOtherErrorType(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetOtherErrorType()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid Handle - cer...");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgOtherErrorType;

        try
        {
            notNull = cer->getOtherErrorType(pgOtherErrorType);
            if (!notNull)
            {
                PEG_TRACE_CSTRING(
                    TRC_CMPIPROVIDERINTERFACE,
                    Tracer::LEVEL1,
                    "Received invalid Parameter...");
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Exception: Unknown Exception caught...");
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString(pgOtherErrorType);
    }

    static CMPIString* errGetOwningEntity(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetOwningEntity()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid Handle - cer...");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgOwningEntity;

        try
        {
            notNull = cer->getOwningEntity(pgOwningEntity);
            if (!notNull)
            {
                PEG_TRACE_CSTRING(
                    TRC_CMPIPROVIDERINTERFACE,
                    Tracer::LEVEL1,
                    "Received invalid Parameter...");
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Exception: Unknown Exception caught...");
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString(pgOwningEntity);
    }

    static CMPIString* errGetMessageID(const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetMessageID()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgMessageID;

        try
        {
            notNull = cer->getMessageID(pgMessageID);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString(pgMessageID);
    }

    static CMPIString* errGetMessage(const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetMessage()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgMessage;

        try
        {
            notNull = cer->getMessage(pgMessage);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString(pgMessage);
    }

    static CMPIErrorSeverity errGetPerceivedSeverity(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetPerceivedSeverity()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return ErrorSevUnknown;
        }

        CMPIBoolean notNull;
        CIMError::PerceivedSeverityEnum pgPerceivedSeverity;

        try
        {
            notNull = cer->getPerceivedSeverity(pgPerceivedSeverity);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return ErrorSevUnknown;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return ErrorSevUnknown;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return(CMPIErrorSeverity)pgPerceivedSeverity;
    }

    static CMPIErrorProbableCause errGetProbableCause(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetProbableCause()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return ErrorProbCauseUnknown;
        }

        CMPIBoolean notNull;
        CIMError::ProbableCauseEnum pgProbableCause;

        try
        {
            notNull = cer->getProbableCause(pgProbableCause);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return ErrorProbCauseUnknown;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return ErrorProbCauseUnknown;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return(CMPIErrorProbableCause)pgProbableCause;
    }

    static CMPIString* errGetProbableCauseDescription(
        const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetProbableCauseDescription()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgProbCauseDesc;

        try
        {
            notNull = cer->getProbableCauseDescription(pgProbCauseDesc);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString(pgProbCauseDesc);
    }

    static CMPIArray* errGetRecommendedActions(
        const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetRecommendedActions()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        Array<String> pgRecommendedActions;

        try
        {
            notNull = cer->getRecommendedActions(pgRecommendedActions);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIUint32 arrSize = pgRecommendedActions.size();
        CMPIData *dta=new CMPIData[arrSize+1];
        // first element reserved for type and size
        dta->type=CMPI_string;
        dta->value.uint32=arrSize;
        for (unsigned int i=1; i<=arrSize; i++)
        {
            dta[i].type=CMPI_string;
            dta[i].state=CMPI_goodValue;
            String s = pgRecommendedActions[i-1];
            dta[i].value.string=string2CMPIString(s);
        }
        CMPI_Array *arr = new CMPI_Array(dta);
        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return reinterpret_cast<CMPIArray*>(new CMPI_Object(arr));
    }

    static CMPIString* errGetErrorSource(const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetErrorSource()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgErrorSource;

        try
        {
            notNull = cer->getErrorSource(pgErrorSource);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString(pgErrorSource);
    }

    static CMPIErrorSrcFormat errGetErrorSourceFormat(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetErrorSourceFormat()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return CMPIErrSrcUnknown;
        }

        CMPIBoolean notNull;
        CIMError::ErrorSourceFormatEnum pgErrorSourceFormat;

        try
        {
            notNull = cer->getErrorSourceFormat(pgErrorSourceFormat);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return CMPIErrSrcUnknown;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return CMPIErrSrcUnknown;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return(CMPIErrorSrcFormat)pgErrorSourceFormat;
    }

    static CMPIString* errGetOtherErrorSourceFormat(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetOtherErrorSourceFormat()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgOtherErrorSourceFormat;

        try
        {
            notNull = cer->getOtherErrorSourceFormat(pgOtherErrorSourceFormat);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        CMPIString* cmpiString = string2CMPIString(pgOtherErrorSourceFormat);

        PEG_METHOD_EXIT();
        return cmpiString;
    }

    static CMPIrc errGetCIMStatusCode(const CMPIError* eErr, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetCIMStatusCode()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return CMPI_RC_ERR_INVALID_PARAMETER;
        }

        CMPIBoolean notNull;
        CIMError::CIMStatusCodeEnum pgCIMStatusCode;

        try
        {
            notNull = cer->getCIMStatusCode(pgCIMStatusCode);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return CMPI_RC_ERR_INVALID_PARAMETER;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return CMPI_RC_ERR_FAILED;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return(CMPIrc)pgCIMStatusCode;
    }

    static CMPIString* errGetCIMStatusCodeDescription(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetCIMStatusCodeDescription()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        String pgCIMStatusCodeDescription;

        try
        {
            notNull = cer->getCIMStatusCodeDescription(
                pgCIMStatusCodeDescription);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString(pgCIMStatusCodeDescription);
    }

    static CMPIArray* errGetMessageArguments(
        const CMPIError* eErr,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errGetMessageArguments()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIBoolean notNull;
        Array<String> pgMessageArguments;

        try
        {
            notNull = cer->getMessageArguments(pgMessageArguments);
            if (!notNull)
            {
                CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
                PEG_METHOD_EXIT();
                return NULL;
            }
        }
        catch (...)
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return NULL;
        }

        CMPIUint32 arrSize = pgMessageArguments.size();
        CMPIData *dta=new CMPIData[arrSize+1];
        // first element reserved for type and size
        dta->type=CMPI_string;
        dta->value.uint32=arrSize;
        for (unsigned int i=1; i<=arrSize; i++)
        {
            dta[i].type=CMPI_string;
            dta[i].state=CMPI_goodValue;
            String s = pgMessageArguments[i-1];
            dta[i].value.string=string2CMPIString(s);
        }
        CMSetStatus(rc,CMPI_RC_OK);
        CMPIArray* cmpiArray =
            reinterpret_cast<CMPIArray*>(new CMPI_Object(new CMPI_Array(dta)));
        PEG_METHOD_EXIT();
        return cmpiArray;
    }

    static CMPIStatus errSetErrorType(
        CMPIError* eErr,
        const CMPIErrorType errorType)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetErrorType()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        CIMError::ErrorTypeEnum pgErrorType;
        pgErrorType = (CIMError::ErrorTypeEnum)errorType;

        try
        {
            cer->setErrorType(pgErrorType);
        }
        catch (...)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetOtherErrorType(
        CMPIError* eErr,
        const char* otherErrorType)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetOtherErrorType()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        String pgOtherErrorType(otherErrorType);

        try
        {
            cer->setOtherErrorType(pgOtherErrorType);
        }
        catch (...)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetProbableCauseDescription(
        CMPIError* eErr,
        const char* probableCauseDescription)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetProbableCauseDescription()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        String pgProbableCauseDescription(probableCauseDescription);

        try
        {
            cer->setProbableCauseDescription(pgProbableCauseDescription);
        }
        catch (...)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetRecommendedActions(
        CMPIError* eErr,
        const CMPIArray* recommendedActions)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetRecommendedActions()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        Array<String> pgRecommendedActions;
        CMPI_Array* arr = (CMPI_Array*)recommendedActions->hdl;
        if (!arr)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }
        CMPIData* dta=(CMPIData*)arr->hdl;
        if (!dta)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            for (unsigned int i=0; i<dta->value.uint32; i++)
            {
                CMPIData arrEl;
                const char * arrElStr;
                CMPIStatus rc = {CMPI_RC_OK,NULL};

                if (dta[i].type!=CMPI_string)
                {
                    PEG_METHOD_EXIT();
                    CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
                }

                arrEl = CMGetArrayElementAt(recommendedActions, i, &rc);
                if (rc.rc != CMPI_RC_OK)
                {
                    PEG_METHOD_EXIT();
                    return rc;
                }

                arrElStr = CMGetCharsPtr(arrEl.value.string, &rc);
                if (rc.rc != CMPI_RC_OK)
                {
                    PEG_METHOD_EXIT();
                    return rc;
                }

                pgRecommendedActions.append(String(arrElStr));
            }

            cer->setRecommendedActions(pgRecommendedActions);
        }
        catch (...)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetErrorSource(
        CMPIError* eErr,
        const char* errorSource)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetErrorSource()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid Handle - cer...");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        String pgErrorSource(errorSource);

        try
        {
            cer->setErrorSource(pgErrorSource);
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Exception: Unknown Exception received...");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetErrorSourceFormat(
        CMPIError* eErr,
        const CMPIErrorSrcFormat errorSrcFormat)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetErrorSourceFormat()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        CIMError::ErrorSourceFormatEnum pgErrorSourceFormat;
        pgErrorSourceFormat = (CIMError::ErrorSourceFormatEnum)errorSrcFormat;

        try
        {
            cer->setErrorSourceFormat(pgErrorSourceFormat);
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Exception: Unknown Exception received...");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetOtherErrorSourceFormat(
        CMPIError* eErr,
        const char* otherErrorSourceFormat)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetOtherErrorSourceFormat()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        String pgOtherErrorSourceFormat(otherErrorSourceFormat);

        try
        {
            cer->setOtherErrorSourceFormat(pgOtherErrorSourceFormat);
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Exception: Unknown Exception received...");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetCIMStatusCodeDescription(
        CMPIError* eErr,
        const char* cimStatusCodeDescription)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetCIMStatusCodeDescription()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        String pgCIMStatusCodeDescription(cimStatusCodeDescription);

        try
        {
            cer->setCIMStatusCodeDescription(pgCIMStatusCodeDescription);
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Exception: Unknown Exception received...");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus errSetMessageArguments(
        CMPIError* eErr,
        CMPIArray* messageArguments)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Error:errSetMessageArguments()");
        CIMError* cer=(CIMError*)eErr->hdl;
        if (!cer)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        Array<String> pgMessageArguments;
        CMPI_Array* arr = (CMPI_Array*)messageArguments->hdl;
        if (!arr)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }
        CMPIData* dta=(CMPIData*)arr->hdl;
        if (!dta)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            for (unsigned int i=0; i<dta->value.uint32; i++)
            {
                CMPIData arrEl;
                const char * arrElStr;
                CMPIStatus rc = {CMPI_RC_OK,NULL};

                if (dta[i].type!=CMPI_string)
                {
                    PEG_METHOD_EXIT();
                    CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
                }

                arrEl = CMGetArrayElementAt(messageArguments, i, &rc);
                if (rc.rc != CMPI_RC_OK)
                {
                    PEG_METHOD_EXIT();
                    return rc;
                }

                arrElStr = CMGetCharsPtr(arrEl.value.string, &rc);
                if (rc.rc != CMPI_RC_OK)
                {
                    PEG_METHOD_EXIT();
                    return rc;
                }

                pgMessageArguments.append(String(arrElStr));
            }

            cer->setMessageArguments(pgMessageArguments);
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Exception: Unknown Exception received...");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_FAILED);
        }

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

}

static CMPIErrorFT error_FT =
{
    CMPICurrentVersion,
    errRelease,
    errClone,
    errGetErrorType,
    errGetOtherErrorType,
    errGetOwningEntity,
    errGetMessageID,
    errGetMessage,
    errGetPerceivedSeverity,
    errGetProbableCause,
    errGetProbableCauseDescription,
    errGetRecommendedActions,
    errGetErrorSource,
    errGetErrorSourceFormat,
    errGetOtherErrorSourceFormat,
    errGetCIMStatusCode,
    errGetCIMStatusCodeDescription,
    errGetMessageArguments,
    errSetErrorType,
    errSetOtherErrorType,
    errSetProbableCauseDescription,
    errSetRecommendedActions,
    errSetErrorSource,
    errSetErrorSourceFormat,
    errSetOtherErrorSourceFormat,
    errSetCIMStatusCodeDescription,
    errSetMessageArguments,
};

CMPIErrorFT *CMPI_Error_Ftab=&error_FT;

PEGASUS_NAMESPACE_END


