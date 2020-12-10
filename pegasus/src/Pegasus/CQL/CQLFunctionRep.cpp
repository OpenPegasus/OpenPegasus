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

#include <Pegasus/CQL/CQLFunctionRep.h>

// CQL includes
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/CQL/CQLUtilities.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/CQL/CQLValueRep.h>

// Common Query includes
#include <Pegasus/Query/QueryCommon/QueryContext.h>

// Common Pegasus includes
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

CQLFunctionRep::CQLFunctionRep(): _funcOpType(UNKNOWN), _parms() {}

CQLFunctionRep::CQLFunctionRep(
    CQLIdentifier inOpType,
    Array<CQLPredicate> inParms)
    : _funcOpType(UNKNOWN), _parms(inParms)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::CQLFunctionRep()");

    String opType(inOpType.getName().getString());

    if (String::compareNoCase(opType,String("DATETIMETOMICROSECOND")) == 0)
    {
        _funcOpType = DATETIMETOMICROSECOND;
    }
    else if (String::compareNoCase(opType, String("STRINGTOUINT")) == 0)
    {
        _funcOpType = STRINGTOUINT;
    }
    else if (String::compareNoCase(opType, String("STRINGTOSINT")) == 0)
    {
        _funcOpType = STRINGTOSINT;
    }
    else if (String::compareNoCase(opType, String("STRINGTOREAL")) == 0)
    {
        _funcOpType = STRINGTOREAL;
    }
    else if (String::compareNoCase(opType, String("STRINGTONUMERIC")) == 0)
    {
        _funcOpType = STRINGTONUMERIC;
    }
    else if (String::compareNoCase(opType, String("UPPERCASE")) == 0)
    {
        _funcOpType = UPPERCASE;
    }
    else if (String::compareNoCase(opType, String("NUMERICTOSTRING")) == 0)
    {
        _funcOpType = NUMERICTOSTRING;
    }
    else if (String::compareNoCase(opType, String("REFERENCETOSTRING")) == 0)
    {
        _funcOpType = REFERENCETOSTRING;
    }
    else if (String::compareNoCase(opType, String("CLASSNAME")) == 0)
    {
        _funcOpType = CLASSNAME;
    }
    else if (String::compareNoCase(opType, String("NAMESPACENAME")) == 0)
    {
        _funcOpType = NAMESPACENAME;
    }
    else if (String::compareNoCase(opType, String("NAMESPACETYPE")) == 0)
    {
        _funcOpType = NAMESPACETYPE;
    }
    else if (String::compareNoCase(opType, String("HOSTPORT")) == 0)
    {
        _funcOpType = HOSTPORT;
    }
    else if (String::compareNoCase(opType, String("MODELPATH")) == 0)
    {
        _funcOpType = MODELPATH;
    }
    else if (String::compareNoCase(opType, String("CLASSPATH")) == 0)
    {
        _funcOpType = CLASSPATH;
    }
    else if (String::compareNoCase(opType, String("OBJECTPATH")) == 0)
    {
        _funcOpType = OBJECTPATH;
    }
    else if (String::compareNoCase(opType, String("INSTANCETOREFERENCE")) == 0)
    {
        _funcOpType = INSTANCETOREFERENCE;
    }
    else if (String::compareNoCase(opType, String("CURRENTDATETIME")) == 0)
    {
        _funcOpType = CURRENTDATETIME;
    }
    else if (String::compareNoCase(opType, String("DATETIME")) == 0)
    {
        _funcOpType = DATETIME;
    }
    else if (String::compareNoCase(
                 opType, String("MICROSECONDTOTIMESTAMP")) == 0)
    {
        _funcOpType = MICROSECONDTOTIMESTAMP;
    }
    else if (String::compareNoCase(
                 opType, String("MICROSECONDTOINTERVAL")) == 0)
    {
        _funcOpType = MICROSECONDTOINTERVAL;
    }
    else
    {
        // Unknown function
        MessageLoaderParms mload(
            "CQL.CQLFunctionRep.INVALID_FUNCTION",
            "Function: $0 is not a supported function.",
            opType);
        throw CQLSyntaxErrorException(mload);
    }
    PEG_METHOD_EXIT();
}

CQLFunctionRep::CQLFunctionRep(const CQLFunctionRep* rep)
    : _funcOpType(rep->_funcOpType), _parms(rep->_parms)
{
}

CQLFunctionRep::~CQLFunctionRep()
{
}

