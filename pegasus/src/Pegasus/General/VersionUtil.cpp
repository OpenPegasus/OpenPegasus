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

#include <Pegasus/Common/Config.h>
#include "VersionUtil.h"

PEGASUS_NAMESPACE_BEGIN

//
// Returns true if v1 has a higher or same version, else return false.
//
Boolean VersionUtil::isVersionGreaterOrEqual(
    const String& v1,
    Uint32 majorV2,
    Uint32 minorV2,
    Uint32 updateV2)
{
    Uint32 minorV1 = PEG_NOT_FOUND;
    Uint32 majorV1 = PEG_NOT_FOUND;
    Uint32 updateV1 = PEG_NOT_FOUND;

    int result;
    char dummychar;
    result = sscanf((const char*)v1.getCString(),
        "%d.%d.%d%c",
        &majorV1,
        &minorV1,
        &updateV1,
        &dummychar);

    if (result != 3)
    {
        return false;
    }

    return ((majorV1 > majorV2) ||
            ((majorV1 == majorV2) &&
             ((minorV1 > minorV2) ||
              ((minorV1 == minorV2) && (updateV1 >= updateV2)))));
}

Boolean VersionUtil::parseVersion(
    const String& version,
    Sint32& iMajor,
    Sint32& iMinor,
    Sint32& iUpdate)
{
    const char CHAR_PERIOD = '.';

    if (!version.size())
    {
        return true;
    }

    iMajor = 0;
    iMinor = 0;
    iUpdate = 0;

    Uint32 indexM = 0;
    Uint32 sizeM = PEG_NOT_FOUND;
    Uint32 indexN = PEG_NOT_FOUND;
    Uint32 sizeN = PEG_NOT_FOUND;
    Uint32 indexU = PEG_NOT_FOUND;

    // If "V" specified as first character, ignore it
    if ((version[0] == 'V') || (version[0] == 'v'))
    {
        indexM = 1;
    }

    // Find the M, N, and U version fields delimited by '.' characters

    indexN = version.find(indexM, CHAR_PERIOD);

    if (indexN != PEG_NOT_FOUND)
    {
        sizeM = indexN++ - indexM;

        indexU = version.find(indexN, CHAR_PERIOD);
        if (indexU != PEG_NOT_FOUND)
        {
            sizeN = indexU++ - indexN;
        }
    }

    // Parse the major version
    char dummyChar;
    int numConversions = sscanf(
        version.subString(indexM, sizeM).getCString(),
        "%u%c",
        &iMajor,
        &dummyChar);

    if (numConversions != 1)
    {
        return false;
    }

    // Parse the minor version
    if (indexN != PEG_NOT_FOUND)
    {
        numConversions = sscanf(
            version.subString(indexN, sizeN).getCString(),
            "%u%c",
            &iMinor,
            &dummyChar);

        if (numConversions != 1)
        {
            return false;
        }
    }

    // Parse the update version
    if (indexU != PEG_NOT_FOUND)
    {
        numConversions = sscanf(
            version.subString(indexU).getCString(),
            "%u%c",
            &iUpdate,
            &dummyChar);

        if (numConversions != 1)
        {
            return false;
        }
    }

    return true;
}
PEGASUS_NAMESPACE_END
