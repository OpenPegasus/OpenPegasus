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

#ifndef Pegasus_InternalException_h
#define Pegasus_InternalException_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_NAMESPACE_BEGIN

/** Class AssertionFailureException
This is an Exception class tied to the definiton of an assert named
PEGASUS_ASSERT.  This assertion can be included at any point in Pegasus
code
*/
class PEGASUS_COMMON_LINKAGE AssertionFailureException : public Exception
{
public:

    AssertionFailureException(
        const char* file,
        size_t line,
        const String& message);

    virtual ~AssertionFailureException();
};


/* Macro to Create the equivalent of an assert but without the
   termination.  This can be used as a temporary marker for asserts
   that are not working.  Prints out the error but continues.
   NOTE: This is useful in test programs to keep us aware that we
   have problems without halting the test sequence.
   This was created primarily to put temporary asserts into tests that
   are not yet working correctly but will not stop the test sequence.
*/
#define ASSERTTEMP(COND) \
    do \
    { \
        if (!(COND)) \
        { \
            cerr << "TEMP Assert Error TEMP **********" \
                <<__FILE__ << " " << __LINE__ \
                << " " << #COND << endl; \
        } \
    } while (0)


// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE NullPointer : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    NullPointer();

    virtual ~NullPointer();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE UndeclaredQualifier : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    UndeclaredQualifier(const String& qualifierName) ;

    virtual ~UndeclaredQualifier();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE BadQualifierScope : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    BadQualifierScope(
        const String& qualifierName,
        const String& scopeString);

    virtual ~BadQualifierScope();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE BadQualifierOverride : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    BadQualifierOverride(const String& qualifierName);

    virtual ~BadQualifierOverride();
};

class PEGASUS_COMMON_LINKAGE BadQualifierType : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];
    BadQualifierType(const String& qualifierName,
        const String& className = String());
    virtual ~BadQualifierType();
    const String _qualifierName;
    const String _className;
    const String& getQualifierName() const;
    const String& getClassName() const;

};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE InstantiatedAbstractClass : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    InstantiatedAbstractClass(const String& className);
    virtual ~InstantiatedAbstractClass();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE NoSuchProperty : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    NoSuchProperty(const String& propertyName);
    virtual ~NoSuchProperty();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE NoSuchFile : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    NoSuchFile(const String& fileName);
    virtual ~NoSuchFile();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE FileNotReadable : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    FileNotReadable(const String& fileName);
    virtual ~FileNotReadable();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE CannotRemoveDirectory : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    CannotRemoveDirectory(const String& path);
    virtual ~CannotRemoveDirectory();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE CannotRemoveFile : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    CannotRemoveFile(const String& path);
    virtual ~CannotRemoveFile();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE CannotRenameFile : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    CannotRenameFile(const String& path);
    virtual ~CannotRenameFile();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE NoSuchDirectory : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    NoSuchDirectory(const String& directoryName);
    virtual ~NoSuchDirectory();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE CannotCreateDirectory : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    CannotCreateDirectory(const String& path);
    virtual ~CannotCreateDirectory();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE CannotOpenFile : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    CannotOpenFile(const String& path);
    virtual ~CannotOpenFile();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE CannotChangeFilePerm : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    CannotChangeFilePerm(const String& path);
    virtual ~CannotChangeFilePerm();
};

// ATTN: P3  KS documentation Required
class PEGASUS_COMMON_LINKAGE NotImplemented : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    NotImplemented(const String& method);
    virtual ~NotImplemented();
};

class PEGASUS_COMMON_LINKAGE StackUnderflow : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    StackUnderflow();
    virtual ~StackUnderflow();
};

class PEGASUS_COMMON_LINKAGE StackOverflow : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    StackOverflow();
    virtual ~StackOverflow();
};

class PEGASUS_COMMON_LINKAGE DynamicLoadFailed : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    DynamicLoadFailed(const String& libraryName);
    virtual ~DynamicLoadFailed();
};

class PEGASUS_COMMON_LINKAGE DynamicLookupFailed : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    DynamicLookupFailed(const String& symbolName);
    virtual ~DynamicLookupFailed();
};

class PEGASUS_COMMON_LINKAGE CannotOpenDirectory : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    CannotOpenDirectory(const String& path);
    virtual ~CannotOpenDirectory();
};

class PEGASUS_COMMON_LINKAGE ParseError : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    ParseError(const String& message);
    virtual ~ParseError();
};

class PEGASUS_COMMON_LINKAGE MissingNullTerminator : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    MissingNullTerminator();
    virtual ~MissingNullTerminator();
};

class PEGASUS_COMMON_LINKAGE MalformedLanguageHeader: public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    MalformedLanguageHeader(const String& error);
    virtual ~MalformedLanguageHeader();
};

