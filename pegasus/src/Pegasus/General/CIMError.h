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

#ifndef Pegasus_CIMError_h
#define Pegasus_CIMError_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/General/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** CIMError is a specialized class that contains information about the
    severity, cause, recommended actions and other data related to the failure
    of a CIM Operation.  This class is defined in the DMTF Interop schema.
    Instances of this type MAY be included by a CIMServer as part of the
    response to a CIM Operation as part of an ERROR entity.  CIMErrors like
    CIM_Indications do not persist, and therefore do not have keys.
    Currently, this is only allowed by defining a concrete class with the
    Indication qualifier.  Until the CIM Specification is modified to allow
    other concrete classes without keys, the CIMError class MUST be qualified
    by both Indication and Exception.  After this, the Indication qualifier
    can be removed.  This class is implemented as a Pegasus CIMInstance but
    becasue of the special nature of the usage of CIMError this class provides
    specific accessors and modifiersfor each property in the class.

    Because CIMError is implemented as a facade for a CIMInstance it
    uses a shared representation model for the data. More information about
    the effects of using this model are in the CIMInstance documentation.

    NOTE: In the  only a single public constructor is provided that
    requires setting all of the required properties in any constructed
    CIMError class.  This class also provides specific enumerations for
    those properties in the CIM_Error class that are mof enumerations
    (values and valuemaps).
*/

class PEGASUS_GENERAL_LINKAGE CIMError { public:

    /** Enumeration of the possible values in the
        ErrorType Property of the CIM_Error Class
    */
    enum ErrorTypeEnum
    {
        ERROR_TYPE_UNKNOWN = 0,
        ERROR_TYPE_OTHER = 1,
        ERROR_TYPE_COMMUNICATIONS_ERROR = 2,
        ERROR_TYPE_QUALITY_OF_SERVICE_ERROR = 3,
        ERROR_TYPE_SOFTWARE_ERROR = 4,
        ERROR_TYPE_HARDWARE_ERROR = 5,
        ERROR_TYPE_ENVIRONMENTAL_ERROR = 6,
        ERROR_TYPE_SECURITY_ERROR = 7,
        ERROR_TYPE_OVERSUBSCRIPTION_ERROR = 8,
        ERROR_TYPE_UNAVAILABLE_RESOURCE_ERROR = 9,
        ERROR_TYPE_UNSUPPORTED_OPERATION_ERROR = 10
    };

    /** Enumeration of the possiblle values in the
        Perceived Severity Property of the CIM_Error Class
    */
    enum PerceivedSeverityEnum
    {
        PERCEIVED_SEVERITY_UNKNOWN = 0,
        PERCEIVED_SEVERITY_UNUSED_1 = 1,
        PERCEIVED_SEVERITY_LOW = 2,
        PERCEIVED_SEVERITY_MEDIUM = 3,
        PERCEIVED_SEVERITY_HIGH = 4,
        PERCEIVED_SEVERITY_FATAL = 5
    };

