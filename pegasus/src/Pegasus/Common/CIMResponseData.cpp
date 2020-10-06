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
// Class CIMResponseData encapsulates the possible types of response data
// representations and supplies conversion methods between these types.
// PEP#348 - The CMPI infrastructure using SCMO (Single Chunk Memory Objects)
// describes its usage in the server flow.
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////

#include "CIMResponseData.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/SCMOXmlWriter.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/CIMInternalXmlEncoder.h>
#include <Pegasus/Common/SCMOInternalXmlEncoder.h>


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// Defines debug code in CIMResponseData.  This under this
// special compile flag so that it can be compiled independent of
// PEGASUS_DEBUG flags.
// #define CIMRESPONSEDATA_DEBUG

#define LOCAL_MIN(a, b) ((a < b) ? a : b)

/*
    Append an InstanceElement to the Buffer. This function accounts
    for the existence of a propertyList.
*/
void CIMResponseData::_appendInstanceElement(
    Buffer& out,
    SCMOInstance _scmoInstance)
{
    if(_propertyList.isNull())
    {
        Array<Uint32> emptyNodes;
        SCMOXmlWriter::appendInstanceElement(
            out,
            _scmoInstance,
            false,
            emptyNodes);
    }
    else
    {
        Array<propertyFilterNodesArray_t> propFilterNodesArrays;
        // This searches for an already created array of nodes,
        //if not found, creates it inside propFilterNodesArrays
        const Array<Uint32> & nodes=
            SCMOXmlWriter::getFilteredNodesArray(
                propFilterNodesArrays,
                _scmoInstance,
                _propertyList);
        SCMOXmlWriter::appendInstanceElement(
            out,
            _scmoInstance,
            true,
            nodes);
    }
}

// Instance Names handling
Array<CIMObjectPath>& CIMResponseData::getInstanceNames()
{
    PEGASUS_DEBUG_ASSERT(
    (_dataType==RESP_INSTNAMES || _dataType==RESP_OBJECTPATHS));
    _resolveToCIM();
    PEGASUS_DEBUG_ASSERT(_encoding==RESP_ENC_CIM || _encoding == 0);
    return _instanceNames;
}

// Get a single instance as a CIM instance.
// This converts all of the objects in the response data to
// CIM form as part of the conversion.
// If there are no instances in the object, returns CIMInstance(),
// an empty instance.
CIMInstance& CIMResponseData::getInstance()
{
    PEGASUS_DEBUG_ASSERT(_dataType == RESP_INSTANCE);
    _resolveToCIM();
    if (0 == _instances.size())
    {
        _instances.append(CIMInstance());
    }
    return _instances[0];
}

// Instances handling
Array<CIMInstance>& CIMResponseData::getInstances()
{
    PEGASUS_DEBUG_ASSERT(_dataType == RESP_INSTANCES);
    _resolveToCIM();
    return _instances;
}

// Instances handling specifically for the client where the call may
// get either instances or objects and must convert them to instances
// NOTE: This is a temporary solution to satisfy the BinaryCodec passing
// of data to the client where the data could be either instances or
// objects.  The correct solution is to convert back when the provider, etc.
// returns the data to the server.  We must convert to that solution but
// this keeps it working for the moment.
Array<CIMInstance>& CIMResponseData::getInstancesFromInstancesOrObjects()
{
    if (_dataType == RESP_INSTANCES)
    {
        _resolveToCIM();
        return _instances;
    }
    else if (_dataType == RESP_OBJECTS)
    {
        _resolveToCIM();
        for (Uint32 i = 0 ; i < _objects.size() ; i++)
        {
            _instances.append((CIMInstance)_objects[i]);
        }
        return _instances;

    }
    PEGASUS_ASSERT(false);
}

// Objects handling
Array<CIMObject>& CIMResponseData::getObjects()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMResponseData::getObjects");
    PEGASUS_DEBUG_ASSERT(_dataType == RESP_OBJECTS);
    _resolveToCIM();
    PEG_METHOD_EXIT();
    return _objects;
}

// SCMO representation, single instance stored as one element array
// object paths are represented as SCMOInstance
// Resolve all of the information in the CIMResponseData container to
// SCMO  and return all scmoInstances.
// Note that since the SCMO representation,
// a is single instance stored as one element array and object paths are
// represented as SCMOInstance this returns array of SCMOInstance.
Array<SCMOInstance>& CIMResponseData::getSCMO()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMResponseData::getSCMO");
    // This function resolves to instances and so cannot handle responses to
    // the associators,etc.requests that return classes (input object path with
    // no keys). That issue is resolved however, since CIMResponseData uses the
    // _isClassOperation variable (set by the request) to determine whether
    // the responses are classpaths or instancepaths and the default is
    // false(instancePaths) so that this should always produce instance paths.

    _resolveToSCMO();
    PEG_METHOD_EXIT();
    return _scmoInstances;
}

// set an array of SCMOInstances into the response data object
void CIMResponseData::setSCMO(const Array<SCMOInstance>& x)
{

    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMResponseData::setSCMO");
    _scmoInstances=x;
    _encoding |= RESP_ENC_SCMO;
    _size += x.size();
    PEG_METHOD_EXIT();
}

// Binary data is just a data stream
Array<Uint8>& CIMResponseData::getBinary()
{
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_BINARY || _encoding == 0);
    return _binaryData;
}

bool CIMResponseData::setBinary(CIMBuffer& in)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMResponseData::setBinary");

    // Append all serial data from the CIMBuffer to the local data store.
    // Returns error if input not a serialized Uint8A
    if (!in.getUint8A(_binaryData))
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to get binary input data!");
        PEG_METHOD_EXIT();
        return false;
    }
    _encoding |= RESP_ENC_BINARY;
    PEG_METHOD_EXIT();
    return true;
}

bool CIMResponseData::setRemainingBinaryData(CIMBuffer& in)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMResponseData::setRemainingBinaryData");

    // Append any data that has not been deserialized already from
    // the CIMBuffer.
    size_t remainingDataLength = in.remainingDataLength();
    _binaryData.append((Uint8*)in.getPtr(), remainingDataLength);

    _encoding |= RESP_ENC_BINARY;
    PEG_METHOD_EXIT();
    return true;
}

