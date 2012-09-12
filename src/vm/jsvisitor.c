#include <stdio.h>

#define USE_JS_VISITOR
#ifdef USE_JS_VISITOR
#define DUMPER(BUILDER)  ((struct JSVisitor*)BUILDER)

#define MN_isNotNull MN_("isNotNull")
#define MN_isNull    MN_("isNull")
#define MN_get    MN_("get")
#define MN_set    MN_("set")
#define MN_opNOT  MN_("!")
#define MN_opNEG  MN_("opNEG")
#define MN_opADD  MN_("+")
#define MN_opSUB  MN_("-")
#define MN_opMUL  MN_("*")
#define MN_opDIV  MN_("/")
#define MN_opMOD  MN_("%")
#define MN_opEQ   MN_("==")
#define MN_opNEQ  MN_("!=")
#define MN_opLT   MN_("<")
#define MN_opLTE  MN_("<=")
#define MN_opGT   MN_(">")
#define MN_opGTE  MN_(">=")
#define MN_opLAND MN_("&")
#define MN_opLOR  MN_("|")
#define MN_opLXOR MN_("^")
#define MN_opLSFT MN_("<<")
#define MN_opRSFT MN_(">>")

#define TY_GlobalVariable      (CT_InstTBL[ 9])->typeId

static void emit_string_js(const char *str, const char *prefix, const char *suffix)
{
	printf("%s%s%s", prefix, str, suffix);
}

static void emit_newline(const char *str, int indent)
{
	printf("%s\n", str);
	int i;
	for (i = 0; i < indent; i++) {
		putchar(' ');
	}
}

static int MethodName_isBinaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	//if (mn == MN_opNEG ) return OPCODE_iNEG;
	if (mn == MN_opADD ) return 1;
	if (mn == MN_opSUB ) return 1;
	if (mn == MN_opMUL ) return 1;
	if (mn == MN_opDIV ) return 1;
	if (mn == MN_opMOD ) return 1;
	if (mn == MN_opEQ  ) return 1;
	if (mn == MN_opNEQ ) return 1;
	if (mn == MN_opLT  ) return 1;
	if (mn == MN_opLTE ) return 1;
	if (mn == MN_opGT  ) return 1;
	if (mn == MN_opGTE ) return 1;
#ifdef OPCODE_iAND
	if (mn == MN_opLAND) return 1;
	if (mn == MN_opLOR ) return 1;
	if (mn == MN_opLXOR) return 1;
	if (mn == MN_opLSFT) return 1;
	if (mn == MN_opRSFT) return 1;
#endif
	return 0;
}

static int MethodName_isUnaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if (mn == MN_opNEG) return 1;
	if (mn == MN_opNOT) return 1;
	return 0;
}

static void JSVisitor_visitErrStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	emit_string_js(S_text(kStmt_getObjectNULL(kctx, stmt, KW_ERR)), "", "");
}

static void JSVisitor_visitExprStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	handleExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt));
	emit_newline(";", DUMPER(self)->indent - 1);
}

static void JSVisitor_visitBlockStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	emit_newline("{", DUMPER(self)->indent);
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	emit_newline("}", DUMPER(self)->indent);
}

static void JSVisitor_visitReturnStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	emit_string_js("return ", "", "");
	kExpr* expr = Stmt_getFirstExpr(kctx, stmt);
	if (expr != NULL && IS_Expr(expr)) {
		handleExpr(kctx, self, expr);
	}
	emit_newline(";", DUMPER(self)->indent - 1);
}

static void JSVisitor_visitIfStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	DUMPER(self)->indent++;
	emit_string_js("if(", "", "");
	handleExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt));
	emit_newline("){", DUMPER(self)->indent - 1);
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	emit_newline("}else{", DUMPER(self)->indent - 1);
	visitBlock(kctx, self, Stmt_getElseBlock(kctx, stmt));
	emit_newline("}", DUMPER(self)->indent - 1);
	DUMPER(self)->indent--;
}

static void JSVisitor_visitLoopStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	DUMPER(self)->indent++;
	emit_string_js("while(", "", "");
	handleExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt));
	emit_newline("){", DUMPER(self)->indent - 1);
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	emit_newline("}", DUMPER(self)->indent - 1);
	DUMPER(self)->indent--;
}

static void JSVisitor_visitJumpStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	//emit_string_js("Jump", "", "");
}

static void JSVisitor_visitUndefinedStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	//emit_string_js("UNDEF", "", "");
}