    /**  Enumeration of the possible values in the
         ProbableCause Property of hte CIM_Error Class
    */
    enum ProbableCauseEnum
    {
        PROBABLE_CAUSE_UNKNOWN = 0,
        PROBABLE_CAUSE_OTHER = 1,
        PROBABLE_CAUSE_ADAPTER_CARD_ERROR = 2,
        PROBABLE_CAUSE_APPLICATION_SUBSYSTEM_FAILURE = 3,
        PROBABLE_CAUSE_BANDWIDTH_REDUCED = 4,
        PROBABLE_CAUSE_CONNECTION_ESTABLISHMENT_ERROR = 5,
        PROBABLE_CAUSE_COMMUNICATIONS_PROTOCOL_ERROR = 6,
        PROBABLE_CAUSE_COMMUNICATIONS_SUBSYSTEM_FAILURE = 7,
        PROBABLE_CAUSE_CONFIGURATION_CUSTOMIZATION_ERROR = 8,
        PROBABLE_CAUSE_CONGESTION = 9,
        PROBABLE_CAUSE_CORRUPT_DATA = 10,
        PROBABLE_CAUSE_CPU_CYCLES_LIMIT_EXCEEDED = 11,
        PROBABLE_CAUSE_DATASET_MODEM_ERROR = 12,
        PROBABLE_CAUSE_DEGRADED_SIGNAL = 13,
        PROBABLE_CAUSE_DTE_DCE_INTERFACE_ERROR = 14,
        PROBABLE_CAUSE_ENCLOSURE_DOOR_OPEN = 15,
        PROBABLE_CAUSE_EQUIPMENT_MALFUNCTION = 16,
        PROBABLE_CAUSE_EXCESSIVE_VIBRATION = 17,
        PROBABLE_CAUSE_FILE_FORMAT_ERROR = 18,
        PROBABLE_CAUSE_FIRE_DETECTED = 19,
        PROBABLE_CAUSE_FLOOD_DETECTED = 20,
        PROBABLE_CAUSE_FRAMING_ERROR = 21,
        PROBABLE_CAUSE_HVAC_PROBLEM = 22,
        PROBABLE_CAUSE_HUMIDITY_UNACCEPTABLE = 23,
        PROBABLE_CAUSE_I_O_DEVICE_ERROR = 24,
        PROBABLE_CAUSE_INPUT_DEVICE_ERROR = 25,
        PROBABLE_CAUSE_LAN_ERROR = 26,
        PROBABLE_CAUSE_NON_TOXIC_LEAK_DETECTED = 27,
        PROBABLE_CAUSE_LOCAL_NODE_TRANSMISSION_ERROR = 28,
        PROBABLE_CAUSE_LOSS_OF_FRAME = 29,
        PROBABLE_CAUSE_LOSS_OF_SIGNAL = 30,
        PROBABLE_CAUSE_MATERIAL_SUPPLY_EXHAUSTED = 31,
        PROBABLE_CAUSE_MULTIPLEXER_PROBLEM = 32,
        PROBABLE_CAUSE_OUT_OF_MEMORY = 33,
        PROBABLE_CAUSE_OUTPUT_DEVICE_ERROR = 34,
        PROBABLE_CAUSE_PERFORMANCE_DEGRADED = 35,
        PROBABLE_CAUSE_POWER_PROBLEM = 36,
        PROBABLE_CAUSE_PRESSURE_UNACCEPTABLE = 37,
        PROBABLE_CAUSE_PROCESSOR_PROBLEM__INTERNAL_MACHINE_ERROR_ = 38,
        PROBABLE_CAUSE_PUMP_FAILURE = 39,
        PROBABLE_CAUSE_QUEUE_SIZE_EXCEEDED = 40,
        PROBABLE_CAUSE_RECEIVE_FAILURE = 41,
        PROBABLE_CAUSE_RECEIVER_FAILURE = 42,
        PROBABLE_CAUSE_REMOTE_NODE_TRANSMISSION_ERROR = 43,
        PROBABLE_CAUSE_RESOURCE_AT_OR_NEARING_CAPACITY = 44,
        PROBABLE_CAUSE_RESPONSE_TIME_EXCESSIVE = 45,
        PROBABLE_CAUSE_RETRANSMISSION_RATE_EXCESSIVE = 46,
        PROBABLE_CAUSE_SOFTWARE_ERROR = 47,
        PROBABLE_CAUSE_SOFTWARE_PROGRAM_ABNORMALLY_TERMINATED = 48,
        PROBABLE_CAUSE_SOFTWARE_PROGRAM_ERROR__INCORRECT_RESULTS_ = 49,
        PROBABLE_CAUSE_STORAGE_CAPACITY_PROBLEM = 50,
        PROBABLE_CAUSE_TEMPERATURE_UNACCEPTABLE = 51,
        PROBABLE_CAUSE_THRESHOLD_CROSSED = 52,
        PROBABLE_CAUSE_TIMING_PROBLEM = 53,
        PROBABLE_CAUSE_TOXIC_LEAK_DETECTED = 54,
        PROBABLE_CAUSE_TRANSMIT_FAILURE = 55,
        PROBABLE_CAUSE_TRANSMITTER_FAILURE = 56,
        PROBABLE_CAUSE_UNDERLYING_RESOURCE_UNAVAILABLE = 57,
        PROBABLE_CAUSE_VERSION_MISMATCH = 58,
        PROBABLE_CAUSE_PREVIOUS_ALERT_CLEARED = 59,
        PROBABLE_CAUSE_LOGIN_ATTEMPTS_FAILED = 60,
        PROBABLE_CAUSE_SOFTWARE_VIRUS_DETECTED = 61,
        PROBABLE_CAUSE_HARDWARE_SECURITY_BREACHED = 62,
        PROBABLE_CAUSE_DENIAL_OF_SERVICE_DETECTED = 63,
        PROBABLE_CAUSE_SECURITY_CREDENTIAL_MISMATCH = 64,
        PROBABLE_CAUSE_UNAUTHORIZED_ACCESS = 65,
        PROBABLE_CAUSE_ALARM_RECEIVED = 66,
        PROBABLE_CAUSE_LOSS_OF_POINTER = 67,
        PROBABLE_CAUSE_PAYLOAD_MISMATCH = 68,
        PROBABLE_CAUSE_TRANSMISSION_ERROR = 69,
        PROBABLE_CAUSE_EXCESSIVE_ERROR_RATE = 70,
        PROBABLE_CAUSE_TRACE_PROBLEM = 71,
        PROBABLE_CAUSE_ELEMENT_UNAVAILABLE = 72,
        PROBABLE_CAUSE_ELEMENT_MISSING = 73,
        PROBABLE_CAUSE_LOSS_OF_MULTI_FRAME = 74,
        PROBABLE_CAUSE_BROADCAST_CHANNEL_FAILURE = 75,
        PROBABLE_CAUSE_INVALID_MESSAGE_RECEIVED = 76,
        PROBABLE_CAUSE_ROUTING_FAILURE = 77,
        PROBABLE_CAUSE_BACKPLANE_FAILURE = 78,
        PROBABLE_CAUSE_IDENTIFIER_DUPLICATION = 79,
        PROBABLE_CAUSE_PROTECTION_PATH_FAILURE = 80,
        PROBABLE_CAUSE_SYNC_LOSS_OR_MISMATCH = 81,
        PROBABLE_CAUSE_TERMINAL_PROBLEM = 82,
        PROBABLE_CAUSE_REAL_TIME_CLOCK_FAILURE = 83,
        PROBABLE_CAUSE_ANTENNA_FAILURE = 84,
        PROBABLE_CAUSE_BATTERY_CHARGING_FAILURE = 85,
        PROBABLE_CAUSE_DISK_FAILURE = 86,
        PROBABLE_CAUSE_FREQUENCY_HOPPING_FAILURE = 87,
        PROBABLE_CAUSE_LOSS_OF_REDUNDANCY = 88,
        PROBABLE_CAUSE_POWER_SUPPLY_FAILURE = 89,
        PROBABLE_CAUSE_SIGNAL_QUALITY_PROBLEM = 90,
        PROBABLE_CAUSE_BATTERY_DISCHARGING = 91,
        PROBABLE_CAUSE_BATTERY_FAILURE = 92,
        PROBABLE_CAUSE_COMMERCIAL_POWER_PROBLEM = 93,
        PROBABLE_CAUSE_FAN_FAILURE = 94,
        PROBABLE_CAUSE_ENGINE_FAILURE = 95,
        PROBABLE_CAUSE_SENSOR_FAILURE = 96,
        PROBABLE_CAUSE_FUSE_FAILURE = 97,
        PROBABLE_CAUSE_GENERATOR_FAILURE = 98,
        PROBABLE_CAUSE_LOW_BATTERY = 99,
        PROBABLE_CAUSE_LOW_FUEL = 100,
        PROBABLE_CAUSE_LOW_WATER = 101,
        PROBABLE_CAUSE_EXPLOSIVE_GAS = 102,
        PROBABLE_CAUSE_HIGH_WINDS = 103,
        PROBABLE_CAUSE_ICE_BUILDUP = 104,
        PROBABLE_CAUSE_SMOKE = 105,
        PROBABLE_CAUSE_MEMORY_MISMATCH = 106,
        PROBABLE_CAUSE_OUT_OF_CPU_CYCLES = 107,
        PROBABLE_CAUSE_SOFTWARE_ENVIRONMENT_PROBLEM = 108,
        PROBABLE_CAUSE_SOFTWARE_DOWNLOAD_FAILURE = 109,
        PROBABLE_CAUSE_ELEMENT_REINITIALIZED = 110,
        PROBABLE_CAUSE_TIMEOUT = 111,
        PROBABLE_CAUSE_LOGGING_PROBLEMS = 112,
        PROBABLE_CAUSE_LEAK_DETECTED = 113,
        PROBABLE_CAUSE_PROTECTION_MECHANISM_FAILURE = 114,
        PROBABLE_CAUSE_PROTECTING_RESOURCE_FAILURE = 115,
        PROBABLE_CAUSE_DATABASE_INCONSISTENCY = 116,
        PROBABLE_CAUSE_AUTHENTICATION_FAILURE = 117,
        PROBABLE_CAUSE_BREACH_OF_CONFIDENTIALITY = 118,
        PROBABLE_CAUSE_CABLE_TAMPER = 119,
        PROBABLE_CAUSE_DELAYED_INFORMATION = 120,
        PROBABLE_CAUSE_DUPLICATE_INFORMATION = 121,
        PROBABLE_CAUSE_INFORMATION_MISSING = 122,
        PROBABLE_CAUSE_INFORMATION_MODIFICATION = 123,
        PROBABLE_CAUSE_INFORMATION_OUT_OF_SEQUENCE = 124,
        PROBABLE_CAUSE_KEY_EXPIRED = 125,
        PROBABLE_CAUSE_NON_REPUDIATION_FAILURE = 126,
        PROBABLE_CAUSE_OUT_OF_HOURS_ACTIVITY = 127,
        PROBABLE_CAUSE_OUT_OF_SERVICE = 128,
        PROBABLE_CAUSE_PROCEDURAL_ERROR = 129,
        PROBABLE_CAUSE_UNEXPECTED_INFORMATION = 130
    };