bool CIMResponseData::setXml(CIMBuffer& in)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMResponseData::setXml");

    switch (_dataType)
    {
        case RESP_INSTANCE:
        {
            Array<Sint8> inst;
            Array<Sint8> ref;
            CIMNamespaceName ns;
            String host;
            if (!in.getSint8A(inst))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML instance data!");
                return false;
            }
            _instanceData.insert(0,inst);
            if (!in.getSint8A(ref))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML instance data (reference)!");
                return false;
            }
            _referencesData.insert(0,ref);
            if (!in.getString(host))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML instance data (host)!");
                return false;
            }
            _hostsData.insert(0,host);
            if (!in.getNamespaceName(ns))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML instance data (namespace)!");
                return false;
            }
            _nameSpacesData.insert(0,ns);
            _size++;
            break;
        }
        case RESP_INSTANCES:
        {
            Uint32 count;
            if (!in.getUint32(count))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML instance data (number of instance)!");
                return false;
            }
            for (Uint32 i = 0; i < count; i++)
            {
                Array<Sint8> inst;
                Array<Sint8> ref;
                CIMNamespaceName ns;
                String host;
                if (!in.getSint8A(inst))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML instance data (instances)!");
                    return false;
                }
                if (!in.getSint8A(ref))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML instance data (references)!");
                    return false;
                }
                if (!in.getString(host))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML instance data (host)!");
                    return false;
                }
                if (!in.getNamespaceName(ns))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML instance data (namespace)!");
                    return false;
                }
                _instanceData.append(inst);
                _referencesData.append(ref);
                _hostsData.append(host);
                _nameSpacesData.append(ns);
            }
            _size += count;
            break;
        }
        case RESP_OBJECTS:
        {
            Uint32 count;
            if (!in.getUint32(count))
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to get XML object data (number of objects)!");
                return false;
            }
            for (Uint32 i = 0; i < count; i++)
            {
                Array<Sint8> obj;
                Array<Sint8> ref;
                CIMNamespaceName ns;
                String host;
                if (!in.getSint8A(obj))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML object data (object)!");
                    return false;
                }
                if (!in.getSint8A(ref))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML object data (reference)!");
                    return false;
                }
                if (!in.getString(host))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML object data (host)!");
                    return false;
                }
                if (!in.getNamespaceName(ns))
                {
                    PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to get XML object data (namespace)!");
                    return false;
                }
                _instanceData.append(obj);
                _referencesData.append(ref);
                _hostsData.append(host);
                _nameSpacesData.append(ns);
            }
            _size += count;
            break;
        }
        // internal xml encoding of instance names and object paths not
        // done today
        case RESP_INSTNAMES:
        case RESP_OBJECTPATHS:
        default:
        {
            PEGASUS_ASSERT(false);
        }
    }
    _encoding |= RESP_ENC_XML;

    PEG_METHOD_EXIT();
    return true;
}

// Move the number of objects defined by the input parameter from
// one CIMResponse Object to another CIMResponse Object.
// Returns the new size of the CIMResponseData object.
// NOTE: This is not protected by a mutex so the user must be certain
// that the from object is not used during the move.
Uint32 CIMResponseData::moveObjects(CIMResponseData & from, Uint32 count)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMResponseData::moveObjects");

    PEG_TRACE((TRC_XML, Tracer::LEVEL3,
        "CIMResponseData::move(%u)", count));

    PEGASUS_DEBUG_ASSERT(valid());                 // KS_TEMP
    PEGASUS_DEBUG_ASSERT(_size == 0);    // Validate size == 0 or fix below
    PEGASUS_DEBUG_ASSERT(_dataType == from._dataType);

    Uint32 rtnSize = 0;
    Uint32 toMove = count;

    if (RESP_ENC_XML == (from._encoding & RESP_ENC_XML))
    {
        switch (_dataType)
        {
            case RESP_OBJECTPATHS:
            case RESP_INSTNAMES:
                break;
            case RESP_INSTANCE:
                {
                    if (from._instanceData.size() > 0)
                    {
                        // temp test to assure all sizes are the same.
                        PEGASUS_DEBUG_ASSERT(from._hostsData.size() ==
                                        from._instanceData.size());
                        PEGASUS_DEBUG_ASSERT(from._referencesData.size() ==
                                        from._instanceData.size());
                        PEGASUS_DEBUG_ASSERT(from._nameSpacesData.size() ==
                                        from._instanceData.size());
                        _instanceData.append(from._instanceData.getData(),1);
                        from._instanceData.remove(0, 1);
                        _referencesData.append(
                            from._referencesData.getData(),1);
                        from._referencesData.remove(0, 1);
                        if (_hostsData.size())
                        {
                            _hostsData.append(from._hostsData.getData(),1);
                            from._hostsData.remove(0, 1);
                        }
                        if (_nameSpacesData.size())
                        {
                            _nameSpacesData.append(
                                from._nameSpacesData.getData(),1);
                            from._nameSpacesData.remove(0, 1);
                        }
                        rtnSize += 1;
                        toMove--;
                        _encoding |= RESP_ENC_XML;
                    }
                }
                break;

            // KS-FUTURE The above could probably be folded into the following.
            // Need something like an assert if there is ever more than
            // one instance in _instanceData for type RESP_INSTANCE
            case RESP_INSTANCES:
            case RESP_OBJECTS:
                {
                    Uint32 moveCount = LOCAL_MIN(toMove,
                                                 from._instanceData.size());

                    PEGASUS_DEBUG_ASSERT(from._referencesData.size() ==
                                    from._instanceData.size());
                    _instanceData.append(from._instanceData.getData(),
                                         moveCount);
                    from._instanceData.remove(0, moveCount);
                    _referencesData.append(from._referencesData.getData(),
                                           moveCount);
                    from._referencesData.remove(0, moveCount);
                    _hostsData.append(from._hostsData.getData(),
                                         moveCount);
                    from._hostsData.remove(0, moveCount);
                    _nameSpacesData.append(from._nameSpacesData.getData(),
                                         moveCount);
                    from._nameSpacesData.remove(0, moveCount);
                    rtnSize += moveCount;
                    toMove -= moveCount;
                    _encoding |= RESP_ENC_XML;
                }
                break;
        }
    }
    if (RESP_ENC_BINARY == (from._encoding & RESP_ENC_BINARY))
    {
        // Cannot resolve this one without actually processing
        // the data since it is a stream. Concluded that we do not
        // want to do that since cost higher than gain.  Therefore we do
        // not allow this option.  Should only mean that provider agent
        // cannot generate binary for pull operations.
        rtnSize += 0;
        PEGASUS_ASSERT(false);
    }

    if (RESP_ENC_SCMO == (from._encoding & RESP_ENC_SCMO))
    {
        Uint32 moveCount = LOCAL_MIN(toMove, from._scmoInstances.size());

        _scmoInstances.append(from._scmoInstances.getData(), moveCount);
        from._scmoInstances.remove(0, moveCount);
        rtnSize += moveCount;
        toMove -= moveCount;
        _encoding |= RESP_ENC_SCMO;
    }

    if (RESP_ENC_CIM == (from._encoding & RESP_ENC_CIM))
    {
        switch (_dataType)
        {
            case RESP_OBJECTPATHS:
            case RESP_INSTNAMES:
                {
                    Uint32 moveCount = LOCAL_MIN(toMove,
                                                 from._instanceNames.size());

                    _instanceNames.append(
                        from._instanceNames.getData(), moveCount);
                    from._instanceNames.remove(0, moveCount);
                    rtnSize += moveCount;
                    toMove -= moveCount;
                    _encoding |= RESP_ENC_CIM;
                }
                break;
            case RESP_INSTANCE:
            case RESP_INSTANCES:
                {

                    Uint32 moveCount = LOCAL_MIN(toMove,
                                                 from._instances.size());

                    _instances.append(from._instances.getData(), moveCount);
                    from._instances.remove(0, moveCount);
                    rtnSize += moveCount;
                    toMove -= moveCount;
                    _encoding |= RESP_ENC_CIM;
                }
                break;
            case RESP_OBJECTS:
                {
                    Uint32 moveCount = LOCAL_MIN(toMove,
                                                 from._objects.size());
                    _objects.append(from._objects.getData(), moveCount);
                    from._objects.remove(0, moveCount);
                    rtnSize += moveCount;
                    toMove -= moveCount;
                    _encoding |= RESP_ENC_CIM;
                }
                break;
        }
    }
    PEGASUS_ASSERT(rtnSize == (count - toMove));

    _size += rtnSize;

    // Insure that _size never goes negative.  This is a
    // diagnostic. KS_TODO remove before release
    if (from._size >= rtnSize)
    {

        from._size -= rtnSize;
    }
    else
    {
        from._size = 0;
    }

    //// KS_TODO diagnostic that we should be able to remove before
    //// release
    if (rtnSize != _size)
    {
        PEG_TRACE((TRC_XML, Tracer::LEVEL1,
            "Size calc error _size %u rtnSWize = %u", _size, rtnSize));
    }
    PEG_METHOD_EXIT();
    return rtnSize;
}

