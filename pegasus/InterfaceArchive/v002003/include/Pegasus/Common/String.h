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
/* NOCHKSRC */

//
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Karl Schopmeyer(k.schopmeyer@opengroup.org)
//              Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
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

const char STRING_FLAG_ASCII[] = "ASCII";
const char STRING_FLAG_UTF8[]  = "UTF8";   

PEGASUS_NAMESPACE_BEGIN

class String;
class StringRep;

/** The CString class provides access to an 8-bit String representation.
*/
class PEGASUS_COMMON_LINKAGE CString
{
public:

    /** Constructs a CString object with null values (default constructor).
    */
    CString();

    /** REVIEWERS: Describe method here.
    @param cstr Specifies the name of the CString instance.
    */
    CString(const CString& cstr);

    /** CString destructor.
    */
    ~CString();

    /** Assigns the values of one CString instance to another.
    @param cstr Specifies the name of the CString instance whose values 
    are assigned to CString.
    */
    CString& operator=(const CString& cstr);

    /** REVIEWERS: Describe constructor here.
    */
    operator const char*() const;

private:

    CString(char* cstr);

    friend class String;

    void* _rep;
};

/**
    The Pegasus String C++ Class implements the CIM string type.
    REVIEWERS: We need more definition here.
*/
class PEGASUS_COMMON_LINKAGE String
{
public:

    /**	This member is used to represent an empty string. Using this 
        member avoids construction of an empty string (for example, String()).
    */
    static const String EMPTY;

    /** Default constructor without parameters. This constructor creates a
	null string. For example, 
	<pre>
	    String test;
	</pre>
    */
    String();

    /** Copy constructor.
    @param str Specifies the name of the String instance.
    */
    String(const String& str);

    /** Initialize with first n characters from str.
    @param str Specifies the name of the String instance.
    @param n Specifies the Uint32 size to use for the length of the string object.
    */
    String(const String& str, Uint32 n);

    /** Initialize with str.
    @param str Specifies the name of the String instance.
    */
    String(const Char16* str);

    /** Initialize with first n characters of str.
    @param str Specifies the name of the String instance.
    @param n Specifies the Uint32 size.
    */
    String(const Char16* str, Uint32 n);

    /** Initialize from a plain C-String:
    @param str Specifies the name of the String instance.
    */
    String(const char* str);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** Initialize from a plain C-String that allows UTF-8:
    @param str Specifies the name of the String instance.
    @param utfFlag Specifies the name of the character constructor.
    */
    String(const char* str, const char* utfFlag);
#endif

    /** Initialize from the first n characters of a plain C-String:
    @param str Specifies the name of the String instance.
    @param u Specifies the Uint32 size.
    */
    String(const char* str, Uint32 n);

    /** String destructor. 
    */
    ~String();

    /** Assign this string with str. For example,
	<pre>
	    String t1 = "abc";
	    String t2 = t1;
	</pre>
	String t2 is assigned the value of t1.
        @param str Specifies the name of the String to assign to another 
        String instance.
    */
    String& operator=(const String& str);

    /** Assign this string with String str.
        @param str String to assign.
        @return Returns the String.
    */
    String& assign(const String& str);

    /** Assign this string with str.
    */
    String& assign(const Char16* str);

    /** Assign this string with first n characters of str.
    @param n REVIEWERS: Insert text here.
    @param str REVIEWERS: Insert text here.
    */
    String& assign(const Char16* str, Uint32 n);

    /** Assign this string with the plain C-String str.
    @param str REVIEWERS: Insert text here.
    */
    String& assign(const char* str);

    /** Assign this string with first n characters of the plain C-String str.
    @param str REVIEWERS: Insert text here.
    @param n REVIEWERS: Insert text here.
    */
    String& assign(const char* str, Uint32 n);

    /** Clear this string. After calling clear(), size() will return 0.
	<pre>
	    String test = "abc";
	    test.clear();
	</pre>
    */
    void clear();


    /** Reserves memory for capacity characters. Notice
        that this does not change the size of the string (size() returns
        what it did before).  If the capacity of the string is already
        greater or equal to the capacity argument, this method has no
        effect.  The capacity of a String object has no bearing on its
        external behavior.  The capacity of a String is set only for
        performance reasons.
	@param capacity Defines the capacity in characters to reserve.
    */
    void reserveCapacity(Uint32 capacity);

