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

#ifndef __MYSTRING_H_
#define __MYSTRING_H_

#ifdef _UNICODE
#define CHAR_FUDGE 1    // one TCHAR unused is good enough
#else
#define CHAR_FUDGE 2    // two BYTES unused for case of DBC last char
#endif

// general purpose defines - may be modified later
#define BUF_MAX            256

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WMIPROVIDER_LINKAGE CMyString
{
public:

// Constructors
    CMyString( );
    CMyString(const CMyString& strSrc);
    CMyString(LPCWSTR lpsz);
    CMyString(LPCSTR lpsz);
    CMyString(const unsigned char* psz);
    CMyString(String sStr);

    BSTR Bstr();
    BOOL Compare(BSTR bStr);
    BOOL Compare(LPCTSTR szStr);

// Attributes & Operations
    // as an array of characters
    int GetLength() const{return m_nStrLen;}
    BOOL IsEmpty() const {return m_nBufLen == 0;}
    int GetBufLength() const {return m_nBufLen;}
    void Empty(){DeallocBuffer();}        // free up the buffer
    //format data buffer in hex
    void GetPrintableHex(int len, const unsigned char* data);
    int Find(TCHAR ch) const;
    int Find(TCHAR ch, int nStart) const;


    LPTSTR Copy();

    operator LPCTSTR() const;

    // overloaded assignment
    const CMyString& operator=(const CMyString& stringSrc);
    const CMyString& operator=(LPCWSTR lpsz);
    const CMyString& operator=(LPCSTR lpsz);
    const CMyString& operator=(const unsigned char* psz);
    const CMyString& operator=(BSTR bStr);
    const CMyString& operator=(String sStr);

    // string concatenation
    const CMyString& operator+=(const CMyString& stringSrc);
    const CMyString& operator+=(LPCTSTR lpsz);
    const CMyString& operator+=(String sStr);

// Implementation
public:
//    BOOL LoadString(UINT nID);
    int Format(LPCTSTR lpszFormat, int iSize, ...);
//    int Format(UINT nID, int iSize, ...);

    ~CMyString();
    HRESULT ErrorStatus(){return m_nStatus;}

protected:
    void Init();
    BOOL AllocBuffer(int nLen);
    void AllocBeforeWrite(int nLen);
    void AssignCopy( int nLen, LPCTSTR lpsz);
    void DeallocBuffer();
    void ConcatCopy(int nLen1, LPCTSTR lpszSrc1, int nLen2, LPCTSTR lpszSrc2);
    void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrc);
    int FormatV(LPCTSTR lpszFormat, int iSize, va_list argList);

    //    lengths/sizes in characters
    //  (note: an extra character is always allocated)
    //    (note: the buffer storage is always type TCHAR)
    LPTSTR          m_pszData;      // actual string (zero terminated)
    int             m_nStrLen;      // does not include terminating 0
    int                m_nBufLen;        // length of buffer in TCHARs
    HRESULT            m_nStatus;
};

PEGASUS_NAMESPACE_END

#endif // __MYSTRING_H_

