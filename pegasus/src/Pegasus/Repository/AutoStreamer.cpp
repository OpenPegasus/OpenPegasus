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


#include "ObjectStreamer.h"

#include "AutoStreamer.h"

PEGASUS_NAMESPACE_BEGIN

AutoStreamer::AutoStreamer(ObjectStreamer *primary, Uint8 marker)
{
    _readerCount=0;
    if (marker)
    {
        _readers[_readerCount].reader=primary;
        _readers[_readerCount++].marker=marker;
    }
    else _defaultReader=primary;
    _primary=primary;
}

AutoStreamer::~AutoStreamer()
{
    for (Uint32 i=0; i<=_readerCount; ++i)
    {
        if (_defaultReader != _readers[i].reader)
            delete _readers[i].reader;
    }
    delete _defaultReader;
}

void AutoStreamer::addReader(ObjectStreamer *reader, Uint8 marker)
{
    if (marker)
    {
        _readers[_readerCount].reader=reader;
        _readers[_readerCount++].marker=marker;
    }
    else _defaultReader=reader;
}


void AutoStreamer::encode(Buffer& out, const CIMClass& cls)
{
   _primary->encode(out,cls);
}

void AutoStreamer::encode(Buffer& out, const CIMInstance& inst)
{
   _primary->encode(out,inst);
}

void AutoStreamer::encode(Buffer& out, const CIMQualifierDecl& qual)
{
   _primary->encode(out,qual);
}

void AutoStreamer::write(PEGASUS_STD(ostream)& os, Buffer& in)
{
   _primary->write(os,in);
}


void AutoStreamer::decode(
    const Buffer& in,
    unsigned int pos,
    CIMClass& cls)
{
    for (Uint16 i=0,m=_readerCount; i<m; i++)
    {
        if (_readers[i].marker==in[pos])
        {
            _readers[i].reader->decode(in,pos,cls);
            return;
        }
    }
    _defaultReader->decode(in,pos,cls);
}

void AutoStreamer::decode(
    const Buffer& in,
    unsigned int pos,
    CIMInstance& inst)
{
    for (Uint16 i=0,m=_readerCount; i<m; i++)
    {
        if (_readers[i].marker==in[pos])
        {
            _readers[i].reader->decode(in,pos,inst);
            return;
        }
    }
    _defaultReader->decode(in,pos,inst);
}

void AutoStreamer::decode(
    const Buffer& in,
    unsigned int pos,
    CIMQualifierDecl& qual)
{
    for (Uint16 i=0,m=_readerCount; i<m; i++)
    {
        if (_readers[i].marker==in[pos])
        {
            _readers[i].reader->decode(in,pos,qual);
            return;
        }
    }
    _defaultReader->decode(in,pos,qual);
}

PEGASUS_NAMESPACE_END
