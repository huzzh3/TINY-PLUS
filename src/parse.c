#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

/* 存储当前 token */
static TokenType token; 

/* 函数声明 */
static TreeNode * program(void);
static TreeNode * stmt_sequence(void);
static TreeNode * declarations(void);
static TreeNode * decl(void);
static TreeNode * type_specifer(void);
static TreeNode * varlist(void);
static TreeNode * statement(void);
static TreeNode * if_stmt(void);
static TreeNode * repeat_stmt(void);
static TreeNode * assign_stmt(void);
static TreeNode * read_stmt(void);
static TreeNode * write_stmt(void);
static TreeNode * while_stmt(void);
static TreeNode * expr(void);
static TreeNode * simple_exp(void);
static TreeNode * term(void);
static TreeNode * factor(void);

/* 当发生了语法错误的时候，会调用该函数 */
static void syntaxError(char * message) { 
    fprintf(listing,"\n>>> ");
    fprintf(listing,"Syntax error at line %d: %s",lineno,message);
    Error = TRUE;
}

/*  判断当前的 token 是否与期望的 token 匹配
 *  如果匹配，那么就读取下一个 token，如果不匹配就报错
 */
static void match(TokenType expected) { 
    if (token == expected) token = getToken();
    else {
        syntaxError("unexpected token -> ");
        printToken(token,tokenString);
        fprintf(listing,"      ");
    }
}

/*  program -> declarations stmt_sequence */
TreeNode *program(void) {
    TreeNode *t = declarations();
    TreeNode *p = stmt_sequence();

    if (t != NULL) {
        if (p != NULL) {
            /* 将 stmt_sequence 接到 declarations 后面 */
            TreeNode *q = t;
            while (q->sibling != NULL) {
                q = q->sibling;
            }
            q->sibling = p;
        }
    }

    return t;
}


/* stmt_sequence -> statement {; statement} */
TreeNode * stmt_sequence(void) {
    TreeNode * t = statement();
    TreeNode * p = t;
    while ((token != ENDFILE) && (token != END) && (token != ELSE) && (token != UNTIL)) {
        /* ENDFILE 表示到达文件末尾，END 表示到达块末尾，ELSE 要与 IF 配套使用，UNTIL 要和 REPEAT 配套使用 */
        TreeNode * q;
        /* 注意看 TINY 的产生式，必须要有 SEMI，才表示后面还有语句，否则语句就要结束了 */
        match(SEMI);

        /* 获取后面的语句 */
        q = statement();
        if (q != NULL) {
            /* 如果成功获取了后面的语句，那么就要进行处理 */
            if (t == NULL) {
                /* 个人认为这个地方可以不用，后续可能会删除 */
                 t = p = q;
            }
            else { 
                /* 将节点串联起来 */
                p->sibling = q;
                p = q;
            }
        }
    }

    return t;
}

/* declaration -> decl {;, declarations} */
TreeNode *declarations(void) {
    TreeNode *t = decl();
    TreeNode *p = t;

    while (token == SEMI) {
        match(SEMI);
        TreeNode *q = decl();
        if (q != NULL) {
            if (t == NULL) {
                t = p = q;
            }
            else {
                p->sibling = q;
                p = q;
            }
        }
    }

    return t;
}

/* decl -> type-specifier varlist */
TreeNode *decl(void) {
    TreeNode *t = type_specifer();
    TreeNode *p = varlist();

    if (t != NULL && p != NULL) {
        t->child[0] = p;
    }

    return t;
}

/* type-specifier -> int|bool|char */
TreeNode *type_specifer(void) {
    TreeNode *t = NULL;

    if (token == INT) {
        t = newDeclNode(IntK);
        match(INT);
    }
    else if (token == BOOL) {
        t = newDeclNode(BoolK);
        match(BOOL);
    }
    else if (token == CHAR) {
        t = newDeclNode(CharK);
        match(CHAR);
    }

    return t;
}

TreeNode *varlist(void) {
    TreeNode *t = factor();
    TreeNode *p = t;

    while (token == COMMA) {
        match(COMMA);
        TreeNode *q = factor();

        if (q != NULL) {
            p->sibling = q;
            p = q;
        }
    }

    return t;
}

/*  statement -> if-stmt|repeat-stmt|assign-stmt|read-stmt|write-stmt   */
TreeNode * statement(void) { 
    TreeNode * t = NULL;
    switch (token) {
        case IF : 
            t = if_stmt(); 
            break;
        case REPEAT : 
            t = repeat_stmt(); 
            break;
        case ID : 
            t = assign_stmt(); 
            break;
        case READ : 
            t = read_stmt(); 
            break;
        case WRITE : 
            t = write_stmt(); 
            break;
        case WHILE :
            t = while_stmt();
            break;
        default : 
            syntaxError("unexpected token -> ");
            printToken(token,tokenString);
            token = getToken();
            break;
    }

    return t;
}