Boolean CIMResponseData::hasBinaryData() const
{
    return (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY));
}
// Sets the _size variable based on the internal size counts.
void CIMResponseData::setSize()
{
    PEGASUS_DEBUG_ASSERT(valid());

    Uint32 rtnSize = 0;
    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        switch (_dataType)
        {
            case RESP_OBJECTPATHS:
            case RESP_INSTNAMES:
                break;
            case RESP_INSTANCE:
                rtnSize +=1;
                break;
            case RESP_INSTANCES:
            case RESP_OBJECTS:
                rtnSize += _instanceData.size();
                break;
        }
    }
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        // KS_PULL_TODO
        // Cannot resolve this one without actually processing
        // the data since it is a stream.
        rtnSize += 0;
    }

    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        rtnSize += _scmoInstances.size();
    }

    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        switch (_dataType)
        {
            case RESP_OBJECTPATHS:
            case RESP_INSTNAMES:
                rtnSize += _instanceNames.size();
                break;
            case RESP_INSTANCE:
            case RESP_INSTANCES:
                rtnSize += _instances.size();
                break;
            case RESP_OBJECTS:
                rtnSize += _objects.size();
                break;
        }
    }
    _size = rtnSize;
}
//
// Return the number of CIM objects in the CIM Response data object by
// aggregating sizes of each of the encodings
//
Uint32 CIMResponseData::size()
{
// If debug mode, add up all the individual size components to
// determine overall size of this object.  Then compare this with
// the _size variable.  This is a check on the completeness of the
// size computations.  We should be able to remove this at some point
// but there are many sources of size info and we need to be sure we
// have covered them all.
#ifdef CIMRESPONSEDATA_DEBUG
    PEGASUS_DEBUG_ASSERT(valid());

    Uint32 rtnSize = 0;

    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        switch (_dataType)
        {
            case RESP_OBJECTPATHS:
            case RESP_INSTNAMES:
                break;
            case RESP_INSTANCE:
                rtnSize +=1;
                break;
            case RESP_INSTANCES:
            case RESP_OBJECTS:
                rtnSize += _instanceData.size();
                break;
        }
    }
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        // KS_PULL_TODO
        // Cannot resolve this one without actually processing
        // the data since it is a stream.
        rtnSize += 0;
    }

    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        rtnSize += _scmoInstances.size();
    }

    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        switch (_dataType)
        {
            case RESP_OBJECTPATHS:
            case RESP_INSTNAMES:
                rtnSize += _instanceNames.size();
                break;
            case RESP_INSTANCE:
            case RESP_INSTANCES:
                rtnSize += _instances.size();
                break;
            case RESP_OBJECTS:
                rtnSize += _objects.size();
                break;
        }
    }
    // Test of actual count against _size variable. KS_TODO diagnostic
    Uint32 lsize = _size;
    if (rtnSize != lsize)
    {
        PEG_TRACE((TRC_XML, Tracer::LEVEL1,
        "CIMResponseData::size ERROR. debug size mismatch."
            "Computed = %u. variable = %u inc binary %s",rtnSize, _size,
         boolToString(RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
                              ));
    }
#endif
    return _size;
}

// function used by OperationAggregator to aggregate response data in a
// single ResponseData object. Adds all data in the from ResponseData object
// input variable to the target ResponseData object
// target array
void CIMResponseData::appendResponseData(const CIMResponseData & x)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::appendResponseData");

    PEGASUS_DEBUG_ASSERT(valid());

    // Confirm that the CIMResponseData type matches the type
    // of the data being appended
    // A CIMResponseData must represent a single data content type.
    // ex. Cannot mix objects and instances.

    PEGASUS_DEBUG_ASSERT(_dataType == x._dataType);
    _encoding |= x._encoding;

    // add all binary data
    _binaryData.appendArray(x._binaryData);

    // add all the C++ stuff
    _instanceNames.appendArray(x._instanceNames);
    _size += x._instanceNames.size();
    _instances.appendArray(x._instances);
    _size += x._instances.size();
    _objects.appendArray(x._objects);
    _size += x._objects.size();

    // add the SCMO instances
    _scmoInstances.appendArray(x._scmoInstances);
    _size += x._scmoInstances.size();

    _referencesData.appendArray(x._referencesData);
    _instanceData.appendArray(x._instanceData);
    _hostsData.appendArray(x._hostsData);
    _nameSpacesData.appendArray(x._nameSpacesData);
    _size += x._instanceData.size();

    // transfer property list
    _propertyList = x._propertyList;

    PEG_METHOD_EXIT();
}

// Encoding responses into output format
void CIMResponseData::encodeBinaryResponse(CIMBuffer& out)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::encodeBinaryResponse");

    // Need to do a complete job here by transferring all contained data
    // into binary format and handing it out in the CIMBuffer
    // FUTURE. For now we do not handle binary in the pull
    // operations and therefore convert back to another encoding.
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        // Binary does NOT need a marker as it consists of C++ and SCMO
        const Array<Uint8>& data = _binaryData;
        out.putBytes(data.getData(), data.size());
    }

    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        out.putTypeMarker(BIN_TYPE_MARKER_CPPD);
        switch (_dataType)
        {
            case RESP_INSTNAMES:
            {
                out.putObjectPathA(_instanceNames);
                break;
            }
            case RESP_INSTANCE:
            {
                if (0 == _instances.size())
                {
                    _instances.append(CIMInstance());
                }
                out.putInstance(_instances[0], true, true);
                break;
            }
            case RESP_INSTANCES:
            {
                out.putInstanceA(_instances);
                break;
            }
            case RESP_OBJECTS:
            {
                out.putObjectA(_objects);
                break;
            }
            case RESP_OBJECTPATHS:
            {
                out.putObjectPathA(_instanceNames);
                break;
            }
            default:
            {
                PEGASUS_DEBUG_ASSERT(false);
            }
        }
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        out.putTypeMarker(BIN_TYPE_MARKER_SCMO);
        out.putSCMOInstanceA(_scmoInstances);
    }
    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        // This actually should not happen following general code logic
        PEGASUS_DEBUG_ASSERT(false);
    }

    PEG_METHOD_EXIT();
}

