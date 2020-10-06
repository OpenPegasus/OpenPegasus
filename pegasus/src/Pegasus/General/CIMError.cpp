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

#include "MofWriter.h"
#include "CIMError.h"
#include "PropertyAccessor.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// Required property list.

static const char* _requiredProperties[] =
{
    "OwningEntity",
    "MessageID",
    "Message",
    "PerceivedSeverity",
    "ProbableCause",
    "CIMStatusCode",
};

static const size_t _numRequiredProperties =
    sizeof(_requiredProperties) / sizeof(_requiredProperties[0]);

CIMError::CIMError() : _inst("CIM_Error")
{
    _inst.addProperty(CIMProperty(
        "ErrorType", CIMValue(CIMTYPE_UINT16, false)));
    _inst.addProperty(CIMProperty(
        "OtherErrorType", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "OwningEntity", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "MessageID", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "Message", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "MessageArguments", CIMValue(CIMTYPE_STRING, true)));
    _inst.addProperty(CIMProperty(
        "PerceivedSeverity", CIMValue(CIMTYPE_UINT16, false)));
    _inst.addProperty(CIMProperty(
        "ProbableCause", CIMValue(CIMTYPE_UINT16, false)));
    _inst.addProperty(CIMProperty(
        "ProbableCauseDescription", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "RecommendedActions", CIMValue(CIMTYPE_STRING, true)));
    _inst.addProperty(CIMProperty(
        "ErrorSource", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "ErrorSourceFormat", CIMValue(CIMTYPE_UINT16, false)));
    _inst.addProperty(CIMProperty(
        "OtherErrorSourceFormat", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "CIMStatusCode", CIMValue(CIMTYPE_UINT32, false)));
    _inst.addProperty(CIMProperty(
        "CIMStatusCodeDescription", CIMValue(CIMTYPE_STRING, false)));
}

CIMError::CIMError(const String& owningEntity,
                   const String& messageID,
                   const String& message,
                   const PerceivedSeverityEnum& perceivedSeverity,
                   const ProbableCauseEnum& probableCause,
                   const CIMStatusCodeEnum& cimStatusCode)
: _inst("CIM_Error")
{
    _inst.addProperty(CIMProperty(
        "ErrorType", CIMValue(CIMTYPE_UINT16, false)));
    _inst.addProperty(CIMProperty(
        "OtherErrorType", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "OwningEntity", CIMValue(owningEntity)));
    _inst.addProperty(CIMProperty(
        "MessageID", CIMValue(messageID)));
    _inst.addProperty(CIMProperty(
        "Message", CIMValue(message)));
    _inst.addProperty(CIMProperty(
        "MessageArguments", CIMValue(CIMTYPE_STRING, true)));
    _inst.addProperty(CIMProperty(
        "PerceivedSeverity", CIMValue(Uint16(perceivedSeverity))));
    _inst.addProperty(CIMProperty(
        "ProbableCause", CIMValue(Uint16(probableCause))));
    _inst.addProperty(CIMProperty(
        "ProbableCauseDescription", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "RecommendedActions", CIMValue(CIMTYPE_STRING, true)));
    _inst.addProperty(CIMProperty(
        "ErrorSource", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "ErrorSourceFormat", CIMValue(CIMTYPE_UINT16, false)));
    _inst.addProperty(CIMProperty(
        "OtherErrorSourceFormat", CIMValue(CIMTYPE_STRING, false)));
    _inst.addProperty(CIMProperty(
        "CIMStatusCode", CIMValue(Uint32(cimStatusCode))));
    _inst.addProperty(CIMProperty(
        "CIMStatusCodeDescription", CIMValue(CIMTYPE_STRING, false)));
}

CIMError::CIMError(const CIMError& x) : _inst(x._inst)
{
}

CIMError::~CIMError()
{
}

bool CIMError::getErrorType(ErrorTypeEnum& value) const
{
    Uint16 t;
    bool nullStat = Get(_inst, "ErrorType", t);
    value = ErrorTypeEnum(t);
    return nullStat;
}

void CIMError::setErrorType(ErrorTypeEnum value, bool null)
{
    Set(_inst, "ErrorType", Uint16(value), null);
}

bool CIMError::getOtherErrorType(String& value) const
{
    return Get(_inst, "OtherErrorType", value);
}

void CIMError::setOtherErrorType(const String& value, bool null)
{
    Set(_inst, "OtherErrorType", value, null);
}

bool CIMError::getOwningEntity(String& value) const
{
    return Get(_inst, "OwningEntity", value);
}

void CIMError::setOwningEntity(const String& value, bool null)
{
    Set(_inst, "OwningEntity", value, null);
}

bool CIMError::getMessageID(String& value) const
{
   return Get(_inst, "MessageID", value);
}

void CIMError::setMessageID(const String& value, bool null)
{
    Set(_inst, "MessageID", value, null);
}

bool CIMError::getMessage(String& value) const
{
    return Get(_inst, "Message", value);
}

void CIMError::setMessage(const String& value, bool null)
{
    Set(_inst, "Message", value, null);
}

bool CIMError::getMessageArguments(Array<String>&  value) const
{
    return Get(_inst, "MessageArguments", value);
}

void CIMError::setMessageArguments(const Array<String>& value, bool null)
{
    Set(_inst, "MessageArguments", value, null);
}

bool CIMError::getPerceivedSeverity(
    PerceivedSeverityEnum& value) const
{
    Uint16 t;
    bool nullStat = Get(_inst, "PerceivedSeverity", t);
    value = PerceivedSeverityEnum(t);
    return nullStat;
}