    /** Enumeration of the possible values for the
        ErrorSourceFormat Property of the CIM_Error Class
    */
    enum ErrorSourceFormatEnum
    {
        ERROR_SOURCE_FORMAT_UNKNOWN = 0,
        ERROR_SOURCE_FORMAT_OTHER = 1,
        ERROR_SOURCE_FORMAT_CIM_OBJECT_HANDLE = 2
    };

    /** Enumeration of possible values for the
        CIMStatus Code Property of the CIM_Error Class
    */
    enum CIMStatusCodeEnum
    {
        CIM_STATUS_CODE_CIM_ERR_FAILED = 1,
        CIM_STATUS_CODE_CIM_ERR_ACCESS_DENIED = 2,
        CIM_STATUS_CODE_CIM_ERR_INVALID_NAMESPACE = 3,
        CIM_STATUS_CODE_CIM_ERR_INVALID_PARAMETER = 4,
        CIM_STATUS_CODE_CIM_ERR_INVALID_CLASS = 5,
        CIM_STATUS_CODE_CIM_ERR_NOT_FOUND = 6,
        CIM_STATUS_CODE_CIM_ERR_NOT_SUPPORTED = 7,
        CIM_STATUS_CODE_CIM_ERR_CLASS_HAS_CHILDREN = 8,
        CIM_STATUS_CODE_CIM_ERR_CLASS_HAS_INSTANCES = 9,
        CIM_STATUS_CODE_CIM_ERR_INVALID_SUPERCLASS = 10,
        CIM_STATUS_CODE_CIM_ERR_ALREADY_EXISTS = 11,
        CIM_STATUS_CODE_CIM_ERR_NO_SUCH_PROPERTY = 12,
        CIM_STATUS_CODE_CIM_ERR_TYPE_MISMATCH = 13,
        CIM_STATUS_CODE_CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED = 14,
        CIM_STATUS_CODE_CIM_ERR_INVALID_QUERY = 15,
        CIM_STATUS_CODE_CIM_ERR_METHOD_NOT_AVAILABLE = 16,
        CIM_STATUS_CODE_CIM_ERR_METHOD_NOT_FOUND = 17,
        CIM_STATUS_CODE_CIM_ERR_UNEXPECTED_RESPONSE = 18,
        CIM_STATUS_CODE_CIM_ERR_INVALID_RESPONSE_DESTINATION = 19,
        CIM_STATUS_CODE_CIM_ERR_NAMESPACE_NOT_EMPTY = 20
    };

