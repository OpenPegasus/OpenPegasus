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
// This code implements part of PEP#348 - The CMPI infrastructure using SCMO
// (Single Chunk Memory Objects).
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _SCMO_H_
#define _SCMO_H_


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMDateTimeRep.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/CIMQualifierList.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/System.h>

PEGASUS_NAMESPACE_BEGIN

//  Constants defining the size of the hash table used in the PropertySet
//  implementation.if size need to be changed, ensure size is power of two
//  to simplify moudulus calculation .

#define PEGASUS_PROPERTY_SCMB_HASHSIZE 64
#define PEGASUS_KEYBINDIG_SCMB_HASHSIZE 32

class SCMOClass;
class SCMOInstance;

// The enum of return values for SCMO functions.
enum SCMO_RC
{
    SCMO_OK = 0,
    SCMO_NULL_VALUE,
    SCMO_NOT_FOUND,
    SCMO_INDEX_OUT_OF_BOUND,
    SCMO_NOT_SAME_ORIGIN,
    SCMO_INVALID_PARAMETER,
    SCMO_TYPE_MISSMATCH,
    SCMO_WRONG_TYPE,
    SCMO_NOT_AN_ARRAY,
    SCMO_IS_AN_ARRAY
};

// This the definition of a relative pointer for objects stored in
// the single chunk memory. The memory is handled as array of chars.
struct SCMBDataPtr
{
    // start index of the data area
    Uint64      start;
    union
    {
        Uint64 __align64;
        // size of data area
        Uint32      size;
    };
};

//
// This enumeration and string array defines the qualifier names
// defined by the DMTF (standardized and optional ).
//
// The value QUALNAME_USERDEFINED indicates that the qualifier is
// user defined and the qualifier name must be specified.
//// KS_FUTURE  bug 9930 this may no longer be the correct list. ex. REFERENCE
/// qualifier Struct, etc.


enum QualifierNameEnum
{
    QUALNAME_USERDEFINED=0,
    QUALNAME_ABSTRACT,
    QUALNAME_AGGREGATE,
    QUALNAME_AGGREGATION,
    QUALNAME_ALIAS,
    QUALNAME_ARRAYTYPE,
    QUALNAME_ASSOCIATION,
    QUALNAME_BITMAP,
    QUALNAME_BITVALUES,
    QUALNAME_CLASSCONSTRAINT,
    QUALNAME_COMPOSITION,
    QUALNAME_CORRELATABLE,
    QUALNAME_COUNTER,
    QUALNAME_DELETE,
    QUALNAME_DEPRECATED,
    QUALNAME_DESCRIPTION,
    QUALNAME_DISPLAYDESCRIPTION,
    QUALNAME_DISPLAYNAME,
    QUALNAME_DN,
    QUALNAME_EMBEDDEDINSTANCE,
    QUALNAME_EMBEDDEDOBJECT,
    QUALNAME_EXCEPTION,
    QUALNAME_EXPENSIVE,
    QUALNAME_EXPERIMENTAL,
    QUALNAME_GAUGE,
    QUALNAME_IFDELETE,
    QUALNAME_IN,
    QUALNAME_INDICATION,
    QUALNAME_INVISIBLE,
    QUALNAME_ISPUNIT,
    QUALNAME_KEY,
    QUALNAME_LARGE,
    QUALNAME_MAPPINGSTRINGS,
    QUALNAME_MAX,
    QUALNAME_MAXLEN,
    QUALNAME_MAXVALUE,
    QUALNAME_METHODCONSTRAINT,
    QUALNAME_MIN,
    QUALNAME_MINLEN,
    QUALNAME_MINVALUE,
    QUALNAME_MODELCORRESPONDENCE,
    QUALNAME_NONLOCAL,
    QUALNAME_NONLOCALTYPE,
    QUALNAME_NULLVALUE,
    QUALNAME_OCTETSTRING,
    QUALNAME_OUT,
    QUALNAME_OVERRIDE,
    QUALNAME_PROPAGATED,
    QUALNAME_PROPERTYCONSTRAINT,
    QUALNAME_PROPERTYUSAGE,
    QUALNAME_PROVIDER,
    QUALNAME_PUNIT,
    QUALNAME_READ,
    QUALNAME_REQUIRED,
    QUALNAME_REVISION,
    QUALNAME_SCHEMA,
    QUALNAME_SOURCE,
    QUALNAME_SOURCETYPE,
    QUALNAME_STATIC,
    QUALNAME_SYNTAX,
    QUALNAME_SYNTAXTYPE,
    QUALNAME_TERMINAL,
    QUALNAME_TRIGGERTYPE,
    QUALNAME_UMLPACKAGEPATH,
    QUALNAME_UNITS,
    QUALNAME_UNKNOWNVALUES,
    QUALNAME_UNSUPPORTEDVALUES,
    QUALNAME_VALUEMAP,
    QUALNAME_VALUES,
    QUALNAME_VERSION,
    QUALNAME_WEEK,
    QUALNAME_WRITE
};

