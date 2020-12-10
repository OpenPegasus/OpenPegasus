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
#include <string.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/General/OptionManager.h>

#include <Pegasus/DynListener/DynamicListener.h>
#include <Pegasus/DynListener/DynamicListenerConfig.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main()
{
    //set logging options
    String currentDirectory;
    FileSystem::getCurrentDirectory(currentDirectory);

    //you MUST translate slashes or the logger will fail on Windows
    FileSystem::translateSlashes(currentDirectory);
    Logger::setHomeDirectory(currentDirectory);

    //must list these in descending order or the logger will fail
    Logger::setlogLevelMask("FATAL");
    Logger::setlogLevelMask("SEVERE");
    Logger::setlogLevelMask("WARNING");

    //ATTN: Need to be able to specify a different conf file as cmdline arg
    String configFile = "cimlistener.conf";

    try
    {
        DynamicListenerConfig config;

        String test = FileSystem::getAbsolutePath("","");
        DynamicListenerConfig::setListenerHome(currentDirectory);
        config.initOptions(configFile);

        Uint32 listenerPort;
        Boolean httpsConnection;
        String sslKeyFilePath;
        String sslCertificateFilePath;
        String consumerDir;
        String consumerConfigDir;
        Boolean enableConsumerUnload;
        Uint32 consumerIdleTimeout;
        Uint32 shutdownTimeout;
        String traceFile;
        Uint32 traceLevel;
        String traceComponents;

        printf(
            "Home dir: %s\n",
            (const char*)
                DynamicListenerConfig::getListenerHome().getCString());

        if (!config.lookupIntegerValue("listenerPort", listenerPort))
        {
            printf("Invalid port number");
        }

        httpsConnection = config.isTrue("enableHttpsListenerConnection");

        if (!config.lookupValue("sslKeyFilePath", sslKeyFilePath))
        {
            //ATTN:confirm path?
            printf("Invalid key file path");
        }

        if (!config.lookupValue(
                 "sslCertificateFilePath",
                 sslCertificateFilePath))
        {
            //ATTN:confirm path?
            printf("Invalid certificate file path");
        }

        if (!config.lookupValue("consumerDir", consumerDir))
        {
            //ATTN:confirm path?
            printf("Invalid directory");
        }

        if (!config.lookupValue("consumerConfigDir", consumerConfigDir))
        {
            //ATTN:confirm path?
            printf("Invalid directory");
        }

        enableConsumerUnload = config.isTrue("enableConsumerUnload");

        if (!config.lookupIntegerValue(
                 "consumerIdleTimeout",
                 consumerIdleTimeout))
        {
            printf("Invalid consumerIdleTimeout");
        }

        if (!config.lookupValue("traceFilePath", traceFile))
        {
            printf("Invalid directory");
        }

        if (!config.lookupIntegerValue(
                 "consumerIdleTimeout",
                 consumerIdleTimeout))
        {
            printf("Invalid consumerIdleTimeout");
        }

        if (!config.lookupIntegerValue(
                 "shutdownTimeout",
                 shutdownTimeout))
        {
            printf("Invalid consumerIdleTimeout");
        }

        if (!config.lookupValue("traceFilePath", traceFile))
        {
            //ATTN:confirm path?
            printf("Invalid traceFilePath");
        }

        if (!config.lookupIntegerValue("traceLevel", traceLevel))
        {
            printf("Invalid traceLevel");
        }

        if (!config.lookupValue("traceComponents", traceComponents))
        {
            printf("Invalid traceComponents");
        }

        //log startup options
        printf("Starting CIMListener with the following options\n");
        printf("\tlistenerPort %u\n", listenerPort);
        printf("\thttpsConnection %d\n", httpsConnection);
        printf("\tsslKeyFilePath %s\n",
               (const char*)sslKeyFilePath.getCString());
        printf("\tsslCertificateFilePath %s\n",
               (const char*)sslCertificateFilePath.getCString());
        printf("\tconsumerDir %s\n",
               (const char*)consumerDir.getCString());
        printf("\tconsumerConfigDir %s\n",
               (const char*)consumerConfigDir.getCString());
        printf("\tenableConsumerUnload %d\n", enableConsumerUnload);
        printf("\tconsumerIdleTimeout %u\n", consumerIdleTimeout);
        printf("\tshutdownTimeout %u\n", shutdownTimeout);
        printf("\ttraceFilePath %s\n", (const char*)traceFile.getCString());
        printf("\ttraceLevel %u\n", traceLevel);
        printf("\ttraceComponents %s\n",
               (const char*)traceComponents.getCString());
        printf("\n\nType \"exit\" to shutdown the listener gracefully.\n");

        PEGASUS_STD(cout) << "Testing tracing\n";
        //keep tracing in configuration, not in listener itself
        if (traceLevel > 0)
        {
            Uint32 traceLevelArg;

            switch (traceLevel)
            {
            case 1: traceLevelArg = Tracer::LEVEL1; break;
            case 2: traceLevelArg = Tracer::LEVEL2; break;
            case 3: traceLevelArg = Tracer::LEVEL3; break;
            case 4: traceLevelArg = Tracer::LEVEL4; break;
            default: printf("Invalid trace level\n");
                return 0;
            }

            Tracer::setTraceFile((const char*)traceFile.getCString());
            Tracer::setTraceComponents(traceComponents);
            Tracer::setTraceLevel(traceLevelArg);

            PEG_TRACE_CSTRING(
                TRC_LISTENER,
                Tracer::LEVEL2,
                "Testing trace LEVEL2");
            PEG_TRACE_CSTRING(
                TRC_LISTENER,
                Tracer::LEVEL3,
                "Testing trace LEVEL3");
            PEG_TRACE_CSTRING(
                TRC_LISTENER,
                Tracer::LEVEL4,
                "Testing trace LEVEL4");
        }

        //ATTN: Need to handle SSL cases

        //create DynListener
        DynamicListener listener(listenerPort,
                                 consumerDir,
                                 consumerConfigDir,
                                 enableConsumerUnload,
                                 consumerIdleTimeout,
                                 shutdownTimeout);

        listener.start();

        //wait for shutdown signal
        char buf[255]={0};
        while (true)
        {
            cin.getline(buf,255);

            if (strlen(buf)>0 && strcmp(buf,"exit")==0)
                break;
        }

        listener.stop();


    } catch (OMConfigFileSyntaxError& cfs)
    {
        printf("ConfigFileSyntaxError %s\n",
               (const char*)cfs.getMessage().getCString());
    } catch (OMUnrecognizedConfigFileOption& cfs)
    {
        printf("OMUnrecognizedConfigFileOption %s\n",
               (const char*)cfs.getMessage().getCString());
    } catch (OMInvalidOptionValue& cfs)
    {
        printf("OMInvalidOptionValue %s\n",
               (const char*)cfs.getMessage().getCString());
    } catch (CIMException& ce)
    {
        printf("CIMException %s\n",
               (const char*)ce.getMessage().getCString());
    }
    catch (Exception e)
    {
        printf("Exception %s\n",
               (const char*)e.getMessage().getCString());
    }

    return 0;
}