void CIMResponseData::completeNamespace(const SCMOInstance * x)
{
    const char * ns;
    Uint32 len;
    ns = x->getNameSpace_l(len);
    // Both internal XML as well as binary always contain a namespace
    // don't have to do anything for those two encodings
    if ((RESP_ENC_BINARY == (_encoding&RESP_ENC_BINARY)) && (len != 0))
    {
        _defaultNamespace = CIMNamespaceName(ns);
    }
    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        CIMNamespaceName nsName(ns);
        switch (_dataType)
        {
            case RESP_INSTANCE:
            {
                if (_instances.size() > 0)
                {
                    const CIMInstance& inst = _instances[0];
                    CIMObjectPath& p =
                        const_cast<CIMObjectPath&>(inst.getPath());
                    if (p.getNameSpace().isNull())
                    {
                        p.setNameSpace(nsName);
                    }
                }
                break;
            }
            case RESP_INSTANCES:
            {
                for (Uint32 j = 0, n = _instances.size(); j < n; j++)
                {
                    const CIMInstance& inst = _instances[j];
                    CIMObjectPath& p =
                        const_cast<CIMObjectPath&>(inst.getPath());
                    if (p.getNameSpace().isNull())
                    {
                        p.setNameSpace(nsName);
                    }
                }
                break;
            }
            case RESP_OBJECTS:
            {
                for (Uint32 j = 0, n = _objects.size(); j < n; j++)
                {
                    const CIMObject& object = _objects[j];
                    CIMObjectPath& p =
                        const_cast<CIMObjectPath&>(object.getPath());
                    if (p.getNameSpace().isNull())
                    {
                        p.setNameSpace(nsName);
                    }
                }
                break;
            }
            case RESP_INSTNAMES:
            case RESP_OBJECTPATHS:
            {
                for (Uint32 j = 0, n = _instanceNames.size(); j < n; j++)
                {
                    CIMObjectPath& p = _instanceNames[j];
                    if (p.getNameSpace().isNull())
                    {
                        p.setNameSpace(nsName);
                    }
                }
                break;
            }
            default:
            {
                PEGASUS_DEBUG_ASSERT(false);
            }
        }
    }

    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        for (Uint32 j = 0, n = _scmoInstances.size(); j < n; j++)
        {
            SCMOInstance & scmoInst=_scmoInstances[j];
            if (0 == scmoInst.getNameSpace())
            {
                scmoInst.setNameSpace_l(ns,len);
            }
        }
    }
}

void CIMResponseData::completeHostNameAndNamespace(
    const String & hn,
    const CIMNamespaceName & ns,
    Boolean isPullOperation)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::completeHostNameAndNamespace");

    PEGASUS_DEBUG_ASSERT(valid());

    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        // On binary need to remember hostname and namespace in case someone
        // builds C++ default objects or Xml types later i.e.
        // -> usage: See resolveBinary()
        _defaultNamespace=ns;
        _defaultHostname=hn;
    }
    // InternalXml does not support objectPath calls
    if ((RESP_ENC_XML == (_encoding & RESP_ENC_XML)) &&
            (RESP_OBJECTS == _dataType))
    {
        for (Uint32 j = 0, n = _referencesData.size(); j < n; j++)
        {
            if (0 == _hostsData[j].size())
            {
                _hostsData[j]=hn;
            }
            if (_nameSpacesData[j].isNull())
            {
                _nameSpacesData[j]=ns;
            }
        }
    }
    // Need to set for Pull Enumeration operations
    if ((RESP_ENC_XML == (_encoding & RESP_ENC_XML)) &&
            ((RESP_INSTANCES == _dataType) || isPullOperation))
    {
        for (Uint32 j = 0, n = _referencesData.size(); j < n; j++)
        {
            if (0 == _hostsData[j].size())
            {
                _hostsData[j]=hn;
            }
            if (_nameSpacesData[j].isNull())
            {
                _nameSpacesData[j]=ns;
            }

            // KS_TODO Remove Diagnostic
            PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL4,
              "completeHostNameAndNamespace Setting hostName, etc "
              "host %s ns %s set to _hostData %s _namespaceData %s",
                  (const char *)hn.getCString(),
                  (const char *)ns.getString().getCString(),
                  (const char *)_hostsData[j].getCString(),
                  (const char *)_nameSpacesData[j].getString().getCString() ));
        }
    }

    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        switch (_dataType)
        {
            // Instances added to account for namedInstance in Pull operations.
            case RESP_INSTANCES:

                for (Uint32 j = 0, n = _instances.size(); j < n; j++)
                {
                    const CIMInstance& instance = _instances[j];
                    CIMObjectPath& p =
                        const_cast<CIMObjectPath&>(instance.getPath());
                    if (p.getHost().size()==0)
                    {
                        p.setHost(hn);
                    }
                    if (p.getNameSpace().isNull())
                    {
                        p.setNameSpace(ns);
                    }
                }
            case RESP_OBJECTS:
            {
                for (Uint32 j = 0, n = _objects.size(); j < n; j++)
                {
                    const CIMObject& object = _objects[j];
                    CIMObjectPath& p =
                        const_cast<CIMObjectPath&>(object.getPath());
                    if (p.getHost().size()==0)
                    {
                        p.setHost(hn);
                    }
                    if (p.getNameSpace().isNull())
                    {
                        p.setNameSpace(ns);
                    }
                }
                break;
            }
            // INSTNAMES added to account for instance paths in pull name
            // operations
            case RESP_INSTNAMES:
            case RESP_OBJECTPATHS:
            {
                for (Uint32 j = 0, n = _instanceNames.size(); j < n; j++)
                {
                    CIMObjectPath& p = _instanceNames[j];
                    if (p.getHost().size() == 0)
                    {
                        p.setHost(hn);
                    }
                    if (p.getNameSpace().isNull())
                    {
                        p.setNameSpace(ns);
                    }
                }
                break;
            }
            default:
            {
                PEGASUS_DEBUG_ASSERT(false);
            }
        }
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        CString hnCString=hn.getCString();
        const char* hnChars = hnCString;
        Uint32 hnLen = strlen(hnChars);
        CString nsCString=ns.getString().getCString();
        const char* nsChars=nsCString;
        Uint32 nsLen = strlen(nsChars);
        switch (_dataType)
        {
            // KS_PULL add Instances and InstNames to cover pull operations
            // KS_PULL - Confirm that this OK.
            case RESP_INSTNAMES:
            case RESP_INSTANCES:
            case RESP_OBJECTS:
            case RESP_OBJECTPATHS:
            {
                for (Uint32 j = 0, n = _scmoInstances.size(); j < n; j++)
                {
                    SCMOInstance & scmoInst=_scmoInstances[j];
                    scmoInst.completeHostNameAndNamespace(
                        hnChars,
                        hnLen,
                        nsChars,
                        nsLen);
                }
                break;
            }
            default:
            {
                PEGASUS_DEBUG_ASSERT(false);
            }
        }
    }
    PEG_METHOD_EXIT();
}