    /** Default constructor creates a single CIMInstance representing the
        CIM_Error.  It builds in all of the properties in a CIM_Error and
        sets them to NULL.
    */
    CIMError();

    /** Constructor for minimum legal CIM_Error instance.  Requires input of
        all of the properties defined as required properties for CIM_Error.
        @param messageID String
        @param message String
        @param perceivedSeverity PerceivedSeverityEnum
        @param probableCause ProbableCauseEnum
        @param cimStatusCode CIMStatusCodeEnum
        @exceptions TBD
        <p><b>example</b>
        CIMError err1(PegasusOwningEntityName,
                  "MessageIDString",
                  "Text Of Message",
                  CIMError::PERCEIVED_SEVERITY_LOW,
                  CIMError::PROBABLE_CAUSE_UNEXPECTED_INFORMATION,
                  CIMError::CIM_STATUS_CODE_CIM_ERR_FAILED);
    */
    CIMError(const String& owningEntity,
             const String& messageID,
             const String& message,
             const PerceivedSeverityEnum& perceivedSeverity,
             const ProbableCauseEnum& probableCause,
             const CIMStatusCodeEnum& cimStatusCode);

    /** Copy constructor.
    */
    CIMError(const CIMError& x);

    /** Destructor.
    */
    ~CIMError();

