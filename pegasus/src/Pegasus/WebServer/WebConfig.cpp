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


#include <fstream>
#include <iostream>

#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/WebServer/WebConfig.h>


PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN


const String WebConfig::PROPERTY_WEB_ROOT = "webRoot";
const String WebConfig::PROPERTY_INDEX_FILE = "indexFile";
const String WebConfig::PROPERTY_MIMETYPES_FILE = "mimeTypesFile";


WebConfig::WebConfig()
{
    _loadConfig();
}

WebConfig::~WebConfig()
{
}

String WebConfig::getWebRoot() const
{
    return _webRoot;
}

String WebConfig::getIndexFile() const
{
    return _indexFile;
}

WebConfig::MimeTypes WebConfig::getMimeTypes() const
{
    return _mimeTypes;
}

void WebConfig::reload()
{
    _loadConfig();
}

void WebConfig::_loadConfig()
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebConfig::_loadConfig()");

    ConfigManager* _configManager = ConfigManager::getInstance();

    // get values from current config
    _webRoot = ConfigManager::getHomedPath(
                 _configManager->getCurrentValue(PROPERTY_WEB_ROOT));

    _indexFile = _configManager->getCurrentValue(PROPERTY_INDEX_FILE);

    String mimeTypesFile = ConfigManager::getHomedPath(
                               _configManager->getCurrentValue(
                                   PROPERTY_MIMETYPES_FILE));

    PEG_TRACE((TRC_WEBSERVER, Tracer::LEVEL4,
        "WebConfig::_loadConfig() - "
            "ConfigManager returned\nwebRoot='%s',\nindexFile='%s',\n"
                "mimeTypesFile='%s'",
        (const char*)_webRoot.getCString(),
        (const char*)_indexFile.getCString(),
        (const char*)mimeTypesFile.getCString()));

    // load mime types map
    _loadMimeTypes(mimeTypesFile);

    PEG_METHOD_EXIT();
}

void WebConfig::_loadMimeTypes(String& mimeTypesFile)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebConfig::_loadMimeTypes()");

    // ensure an empty list
    _mimeTypes.clear();

    const char delimiter = '=';
    ifstream fileStream(mimeTypesFile.getCString());

    // mimetypes file exists ?
    if (!fileStream)
    {// no, use defaults and abort
        // set defaults
        _mimeTypes.insert("html", "text/html");
        _mimeTypes.insert("htm", "text/html");
        _mimeTypes.insert("css", "text/css");
        _mimeTypes.insert("txt", "text/plain");
        _mimeTypes.insert("js", "text/javascript");
        _mimeTypes.insert("png", "image/png");
        _mimeTypes.insert("gif", "image/gif");
        _mimeTypes.insert("jpg", "image/jpeg");
        // write trace
        PEG_TRACE_CSTRING(TRC_WEBSERVER, Tracer::LEVEL4,
            "WebConfig::_loadMimeTypes() - "
                "Failed to read mime-types file, using defaults.");

        // trace
        String defaultMimeTypes = "";
        for (MimeTypes::Iterator i = _mimeTypes.start(); i; i++)
        {
            defaultMimeTypes.append(i.key() + ":" + i.value() + "\n");
        }

        defaultMimeTypes.append("\n");
        PEG_TRACE((TRC_WEBSERVER, Tracer::LEVEL4,
            "WebConfig::_loadMimeTypes() - "
                 "Failed to read mime-types file, using defaults."
                     " Default MIME-types are: \n%s",
            (const char*)defaultMimeTypes.getCString()));
        return;
    }
    PEG_TRACE((TRC_WEBSERVER, Tracer::LEVEL4,
        "WebConfig::_loadMimeTypes() - "
            "Reading mime-types file '%s'",
    (const char*)mimeTypesFile.getCString()));

    String line;
    while (!fileStream.eof())
    {
        // read line
        GetLine(fileStream, line);

        // is it a comment line ?
        if (!line.compare("#", line, 1))
        {
            continue;
        }
        // find delimiter
        Uint32 delimPos = line.find(delimiter);
        if (PEG_NOT_FOUND == delimPos)
        {// delimiter not found
            continue;
        }

        // extract key/value
        String key = line.subString(0, delimPos);
        String value = line.subString(delimPos+1);

        // apply config value
        _mimeTypes.insert(key, value);
    }

    String definedMimeTypes = "";
    for (MimeTypes::Iterator i = _mimeTypes.start(); i; i++)
    {
        definedMimeTypes.append(i.key() + ":" + i.value() + "\n");
    }
    definedMimeTypes.append("\n");

    PEG_TRACE((TRC_WEBSERVER, Tracer::LEVEL4,
        "WebConfig::_loadMimeTypes() - "
            "Loaded MIME-types are: \n%s",
        (const char*)definedMimeTypes.getCString()));

    PEG_METHOD_EXIT();
}

/* END */

PEGASUS_NAMESPACE_END
