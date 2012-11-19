/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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

#ifndef MINIVM_H
#define MINIVM_H

/* minivm arch */

typedef intptr_t   kreg_t;
typedef void (*ThreadCodeFunc)(KonohaContext *kctx, struct VirtualCode *, void**, size_t size);
typedef void (*TraceFunc)(KonohaContext *kctx, KonohaStack *sfp, KTraceInfo *trace);

typedef struct {
	kMethod *mtd;
	ktype_t typeId; kparamId_t signature;
} kMethodInlineCache;

#if defined(USE_DIRECT_THREADED_CODE)
#define OP_(T)  NULL, 0, OPCODE_##T, 0
#define KCODE_HEAD\
	void *codeaddr;\
	size_t count;\
	kushort_t opcode;\
	kfileline_t line

#else
#define OP_(T)  0, OPCODE_##T, 0
#define KCODE_HEAD \
	size_t count; \
	kopcode_t opcode; \
	kfileline_t line

#endif/*USE_DIRECT_THREADED_CODE*/

#define OPARGSIZE 4

typedef struct VirtualCode {
	KCODE_HEAD;
	union {
		intptr_t data[OPARGSIZE];
		void *p[OPARGSIZE];
		kObject *o[OPARGSIZE];
		KonohaClass *ct[OPARGSIZE];
		char *u[OPARGSIZE];
	};
} VirtualCode;

typedef enum {
	VMT_VOID,
	VMT_ADDR,
	VMT_UL,
	VMT_R,
	VMT_FX,
	VMT_U,
	VMT_C,
	VMT_TY,
	VMT_F,        /*function*/
	VMT_Object,
	VMT_HCACHE,
} VirtualCodeType;

#define PC_NEXT(pc)   pc+1

/* NOP */
#define VPARAM_NOP 0
typedef struct OPNOP {
	KCODE_HEAD;
} OPNOP;

#ifndef OPEXEC_NOP
#define OPEXEC_NOP(PC)
#endif

/* THCODE */
#define VPARAM_THCODE 2, VMT_U, VMT_F
typedef struct OPTHCODE {
	KCODE_HEAD;
	size_t codesize;
	ThreadCodeFunc threadCode;
} OPTHCODE;

#ifndef OPEXEC_THCODE
#define OPEXEC_THCODE(pc) do {\
	OPTHCODE *op = (OPTHCODE *)pc;\
	op->threadCode(kctx, pc, OPJUMP, op->opcode); \
	return PC_NEXT(pc);\
} while(0)
#endif

/* ENTER */
#define VPARAM_ENTER 0
typedef struct OPENTER {
	KCODE_HEAD;
} OPENTER;

#ifndef OPEXEC_ENTER
#define OPEXEC_ENTER(pc) do {\
	rbp[K_PCIDX2].pc = PC_NEXT(pc);\
	pc = (rbp[K_MTDIDX2].calledMethod)->pc_start;\
	GOTO_PC(pc); \
} while(0)
#endif

/* EXIT */
#define VPARAM_EXIT 0
typedef struct OPEXIT {
	KCODE_HEAD;
} OPEXIT;

#ifndef OPEXEC_EXIT
#define OPEXEC_EXIT(pc) do {\
	pc = NULL; \
	goto L_RETURN;\
} while(0)
#endif

/* NCALL */
#define VPARAM_NCALL     0
typedef struct OPNCALL {
	KCODE_HEAD;
} OPNCALL;

#ifndef OPEXEC_NCALL
#define OPEXEC_NCALL(pc) do {\
	(rbp[K_MTDIDX2].calledMethod)->invokeMethodFunc(kctx, (KonohaStack *)(rbp));\
	OPEXEC_RET(pc);\
} while(0)
#endif

/* NSET */
#define VPARAM_NSET        3, VMT_R, VMT_U, VMT_TY
typedef struct OPNSET {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	KonohaClass* ty;
} OPNSET;

#ifndef OPEXEC_NSET
#define OPEXEC_NSET(pc) do {\
	OPNSET *op = (OPNSET *)pc;\
	rbp[op->a].unboxValue = (op->n);\
} while(0)
#endif

/* NMOV */
#define VPARAM_NMOV       3, VMT_R, VMT_R, VMT_TY
typedef struct OPNMOV {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KonohaClass* ty;
} OPNMOV;

#ifndef OPEXEC_NMOV
#define OPEXEC_NMOV(pc) do {\
	OPNMOV *op = (OPNMOV *)pc;\
	rbp[op->a].unboxValue = rbp[op->b].unboxValue;\
} while(0)
#endif

