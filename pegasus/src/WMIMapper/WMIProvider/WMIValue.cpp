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
// Modified By:    Barbara Packard (barbara_packard@hp.com)
//              Paulo Sehn (paulo_sehn@hp.com)
//              Adriano Zanuz (adriano.zanuz@hp.com)
//              Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//              Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <Pegasus/Common/CIMInstance.h>

#include "WMIValue.h"
#include "WMIString.h"
#include "WMIType.h"
#include "WMIDateTime.h"
#include "WMIObjectPath.h"
#include "WMIObject.h"

PEGASUS_NAMESPACE_BEGIN

static const char _NULL_INTERVAL_TYPE_STRING[] = "00000000000000.000000:000";
static const char _NULL_DATE_TYPE_STRING[] = "00000000000000.000000-000";
static const char _ZEROED_DATE_TYPE_STRING[] = "00000000000000.000000+000";

WMIValue::WMIValue(const CIMValue & value) : CIMValue(value)
{
}

WMIValue::WMIValue(const CComVariant & vValue)
{
    CIMValue val;
    val = getValue(vValue);
    *this = CIMValue(val);
}

WMIValue::WMIValue(VARTYPE vt, void *pVal)
{
    CIMValue val;
    val = getCIMValueFromVariant(vt, pVal);
    *this = CIMValue(val);
}

WMIValue::WMIValue(VARTYPE vt, void *pVal, const CIMTYPE type)
{
    CIMValue val;
    val = getCIMValueFromVariant(vt, pVal, type);
    *this = CIMValue(val);
}


