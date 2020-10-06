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
// Author: Thilo Boehm (tboehm@de.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef ARM_zOS_h
#define ARM_zOS_h

// #include <Pegasus/Common/Config.h>

PEGASUS_NAMESPACE_BEGIN

// Register status of CIM Server with ARM
enum __arm_status_tags {
    NOT_REGISTERED,
    REGISTERED,
    RESTARTED
};


class ARM_zOS {

private:
    __arm_status_tags ARM_zOS_Status;

public:
    // constructor
    ARM_zOS(){
        ARM_zOS_Status =  NOT_REGISTERED;
    };

    // destructor
    ~ARM_zOS(){};

    // Register CIM Server with ARM
    void Register(void);
    // De-Register CIM Server from ARM
    void DeRegister(void);

    // get register status
    __arm_status_tags getStatus(void){
        return ARM_zOS_Status;
    };

}; //class ARM_zOS

PEGASUS_NAMESPACE_END

#endif // ARM_zOS_h