CQLValue CQLFunctionRep::resolveValue(
    const CIMInstance& CI,
    const QueryContext& queryCtx)
{
    switch(_funcOpType)
    {
        case DATETIMETOMICROSECOND:
          return dateTimeToMicrosecond(CI, queryCtx);
        case STRINGTOUINT:
          return stringToUint(CI, queryCtx);
        case STRINGTOSINT:
          return stringToSint(CI, queryCtx);
        case STRINGTOREAL:
          return stringToReal(CI, queryCtx);
        case STRINGTONUMERIC:
          return stringToNumeric(CI, queryCtx);
        case UPPERCASE:
          return upperCase(CI, queryCtx);
        case NUMERICTOSTRING:
          return numericToString(CI, queryCtx);
        case REFERENCETOSTRING:
          return referenceToString(CI, queryCtx);
        case CLASSNAME:
          return className(CI, queryCtx);
        case NAMESPACENAME:
          return nameSpaceName(CI, queryCtx);
        case NAMESPACETYPE:
          return nameSpaceType(CI, queryCtx);
        case HOSTPORT:
          return hostPort(CI, queryCtx);
        case MODELPATH:
          return modelPath(CI, queryCtx);
        case CLASSPATH:
          return classPath(CI, queryCtx);
        case OBJECTPATH:
          return objectPath(CI, queryCtx);
        case INSTANCETOREFERENCE:
          return instanceToReference(CI, queryCtx);
        case CURRENTDATETIME:
          return currentDateTime();
        case DATETIME:
          return dateTime(CI, queryCtx);
        case MICROSECONDTOTIMESTAMP:
          return microsecondToTimestamp(CI, queryCtx);
        case MICROSECONDTOINTERVAL:
          return microsecondToInterval(CI, queryCtx);
        case UNKNOWN:
            // Unknown function; this case is handled below
            break;
    }

    // Unknown function
    MessageLoaderParms mload(
        "CQL.CQLFunctionRep.INVALID_FUNCTION_ID",
        "Function ID: $0 is not a supported function ID.",
        Formatter::Arg(Uint32(_funcOpType)));
    throw CQLSyntaxErrorException(mload);
}

String CQLFunctionRep::toString() const
{
    String returnStr = functionTypeToString();

    returnStr.append("(");
    Uint32 parmSize = _parms.size();
    for(Uint32 i = 0; i < parmSize; ++i)
    {
        returnStr.append(_parms[i].toString());
        if (i+1 < parmSize)
            returnStr.append(", ");
    }
    returnStr.append(")");
    return returnStr;
}

String CQLFunctionRep::functionTypeToString() const
{
    String returnStr;

    switch(_funcOpType)
    {
        case DATETIMETOMICROSECOND:
            returnStr.append("DATETIMETOMICROSECOND");
            break;
        case STRINGTOUINT:
            returnStr.append("STRINGTOUINT");
            break;
        case STRINGTOSINT:
            returnStr.append("STRINGTOSINT");
            break;
        case STRINGTOREAL:
            returnStr.append("STRINGTOREAL");
            break;
        case STRINGTONUMERIC:
            returnStr.append("STRINGTONUMERIC");
            break;
        case UPPERCASE:
            returnStr.append("UPPERCASE");
            break;
        case NUMERICTOSTRING:
            returnStr.append("NUMERICTOSTRING");
            break;
        case REFERENCETOSTRING:
            returnStr.append("REFERENCETOSTRING");
            break;
        case CLASSNAME:
            returnStr.append("CLASSNAME");
            break;
        case NAMESPACENAME:
            returnStr.append("NAMESPACENAME");
            break;
        case NAMESPACETYPE:
            returnStr.append("NAMESPACETYPE");
            break;
        case HOSTPORT:
            returnStr.append("HOSTPORT");
            break;
        case MODELPATH:
            returnStr.append("MODELPATH");
            break;
        case CLASSPATH:
            returnStr.append("CLASSPATH");
            break;
        case OBJECTPATH:
            returnStr.append("OBJECTPATH");
            break;
        case INSTANCETOREFERENCE:
            returnStr.append("INSTANCETOREFERENCE");
            break;
        case CURRENTDATETIME:
            returnStr.append("CURRENTDATETIME");
            break;
        case DATETIME:
            returnStr.append("DATETIME");
            break;
        case MICROSECONDTOTIMESTAMP:
            returnStr.append("MICROSECONDTOTIMESTAMP");
            break;
        case MICROSECONDTOINTERVAL:
            returnStr.append("MICROSECONDTOINTERVAL");
            break;
        case UNKNOWN:
            returnStr.append("UNKNOWN");
        default:
            returnStr.append("UNKNOWN");
            break;
    }
    return returnStr;
}

