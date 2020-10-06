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

#ifndef Audit_zOS_SMFStructures_h
#define Audit_zOS_SMFStructures_h

#pragma pack(packed)

typedef struct {
    unsigned short    SMF86LEN;      // Record length
    unsigned short    SMF86SEG;      // Segmentation desc.
    unsigned char     SMF86FLG;      // System indicator
    unsigned char     SMF86RTY;      // Record type 86 (X'56')
    unsigned int      SMF86TME;      // ms Time record moved to SMF
    unsigned char     SMF86DTE[4];   // Packed BCD date (0cyyddF) record
                                     // moved to SMF
    unsigned char     SMF86SID[4];   // System ID in EBCDIC
    unsigned char     SMF86SSI[4];   // SubsystemID 'CFZ' in EBCDIC
    unsigned short    SMF86STY;      // Record subtype:
                                     // Subtype Description
                                     //    1    Authentication
                                     //    2    Configuration
                                     //    3    Provider Status
                                     //    4    CIM Operations
                                     //    5    Indicaton Operations
    unsigned short    SMF86TRN;      // Number of tripltes
    unsigned short    reserverd;     // Reserved
    unsigned int      SMF86PRO;      // Offset to product section
    unsigned short    SMF86PRL;      // Length of product section
    unsigned short    SMF86PRN;      // Number of product sections (=1)
    unsigned int      SMF86STO;      // Offset to subtype section
    unsigned short    SMF86STL;      // Length of subtype section
    unsigned short    SMF86STN;      // Number of subtype sections (=1)
} _smf86_header;

typedef struct {
    unsigned int      SMF86PRRVN;    // SMF Record version
    unsigned char     SMF86SSI[4];   // SubSytem ID 'CFZ' in EBCDIC
    unsigned char     SMF86VRM[8];   // CIM software level 'VV.RR.MM' EBCDIC
    unsigned char     SMF86OSL[8];   // MVS software level 'VV.RR.MM' EBCDIC
    unsigned char     SMF86SYN[8];   // System name in EBCDIC
    unsigned char     SMF86SYP[8];   // Sysplex name in EBCDIC
    unsigned int      SMF86OPI;      // USS process ID
    unsigned char     SMF86THID[22]; // Thread Id in EBCDIC
    unsigned char     reserved[2];   // reserved
} _smf86_product;

typedef struct {
    unsigned short    AuthMode;      // Authentication mode:
                                     // AuthMode Description
                                     //    0     Local
                                     //    1     Basic
                                     //    2     AT-TLS
    unsigned char     UserID[8];     // User Id
    unsigned short    AuthResult;    // Authentication result:
                                     // AuthResult Description
                                     //     0      Successful
                                     //     1      Failed
    unsigned char     ClientIP[42];  // Clinet IP adresse X'00' terminated
    unsigned char     reserved[2];   // reserved
} _smf86_authentication;

typedef struct {
    unsigned char     UserID[8];     // User Id
    unsigned short    PropChange;    // Mode of property change:
                                     // PropChange Comment
                                     //     0      List Conf.
                                     //     1      Current Conf.
                                     //     2      Planed Conf.
    unsigned char     reserved[2];   // reserved
    unsigned int      NameOf;        // Offset to property name
                                     // from section start
    unsigned short    NameLen;       // Length of property name
    unsigned short    NameNo;        // Number of property names (=1)
    unsigned int      ValueOf;       // Offset to property value
                                     // from section start
    unsigned short    ValueLen;      // Length of property value
    unsigned short    ValueNo;       // Number of property values (=1)
    unsigned int      NewValueOf;    // Offset to property new value
                                     // from section start
    unsigned short    NewValueLen;   // Length of property new value
    unsigned short    NewValueNo;    // Number of property new values (=1)
} _smf86_configuration;

typedef struct {
    unsigned int      CurrStatus;    // The current status of prov.
    unsigned short    IsChanging;    // If set to 1, then the provider is
                                     // changing his state and NewStatus
                                     // value is valid.
    unsigned char     reserved[2];   // reserved
    unsigned int      NewStatus;     // The new status of prov.
    unsigned int      ProvNameOf;    // Offset to provider name
    unsigned short    ProvNameLen;   // Length of provider name
    unsigned short    ProvNameNo;    // Number of provider name (=1)
} _smf86_provider_status;

typedef struct {
    unsigned short    CIMOpType;     // The CIM operation type:
                                     // CIMOpTyp Comment
                                     //    0     Class oper.
                                     //    1     Qualifyer oper.
                                     //    2     Instance oper.
                                     //    3     Invoke methode
    unsigned char     UserID[8];     // User Id
    unsigned short    CIMStatusCode; // The result status of the oper.
    unsigned char     ClientIP[42];  // Clinet IP adresse X'00' terminated
    unsigned char     reserved[2];   // reserved
    unsigned int      OperNameOf;    // Offset to oper. name
    unsigned short    OperNameLen;   // Length of oper. name
    unsigned short    OperNameNo;    // Number of oper. name
    unsigned int      ObjPathOf;     // Offset to object path
    unsigned short    ObjPathLen;    // Length of object path
    unsigned short    ObjPathNo;     // Number of object path
    unsigned int      NameSpaceOf;   // Offset to name space
    unsigned short    NameSpaceLen;  // Length of name space
    unsigned short    NameSpaceNo;   // Number of name space
    unsigned int      ProvNameOf;    // Offset to prov. name
    unsigned short    ProvNameLen;   // Length of prov. name
    unsigned short    ProvNameNo;    // Number of prov. name
    unsigned int      ProvModNameOf; // Offset to prov. module name
    unsigned short    ProvModNameLen;// Length of prov. module name
    unsigned short    ProvModNameNo; // Number of prov. module name
} _smf86_cim_operation;

typedef struct {
    _smf86_header           header;
    _smf86_product          product;
    _smf86_authentication   authentication;
} _smf86_auththentication_record;

typedef struct {
    _smf86_header           header;
    _smf86_product          product;
    _smf86_configuration    configuration;
} _smf86_configuration_record;

typedef struct {
    _smf86_header           header;
    _smf86_product          product;
    _smf86_provider_status  provider_status;
} _smf86_provider_status_record;

typedef struct {
    _smf86_header           header;
    _smf86_product          product;
    _smf86_cim_operation    cim_operation;
} _smf86_cim_operation_record;

typedef struct {
    _smf86_header           header;
    _smf86_product          product;
} _smf86_record_prolog;


#pragma pack(reset)

#endif // Audit_zOS_SMFStructures_h
