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
    \file native.h
    \brief Header file for the native encapsulated CMPI data type
    implementation.

    This file defines all the data types and functions necessary to use
    native encapsulated CMPI data objects. These are clones of the regular
    CMPI data types like CMPIObjectPath, CMPIInstance etc., however, they
    can be instantiated and manipulated without a full blown CIMOM. Instead,
    they use an autononmous CIMOM clone that provides all the functions
    to create these objects as defined by the CMPIBrokerEncFT.

*/

#ifndef _REMOTE_CMPI_NATIVE_DATA_H
#define _REMOTE_CMPI_NATIVE_DATA_H

#define NATIVE_FT_VERSION 1

#ifndef CMPI_VERSION
# define CMPI_VERSION 200
#endif

#include "cmpir_common.h"
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

//! Utility functions, used to validate input parameters.
CMPIStatus checkArgsReturnStatus(const void *ptr);
CMPIData checkArgsReturnData(const void *ptr, CMPIStatus *rc);
void* checkArgs(const void *ptr, CMPIStatus *rc);


//! Forward declaration for anonymous struct.
struct native_property;


//! Function table for native_property handling functions.
/*!
    This structure holds all the function pointers necessary to handle
    linked lists of native_property structs.

    \sa propertyFT in native.h
*/
#ifdef CMPI_VER_100
# define CONST const

struct native_propertyFT
{

    //! Adds a new native_property to a list.
    int (* addProperty) (
        struct native_property **,
        int,
        const char *,
        const CMPIType,
        CMPIValueState,
        const CMPIValue * );

    //! Resets the values of an existing native_property, if existant.
    int (* setProperty) (
        struct native_property *,
        int,
        const char *,
        CMPIType,
        const CMPIValue * );

     //! Sets PropertyOrigin,this internal function is called after setProperty.
     int (* setPropertyOrigin) (
         struct native_property *,
         const char *,
         const char *,
         int );

    //! Looks up a specifix native_property in CMPIData format.
    CMPIData (* getDataProperty) (
        struct native_property *,
        const char *,
        CMPIStatus * );

    //! Extract an indexed native_property in CMPIData format.
    CMPIData (* getDataPropertyAt) (
        struct native_property *,
        unsigned int,
        CMPIString **,
        CMPIStatus * );

    //! Yields the number of native_property items in a list.
    CMPICount (* getPropertyCount) (
        struct native_property *,
        CMPIStatus * );

    //! Releases a complete list of native_property items.
    void (* release) ( struct native_property * );

    //! Clones a complete list of native_property items.
    struct native_property * (* clone) (
        struct native_property *,
        CMPIStatus * );
};
struct _NativeCMPIBrokerFT
{
    CMPIBrokerFT brokerFt;
    CMPIString* (*getMessage) (
        const CMPIBroker* mb,
        const char *msgId,
        const char *defMsg,
        CMPIStatus* rc,
        unsigned int count,
        ...);
    CMPIStatus (*logMessage) (
        const CMPIBroker*,
        CMPISeverity severity ,
        const char *id,
        const char *text,
        const CMPIString *string);

    CMPIStatus (*trace) (
        const CMPIBroker*,
        CMPILevel level,
        const char *component,
        const char *text,
        const CMPIString *string);

    CMPIBoolean (*classPathIsA) (
        const CMPIBroker * broker,
        const CMPIObjectPath * cop,
        const char * type,
        CMPIStatus * rc );

    // Added for Remote CMPI support for indications.
    // TODO: We need to remove these functions, once we done with
    // complete implementation of these functionalities on remote side.
    // As of now we are taking help of MB by making UP calls. -V 5245
    CMPIBoolean (*selExp_evaluate) (
        CONST CMPISelectExp * exp,
        CONST CMPIInstance *inst,
        CMPIStatus *rc);
    CMPISelectCond* (*selExp_getDOC) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPISelectCond* (*selExp_getCOD) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPIBoolean (*selExp_evaluateUsingAccessor) (
        CONST CMPISelectExp * exp,
        CMPIAccessor *accessor,
        void *parm,
        CMPIStatus *rc);
    CMPISelectExp* (*selExp_clone) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPIString* (*selExp_getString) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPIStatus  (*selExp_release) (CONST CMPISelectExp* se);
    CMPISelectExp* (*selExp_newSelExp) (
        const char * queryString,
        const char * language,
        CMPIArray ** projection,
        CMPIStatus * rc );
#ifdef CMPI_VER_200

    CMPIError* (*newCMPIError) (
        const CMPIBroker*,
        const char*,
        const char*,
        const char*,
        const CMPIErrorSeverity,
        const CMPIErrorProbableCause,
        const CMPIrc, CMPIStatus*);
    CMPIStatus (*openMessageFile) (
        const CMPIBroker *,
        const char*,
        CMPIMsgFileHandle*);
    CMPIStatus (*closeMessageFile) (
        const CMPIBroker *,
        const CMPIMsgFileHandle);
    CMPIString* (*getMessage2) (
        const CMPIBroker *,
        const char *,
        const CMPIMsgFileHandle,
        const char *,
        CMPIStatus*,
        CMPICount,
        ...);
#endif /* CMPI_VER_200 */

};
#else
# define CONST

struct native_propertyFT
{

    //! Adds a new native_property to a list.
    int (* addProperty) (
        struct native_property **,
        int,
        const char *,
        CMPIType,
        CMPIValueState,
        CMPIValue * );

    //! Resets the values of an existing native_property, if existant.
    int (* setProperty) (
        struct native_property *,
        int,
        const char *,
        CMPIType,
        CMPIValue * );

