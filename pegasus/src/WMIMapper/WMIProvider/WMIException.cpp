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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:
//
//%////////////////////////////////////////////////////////////////////////////

#include "WMIException.h"

PEGASUS_NAMESPACE_BEGIN

WMIException::WMIException(const CIMException& exception) : Exception("")
{
}

WMIException::WMIException(const HRESULT hRes) : Exception("")
{
    switch(hRes) {
    case WBEM_NO_ERROR:
    /*
    case WBEM_S_NO_ERROR:
    case WBEM_S_SAME:
    case WBEM_S_FALSE:
    case WBEM_S_ALREADY_EXISTS:
    case WBEM_S_RESET_TO_DEFAULT:
    case WBEM_S_DIFFERENT:
    case WBEM_S_TIMEDOUT:
    case WBEM_S_NO_MORE_DATA:
    case WBEM_S_OPERATION_CANCELLED:
    case WBEM_S_PENDING:
    case WBEM_S_DUPLICATE_OBJECTS:
    case WBEM_S_ACCESS_DENIED:
    case WBEM_S_PARTIAL_RESULTS:
    */
    case WBEM_E_FAILED:
    case WBEM_E_NOT_FOUND:
    case WBEM_E_ACCESS_DENIED:
    case WBEM_E_PROVIDER_FAILURE:
    case WBEM_E_TYPE_MISMATCH:
    case WBEM_E_OUT_OF_MEMORY:
    case WBEM_E_INVALID_CONTEXT:
    case WBEM_E_INVALID_PARAMETER:
    case WBEM_E_NOT_AVAILABLE:
    case WBEM_E_CRITICAL_ERROR:
    case WBEM_E_INVALID_STREAM:
    case WBEM_E_NOT_SUPPORTED:
    case WBEM_E_INVALID_SUPERCLASS:
    case WBEM_E_INVALID_NAMESPACE:
    case WBEM_E_INVALID_OBJECT:
    case WBEM_E_INVALID_CLASS:
    case WBEM_E_PROVIDER_NOT_FOUND:
    case WBEM_E_INVALID_PROVIDER_REGISTRATION:
    case WBEM_E_PROVIDER_LOAD_FAILURE:
    case WBEM_E_INITIALIZATION_FAILURE:
    case WBEM_E_TRANSPORT_FAILURE:
    case WBEM_E_INVALID_OPERATION:
    case WBEM_E_INVALID_QUERY:
    case WBEM_E_INVALID_QUERY_TYPE:
    case WBEM_E_ALREADY_EXISTS:
    case WBEM_E_OVERRIDE_NOT_ALLOWED:
    case WBEM_E_PROPAGATED_QUALIFIER:
    case WBEM_E_PROPAGATED_PROPERTY:
    case WBEM_E_UNEXPECTED:
    case WBEM_E_ILLEGAL_OPERATION:
    case WBEM_E_CANNOT_BE_KEY:
    case WBEM_E_INCOMPLETE_CLASS:
    case WBEM_E_INVALID_SYNTAX:
    case WBEM_E_NONDECORATED_OBJECT:
    case WBEM_E_READ_ONLY:
    case WBEM_E_PROVIDER_NOT_CAPABLE:
    case WBEM_E_CLASS_HAS_CHILDREN:
    case WBEM_E_CLASS_HAS_INSTANCES:
    case WBEM_E_QUERY_NOT_IMPLEMENTED:
    case WBEM_E_ILLEGAL_NULL:
    case WBEM_E_INVALID_QUALIFIER_TYPE:
    case WBEM_E_INVALID_PROPERTY_TYPE:
    case WBEM_E_VALUE_OUT_OF_RANGE:
    case WBEM_E_CANNOT_BE_SINGLETON:
    case WBEM_E_INVALID_CIM_TYPE:
    case WBEM_E_INVALID_METHOD:
    case WBEM_E_INVALID_METHOD_PARAMETERS:
    case WBEM_E_SYSTEM_PROPERTY:
    case WBEM_E_INVALID_PROPERTY:
    case WBEM_E_CALL_CANCELLED:
    case WBEM_E_SHUTTING_DOWN:
    case WBEM_E_PROPAGATED_METHOD:
    case WBEM_E_UNSUPPORTED_PARAMETER:
    case WBEM_E_MISSING_PARAMETER_ID:
    case WBEM_E_INVALID_PARAMETER_ID:
    case WBEM_E_NONCONSECUTIVE_PARAMETER_IDS:
    case WBEM_E_PARAMETER_ID_ON_RETVAL:
    case WBEM_E_INVALID_OBJECT_PATH:
    case WBEM_E_OUT_OF_DISK_SPACE:
    case WBEM_E_BUFFER_TOO_SMALL:
    case WBEM_E_UNSUPPORTED_PUT_EXTENSION:
    case WBEM_E_UNKNOWN_OBJECT_TYPE:
    case WBEM_E_UNKNOWN_PACKET_TYPE:
    case WBEM_E_MARSHAL_VERSION_MISMATCH:
    case WBEM_E_MARSHAL_INVALID_SIGNATURE:
    case WBEM_E_INVALID_QUALIFIER:
    case WBEM_E_INVALID_DUPLICATE_PARAMETER:
    case WBEM_E_TOO_MUCH_DATA:
    case WBEM_E_SERVER_TOO_BUSY:
    case WBEM_E_INVALID_FLAVOR:
    case WBEM_E_CIRCULAR_REFERENCE:
    case WBEM_E_UNSUPPORTED_CLASS_UPDATE:
    case WBEM_E_CANNOT_CHANGE_KEY_INHERITANCE:
    case WBEM_E_CANNOT_CHANGE_INDEX_INHERITANCE:
    case WBEM_E_TOO_MANY_PROPERTIES:
    case WBEM_E_UPDATE_TYPE_MISMATCH:
    case WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED:
    case WBEM_E_UPDATE_PROPAGATED_METHOD:
    case WBEM_E_METHOD_NOT_IMPLEMENTED:
    case WBEM_E_METHOD_DISABLED:
    case WBEM_E_REFRESHER_BUSY:
    case WBEM_E_UNPARSABLE_QUERY:
    case WBEM_E_NOT_EVENT_CLASS:
    case WBEM_E_MISSING_GROUP_WITHIN:
    case WBEM_E_MISSING_AGGREGATION_LIST:
    case WBEM_E_PROPERTY_NOT_AN_OBJECT:
    case WBEM_E_AGGREGATING_BY_OBJECT:
    case WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY:
    case WBEM_E_BACKUP_RESTORE_WINMGMT_RUNNING:
    case WBEM_E_QUEUE_OVERFLOW:
    case WBEM_E_PRIVILEGE_NOT_HELD:
    case WBEM_E_INVALID_OPERATOR:
    case WBEM_E_LOCAL_CREDENTIALS:
    case WBEM_E_CANNOT_BE_ABSTRACT:
    case WBEM_E_AMENDED_OBJECT:
    case WBEM_E_CLIENT_TOO_SLOW:
    case WBEMESS_E_REGISTRATION_TOO_BROAD:
    case WBEMESS_E_REGISTRATION_TOO_PRECISE:
        break;
    }
}

PEGASUS_NAMESPACE_END
