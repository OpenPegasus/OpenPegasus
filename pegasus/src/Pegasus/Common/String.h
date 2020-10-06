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

#ifndef Pegasus_String_h
#define Pegasus_String_h

#ifdef PEGASUS_OS_HPUX
# ifdef HPUX_IA64_NATIVE_COMPILER
#  include <iostream>
# else
#  include <iostream.h>
# endif
#else
# include <iostream>
#endif
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Char16.h>
#include <Pegasus/Common/Linkage.h>
#include <cstdarg>

PEGASUS_NAMESPACE_BEGIN

class String;
struct StringRep;

/**
    The CString class provides access to an 8-bit String representation.
*/
class PEGASUS_COMMON_LINKAGE CString
{
public:

    /**
        Constructs a CString object with a null string value.
    */
    CString();

    /**
        Constructs an independent copy of a CString object.
        @param cstr The CString instance to copy.
    */
    CString(const CString& cstr);

    /**
        Destructs a CString object.
    */
    ~CString();

    /**
        Copies the value of another CString object.
        @param cstr The CString object from which to copy the value.
        @return A reference to the target CString object with its newly
            assigned value.
    */
    CString& operator=(const CString& cstr);

    /**
        Gets the CString's data as a C string pointer.  IMPORTANT:  The
        returned pointer refers to memory owned by the CString object.  The
        caller must not free this memory.  The returned pointer is valid only
        until the CString object is destructed or reassigned.  Use of this
        operator on a temporary CString object may result in a memory error.
        For example, this usage is invalid:

            const char* cstr = String("Hello").getCString();
            printf(cstr);

        @return Returns a const char pointer to the CString's data.
    */
    operator const char*() const;

private:

    CString(char* cstr);

    friend class String;

    char* _rep;
};

/**
    This class implements the CIM string type.  The intrinsic string format
    is UTF-16, which is a superset of the UCS-2 characters allowed in CIM
    strings.  Facilities are provided for converting to and from UTF-8
    character strings.

    Many of the method interfaces refer to a number of characters.  In all
    cases, these characters are counted as 8- or 16-bit memory chunks rather
    than logical UTF-8 or UTF-16 character chains.
*/
class PEGASUS_COMMON_LINKAGE String
{
public:

    /**
        Represents an empty string.  This value may be used as a convenience
        to avoid construction of an empty String object.
    */
    static const String EMPTY;

    /**
        Constructs an empty String.
    */
    String();

    /**
        Constructs a String with the value of another String.
        @param str The String from which to copy the value.
    */
    String(const String& str);

    /**
        Constructs a String with a specified number of characters of the
        value of another String.
        @param str The String from which to copy the value.
        @param n A Uint32 specifying the number of characters to copy.
        @exception IndexOutOfBoundsException If the specified String does not
            contain the specified number of characters.
        @exception bad_alloc If the construction fails because of a memory
            allocation failure.
    */
    String(const String& str, Uint32 n);

    /**
        Constructs a String with the value from a Char16 buffer.
        @param str The Char16 buffer from which to copy the value.
        @exception NullPointer If the buffer pointer is NULL.
        @exception bad_alloc If the construction fails because of a memory
            allocation failure.
    */
    String(const Char16* str);

    /**
        Constructs a String with a specified number of characters of the
        value from a Char16 buffer.
        @param str The Char16 buffer from which to copy the value.
        @param n A Uint32 specifying the number of characters to copy.
        @exception NullPointer If the buffer pointer is NULL.
        @exception bad_alloc If the construction fails because of a memory
            allocation failure.
    */
    String(const Char16* str, Uint32 n);

    /**
        Constructs a String with the value from a C string in UTF-8 format.
        @param str The C string from which to copy the value.
        @exception NullPointer If the C string pointer is NULL.
        @exception bad_alloc If the construction fails because of a memory
            allocation failure.
        @exception Exception If the C string contains invalid UTF-8.
    */
    String(const char* str);

    /**
        Constructs a String with a specified number of characters of the
        value from a C string in UTF-8 format.
        @param str The C string from which to copy the value.
        @param n A Uint32 specifying the number of characters to copy.
        @exception NullPointer If the C string pointer is NULL.
        @exception bad_alloc If the construction fails because of a memory
            allocation failure.
        @exception Exception If the C string contains invalid UTF-8.
    */
    String(const char* str, Uint32 n);

    /**
        Destructs a String object.
    */
    ~String();

    /**
        Assigns the value of a String to the value of another String.
        @param str The String from which to copy the value.
        @return A reference to the target String object with its newly
            assigned value.
        @exception bad_alloc If the assignment fails because of a memory
            allocation failure.
    */
    String& operator=(const String& str);