Array<CQLPredicate> CQLFunctionRep::getParms()const
{
    return _parms;
}

FunctionOpType CQLFunctionRep::getFunctionType()const
{
    return _funcOpType;
}

void CQLFunctionRep::applyContext(const QueryContext& inContext)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::applyContext()");

    for(Uint32 i = 0; i < _parms.size(); ++i)
    {
        _parms[i].applyContext(inContext);
    }
    PEG_METHOD_EXIT();
}
/*
Boolean CQLFunctionRep::operator==(const CQLFunctionRep& func)const
{
  if(_funcOpType != func._funcOpType || _parms.size() != func._parms.size())
  {
    return false;
  }

//  for(Uint32 i = 0; i < _parms.size(); ++i)
//  {
//    if(!(_parms[i] == func._parms[i]))
//    {
//      return false;
//    }
//  }

  return true;
}

Boolean CQLFunctionRep::operator!=(const CQLFunctionRep& func)const{
    return (!operator==(func));
}
*/

CQLValue CQLFunctionRep::dateTimeToMicrosecond(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::dateTimeToMicrosecond()");

    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters.  It must have between"
                " $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    // resolve the parameter
    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
        resolveValue(CI,queryCtx);

    if(cqlVal.getValueType() != CQLValue::CIMDateTime_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            CQLValueRep::valueTypeToString(CQLValue::CIMDateTime_type));
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_UINT64, false));
    }

    PEG_METHOD_EXIT();
    return CQLValue(cqlVal.getDateTime().toMicroSeconds());
}

CQLValue CQLFunctionRep::stringToUint(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::stringToUint()");
    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    if(cqlVal.getValueType() != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            CQLValueRep::valueTypeToString(CQLValue::String_type));
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_UINT64, false));
    }

    PEG_METHOD_EXIT();
    return CQLValue(CQLUtilities::stringToUint64(cqlVal.getString()));
}

CQLValue CQLFunctionRep::stringToSint(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::stringToSint()");
    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
        resolveValue(CI,queryCtx);

    if(cqlVal.getValueType() != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            CQLValueRep::valueTypeToString(CQLValue::String_type));
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_SINT64, false));
    }

    PEG_METHOD_EXIT();
    return CQLValue(CQLUtilities::stringToSint64(cqlVal.getString()));
}

CQLValue CQLFunctionRep::stringToReal(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::stringToReal()");
    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    if(cqlVal.getValueType() != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            CQLValueRep::valueTypeToString(CQLValue::String_type));
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_REAL64, false));
    }

    PEG_METHOD_EXIT();
    return CQLValue(CQLUtilities::stringToReal64(cqlVal.getString()));
}

CQLValue CQLFunctionRep::stringToNumeric(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    return stringToReal(CI, queryCtx);
}

CQLValue CQLFunctionRep::upperCase(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::upperCase()");
    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    if(cqlVal.getValueType() != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            CQLValueRep::valueTypeToString(CQLValue::String_type));
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return cqlVal;
    }

    String tmpStr = cqlVal.getString();
    tmpStr.toUpper();

    PEG_METHOD_EXIT();
    return CQLValue(tmpStr);
}

CQLValue CQLFunctionRep::numericToString(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::numericToString()");

    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
        resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::Sint64_type && valType != CQLValue::Uint64_type
        && valType != CQLValue::Real_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            "Integer or Real");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_STRING, false));
    }

    char buffer[128];

    if (valType == CQLValue::Sint64_type)
    {
        sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d",
                cqlVal.getSint());
    }
    else if (valType == CQLValue::Uint64_type)
    {
        sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u",
                cqlVal.getUint());
    }
    else
    {
        sprintf(buffer, "%.16E", cqlVal.getReal());
    }

    String num(buffer);
    if (valType == CQLValue::Real_type)
    {
        // format the exponent
        num = CQLUtilities::formatRealStringExponent(num);
    }

    PEG_METHOD_EXIT();
    return CQLValue(num);
}

CQLValue CQLFunctionRep::referenceToString(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::referenceToString()");
    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
        resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::CIMReference_type &&
        valType != CQLValue::CIMObject_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(valType),
            "Reference, or Object");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_STRING, false));
    }

    if (valType == CQLValue::CIMReference_type)
    {
        PEG_METHOD_EXIT();
        return CQLValue(cqlVal.getReference().toString());
    }

    // We have CIMObject now, convert to string.
    PEG_METHOD_EXIT();
    return CQLValue(cqlVal.getObject().getPath().toString());
}