static void JSVisitor_visitConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	if(expr->ty == TY_System){
		emit_string_js("System", "", "");
		return;
	}else if(Expr_isTerm(expr)){
		int is_string = expr->ty == TY_String;
		kToken *tk = (kToken*)expr->termToken;
		if(is_string) emit_string_js("\"", "", "");
		emit_string_js(S_text(tk->text), "", "");
		if(is_string) emit_string_js("\"", "", "");
		return;
	}	
	kObject *obj = expr->objectConstValue;
	emit_string_js(CT_t(O_ct(obj)), "", "");
}

static void JSVisitor_visitNConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KonohaStack sfp[1];
	unsigned long unboxVal = expr->unboxConstValue;
	KonohaClass *ct = CT_(expr->ty);
	sfp[0].unboxValue = unboxVal;
	ct->p(kctx, sfp, 0, &wb, 0);
	char  *str = (char *) KLIB Kwb_top(kctx, &wb, 0);
	emit_string_js(str, "", "");
	KLIB Kwb_free(&wb);
}

static void JSVisitor_visitNewExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	emit_string_js("new", "", "");
}

static void JSVisitor_visitNullExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	emit_string_js("null", "", "");
}

static void JSVisitor_visitLocalExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	//char buf[128];
	//snprintf(buf, 128, "LOCAL(%d, %s)", (int)expr->index, CT_t(CT_(expr->ty)));
	//char* name = GET_VARIABLE_NAME_FROM_EXPR(kctx, expr);
	//emit_string_js(name, "", "");
	kToken *tk = (kToken*)expr->termToken;
	emit_string_js(S_text(tk->text), "", "");
}

static void JSVisitor_visitBlockExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	emit_newline("{", DUMPER(self)->indent);
	visitBlock(kctx, self, expr->block);
	emit_newline("}", DUMPER(self)->indent);
}

static void JSVisitor_visitFieldExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	emit_string_js("FIELD", "", "");
}

static void JSVisitor_visitCallExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	kMethod *mtd = CallExpr_getMethod(expr);
	if(MethodName_isBinaryOperator(kctx, mtd->mn)){
		emit_string_js("(", "", "");
		handleExpr(kctx, self, kExpr_at(expr, 1));
		emit_string_js("", T_mn(mtd->mn));
		handleExpr(kctx, self, kExpr_at(expr, 2));
		emit_string_js(")", "", "");
	}else if(MethodName_isUnaryOperator(kctx, mtd->mn)){
		emit_string_js("", T_mn(mtd->mn));
		emit_string_js("(", "", "");
		handleExpr(kctx, self, kExpr_at(expr, 1));
		emit_string_js(")", "", "");
	}else{
		handleExpr(kctx, self, kExpr_at(expr, 1));
		emit_string_js(".", "", "");
		emit_string_js("", T_mn(mtd->mn));
		emit_string_js("(", "", "");
		unsigned i;
		unsigned n = kArray_size(expr->cons);
		for (i = 2; i < n;) {
			handleExpr(kctx, self, kExpr_at(expr, i));
			if(++i < n){
				emit_string_js(", ", "", "");
			}
		}
		emit_string_js(")", "", "");
	}
}

static void JSVisitor_visitAndExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	unsigned n = kArray_size(expr->cons);
	unsigned i;
	emit_string_js("(", "", "");
	for (i = 1; i < kArray_size(expr->cons);) {
		handleExpr(kctx, self, kExpr_at(expr, i));
		if(++i < n){
			emit_string_js(" && ", "", "");
		}
	}
	emit_string_js(")", "", "");
}

static void JSVisitor_visitOrExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	unsigned n = kArray_size(expr->cons);
	unsigned i;
	emit_string_js("(", "", "");
	for (i = 1; i < kArray_size(expr->cons);) {
		handleExpr(kctx, self, kExpr_at(expr, i));
		if(++i < n){
			emit_string_js(" || ", "", "");
		}
	}
	emit_string_js(")", "", "");
}

static void JSVisitor_visitLetExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	DUMPER(self)->indent++;
	handleExpr(kctx, self, kExpr_at(expr, 1));
	emit_string_js(" = ", "", "");
	handleExpr(kctx, self, kExpr_at(expr, 2));
	DUMPER(self)->indent--;
}

static void JSVisitor_visitStackTopExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	//emit_string_js("STACKTOP", "", "");
}

static IRBuilder *createJSVisitor(IRBuilder *builder)
{
#define DEFINE_BUILDER_API(NAME) builder->api.visit##NAME = JSVisitor_visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	DUMPER(builder)->indent = 0;
	return builder;
}

#endif