class PEGASUS_COMMON_LINKAGE InvalidAcceptLanguageHeader: public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    InvalidAcceptLanguageHeader(const String& error);
    virtual ~InvalidAcceptLanguageHeader();
};

class PEGASUS_COMMON_LINKAGE InvalidContentLanguageHeader: public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    InvalidContentLanguageHeader(const String& error);
    virtual ~InvalidContentLanguageHeader();
};

class PEGASUS_COMMON_LINKAGE InvalidAuthHeader: public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    InvalidAuthHeader();
    virtual ~InvalidAuthHeader();
};

class PEGASUS_COMMON_LINKAGE UnauthorizedAccess: public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    UnauthorizedAccess();
    virtual ~UnauthorizedAccess();
};

class PEGASUS_COMMON_LINKAGE IncompatibleTypesException : public Exception
{
public:
    IncompatibleTypesException();
    virtual ~IncompatibleTypesException();
};

class PEGASUS_COMMON_LINKAGE InternalSystemError : public Exception
{
public:

    static const char MSG[];

    InternalSystemError();
    virtual ~InternalSystemError();
};

class PEGASUS_COMMON_LINKAGE SocketWriteError : public Exception
{
public:

    static const char MSG[];
    static const char KEY[];

    SocketWriteError(const String& error);
    virtual ~SocketWriteError();
};

class PEGASUS_COMMON_LINKAGE TooManyHTTPHeadersException : public Exception
{
public:
    TooManyHTTPHeadersException();
    virtual ~TooManyHTTPHeadersException();
};

class PEGASUS_COMMON_LINKAGE TooManyElementsException : public Exception
{
public:
    static const char MSG[];
    static const char KEY[];
    
    TooManyElementsException();
    virtual ~TooManyElementsException();
};
// function used to throw TooManyElementsException for use in inlined functions
// to reduce static code size
PEGASUS_COMMON_LINKAGE extern void ThrowTooManyElementsException();

// function used to throw IndexOutOfBoundsException for use in inlined functions
// to reduce static code size
PEGASUS_COMMON_LINKAGE extern void ThrowIndexOutOfBoundsException();


/** The CIMException defines the CIM exceptions that are formally defined in
    the CIM Operations over HTTP specification.  TraceableCIMException allows
    file name and line number information to be added for diagnostic purposes.
*/
class PEGASUS_COMMON_LINKAGE TraceableCIMException : public CIMException
{
public:

    TraceableCIMException(
        CIMStatusCode code,
        const String& message,
        const String& file,
        Uint32 line);

    TraceableCIMException(
        CIMStatusCode code,
        const MessageLoaderParms& parms,
        const String& file,
        Uint32 line);

    TraceableCIMException(
        const ContentLanguageList& langs,
        CIMStatusCode code,
        const String& message,
        const String& file,
        Uint32 line);

    TraceableCIMException(const CIMException & cimException);

    String getDescription() const;

    String getTraceDescription() const;

    String getCIMMessage() const;
    void setCIMMessage(const String& cimMessage);

    String getFile() const;
    Uint32 getLine() const;
    const ContentLanguageList& getContentLanguages() const;
};

#define PEGASUS_CIM_EXCEPTION(CODE, EXTRA_MESSAGE) \
    TraceableCIMException(CODE, EXTRA_MESSAGE, String(__FILE__), __LINE__)

#define PEGASUS_CIM_EXCEPTION_LANG(LANGS, CODE, EXTRA_MESSAGE) \
    TraceableCIMException( \
        LANGS, CODE, EXTRA_MESSAGE, String(__FILE__), __LINE__)

#define PEGASUS_CIM_EXCEPTION_L(CODE, MSG_PARMS) \
    TraceableCIMException(CODE, MSG_PARMS, String(__FILE__), __LINE__)

/** Macro to prevent overflow of a signed int value when said value is
 *  doubled. If String/Array/Buffer size is greater than 0x3FFFFFFF, then
 *  something is suspect, throw bad_alloc exception.
 */
#define PEGASUS_CHECK_CAPACITY_OVERFLOW(capacity) \
    do \
    { \
        if (capacity > 0x3FFFFFFF) \
            throw PEGASUS_STD(bad_alloc)(); \
    } \
    while (0)

/** The following function CheckRep serves to reduce the function call
 *  overhead caused by calling a non-inlined function that only checks
 *  if a pointer is not NULL.
 *  The function ThrowUnitializedObjectException throws
 *  an UninitialisedObjectException.
 *  A function to throw the exception is used to reduce object size of
 *  function CheckRep which is inlined in many places across
 *  the common library.
 */
PEGASUS_COMMON_LINKAGE extern void ThrowUninitializedObjectException();

PEGASUS_COMMON_LINKAGE extern void ThrowCannotOpenFileException(
    const char* path);

inline void CheckRep(void * rep)
{
    if (!rep)
        ThrowUninitializedObjectException();
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_InternalException_h */