CQLValue CQLFunctionRep::className(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::className()");
    int parmSize = _parms.size();
    if (parmSize != 0 && parmSize != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "0", "1");
        throw CQLRuntimeException(mload);
    }

    // If there are no parameters, the default behavior is to
    // return the class name for the instance being examined (CI).
    if (parmSize == 0)
    {
        PEG_METHOD_EXIT();
        return CQLValue(CI.getClassName().getString());
    }

    // We have a parameter, so resolve it first before we use it.
    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
        resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::CIMReference_type &&
        valType != CQLValue::CIMObject_type &&
        valType != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(valType),
            "Reference, String, or Object");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_STRING, false));
    }

    // If we have a String parameter, then we'll use it to
    //  create a CIMObjectPath in order to verify the format is correct.
    //   We will then get the class from the object path and return it.
    if (cqlVal.getValueType() == CQLValue::String_type)
    {
        CIMObjectPath objPath(cqlVal.getString());
        PEG_METHOD_EXIT();
        return CQLValue(objPath.getClassName().getString());
    }

    // If we have a CIMReference parameter, then we will just get the class
    //  name from the reference and return it.  A refernce is a CIMObjectPath.
    if (cqlVal.getValueType() == CQLValue::CIMReference_type)
    {
        PEG_METHOD_EXIT();
        return CQLValue(cqlVal.getReference().getClassName().getString());
    }

    // We have a CIMObject, then we return the class name of the obejct
    PEG_METHOD_EXIT();
    return CQLValue(cqlVal.getObject().getClassName().getString());
}

CQLValue CQLFunctionRep::nameSpaceName(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::nameSpaceName()");
    Uint32 parmSize = _parms.size();
    if (parmSize != 0 && parmSize != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "0", "1");
        throw CQLRuntimeException(mload);
    }

    // The default beavior for this function will first look at the object path
    //  of the instance being examined (CI).  If the path contains a namespace,
    //  we will return it.  If it does not, then we will return the default
    //  namespace from the query context.
    // *** NOTE ***  This does not function entirely according to the CQL spec.
    //  The CQL spec says to return the namespace of the instance regardless
    //  if it is set or not.  However, with the current implementation
    //  (CQL phase 1 PEP 193) we only operate a query engine within a single
    //  namespace and so we can assume the default namespace.
    if (parmSize == 0)
    {
        CIMNamespaceName ns = CI.getPath().getNameSpace();
        if (ns.isNull() || String::equal(ns.getString(), String::EMPTY))
            ns = queryCtx.getNamespace();
        PEG_METHOD_EXIT();
        return CQLValue(ns.getString());
    }

    // We have a parameter, so resolve it first before we use it.
    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
        resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::CIMReference_type &&
        valType != CQLValue::CIMObject_type &&
        valType != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            "Reference, String, or Object");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_STRING, false));
    }

    // If we have a String parameter, then we'll use it to create a
    //  CIMObjectPath in order to verify the format is correct.
    //   We will then get the namespace from the object path and return it.
    //   If there is no namespace in the path given, then an empty string will
    //  be returned.
    if (cqlVal.getValueType() == CQLValue::String_type)
    {
        CIMObjectPath objPath(cqlVal.getString());
        PEG_METHOD_EXIT();
        return CQLValue(objPath.getNameSpace().getString());
    }

    // If we have a CIMReference parameter, then we will just get the
    //  namespace name from the reference and return it.  A refernce is a
    // CIMObjectPath.  If there is no namespace in the path given, then
    //  an empty string will be returned.
    if (cqlVal.getValueType() == CQLValue::CIMReference_type)
    {
        PEG_METHOD_EXIT();
        return CQLValue(cqlVal.getReference().getNameSpace().getString());
    }

    // We have a CIMObject, now we retrieve the path of the obejct and
    //  check to see if it has a namespace set in it.  If the path contains
    //  a namespace, we will return it.  If it does not, then we will
    //  return the default namespace from the query context.
    // *** NOTE ***  This does not function entirely according to the CQL spec.
    //   The CQL spec says to return the namespace of the instance
    //  regardless if it is set or not.  However, with the current
    //  implementation (CQL phase 1 PEP 193) we only operate a query
    //  engine within a single namespace and so we can assume the default
    //  namespace.
    CIMNamespaceName ns = cqlVal.getObject().getPath().getNameSpace();
    if (ns.isNull() || String::equal(ns.getString(), String::EMPTY))
        ns = queryCtx.getNamespace();
    PEG_METHOD_EXIT();
    return CQLValue(ns.getString());
}