// NOTE: The reason for the isPullResponse variable is that there are
// some variations in ouput to Xml depending on whether the responses
// are one of the pull responses or not
void CIMResponseData::encodeXmlResponse(Buffer& out,
    Boolean isPullResponse,
    Boolean encodeInstanceOnly)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::encodeXmlResponse");

    PEG_TRACE((TRC_XML, Tracer::LEVEL3,
        "CIMResponseData::encodeXmlResponse(encoding=%X,dataType=%X, isPull= %s"
        " encodeInstanceOnly= %s)",
        _encoding,
        _dataType,
        boolToString(isPullResponse),
        boolToString(encodeInstanceOnly) ));

    // already existing Internal XML does not need to be encoded further
    // binary input is not actually impossible here, but we have an established
    // fallback
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        _resolveBinaryToSCMO();
    }
    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        switch (_dataType)
        {
            case RESP_INSTANCE:
            {
                const Array<ArraySint8>& a = _instanceData;
                out.append((char*)a[0].getData(), a[0].size() - 1);
                break;
            }
            case RESP_INSTANCES:
            {
                const Array<ArraySint8>& a = _instanceData;
                const Array<ArraySint8>& b = _referencesData;

                for (Uint32 i = 0, n = a.size(); i < n; i++)
                {
                    if (isPullResponse)
                    {
                        out << STRLIT("<VALUE.INSTANCEWITHPATH>\n"
                                      "<INSTANCEPATH>\n");
                        XmlWriter::appendNameSpacePathElement(out,
                            _hostsData[i],
                            _nameSpacesData[i]);
                        out.append((char*)b[i].getData(), b[i].size() - 1);
                        out << STRLIT("</INSTANCEPATH>\n");
                        out.append((char *)a[i].getData(), a[i].size() - 1);
                        out << STRLIT("</VALUE.INSTANCEWITHPATH>\n");
                    }
                    else
                    {
                        out << STRLIT("<VALUE.NAMEDINSTANCE>\n");
                        out.append((char*)b[i].getData(), b[i].size() - 1);
                        out.append((char *)a[i].getData(), a[i].size() - 1);
                        out << STRLIT("</VALUE.NAMEDINSTANCE>\n");
                    }
                }
                break;
            }
            case RESP_OBJECTS:
            {
                const Array<ArraySint8>& a = _instanceData;
                const Array<ArraySint8>& b = _referencesData;

                for (Uint32 i = 0, n = a.size(); i < n; i++)
                {
                    if (isPullResponse)
                    {
                        out << STRLIT("<VALUE.INSTANCEWITHPATH>\n");
                    }
                    else
                    {
                        out << STRLIT("<VALUE.OBJECTWITHPATH>\n");
                    }
                    out << STRLIT("<INSTANCEPATH>\n");
                    XmlWriter::appendNameSpacePathElement(
                            out,
                            _hostsData[i],
                            _nameSpacesData[i]);

                    if (isPullResponse)
                    {
                        out.append((char*)b[i].getData(),b[i].size()-1);
                    }
                    else
                    {
                        // Leave out the surrounding tags "<VALUE.REFERENCE>\n"
                        // and "</VALUE.REFERENCE>\n" which are 18 and 19
                        // characters long
                        //// KS_TODO Should be able to do this by properly
                        //// building in the CIMXmlInternalEncoder
                        out.append(
                            ((char*)b[i].getData())+18,
                            b[i].size() - 1 - 18 -19);
                    }

                    out << STRLIT("</INSTANCEPATH>\n");
                    // append instance body
                    out.append((char*)a[i].getData(), a[i].size() - 1);
                    if (isPullResponse)
                    {
                        out << STRLIT("</VALUE.INSTANCEWITHPATH>\n");
                    }
                    else
                    {
                        out << STRLIT("</VALUE.OBJECTWITHPATH>\n");
                    }
                }
                break;
            }
            // internal xml encoding of instance names and object paths not
            // done today
            case RESP_INSTNAMES:
            case RESP_OBJECTPATHS:
            default:
            {
                PEGASUS_ASSERT(false);
            }
        }
    }

    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        switch (_dataType)
        {
            case RESP_INSTNAMES:
            {
                for (Uint32 i = 0, n = _instanceNames.size(); i < n; i++)
                {
                    // Element type is different for Pull responses
                    if (isPullResponse)
                    {
                        XmlWriter::appendInstancePathElement(out,
                            _instanceNames[i]);
                    }
                    else
                    {
                        XmlWriter::appendInstanceNameElement(out,
                            _instanceNames[i]);
                    }
                }
                break;
            }
            case RESP_INSTANCE:
            {
                if (_instances.size() > 0)
                {
                    XmlWriter::appendInstanceElement(
                        out,
                        _instances[0],
                        _includeQualifiers,
                        _includeClassOrigin,
                        _propertyList);
                }
                break;
            }
            case RESP_INSTANCES:
            {
                for (Uint32 i = 0, n = _instances.size(); i < n; i++)
                {
                    if (isPullResponse)
                    {
                        if (encodeInstanceOnly)
                        {
                            XmlWriter::appendInstanceElement(
                                out,
                                _instances[i],
                                _includeQualifiers,
                                _includeClassOrigin,
                                _propertyList);
                        }
                        else
                        {
                            XmlWriter::appendValueInstanceWithPathElement(
                                out,
                                _instances[i],
                                _includeQualifiers,
                                _includeClassOrigin,
                                _propertyList);
                        }
                    }
                    else
                    {
                        XmlWriter::appendValueNamedInstanceElement(
                            out,
                            _instances[i],
                            _includeQualifiers,
                            _includeClassOrigin,
                            _propertyList);
                    }
                }
                break;
            }
            case RESP_OBJECTS:
            {
                for (Uint32 i = 0; i < _objects.size(); i++)
                {
                    // If pull, map to instances
                    if (isPullResponse)
                    {

                        CIMInstance x = (CIMInstance)_objects[i];
                        if (encodeInstanceOnly)
                        {
                            XmlWriter::appendInstanceElement(
                                out,
                                x,
                                _includeQualifiers,
                                _includeClassOrigin,
                                _propertyList);
                        }
                        else
                        {
                            XmlWriter::appendValueInstanceWithPathElement(
                                out,
                                x,
                                _includeQualifiers,
                                _includeClassOrigin,
                                _propertyList);
                        }
                    }
                    else
                    {
                        XmlWriter::appendValueObjectWithPathElement(
                            out,
                            _objects[i],
                            _includeQualifiers,
                            _includeClassOrigin,
                            _isClassOperation,
                            _propertyList);
                    }
                }
                break;
            }
            case RESP_OBJECTPATHS:
            {
                for (Uint32 i = 0, n = _instanceNames.size(); i < n; i++)
                {
                    // ObjectPaths come from providers for pull operations
                    // but are encoded as instancePathElements. If pull
                    // only instances allowed.
                    if (isPullResponse)
                    {
                        XmlWriter::appendInstancePathElement(
                            out,
                           _instanceNames[i]);
                    }
                    else
                    {
                        //Append The path element (Class or instance)
                        out << "<OBJECTPATH>\n";
                        XmlWriter::appendClassOrInstancePathElement(
                            out,
                            _instanceNames[i],
                            _isClassOperation);
                        out << "</OBJECTPATH>\n";
                    }
                }
                break;
            }
            default:
            {
                PEGASUS_ASSERT(false);
            }
        }
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        switch (_dataType)
        {
            case RESP_INSTNAMES:
            {
                for (Uint32 i = 0, n = _scmoInstances.size();i < n; i++)
                {
                    if (isPullResponse)
                    {
                        SCMOXmlWriter::appendInstancePathElement(
                           out,
                           _scmoInstances[i]);
                    }
                    else
                    {
                        SCMOXmlWriter::appendInstanceNameElement(
                            out,
                            _scmoInstances[i]);
                    }
                }
                break;
            }
            case RESP_INSTANCE:
            {
                if (_scmoInstances.size() > 0)
                {
                    _appendInstanceElement(out, _scmoInstances[0]);
                }
                break;
            }
            case RESP_INSTANCES:
            {
                if (isPullResponse)
                {
                    // pull and encodeInstanceOnly (i.e. response to
                    // OpenQueryInstances and pullInstances
                    if (encodeInstanceOnly)
                    {
                        // KS_FUTURE move this to SCMOXmlWriter
                        for (Uint32 i = 0, n = _scmoInstances.size();i < n; i++)
                        {
                            _appendInstanceElement(out, _scmoInstances[i]);
                        }
                    }
                    else
                    {
                        SCMOXmlWriter::appendValueSCMOInstanceWithPathElements(
                            out, _scmoInstances, _propertyList);
                    }
                }
                else
                {
                    SCMOXmlWriter::appendValueSCMOInstanceElements(
                        out, _scmoInstances, _propertyList);
                }
                break;
            }
            case RESP_OBJECTS:
            {
                if (isPullResponse)
                {
                    // if encodeInstanceOnly flag, encode objects as instances
                    // Used by OpenQueryInstances and pullInstances.
                    if (encodeInstanceOnly)
                    {
                        for (Uint32 i = 0, n = _scmoInstances.size();i < n; i++)
                        {
                            _appendInstanceElement(out, _scmoInstances[i]);
                        }
                    }
                    else
                    {
                        SCMOXmlWriter::appendValueSCMOInstanceWithPathElements(
                            out,_scmoInstances, _propertyList);
                    }
                }
                else
                {
                    // KS_TODO why is this one named element rather than
                    // elements
                    SCMOXmlWriter::appendValueObjectWithPathElement(
                        out, _scmoInstances, _propertyList);
                }
                break;
            }
            case RESP_OBJECTPATHS:
            {
                for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
                {
                    if (isPullResponse)
                    {
                        SCMOXmlWriter::appendInstancePathElement(out,
                            _scmoInstances[i]);
                    }
                    else
                    {
                        out << "<OBJECTPATH>\n";
                        SCMOXmlWriter::appendClassOrInstancePathElement(
                            out, _scmoInstances[i]);
                        out << "</OBJECTPATH>\n";
                    }
                }
                break;
            }
            default:
            {
                PEGASUS_ASSERT(false);
            }
        }
    }
    PEG_METHOD_EXIT();
}