    /** Returns the length of the String object.
	@return Length of the String in characters. For example, 
	<pre>
	    String s = "abcd";
	    assert(s.size() == 4);
	</pre>
        returns a value of 4 for the length.
    */
    Uint32 size() const;

    /** Returns a pointer to the first character in the 
	null-terminated Char16 buffer of the String object.
	@return	Pointer to the first character of the String object. For example, 
    	<pre>
	    String test = "abc";
	    const Char16* q = test.getChar16Data();
	</pre>
        points to the first character in the String instance named test.
    */
    const Char16* getChar16Data() const;

    /** Create an 8-bit representation of this String object. For example,

        @return CString object that provides access to the 8-bit String
        representation.

	<pre>
	    String test = "abc";
            printf("test = %s\n", (const char*)test.getCString());

            NOTE:  Do not do the following:
            const char * p = (const char *)test.getCString();
            The pointer p will be invalid.  This is because
            the CString object is destructed, which deletes
            the heap space for p.
	</pre>
    */
    CString getCString() const;

    /** Returns the specified character of the String object.
	@param index Index of the character to access.
        @return Specified character of the String object.
	@exception IndexOutOfBoundsException If the index
	is outside the bounds of the String.
	<pre>
	    String test = "abc;
	    Char16 c = test[1];
	</pre>
    */
    Char16& operator[](Uint32 index);

    /** Returns the specified character of the String object (const version).
	@param index Index of the character to access.
        @return Specified character of the String object.
	@exception IndexOutOfBoundsException If the index
	is outside the bounds of the String.
    */
    const Char16 operator[](Uint32 index) const;

    /** Append the given character to this String.
	@param c Character to append.
	@return This String.
	<pre>
	    String test = "abc";
	    test.append(Char16('d'));
	    assert(test == "abcd");
	</pre>
    */
    String& append(const Char16& c);

    /** Append n characters from str to this String.
    @param str REVIEWERS: Insert text here.
    @param n REVIEWERS: Insert text here.
    */
    String& append(const Char16* str, Uint32 n);

    /** Append the given String to this String.
	@param str String to append.
	@return This String.
	<pre>
	String test = "abc";
	test.append("def");
	assert(test == "abcdef");
	</pre>
    */
    String& append(const String& str);

    /** Remove size characters from the string starting at the given
	index. If size is PEG_NOT_FOUND, then all characters after index are
	removed.
	@param index Position in string to start remove.
	@param size Number of characters to remove. Default is PEG_NOT_FOUND
	which causes all characters after index to be removed.
	<pre>
	    String s;
	    s = "abc";
	    s.remove(0, 1);
	    assert(String::equal(s, "bc"));
	    assert(s.size() == 2);
	    s.remove(0);
	    assert(String::equal(s, ""));
	    assert(s.size() == 0);
	</pre>
	@exception IndexOutOfBoundsException If size is greater than
	length of String plus starting index for remove.
    */
    void remove(Uint32 index, Uint32 size = PEG_NOT_FOUND);

    /** Return a new String which is initialzed with <TT>length</TT>
	characters from this string starting at <TT>index</TT>.
	@param index Specifies the index in string to start getting the
	substring.
	@param length Specifies the number of characters to get. If length
	is PEG_NOT_FOUND, then all characters after index are added to the new
	string.
	@return String Specifies the Sting with the defined substring.
    */
    String subString(Uint32 index, Uint32 length = PEG_NOT_FOUND) const;

    /** Find the index of the first occurrence of the character c.
	If the character is not found, PEG_NOT_FOUND is returned.
	@param c Char to be found in the String.
	@return Position of the character in the string or PEG_NOT_FOUND if not
	found.
    */
    Uint32 find(Char16 c) const;

    /** Find the index of the first occurence of the character c.
	If the character is not found, PEG_NOT_FOUND is returned.
        This begins searching from the given index. 
	@param c Char to be found in the String.
	@return Position of the character in the string or PEG_NOT_FOUND if not
	found.
    */
    Uint32 find(Uint32 index, Char16 c) const;

    /** Find the index of the first occurence of the string object.
	This function finds one string inside another.
	If the matching substring is not found, PEG_NOT_FOUND is returned.
	@param s String object to be found in the String.
	@return Position of the substring in the String or PEG_NOT_FOUND if not
	found.
    */
    Uint32 find(const String& s) const;

    /** Same as find() but start looking in reverse (last character first).
    	@param c Char16 character to find in String.
	@return Position of the character in the string or PEG_NOT_FOUND if not
	found.
    */
    Uint32 reverseFind(Char16 c) const;