    /**
        Assigns the value of a String to the value of another String.
        @param str The String from which to copy the value.
        @return A reference to the target String object with its newly
            assigned value.
        @exception bad_alloc If the assignment fails because of a memory
            allocation failure.
    */
    String& assign(const String& str);

    /**
        Assigns the value of a String to the value in a Char16 buffer.
        @param str The Char16 buffer from which to copy the value.
        @return A reference to the target String object with its newly
            assigned value.
        @exception NullPointer If the buffer pointer is NULL.
        @exception bad_alloc If the assignment fails because of a memory
            allocation failure.
    */
    String& assign(const Char16* str);

    /**
        Assigns the value of a String with a specified number of characters
        of the value from a Char16 buffer.
        @param str The Char16 buffer from which to copy the value.
        @param n A Uint32 specifying the number of characters to copy.
        @return A reference to the target String object with its newly
            assigned value.
        @exception NullPointer If the buffer pointer is NULL.
        @exception bad_alloc If the assignment fails because of a memory
            allocation failure.
    */
    String& assign(const Char16* str, Uint32 n);

    /**
        Assigns the value of a String to the value from a C string in UTF-8
        format.
        @param str The C string from which to copy the value.
        @return A reference to the target String object with its newly
            assigned value.
        @exception NullPointer If the C string pointer is NULL.
        @exception bad_alloc If the assignment fails because of a memory
            allocation failure.
        @exception Exception If the C string contains invalid UTF-8.
    */
    String& assign(const char* str);

    /**
        Assigns the value of a String with a specified number of characters
        of the value from a C string in UTF-8 format.
        @param str The C string from which to copy the value.
        @param n A Uint32 specifying the number of characters to copy.
        @return A reference to the target String object with its newly
            assigned value.
        @exception NullPointer If the C string pointer is NULL.
        @exception bad_alloc If the assignment fails because of a memory
            allocation failure.
        @exception Exception If the C string contains invalid UTF-8.
    */
    String& assign(const char* str, Uint32 n);

    /**
        Sets a String value to the empty String.
    */
    void clear();

    /**
        Reserves memory for a specified number of (16-bit) characters.
        This method does not change the size() of the string or any other
        external behavior.  If the capacity of the string is already greater
        than or equal to the specified size, this method has no effect.  The
        capacity of a String is set only for performance reasons.
        @param capacity A Uint32 specifying the number of characters the
            String should be prepared to hold.
    */
    void reserveCapacity(Uint32 capacity);

    /**
        Returns the number of characters in a String value.  No termination
        character is included in the count.  For example, String("abcd").size()
        returns 4.
    */
    Uint32 size() const;

    /**
        Gets a null-terminated Char16 buffer containing the String value.
        The buffer is valid until the original String object is modified or
        destructed.
        @return A pointer to a null-terminated Char16 buffer containing the
            String value.
    */
    const Char16* getChar16Data() const;

    /**
        Gets a CString object containing the String value in UTF-8 format.
        Important:  A character pointer extracted from a CString object is
        only valid while the CString object exists and is unmodified.  (See
        the CString documentation.)  Thus, in the following example, the
        variable p holds a dangling (invalid) pointer:
        <pre>
              const char * p = (const char *)test.getCString();
        </pre>
        This situation can be corrected by declaring a CString variable in
        the same scope.

        @return A CString object containing the String value in UTF-8 format.
        @exception bad_alloc If the operation fails because of a memory
            allocation failure.
    */
    CString getCString() const;

    /**
        Gets a specified character from the String value.
        @param index Index of the character to access.
        @return The Char16 character at the specified index.
        @exception IndexOutOfBoundsException If the String does not contain a
            character at the specified index.
    */
    Char16& operator[](Uint32 index);

    /**
        Gets a specified character from the String value.
        @param index Index of the character to access.
        @return The Char16 character at the specified index.
        @exception IndexOutOfBoundsException If the String does not contain a
            character at the specified index.
    */
    const Char16 operator[](Uint32 index) const;

    /**
        Appends a character to the String.
        @param c The Char16 character to append.
        @return A reference to the String object containing the newly appended
            character.
        @exception bad_alloc If the append fails because of a memory
            allocation failure.
    */
    String& append(const Char16& c);

    /**
        Appends a specified number of characters to the String from a Char16
        buffer.
        @param str The Char16 buffer from which to append the characters.
        @param n A Uint32 specifying the number of characters to append from
            the buffer.
        @return A reference to the String object containing the newly appended
            characters.
        @exception NullPointer If the buffer pointer is NULL.
        @exception bad_alloc If the append fails because of a memory
            allocation failure.
    */
    String& append(const Char16* str, Uint32 n);