     //! Sets PropertyOrigin,this internal function is called after setProperty.
     int (* setPropertyOrigin) (
         struct native_property *,
         const char *,
         const char *,
         int );

    //! Looks up a specifix native_property in CMPIData format.
    CMPIData (* getDataProperty) (
        struct native_property *,
        const char *,
        CMPIStatus * );

    //! Extract an indexed native_property in CMPIData format.
    CMPIData (* getDataPropertyAt) (
        struct native_property *,
        unsigned int,
        CMPIString **,
        CMPIStatus * );

    //! Yields the number of native_property items in a list.
    CMPICount (* getPropertyCount) (
        struct native_property *,
        CMPIStatus * );

    //! Releases a complete list of native_property items.
    void (* release) ( struct native_property * );

    //! Clones a complete list of native_property items.
    struct native_property * (* clone) (
        struct native_property *,
        CMPIStatus * );
};

struct _NativeCMPIBrokerFT
{
    CMPIBrokerFT brokerFt;
    CMPIArray *(*getKeyNames)(
        CMPIBroker * broker,
        CMPIContext * context,
        CMPIObjectPath * cop,
        CMPIStatus * rc);
    CMPIString* (*getMessage) (
        CMPIBroker* mb,
        const char *msgId,
        const char *defMsg,
        CMPIStatus* rc,
        unsigned int count,
        va_list);
    CMPIBoolean (*classPathIsA) (
        CMPIBroker * broker,
        CMPIObjectPath * cop,
        const char * type,
        CMPIStatus * rc );

    // Added for Remote CMPI support for indications.
    // TODO: We need to remove these functions, once we done with
    // complete implementation of these functionalities on remote side.
    // As of now we are taking help of MB by making UP calls. -V 5245
    CMPIBoolean (*selExp_evaluate) (
        CONST CMPISelectExp * exp,
        CONST CMPIInstance *inst,
        CMPIStatus *rc);
    CMPISelectCond* (*selExp_getDOC) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPISelectCond* (*selExp_getCOD) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPIBoolean (*selExp_evaluateUsingAccessor) (
        CONST CMPISelectExp * exp,
        CMPIAccessor *accessor,
        void *parm,
        CMPIStatus *rc);
    CMPISelectExp* (*selExp_clone) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPIString* (*selExp_getString) (
        CONST CMPISelectExp * exp,
        CMPIStatus * rc);
    CMPIStatus  (*selExp_release) (CONST CMPISelectExp* se);
    CMPISelectExp* (*selExp_newSelExp) (
        const char * queryString,
        const char * language,
        CMPIArray ** projection,
        CMPIStatus * rc );
};
#endif
typedef struct _NativeCMPIBrokerFT NativeCMPIBrokerFT;


/****************************************************************************/

PEGASUS_EXPORT void native_release_CMPIValue ( CMPIType, CMPIValue * val );
PEGASUS_EXPORT CMPIValue native_clone_CMPIValue (
    CMPIType,
    CONST CMPIValue * val,
    CMPIStatus * );
PEGASUS_EXPORT CMPIString * native_new_CMPIString (
    const char *,
    CMPIStatus * );
PEGASUS_EXPORT CMPIArray * native_new_CMPIArray (
    CMPICount size,
    CMPIType type,
    CMPIStatus * );
PEGASUS_EXPORT void native_array_increase_size ( CMPIArray *, CMPICount );
PEGASUS_EXPORT CMPIInstance * native_new_CMPIInstance (
    CONST CMPIObjectPath *,
    CMPIStatus * );
PEGASUS_EXPORT CMPIResult *  native_new_CMPIResult ( CMPIStatus * );
PEGASUS_EXPORT CMPIArray *  native_result2array ( CMPIResult * );
PEGASUS_EXPORT CMPIEnumeration *  native_new_CMPIEnumeration (
    CMPIArray *,
    CMPIStatus * );
PEGASUS_EXPORT CMPIObjectPath * native_new_CMPIObjectPath (
    const char *,
    const char *,
    CMPIStatus * );
PEGASUS_EXPORT CMPIArgs * native_new_CMPIArgs ( CMPIStatus * );
PEGASUS_EXPORT CMPIDateTime * native_new_CMPIDateTime ( CMPIStatus * );
PEGASUS_EXPORT CMPIDateTime * native_new_CMPIDateTime_fromBinary (
    CMPIUint64,
    CMPIBoolean,
    CMPIStatus * );
PEGASUS_EXPORT CMPIDateTime * native_new_CMPIDateTime_fromChars (
    const char *,
    CMPIStatus * );
PEGASUS_EXPORT CMPISelectExp * native_new_CMPISelectExp (
    CMPIUint64,
    CMPIStatus * );
PEGASUS_EXPORT CMPIContext *  native_new_CMPIContext ( int mem_state );
PEGASUS_EXPORT void  native_release_CMPIContext ( CONST CMPIContext * );
PEGASUS_EXPORT void  native_release_CMPISelectExp ( CONST CMPISelectExp *);

#ifndef PEGASUS_TCPCOMM_REMOTE
# ifdef PEGASUS_OS_TYPE_WINDOWS

PEGASUS_EXPORT extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;

# else

extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;
# endif
#endif

#ifdef PEGASUS_TCPCOMM_REMOTE
PEGASUS_IMPORT extern CMPIBrokerEncFT  native_brokerEncFT;
#else
PEGASUS_EXPORT extern CMPIBrokerEncFT  native_brokerEncFT;
#endif

extern struct native_propertyFT propertyFT;

#endif

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/