CQLValue CQLFunctionRep::nameSpaceType(const CIMInstance& CI,
                                       const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::nameSpaceType()");
    // This is currently (as of CQL Phase 1, PEP 193) not supported in
    // Pegasus since Pegasus does not yet support WEBM URI references.
    // Nothing in the current object path can be used to represent
    // the name space type (i.e. the protocol).
    MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
        "Function $0 is not supported.",
        functionTypeToString());
    throw CQLRuntimeException(mload);

    PEGASUS_UNREACHABLE
    (
        int parmSize = _parms.size();
        if(parmSize != 0 && parmSize != 1)
        {
            MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
                "Function $0 has $1 parameters."
                    "  It must have between $2 and $3.",
                functionTypeToString(),
                _parms.size(),
                "0", "1");
            throw CQLRuntimeException(mload);
        }

        PEG_METHOD_EXIT();
        return CQLValue(Uint64(0));
    ) // End PEGASUS_UNREACHABLE
}

CQLValue CQLFunctionRep::hostPort(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::hostPort()");
    // Pegasus currently (as of CQL Phase 1, PEP 193) does not
    // support WEBM URI references, however the current object
    // path does have a host on it which we will return.  Until
    // Pegasus supports WBEM URI, it is not guaranteed that this
    // will conform to the format defined in WBEM URI, and therefore
    //  this function is not entirely in accordance with the CQL Specification.
    // In addition, the CQL specification allows for the parameter
    //  to be left off when the query is executed as a result of
    // an ExecuteQuery operation.  However, right now
    //  (as of CQL Phase 1, PEP 193) we are only using this for
    //  Indications, and so we are assuming a "Mode" of operation.
    //   For this function to be completely compliant with the CQL
    // specification, it will eventually need to be "mode" aware.

    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    // We have a parameter, so resolve it first before we use it.
    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::CIMReference_type &&
        valType != CQLValue::CIMObject_type &&
        valType != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(valType),
            "Reference, String, or Object");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_STRING, false));
    }

    // If we have a String parameter, then we'll use it to create a
    // CIMObjectPath in order to verify the format is correct.  We will
    //  then get the host from the object path and return it.  If there
    //  is no host in the path given, then an empty string will be returned.
    if (cqlVal.getValueType() == CQLValue::String_type)
    {
        CIMObjectPath objPath(cqlVal.getString());
        PEG_METHOD_EXIT();
        return CQLValue(objPath.getHost());
    }

    // If we have a CIMReference parameter, then we will just get the
    // host name from the reference and return it.  A reference is a
    //  CIMObjectPath.  If there is no host in the path given, then
    //  an empty string will be returned.
    if (cqlVal.getValueType() == CQLValue::CIMReference_type)
    {
        PEG_METHOD_EXIT();
        return CQLValue(cqlVal.getReference().getHost());
    }

    // We have a CIMObject, now we retrieve the path of the obejct
    //  and return the host from the path.  If there is no host in the
    //  path given, then an empty string will be returned.
    PEG_METHOD_EXIT();
    return CQLValue(cqlVal.getObject().getPath().getHost());
}

CQLValue CQLFunctionRep::modelPath(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::modelPath()");
    // This method returns the model path portion of an object path.
    // The model path is the class name and the key bindings (if included)..

    Uint32 parmSize = _parms.size();
    if (parmSize != 0 && parmSize != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "0", "1");
        throw CQLRuntimeException(mload);
    }

    // The default behavior for this function will be to retrieve
    // the object path from the instance being examined (CI) and
    // then return the model path from that.
    if (parmSize == 0)
    {
        PEG_METHOD_EXIT();
        return buildModelPath(CI.getPath());
    }

    // We have a parameter, so resolve it first before we use it.
    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::CIMReference_type &&
        valType != CQLValue::CIMObject_type &&
        valType != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(valType),
            "Reference, String, or Object");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_REFERENCE, false));
    }

    // If we have a String parameter, then we'll use it to create a
    // CIMObjectPath in order to verify the format is correct.
    // We will then get model path from the object path and return it.
    if (cqlVal.getValueType() == CQLValue::String_type)
    {
        CIMObjectPath objPath(cqlVal.getString());
        PEG_METHOD_EXIT();
        return buildModelPath(objPath);
    }

    // If we have a CIMReference parameter, then we will get the model
    // path from the reference and return it.  A reference is a
    // CIMObjectPath.
    if (cqlVal.getValueType() == CQLValue::CIMReference_type)
    {
        PEG_METHOD_EXIT();
        return buildModelPath(cqlVal.getReference());
    }

    // We have a CIMObject, now we retrieve the path of the obejct
    // and return the model path.
    PEG_METHOD_EXIT();
    return buildModelPath(cqlVal.getObject().getPath());
}

