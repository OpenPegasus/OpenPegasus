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
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/CIMNameCast.h>
#include "AssocClassTable.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static inline Boolean _MatchNoCase(const String& x, const String& pattern)
{
    return pattern.size() == 0 || String::equalNoCase(x, pattern);
}

static Boolean _GetRecord(ifstream& is, ClassAssociation& record)
{
    String line;

    if (!GetLine(is, line))
        return false;
    record.assocClassName = CIMNameCast(line);

    if (!GetLine(is, line))
        return false;
    record.fromClassName = CIMNameCast(line);

    if (!GetLine(is, line))
        return false;
    record.fromPropertyName = CIMNameCast(line);

    if (!GetLine(is, line))
        return false;
    record.toClassName = CIMNameCast(line);

    if (!GetLine(is, line))
        return false;
    record.toPropertyName = CIMNameCast(line);

    // Skip the blank line:

    if (!GetLine(is, line))
        return false;

    return true;
}

static inline void _PutField(ofstream& os, const CIMName& field)
{
    // Write the field in UTF-8.  Call write() to ensure no data
    // conversion by the stream.  Since all the fields contain CIM names,
    // it is not necessary to escape CR/LF characters.
    CString buffer = field.getString().getCString();
    os.write((const char *)buffer,
        static_cast<streamsize>(strlen((const char *)buffer)));
    os << endl;
}

static void _PutRecord(ofstream& os, const ClassAssociation& record)
{
    _PutField(os, record.assocClassName);
    _PutField(os, record.fromClassName);
    _PutField(os, record.fromPropertyName);
    _PutField(os, record.toClassName);
    _PutField(os, record.toPropertyName);
    os << endl;
}

void AssocClassTable::append(
    PEGASUS_STD(ofstream)& os,
    const String& path,
    const ClassAssociation& classAssociation)
{
    _PutRecord(os, classAssociation);

    // Update cache
    AssocClassCache* cache = _assocClassCacheManager.getAssocClassCache(path);
    if (cache->isActive())
    {
        cache->addRecord(classAssociation.fromClassName, classAssociation);
    }
}

void AssocClassTable::append(
    const String& path,
    const ClassAssociation& classAssociation)
{
    // Open input file:

    ofstream os;

    if (!OpenAppend(os, path))
        throw CannotOpenFile(path);

    _PutRecord(os, classAssociation);

    // Update cache
    AssocClassCache* cache = _assocClassCacheManager.getAssocClassCache(path);
    if (cache->isActive())
    {
        cache->addRecord(classAssociation.fromClassName, classAssociation);
    }
}

Boolean AssocClassTable::deleteAssociation(
    const String& path,
    const CIMName& assocClassName)
{
    // Open input file:

    ifstream is;
    if (!FileSystem::exists(path))
    {
        return false;
    }

    if (!Open(is, path))
    {
        throw CannotOpenFile(path);
    }

    // Open output file:

    String tmpPath = path + ".tmp";
    ofstream os;
    if (!Open(os, tmpPath))
    {
        throw CannotOpenFile(tmpPath);
    }

    // Copy over all lines except ones with the given association instance name:

    ClassAssociation classAssociation;
    Array<ClassAssociation> classAssociationsToDelete;

    while (_GetRecord(is, classAssociation))
    {
        if (assocClassName.getString() != classAssociation.assocClassName)
        {
            _PutRecord(os, classAssociation);
        }
        else
        {
            classAssociationsToDelete.append(classAssociation);
        }
    }

    // Close both files:

    is.close();
    os.close();

    // Rename back to original if tmp file is not empty
    Uint32 size = 0;
    Boolean gotFileSize = FileSystem::getFileSize(tmpPath, size);
    if (size || !gotFileSize)
    {
        if (!FileSystem::renameFile(tmpPath, path))
        {
            throw CannotRenameFile(path);
        }
    }
    else
    {
        // Delete tmp and original files
        FileSystem::removeFile(tmpPath);
        FileSystem::removeFile(path);
    }

    // Update cache
    AssocClassCache* cache = _assocClassCacheManager.getAssocClassCache(path);
    for (Uint32 i = 0; i < classAssociationsToDelete.size(); i++)
    {
        if (cache->isActive())
        {
            cache->removeRecord(
                classAssociationsToDelete[i].fromClassName,
                classAssociationsToDelete[i].assocClassName);
        }
    }

    return classAssociationsToDelete.size();
}

Boolean AssocClassTable::getAssociatorNames(
    const String& path,
    const Array<CIMName>& classList,
    const Array<CIMName>& assocClassList,
    const Array<CIMName>& resultClassList,
    const String& role,
    const String& resultRole,
    Array<String>& associatorNames)
{
    // Open input file:
    ifstream is;
    if (!FileSystem::exists(path))
    {
        return false;
    }

    if (!Open(is, path))
    {
        throw CannotOpenFile(path);
    }

    ClassAssociation classAssociation;
    Boolean found = false;

    // For each line in the associations table:
    while (_GetRecord(is, classAssociation))
    {
        // Process associations from the right end class and with right roles
        if (Contains(classList, classAssociation.fromClassName) &&
            _MatchNoCase(classAssociation.fromPropertyName.getString(), role) &&
            _MatchNoCase(
                classAssociation.toPropertyName.getString(), resultRole))
        {
            // Skip classes that do not appear in the association class list
            if ((assocClassList.size() != 0) &&
                (!Contains(assocClassList, classAssociation.assocClassName)))
            {
                continue;
            }

            // Skip classes that do not appear in the result class list
            if ((resultClassList.size() != 0) &&
                (!Contains(resultClassList, classAssociation.toClassName)))
            {
                continue;
            }

            // This class qualifies; add it to the list (skipping duplicates)
            if (!Contains(
                    associatorNames, classAssociation.toClassName.getString()))
            {
                associatorNames.append(
                    classAssociation.toClassName.getString());
            }
            found = true;
        }
    }

    return found;
}

Boolean AssocClassTable::_InitializeCache(
    AssocClassCache* cache,
    const String& path)
{
    if (!cache->isActive())
    {

        // Open input file:
        ifstream is;
        if (!FileSystem::exists(path))
        {
            return false;
        }

        if (!Open(is, path))
        {
            throw CannotOpenFile(path);
        }

        ClassAssociation classAssociation;

        // For each line in the associations table:
        while (_GetRecord(is, classAssociation))
        {
            cache->addRecord(classAssociation.fromClassName,
                             classAssociation);
        }

        cache->setActive(true);
    }

    return true;
}

Boolean AssocClassTable::getReferenceNames(
    const String& path,
    const Array<CIMName>& classList,
    const Array<CIMName>& resultClassList,
    const String& role,
    Array<String>& referenceNames)
{
    // Get the information from the association class cache.
    AssocClassCache* cache = _assocClassCacheManager.getAssocClassCache(path);

    if (!cache->isActive())
    {
        if (!_InitializeCache(cache,path))
            return false;
    }

    return cache->getReferenceNames(
        classList, resultClassList, role, referenceNames);
}

PEGASUS_NAMESPACE_END
