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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/ExportClient/WSMANExportClient.h>
#include <Pegasus/WsmServer/WsmRequest.h>
#include <Pegasus/WsmServer/WsmWriter.h>
#include <Pegasus/WsmServer/CimToWsmResponseMapper.h>
#include <Pegasus/ExportClient/WSMANExportRequestEncoder.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN
class TestRequestEncoder
{
    public:
    void testWSMANReqEncoder();
};

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

const String NAMESPACE = "root/cimv2";

void TestRequestEncoder::testWSMANReqEncoder()
{
    CIMInstance indication(CIMName("My_IndicationClass"));
    indication.addProperty(CIMProperty(CIMName("DeviceName"), String("Disk")));
    indication.addProperty(CIMProperty(CIMName("DeviceId"), Uint32(1)));

    CimToWsmResponseMapper wsmMapper;
    WsmInstance wsmInstance;
    // Dummy values
    String url = "Dummy";
    String toPath = "Dummy";
    char portStr[32] = "5989";
    String connectHost = "Dummy";
    HTTPConnection* httpConnection = NULL;
    ClientAuthenticator authenticator;

    wsmMapper.convertCimToWsmInstance(url, indication, 
        wsmInstance, PEGASUS_INSTANCE_NS);
    AutoPtr<WsmRequest> request(new WsExportIndicationRequest(
        "uuid:6B8B4567-23C6-9869-4873-74B0DC515CFF",
        url,
        toPath,
        wsmInstance));
    
    AutoPtr<WSMANExportRequestEncoder> requestEncoder(
        new WSMANExportRequestEncoder(
            httpConnection,
            connectHost,
            portStr,
            &authenticator));
    deliveryMode mode = Push; 
    requestEncoder.get()->setDeliveryMode(mode);
    Buffer copyBuf, masterBuf;
    const char* master="<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/"
        "2003/05/soap-envelope\" xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/"
        "soap-encoding\" xmlns:xml=\"http://www.w3.org/XML/1998/namespace\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd="
        "\"http://www.w3.org/2001/XMLSchema\" xmlns:wsman=\"http://schemas."
        "dmtf.org/wbem/wsman/1/wsman.xsd\" xmlns:wsmb=\"http://schemas.dmtf"
        ".org/wbem/wsman/1/cimbinding.xsd\" xmlns:wsa=\"http://schemas."
        "xmlsoap.org/ws/2004/08/addressing\" xmlns:wxf=\"http://schemas."
        "xmlsoap.org/ws/2004/09/transfer\" xmlns:wsen=\"http://schemas."
        "xmlsoap.org/ws/2004/09/enumeration\" xmlns:wse=\"http://schemas."
        "xmlsoap.org/ws/2004/08/eventing\" xmlns:wsp=\"http://schemas.xmlsoap"
        ".org/ws/2004/09/policy\" xmlns:wsdl=\"http://schemas.xmlsoap.org/"
        "wsdl\" xmlns:wscim=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema"
        "/2\"><SOAP-ENV:Header><wsa:To>Dummy</wsa:To><wsa:Action>http://"
        "schemas.dmtf.org/wbem/wsman/1/wsman/Event</wsa:Action><wsa:MessageID"
        ">uuid:6B8B4567-23C6-9869-4873-74B0DC515CFF</wsa:MessageID></SOAP-ENV"
        ":Header><SOAP-ENV:Body><p:My_IndicationClass xmlns:p=\"http://"
        "schemas.dmtf.org/wbem/wscim/1/cim-schema/2/My_IndicationClass\">"
        "<p:DeviceId>1</p:DeviceId><p:DeviceName>Disk</p:DeviceName>"
        "</p:My_IndicationClass></SOAP-ENV:Body></SOAP-ENV:Envelope>";

    masterBuf = Buffer(master, strlen(master));
    requestEncoder.get()->_encodeWSMANIndication(
        (WsExportIndicationRequest*)request.get(), copyBuf);
    PEGASUS_TEST_ASSERT(masterBuf == copyBuf);
}

int main()
{
    TestRequestEncoder test1 = TestRequestEncoder();
    test1.testWSMANReqEncoder();
    cout << "+++++ passed all tests" << endl;
    return 0;
}