CQLValue CQLFunctionRep::buildModelPath(const CIMObjectPath& objPath) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::buildModelPath()");
    // This method will take an existing objet path, pick out the key
    // bindings and the class name, and use those parts to build a new
    // object path with just those parts in it.  This is used to
    // represent the model path.
    CIMObjectPath newPath;
    newPath.setClassName(objPath.getClassName());
    newPath.setKeyBindings(objPath.getKeyBindings());
    //printf("ModelPath --> %s\n", (const char *)newPath.toString().
    // getCString());
    PEG_METHOD_EXIT();
    return CQLValue(newPath.toString());
}

CQLValue CQLFunctionRep::classPath(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::classPath()");
    // This method returns a class path.  The class path will only have
    //  a namespace and a class name in it.  All other path information
    //  will be stripped off.
    Uint32 parmSize = _parms.size();
    if (parmSize != 0 && parmSize != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "0", "1");
        throw CQLRuntimeException(mload);
    }

    // The default behavior for this function will be to retrieve the
    // object path from the instance being examined (CI) and build the
    // class path from it.  If the path does not have a namespace, then
    // the default namespace is used.
    if (parmSize == 0)
    {
        CIMObjectPath objPath(CI.getPath());
        CIMNamespaceName ns = objPath.getNameSpace();
        if (ns.isNull() || String::equal(ns.getString(), String::EMPTY))
          ns = queryCtx.getNamespace();
        PEG_METHOD_EXIT();
        return buildClassPath(objPath, ns);
    }

    // We have a parameter, so resolve it first before we use it.
    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::CIMReference_type &&
        valType != CQLValue::CIMObject_type &&
        valType != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            "Reference, String, or Object");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_REFERENCE, false));
    }

    // If we have a String parameter, then we'll use it to create a
    // CIMObjectPath in order to verify the format is correct.  We will
    // then build the class path from the object path and return it.
    // If the namespace is NOT set in the object path, it will remain
    // unset in the returned reference.
    if (cqlVal.getValueType() == CQLValue::String_type)
    {
        CIMObjectPath objPath(cqlVal.getString());
        PEG_METHOD_EXIT();
        return buildClassPath(objPath, objPath.getNameSpace());
    }

    // If we have a CIMReference parameter, then we will build the class
    // path from the reference and return it.  If the namespace is NOT
    // set in the object path, it will remain unset in the returned reference.
    if (cqlVal.getValueType() == CQLValue::CIMReference_type)
    {
        CIMObjectPath objPath = cqlVal.getReference();
        PEG_METHOD_EXIT();
        return buildClassPath(objPath, objPath.getNameSpace());
    }

    // We have a CIMObject, now we retrieve the object path  and
    // build the class path from it.  If the path does not have a namespace,
    // then the default namespace is used.
    CIMObjectPath objPath = cqlVal.getObject().getPath();
    CIMNamespaceName ns = objPath.getNameSpace();
    if (ns.isNull() || String::equal(ns.getString(), String::EMPTY))
    {
        ns = queryCtx.getNamespace();
    }
    PEG_METHOD_EXIT();
    return buildClassPath(objPath, ns);
}

CQLValue CQLFunctionRep::buildClassPath(const CIMObjectPath& objPath,
                                        const CIMNamespaceName& ns) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::buildClassPath()");
    // This method will take the object path pass in and pick out the host,
    // the class name and the namespace.  The 2 parts are then combined
    // together into a new object path which will be used as the class
    // path and returned.
    CIMObjectPath newPath;
    newPath.setHost(objPath.getHost());
    newPath.setClassName(objPath.getClassName());
    newPath.setNameSpace(ns);
    // printf("ClassPath --> %s\n", (const char *)newPath.toString().
    // getCString());
    PEG_METHOD_EXIT();
    return CQLValue(newPath);
}


