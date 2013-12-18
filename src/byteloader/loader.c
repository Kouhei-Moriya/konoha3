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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "konoha3.h"
#include "konoha3/konoha_common.h"

#include "src/arch/MiniVM/MiniVM.c"

#include "import/loader.h"

void LoadByteCode(KonohaContext *kctx, const char *filename); //API

static int EatOneParam(FILE *fp);
static kint_t EatIntValue(KonohaContext *kctx, FILE *fp);
static kint_t EatFloatValue(KonohaContext *kctx, FILE *fp);
static kint_t EatBoolValue(KonohaContext *kctx, FILE *fp);
static kint_t EatStringValue(KonohaContext *kctx, FILE *fp);
static kmethodn_t EatMethodName(KonohaContext *kctx, FILE *fp);
static kint_t (*EatConstant[])(KonohaContext *kctx, FILE *fp) = {
	NULL,
	EatIntValue,
	EatFloatValue,
	EatBoolValue,
	EatStringValue
};

static ktypeattr_t GenTypeAttr(KonohaContext *kctx, FILE *fp);
static kMethod *GetInternalMethod(KonohaContext *kctx, int methodnum);
static void GenClass(KonohaContext *kctx, FILE *fp);
static void GenMethod(KonohaContext *kctx, kArray *gcstack, kint_t *constList, kMethod **methodList, int index, FILE *fp);


static void Setcodeaddr(KonohaContext *kctx, KVirtualCode *vcode, int codesize);

