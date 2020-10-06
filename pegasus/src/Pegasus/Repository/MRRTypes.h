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

#ifndef Pegasus_MRRTypes_h
#define Pegasus_MRRTypes_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMType.h>
#include "Linkage.h"

#define MRR_FLAG_PROPERTY           (1 << 0)
#define MRR_FLAG_REFERENCE          (1 << 1)
#define MRR_FLAG_METHOD             (1 << 2)
#define MRR_FLAG_CLASS              (1 << 3)
#define MRR_FLAG_ABSTRACT           (1 << 4)
#define MRR_FLAG_AGGREGATE          (1 << 5)
#define MRR_FLAG_AGGREGATION        (1 << 6)
#define MRR_FLAG_ASSOCIATION        (1 << 7)
#define MRR_FLAG_COMPOSITION        (1 << 8)
#define MRR_FLAG_COUNTER            (1 << 9)
#define MRR_FLAG_DELETE             (1 << 10)
#define MRR_FLAG_DN                 (1 << 11)
#define MRR_FLAG_EMBEDDEDOBJECT     (1 << 12)
#define MRR_FLAG_EXCEPTION          (1 << 13)
#define MRR_FLAG_EXPENSIVE          (1 << 14)
#define MRR_FLAG_EXPERIMENTAL       (1 << 15)
#define MRR_FLAG_GAUGE              (1 << 16)
#define MRR_FLAG_IFDELETED          (1 << 17)
#define MRR_FLAG_IN                 (1 << 18)
#define MRR_FLAG_INDICATION         (1 << 19)
#define MRR_FLAG_INVISIBLE          (1 << 20)
#define MRR_FLAG_KEY                (1 << 21)
#define MRR_FLAG_LARGE              (1 << 22)
#define MRR_FLAG_OCTETSTRING        (1 << 23)
#define MRR_FLAG_OUT                (1 << 24)
#define MRR_FLAG_READ               (1 << 25)
#define MRR_FLAG_REQUIRED           (1 << 26)
#define MRR_FLAG_STATIC             (1 << 27)
#define MRR_FLAG_TERMINAL           (1 << 28)
#define MRR_FLAG_WEAK               (1 << 29)
#define MRR_FLAG_WRITE              (1 << 30)
#define MRR_FLAG_EMBEDDEDINSTANCE   (1 << 31)

#define MRR_SCOPE_MRR            (1 << 0)
#define MRR_SCOPE_CLASS             (1 << 1)
#define MRR_SCOPE_ASSOCIATION       (1 << 2)
#define MRR_SCOPE_INDICATION        (1 << 3)
#define MRR_SCOPE_PROPERTY          (1 << 4)
#define MRR_SCOPE_REFERENCE         (1 << 5)
#define MRR_SCOPE_METHOD            (1 << 6)
#define MRR_SCOPE_PARAMETER         (1 << 7)
#define MRR_SCOPE_ANY               (1|2|4|8|16|32|64|128)

#define MRR_FLAVOR_OVERRIDABLE      (1 << 0)
#define MRR_FLAVOR_TOSUBCLASS       (1 << 1)
//#define MRR_FLAVOR_TOINSTANCE       (1 << 2)
#define MRR_FLAVOR_TRANSLATABLE     (1 << 3)
#define MRR_FLAVOR_DISABLEOVERRIDE  (1 << 4)
#define MRR_FLAVOR_RESTRICTED       (1 << 5)

#define MRR_MAX_FEATURES 256

PEGASUS_NAMESPACE_BEGIN

struct MRRFeature
{
    Uint32 flags;
    char* name;
    const char** qualifiers;
};

struct MRRProperty /* extends MRRFeature */
{
    // Inherited fields:
    Uint32 flags;
    char* name;
    const char** qualifiers;

    // Local fields:
    Uint16 type;
    Sint16 subscript;
    const char* value;
};

struct MRRReference /* extends MRRFeature */
{
    // Inherited fields:
    Uint32 flags;
    char* name;
    const char** qualifiers;

    // Local fields:
    Sint16 subscript;
    struct MRRClass* ref;
};

struct MRRMethod /* extends MRRFeature */
{
    // Inherited fields:
    Uint32 flags;
    char* name;
    const char** qualifiers;

    // Local fields:
    Uint16 type;
    MRRFeature** parameters;
};

struct MRRClass
{
    Uint32 flags;
    char* name;
    const char** qualifiers;
    struct MRRClass* super;
    MRRFeature** features;
};

struct MRRQualifierDecl
{
    char* name;
    Uint16 type;
    Sint16 subscript;
    Uint16 scope;
    Uint16 flavor;
    const char* value;
};

struct MRRNameSpace
{
    char* name;
    MRRQualifierDecl** qualifiers;
    MRRClass** classes;
};

const MRRQualifierDecl* FindQualifierDecl(
    const MRRNameSpace* ns,
    const char* name);

const MRRClass* FindClass(const MRRNameSpace* ns, const char* name);

bool IsSubClass(const MRRClass* super, const MRRClass* sub);

inline bool IsA(const MRRClass* super, const MRRClass* sub)
{
    return sub == super || IsSubClass(super, sub);
}

struct MRRFeatureInfo
{
    const MRRFeature* sf;
    const MRRClass* sc;
};

int MergeFeatures(
    const MRRClass* sc,
    bool localOnly,
    Uint32 flags,
    MRRFeatureInfo features[MRR_MAX_FEATURES],
    size_t& numFeatures);

int MakeClass(
    const char* hostName,
    const MRRNameSpace* ns,
    const MRRClass* sc,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const char* const* propertyList,
    class CIMClass& cc);

int MakeQualifierDecl(
    const MRRNameSpace* ns,
    const MRRQualifierDecl* mqd,
    class CIMQualifierDecl& cqd);

const MRRFeature* FindFeature(
    const MRRClass* sc,
    const char* name,
    Uint32 flags =
        (MRR_FLAG_PROPERTY|MRR_FLAG_REFERENCE|MRR_FLAG_METHOD));

PEGASUS_NAMESPACE_END

#endif /* Pegasus_MRRTypes_h */