    /** Converts all characters in this string to lowercase characters.
    */
    void toLower();

    /** Compare the first n characters of the two strings.
    	@param s1 First null-terminated string for the comparison.
	@param s2 Second null-terminated string for the comparison.
	@param n Number of characters to compare.
	@return Return -1 If s1 is lexographically less than s2; if they are
	equivalent return 0; otherwise return 1.
    */
    static int compare(const String& s1, const String& s2, Uint32 n);

    /** Compare two null-terminated strings.
    	@param s1 First null-terminated string for the comparison.
	@param s2 Second null-terminated string for the comparison.
	@return Return -1 if s1 is less than s2; if equal return 0;
	otherwise return 1.

	NOTE: Use the comparison operators <,<= > >= to compare
	String objects.
    */
    static int compare(const String& s1, const String& s2);

    /** Compare two null-terminated strings but ignore case.
        @param s1 First null-terminated string for the comparison.
	@param s2 Second null-terminated string for the comparison.
	@return Return -1 if s1 is less than s2; if equal return 0;
	otherwise return 1.

	NOTE: Use the comparison operators <,<= > >= to compare
	String objects.
    */
    static int compareNoCase(const String& s1, const String& s2);

    /** Compare two String objects for equality.
	@param s1 First <TT>String</TT> for comparison.
	@param s2 Second <TT>String</TT> for comparison.

	@return true If the two strings are equal; otherwise, false. For example, 
	<pre>
	    String s1 = "Hello World";
	    String s2 = s1;
	    String s3(s2);
	    assert(String::equal(s1, s3));
	</pre>
    */
    static Boolean equal(const String& str1, const String& str2);

    /** Compares two strings and returns true if they
	are equal indepedent of case of the characters.
	@param str1 First String parameter.
	@param str2 Second String parameter.
	@return true If strings are equal independent of case, flase
        otherwise.
    */
    static Boolean equalNoCase(const String& str1, const String& str2);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    // UTF8 specific code:

    /** Assign this string with a C string that may contain UTF-8.
	@param str The C string
    */
    String& assignUTF8(const char* str);

    /** Create an 8-bit UTF-8 representation of this String object.
        @return CString object that provides access to the 8-bit UTF-8 String
        representation.
    */
    CString getCStringUTF8() const;

    /** Tests whether a C string contains valid UTF-8 characters.
	@param str The C string
    */
    static Boolean isUTF8(const char*);
#endif

private:

    StringRep* _rep;
};

/** String operator == tests for equality between two strings of any of the
    types String or char*.
    @return true If the strings are equal; otherwise, false.
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const String& str1,
    const String& str2);

/** String operator ==. Test for equality between two strings.
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const String& str1, const char* str2);

/** String operator ==. Test for equality between two strings.
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const char* str1, const String& str2);

/** String operator ==. Test for equality between two strings.
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(
    const String& str1,
    const String& str2);

/** REVIEWERS: Insert description here.
    @param str REVIEWERS: Insert description here.
    @param os REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE PEGASUS_STD(ostream)& operator<<(
    PEGASUS_STD(ostream)& os,
    const String& str);

/** This overload operator (+) concatenates String objects. For example, 
    <pre>
	String t1 = "abc";
	String t2;
	t2 = t1 + "def"
	assert(t2 == "abcdef");
    </pre>
*/
PEGASUS_COMMON_LINKAGE String operator+(const String& str1, const String& str2);

/** The overload operator (<) compares String obects.
    <pre>
	String t1 = "def";
	String t2 = "a";
	assert (t2 < t1);
    </pre>
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator<(
    const String& str1,
    const String& str2);

/** The overload operator (<=) compares String objects.
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator<=(
    const String& str1,
    const String& str2);

/** The overload operator (>) compares String objects.
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator>(
    const String& str1,
    const String& str2);

/** The overload operator (>=) compares String objects.
    @param str1 REVIEWERS: Insert description here.
    @param str2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE Boolean operator>=(
    const String& str1,
    const String& str2);

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
/** Compares two strings but ignores any case differences.
    @param s1 REVIEWERS: Insert description here.
    @param s2 REVIEWERS: Insert description here.
*/
PEGASUS_COMMON_LINKAGE int CompareNoCase(const char* s1, const char* s2);
#endif

PEGASUS_NAMESPACE_END

#endif /* Pegasus_String_h */