    /**
        Appends a String value to the String.
        @param str The String to append.
        @return A reference to the String object containing the newly appended
            characters.
        @exception bad_alloc If the append fails because of a memory
            allocation failure.
    */
    String& append(const String& str);

    /**
        Removes a specified number of characters from the String starting at a
        given index.  If the number of characters to remove is specified as
        PEG_NOT_FOUND, then all characters from the index to the end of the
        String are removed.
        @param index Uint32 position in String from which to remove characters.
        @param size A Uint32 specifying the number of characters to remove.
            The default value is PEG_NOT_FOUND, which means all characters
            from the index to the end of the String are to be removed.
        @exception IndexOutOfBoundsException If the index plus the size (if not
            PEG_NOT_FOUND) is greater than the number of characters in the
            String.
    */
    void remove(Uint32 index, Uint32 size = PEG_NOT_FOUND);

    /**
        Creates a new String containing up to the specified number of
        characters from the specified index in the String.
        @param index A Uint32 specifying the index at which to copy characters
            into the new String.
        @param n A Uint32 specifying the maximum number of characters to copy
            into the new String.  If the value is PEG_NOT_FOUND or is greater
            than the number of characters from the index to the end of the
            String, the new String contains all characters from the index to
            the end of the String.
        @return A new String containing up to the specified number of
            characters from the specified index in the String.
        @exception bad_alloc If the operation fails because of a memory
            allocation failure.
    */
    String subString(Uint32 index, Uint32 n = PEG_NOT_FOUND) const;

    /**
        Finds the index of the first occurrence of a specified character in
        the String.  If the character is not found, PEG_NOT_FOUND is returned.
        @param c The Char16 value to find in the String.
        @return The Uint32 index of the character in the String if found,
            PEG_NOT_FOUND otherwise.
    */
    Uint32 find(Char16 c) const;

    /**
        Finds the index of the first occurrence of a specified character in
        the String beginning at a specified index.  If the character is not
        found, PEG_NOT_FOUND is returned.
        @param c The Char16 value to find in the String.
        @param index The Uint32 index at which to start the search.
        @return The Uint32 index of the character in the String if found,
            PEG_NOT_FOUND otherwise.
    */
    Uint32 find(Uint32 index, Char16 c) const;

    /**
        Finds the index of the first occurrence of a specified String value in
        the String.  If the String value is not found, PEG_NOT_FOUND is
        returned.
        @param s The String value to find in the String.
        @return The Uint32 index of the beginning of the String value if found,
            PEG_NOT_FOUND otherwise.
    */
    Uint32 find(const String& s) const;

    /**
        Finds the index of the last occurrence of a specified character in
        the String.  If the character is not found, PEG_NOT_FOUND is returned.
        @param c The Char16 value to find in the String.
        @return The Uint32 index of the character in the String if found,
            PEG_NOT_FOUND otherwise.
    */
    Uint32 reverseFind(Char16 c) const;

    /**
        Converts all characters in the String to lower case.
    */
    void toLower();

    /**
        Constructs a String based on printf specifications. For some
        compilers the PEGASUS_FORMAT generates warning messages if
        the format string does not match the input arguments.
        @param format  const char * The format specification as defined
            for printf. The format specification corresponds to the
            standard C++ printf format specification
        @param ... The list of arguments that will be formated.
        @version 2.1.4
    */
    PEGASUS_FORMAT(2,3)
    void appendPrintf(const char* format, ...);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /**
        <I><B>Experimental Interface</B></I><BR>
        Converts all characters in the String to upper case.
    */
    void toUpper();
#endif

    /**
        Compares the first n characters of two String objects.
        @param s1 The first String to compare.
        @param s2 The second String to compare.
        @param n The maximum number of characters to compare.
        @return A negative integer if the first n characters of s1 are
            lexographically less than s2, 0 if the first n characters of s1
            and s2 are equal, and a positive integer otherwise.
    */
    static int compare(const String& s1, const String& s2, Uint32 n);

    /**
        Compares two String objects.  (Note: Use the comparison
        operators < <= > >= to compare String objects.)
        @param s1 The first String to compare.
        @param s2 The second String to compare.
        @return A negative integer if s1 is lexographically less than s2,
            0 if s1 and s2 are equal, and a positive integer otherwise.
    */
    static int compare(const String& s1, const String& s2);

    /**
        Compares two String objects, ignoring case differences.
        @param s1 The first String to compare.
        @param s2 The second String to compare.
        @return A negative integer if s1 is lexographically less than s2,
            0 if s1 and s2 are equal, and a positive integer otherwise.
            (Case differences are ignored in all cases.)
    */
    static int compareNoCase(const String& s1, const String& s2);