WMIValue::WMIValue(const VARIANT & value, const CIMTYPE type)
{
    CIMValue val;
    CComVariant vValue;
    vValue = value;

    if ((vValue.vt != VT_NULL) && ((type & ~CIM_FLAG_ARRAY) != CIM_REFERENCE))
    {
        val = getValue(vValue, type);
    }
    else
    {
        // for class definitions, the value will be null and will except
        _variant_t vt(value);

        switch(type)
        {
        case CIM_BOOLEAN:
            try
            {
                val.set(Boolean(bool(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_BOOLEAN, false, 0);
            }

            break;
        case CIM_BOOLEAN | CIM_FLAG_ARRAY:
            val.setNullValue(CIMType(CIMTYPE_BOOLEAN), true, 0);

            break;
        case CIM_SINT8:
            try
            {
                val.set(Sint8(BYTE(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_SINT8, false, 0);
            }

            break;
        case CIM_SINT8 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_SINT8, true, 0);

            break;
        case CIM_UINT8:
            try
            {
                val.set(Uint8(BYTE(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_UINT8, false, 0);
            }

            break;
        case CIM_UINT8 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_UINT8, true, 0);

            break;
        case CIM_SINT16:
            try
            {
                val.set(Sint16(short(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_SINT16, false, 0);
            }

            break;
        case CIM_SINT16 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_SINT16, true, 0);

            break;
        case CIM_UINT16:
            try
            {
                val.set(Uint16(short(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_UINT16, false, 0);
            }

            break;
        case CIM_UINT16 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_UINT16, true, 0);

            break;
        case CIM_SINT32:
            try
            {
                val.set(Sint32(long(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_SINT32, false, 0);
            }

            break;
        case CIM_SINT32 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_SINT32, true, 0);

            break;
        case CIM_UINT32:
            try
            {
                val.set(Uint32(long(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_UINT32, false);
            }

            break;
        case CIM_UINT32 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_UINT32, true, 0);

            break;
        case CIM_SINT64:
            try
            {
                _bstr_t bstr(vt);

                val.set(Sint64(_wtoi64(bstr)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_SINT64, false, 0);
            }

            break;
        case CIM_SINT64 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_SINT64, true, 0);

            break;
        case CIM_UINT64:
            try
            {
                _bstr_t bstr(vt);

                val.set(Uint64(_wtoi64(bstr)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_UINT64, false, 0);
            }

            break;
        case CIM_UINT64 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_UINT64, true, 0);

            break;
        case CIM_REAL32:
            try
            {
                val.set(Real32(float(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_REAL32, false, 0);
            }

            break;
        case CIM_REAL32 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_REAL32, true, 0);

            break;
        case CIM_REAL64:
            try
            {
                val.set(Real64(double(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_REAL64, false, 0);
            }

            break;
        case CIM_REAL64 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_REAL64, true, 0);

            break;
        case CIM_STRING:
            try
            {
                val.set(WMIString(_bstr_t(vt)));
            }
            catch(...)
            {
                PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL2,
                    "WMIValue::WMIValue - Constructor failed; "
                    "setting NULL string");
                    val.setNullValue(CIMTYPE_STRING, false, 0);
            }

            break;
        case CIM_STRING | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_STRING, true, 0);

            break;
        case CIM_DATETIME:
            try
            {
                val.set(WMIDateTime(_bstr_t(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_DATETIME, false, 0);
            }

            break;
        case CIM_DATETIME | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_DATETIME, true, 0);

            break;
        case CIM_REFERENCE:
            try
            {
                val.set(WMIObjectPath(_bstr_t(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_REFERENCE, false);
            }

            break;
        case CIM_REFERENCE | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_REFERENCE, true, 0);

            break;
        case CIM_CHAR16:
            try
            {
                val.set(Char16(short(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_CHAR16, false);
            }

            break;
        case CIM_CHAR16 | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_CHAR16, true, 0);

            break;
        case CIM_OBJECT:
            try
            {
                val.set(WMIObjectPath(_bstr_t(vt)));
            }
            catch(...)
            {
                val.setNullValue(CIMTYPE_OBJECT, false);
            }
            break;
        case CIM_OBJECT | CIM_FLAG_ARRAY:
            val.setNullValue(CIMTYPE_OBJECT, true);
            break;
        case CIM_EMPTY:

            break;
        case CIM_ILLEGAL:
        default:

            break;
        }
    }

    vValue.Clear();
    *this = CIMValue(val);
}

/////////////////////////////////////////////////////////////////////////////
// WMIValue::getCIMValueFromVariant - helper function to convert a variant
//        value to a CIM value
//
// ///////////////////////////////////////////////////////////////////////////
CIMValue WMIValue::getCIMValueFromVariant(
        VARTYPE vt,
        void *pVal,
        const CIMTYPE Type)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIValue::getCIMValueFromVariant()");

    CIMValue cimValue;
    String str;

    switch (vt)
    {
        case VT_I2:
            // have to screw around here because automation type
            // is not necessarily the mof type...
            if (CIM_SINT8 == Type)
            {
                return (CIMValue((Sint8)*((BYTE *)pVal)));
            }
            else if (CIM_CHAR16 == Type)
            {
                return (CIMValue((Char16)*((short *)pVal)));
            }
            else
            {
                return (CIMValue((Sint16)*((short *)pVal)));
            }

            break;

        case VT_I4:
            // have to screw around here because automation type
            // is not necessarily the mof type...
            if (CIM_UINT16 == Type)
            {
                return (CIMValue((Uint16)*((unsigned short *)pVal)));
            }
            else if (CIM_UINT32 == Type)
            {
                return (CIMValue((Uint32)*((unsigned long *)pVal)));
            }
            else
            {
                return (CIMValue((Sint32)*((int *)pVal)));
            }
            break;

        case VT_R4:
            return (CIMValue((Real32)*((float *)pVal)));
            break;

        case VT_R8:
            return (CIMValue((Real64)*((double *)pVal)));
            break;

        case VT_DATE:
        {
            ////ATTN: it needs to be checked to see if this really
            // works!!
            DATE date;
            CIMDateTime tmp;
            SYSTEMTIME sTime;

            date = *(DATE *)pVal;

            if (VariantTimeToSystemTime(date, &sTime))
            {
                WBEMTime wTime(sTime);

                char value[26];
                LONG diffUTC = WBEMTime::GetLocalOffsetForDate(&sTime);
                sprintf(value, "%04d%02d%02d%02d%02d%02d.%06d%c%03d",
                    sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour,
                    sTime.wMinute, sTime.wSecond, sTime.wMilliseconds,
                    ((diffUTC < 0) ? '-' : '+'), abs(diffUTC));

                str = value;

                PEG_TRACE((TRC_WMIPROVIDER,Tracer::LEVEL4,
                    "WMIValue::getValue() - Date is %s",
                    (LPCSTR)str.getCString()));
            }
            else
            {
                //ATTN: just to have something for now
                str = _NULL_INTERVAL_TYPE_STRING;
            }

            tmp.set(str);

            return CIMValue(tmp);
        }
        break;

        case VT_BSTR:
        {
            BSTR bsTemp = *((BSTR *)pVal);
            // Not making a copy when constructing the _bstr_t,
            // For some reason, setting fCopy to TRUE causes a memory leak
            // to be reported (even though the destructor is freeing
            // the string!)
            // This seems to work, but needs to be looked at closer some day:
            _bstr_t bstr(bsTemp, FALSE);
            str.assign((Char16*)((wchar_t*)bstr));

            //By Jair - Due to Windows automation limitations
            //the 64 integer bit numbers and the datetime routine
            //are being cast as strings. We must to handle this in
            //order to correctly answer the dispatcher.
            if (CIM_SINT64 == Type)
            {
                return (CIMValue(Sint64(_wtoi64(bstr))));
            }
            else if (CIM_UINT64 == Type)
            {
                return (CIMValue(Uint64(_wtoi64(bstr))));
            }
            else if (CIM_DATETIME == Type)
            {
                CIMDateTime dt;

                //By Jair - Exchanging asterisks for zeros
                Uint32 iCount = 0;

                for (Uint32 i = 0; i < str.size(); i++)
                {
                    if (str[i] == '*')
                    {
                        str[i] = '0';
                        iCount++;
                    }
                }

                //if there are only asterisks, then pass a NULL interval
                if (iCount == str.size() - 2)
                {
                    str = _NULL_INTERVAL_TYPE_STRING;
                }

                // if WMI datetime value have a zeroed value
                // then the CIMValue will be set as null. CIM Schema do not
                // permits zeroed values to datetime and consider it a
                // out of range value.
                if (String::equal(str, String(_ZEROED_DATE_TYPE_STRING)))
                {
                    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
                        "Date Time Zeroed. Setting Value as NULL."));


                    CIMValue dtValue;
                    dtValue.setNullValue(CIMTYPE_DATETIME, false, 0);
                    return (dtValue);
                }

                dt.set(str);

                return (CIMValue(WMIDateTime(dt)));
            }
            else
            {
                if (0 == str.size())
                {
                    return CIMValue(String(""));
                }
                else
                {
                    return CIMValue(str);
                }
            }
        }
        break;

        case VT_BOOL:
            return ((*(VARIANT_BOOL *)pVal) ?
                CIMValue(true) : CIMValue(false));
            break;

        case VT_I1:
            return (CIMValue((Sint8)*((char *)pVal)));
            break;

        case VT_UI1:
            return (CIMValue((Uint8)*((BYTE *)pVal)));
            break;

        case VT_UI2:
            return (CIMValue((Uint16)*((unsigned short *)pVal)));
            break;

        case VT_UI4:
            return (CIMValue((Uint32)*((unsigned long *)pVal)));
            break;

        case VT_I8:
            ////ATTN: Will this conversion work??
            return (CIMValue((Sint64)*((_int64 *)pVal)));
            break;

        case VT_UI8:
            ////ATTN: Will this conversion work??
            return (CIMValue((Uint64)*((unsigned _int64 *)pVal)));
            break;

        case VT_INT:
            if (sizeof(int) == 8)
            {
                return (CIMValue((Sint64)*((_int64 *)pVal)));
            }
            else
            {
                return (CIMValue((Sint32)*((long *)pVal)));
            }
            break;

        case VT_UINT:
            if (sizeof(int) == 8)
            {
                return (CIMValue((Uint64)*((unsigned _int64 *)pVal)));
            }
            else
            {
                return (CIMValue((Uint32)*((unsigned long *)pVal)));
            }
            break;
        case VT_UNKNOWN:
            if (CIM_OBJECT == Type)
            {
                IUnknown *punkVal = NULL;
                CComPtr <IWbemClassObject> pClass = NULL;

                punkVal = (IUnknown *)*((unsigned long *)pVal);
                HRESULT hr = punkVal->QueryInterface(
                    IID_IWbemClassObject,
                    reinterpret_cast< void** >(&pClass));

                if (S_OK == hr) {
                   BSTR pstrObjectText;
                   hr = pClass->GetObjectText(0, &pstrObjectText);

                   PEG_METHOD_EXIT();
                   return (CIMObject(WMIObject(pClass)));
                }

                throw TypeMismatchException();
            }

            break;

        default:
            throw TypeMismatchException();
            break;
    };
}

/////////////////////////////////////////////////////////////////////////////
// WMIValue::getValue - converts a variant to a Pegasus CIMValue
//
// ///////////////////////////////////////////////////////////////////////////
CIMValue WMIValue::getValue(const CComVariant & vValue, const CIMTYPE Type)
{
    bool isRef = isReferenceType(vValue.vt);
    bool isArray = isArrayType(vValue.vt);

    if (isArray)
    {
        return getArrayValue(vValue, Type);
    }

    VARTYPE vt = isRef ? vValue.vt ^ VT_BYREF : vValue.vt;

    void *pValue;

    // just getting the address of the value field, type doesn't matter
    if (isRef)
    {
        pValue = (void *)vValue.plVal;
    }
    else
    {
        pValue = (void *)&(vValue.lVal);
    }

    return (getCIMValueFromVariant(vt, pValue, Type));
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getArrayValueAux - helper function for getArrayValue
//
// ///////////////////////////////////////////////////////////////////////////
template<class T>
CIMValue getArrayValueAux(LPSAFEARRAY psa, VARTYPE vt, const CIMTYPE Type, T*)
{
    long index, numElements, i;
    UINT uSize;
    BYTE *pValue;
    HRESULT hr;

    Array <T> array;

    index = psa->rgsabound[0].lLbound;    // get the beginning index
    numElements = psa->rgsabound[0].cElements;    // elements in the array
    uSize = SafeArrayGetElemsize(psa);        // size of each element

    pValue = new BYTE[uSize];
    void *pVal = (void *)pValue;

    hr = SafeArrayGetVartype(psa, &vt);

    for (i = 0; ((i < numElements) && (SUCCEEDED(hr))); i++, index++)
    {
        hr = SafeArrayGetElement(psa, &index, pVal);

        if (SUCCEEDED(hr))
        {
            CIMValue cimValue = WMIValue(vt, pVal, Type);

            T x;
            cimValue.get(x);
            array.append(x);
        }
    }

    delete [] pValue;

    if (SUCCEEDED(hr))
    {
        return CIMValue(array);
    }
    else
    {
        PEG_TRACE((TRC_WMIPROVIDER,Tracer::LEVEL1,
            "WMIValue::getArrayValueAux() - SafeArray result is %x",  hr));
        throw CIMException(CIM_ERR_FAILED, "getArrayValueAux()");
    }
}

/////////////////////////////////////////////////////////////////////////////
// WMIValue::getArrayValue- converts a variant array to a
//        Pegasus CIMValue of type array
//        - uses all the SafeArray garbage
//    NOTE:
//        We are assuming one-dimensional arrays
//
// ///////////////////////////////////////////////////////////////////////////
CIMValue WMIValue::getArrayValue(const CComVariant& vValue, const CIMTYPE Type)
{

    LPSAFEARRAY psa = NULL;
    HRESULT hr;

    CIMTYPE type = Type & (~CIM_FLAG_ARRAY);

    if (!(isArrayType(vValue.vt)))
    {
        return (getValue(vValue));
    }

    if (isReferenceType(vValue.vt))
    {
        psa = *V_ARRAYREF(&vValue);
    }
    else
    {
        psa = V_ARRAY(&vValue);
    }

    VARTYPE vt = VT_NULL;
    CIMType cimType;

    hr = SafeArrayGetVartype(psa, &vt);

    // check for Mof and Automation type discrepancy
    if (VT_I4 == vt)
    {
        if (CIM_UINT16 == type)
        {
            cimType = CIMTYPE_UINT16;
        }
        else if (CIM_UINT32 == type)
        {
            cimType = CIMTYPE_UINT32;
        }
        else
        {
            cimType = vartypeToCIMType(vt);
        }
    }
    else if (VT_I2 == vt)
    {
        if (CIM_SINT8 == type)
        {
            cimType = CIMTYPE_SINT8;
        }
        else if (CIM_CHAR16 == type)
        {
            cimType = CIMTYPE_CHAR16;
        }
        else
        {
            cimType = vartypeToCIMType(vt);
        }
    }
    else if (VT_BSTR == vt)
    {
        if (CIM_SINT64 == type)
        {
            cimType = CIMTYPE_SINT64;
        }
        else if (CIM_UINT64 == type)
        {
            cimType = CIMTYPE_UINT64;
        }
        else if (CIM_DATETIME == type)
        {
            cimType = CIMTYPE_DATETIME;
        }
        else
        {
            cimType = vartypeToCIMType(vt);
        }
    }
    else if (VT_UNKNOWN == vt)
    {
        if (CIM_OBJECT == type)
        {
            cimType = CIMTYPE_OBJECT;
        }
        else
        {
            cimType = vartypeToCIMType(vt);
        }
    }
    else
    {
        cimType = vartypeToCIMType(vt);
    }

    if (SUCCEEDED(hr))
    {
        switch (cimType)
        {
            case CIMTYPE_BOOLEAN:
                return getArrayValueAux(psa, vt, type, (Boolean*)0);

            case CIMTYPE_STRING:
                return getArrayValueAux(psa, vt, type, (String*)0);

            case CIMTYPE_CHAR16:
                return getArrayValueAux(psa, vt, type, (Char16*)0);

            case CIMTYPE_UINT8:
                return getArrayValueAux(psa, vt, type, (Uint8*)0);

            case CIMTYPE_UINT16:
                return getArrayValueAux(psa, vt, type, (Uint16*)0);

            case CIMTYPE_UINT32:
                return getArrayValueAux(psa, vt, type, (Uint32*)0);

            case CIMTYPE_UINT64:
                return getArrayValueAux(psa, vt, type, (Uint64*)0);

            case CIMTYPE_SINT8:
                return getArrayValueAux(psa, vt, type, (Sint8*)0);

            case CIMTYPE_SINT16:
                return getArrayValueAux(psa, vt, type, (Sint16*)0);

            case CIMTYPE_SINT32:
                return getArrayValueAux(psa, vt, type, (Sint32*)0);

            case CIMTYPE_SINT64:
                return getArrayValueAux(psa, vt, type, (Sint64*)0);

            case CIMTYPE_DATETIME:
                return getArrayValueAux(psa, vt, type, (CIMDateTime*)0);

            case CIMTYPE_REAL32:
                return getArrayValueAux(psa, vt, type, (Real32*)0);

            case CIMTYPE_REAL64:
                return getArrayValueAux(psa, vt, type, (Real64*)0);

            case CIMTYPE_OBJECT:
                PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3, " CIMTYPE_OBJECT"));
                return getArrayValueAux(psa, vt, type, (CIMObject*)0);

            default:
                break;
        }
    }

    // if we get here, we are in trouble...
    throw CIMException(CIM_ERR_FAILED, "getArrayValue()");
}


VARIANT WMIValue::toVariant()
{
    CComVariant cv;
    VARIANT v;
    cv.Detach(&v);
    cv.Clear();
    return v;

}

/////////////////////////////////////////////////////////////////////////////
// WMIValue::getAsVariant - helper function to convert a CIM value
//             to a variant including arrays
//
// ///////////////////////////////////////////////////////////////////////////
void WMIValue::getAsVariant(CComVariant *var,
                            const String& nameSpace,
                            const String& userName,
                            const String& password)
{
    HRESULT hr;
    CMyString tmp;

    SAFEARRAY * pSA = NULL;
    SAFEARRAYBOUND rgsabound;

    switch (CIMTypeToWMIType(getType()))
    {
       case CIM_BOOLEAN:
            if (!isArray())
            {
                Boolean value;

                var->vt = VT_BOOL;
                get(value);
                var->boolVal = value;
            }
            else
            {
                //creates the safearray
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BOOL, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                       "Array creation failed.");
                }

                Array<Boolean> arValue;
                get(arValue);

                //sets the values to the array
                for (long i = 0; i < getArraySize(); i++)
                {
                    if (hr = SafeArrayPutElement(pSA, &i, &arValue[i]))
                    {
                      SafeArrayDestroy(pSA);
                      throw CIMException(CIM_ERR_FAILED,
                                         "Array put operation failed.");
                    }
                }

                //sets the value to the variant
                // by Jair - due to WMI coercion rules
                // the array of boolean must be set as an array of uint8.
                var->vt = VT_ARRAY | VT_UI1;
                var->parray = pSA;

            }
            break;

        case CIM_SINT8:
            if (!isArray())
            {
                Sint8 value;

                var->vt = VT_I2;
                get(value);
                var->iVal = value;
            }
            else
            {
                //creates the safearray
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BSTR, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                       "Array creation failed.");
                }

                Array<Sint8> arValue;
                get(arValue);

                //put the values to the array
                for (long i = 0; i < getArraySize(); i++)
                {
                    tmp.Format("%d", 21, arValue[i]);

                    CComVariant vOut;
                    vOut.vt = VT_BSTR;
                    vOut.bstrVal = tmp.Bstr();

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.bstrVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                    vOut.Clear();
                }

                // by Jair - due to WMI coercion rules
                // the array of boolean must be set as an array of uint8.
                var->vt = VT_ARRAY | VT_BSTR;
                var->parray = pSA;

            }
            break;

        case CIM_UINT8:
            if (!isArray())
            {
                Uint8 value;

                var->vt = VT_UI1;
                get(value);
                var->bVal = value;
            }
            else
            {
                //create the safearray
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_UI1, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Uint8> arValue;
                get(arValue);

              //put the values into the safearray
              for (long i = 0; i < getArraySize(); i++)
              {
                  if ((hr = SafeArrayPutElement(pSA, &i, &arValue[i])))
                  {
                      SafeArrayDestroy(pSA);
                      throw CIMException(CIM_ERR_FAILED,
                                         "Array put operation failed.");
                  }
              }

              //sets the value to the variant
              var->vt = VT_ARRAY | VT_UI1;
              var->parray = pSA;

          }
          break;

       case CIM_SINT16:
          if (!isArray ())
          {
              Sint16 value;

              var->vt = VT_I2;
              get(value);
              var->iVal = value;
          }
          else
          {
              rgsabound.lLbound = 0;
              rgsabound.cElements = getArraySize();
              pSA = SafeArrayCreate(VT_I2, 1, &rgsabound);

              if (!pSA)
              {
                  throw CIMException (CIM_ERR_FAILED,
                                      "Array creation failed.");
              }

              Array<Sint16> arValue;
              get(arValue);

              for (long i = 0; i < getArraySize(); i++)
              {
                  if ((hr = SafeArrayPutElement(pSA, &i, &arValue[i])))
                  {
                      SafeArrayDestroy(pSA);
                      throw CIMException(CIM_ERR_FAILED,
                                         "Array put operation failed.");
                  }
              }

              var->vt = VT_ARRAY | VT_I2;
              var->parray = pSA;

          }
          break;

        case CIM_UINT16:
            if (!isArray())
            {
                Uint16 value;

                var->vt = VT_I4;
                get(value);
                var->lVal = value;
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BSTR, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Uint16> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    tmp.Format("%u", 21, arValue[i]);

                    CComVariant vOut;
                    vOut.vt = VT_BSTR;
                    vOut.bstrVal = tmp.Bstr();

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.bstrVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                    vOut.Clear();
                }

                var->vt = VT_ARRAY | VT_BSTR;
                var->parray = pSA;

            }
            break;

        case CIM_SINT32:
            if (!isArray())
            {
                Sint32 value;

                var->vt = VT_I4;
                get(value);
                var->lVal = value;
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_I4, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Sint32> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    if ((hr = SafeArrayPutElement(pSA, &i, &arValue[i])))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                }

                var->vt = VT_ARRAY | VT_I4;
                var->parray = pSA;

            }
            break;

        case CIM_UINT32:
            if (!isArray ())
            {
                Uint32 value;

                var->vt = VT_I4;
                get(value);
                var->lVal = value;
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_I4, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Uint32> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    if ((hr = SafeArrayPutElement(pSA, &i, &arValue[i])))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                }

                var->vt = VT_ARRAY | VT_I4;
                var->parray = pSA;

            }
            break;

        case CIM_UINT64:
            if (!isArray())
            {
                Uint64 value;

                get(value);
                tmp.Format("%I64u", 21, value);
                var->vt = VT_BSTR;
                var->bstrVal = tmp.Bstr();
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BSTR, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Uint64> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    tmp.Format("%I64u", 21, arValue[i]);

                    CComVariant vOut;
                    vOut.vt = VT_BSTR;
                    vOut.bstrVal = tmp.Bstr();

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.bstrVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                    vOut.Clear();
                }

                var->vt = VT_ARRAY | VT_BSTR;
                var->parray = pSA;

            }
            break;

        case CIM_SINT64:
            if (!isArray ())
            {
                Sint64 value;

                get(value);
                tmp.Format("%I64d", 21, value);
                var->vt = VT_BSTR;
                var->bstrVal = tmp.Bstr();
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BSTR, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Sint64> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    CComVariant vOut;

                    tmp.Format("%I64d", 21, arValue[i]);
                    vOut.vt = VT_BSTR;
                    vOut.bstrVal = tmp.Bstr();

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.bstrVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                    vOut.Clear();
                }

                var->vt = VT_ARRAY | VT_BSTR;
                var->parray = pSA;

            }
            break;

        case CIM_REAL32:
            if (!isArray ())
            {
                Real32 value;

                var->vt = VT_R4;
                get(value);
                var->fltVal = value;
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_R4, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Real32> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    if ((hr = SafeArrayPutElement(pSA, &i, &arValue[i])))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                }

                var->vt = VT_ARRAY | VT_R4;
                var->parray = pSA;

            }
            break;

        case CIM_REAL64:
            if (!isArray ())
            {
                Real64 value;

                var->vt = VT_R8;
                get(value);
                var->dblVal = value;
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_R8, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Real64> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    if ((hr = SafeArrayPutElement(pSA, &i, &arValue[i])))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                }

                var->vt = VT_ARRAY | VT_R8;
                var->parray = pSA;

            }
            break;

        case CIM_CHAR16:
            if (!isArray ())
            {
                Char16 value;

                var->vt = VT_I2;
                get(value);
                var->iVal = value;
            }
            else
            {
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_I2, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<Char16> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    if ((hr = SafeArrayPutElement(pSA, &i, &arValue[i])))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                }

                var->vt = VT_ARRAY | VT_I2;
                var->parray = pSA;

            }
            break;

        case CIM_STRING:
            if (!isArray())
            {
                String data;

                get (data);
                var->vt = VT_BSTR;
                var->bstrVal = SysAllocString(
                    (const WCHAR *)data.getChar16Data());
            }
            else
            {
                // creates an array of BSTRs
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BSTR, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<String> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    CComVariant vOut;

                    vOut.vt = VT_BSTR;  // set type
                    vOut.bstrVal = SysAllocString(
                        (const WCHAR *)arValue[i].getChar16Data());

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.bstrVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                    vOut.Clear();
                }

                var->vt = VT_ARRAY | VT_BSTR;
                var->parray = pSA;

            }
            break;

        case CIM_DATETIME:
            if(!isArray())
            {
                CIMDateTime value;

                get(value);
                tmp = value.toString();
                var->vt = VT_BSTR;
                var->bstrVal = tmp.Bstr();
            }
            else
            {
                // creates an array of BSTRs
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BSTR, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                        "Array creation failed.");
                }

                Array<CIMDateTime> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    CComVariant vOut;

                    tmp = arValue[i].toString();
                    vOut.vt = VT_BSTR;  // set type
                    vOut.bstrVal = tmp.Bstr();

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.bstrVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                    vOut.Clear();
                }

                var->vt = VT_ARRAY | VT_BSTR;
                var->parray = pSA;
            }
            break;

        case CIM_REFERENCE:
            if(!isArray())
            {
                CIMObjectPath value;
                String sObjName;

                get(value);
                sObjName = value.toString();

                tmp = sObjName;

                var->vt = VT_BSTR;
                var->bstrVal = tmp.Bstr();
            }
            else
            {
                // creates an array of BSTRs
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_BSTR, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                       "Array creation failed.");
                }

                Array<CIMObjectPath> arValue;
                get(arValue);

                for (long i = 0; i < getArraySize(); i++)
                {
                    String sObjName;
                    CComVariant vOut;

                    sObjName = arValue[i].toString();

                    tmp = sObjName;
                    vOut.vt = VT_BSTR;  // set type
                    vOut.bstrVal = tmp.Bstr();

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.bstrVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }
                    vOut.Clear();
                }
                var->vt = VT_ARRAY | VT_BSTR;
                var->parray = pSA;

            }

            break;

        case CIM_OBJECT:
            // TODO: include Object support
            if(!isArray())
            {
                CIMObject mTmpObj;
                get(mTmpObj);
                WMIObject mObj(mTmpObj);

                *var = mObj.toVariant(nameSpace, userName, password);
            }
            else
            {
                // creates an array of Objects
                rgsabound.lLbound = 0;
                rgsabound.cElements = getArraySize();
                pSA = SafeArrayCreate(VT_UNKNOWN, 1, &rgsabound);

                if (!pSA)
                {
                    throw CIMException (CIM_ERR_FAILED,
                                       "Array creation failed.");
                }

                Array<CIMObject> mTmpObjArray;
                get(mTmpObjArray);

                for (long i = 0; i < getArraySize(); i++)
                {

                    WMIObject mObj(mTmpObjArray[i]);

                    CComVariant vOut;

                    vOut = mObj.toVariant(nameSpace, userName, password);

                    if ((hr = SafeArrayPutElement(pSA, &i, vOut.punkVal)))
                    {
                        SafeArrayDestroy(pSA);
                        throw CIMException(CIM_ERR_FAILED,
                                           "Array put operation failed.");
                    }

                    vOut.Clear();
                }

                var->vt = VT_ARRAY | VT_UNKNOWN;
                var->parray = pSA;
            }
            break;
        case CIM_FLAG_ARRAY:
        case CIM_EMPTY:
        case CIM_ILLEGAL:
        default:
            throw TypeMismatchException();

        break;
    }
}

PEGASUS_NAMESPACE_END
