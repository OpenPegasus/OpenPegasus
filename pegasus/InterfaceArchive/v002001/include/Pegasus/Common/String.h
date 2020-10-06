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

PEGASUS_NAMESPACE_BEGIN

class String;
class StringRep;

/** The CString class provides access to an 8-bit String representation.
*/
class PEGASUS_COMMON_LINKAGE CString
{
public:

    ///
    CString();

    ///
    CString(const CString& cstr);

    ///
    ~CString();

    ///
    CString& operator=(const CString& cstr);

    operator const char*() const;

private:

    CString(char* cstr);

    friend class String;

    void* _rep;
};

/**
    The Pegasus String C++ Class implements the CIM string type.
*/
class PEGASUS_COMMON_LINKAGE String
{
public:

    /**	This member is used to represent an empty string. Using this 
        member avoids construction of an empty string (e.g., String()).
    */
    static const String EMPTY;

    /** Default constructor without parameters. This constructor creates a
	null string.
	<pre>
	    String test;
	</pre>
    */
    String();

    /// Copy constructor.
    String(const String& str);

    /// Initialize with first n characters from str.
    String(const String& str, Uint32 n);

    /// Initialize with str.
    String(const Char16* str);

    /// Initialize with first n characters of str.
    String(const Char16* str, Uint32 n);

    /// Initialize from a plain old C-String:
    String(const char* str);

    /// Initialize from the first n characters of a plain old C-String:
    String(const char* str, Uint32 n);

    /// Destructor. 
    ~String();

    /** Assign this string with str.
	<pre>
	    String t1 = "abc";
	    String t2 = t1;
	</pre>
    */
    String& operator=(const String& str);

    /** Assign this string with String str.
        @param str String to assign.
        @return Returns the String.
    */
    String& assign(const String& str);

    /// Assign this string with str.
    String& assign(const Char16* str);

    /// Assign this string with first n characters of str.
    String& assign(const Char16* str, Uint32 n);

    /// Assign this string with the plain old C-String str.
    String& assign(const char* str);

    /// Assign this string with first n characters of the plain old C-String str.
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
	@param capacity defines the capacity in characters to reserve.
    */
    void reserveCapacity(Uint32 capacity);

    /** Returns the length of the String object.
	@return Length of the string in characters.
	<pre>
	    String s = "abcd";
	    assert(s.size() == 4);
	</pre>
    */
    Uint32 size() const;

    /** Returns a pointer to the first character in the 
	null-terminated Char16 buffer of the String object.
	@return	Pointer to the first character of the String object.
    	<pre>
	    String test = "abc";
	    const Char16* q = test.getChar16Data();
	</pre>
    */
    const Char16* getChar16Data() const;

    /** Create an 8-bit representation of this String object.

        @return CString object that provides access to the 8-bit String
        representation.

	<pre>
	    String test = "abc";
            printf("test = %s\n", (const char*)test.getCString());
	</pre>
    */
    CString getCString() const;

    /** Returns the specified character of the String object.
	@param index Index of the character to access.
        @return specified character of the String object.
	@exception IndexOutOfBoundsException if the index
	is outside the bounds of the String.
	<pre>
	    String test = "abc;
	    Char16 c = test[1];
	</pre>
    */
    Char16& operator[](Uint32 index);

    /** Returns the specified character of the String object (const version).
	@param index Index of the character to access.
        @return specified character of the String object.
	@exception IndexOutOfBoundsException if the index
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

    /// Append n characters from str to this String.
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
	@exception IndexOutOfBoundsException if size is greater than
	length of String plus starting index for remove.
    */
    void remove(Uint32 index, Uint32 size = PEG_NOT_FOUND);

    /** Return a new String which is initialzed with <TT>length</TT>
	characters from this string starting at <TT>index</TT>.
	@param <TT>index</TT> is the index in string to start getting the
	substring.
	@param <TT>length</TT> is the number of characters to get. If length
	is PEG_NOT_FOUND, then all characters after index are added to the new
	string.
	@return String with the defined substring.
    */
    String subString(Uint32 index, Uint32 length = PEG_NOT_FOUND) const;

    /** Find the index of the first occurence of the character c.
	If the character is not found, PEG_NOT_FOUND is returned.
	@param c Char to be found in the String.
	@return Position of the character in the string or PEG_NOT_FOUND if not
	found.
    */
    Uint32 find(Char16 c) const;

    /** Same as above but starts searching from the given index. 
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

    /** Converts all characters in this string to lower case.
    */
    void toLower();

    /** Compare the first n characters of the two strings..
    	@param s1 First null-terminated string for the comparison.
	@param s2 Second null-terminated string for the comparison.
	@param n Number of characters to compare.
	@return Return -1 if s1 is lexographically less than s2; if they are
	equavalent return 0; otherwise return 1.
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

    /** Just like the compare method defined above except that 
        the compareNoCase ignores case differences.
    */
    static int compareNoCase(const String& s1, const String& s2);

    /** Compare two String objects for equality.
	@param s1 First <TT>String</TT> for comparison.
	@param s2 Second <TT>String</TT> for comparison.

	@return true if the two strings are equal, false otherwise.
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
	@return true if strings are equal independent of case, flase
        otherwise.
    */
    static Boolean equalNoCase(const String& str1, const String& str2);

private:

    StringRep* _rep;
};

/** String operator ==. Test for equality between two strings of any of the
    types String or char*.
    @return true if the strings are equal, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const String& str1,
    const String& str2);

/** String operator ==. Test for equality between two strings.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const String& str1, const char* str2);

/** String operator ==. Test for equality between two strings.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const char* str1, const String& str2);

/** String operator ==. Test for equality between two strings.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(
    const String& str1,
    const String& str2);

///
PEGASUS_COMMON_LINKAGE PEGASUS_STD(ostream)& operator<<(
    PEGASUS_STD(ostream)& os,
    const String& str);

/** overload operator +	 - Concatenates String objects.
    <pre>
	String t1 = "abc";
	String t2;
	t2 = t1 + "def"
	assert(t2 == "abcdef");
    </pre>
*/
PEGASUS_COMMON_LINKAGE String operator+(const String& str1, const String& str2);

/** overload operator < - Compares String obects.
    <pre>
	String t1 = "def";
	String t2 = "a";
	assert (t2 < t1);
    </pre>
*/
PEGASUS_COMMON_LINKAGE Boolean operator<(
    const String& str1,
    const String& str2);

/** overload operator <= compares String objects.
*/
PEGASUS_COMMON_LINKAGE Boolean operator<=(
    const String& str1,
    const String& str2);

/** Overload operator > compares String objects
*/
PEGASUS_COMMON_LINKAGE Boolean operator>(
    const String& str1,
    const String& str2);

/** overload operator >= - Compares String objects
*/
PEGASUS_COMMON_LINKAGE Boolean operator>=(
    const String& str1,
    const String& str2);

PEGASUS_NAMESPACE_END

#endif /* Pegasus_String_h */