typedef CIMDateTimeRep SCMBDateTime;

//
// This union is used to represent the values of properties, qualifiers,
// method return values, and method arguments.
//
// This union is only used for simple CIMTypes.
// If the CIMType is a String the union contains a relative pointer to the
// string.
// If the CIMType is an Array the union contains
// a relative pointer to of SCMBUnion[].
//
// To be able to return an absloute pointer to a string including the string
// length ( without trailing '\0' ) the union contains the element extString.
//
union SCMBUnion
{
    struct
    {
        union
        {
            Uint64   u64;
            Boolean  bin;
            Uint8    u8;
            Sint8    s8;
            Uint16   u16;
            Sint16   s16;
            Uint32   u32;
            Sint32   s32;
            Sint64   s64;
            Real32   r32;
            Real64   r64;
            Uint16   c16;
        }val;
        // SCMBUnion used in array values.
        // This indicates if the single array member is a Null value.
        // The type of size 64 is used to fill up the whole union.
        Uint64 hasValue;
    }simple;

    SCMBDataPtr arrayValue;
    SCMBDataPtr stringValue;
    SCMBDateTime dateTimeValue;
    // Used for embedded references, instances, and objects
    // as an external references to SCMO_Instances.
    SCMOInstance* extRefPtr;

    // This structure is used to handle an absolute char*
    // including the length ( without traling '\0')
    struct
    {
        Uint32 length;
        char*  pchar;
    }extString;
};

struct SCMBValue
{
    union
    {
        Uint32 __align32;
        // The CIMType of the value
        CIMType         valueType;
    };

    struct{
        // If the value not set
        unsigned isNull:1;
        // If value is a type array
        unsigned isArray:1;
        // If value is set by the provider ( valid for SCMOInstance )
        unsigned isSet:1;
    } flags;

    union
    {
        Uint64 __align64;
        // The number of elements if the value is a type array.
        Uint32          valueArraySize;
    };

    SCMBUnion       value;
};

struct SCMBKeyBindingValue
{
    union
    {
        Uint64 __align64;
        // Boolean flag, if the key binding was set by the provider.
        Sint32       isSet;
    };
    // The value of the key binding
    SCMBUnion    data;
};

struct SCMBUserKeyBindingElement
{
    // If not 0, a relative pointer to the next element.
    SCMBDataPtr   nextElement;
    union
    {
        Uint64 __align64;
        // The cim type
        CIMType       type;
    };
    // Relative pointer to the key property name.
    SCMBDataPtr   name;
    // The value.
    SCMBKeyBindingValue value;
};

struct SCMBUserPropertyElement
{
    // If not 0, a relative pointer to the next element.
    SCMBDataPtr   nextElement;

    SCMBDataPtr classOrigin;

    // Relative pointer to the property name.
    SCMBDataPtr   name;
    // The value.
    SCMBValue value;
};

struct SCMBQualifier
{
    //Boolean flag
    Sint32              propagated;
    QualifierNameEnum   name;
    union
    {
        Uint64 __align64;
        // The same value as CIMFlavor.
        Uint32              flavor;
    };
    // if name == QUALNAME_USERDEFINED
    // the relative pointer to the user defined name
    SCMBDataPtr     userDefName;
    // Qualifier Value
    SCMBValue       value;
};

struct SCMBMgmt_Header
{
    union
    {
        Uint64 __align64;
        // The magic number for a SCMB memory object
        Uint32          magic;
    };
    // Total size of the SCMB memory block( # bytes )
    Uint64          totalSize;
    // The # of bytes avaialable in the dynamic area of SCMB memory block.
    Uint64          freeBytes;
    // Index to the start of the free space in this SCMB memory block.
    Uint64          startOfFreeSpace;
    // Number of external references in this instance.
    Uint32          numberExtRef;
    // Size of external reference index array;
    Uint32          sizeExtRefIndexArray;
    // Relative pointer to the external reference Array.
    SCMBDataPtr     extRefIndexArray;

};

//
// The SCMB design needs the capability to access the properties via property
// name and index.
//
// Therefore the OrderedSet implementation is mapped into SCMB class properties
// and key bindigs.
//