CQLValue CQLFunctionRep::objectPath(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::objectPath()");
    // This method returns an object path.  The object path will only have a
    // namespace, a class name, and key bindings if it is a path to an
    // instance.  All other path information will be stripped off.

    Uint32 parmSize = _parms.size();
    if (parmSize != 0 && parmSize != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "0", "1");
        throw CQLRuntimeException(mload);
    }

    // The default behavior for this function will be to retrieve the
    // object path from the instance being examined (CI) and build the
    // object path from it.  If the path does not have a namespace,
    // then the default namespace is used.
    if (parmSize == 0)
    {
        CIMObjectPath objPath(CI.getPath());
        CIMNamespaceName ns = objPath.getNameSpace();
        if (ns.isNull() || String::equal(ns.getString(), String::EMPTY))
          ns = queryCtx.getNamespace();
        PEG_METHOD_EXIT();
        return buildObjectPath(objPath, ns);
    }

    // We have a parameter, so resolve it first before we use it.
    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();

    if (valType != CQLValue::CIMReference_type &&
        valType != CQLValue::CIMObject_type &&
        valType != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            "Reference, String, or Object");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_REFERENCE, false));
    }

    // If we have a String parameter, then we'll use it to create a
    // CIMObjectPath in order to verify the format is correct.
    // We will then build the object path from the object path and return it.
    // If the namespace is NOT set in the object path, it will remain
    // unset in the returned reference.
    if (cqlVal.getValueType() == CQLValue::String_type)
    {
        CIMObjectPath objPath(cqlVal.getString());
        PEG_METHOD_EXIT();
        return buildObjectPath(objPath, objPath.getNameSpace());
    }

    // If we have a CIMReference parameter, then we will build the object
    // path from the reference and return it.  If the namespace is NOT
    // set in the object path, it will remain unset in the returned reference.
    if (cqlVal.getValueType() == CQLValue::CIMReference_type)
    {
        CIMObjectPath objPath = cqlVal.getReference();
        PEG_METHOD_EXIT();
        return buildObjectPath(objPath, objPath.getNameSpace());
    }

    // We have a CIMObject, now we retrieve the object path of the
    // obejct and build the object path from it.  If the path does not
    // have a namespace, then the default namespace is used.
    CIMObjectPath objPath = cqlVal.getObject().getPath();
    CIMNamespaceName ns = objPath.getNameSpace();
    if (ns.isNull() || String::equal(ns.getString(), String::EMPTY))
    {
        ns = queryCtx.getNamespace();
    }
    PEG_METHOD_EXIT();
    return buildObjectPath(objPath, ns);
}

CQLValue CQLFunctionRep::buildObjectPath(const CIMObjectPath& objPath,
                                         const CIMNamespaceName& ns) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::buildObjectPath()");
    // This method will take the object path passed in and pick out the host,
    // the class name, the namespace, and the key bindings.  The parts are
    // then combined together into a new object path which will be used
    // as the object path and returned.
    CIMObjectPath newPath;
    newPath.setHost(objPath.getHost());
    newPath.setClassName(objPath.getClassName());
    newPath.setNameSpace(ns);
    newPath.setKeyBindings(objPath.getKeyBindings());
    PEG_METHOD_EXIT();
    return CQLValue(newPath);
}

CQLValue CQLFunctionRep::instanceToReference(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::instanceToReference()");
    // The parameter to this function MUST be an instance object.
    // We will use buildPath on the instance to make the path.
    // If there is no namespace on the instance, then the default namespace
    // will be inserted.  The completed path is then returned.
    // Note, this could, and should be a more complete reference
    // than the other path functions.

    Uint32 parmSize = _parms.size();
    if (parmSize != 0 && parmSize != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "0", "1");
        throw CQLRuntimeException(mload);
    }
    CIMInstance *inst = NULL;
    Boolean cleanup = false;  // whether or not to delete the memory
    CIMObject obj;

    // The default behavior is to use the instance being examined
    // as the source instance (CI).
    if (parmSize == 0)
        inst = (CIMInstance *)&CI;
    else
    {
        // We have a parameter, so resolve it first before we use it.
        CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
            resolveValue(CI,queryCtx);

        // Parameter MUST be an instance object
        if (cqlVal.getValueType() != CQLValue::CIMObject_type)
        {
            MessageLoaderParms mload(
                "CQL.CQLFunctionRep.INVALID_PARM_TYPE",
                "Parameter $0 for function $1 has type $2."
                    "  It must be type $3.",
                "1",
                functionTypeToString(),
                CQLValueRep::valueTypeToString(cqlVal.getValueType()),
                CQLValueRep::valueTypeToString(CQLValue::CIMObject_type));
            throw CQLRuntimeException(mload);
        }

        if (cqlVal.isNull())
        {
            return CQLValue(CIMValue(CIMTYPE_REFERENCE, false));
        }

        // REVIEW question.  Inefficient since the CIMobject is copied
        //  via the return by value, then it is copied again via the
        // assignment.  Is there a better way to handle this?
        obj = cqlVal.getObject();
        if (!obj.isInstance())
        {
            MessageLoaderParms mload(
                "CQL.CQLFunctionRep.INVALID_OBJECT_TYPE",
                "Parameter $0 for function $1 must be a CIM instance.",
                "1",
                functionTypeToString());
            throw CQLRuntimeException(mload);
        }
        // Make a CIM Instance
        inst = new CIMInstance(obj);
        cleanup = true;
    }

    // Get the class and build the path
    CIMConstClass cls = queryCtx.getClass(inst->getClassName());
    CIMObjectPath objPath = inst->buildPath(cls);
    CIMNamespaceName ns = objPath.getNameSpace();

    if (ns.isNull() || String::equal(ns.getString(), String::EMPTY))
        objPath.setNameSpace(queryCtx.getNamespace());

    if (cleanup)
    {
        delete inst;
        inst = NULL;
    }
    PEG_METHOD_EXIT();
    return CQLValue(objPath);
}