void CIMError::setPerceivedSeverity(
    PerceivedSeverityEnum value, bool null)
{
    Set(_inst, "PerceivedSeverity", Uint16(value), null);
}

bool CIMError::getProbableCause(ProbableCauseEnum& value) const
{
    Uint16 t;
    bool nullStat = Get(_inst, "ProbableCause", t);
    value = ProbableCauseEnum(t);
    return nullStat;
}

void CIMError::setProbableCause(ProbableCauseEnum value, bool null)
{
    Set(_inst, "ProbableCause", (Uint16)value, null);
}

bool CIMError::getProbableCauseDescription(String& value) const
{
    return Get(_inst, "ProbableCauseDescription", value);
}

void CIMError::setProbableCauseDescription(const String& value, bool null)
{
    Set(_inst, "ProbableCauseDescription", value, null);
}

bool CIMError::getRecommendedActions(Array<String>& value) const
{
    return Get(_inst, "RecommendedActions", value);
}

void CIMError::setRecommendedActions(const Array<String>& value, bool null)
{
    Set(_inst, "RecommendedActions", value, null);
}

bool CIMError::getErrorSource(String& value) const
{
    return Get(_inst, "ErrorSource", value);
}

void CIMError::setErrorSource(const String& value, bool null)
{
    Set(_inst, "ErrorSource", value, null);
}

bool CIMError::getErrorSourceFormat(
    ErrorSourceFormatEnum& value) const
{
    Uint16 t;
    bool nullStat = Get(_inst, "ErrorSourceFormat", t);
    value = ErrorSourceFormatEnum(t);
    return nullStat;
}

void CIMError::setErrorSourceFormat(ErrorSourceFormatEnum value, bool null)
{
    Set(_inst, "ErrorSourceFormat", Uint16(value), null);
}

bool CIMError::getOtherErrorSourceFormat(String& value) const
{
    return Get(_inst, "OtherErrorSourceFormat", value);
}

void CIMError::setOtherErrorSourceFormat(const String& value, bool null)
{
    Set(_inst, "OtherErrorSourceFormat", value, null);
}

bool CIMError::getCIMStatusCode(CIMStatusCodeEnum& value) const
{
    Uint32 t;
    bool nullStat = Get(_inst, "CIMStatusCode", t);
    value = CIMStatusCodeEnum(t);
    return nullStat;
}

void CIMError::setCIMStatusCode(CIMStatusCodeEnum value, bool null)
{
    Set(_inst, "CIMStatusCode", Uint32(value), null);
}

bool CIMError::getCIMStatusCodeDescription(String& value) const
{
    return Get(_inst, "CIMStatusCodeDescription", value);
}

void CIMError::setCIMStatusCodeDescription(const String& value, bool null)
{
    Set(_inst, "CIMStatusCodeDescription", value, null);
}

const CIMInstance& CIMError::getInstance() const
{
    return _inst;
}

template<class T>
void _Check(const String& name, CIMConstProperty& p, T* tag)
{
    if (p.getName() == name)
    {
        if (IsArray(tag) != p.isArray() || GetType(tag) != p.getType())
            throw CIMException(CIM_ERR_TYPE_MISMATCH, name);
    }
}

void CIMError::setInstance(const CIMInstance& instance)
{
    for (Uint32 i = 0; i < instance.getPropertyCount(); i++)
    {
        CIMConstProperty p = instance.getProperty(i);

        _Check("ErrorType", p, (Uint16*)0);
        _Check("OtherErrorType", p, (String*)0);
        _Check("OwningEntity", p, (String*)0);
        _Check("MessageID", p, (String*)0);
        _Check("Message", p, (String*)0);
        _Check("MessageArguments", p, (Array<String>*)0);
        _Check("PerceivedSeverity", p, (Uint16*)0);
        _Check("ProbableCause", p, (Uint16*)0);
        _Check("ProbableCauseDescription", p, (String*)0);
        _Check("RecommendedActions", p, (Array<String>*)0);
        _Check("ErrorSource", p, (String*)0);
        _Check("ErrorSourceFormat", p, (Uint16*)0);
        _Check("OtherErrorSourceFormat", p, (String*)0);
        _Check("CIMStatusCode", p, (Uint32*)0);
        _Check("CIMStatusCodeDescription", p, (String*)0);
    }

    // Verify that the instance contains all of the required properties.

    for (Uint32 i = 0; i < _numRequiredProperties; i++)
    {
        // Does inst have this property?

        Uint32 pos = instance.findProperty(_requiredProperties[i]);

        if (pos == PEG_NOT_FOUND)
        {
            char buffer[80];
            sprintf(buffer, "required property does not exist: %s",
                _requiredProperties[i]);
            throw CIMException(CIM_ERR_NO_SUCH_PROPERTY, buffer);
        }
        // is required property non-null?
        CIMConstProperty p = instance.getProperty(pos);
        CIMValue v = p.getValue();
        if (v.isNull())
        {
            char buffer[80];
            sprintf(buffer, "required property MUST NOT be Null: %s",
                _requiredProperties[i]);
            throw CIMException(CIM_ERR_FAILED, buffer);
        }
    }
    _inst = instance;
}

void CIMError::print() const
{
    Buffer buf;
    MofWriter::appendInstanceElement(buf, _inst);
    printf("%.*s\n", int(buf.size()), buf.getData());
}

PEGASUS_NAMESPACE_END
