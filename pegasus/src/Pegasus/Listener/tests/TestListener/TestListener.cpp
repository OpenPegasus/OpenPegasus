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

#include <string.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/FileSystem.h>
#ifdef PEGASUS_HAS_SSL
#include <Pegasus/Common/SSLContext.h>
#endif
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Listener/CIMListener.h>


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

////////////////////////////////////////////////////////////////////////////////
//
// MyIndicationConsumer
//
////////////////////////////////////////////////////////////////////////////////
class MyIndicationConsumer : public CIMIndicationConsumer
{
public:
    MyIndicationConsumer(const String& name_);
    ~MyIndicationConsumer();

    void consumeIndication(const OperationContext& context,
        const String & url,
        const CIMInstance& indicationInstance);

private:
    String name;

};

MyIndicationConsumer::MyIndicationConsumer(const String& name_)
{
    this->name = name_;
}

MyIndicationConsumer::~MyIndicationConsumer()
{
}

void MyIndicationConsumer::consumeIndication(
        const OperationContext & context,
        const String & url,
      const CIMInstance& indicationInstance)
    {
    String msg = "Consumer <" + name + "> received " +
        indicationInstance.getPath().toString();

    PEG_TRACE_CSTRING(TRC_LISTENER,Tracer::LEVEL4,
        (const char*)msg.getCString());
    PEGASUS_STD(cerr) << msg << PEGASUS_STD(endl);
    }

////////////////////////////////////////////////////////////////////////////////
//
// MyTraceSettings
//
///////////////////////////////////////////////////////////////////////////////
class MyTraceSettings
{
public:
    static void setTraceComponents(String traceComponents);

    static String _traceFileName;
};

String MyTraceSettings::_traceFileName = "cimlistener.trc";

void MyTraceSettings::setTraceComponents(String traceComponents)
{
    // set trace path
    const char* tmp = getenv("PEGASUS_HOME");
    String pegasusHome = tmp;

    // set logger directory
    String logsDirectory = pegasusHome + String("/logs");
    Logger::setHomeDirectory(logsDirectory);
    Logger::setlogLevelMask("TRACE");

    // Set the file path to  $PEGASUS_HOME directory
  String traceFilePath = pegasusHome + String("/") + _traceFileName;


    CString fileName = traceFilePath.getCString();
    if (Tracer::isValidFileName(fileName))
    {
        cout << "setTraceFile: " << (const char*)fileName << endl;

        Uint32 retCode = Tracer::setTraceFile(fileName);
      // Check whether the filepath was set
      if(retCode == 1)
      {
            cout << "Unable to write to trace file: " << fileName << endl;

            Logger::put(
                Logger::ERROR_LOG,
                System::CIMLISTENER,
                Logger::WARNING,
                "Unable to write to trace file $0",
                (const char*)fileName);
    }
    }
    // set trace level
  Tracer::setTraceLevel(Tracer::LEVEL4);

    // set trace components
    Tracer::setTraceComponents(traceComponents);
}

////////////////////////////////////////////////////////////////////////////////
//
// main()
//
////////////////////////////////////////////////////////////////////////////////

int main()
{
    //String traceComponents = "Http,XmlIO";
    String traceComponents = "Listener";
    MyTraceSettings::setTraceComponents(traceComponents);

    try
    {

        int portNumber = 2003;
        CIMListener listener(portNumber);

        // add cosumer
        MyIndicationConsumer* consumer1 = new MyIndicationConsumer("1");
        listener.addConsumer(consumer1);

        MyIndicationConsumer* consumer2 = new MyIndicationConsumer("2");
        listener.addConsumer(consumer2);

        // start listener
        listener.start();

        cout << "type \"exit\" to end listener." << endl;
        char buf[255]={0};
        while(true)
        {
            cin.getline(buf,255);

            if(strlen(buf)>0 && strcmp(buf,"exit")==0)
                break;
        }

        delete consumer1;
        delete consumer2;

        listener.stop();
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        return 1;
    }

#ifdef PEGASUS_HAS_SSL
    try
    {
        const char *pegasusHome = getenv("PEGASUS_HOME");
        PEGASUS_TEST_ASSERT(pegasusHome);

        int portNumber = 2008;
        CIMListener listener(portNumber);

        // add cosumer
        MyIndicationConsumer* sslConsumer = new MyIndicationConsumer("ssl");

        String trustPath = FileSystem::getAbsolutePath(pegasusHome,
                PEGASUS_SSLCLIENT_CERTIFICATEFILE);
        String randFile;
# ifdef PEGASUS_SSL_RANDOMFILE
        randFile = FileSystem::getAbsolutePath(pegasusHome,
                PEGASUS_SSLCLIENT_RANDOMFILE);
# endif

        // listener destructor will delete it.
        SSLContext *sslContext = new SSLContext(trustPath,
                String::EMPTY, String::EMPTY,
                String::EMPTY, 0, randFile);

        listener.setSSLContext(sslContext);
        listener.addConsumer(sslConsumer);

        cout << "testing SSL listener." << endl;
        // start listener
        listener.start();

        cout << "type \"exit\" to end listener." << endl;
        char buf[255]={0};
        while(true)
        {
            cin.getline(buf,255);

            if(strlen(buf)>0 && strcmp(buf,"exit")==0)
                break;
        }

        delete sslConsumer;

        listener.stop();
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        return 1;
    }
#endif

    return 0;
}