// contrary to encodeXmlResponse this function encodes the Xml in a format
// not usable by clients
void CIMResponseData::encodeInternalXmlResponse(CIMBuffer& out,
    Boolean isPullOperation)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::encodeInternalXmlResponse");

    PEG_TRACE((TRC_XML, Tracer::LEVEL3,
        "CIMResponseData::encodeInternalXmlResponse(encoding=%X,content=%X"
        " isPullOperation=%s)",
        _encoding,
        _dataType,
        boolToString(isPullOperation)));

    // For mixed (CIM+SCMO) responses, we need to tell the receiver the
    // total number of instances. The totalSize variable is used to keep track
    // of this.
    Uint32 totalSize = 0;

    // already existing Internal XML does not need to be encoded further
    // binary input is not actually impossible here, but we have an established
    // fallback
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        _resolveBinaryToSCMO();
    }
    if ((0 == _encoding) ||
        (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM)))
    {
        switch (_dataType)
        {
            case RESP_INSTANCE:
            {
                if (0 == _instances.size())
                {
                    _instances.append(CIMInstance());
                    CIMInternalXmlEncoder::_putXMLInstance(
                        out,
                        _instances[0]);
                    break;
                }
                CIMInternalXmlEncoder::_putXMLInstance(
                    out,
                    _instances[0],
                    _includeQualifiers,
                    _includeClassOrigin,
                    _propertyList);
                break;
            }
            case RESP_INSTANCES:
            {
                Uint32 n = _instances.size();
                totalSize = n + _scmoInstances.size();
                out.putUint32(totalSize);
                for (Uint32 i = 0; i < n; i++)
                {
                    CIMInternalXmlEncoder::_putXMLNamedInstance(
                        out,
                        _instances[i],
                        _includeQualifiers,
                        _includeClassOrigin,
                        _propertyList);
                }
                break;
            }
            case RESP_OBJECTS:
            {
                Uint32 n = _objects.size();
                totalSize = n + _scmoInstances.size();
                out.putUint32(totalSize);
                for (Uint32 i = 0; i < n; i++)
                {
                    // if is pull map to instances.
                    if (isPullOperation)
                    {
                        CIMInternalXmlEncoder::_putXMLNamedInstance(
                            out,
                            (CIMInstance)_objects[i],
                            _includeQualifiers,
                            _includeClassOrigin,
                            _propertyList);
                    }
                    else
                    {
                        CIMInternalXmlEncoder::_putXMLObject(
                            out,
                            _objects[i],
                            _includeQualifiers,
                            _includeClassOrigin,
                            _propertyList);
                    }
                }
                break;
            }
            // internal xml encoding of instance names and object paths not
            // done today
            case RESP_INSTNAMES:
            case RESP_OBJECTPATHS:
            default:
            {
                PEGASUS_DEBUG_ASSERT(false);
            }
        }
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        switch (_dataType)
        {
            case RESP_INSTANCE:
            {
                if (0 == _scmoInstances.size())
                {
                    _scmoInstances.append(SCMOInstance());
                }
                SCMOInternalXmlEncoder::_putXMLInstance(
                    out,
                    _scmoInstances[0],
                    _propertyList);
                break;
            }
            case RESP_INSTANCES:
            {
                Uint32 n = _scmoInstances.size();
                // Only put the size when not already done above
                if (0==totalSize)
                {
                    out.putUint32(n);
                }
                SCMOInternalXmlEncoder::_putXMLNamedInstance(
                    out,
                    _scmoInstances,
                    _propertyList);
                break;
            }
            case RESP_OBJECTS:
            {
                Uint32 n = _scmoInstances.size();
                // Only put the size when not already done above
                if (0==totalSize)
                {
                    out.putUint32(n);
                }
                    // if is pull map to instances.
                if (isPullOperation)
                {
                    SCMOInternalXmlEncoder::_putXMLNamedInstance(
                        out,
                        _scmoInstances,
                        _propertyList);
                }
                else
                {
                    SCMOInternalXmlEncoder::_putXMLObject(
                        out,
                        _scmoInstances,
                        _propertyList);
                }
                break;
            }
            // internal xml encoding of instance names and object paths not
            // done today
            case RESP_INSTNAMES:
            case RESP_OBJECTPATHS:
            default:
            {
                PEGASUS_DEBUG_ASSERT(false);
            }
        }
    }
    PEG_METHOD_EXIT();
}

void CIMResponseData::_resolveToCIM()
{
    PEG_TRACE((TRC_XML, Tracer::LEVEL3,
        "CIMResponseData::_resolveToCIM(encoding=%X,content=%X)",
        _encoding,
        _dataType));

    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        _resolveXmlToCIM();
    }
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        _resolveBinaryToSCMO();
    }
    if (RESP_ENC_SCMO == (_encoding & RESP_ENC_SCMO))
    {
        _resolveSCMOToCIM();
    }

    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_CIM || _encoding == 0);
}

// Resolve any binary data to SCMO. This externalfunction added because we
// cannot do a move on Binary data so convert it a to movable format
void CIMResponseData::resolveBinaryToSCMO()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::resolveBinaryToSCMO");
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        _resolveBinaryToSCMO();
    }
    PEG_METHOD_EXIT();
}