/* if_stmt -> if exp then stmt_sequence [else stmt_sequence] end */
TreeNode *if_stmt(void) {
    /* 产生一个新的节点 */
    TreeNode *t = newStmtNode(IfK);

    /* 进行匹配，并移动 token */
    match(IF);

    if (t != NULL) {
        /* if 语句第一个孩子节点必须是表达式 */
        t->child[0] = expr();
    }

    /* 进行匹配，并移动 token */
    match(THEN);

    if (t != NULL) {
        /* if 语句第二个孩子节点必须是语句 */
        t->child[1] = stmt_sequence();
    }

    /* 检测是否有 ELSE */
    if (token==ELSE) {
        match(ELSE);
        if (t!=NULL){
            /* 在有 ELSE 的情况下， if 语句第三个孩子节点必须是语句 */
            t->child[2] = stmt_sequence();
        }
    }

    /* 进行匹配，并移动 token */
    match(END);
    return t;
}

/* repeat_stmt -> repeat stmt_sequence until exp */
TreeNode * repeat_stmt(void) { 
    TreeNode *t = newStmtNode(RepeatK);
    match(REPEAT);
    if (t != NULL) {
        t->child[0] = stmt_sequence();
    }

    match(UNTIL);
    if (t!=NULL) {
        t->child[1] = expr();
    }

    return t;
}

/* assign_stmt -> identifier := exp */
TreeNode * assign_stmt(void) { 
    TreeNode * t = newStmtNode(AssignK);
    if ((t!=NULL) && (token==ID)) {
        t->attr.name = copyString(tokenString);
    }
        
    match(ID);
    match(ASSIGN);

    if (t!=NULL){
        t->child[0] = expr();
    }

    return t;
}

/* read_stmt -> read identifier */
TreeNode * read_stmt(void) { 
    TreeNode * t = newStmtNode(ReadK);
    match(READ);

    if ((t!=NULL) && (token==ID)) {
        t->attr.name = copyString(tokenString);
    }
    match(ID);

    return t;
}

/* write_stmt -> write exp */
TreeNode * write_stmt(void) { 
    TreeNode * t = newStmtNode(WriteK);
    match(WRITE);

    if (t!=NULL) {
        t->child[0] = expr();
    }

    return t;
}

/* while_stmt -> while bool_exp do stmt_sequence end */
TreeNode *while_stmt(void) {
    TreeNode *t = newStmtNode(WhileK);

    match(WHILE);

    if (t != NULL) {
        t->child[0] = expr();
    }

    match(DO);

    if (t != NULL) {
        t->child[1] = stmt_sequence();
    }

    match(END);

    return t;
}

/* exp -> simple-exp [comparision-op simple-exp] */
TreeNode *expr(void) { 
    TreeNode *t = simple_exp();
    if ((token==LT)||(token==EQ)||(token==GT)||(token==LE)||(token==GE)) {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);

        if (t!=NULL) {
            t->child[1] = simple_exp();
        }
    }
    return t;
}

/* simple_exp -> term {addop term} */
TreeNode * simple_exp(void) { 
    TreeNode * t = term();
    while ((token == PLUS) || (token == MINUS)) { 
        TreeNode *p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);

            t->child[1] = term();
        }
    }
    return t;
}

/* term -> factor {mulop factor} */
TreeNode * term(void) { 
    TreeNode * t = factor();
    while ((token==TIMES)||(token==OVER)) { 
            TreeNode * p = newExpNode(OpK);
            if (p!=NULL) {
                p->child[0] = t;
                p->attr.op = token;
                t = p;
                match(token);
                p->child[1] = factor();
            }
    }
    return t;
}

/* factor -> (exp) | number | identifier */
TreeNode * factor(void) { 
    TreeNode * t = NULL;
    switch (token) {
        case NUM :
            t = newExpNode(ConstK);
            if ((t!=NULL) && (token==NUM)) {
                t->attr.val = atoi(tokenString);
            }
            match(NUM);
            break;
        case ID :
            t = newExpNode(IdK);
            if ((t!=NULL) && (token==ID)) {
                t->attr.name = copyString(tokenString);
            }
            match(ID);
            break;
        case LPAREN :
            match(LPAREN);
            t = expr();
            match(RPAREN);
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token,tokenString);
            token = getToken();
            break;
    }

    return t;
}

/****************************************/
/*          parser 的主函数             */
/****************************************/
/*   parse 函数返回一个最新创建的语法树
 *   只在语法分析中有效
 */
TreeNode * parse(void) { 
    TreeNode * t;
    token = getToken();
    t = program();
    if (token != ENDFILE) {
        syntaxError("Code ends before file\n");
    }
    return t;
}
