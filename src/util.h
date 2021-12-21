#ifndef _UTIL_H_
#define _UTIL_H_

/*  printToken 函数根据 TokenType 和扫描出来的值，
 *  将 token 输出到屏幕上。
 *  注意，只有 TraceScan == TRUE 的时候，才会输出到屏幕上。
 */
void printToken( TokenType, const char* );

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind);

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind);

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString( char * );

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * );

#endif