void CIMResponseData::_resolveToSCMO()
{
    PEG_TRACE((TRC_XML, Tracer::LEVEL3,
        "CIMResponseData::_resolveToSCMO(encoding=%X,content=%X)",
        _encoding,
        _dataType));

    if (RESP_ENC_XML == (_encoding & RESP_ENC_XML))
    {
        _resolveXmlToSCMO();
    }
    if (RESP_ENC_BINARY == (_encoding & RESP_ENC_BINARY))
    {
        _resolveBinaryToSCMO();
    }
    if (RESP_ENC_CIM == (_encoding & RESP_ENC_CIM))
    {
        _resolveCIMToSCMO();
    }
    PEGASUS_DEBUG_ASSERT(_encoding == RESP_ENC_SCMO || _encoding == 0);
}

// helper functions to transform different formats into one-another
// functions work on the internal data and calling of them should be
// avoided whenever possible
void CIMResponseData::_resolveBinaryToSCMO()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::_resolveBinaryToSCMO");

    CIMBuffer in((char*)_binaryData.getData(), _binaryData.size());

    while (in.more())
    {
        Uint32 binaryTypeMarker=0;
        if(!in.getTypeMarker(binaryTypeMarker))
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Failed to get type marker for binary objects!");
            PEG_METHOD_EXIT();
            in.release();
            return;
        }

        if (BIN_TYPE_MARKER_SCMO==binaryTypeMarker)
        {
            if (!in.getSCMOInstanceA(_scmoInstances))
            {
                _encoding &=(~RESP_ENC_BINARY);
                in.release();
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Failed to resolve binary SCMOInstances!");
                PEG_METHOD_EXIT();
                return;
            }

            _encoding |= RESP_ENC_SCMO;
        }
        else
        {
            switch (_dataType)
            {
                case RESP_INSTNAMES:
                case RESP_OBJECTPATHS:
                {
                    if (!in.getObjectPathA(_instanceNames))
                    {
                        _encoding &=(~RESP_ENC_BINARY);
                        in.release();
                        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                            "Failed to resolve binary CIMObjectPaths!");
                        PEG_METHOD_EXIT();
                        return;
                    }
                    break;
                }
                case RESP_INSTANCE:
                {
                    CIMInstance instance;
                    if (!in.getInstance(instance))
                    {
                        _encoding &=(~RESP_ENC_BINARY);
                        _encoding |= RESP_ENC_CIM;
                        _instances.append(instance);
                        in.release();
                        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                            "Failed to resolve binary instance!");
                        PEG_METHOD_EXIT();
                        return;
                    }

                    _instances.append(instance);
                    break;
                }
                case RESP_INSTANCES:
                {
                    if (!in.getInstanceA(_instances))
                    {
                        _encoding &=(~RESP_ENC_BINARY);
                        in.release();
                        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                            "Failed to resolve binary CIMInstances!");
                        PEG_METHOD_EXIT();
                        return;
                    }
                    break;
                }
                case RESP_OBJECTS:
                {
                    if (!in.getObjectA(_objects))
                    {
                        in.release();
                        _encoding &=(~RESP_ENC_BINARY);
                        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                            "Failed to resolve binary CIMObjects!");
                        PEG_METHOD_EXIT();
                        return;
                    }
                    break;
                }
                default:
                {
                    PEGASUS_DEBUG_ASSERT(false);
                }
            } // switch
            _encoding |= RESP_ENC_CIM;
        } // else SCMO
    }
    _encoding &=(~RESP_ENC_BINARY);
    // fix up the hostname and namespace for objects if defaults
    // were set
    if (_defaultHostname.size() > 0 && !_defaultNamespace.isNull())
    {
        completeHostNameAndNamespace(_defaultHostname, _defaultNamespace);
    }
    in.release();
    PEG_METHOD_EXIT();
}


void CIMResponseData::_deserializeObject(Uint32 idx,CIMObject& cimObject)
{

    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::_deserializeObject");
    // Only start the parser when instance data is present.
    if (0 != _instanceData[idx].size())
    {
        CIMInstance cimInstance;
        CIMClass cimClass;

        XmlParser parser((char*)_instanceData[idx].getData());

        if (XmlReader::getInstanceElement(parser, cimInstance))
        {
            cimObject = CIMObject(cimInstance);
            return;
        }

        if (XmlReader::getClassElement(parser, cimClass))
        {
            cimObject = CIMObject(cimClass);
            return;
        }
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to resolve XML object data, parser error!");
    }
    PEG_METHOD_EXIT();
}

void CIMResponseData::_deserializeInstance(Uint32 idx,CIMInstance& cimInstance)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::_deserializeInstance");
    // Only start the parser when instance data is present.
    if (0 != _instanceData[idx].size())
    {
        XmlParser parser((char*)_instanceData[idx].getData());
        if (XmlReader::getInstanceElement(parser, cimInstance))
        {
            return;
        }
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to resolve XML instance, parser error!");
    }
    // reset instance when parsing may not be successfull or
    // no instance is present.
    cimInstance = CIMInstance();

    PEG_METHOD_EXIT();
}

Boolean CIMResponseData::_deserializeReference(
    Uint32 idx,
    CIMObjectPath& cimObjectPath)
{
    // Only start the parser when reference data is present.
    if (0 != _referencesData[idx].size())
    {
        XmlParser parser((char*)_referencesData[idx].getData());
        if (XmlReader::getValueReferenceElement(parser, cimObjectPath))
        {
            if (_hostsData[idx].size())
            {
                cimObjectPath.setHost(_hostsData[idx]);
            }
            if (!_nameSpacesData[idx].isNull())
            {
                cimObjectPath.setNameSpace(_nameSpacesData[idx]);
            }
            return true;
        }
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to resolve XML reference, parser error!");

    }
    return false;
}

Boolean CIMResponseData::_deserializeInstanceName(
    Uint32 idx,
    CIMObjectPath& cimObjectPath)
{
    // Only start the parser when instance name data is present.
    if (0 != _referencesData[idx].size())
    {
        XmlParser parser((char*)_referencesData[idx].getData());
        if (XmlReader::getInstanceNameElement(parser, cimObjectPath))
        {
            if (_hostsData[idx].size())
            {
                cimObjectPath.setHost(_hostsData[idx]);
            }
            if (!_nameSpacesData[idx].isNull())
            {
                cimObjectPath.setNameSpace(_nameSpacesData[idx]);
            }
            return true;
        }
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Failed to resolve XML instance name, parser error!");

    }
    return false;
}

void CIMResponseData::_resolveXmlToCIM()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::_resolveXmlToCIM");

    switch (_dataType)
    {
        // Xml encoding for instance names and object paths not used
        case RESP_OBJECTPATHS:
        case RESP_INSTNAMES:
        {
            break;
        }
        case RESP_INSTANCE:
        {
            CIMInstance cimInstance;
            CIMObjectPath cimObjectPath;

            _deserializeInstance(0,cimInstance);
            if (_deserializeReference(0,cimObjectPath))
            {
                cimInstance.setPath(cimObjectPath);
                // A single CIMInstance has to have an objectpath.
                // So only add it when an objectpath exists.
                _instances.append(cimInstance);
            }
            break;
        }
        case RESP_INSTANCES:
        {
            for (Uint32 i = 0; i < _instanceData.size(); i++)
            {
                CIMInstance cimInstance;
                CIMObjectPath cimObjectPath;

                _deserializeInstance(i,cimInstance);
                if (_deserializeInstanceName(i,cimObjectPath))
                {
                    cimInstance.setPath(cimObjectPath);
                }
                // enumarate instances can be without name
                _instances.append(cimInstance);
            }
            break;
        }
        case RESP_OBJECTS:
        {
            for (Uint32 i=0, n=_instanceData.size(); i<n; i++)
            {
                CIMObject cimObject;
                CIMObjectPath cimObjectPath;

                _deserializeObject(i,cimObject);
                if (_deserializeReference(i,cimObjectPath))
                {
                    cimObject.setPath(cimObjectPath);
                }
                _objects.append(cimObject);
            }
            break;
        }
        default:
        {
            PEGASUS_ASSERT(false);
        }
    }
    // Xml was resolved, release Xml content now
    _referencesData.clear();
    _hostsData.clear();
    _nameSpacesData.clear();
    _instanceData.clear();
    // remove Xml Encoding flag
    _encoding &=(~RESP_ENC_XML);
    // add CIM Encoding flag
    _encoding |=RESP_ENC_CIM;

    PEG_METHOD_EXIT();
}

