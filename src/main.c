#include "globals.h"

/* 如果将 NO_PARSE 设置为 TRUE, 则不做语法分析 */
#define NO_PARSE FALSE
/* 如果将 NO_ANALYZE 设置为 TRUE, 则不做语义分析 */
#define NO_ANALYZE TRUE
/* 如果将 NO_CODE 设置为 TRUE, 则不生成中间代码 */
#define NO_CODE TRUE

#include "util.h"
//#if NO_PARSE
#include "scan.h"
//#else
#include "parse.h"
//#if !NO_ANALYZE
#include "analyze.h"
//#if !NO_CODE
#include "cgen.h"
//#endif
//#endif
//#endif

/* 在 main 中设置全局变量 */
int lineno = 0;
FILE *source;
FILE *listing;
FILE *code;

/* 设置 flags 用于看到输出结果 */
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = TRUE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;

int Error = FALSE;

int main(int argc, char *argv[])
{
    TreeNode *syntaxTree;
    /* Tiny 源代码的位置 */
    char pgm[120];
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    strcpy(pgm, argv[1]);
    if (strchr(pgm, '.') == NULL) {
        strcat(pgm, ".tny");
    }
    
    /* 打开文件 */
    source = fopen(pgm, "r");
    if (source == NULL) {
        fprintf(stderr, "File %s not found\n", pgm);
        exit(1);
    }

    /* 将结果输出至屏幕 */
    listing = stdout; 
    fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);
    
#if NO_PARSE
    while (getToken() != ENDFILE)
        ;
#else
    syntaxTree = parse();
    if (TraceParse) {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(syntaxTree);
    }
#if !NO_ANALYZE
    if (!Error)
    {
        if (TraceAnalyze)
            fprintf(listing, "\nBuilding Symbol Table...\n");
        buildSymtab(syntaxTree);
        if (TraceAnalyze)
            fprintf(listing, "\nChecking Types...\n");
        typeCheck(syntaxTree);
        if (TraceAnalyze)
            fprintf(listing, "\nType Checking Finished\n");
    }
#if !NO_CODE
    if (!Error)
    {
        char *codefile;
        int fnlen = strcspn(pgm, ".");
        codefile = (char *)calloc(fnlen + 4, sizeof(char));
        strncpy(codefile, pgm, fnlen);
        strcat(codefile, ".tm");
        code = fopen(codefile, "w");
        if (code == NULL)
        {
            printf("Unable to open %s\n", codefile);
            exit(1);
        }
        codeGen(syntaxTree, codefile);
        fclose(code);
    }
#endif
#endif
#endif
    fclose(source);
    return 0;
}
