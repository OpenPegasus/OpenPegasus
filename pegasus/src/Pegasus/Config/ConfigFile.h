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

#ifndef Pegasus_ConfigFile_h
#define Pegasus_ConfigFile_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Config/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//  ConfigFile Class
////////////////////////////////////////////////////////////////////////////////

struct ConfigTable;

/**
  This class provides methods to read/write configuration properties from the
  config file.
*/
class PEGASUS_CONFIG_LINKAGE ConfigFile
{
public:

    /**
    Constructor.

    @exception  FileNotReadable  if the specified file is not readable.
    */
    ConfigFile(const String& fileName);


    /** Destructor. */
    ~ConfigFile();


    /**
    Get the name of the configuration file.

    @return String    name of the config file.
    */
    String getFileName() const;


    /**
    Load the properties from the config file.

    @param     confTable   hash table containing the config properties.
    @exception ConfigFileSyntaxError  if config file contains a syntax error.

    Note: Original code was taken from OptionManager::mergeFile()
    */
    void load(ConfigTable* confTable);


    /**
    Save the properties to the config file.

    @param  confTable    hash table containing the config properties.
    @exception  CannotRenameFile  if failed to create the backup file.
    @exception  CannotOpenFile  if failed to open the config file.
    @exception  CannotChangeFilePerm
                                if failed to set permissions on the config file.
    */
    void save(ConfigTable* confTable);


    /**
    Replace the properties in the config file with the properties from
    the given file

    @param      fileName     the config file name to be copied.
    @exception  NoSuchFile   if the specified file cannot be opened.
    @exception  FileNotReadable  if the specified file is not readable.
    @exception  CannotRenameFile if failed to create the backup file.
    @exception  CannotOpenFile  if failed to open the config file.
    @exception  CannotChangeFilePerm
                                if failed to set permissions on the config file.
    */
    void replace(const String& fileName);


private:
    String    _configFile;
    String    _configBackupFile;
};


PEGASUS_NAMESPACE_END

#endif /* Pegasus_ConfigFile_h */