static void GenNOP(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
static void GenNSET(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
static void GenNMOV(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
//static void GenNMOVx(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
//static void GenXNMOV(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
//static void GenNEW(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
//static void GenNUL(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
static void GenCALL(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
static void GenRET(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
static void GenJMP(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
static void GenJMPF(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);
static void GenCHKSTACK(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos);

static void (*GenCode[])(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) = {
	GenNOP, NULL, NULL, NULL,
	//GenNSET, GenNMOV, GenNMOVx, GenXNMOV,
	GenNSET, GenNMOV, NULL, NULL,
	//GenNEW, GenNUL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, GenCALL, GenRET, NULL,
	GenJMP, GenJMPF, NULL, NULL,
	NULL, NULL, GenCHKSTACK
};

static kstatus_t EvalMethod(KonohaContext *kctx, kMethod *mtd);


void LoadByteCode(KonohaContext *kctx, const char *filename) {
	INIT_GCSTACK();

	FILE *fp;
	int i;
	if((fp = fopen(filename, "r")) == NULL) {
		printf("file not open\n");
		exit(EXIT_FAILURE);
	}
	int constSize = EatOneParam(fp);
	int classSize = EatOneParam(fp);
	int methodSize = EatOneParam(fp);

	/* get Constant */
	kint_t constList[constSize];
	for(i = 0; i < constSize; ++i) {
		int type = EatOneParam(fp);
		constList[i] = EatConstant[type](kctx, fp);
	}

	/* get Class */
	for(i = 0; i < classSize; ++i) {
		GenClass(kctx, fp);
	}

	/* get Method */
	kMethod *methodList[methodSize];
	for(i = 0; i < methodSize; ++i) {
		GenMethod(kctx, _GcStack, constList, methodList, i, fp);
	}

	/* Execute */
	int mainMethodIdx = EatOneParam(fp);
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kstatus_t status;

	if(mainMethodIdx > -1) {
		status = EvalMethod(kctx, methodList[mainMethodIdx]);
		if(status == K_CONTINUE) {
			DumpEval(kctx, &wb);
			KLIB KBuffer_Free(&wb);
		}
		else {
			PLATAPI printf_i("error\n");
		}
	}

	//printf("%d\n", feof(fp));
	fclose(fp);
	RESET_GCSTACK();
}

static int EatOneParam(FILE *fp) {
	JavaInt value;
	fscanf(fp, "%d\n", &value);
	//printf("%d\n", value);
	return (int)value;
}
static kint_t EatIntValue(KonohaContext *kctx, FILE *fp) {
	EatOneParam(fp); //delete space
	JavaLong value;
	fscanf(fp, "%ld\n", &value);
	//printf("%ld\n", value);
	return (kint_t)value;
}
static kint_t EatFloatValue(KonohaContext *kctx, FILE *fp) {
	EatOneParam(fp); //delete space
	JavaDouble buf;
	fscanf(fp, "%lf\n", &buf);
	//printf("%lf\n", buf);
	union {
		kfloat_t floatValue;
		kint_t intValue;
	} value;
	value.floatValue = (kfloat_t)buf;
	return value.intValue;
}
static kint_t EatBoolValue(KonohaContext *kctx, FILE *fp) {
	EatOneParam(fp); //delete space
	int buf = EatOneParam(fp);
	union {
		kbool_t boolValue;
		kint_t intValue;
	} value;
	switch(buf) {
		case GT_FALSE:
			value.boolValue = false;
			break;
		case GT_TRUE:
		default:
			value.boolValue = true;
			break;
	}
	return value.intValue;
}
static kint_t EatStringValue(KonohaContext *kctx, FILE *fp) {
	int strlen = EatOneParam(fp);
	char *str = (char *) malloc((strlen+1) * sizeof(char));
	fscanf(fp, "%s\n", str);
	kString *kstr = KLIB new_kString(kctx, GcUnsafe, str, strlen, 0);
	free(str);
	//printf("%s\n", str);
	return (kint_t)((uintptr_t)kstr);
}
static kmethodn_t EatMethodName(KonohaContext *kctx, FILE *fp) {
	int strlen = EatOneParam(fp);
	char *str = (char *) malloc((strlen+1) * sizeof(char));
	fscanf(fp, "%s\n", str);
	//printf("%s\n", str);
	kmethodn_t mn = KMethodName_(str);
	free(str);
	return mn;
}

static ktypeattr_t GenTypeAttr(KonohaContext *kctx, FILE *fp) {
	int typeSource = EatOneParam(fp);
	int gttype = EatOneParam(fp);
	if(typeSource == TypeSource_Default) {
		switch(gttype) {
		case GtType_Void:
			return KType_void;
		case GtType_Int:
			return KType_Int;
		case GtType_Float:
			return KType_float;
		case GtType_Bool:
			return KType_Boolean;
		case GtType_String:
			return KType_String;
		default:
			return KType_ERROR;
		}
	}
	else {
		return KType_ERROR;
	}
	return KType_ERROR;
}

static kMethod *GetInternalMethod(KonohaContext *kctx, int methodnum) {
	switch(methodnum) {
	case InternalMethod_Int_opMINUS:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("-"), 0, KMethodMatch_NoOption);
	case InternalMethod_Int_opADD:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("+"), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opSUB:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("-"), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opMUL:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("*"), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opDIV:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("/"), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opMOD:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("%"), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opEQ:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("=="), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opNEQ:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("!="), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opLT:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("<"), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opLTE:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_("<="), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opGT:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_(">"), 1, KMethodMatch_NoOption);
	case InternalMethod_Int_opGTE:
		return KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, NULL, KClass_Int, KMethodName_(">="), 1, KMethodMatch_NoOption);
	//case InternalMethod_Array_get:
	//case InternalMethod_Array_set:
	//case InternalMethod_Array_newList:
	default:
		return NULL;
	}
}

static void GenClass(KonohaContext *kctx, FILE *fp) {
	kString *className = (kString *)EatStringValue(kctx, fp);

	int fieldSize = EatOneParam(fp);
	int fieldTypeId[fieldSize];
	kString *fieldName[fieldSize];
	int i;
	for(i = 0; i < fieldSize; ++i) {
		fieldTypeId[i] = GenTypeAttr(kctx, fp);
		fieldName[i] = (kString *)EatStringValue(kctx, fp);
	}
	//CreateClass(...);
}

static void GenMethod(KonohaContext *kctx, kArray *gcstack, kint_t *constList, kMethod **methodList, int index, FILE *fp) {
	int i;

	ktypeattr_t rtype = GenTypeAttr(kctx, fp);
	kmethodn_t mn = EatMethodName(kctx, fp);
	int psize = EatOneParam(fp);
	kparamtype_t p[psize];
	for(i = 0; i < psize; ++i) {
		p[i].typeAttr = GenTypeAttr(kctx, fp);
		p[i].name = 0;
	}
	int codelen = EatOneParam(fp);
	int codesize = (codelen+1) * sizeof(KVirtualCode);
	KVirtualCode *vcode = (KVirtualCode *)KCalloc_UNTRACE(codesize, 1) + 1;

	KMethodFunc func = (PLATAPI ExecutionEngineModule.GenerateMethodFunc(kctx, vcode));
	uintptr_t flag = kMethod_Static|kMethod_Public;
	kMethodVar *mtd = KLIB new_kMethod(kctx, gcstack, flag, KType_NameSpace, mn, func);
	KLIB kMethod_SetParam(kctx, mtd, rtype, psize, p);
	methodList[index] = (kMethod *)mtd;


	for(i = 0; i < codelen; ++i) {
		int opcode = EatOneParam(fp);
		vcode[i].opcode = opcode;
		GenCode[opcode](kctx, constList, methodList, fp, vcode, i);

	}
	Setcodeaddr(kctx, vcode, codesize);
	mtd->vcode_start = vcode;
	
	//kNameSpace_AddMethod(kctx, ns, mtd, trace);
	/*if (CT_(cid) != NULL) {
		CT_addMethod(kctx, (KonohaClassVar*)CT_(cid), (kMethod*)mtd);
	}*/
}




static void Setcodeaddr(KonohaContext *kctx, KVirtualCode *vcode, int codesize) {
	OPTHCODE *thcode = (OPTHCODE *)(vcode - 1);
	thcode->opcode = OPCODE_THCODE;
	thcode->codesize = codesize;
	thcode->threadCode = _THCODE;

	PLATAPI ExecutionEngineModule.RunExecutionEngine(kctx, NULL, vcode-1);
	//((struct KVirtualCodeAPI **)vcode)[-1] = &vapi;
}

static void GenNOP(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	//delete space
	EatOneParam(fp); 
	EatOneParam(fp);
	EatOneParam(fp);
}

static void GenNSET(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	OPNSET *nset = (OPNSET *)(vcode + pos);
	nset->a = (kreg_t)EatOneParam(fp);
	int setsource = EatOneParam(fp);
	if(setsource == NSET_InternalMethod) {
		nset->n = (kint_t)GetInternalMethod(kctx, EatOneParam(fp));
	}
	else if(setsource == NSET_UserConstant) {
		nset->n = constList[EatOneParam(fp)];
	}
	else if(setsource == NSET_UserMethod) {
		nset->n = (kint_t)methodList[EatOneParam(fp)];
	}
	nset->ty = NULL;
}

static void GenNMOV(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	OPNMOV *nmov = (OPNMOV *)(vcode + pos);
	nmov->dst = (kreg_t)EatOneParam(fp);
	nmov->src = (kreg_t)EatOneParam(fp);
	nmov->ty = NULL;

	//delete space
	EatOneParam(fp);
}

static void GenCALL(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	OPCALL *call = (OPCALL *)(vcode + pos);
	call->uline = 0;
	int thisidx = EatOneParam(fp);
	call->thisidx = (kreg_t)thisidx; 
	call->espshift = (kreg_t)(thisidx + (EatOneParam(fp) + 1) * 2);
	call->tyo = NULL;

	//delete space
	EatOneParam(fp);
}

static void GenRET(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	//delete space
	EatOneParam(fp); 
	EatOneParam(fp);
	EatOneParam(fp);
}

static void GenJMP(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	OPJMPF *jmpf = (OPJMPF *)(vcode + pos);
	jmpf->jumppc = vcode + EatOneParam(fp);

	//delete space
	EatOneParam(fp);
	EatOneParam(fp);
}

static void GenJMPF(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	OPJMPF *jmpf = (OPJMPF *)(vcode + pos);
	jmpf->jumppc = vcode + EatOneParam(fp);
	jmpf->a = (kreg_t)EatOneParam(fp);

	//delete space
	EatOneParam(fp);
}

static void GenCHKSTACK(KonohaContext *kctx, kint_t *constList, kMethod **methodList, FILE *fp, KVirtualCode *vcode, int pos) {
	OPCHKSTACK *chkstack = (OPCHKSTACK *)(vcode + pos);
	chkstack->uline = 0;

	//delete space
	EatOneParam(fp); 
	EatOneParam(fp);
	EatOneParam(fp);
}

static kstatus_t EvalMethod(KonohaContext *kctx, kMethod *mtd)
{
	BEGIN_UnusedStack(lsfp);
	KRuntimeContextVar *runtime = kctx->stack;
	ktypeattr_t rtype = kctx->share->paramList->ParamItems[mtd->paramid]->rtype;
	if(runtime->evalty != KType_void) {
		KStackSetObjectValue(lsfp[1].asObject, runtime->stack[runtime->evalidx].asObject);
		KStackSetUnboxValue(lsfp[1].intValue,  runtime->stack[runtime->evalidx].intValue);
	}   
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(rtype)), 0, mtd, 1); 
	kstatus_t result = K_CONTINUE;
	if(KLIB KRuntime_tryCallMethod(kctx, lsfp)) {
		runtime->evalidx = ((lsfp + K_RTNIDX) - kctx->stack->stack);
		runtime->evalty = rtype;
	}   
	else {
		runtime->evalty = KType_void;  // no value
		result = K_BREAK;		// message must be reported;
	}   
	END_UnusedStack();
	return result;
}



#ifdef __cplusplus
} /* extern "C" */
#endif