void CIMResponseData::_resolveXmlToSCMO()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::_resolveXmlToSCMO");
    // Not optimal, can probably be improved
    // but on the other hand, since using the binary format this case should
    // actually not ever happen.
    _resolveXmlToCIM();
    _resolveCIMToSCMO();

    PEG_METHOD_EXIT();
}

void CIMResponseData::_resolveSCMOToCIM()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::_resolveSCMOToCIM");
    switch(_dataType)
    {
        case RESP_INSTNAMES:
        case RESP_OBJECTPATHS:
        {
            for (Uint32 x=0, n=_scmoInstances.size(); x < n; x++)
            {
                CIMObjectPath newObjectPath;
                _scmoInstances[x].getCIMObjectPath(newObjectPath);
                _instanceNames.append(newObjectPath);
            }
            break;
        }
        case RESP_INSTANCE:
        {
            if (_scmoInstances.size() > 0)
            {
                CIMInstance newInstance;
                _scmoInstances[0].getCIMInstance(newInstance);
                _instances.append(newInstance);
            }
            break;
        }
        case RESP_INSTANCES:
        {
            for (Uint32 x=0, n=_scmoInstances.size(); x < n; x++)
            {
                CIMInstance newInstance;
                _scmoInstances[x].getCIMInstance(newInstance);
                _instances.append(newInstance);
            }
            break;
        }
        case RESP_OBJECTS:
        {
            for (Uint32 x=0, n=_scmoInstances.size(); x < n; x++)
            {
                CIMInstance newInstance;
                _scmoInstances[x].getCIMInstance(newInstance);
                _objects.append(CIMObject(newInstance));
            }
            break;
        }
        default:
        {
            PEGASUS_DEBUG_ASSERT(false);
        }
    }
    _scmoInstances.clear();
    // remove CIM Encoding flag
    _encoding &=(~RESP_ENC_SCMO);
    // add SCMO Encoding flag
    _encoding |=RESP_ENC_CIM;

    PEG_METHOD_EXIT();
}

void CIMResponseData::_resolveCIMToSCMO()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMResponseData::_resolveCIMToSCMO");
    CString nsCString=_defaultNamespace.getString().getCString();
    const char* _defNamespace = nsCString;
    Uint32 _defNamespaceLen;
    if (_defaultNamespace.isNull())
    {
        _defNamespaceLen=0;
    }
    else
    {
        _defNamespaceLen=strlen(_defNamespace);
    }
    switch (_dataType)
    {
        case RESP_INSTNAMES:
        {
            for (Uint32 i=0,n=_instanceNames.size();i<n;i++)
            {
                SCMOInstance addme(
                    _instanceNames[i],
                    _defNamespace,
                    _defNamespaceLen);
                _scmoInstances.append(addme);
            }
            _instanceNames.clear();
            break;
        }
        case RESP_INSTANCE:
        {
            if (_instances.size() > 0)
            {
                SCMOInstance addme(
                    _instances[0],
                    _defNamespace,
                    _defNamespaceLen);
                _scmoInstances.clear();
                _scmoInstances.append(addme);
                _instances.clear();
            }
            break;
        }
        case RESP_INSTANCES:
        {
            for (Uint32 i=0,n=_instances.size();i<n;i++)
            {
                SCMOInstance addme(
                    _instances[i],
                    _defNamespace,
                    _defNamespaceLen);
                _scmoInstances.append(addme);
            }
            _instances.clear();
            break;
        }
        case RESP_OBJECTS:
        {
            for (Uint32 i=0,n=_objects.size();i<n;i++)
            {
                SCMOInstance addme(
                    _objects[i],
                    _defNamespace,
                    _defNamespaceLen);
                _scmoInstances.append(addme);
            }
            _objects.clear();
            break;
        }
        case RESP_OBJECTPATHS:
        {
            for (Uint32 i=0,n=_instanceNames.size();i<n;i++)
            {
                SCMOInstance addme(
                    _instanceNames[i],
                    _defNamespace,
                    _defNamespaceLen);
                if (_isClassOperation)
                {
                    addme.setIsClassOnly(true);
                }
                _scmoInstances.append(addme);
            }
            _instanceNames.clear();
            break;
        }
        default:
        {
            PEGASUS_DEBUG_ASSERT(false);
        }
    }

    // remove CIM Encoding flag
    _encoding &=(~RESP_ENC_CIM);
    // add SCMO Encoding flag
    _encoding |=RESP_ENC_SCMO;

    PEG_METHOD_EXIT();
}

/**
 * Validate the magic object for this CIMResponseData. This
 * compiles only in debug mode and can be use to validate the
 * CIMResponseData object
 *
 * @return Boolean True if valid object.
 */
Boolean CIMResponseData::valid() const
{
    return _magic;
}

void CIMResponseData::setRequestProperties(
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    _includeQualifiers = includeQualifiers;
    _includeClassOrigin = includeClassOrigin;
    _propertyList = propertyList;
}

void CIMResponseData::setIsClassOperation(Boolean b)
{
    _isClassOperation = b;
}

// Clear all of the input encodings by clearing their arrays and
// unsetting the encoding flag.
void CIMResponseData::clear()
{
    // Clear the xml data area
    _referencesData.clear();
    _hostsData.clear();
    _nameSpacesData.clear();
    _instanceData.clear();

    // Clear the binary data area
    _binaryData.clear();

    // Clear the SCMO data
    _scmoInstances.clear();

    //Clear the C++ Data areaa
    _instanceNames.clear();
    _instances.clear();
    _objects.clear();

    _encoding = 0;
    _size = 0;
}

// The following are debugging support only
//// FUTURE: Make this conditional compile
void CIMResponseData::traceResponseData() const
{
    PEG_TRACE((TRC_XML, Tracer::LEVEL3,
        "%s", (const char*)toStringTraceResponseData().getCString() ));
}

String CIMResponseData::toStringTraceResponseData() const
{
    String rtnStr;
    rtnStr.appendPrintf(
        "CIMResponseData::traceResponseData(encoding=%X,dataType=%X "
        " size=%u C++instNamecount=%u c++Instances=%u c++Objects=%u "
        "scomInstances=%u XMLInstData=%u binaryData=%u "
        "xmlref=%u xmlinst=%u, xmlhost=%u xmlns=%u",
        _encoding,_dataType, _size,
        _instanceNames.size(),_instances.size(), _objects.size(),
        _scmoInstances.size(),_instanceData.size(),_binaryData.size(),
        _referencesData.size(), _instanceData.size(), _hostsData.size(),
        _nameSpacesData.size());

    return(rtnStr);
}



PEGASUS_NAMESPACE_END
