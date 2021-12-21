#include "globals.h"
#include "util.h"

/*  printToken 函数根据 TokenType 和扫描出来的值，
 *  将 token 输出到屏幕上。
 *  注意，只有 TraceScan == TRUE 的时候，才会输出到屏幕上。
 */
void printToken( TokenType token, const char* tokenString )
{ 
    switch (token) { 
        case OR:
        case AND:
        case INT:
        case BOOL:
        case CHAR:
        case WHILE:
        case DO:
        case IF:
        case THEN:
        case ELSE:
        case END:
        case REPEAT:
        case UNTIL:
        case READ:
        case WRITE:
            fprintf(listing, "(KEY, %s)\n",tokenString);
            break;
        case ASSIGN: 
            fprintf(listing,"(SYM, :=)\n"); 
            break;
        case LT: 
            fprintf(listing,"(SYM, <)\n"); 
            break;
        case EQ: 
            fprintf(listing,"(SYM, =)\n"); 
            break;
        case LPAREN: 
            fprintf(listing,"(SYM, ()\n"); 
            break;
        case RPAREN: 
            fprintf(listing,"(SYM, ))\n"); 
            break;
        case SEMI: 
            fprintf(listing,"(SYM, ;)\n"); 
            break;
        case PLUS: 
            fprintf(listing,"(SYM, +)\n"); 
            break;
        case MINUS: 
            fprintf(listing,"(SYM, -)\n"); 
            break;
        case TIMES: 
            fprintf(listing,"(SYM, *)\n"); 
            break;
        case OVER: 
            fprintf(listing,"(SYM, /)\n"); 
            break;
        case GT:
            fprintf(listing,"(SYM, >)\n"); 
            break;
        case LE:
            fprintf(listing,"(SYM, <=)\n"); 
            break;
        case GE:
            fprintf(listing,"(SYM, >=)\n"); 
            break;
        case COMMA:
            fprintf(listing,"(SYM, ,)\n"); 
            break;
        case ENDFILE: 
            fprintf(listing,"EOF\n"); 
            break;
        case NUM:
            fprintf(listing, "(NUM, %s)\n",tokenString);
            break;
        case STR:
            fprintf(listing, "(STR, %s)\n",tokenString);
            break;
        case ID:
            fprintf(listing, "(ID, %s)\n",tokenString);
            break;
        case ERROR:
            fprintf(listing, "ERROR: %s\n",tokenString);
            break;
        default: /* 不应该发生 */
            fprintf(listing,"Unknown token: %d\n",token);
    }
}

/* newStmtNode 函数产生一个用于语法树的新的语句节点 */
TreeNode * newStmtNode(StmtKind kind) { 
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    }
    else {
        /* 初始化节点 */
        for (int i=0; i<MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
    }
    return t;
}

/* newExpNode 函数产生一个用于语法树的新的表达式节点 */
TreeNode * newExpNode(ExpKind kind) { 
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    if (t==NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    }
    else {
        for (int i=0;i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->type = Void;
    }
    return t;
}

/* copyString 用于字符串的深拷贝 */
char * copyString(char *s) { 
    int n;
    char * t;
    if (s==NULL) {
        return NULL;
    }
    n = strlen(s)+1;
    t = malloc(n);
    if (t == NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    }
    else {
        strcpy(t,s);
    }

    return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
int static indentno = 0;

/* 用于增加/减少缩进的宏定义 */
#define INDENT indentno += 4
#define UNINDENT indentno -= 4

/* 在打印 syntax 的时候输出空格 */
static void printSpaces(void) { 
  for (int i=0;i<indentno;i++) {
      fprintf(listing," ");
  }
}

/* printTree 能够输出 syntax tree
 * 输出的时候通过缩进来表示节点之间的关系(如子节点和父节点的关系)
 */
void printTree(TreeNode * tree) { 
    INDENT;
    while (tree != NULL) {
        printSpaces();
        if (tree->nodekind == StmtK) { 
            switch (tree->kind.stmt) {
            case IfK:
                fprintf(listing,"If\n");
                break;
            case RepeatK:
                fprintf(listing,"Repeat\n");
                break;
            case AssignK:
                fprintf(listing,"Assign to: %s\n",tree->attr.name);
                break;
            case ReadK:
                fprintf(listing,"Read: %s\n",tree->attr.name);
                break;
            case WriteK:
                fprintf(listing,"Write\n");
                break;
            default:
                fprintf(listing,"Unknown ExpNode kind\n");
                break;
            }
        }
        else if (tree->nodekind == ExpK) { 
            switch (tree->kind.exp) {
            case OpK:
                fprintf(listing,"Op: ");
                printToken(tree->attr.op,"\0");
                break;
            case ConstK:
                fprintf(listing,"Const: %d\n",tree->attr.val);
                break;
            case IdK:
                fprintf(listing,"Id: %s\n",tree->attr.name);
                break;
            default:
                fprintf(listing,"Unknown ExpNode kind\n");
                break;
            }
        }
        else {
            fprintf(listing,"Unknown node kind\n");
        } 
        for (int i=0; i < MAXCHILDREN; i++) {
            printTree(tree->child[i]);
        }
        
        // 输出兄弟节点的值
        tree = tree->sibling;
    }
    UNINDENT;
}
