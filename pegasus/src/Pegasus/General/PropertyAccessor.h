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

#ifndef Pegasus_PropertyAccessor_h
#define Pegasus_PropertyAccessor_h

#include <Pegasus/Common/CIMInstance.h>

PEGASUS_NAMESPACE_BEGIN
/*
    PropertyAccessor is a set of support functions for property value access
    within instances.  These functions provide the capability to get and set
    the value of all of the native property types within Pegasus using only
    the reference to the instance and the property name as identifiers.  They
    provide, in effect, an associative link to property values as an
    alternate to the index based reference defined in the CIMInstance
    object.  Further, they allow directly getting and setting the value in
    place of the multistep approach in the objects where the the CIMValue
    must first be retrieved before the value can be accessed.  This reduces
    the access to a single line of code.

    These functions assume that the property exists in the instance.

    Examples:
        Set(_inst, "ErrorSourceFormat", Uint16(value), null);
        bool nullStat = Get(_inst, "ErrorSourceFormat", t);
    These functions are to be used only internally within OpenPegasus
*/

/** Get the named property value from the target instance.
    @param inst CIMInstance from which the property value is to be retrieved.
    @param name String Name of the property which is to be accessed.
    @param value Boolean The value of the property is returned in this
    variable if the NULL attribute is NOT set for the CIMValue for this
    property.
    @return bool returns true if the CIMValue for the property is NOT Null.
    @exception Returns CIM_ERR_NO_SUCH_PROPERTY if the named property is not
    in the instance.
*/
bool Get(
    const CIMInstance& inst,
    const String& name,
    Boolean& value);

/** Get the named property value from the target instance.
    @param inst CIMInstance from which the property value is to be retrieved.
    @param name String Name of the property which is to be accessed.
    @param value Boolean The value of the property is returned in this
    variable if the NULL attribute is NOT set for the CIMValue for this
    property.
    @param null bool If this is set true, the value parameter is ignored and the
    property is set to NULL.
    @return bool returns true if the CIMValue for the property is NOT Null.
    @exception Returns CIM_ERR_NO_SUCH_PROPERTY if the named property is not
    in the instance.
*/
void Set(
    CIMInstance& inst,
    const String& name,
    const Boolean& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Boolean>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Boolean>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint8& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint8& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint8>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint8>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint8& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint8& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint8>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint8>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint16& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint16& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint16>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint16>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint16& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint16& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint16>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint16>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint32& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint32& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint32>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint32>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint32& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint32& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint32>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint32>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint64& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint64& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint64>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint64>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint64& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint64& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint64>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint64>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Real32& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Real32& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Real32>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Real32>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Real64& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Real64& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Real64>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Real64>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Char16& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Char16& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Char16>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Char16>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    String& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const String& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<String>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<String>& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    CIMDateTime& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const CIMDateTime& value,
    bool null);

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<CIMDateTime>& value);

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<CIMDateTime>& value,
    bool null);