    /** Accessor for the ErrorType property value in a CIM_Error
        Instance.
        @param value ErrorTypeEnum that is the current value of
        the Property in the CIM_Error Instance.
        @return bool returns true if the property is NOT Null.
        returns false the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
        */
    bool getErrorType(ErrorTypeEnum& value) const;

    /** Modify the ErrorType property value for the CIM_Error
        instance.
        @ param value ErrorTypeEnum representing the value to be
        set in the property.  if the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to false. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
        @exception Generates exceptions if missing
    */
    void setErrorType(ErrorTypeEnum value, bool null = false);

    /** Accessor for the OwningEntity property value in a CIM_Error
        Instance.
        @param value String that is the current value of
        the ErrorType Property in the CIM_Error Instance.
        @return bool returns true if the property is NOT Null.
        returns false the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getOwningEntity(String& value) const;

    /** Modify the OwningEntity property value for the CIM_Error
        instance.
        @ param value String representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
        @exception Generates exceptions if TBD
    */
    void setOwningEntity(const String& value, bool null = false);

    /** Accessor for the OtherErrorType property value in a CIM_Error
        Instance.
        @param value String that is the current value of
        the OtherErrorType Property in the CIM_Error Instance.
        @return bool returns true if the property is NOT Null.
        returns false the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getOtherErrorType(String& value) const;

    /** Modify the OtherErrorType property value for the CIM_Error
        instance.
        @param value String representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null bool Sets the property to null if this parameter
        is true, overriding any property value setting.
        @exception Generates exceptions if TBD
    */
    void setOtherErrorType(const String& value, bool null = false);

