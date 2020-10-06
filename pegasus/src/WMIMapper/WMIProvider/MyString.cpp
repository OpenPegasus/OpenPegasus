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
// Author: Barbara Packard (barbara_packard@hp.com)
//
// Modified By: Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Pegasus/Common/Config.h>
#include "MyString.h"

PEGASUS_NAMESPACE_BEGIN

/////////////////////////////////////////////////////////////////////////////
// CMyString

void CMyString::Init()
{
    m_pszData = NULL;
    m_nStrLen = 0;
    m_nBufLen = 0;
    m_nStatus = S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

CMyString::CMyString()
{
    Init();
}

CMyString::CMyString(const CMyString& strSrc)
{
    Init();
    *this = strSrc.m_pszData;
}

CMyString::CMyString(const unsigned char* psz)
{
    Init();
    *this = (LPCSTR)psz;
}

CMyString::CMyString(LPCTSTR psz)
{
    Init();
    *this = psz;
}

CMyString::CMyString(String sStr)
{
    Init();
    CString str = sStr.getCString();
    *this = (const char *)str;
}

#ifdef _UNICODE
CMyString::CMyString(LPCSTR lpsz)
{
    Init();

    int nLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpsz, -1,
        NULL, 0);

    AllocBuffer(nLen);

    ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpsz, -1,
        m_pszData, m_nBufLen);
    m_nStrLen = _tcslen(m_pszData);
}
#endif

CMyString::~CMyString()
{
    //free any pointers
    DeallocBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// Helpers

BOOL CMyString::AllocBuffer(int nLen)
{
    // always allocate one extra character for '\0' termination
    if (m_nBufLen > nLen)
    {
        return TRUE;
    }

    Init();

    if (nLen > 0)
    {
        m_pszData = (LPTSTR)new BYTE[(nLen + 1) * sizeof(TCHAR)];
    }

    if (m_pszData)
    {
        m_nBufLen = nLen + 1;
        memset((void *)m_pszData, '\0', (nLen + 1) * sizeof(TCHAR));
    }
    else
    {
        m_nStatus = E_OUTOFMEMORY;
        return FALSE;
    }

    return TRUE;
}

void CMyString::DeallocBuffer()
{

    if (m_pszData)
    {
        delete [] m_pszData;
    }

    Init();
}

void CMyString::AllocBeforeWrite(int nLen)
{

    if (m_pszData && (m_nBufLen > nLen))
    {
        memset((void *)m_pszData, '\0', m_nBufLen);
        return;
    }

    else
    {
        DeallocBuffer();
        AllocBuffer(nLen);
    }
}

void CMyString::AssignCopy( int nLen, LPCTSTR lpsz)
{
    AllocBeforeWrite(nLen);

    if (m_pszData)
    {
        memcpy(m_pszData, lpsz, nLen * sizeof(TCHAR));
        m_pszData[nLen] = '\0';
        m_nStrLen = nLen;
    }
}


/////////////////////////////////////////////////////////////////////////////
// Overloaded operators

const CMyString& CMyString::operator=(const CMyString& stringSrc)
{
    *this = stringSrc.m_pszData;
    return *this;
}

#ifdef _UNICODE
const CMyString& CMyString::operator=(LPCSTR lpsz)
{
    int nLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpsz,
        -1, NULL, 0);

    AllocBeforeWrite(nLen);

    if (m_pszData)
    {
        ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpsz, -1,
            m_pszData, m_nBufLen);
        m_nStrLen = _tcslen(m_pszData);
    }

    return *this;
}
#else
const CMyString& CMyString::operator=(LPCWSTR lpsz)
{
    if (lpsz && *lpsz)
    {
        int nLen = ::WideCharToMultiByte(CP_UTF8, 0, lpsz, -1,
            NULL, 0, NULL, NULL);

        AllocBeforeWrite(nLen);

        if (m_pszData)
        {
            ::WideCharToMultiByte(CP_UTF8, 0, lpsz, -1,
                m_pszData, m_nBufLen, NULL, NULL);
            m_nStrLen = _tcslen(m_pszData);
        }
    }
    else
    {
        Empty();
    }

    return *this;
}
#endif

//By Jair - here
const CMyString& CMyString::operator=(BSTR bStr)
{
    _bstr_t bstr(bStr, TRUE);
    *this = (LPCWSTR)bstr;
    return *this;
}

const CMyString& CMyString::operator=(LPCTSTR lpsz)
{
    int nLen = (lpsz != NULL) ? _tcslen(lpsz) : 0;
    AssignCopy( nLen, lpsz);
    return *this;
}

const CMyString& CMyString::operator=(const unsigned char* psz)
{
    *this = (LPCSTR)psz;
    return *this;
}

const CMyString& CMyString::operator=(String sStr)
{
    CString str = sStr.getCString();
    *this = (const char *)str;
    return *this;
}

CMyString::operator LPCTSTR() const
{
    return m_pszData;
}

/////////////////////////////////////////////////////////////////////////////
// string concatenation
void CMyString::ConcatCopy(    int nLen1, LPCTSTR lpszSrc1,
                            int nLen2, LPCTSTR lpszSrc2)
{
    // concatenate two sources
    // assumes 'this' is a new CMyString
    int nLen = nLen1 + nLen2;

    if (nLen != 0)
    {
        AllocBuffer(nLen);
        memcpy(m_pszData, lpszSrc1, nLen1 * sizeof(TCHAR));
        memcpy(m_pszData + nLen1, lpszSrc2, nLen2 * sizeof(TCHAR));
        m_nStrLen = nLen;
    }
}

