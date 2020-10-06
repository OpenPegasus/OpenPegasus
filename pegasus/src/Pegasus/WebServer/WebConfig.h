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

#ifndef Pegasus_Webconfig_h
#define Pegasus_Webconfig_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/WebServer/Linkage.h>


PEGASUS_NAMESPACE_BEGIN




/*
 * Contains properties required by the WebProcessor class in order to serve
 * any files.
 */
class PEGASUS_WEBSERVER_LINKAGE WebConfig
{

public:



    /**
       Initializes the config with defaults which will be overriden by
       the current config values and mime-types file.
     */
    WebConfig();

    /**
       Destructor
     */
    ~WebConfig();

    /**
     */
    String getWebRoot() const;

    /**
     */
    String getIndexFile() const;

    /*
     * MimeType mapping, for ex. '.js => text/javascript'
     */
    typedef HashTable<String, String,
                EqualFunc<String>,
                HashFunc<String> > MimeTypes;
    /**
     */
    MimeTypes getMimeTypes() const;

    /**
     */
    void reload();

    
    /*
     * Property-key for the webRoot stored in the current config.
     */
    static const String PROPERTY_WEB_ROOT;

    /*
     * Property-key for the indexFile stored in the current config.
     */
    static const String PROPERTY_INDEX_FILE;

    /*
     * Property-key for the path to the mimetypes file stored in the current
     * config.
     */
    static const String PROPERTY_MIMETYPES_FILE;

private:

    /**
       Absolute path to the document root for the files to serve.
     */
    String _webRoot;

    /**
       Name of index-file, will be served when requesting resource '/'
       by default it is index.html at _webRoot dir
     */
    String _indexFile;
    /**

     */
    MimeTypes _mimeTypes;


    /**
       Locale encoding of files, for ex. 'ISO-5889-1' or 'UTF-8'
     */
    String _localFileEnc;

    /**
       Loads mimetypes from file.
       MimeTypes defined via the file will override default values.

       @param mimeTypesFile
            Absolute filename of text-file containing mime-type definitions
     */
    void _loadMimeTypes(String& mimeTypesFile);

    /**
       Loads the config via the Config-Manager from the current configuration.
     */
    void _loadConfig();
};

PEGASUS_NAMESPACE_END
#endif /* Pegasus_Webconfig_h */