struct SCMBClassProperty
{
    // Relative pointer to property name
    SCMBDataPtr     name;
    Uint32          nameHashTag;
    // Flags
    struct{
        unsigned propagated:1;
        unsigned isKey:1;
    } flags;
    // Relative pointer to the origin class name
    SCMBDataPtr     originClassName;
    // Relative pointer to the reference class name
    SCMBDataPtr     refClassName;
    // Contains the default value if specified
    SCMBValue       defaultValue;
    union
    {
        Uint64 __align64;
        // Number of qualifiers in the array
        Uint32          numberOfQualifiers;
    };
    // Relative pointer to SCMBQualifierArray
    SCMBDataPtr     qualifierArray;
};

struct SCMBClassPropertySet_Header
{
    // Number of property nodes in the nodeArray
    union
    {
        Uint64 __align64;
        Uint32          number;
    };
    // Used for name based lookups based on the name tags.
    // A tag is generated by entangling the bit values of a first
    // and last letter of a CIMName.
    // PEGASUS_PROPERTY_SCMB_HASHSIZE is the hash size.
    // The index of the hashTable is calculated doing a remainder operator
    // with the name tag and hash size PEGASUS_PROPERTY_SCMB_HASHSIZE
    // (NameTag % PEGASUS_PROPERTY_SCMB_HASHSIZE)
    // The hashTable contains the index of the SCMBClassPropertyNode
    // in the nodeArray.
    Uint32  hashTable[PEGASUS_PROPERTY_SCMB_HASHSIZE];
    // Relative pointer to the ClassPropertyNodeArray;
    SCMBDataPtr     nodeArray;
};

struct SCMBClassPropertyNode
{
    // Is there a next node in the hash chain?
    Sint32          hasNext;
    // Array index of next property in hash chain.
    Uint32          nextNode;
    // The class property
    SCMBClassProperty theProperty;
};

struct SCMBKeyBindingNode
{
    // Is there a next node in the hash chain?
    Sint32          hasNext;
    // Array index of next property in hash chain.
    Uint32          nextNode;
    // Relative pointer to the key property name.
    SCMBDataPtr     name;
    Uint32          nameHashTag;
    // The type of the key binding.
    CIMType         type;
};

struct SCMBKeyBindingSet_Header
{
    // Number of keybindings in the nodeArray.
    union
    {
        Uint64 __align64;
        Uint32          number;
    };
    // Used for name based lookups based on the name tags
    // A tag is generated by entangling the bit values of a first
    // and last letter of a CIMName.
    // PEGASUS_KEYBINDIG_SCMB_HASHSIZE is the hash size.
    // The index of the hashTable is calculated doing a remainder operator
    // with the name tag and hash size PEGASUS_KEYBINDIG_SCMB_HASHSIZE
    // (NameTag % PEGASUS_KEYBINDIG_SCMB_HASHSIZE)
    // The hashTable contains the index of the SCMBKeyBindingNode
    // in the keyBindingNodeArray.
    Uint32          hashTable[PEGASUS_KEYBINDIG_SCMB_HASHSIZE];
    // Relative pointer to an array of SCMBKeyBindingNode.
    SCMBDataPtr     nodeArray;
};

struct SCMBClass_Main
{
    // The SCMB management header
    SCMBMgmt_Header     header;
    // The reference counter for this class
    AtomicInt       refCount;
    // Object flags
    struct
    {
      unsigned isEmpty :1;
    }flags;

    // SuperClassName
    SCMBDataPtr     superClassName;
    // Relative pointer to classname
    SCMBDataPtr     className;
    // Relative pointer to name space
    SCMBDataPtr     nameSpace;
    // The key properties of this class are identified
    // by a SCMBKeyPropertyMask
    SCMBDataPtr     keyPropertyMask;
    // A list of index to the key properties in the property set node array.
    SCMBDataPtr     keyIndexList;
    // Keybinding orderd set
    SCMBKeyBindingSet_Header keyBindingSet;
    // A set containing the class properties.
    SCMBClassPropertySet_Header    propertySet;
    // Relative pointer to SCMBQualifierArray
    union
    {
        Uint64 __align64;
        Uint32          numberOfQualifiers;
    };
    SCMBDataPtr     qualifierArray;

};

//
// This is the static header information of a SCMB instance.
// It also holds a reference to the SCMB representation of the class describing
// the instance.
//

struct SCMBInstance_Main
{
    // The SCMB management header
    SCMBMgmt_Header     header;
    // The reference counter for this instance
    AtomicInt           refCount;

    // Instance flags, define bitfields for instance flags.
    struct{
      unsigned includeQualifiers  :1;
      unsigned includeClassOrigin :1;
      unsigned isClassOnly:1;
      unsigned isCompromised:1;
      unsigned exportSetOnly:1;
      unsigned noClassForInstance:1;
    }flags;