    /** Accessor for the MessageID property value in a CIM_Error
        Instance.
        @param value String that is the current value of
        the MessageID Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getMessageID(String& value) const;

    /** Modify the MessageID property value for the CIM_Error
        instance.
        @ param value String representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setMessageID(const String& value, bool null = false);

    /** Accessor for the Message property value in a CIM_Error
        Instance.
        @param value String that is the current value of
        the MessageID Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getMessage(String& value) const;

    /** Modify the Message property value for the CIM_Error
        instance.
        @ param value String representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setMessage(const String& value, bool null = false);

    /** Accessor for the MessageArguments property value in a CIM_Error
        Instance.
        @param value Array<String> that is the current value of
        the MessageArguments Array Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getMessageArguments(Array<String>&  value) const;

    /** Modify the MessageArguments property value for the CIM_Error
        instance.
        @ param value Array<String> representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setMessageArguments(const Array<String>& value, bool null = false);

    /** Accessor for the PerceivedSeverity property value in a CIM_Error
        Instance.
        @param value PerceivedSeverityEnum that is the current value of
        the MessageArguments Array Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getPerceivedSeverity(PerceivedSeverityEnum& value) const;

    /** Modify the PerceivedSeverity property value for the CIM_Error
        instance.
        @ param value PerceivedSeverityEnum representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setPerceivedSeverity(PerceivedSeverityEnum value, bool null = false);

    /** Accessor for the ProbableCause property value in a CIM_Error
        Instance.
        @param value ProbableCauseEnum that is the current value of
        the MessageArguments Array Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getProbableCause(ProbableCauseEnum& value) const;

    /** Modify the ProbableCause property value for the CIM_Error
        instance.
        @ param value ProbableCauseEnum representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setProbableCause(ProbableCauseEnum value, bool null = false);

    /** Accessor for the ProbableCauseDescription property value in a
        CIM_Error Instance.
        @param value ProbableCauseEnumDescription that is the current value of
        the MessageArguments Array Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getProbableCauseDescription(String& value) const;

    /** Modify the ProbableCauseDescription property value for the CIM_Error
        instance.
        @ param value ProbableCauseEnumDescription representing the value to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setProbableCauseDescription(const String& value, bool null = false);

    /** Accessor for the RecommendedActions property value in a
        CIM_Error Instance.
        @param value Array<String> that is the current value of
        the RecommendedActions Array Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getRecommendedActions(Array<String>& value) const;

    /** Modify the RecommendedActions property value for the CIM_Error
      String  instance.
        @ param value Array<String> representing the values to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setRecommendedActions(const Array<String>& value, bool null = false);

    /** Accessor for the ErrorSource property value in a
        CIM_Error Instance.
        @param value String that is the current value of
        the RecommendedActions Array Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getErrorSource(String& value) const;

    /** Modify the ErrorSource property value for the CIM_Error
        instance.
        @ param value String representing the values to be
        set in the property.  If the null property equals true,
        this value is ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setErrorSource(const String& value, bool null = false);

    /** Accessor for the ErrorSourceFormat property value in a
        CIM_Error Instance.
        @param value ErrorSourceFormatEnum that is the current value of
        the RecommendedActions Array Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getErrorSourceFormat(ErrorSourceFormatEnum& value) const;

    /** Modify the ErrorSourceFormat property value for the CIM_Error
        instance.
        @ param value String representing the values to be
        set in the property.  If the null property is true,
        this value should be ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setErrorSourceFormat(ErrorSourceFormatEnum value, bool null = false);

    /** Accessor for the OtherErrorSourceFormat property value in a
        CIM_Error Instance.
        @param value String that is the current value of
        the OtherErrorSourceFormat Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getOtherErrorSourceFormat(String& value) const;

    /** Modify the OtherErrorSourceFormat property value for the
        CIM_Error instance.
        @ param value String representing the values to be
        set in the property.  If the null property is true,
        this value should be ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setOtherErrorSourceFormat(const String& value, bool null = false);

    /** Accessor for the CIMStatusCode property value in a
        CIM_Error Instance.
        @param value CIMStatusCodeEnum that is the current value of
        the CIMStatusCode Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getCIMStatusCode(CIMStatusCodeEnum& value) const;

    /** Modify the CIMStatusCode property value for the
        CIM_Error instance.
        @ param value String representing the values to be
        set in the property.  If the null property is true,
        this value should be ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setCIMStatusCode(CIMStatusCodeEnum value, bool null = false);

    /** Accessor for the CIMStatusCode property value in a
        CIM_Error Instance.
        @param value String that is the current value of
        the CIMStatusCode Property in the CIM_Error Instance.
        @return bool returns true if the property has a legitimate value
        (is NOT Null).
        It returns false if the value of the property is Null.
        This value should be tested before accessing the returned
        value because the returned value will be invalid if
        the variable is null.
        @exception Generates CIM_ERR_TYPE_MISMATCH exception
        if property missing
     */
    bool getCIMStatusCodeDescription(String& value) const;

    /** Modify the CIMStatusCodeDescription property value for the
        CIM_Error instance.
        @ param value String representing the values to be
        set in the property.  If the null property is true,
        this value should be ignored.
        @param null boolean that is defaulted to zero. If set
        true, this causes the property to be set to NULL, overriding
        any value in the property.
    */
    void setCIMStatusCodeDescription(const String& value, bool null = false);

    /** Get reference to internal CIMInstance.  This function tests the
        properties for validy and to assure that all properties that are
        required in the mof are in the CIMInstance
        @return CIMInstance with the target CIM_Error
        <p><b>example</b>
            CIM_Error err1;
            CIM_Error err2;
            err1.setProbableCause(PROBABLE_CAUSE_ADAPTER_CARD_ERROR);
            err2.setInstance(err1.getInstance());
    */

    const CIMInstance& getInstance() const;

    /** Set the internal instance (and throw an exception if instance
        argument has an incompatible property types). This function tests the
        properties for validy and to assure that all properties that are
        required in the mof are in the CIMInstance.
        @param instance CIMInstance which is the CIM_Error

        @exception  CIMExceptions TypeMismatchException if there is a
        property type mismatch and NoSuchPropertyException
        if a required property is missing.
        <p><b>example</b>
            CIM_Error err1;
            CIMInstance instance1 err1.getInstance();
    */
    void setInstance(const CIMInstance& instance);

    /** Print this object to standard output. This is a diagnostic outut
        and the exact format of the output cannot be depended on.  It prints
        the name and value of each property in the CIM_Error instance.
    */
    void print() const;

private:

    CIMInstance _inst;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMError_h */
