/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#ifndef BYTELOADER_H_
#define BYTELOADER_H_

#define NSET_InternalMethod 0
#define NSET_UserConstant   1
#define NSET_UserMethod     2

#define TypeSource_Default     0
#define TypeSource_UserDefined 1

#define GtType_Void   0
#define GtType_Int    1
#define GtType_Float  2
#define GtType_Bool   3
#define GtType_String 4

#define GT_FALSE 0
#define GT_TRUE  1

#define InternalMethod_Op_Undefined 0
#define InternalMethod_Int_opMINUS 1
#define InternalMethod_Int_opADD 2
#define InternalMethod_Int_opSUB 3
#define InternalMethod_Int_opMUL 4
#define InternalMethod_Int_opDIV 5
#define InternalMethod_Int_opMOD 6
#define InternalMethod_Int_opEQ 7
#define InternalMethod_Int_opNEQ 8
#define InternalMethod_Int_opLT 9
#define InternalMethod_Int_opLTE 10
#define InternalMethod_Int_opGT 11
#define InternalMethod_Int_opGTE 12
#define InternalMethod_Array_get 101
#define InternalMethod_Array_set 102
#define InternalMethod_Array_newList 103


#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t JavaLong; /* signed 64bit int */
typedef double JavaDouble; /* 64bit float */
typedef int32_t JavaInt; /* signed 32bit int */

/*#define SetVirtualCode(VCODE, T, ...) do {\
	OP##T op_ = {OP_(T), ## __VA_ARGS__};\
	union { KVirtualCode op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	*(VCODE) = tmp_.op;\
} while(0)*/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef BYTELOADER_H_ */