    union
    {
        // An absolute pointer/reference to the class SCMB for this instance
        SCMOClass*   ptr;
        // To keep 64 Bit space for the pointer the same
        // size on 32 and 64 Bit systems.
        Uint64       placeHolder;
    }theClass;

    // Number of key bindings of the instance
    Uint32          numberKeyBindings;
    // Number of user defined key bindings
    Uint32          numberUserKeyBindings;
    // Relative pointer  SCMBUserKeyBindingElement
    SCMBDataPtr     userKeyBindingElement;
    // Relative pointers to the name space name and class name.
    // Will be initialized by with the values of the linked SCMOClass.
    // If it was overwritten, the new value is stored in the SCMOInstance
    // and the the flag isCompromised is set to true.
    // This flag is also set when a SCMOInstance is created from a
    // CMPEncNewObjectPath and there is not Class in the repository.
    SCMBDataPtr     instNameSpace;
    SCMBDataPtr     instClassName;
    // Relative pointer to hostname
    SCMBDataPtr     hostName;
    // Relative pointer to SCMBInstanceKeyBindingArray
    SCMBDataPtr     keyBindingArray;

    // Number of properties of the class.
    Uint32 numberProperties;
    // Definition of user-defined properties which are properties introduced
    // by the creator of the instance but which are not in the SCMOClass.
    // This is allowed only if the noClassForInstance flag is set indicating
    // that there was no class found for the defined classname/namespace
    // Number of user defined properties
    Uint32 numberUserProperties;

    // Relative pointer to SCMBInstancePropertyArray
    SCMBDataPtr     propertyArray;

    // Relative pointer  SCMBUserPropertyElement
    SCMBDataPtr     userPropertyElement;
};

// The index of an instance key binding is the same as the class.
// If a key binding has to be found by name, the class key binding ordered set
// has to be used to find the right index.
typedef SCMBKeyBindingValue SCMBInstanceKeyBindingArray[];

// The index of an instance property is the same as the class property.
// If a property has to be found by name, the class property ordered set
// has to be used to find the right index.
// The properties of an instances contains only the values provided.
// If a instance property does not contain a value
// the default value of the class has to be used.
typedef SCMBValue   SCMBInstancePropertyArray[];

// It is a bit mask to identify key properties of an instance or class.
// The index of a key property in the property set is eqal to the index with in
// the key property mask.
// If a bit is not set (0) then it is not a key property.
// If a bit is set (1) then it is a key property.
// The array size of the key property mask is (numberOfProperties / 64)
typedef Uint64        SCMBKeyPropertyMask[];

// The static definition of the common SCMO memory functions
inline const void* _resolveDataPtr(
    const SCMBDataPtr& ptr,
    const char* base)
{
    return ((ptr.start==(Uint64)0 ? NULL : (void*)&(base[ptr.start])));
}

inline const char* _getCharString(
    const SCMBDataPtr& ptr,
    const char* base)
{
    return ((ptr.start==(Uint64)0 ? NULL : &(base[ptr.start])));
}

inline Uint32 _generateStringTag(const char* str, Uint32 len)
{
    if (len == 0)
    {
        return 0;
    }
    return
        (Uint32(CharSet::toUpperHash(str[0]) << 1) |
        Uint32(CharSet::toUpperHash(str[len-1])));
}

inline Uint32 _generateSCMOStringTag(
    const SCMBDataPtr& ptr,
    char* base)
{
    // Length of a SCMBDataPtr to a UTF8 string includes the trailing '\0'.
    return _generateStringTag(_getCharString(ptr,base),ptr.size-1);
}

PEGASUS_COMMON_LINKAGE extern void _destroyExternalReferencesInternal(
    SCMBMgmt_Header* memHdr);

#ifdef PEGASUS_HAS_ICU
Uint32 _utf8ICUncasecmp(
    const char* a,
    const char* b,
    Uint32 len);
#endif

static inline Boolean _equalNoCaseUTF8Strings(
    const SCMBDataPtr& ptr_a,
    char* base,
    const char* name,
    Uint32 len)

{
#ifdef PEGASUS_HAS_ICU
    //both are empty strings, so they are equal.
    if (ptr_a.size == 0 && len == 0)
    {
        return true;
    }
    // size without trailing '\0' !!
    if (ptr_a.size-1 != len)
    {
        return false;
    }
    const char* a = (const char*)_getCharString(ptr_a,base);

    return ( _utf8ICUncasecmp(a,name,len)== 0);
#else
    return System::strncasecmp(
        &base[ptr_a.start],
        Uint32(ptr_a.size-1),name,len);
#endif
}

PEGASUS_NAMESPACE_END


#endif
