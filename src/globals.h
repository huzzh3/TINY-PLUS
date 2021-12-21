#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* 保留字的个数 */
#define MAXRESERVED 15

typedef enum {
      /* 特殊 tokens */
      ENDFILE,ERROR,
      /* 保留字 */
      OR,AND,INT,BOOL,CHAR,WHILE,DO,IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,
      /* 多字符 tokens */
      ID,NUM,STR,
      /* 特殊符号 */
      ASSIGN,EQ,LT,PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,SEMI,GT,LE,GE,COMMA
} TokenType;

/* TINY 文件源 */
extern FILE* source; 
/* 结果输出流 */
extern FILE* listing; 
/* TM 模拟器的中间代码文件 */
extern FILE* code; 

/* 输出的行号 */
extern int lineno; 

/**************************************************/
/***********   parser 的语法树结构     ************/
/**************************************************/

typedef enum {StmtK,ExpK} NodeKind;
typedef enum {IfK,RepeatK,AssignK,ReadK,WriteK} StmtKind;
typedef enum {OpK,ConstK,IdK} ExpKind;

/* 用于表达式的类型检查 */
typedef enum {Void,Integer,Boolean} ExpType;

#define MAXCHILDREN 3

typedef struct treeNode {
    /* 每一个节点都有若干个孩子节点 */
    struct treeNode * child[MAXCHILDREN];
    /* 每一个节点还会记录兄弟节点 */
    struct treeNode * sibling;

    /* 行号，该节点对应源代码中的哪一行 */
    int lineno;

    /* 节点类型 */
    NodeKind nodekind;
    /* 详细的节点类型 */
    union { 
        StmtKind stmt; 
        ExpKind exp;
    } kind;

    /* 属性 */
    union { 
        TokenType op;
        int val;
        /* 加入要输出 Assign to: fact，就必须要用 name 来记录下 fact */
        char * name; 
    } attr;

    /* 用于表达式的类型检查 */
    ExpType type; 
} TreeNode;

/**************************************************/
/***********   用于跟踪的 flags        ************/
/**************************************************/

/* EchoSource = TRUE 的话能够输出当前扫描器读入的行的内容
 */
extern int EchoSource;

/* TraceScan = TRUE 的话能够输出扫描得到的 token 的信息
 */
extern int TraceScan;

/* TraceParse = TRUE 的时候就能够输出 syntax tree 的信息
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error; 

#endif