/* NMOVx */
#define VPARAM_NMOVx      4, VMT_R, VMT_R, VMT_FX, VMT_TY
typedef struct OPNMOVx {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	uintptr_t bx;
	KonohaClass* ty;
} OPNMOVx;

#ifndef OPEXEC_NMOVx
#define OPEXEC_NMOVx(pc) do {\
	OPNMOVx *op = (OPNMOVx *)pc;\
	rbp[op->a].unboxValue = (rbp[op->b].asObject)->fieldUnboxItems[op->bx];\
} while(0)
#endif

/* XNMOV */
#define VPARAM_XNMOV     4, VMT_R, VMT_FX, VMT_R, VMT_TY
typedef struct OPXNMOV {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t ax;
	kreg_t b;
	KonohaClass* ty;
} OPXNMOV;

#ifndef OPEXEC_XNMOV
#define OPEXEC_XNMOV(pc) do {\
	OPXNMOV *op = (OPXNMOV *)pc;\
	(rbp[op->a].asObjectVar)->fieldUnboxItems[op->ax] = rbp[op->b].unboxValue;\
} while(0)
#endif

/* NEW */
#define VPARAM_NEW       3, VMT_R, VMT_U, VMT_C
typedef struct OPNEW {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t p;
	KonohaClass* ty;
} OPNEW;

#ifndef OPEXEC_NEW
#define OPEXEC_NEW(pc) do {\
	OPNEW *op = (OPNEW *)pc;\
	rbp[op->a].asObject = KLIB new_kObject(kctx, OnStack, op->ty, op->p);\
} while(0)
#endif

/* OPNULL */
#define VPARAM_NULL       2, VMT_R, VMT_C
typedef struct OPNULL {
	KCODE_HEAD;
	kreg_t a;
	KonohaClass* ty;
} OPNULL;

#ifndef OPEXEC_NULL
#define OPEXEC_NULL(pc) do {\
	OPNULL *op = (OPNULL *)pc;\
	rbp[op->a].asObject = KLIB Knull(kctx, op->ty);\
} while(0)
#endif

/* OPLOOKUP */
#define VPARAM_LOOKUP    3, VMT_R, VMT_Object, VMT_Object
typedef struct OPLOOKUP {
	KCODE_HEAD;
	kreg_t thisidx;
	kNameSpace* ns;
	kMethod*    mtd;
	ktype_t typeId; kparamId_t signature;  // invisible
} OPLOOKUP;

#ifndef OPEXEC_LOOKUP
#define OPEXEC_LOOKUP(pc) do {\
	OPLOOKUP *op = (OPLOOKUP *)pc;\
	kNameSpace_LookupMethodWithInlineCache(kctx, (KonohaStack *)(rbp + op->thisidx), op->ns, (kMethod**)&(op->mtd));\
} while(0)
#endif

/* OPCALL*/
#define VPARAM_CALL     4, VMT_UL, VMT_R, VMT_R, VMT_Object
typedef struct OPCALL {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kObject* tyo;
} OPCALL;

#ifndef OPEXEC_CALL
#define OPEXEC_CALL(pc) do {\
	OPCALL *op = (OPCALL *)pc;\
	kMethod *mtd_ = rbp[op->thisidx + K_MTDIDX2].calledMethod;\
	KonohaStack *sfp_ = (KonohaStack *)(rbp + op->thisidx); \
	KUnsafeFieldSet(sfp_[K_RTNIDX].asObject, op->tyo);\
	sfp_[K_RTNIDX].calledFileLine = op->uline;\
	sfp_[K_SHIFTIDX].previousStack = (KonohaStack *)(rbp);\
	sfp_[K_PCIDX].pc = PC_NEXT(pc);\
	sfp_[K_MTDIDX].calledMethod = mtd_;\
	KonohaRuntime_setesp(kctx, (KonohaStack *)(rbp + op->espshift));\
	(mtd_)->invokeMethodFunc(kctx, sfp_); \
} while(0)
#endif

/* RET */
#define VPARAM_RET       0
typedef struct OPRET {
	KCODE_HEAD;
} OPRET;

#ifndef OPEXEC_RET
#define OPEXEC_RET(PC) do {\
	VirtualCode *vpc = rbp[K_PCIDX2].pc;\
	rbp = (krbp_t *)rbp[K_SHIFTIDX2].previousStack;\
	pc = vpc; \
	GOTO_PC(pc);\
} while(0)
#endif


