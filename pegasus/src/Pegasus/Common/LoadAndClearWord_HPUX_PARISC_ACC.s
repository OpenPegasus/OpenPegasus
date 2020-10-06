;//%LICENSE////////////////////////////////////////////////////////////////
;//
;// Licensed to The Open Group (TOG) under one or more contributor license
;// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
;// this work for additional information regarding copyright ownership.
;// Each contributor licenses this file to you under the OpenPegasus Open
;// Source License; you may not use this file except in compliance with the
;// License.
;//
;// Permission is hereby granted, free of charge, to any person obtaining a
;// copy of this software and associated documentation files (the "Software"),
;// to deal in the Software without restriction, including without limitation
;// the rights to use, copy, modify, merge, publish, distribute, sublicense,
;// and/or sell copies of the Software, and to permit persons to whom the
;// Software is furnished to do so, subject to the following conditions:
;//
;// The above copyright notice and this permission notice shall be included
;// in all copies or substantial portions of the Software.
;//
;// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
;// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
;// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
;// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
;// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
;// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;//
;//////////////////////////////////////////////////////////////////////////

;
; LoadAndClearWord_HPUX_PARISC_ACC.s:
; HP-UX PA-RISC assembly language routine for spinlock support.
;

    .code
#ifdef __LP64__
    .level 2.0W
#else
    .level 1.1
#endif
    .export LoadAndClearWord,entry,priv_lev=3,rtnval=gr
    .proc

LoadAndClearWord
    .callinfo no_calls
    .enter

; create a 16 byte aligned pointer to the load+clear word area
    addi        15,%arg0,%arg2  ; add 15 to the pointer to round up

#ifdef __LP64__
    depdi       0,63,4,%arg2    ; mask the lower 4 bits (64-bit)
#else
    depi        0,31,4,%arg2    ; mask the lower 4 bits (32-bit)
#endif

#ifdef __LP64__
    bve       (%r2)          ; return 0 if already locked, else !0
#else
    bv        (%r2)          ; return 0 if already locked, else !0
#endif
    ldcws,co  (%arg2),%ret0  ; (in branch delay slot)
                        ; load and clear (coherent) the spinlock word

; This code is never executed.  This endless loop reduces the
; prefetching after mispredicted bv on pre PA 8700 processors.
; Leave this dead code here to improve performance.

load_and_clear_loop
      b load_and_clear_loop
      nop
     .leave
     .procend