void CMyString::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrc)
{
    if (nSrcLen == 0)
        return;

    int nLen = m_nStrLen + nSrcLen;

    if (nLen < m_nBufLen)
    {
        // have enough room, just add it
        memcpy(m_pszData + m_nStrLen, lpszSrc, nSrcLen * sizeof(TCHAR));
        m_nStrLen = nLen;
        m_pszData[nLen] = '\0';
    }
    else
    {
        // buffer is too small - reallocate.
        LPTSTR pszOld = m_pszData;
        ConcatCopy(m_nStrLen, m_pszData, nSrcLen, lpszSrc);

        if (pszOld != NULL)
            delete [] pszOld;
    }
}

const CMyString& CMyString::operator+=(const CMyString& stringSrc)
{
    ConcatInPlace(stringSrc.m_nStrLen, stringSrc.m_pszData);
    return *this;
}

#ifdef _UNICODE
const CMyString& CMyString::operator+=(LPCTSTR lpsz)
{

    if (lpsz != NULL)
    {
        ConcatInPlace(_tcslen(lpsz), lpsz);
    }
    return *this;
}
#else    //!_UNICODE
const CMyString& CMyString::operator+=(LPCTSTR lpsz)
{
    if (lpsz != NULL)
    {
        CMyString myStr = lpsz;
        ConcatInPlace(_tcslen(lpsz), myStr.m_pszData);
    }
    return *this;
}
#endif

const CMyString& CMyString::operator+=(String sStr)
{
    if (sStr.size() != 0)
    {
        CMyString myStr = sStr;
        ConcatInPlace(_tcslen((LPCTSTR)myStr), myStr.m_pszData);
    }
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Operations
LPTSTR CMyString::Copy()
{
    LPTSTR lpsz = new TCHAR[m_nStrLen+1];

    if (lpsz)
    {
        _tcscpy(lpsz, m_pszData);
    }
    else
    {
        m_nStatus = (m_nStatus == S_OK) ? E_OUTOFMEMORY : m_nStatus;
    }

    return lpsz;
}

//By Jair - here
BSTR CMyString::Bstr()
{
    CComBSTR cb = (LPCSTR)m_pszData;
    return cb.Copy();
}

BOOL CMyString::Compare(BSTR bStr)
{
    _bstr_t sText1(m_pszData);
    _bstr_t sText2(bStr);
    return (sText1 == sText2);
}

BOOL CMyString::Compare(LPCTSTR szStr)
{
    _bstr_t sText1(m_pszData);
    _bstr_t sText2(szStr);
    return (sText1 == sText2);
}

/////////////////////////////////////////////////////////////////////////////
// Search for a character
//    lifted from CString
//        returns 0 based index of first instance else -1
//
int CMyString::Find(TCHAR ch) const
{
    return Find(ch, 0);
}

int CMyString::Find(TCHAR ch, int nStart) const
{
    if (nStart >= m_nStrLen)
    {
        return -1;
    }

    // find first single character
    LPTSTR lpsz = _tcschr(m_pszData + nStart, (_TUCHAR)ch);

    return (NULL == lpsz) ? -1 : (int)(lpsz - m_pszData);
}

/////////////////////////////////////////////////////////////////////////////
//    method to format data buffer in printable hex format "xx xx xx "
//    lifted from snmp++
//    len - number of bytes to format

void CMyString::GetPrintableHex(int len, const unsigned char* data)
{
    //2 chars for each byte plus space between
    LPTSTR lpsz = new TCHAR[3 * len + 1];
    LPTSTR ptr = lpsz;
    const unsigned char* bytes = data;
    int i;

    if (lpsz)
    {
        for (i = 0; i < len; i++)
        {
            _stprintf(ptr, _T("%2.2X "), *bytes);
            bytes++;
            ptr += 3;
        }
        *ptr = '\0';
    }
    else
    {
        m_nStatus = (m_nStatus == S_OK) ? E_OUTOFMEMORY : m_nStatus;
    }

    if (lpsz)
    {
        *this = lpsz;
        delete[] lpsz;
    }
}


/////////////////////////////////////////////////////////////////////////////
//    methods to format data buffer as in sprintf.
//        NOTE:  iSize is size of output buffer in TCHAR format. Only iSize
//        characters will be copied.
//        uses _vsntprintf
//        returns number of characters in formatted string
//    protected method which calls _vsntprintf

int CMyString::FormatV(LPCTSTR lpszFormat, int iSize, va_list argList)
{
    int        iNumChars, iLen;
    LPTSTR    lpBuf = NULL;

    iLen = iSize - CHAR_FUDGE;

    lpBuf = new TCHAR[iSize];

    iNumChars = _vsntprintf(lpBuf, iLen, lpszFormat, argList);

    if (0 <= iNumChars)
    {
        *this = lpBuf;
    }

    delete [] lpBuf;

    return iNumChars;
}

// uses sprintf format
int CMyString::Format(LPCTSTR lpszFormat, int iSize, ...)
{
    int iNumChars;
    va_list    argList;

    va_start(argList, iSize);
    iNumChars = FormatV(lpszFormat, iSize, argList);
    va_end(argList);

    return iNumChars;
}

/*
// uses resource ID
int CMyString::Format(UINT nID, int iSize, ...)
{
    int iNumChars;
    CMyString s;
    va_list argList;

    s.LoadString(nID);

    va_start(argList, iSize);
    iNumChars = FormatV(s, iSize, argList);
    va_end(argList);

    return iNumChars;
}
*/

PEGASUS_NAMESPACE_END