/* BNOT */
#define VPARAM_BNOT       2, VMT_R, VMT_R
typedef struct OPBNOT {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} OPBNOT;

#ifndef OPEXEC_BNOT
#define OPEXEC_BNOT(pc) do {\
	OPBNOT *op = (OPBNOT *)pc;\
	rbp[op->c].boolValue = !(rbp[op->a].boolValue);\
} while(0)
#endif

/* JMP */
#define VPARAM_JMP       1, VMT_ADDR
typedef struct OPJMP {
	KCODE_HEAD;
	VirtualCode  *jumppc;
} OPJMP;

#ifndef OPEXEC_JMP
#define OPEXEC_JMP(pc) do {\
	OPJMP *op = (OPJMP *)pc;\
	pc = op->jumppc;\
	goto JUMP; \
} while(0)
#endif

/* JMPF */
#define VPARAM_JMPF      2, VMT_ADDR, VMT_R
typedef struct OPJMPF {
	KCODE_HEAD;
	VirtualCode  *jumppc;
	kreg_t a;
} OPJMPF;

#ifndef OPEXEC_JMPF
#define OPEXEC_JMPF(pc) do {\
	OPJMPF *op = (OPJMPF *)pc;\
	if(!rbp[op->a].boolValue) {\
		pc = op->jumppc;\
		goto JUMP;\
	} \
} while(0)
#endif

/* TRYJMP */
#define VPARAM_TRYJMP      1, VMT_ADDR
typedef struct OPTRYJMP {
	KCODE_HEAD;
	VirtualCode  *jumppc;
} OPTRYJMP;

#ifndef OPEXEC_TRYJMP
#define OPEXEC_TRYJMP(pc) do {\
	OPTRYJMP *op = (OPTRYJMP *)pc;\
	pc = KonohaVirtualMachine_tryJump(kctx, (KonohaStack *)rbp, pc+1);\
	if(pc == NULL) {\
		pc = op->jumppc;\
		goto JUMP;\
	} \
} while(0)
#endif

/* YIELD */
#define VPARAM_YIELD      0
typedef struct OPYIELD {
	KCODE_HEAD;
} OPYIELD;

#ifndef OPEXEC_YIELD
#define OPEXEC_YIELD(pc) do {\
	return pc;\
} while(0)
#endif

/* ERROR */
#define VPARAM_ERROR          3, VMT_UL, VMT_Object, VMT_R
typedef struct OPERROR {
	KCODE_HEAD;
	uintptr_t uline;
	kString*  msg;
	kreg_t    esp;
} OPERROR;

#ifndef OPEXEC_ERROR
#define OPEXEC_ERROR(pc) do {\
	OPERROR *op = (OPERROR *)pc;\
	((KonohaStack *)rbp)[K_RTNIDX].calledFileLine = op->uline;\
	KLIB KonohaRuntime_raise(kctx, EXPT_("RuntimeScript"), SoftwareFault, op->msg, (KonohaStack *)rbp);\
} while(0)
#endif

/* SAFEPOINT */
#define VPARAM_SAFEPOINT       2, VMT_UL, VMT_R
typedef struct OPSAFEPOINT {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t    esp;
} OPSAFEPOINT;

#ifndef OPEXEC_SAFEPOINT
#define OPEXEC_SAFEPOINT(pc) do {\
	OPSAFEPOINT *op = (OPSAFEPOINT *)pc;\
	KonohaRuntime_setesp(kctx, (KonohaStack *)(rbp + op->esp));\
	KLIB CheckSafePoint(kctx, (KonohaStack *)rbp, op->uline);\
} while(0)
#endif

/* CHKSTACK */
#define VPARAM_CHKSTACK        1, VMT_UL
typedef struct OPCHKSTACK {
	KCODE_HEAD;
	uintptr_t uline;
} OPCHKSTACK;

#ifndef OPEXEC_CHKSTACK
#define OPEXEC_CHKSTACK(pc) do {\
	if(unlikely(kctx->esp > kctx->stack->stack_uplimit)) {\
		KLIB KonohaRuntime_raise(kctx, EXPT_("StackOverflow"), SoftwareFault, NULL, (KonohaStack *)(rbp));\
	}\
} while(0)
#endif

/* TRACE */
#define VPARAM_TRACE           3, VMT_UL, VMT_R, VMT_F
typedef struct OPTRACE {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t    thisidx;
	TraceFunc trace;
} OPTRACE;

#ifndef OPEXEC_TRACE
#define OPEXEC_TRACE(pc)
#endif

#endif /* MINIVM_H */
