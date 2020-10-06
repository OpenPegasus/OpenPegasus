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



#ifndef Pegasus_inl_h 
# define Pegasus_inl_h

// For bad alloc
#include<new>

/* 
 * This file is intended to hold small convnient inline functions.
 * Please do not include big functions here.  
 *
 * To identify itself as inline function coming from this file, Please
 * name the function name starting with peg_inln_*. For example
 * peg_inln_realloc
 */



/*
 * pegasus wrapper on realloc functions and should prefer
 * over the native realloc as it takes care of failure situation
 * It frees old pointer which hold the allocated memory and hence
 * prevent leak and throws bad_alloc exception.
*/
inline void* peg_inln_realloc(void *oldPtr, size_t newSize)
{
    void *newPtr = NULL;
    if ( (newPtr = ::realloc( oldPtr, newSize) ) == NULL) 
    {
        free( oldPtr);
        throw PEGASUS_STD(bad_alloc)();
    }
    return newPtr;
}

/**
    Return a const char * true or false for the Boolean input.
    Note that this works for Boolean and bool.  It is contained
    here because to work around a bug and should be moved to proper
    place when we have a separate files for CIMTypes

    See the 9785 and 9778

    @return const char* containing "true" or "false"
*/

inline const char* boolToString( const bool x )
{
    return x ? "true": "false";
}


#endif //Pegasus_inl_h end
