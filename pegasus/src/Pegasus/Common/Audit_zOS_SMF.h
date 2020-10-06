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

#ifndef Audit_zOS_SMF_h
#define Audit_zOS_SMF_h


#include <Pegasus/Common/Audit_zOS_SMFStructures.h>
#include <Pegasus/Common/Mutex.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

enum _smf_record_suptype {
    AUTHENTICATION=1,
    CONFIGURATION=2,
    PROVIDER_STATUS=3,
    CIM_OPERATION=4,
    INDICATION_OPERATION=5
} ;

class PEGASUS_COMMON_LINKAGE Audit_zOS_SMF
{

public:

    /**
     * Default constructor
     */
    Audit_zOS_SMF();

    /**
     * Default destructor
     */
    ~Audit_zOS_SMF();

    /**
     * Initialize the SMF record header and product section with values
     * constant for all SMF records. It also set the SMF subtype and
     * the total size of the subtype section, including the size of the
     * variable properties, into the header.
     *
     *  @param myRecProlog - Pointer to the start of the record.
     *
     *  @param subtype - The SMF subtype value for the record.
     *
     *  @param subTypeSize - The size of the subtype section including
     *                       the size of variable properties.
     */
    void initMyProlog( _smf86_record_prolog * myRecProlog,
                                    _smf_record_suptype subtype,
                                    int subTypeSize);
    /**
     * Check with SMF if this subtype is recorded by SMF.
     *
     * @param subtype - The SMF subtype.
     *
     */
    Boolean isRecording(_smf_record_suptype subtype);

    /**
     * Writes a record to SMF
     *
     * @param subtype - The SMF subtype.
     *
     * @param record - Pointer to the record.
     *
     */
    static void  writeRecord(int subtype, char * record);

    /**
     * Dumps a record as HEXDump to stdout.
     *
     * @param subtype - The SMF subtype.
     *
     * @param record - Pointer to the record.
     *
     */
    static void  printRecord(int subtype, char * record);

    /**
     *  Set a SMF record field in EBCDIC. If the value is smaller then the
     *  filed, then the remaining field is filled with spaces.
     *
     * @param field - Pointer to the start of the field.
     *
     * @param value - The value for the field 0x00 terminated.
     *
     * @param size - The size of the filed.
     *
     * @param nullTerminated - If true, then the value is copied including
     *                         the 0x00 termination.
     *
     */
    void    setEBCDICRecordField( unsigned char * field, const char * value,
                int size, bool nullTerminated );

private:

    /**
     * Pointer to a SMF record prolog ( header + product section )
     * with initialized with constant values for all record subtypes.
     */
    _smf86_record_prolog * _recordProlog;
    Mutex _protectProlog;

    /**
     * Initialize a SMF header with constant values.
     *
     * @param smfRecord - The header to initialize.
     *
     */
    void _preInitRecordHeaderSection( _smf86_header* smfRecord );

    /**
     * Initialize a SMF product section with constant values.
     *
     * @param smfRecord - The product section to initialize.
     *
     */
    void _preInitRecordProductSection( _smf86_product* smfRecord );

    /**
     * Set the Sysplex name.
     *
     * @param field - The field for the sysplex name.
     *
     */
    void _setSysplexName(unsigned char * field);

};

PEGASUS_NAMESPACE_END

#endif // Audit_zOS_SMF_h