inline CIMType GetType(Boolean*) { return CIMTYPE_BOOLEAN; }
inline CIMType GetType(Uint8*) { return CIMTYPE_UINT8; }
inline CIMType GetType(Sint8*) { return CIMTYPE_SINT8; }
inline CIMType GetType(Uint16*) { return CIMTYPE_UINT16; }
inline CIMType GetType(Sint16*) { return CIMTYPE_SINT16; }
inline CIMType GetType(Uint32*) { return CIMTYPE_UINT32; }
inline CIMType GetType(Sint32*) { return CIMTYPE_SINT32; }
inline CIMType GetType(Uint64*) { return CIMTYPE_UINT64; }
inline CIMType GetType(Sint64*) { return CIMTYPE_SINT64; }
inline CIMType GetType(Real32*) { return CIMTYPE_REAL32; }
inline CIMType GetType(Real64*) { return CIMTYPE_REAL64; }
inline CIMType GetType(Char16*) { return CIMTYPE_CHAR16; }
inline CIMType GetType(String*) { return CIMTYPE_STRING; }
inline CIMType GetType(CIMDateTime*) { return CIMTYPE_DATETIME; }
inline CIMType GetType(CIMObjectPath*) { return CIMTYPE_REFERENCE; }
inline CIMType GetType(CIMObject*) { return CIMTYPE_OBJECT; }
inline CIMType GetType(Array<Boolean>*) { return CIMTYPE_BOOLEAN; }
inline CIMType GetType(Array<Uint8>*) { return CIMTYPE_UINT8; }
inline CIMType GetType(Array<Sint8>*) { return CIMTYPE_SINT8; }
inline CIMType GetType(Array<Uint16>*) { return CIMTYPE_UINT16; }
inline CIMType GetType(Array<Sint16>*) { return CIMTYPE_SINT16; }
inline CIMType GetType(Array<Uint32>*) { return CIMTYPE_UINT32; }
inline CIMType GetType(Array<Sint32>*) { return CIMTYPE_SINT32; }
inline CIMType GetType(Array<Uint64>*) { return CIMTYPE_UINT64; }
inline CIMType GetType(Array<Sint64>*) { return CIMTYPE_SINT64; }
inline CIMType GetType(Array<Real32>*) { return CIMTYPE_REAL32; }
inline CIMType GetType(Array<Real64>*) { return CIMTYPE_REAL64; }
inline CIMType GetType(Array<Char16>*) { return CIMTYPE_CHAR16; }
inline CIMType GetType(Array<String>*) { return CIMTYPE_STRING; }
inline CIMType GetType(Array<CIMDateTime>*) { return CIMTYPE_DATETIME; }
inline CIMType GetType(Array<CIMObjectPath>*) { return CIMTYPE_REFERENCE; }
inline CIMType GetType(Array<CIMObject>*) { return CIMTYPE_OBJECT; }

inline bool IsArray(Boolean*) { return false; }
inline bool IsArray(Uint8*) { return false; }
inline bool IsArray(Sint8*) { return false; }
inline bool IsArray(Uint16*) { return false; }
inline bool IsArray(Sint16*) { return false; }
inline bool IsArray(Uint32*) { return false; }
inline bool IsArray(Sint32*) { return false; }
inline bool IsArray(Uint64*) { return false; }
inline bool IsArray(Sint64*) { return false; }
inline bool IsArray(Real32*) { return false; }
inline bool IsArray(Real64*) { return false; }
inline bool IsArray(Char16*) { return false; }
inline bool IsArray(String*) { return false; }
inline bool IsArray(CIMDateTime*) { return false; }
inline bool IsArray(CIMObjectPath*) { return false; }
inline bool IsArray(CIMObject*) { return false; }
inline bool IsArray(Array<Boolean>*) { return true; }
inline bool IsArray(Array<Uint8>*) { return true; }
inline bool IsArray(Array<Sint8>*) { return true; }
inline bool IsArray(Array<Uint16>*) { return true; }
inline bool IsArray(Array<Sint16>*) { return true; }
inline bool IsArray(Array<Uint32>*) { return true; }
inline bool IsArray(Array<Sint32>*) { return true; }
inline bool IsArray(Array<Uint64>*) { return true; }
inline bool IsArray(Array<Sint64>*) { return true; }
inline bool IsArray(Array<Real32>*) { return true; }
inline bool IsArray(Array<Real64>*) { return true; }
inline bool IsArray(Array<Char16>*) { return true; }
inline bool IsArray(Array<String>*) { return true; }
inline bool IsArray(Array<CIMDateTime>*) { return true; }
inline bool IsArray(Array<CIMObjectPath>*) { return true; }
inline bool IsArray(Array<CIMObject>*) { return true; }

PEGASUS_NAMESPACE_END

#endif /* Pegasus_PropertyAccessor_h */