    /**
        Compares two String objects for equality.  For example,
        <pre>
            String s1 = "Hello World";
            String s2 = s1;
            assert(String::equal(s1, s2));
        </pre>
        @param s1 The first String to compare.
        @param s2 The second String to compare.
        @return True if the two strings are equal, false otherwise.
    */
    static Boolean equal(const String& s1, const String& s2);

    /**
        Compares two strings and returns true if they are equal independent of
        the case of the characters.
        @param ... Variable arguments as defined for printf
    */
    static Boolean equalNoCase(const String& s1, const String& s2);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

    String(const String& s1, const String& s2);

    String(const String& s1, const char* s2);

    String(const char* s1, const String& s2);

    String& operator=(const char* str);

    Uint32 find(const char* s) const;

    static Boolean equal(const String& s1, const char* s2);

    static int compare(const String& s1, const char* s2);

    String& append(const char* str);

    String& append(const char* str, Uint32 size);

    static Boolean equalNoCase(const String& s1, const char* s2);

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

private:

    StringRep* _rep;
};

/**
    Compares two String objects for equality.
    @param str1 The first String to compare.
    @param str2 The second String to compare.
    @return True if the strings are equal, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const String& str1,
    const String& str2);

/**
    Compares a String and a C string for equality.
    @param str1 The String to compare.
    @param str2 The C string to compare.
    @return True if the strings are equal, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const String& str1, const char* str2);

/**
    Compares a String and a C string for equality.
    @param str1 The C string to compare.
    @param str2 The String to compare.
    @return True if the strings are equal. false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const char* str1, const String& str2);

/**
    Compares two String objects for inequality.
    @param str1 The first String to compare.
    @param str2 The second String to compare.
    @return False if the strings are equal, true otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(
    const String& str1,
    const String& str2);

/**
    Writes a String value to an output stream.  Characters with a zero value or
    with a non-zero high-order byte are written in a hexadecimal encoding.
    @param os The output stream to which the String value is written.
    @param str The String to write to the output stream.
    @return A reference to the output stream.
*/
PEGASUS_COMMON_LINKAGE PEGASUS_STD(ostream)& operator<<(
    PEGASUS_STD(ostream)& os,
    const String& str);

/**
    Concatenates String objects. For example,
    <pre>
        String t1 = "abc";
        String t2;
        t2 = t1 + "def"
        assert(t2 == "abcdef");
    </pre>
    @param str1 The first String to concatenate.
    @param str2 The second String to concatenate.
    @return The concatenated String.
*/
PEGASUS_COMMON_LINKAGE String operator+(const String& str1, const String& str2);

/**
    Compares two String objects.
    @param s1 The first String to compare.
    @param s2 The second String to compare.
    @return True if s1 is lexographically less than s2, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator<(
    const String& str1,
    const String& str2);

/**
    Compares two String objects.
    @param s1 The first String to compare.
    @param s2 The second String to compare.
    @return True if s1 is lexographically less than or equal to s2,
        false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator<=(
    const String& str1,
    const String& str2);

/**
    Compares two String objects.
    @param s1 The first String to compare.
    @param s2 The second String to compare.
    @return True if s1 is lexographically greater than s2, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator>(
    const String& str1,
    const String& str2);

/**
    Compares two String objects.
    @param s1 The first String to compare.
    @param s2 The second String to compare.
    @return True if s1 is lexographically greater than or equal to s2,
        false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator>=(
    const String& str1,
    const String& str2);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_COMMON_LINKAGE Boolean operator==(const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator==(const String& s1, const char* s2);

PEGASUS_COMMON_LINKAGE Boolean operator==(const char* s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator!=(const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator!=(const String& s1, const char* s2);

PEGASUS_COMMON_LINKAGE Boolean operator!=(const char* s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator<(const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator<(const String& s1, const char* s2);

PEGASUS_COMMON_LINKAGE Boolean operator<(const char* s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator>(const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator>(const String& s1, const char* s2);

PEGASUS_COMMON_LINKAGE Boolean operator>(const char* s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator<=(const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator<=(const String& s1, const char* s2);

PEGASUS_COMMON_LINKAGE Boolean operator<=(const char* s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator>=(const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE Boolean operator>=(const String& s1, const char* s2);

PEGASUS_COMMON_LINKAGE Boolean operator>=(const char* s1, const String& s2);

PEGASUS_COMMON_LINKAGE String operator+(const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE String operator+(const String& s1, const char* s2);

PEGASUS_COMMON_LINKAGE String operator+(const char* s1, const String& s2);

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

PEGASUS_NAMESPACE_END

#if defined(PEGASUS_INTERNALONLY)
# include "StringInline.h"
#endif

#endif /* Pegasus_String_h */
