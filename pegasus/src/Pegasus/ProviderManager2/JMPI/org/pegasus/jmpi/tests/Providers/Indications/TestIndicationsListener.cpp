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
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Listener/CIMListener.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

class MyIndicationConsumer : public CIMIndicationConsumer
{
public:
    MyIndicationConsumer ()
    {
    }

    ~MyIndicationConsumer ()
    {
    }

    void consumeIndication (const OperationContext& context,
                            const String&           url,
                            const CIMInstance&      indicationInstance)
    {
       Uint32 ui32Index = indicationInstance.findProperty ("InstanceId");
       Uint64 ui64Num = 0;

       if (ui32Index != PEG_NOT_FOUND)
       {
          try
          {
             indicationInstance.getProperty(ui32Index).getValue().get(ui64Num);
          }
          catch (Exception& e)
          {
          }
       }

       cout << "! "
            << url
            << " ! "
            << indicationInstance.getClassName ().getString ()
            << " ! "
            << (int)ui64Num
            << endl;
    }

private:
};

int main (int argc, char *argv[])
{
   Uint32                portNumber            = 2005;
   CIMListener           listener (portNumber);
   MyIndicationConsumer *consumer              = new MyIndicationConsumer ();

   // Add our consumer
   listener.addConsumer (consumer);

   // Finish starting the CIMListener
   try
   {
       cout << "+++++ Starting the CIMListener at destination\n"
            << "       http://localhost:2005/TestIndicationStressTest"
            << endl;

       // Start the listener
       listener.start ();
   }
   catch (BindFailedException &bfe)
   {
       // Got a bind error.  The port is probably already in use.
       // Put out a message and fail.
       cerr << endl
            << "==>WARNING: unable to bind to listener port 2005"
            << endl;
       cerr << "The listener port may be in use." << endl;

       throw;
   }

   cout << "Press any key to terminate..." << endl;

   cin.get ();

   listener.stop ();
   listener.removeConsumer (consumer);

   delete consumer;

   return 0;
}
