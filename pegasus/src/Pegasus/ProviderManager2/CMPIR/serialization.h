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

/*!
    \file serialization.h
    \brief Header file for the binary serializer component for Remote CMPI.

    This file defines a function table of methods to be used for serializing
    and deserializing CMPI data respectively. It covers most of the
    encapsulated data types, however, those which cannot be fully serialized
    or deserialized due to API restrictions are not included, e.g. CMPIResult,
    CMPIContext or CMPIEnumeration.
*/

#ifndef _SERIALIZATION_H
#define _SERIALIZATION_H

#include "cmpir_common.h"

#ifndef CMPI_VER_100
# define CMPI_VER_100
#endif

#ifndef CONST
# ifdef CMPI_VER_100
#  define CONST const
# else
#  define CONST
# endif
#endif

#include <Pegasus/Provider/CMPI/cmpidt.h>


struct BinarySerializerFT
{
    ssize_t (* serialize_UINT8) ( int, CMPIUint8 );
    CMPIUint8 (* deserialize_UINT8) ( int );

    ssize_t (* serialize_UINT16) ( int, CMPIUint16 );
    CMPIUint16 (* deserialize_UINT16) ( int );

    ssize_t (* serialize_UINT32) ( int, CMPIUint32 );
    CMPIUint32 (* deserialize_UINT32) ( int );

    ssize_t (* serialize_UINT64) ( int, CMPIUint64 );
    CMPIUint64 (* deserialize_UINT64) ( int );

    ssize_t (* serialize_CMPIValue) (int, CONST CMPIType, CONST CMPIValue *);
    CMPIValue (* deserialize_CMPIValue) ( int, CMPIType, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIType) ( int, CMPIType );
    CMPIType (* deserialize_CMPIType) ( int );

    ssize_t (* serialize_CMPIData) ( int, CMPIData );
    CMPIData (* deserialize_CMPIData) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIStatus) ( int, CMPIStatus * );
    CMPIStatus (* deserialize_CMPIStatus) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_string) ( int, const char * );
    char * (* deserialize_string) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIString) ( int, CONST CMPIString * );
    CMPIString * (* deserialize_CMPIString) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIArgs) ( int, CONST CMPIArgs * );
    CMPIArgs * (* deserialize_CMPIArgs) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIObjectPath) ( int, CONST CMPIObjectPath * );
    CMPIObjectPath * (* deserialize_CMPIObjectPath) (int, CONST CMPIBroker *);

    ssize_t (* serialize_CMPIArray) ( int, CONST CMPIArray * );
    CMPIArray * (* deserialize_CMPIArray) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIInstance) ( int, CONST CMPIInstance * );
    CMPIInstance * (* deserialize_CMPIInstance) ( int, CONST CMPIBroker * ,
        CONST CMPIObjectPath *);

    ssize_t (* serialize_CMPISelectExp) ( int, CONST CMPISelectExp *,
        CMPIUint32);
    CMPISelectExp * (* deserialize_CMPISelectExp) ( int, CONST CMPIBroker *,
        CMPIUint32);

    ssize_t (* serialize_CMPIDateTime) ( int, CMPIDateTime * );
    CMPIDateTime * (* deserialize_CMPIDateTime) ( int, CONST CMPIBroker * );

#ifdef CMPI_VER_200
    ssize_t (* serialize_CMPIError) ( int, CMPIError * );
    CMPIError * (* deserialize_CMPIError) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIErrorSeverity) ( int, CMPIErrorSeverity );
    CMPIErrorSeverity (* deserialize_CMPIErrorSeverity)
        (int, CONST CMPIBroker *);

    ssize_t (* serialize_CMPIErrorProbableCause) (int, CMPIErrorProbableCause);
    CMPIErrorProbableCause (* deserialize_CMPIErrorProbableCause) (
        int,
        CONST CMPIBroker * );

    ssize_t (* serialize_CMPIrc) ( int, CMPIrc );
    CMPIrc (* deserialize_CMPIrc) ( int, CONST CMPIBroker * );

    ssize_t (* serialize_CMPIMsgFileHandle) ( int, CMPIMsgFileHandle );
    CMPIMsgFileHandle (* deserialize_CMPIMsgFileHandle) (
        int,
        CONST CMPIBroker * );
#endif

};

extern const struct BinarySerializerFT binarySerializerFT;

#endif

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