CQLValue CQLFunctionRep::currentDateTime() const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::currentDateTime()");
    if (_parms.size() != 0)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "0", "0");
        throw CQLRuntimeException(mload);
    }

    PEG_METHOD_EXIT();
    return(CQLValue(CIMDateTime::getCurrentDateTime()));
}

CQLValue CQLFunctionRep::dateTime(
    const CIMInstance& CI,
    const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::dateTime()");
    if (_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
        resolveValue(CI,queryCtx);

    if(cqlVal.getValueType() != CQLValue::String_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(cqlVal.getValueType()),
            CQLValueRep::valueTypeToString(CQLValue::String_type));
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_DATETIME, false));
    }

    CIMDateTime dt(cqlVal.getString());
    PEG_METHOD_EXIT();
    return(CQLValue(dt));
}

CQLValue CQLFunctionRep::microsecondToTimestamp(const CIMInstance& CI,
                                            const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::microsecondToTimestamp()");
    if(_parms.size() != 1)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();
    if(valType != CQLValue::Uint64_type &&
        valType != CQLValue::Sint64_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(valType),
            "Integer");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_UINT64, false));
    }

    Uint64 uIntVal = 0;
    if (valType == CQLValue::Sint64_type)
    {
        Sint64 intVal = cqlVal.getSint();
        if (intVal < 0)
        {
            MessageLoaderParms mload(
                "CQL.CQLFunctionRep.NEGATIVE_INT_ERROR",
                "Parameter $0 for function $1 has a value of $2."
                    "  It must be non-negative.",
                "1",
                functionTypeToString(),
                intVal);
            throw CQLRuntimeException(mload);
        }
        uIntVal = intVal;
    }
    else
        uIntVal = cqlVal.getUint();

    PEG_METHOD_EXIT();

    return CQLValue(CIMDateTime(uIntVal, false));
}

CQLValue CQLFunctionRep::microsecondToInterval(const CIMInstance& CI,
                                            const QueryContext& queryCtx) const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLFunctionRep::microsecondToInterval()");
    if(_parms.size() != 1)
    {
        MessageLoaderParms mload(
            "CQL.CQLFunctionRep.INVALID_PARM_COUNT",
            "Function $0 has $1 parameters."
                "  It must have between $2 and $3.",
            functionTypeToString(),
            _parms.size(),
            "1", "1");
        throw CQLRuntimeException(mload);
    }

    CQLValue cqlVal = _parms[0].getSimplePredicate().getLeftExpression().
      resolveValue(CI,queryCtx);

    CQLValue::CQLValueType valType = cqlVal.getValueType();
    if(valType != CQLValue::Uint64_type &&
     valType != CQLValue::Sint64_type)
    {
        MessageLoaderParms mload("CQL.CQLFunctionRep.INVALID_PARM_TYPE",
            "Parameter $0 for function $1 has type $2."
                "  It must be type $3.",
            "1",
            functionTypeToString(),
            CQLValueRep::valueTypeToString(valType),
            "Integer");
        throw CQLRuntimeException(mload);
    }

    if (cqlVal.isNull())
    {
        return CQLValue(CIMValue(CIMTYPE_UINT64, false));
    }

    Uint64 uIntVal = 0;

    if (valType == CQLValue::Sint64_type)
    {
        Sint64 intVal = cqlVal.getSint();
        if (intVal < 0)
        {
            MessageLoaderParms mload("CQL.CQLFunctionRep.NEGATIVE_INT_ERROR",
                "Parameter $0 for function $1 has a value of $2."
                    "  It must be non-negative.",
                "1",
                functionTypeToString(),
                intVal);
            throw CQLRuntimeException(mload);
        }
        uIntVal = intVal;
    }
    else
        uIntVal = cqlVal.getUint();

    PEG_METHOD_EXIT();
    return CQLValue(CIMDateTime(uIntVal, true));
}

PEGASUS_NAMESPACE_END
