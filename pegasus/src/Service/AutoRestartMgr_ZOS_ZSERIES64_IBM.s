*/%LICENSE/////////////////////////////////////////////////////////////
*/
*/ Licensed to The Open Group (TOG) under one or more contributor
*/ license agreements.  Refer to the OpenPegasusNOTICE.txt file
*/ distributed with this work for additional information regarding
*/ copyright ownership.
*/ Each contributor licenses this file to you under the OpenPegasus
*/ Open Source License; you may not use this file except in compliance
*/ with the License.
*/
*/ Permission is hereby granted, free of charge, to any person
*/ obtaining a copy of this software and associated documentation files
*/ (the "Software"), to deal in the Software without restriction,
*/ including without limitation the rights to use, copy, modify, merge,
*/ publish, distribute, sublicense, and/or sell copies of the Software,
*/ and to permit persons to whom the Software is furnished to do so,
*/ subject to the following conditions:
*/
*/ The above copyright notice and this permission notice shall be
*/ included in all copies or substantial portions of the Software.
*/
*/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*/ MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
*/ NONINFRINGEMENT.
*/ IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*/ CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*/ TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*/ SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
*//////////////////////////////////////////////////////////////////////

********************************************************************
*  REGARM      - register with ARM
*
*  Function: The elment name and type are hard coded.
*            TERMTYPE=ELEMTERM is used for only app ABENDS.
*            All other input parameters are left at their defaults:
*            ELEMTYPE=NO_ELEMTYPE
*            EVENTEXIT=NO_EVENTEXIT
*            EVENTEXITPL=NO_EVENTEXITPL
*            PLISTVER=IMPLIED_VERSION
*            RESTARTTIMEOUT=NORM
*            STARTTXT=NO_STARTTXT
*
*  USAGE FROM C:
*        void register_arm (char * elemname,
*                           char * elemtype,
*               /* must point to a local buffer which has a length */
*               /* of least 128 bytes */
*                          char * buffer,
*                          int * retcode,
*                          int * reasoncode)
*
********************************************************************
REGARM    CSECT
REGARM    AMODE 64
REGARM    RMODE 31                        MUST RESIDE in 31-BIT MEMORY
*
         EDCPRLG                                                       +
               BASEREG=3,                                              +
               USRDSAL=AUTOSPACE1
*
*
         USING AUTOSPACE,13
* make C-auto-storage accessable using DSECT and register 13
         L     R4,0(0,R1)         get element name address into R4
         L     R7,4(0,R1)         GET buffer PTR into R7
         L     R5,8(0,R1)         GET pointer to retcode variable
         L     R6,12(0,R1)        GET pointer to reascode variable
         MVC   0(IXC1FL,R7),IXC1ST     COPY TEMPLATE LIST FORM MACRO
*         
         IXCARM REQUEST=REGISTER,ELEMENT=(R4),                         +
               ELEMTYPE=NO_ELEMTYPE,                                   + 
               TERMTYPE=ELEMTERM,                                      +
               RETCODE=0(0,R5),                                        +
               RSNCODE=0(0,R6),                                        +
               MF=(E,(R7))
         EDCEPIL
*
*
* use the LIST from of the IXCARM macro to generate a parameter
* block here
AUTOSPACE   EDCDSAD
IXC1ST   IXCARM MF=(L,IXCARML)
IXC1FE   DS    0H
IXC1FL   EQU   IXC1FE-IXC1ST
AUTOSPACE1 EQU *-AUTOSPACE
*
********************************************************************
*  READYARM      - set element to READY state with ARM
*
*  USAGE FROM C:
*        void ready_arm (
*               /* must point to a local buffer which has a length */
*               /* of least 128 bytes */
*                       char * buffer,
*                       int * retcode, int * reasoncode)
*
********************************************************************
READYARM  CSECT
READYARM  AMODE 31
READYARM  RMODE ANY
*
         EDCPRLG                                                       +
               BASEREG=3,                                              +
               USRDSAL=AUTOSPACE2
*
*
         USING AUTOSPACE,13
* make C-auto-storage accessable using DSECT and register 13
         L     R7,0(0,R1)        GET buffer PTR into R7
         L     R5,4(0,R1)        GET pointer to retcode variable
         L     R6,8(0,R1)        GET pointer to reascode variable
         MVC   0(IXC2FL,R7),IXC2ST     COPY TEMPLATE LIST FORM MACRO
*
         IXCARM REQUEST=READY,                                         +
               RETCODE=0(0,R5),                                        +
               RSNCODE=0(0,R6),                                        +
               MF=(E,(R7))
*
*
         EDCEPIL
*
*
AUTOSPACE   EDCDSAD
IXC2ST    IXCARM MF=(L,IXCARM2)
IXC2FE    DS    0H
IXC2FL    EQU   IXC2FE-IXC2ST
AUTOSPACE2 EQU *-AUTOSPACE
*
********************************************************************
*  DEREGARM      - deregister element from ARM
*                  should be called at program termination
*  USAGE FROM C:
*        void deregister_arm (
*               /* must point to a local buffer which has a length */
*               /* of least 128 bytes */
*                            char * buffer,
*                            int * retcode, int * reasoncode)
*
********************************************************************
DEREGARM  CSECT
DEREGARM  AMODE 31
DEREGARM  RMODE ANY
*
         EDCPRLG                                                       +
               BASEREG=3,                                              +
               USRDSAL=AUTOSPACE3
*
*
         USING AUTOSPACE,13
* make C-auto-storage accessable using DSECT and register 13
         L     R7,0(0,R1)         GET buffer PTR into R7
         L     R5,4(0,R1)         GET pointer to retcode variable
         L     R6,8(0,R1)         GET pointer to reascode variable
         MVC   0(IXC3FL,R7),IXC3ST     COPY TEMPLATE LIST FORM MACRO
*         
         IXCARM REQUEST=DEREGISTER,                                    +
               RETCODE=0(0,R5),                                        +
               RSNCODE=0(0,R6),                                        +
               MF=(E,(R7))
*
         EDCEPIL
*
*
AUTOSPACE   EDCDSAD
IXC3ST    IXCARM MF=(L,IXCARM3)
IXC3FE    DS    0H
IXC3FL    EQU   IXC3FE-IXC3ST
AUTOSPACE3 EQU *-AUTOSPACE
*
*
R0       EQU   0
R1       EQU   1
R2       EQU   2
R3       EQU   3
R4       EQU   4
R5       EQU   5
R6       EQU   6
R7       EQU   7
R8       EQU   8
R9       EQU   9
R10      EQU   10
R11      EQU   11
R12      EQU   12
R13      EQU   13
R14      EQU   14
R15      EQU   15
*
