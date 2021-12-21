#include "globals.h"
#include "util.h"
#include "scan.h"

/* scanner 中的 DFA 状态 */
typedef enum {
    START,
    INASSIGN,
    INCOMMENT,
    INNUM,
    INID,
    DONE
} StateType;

/* token 的 lexeme，也就是 read, write, ;, < 那些 */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN 是输入的 BUFFER 大小的最大值 */
#define BUFLEN 256

/* 用来保存当前行的内容 */
static char lineBuf[BUFLEN];
/* 用来保存当前行的行号 */ 
static int linepos = 0;    
/* 用来保存当前 buffer 的大小 */  
static int bufsize = 0;    
/* 用来判断是否已经到了文件末尾 */  
static int EOF_flag = FALSE; 

/* getNextChar 函数获取当前行中下一个非空格的字符，
 * 如果 lineBuf 空了，那么就读入下一行
 */
static int getNextChar(void) {
	if (!(linepos < bufsize)) {

		/* 此时表示当前行已经空了，需要读入下一行 */
		lineno++;

		/* 从 source 中读入一行，存入 lineBuf 中 */
		if (fgets(lineBuf, BUFLEN - 1, source)) {
			/* 可以在 main 函数中设置 EchoSource，让这里输出读入的行的内容 */
			if (EchoSource) {
				fprintf(listing, "%4d: %s", lineno, lineBuf);
			}

			/* 读取完毕之后，设置 bufsize */
			bufsize = strlen(lineBuf);
			/* 将 linepos 复位 */
			linepos = 0;
			return lineBuf[linepos++];
		}
		else {
			/* 如果无法读取 source 中的内容了，就代表到了文件末尾 */
			EOF_flag = TRUE;
			return EOF;
		}
	}
	else {
		/* 表示这一行还没读取完，继续读取下一个字符 */
		return lineBuf[linepos++]; 
	}
}

/* ungetNextChar 函数用于回退一个字符
 * 记得通过 EOF_flag 判断是否要回退
 */
static void ungetNextChar(void) {
	if (!EOF_flag) {
		linepos--;
	}
}

/* 保留字的查找表 */
static struct {
	char *str;
	TokenType tok;
} reservedWords[MAXRESERVED] = {
	{"if", IF}, 
	{"then", THEN}, 
	{"else", ELSE}, 
	{"end", END}, 
	{"repeat", REPEAT}, 
	{"until", UNTIL}, 
	{"read", READ}, 
	{"write", WRITE}
};

/*	通过线性扫描的方式，
 *  检查某一个 ID 是否是保留字
 */
static TokenType reservedLookup(char *s) {
  	for (int i = 0; i < MAXRESERVED; i++) {
		  if (!strcmp(s, reservedWords[i].str)) {
			  return reservedWords[i].tok;
		  }
	  }
    	
  	return ID;
}

/****************************************/
/*           扫描器的主函数             */
/****************************************/
/* getToken 函数能够返回 source 中下一个 token
 * 注意它只返回 tokenType，而不会返回 tokenString
 */
TokenType getToken(void) { 
  	/* index 用来表示现在扫描的字符在字符串的位置 */
  	int tokenStringIndex = 0;
  	/* 保存现在扫描出来的 token */
  	TokenType currentToken;
	/* 初始化现在的 DFA 状态为 START */
	StateType state = START;
	/* flag 用来指示是否要将当前字符保存在字符串中 */
	int save;

	/* 如果当前的状态还没有到结束态，那么就继续扫描下去 */
  	while (state != DONE) {
		/* 获取下一个字符 */	  
    	int c = getNextChar();
    	save = TRUE;

		/* 根据现在所处的 state 决定是否转移状态 */
    	switch (state) {
    		case START:
      			if (isdigit(c)) {
					state = INNUM;
				}
      			else if (isalpha(c)) {
					state = INID;
				}
      			else if (c == ':') {
					state = INASSIGN;
				}
      			else if ((c == ' ') || (c == '\t') || (c == '\n')) {
					save = FALSE;
				} 
				else if (c == '{') {
					save = FALSE;
					state = INCOMMENT;
				}
				else
				{
					state = DONE;
					switch (c) {
						case EOF:
							save = FALSE;
							currentToken = ENDFILE;
							break;
						case '=':
							currentToken = EQ;
							break;
						case '<':
							currentToken = LT;
							break;
						case '+':
							currentToken = PLUS;
							break;
						case '-':
							currentToken = MINUS;
							break;
						case '*':
							currentToken = TIMES;
							break;
						case '/':
							currentToken = OVER;
							break;
						case '(':
							currentToken = LPAREN;
							break;
						case ')':
							currentToken = RPAREN;
							break;
						case ';':
							currentToken = SEMI;
							break;
						default:
							currentToken = ERROR;
							break;
					}
				}
				break;
    		case INCOMMENT:
				save = FALSE;
				if (c == EOF) {
					state = DONE;
					currentToken = ENDFILE;
				}
				else if (c == '}') {
					state = START;
				}
				break;
			case INASSIGN:
				state = DONE;
				if (c == '=') {
					currentToken = ASSIGN;
				}
				else { 
					ungetNextChar();
					save = FALSE;
					currentToken = ERROR;
				}
				break;
			case INNUM:
				if (!isdigit(c)) { 
					ungetNextChar();
					save = FALSE;
					state = DONE;
					currentToken = NUM;
				}
				break;
			case INID:
				if (!isalpha(c)) {
					ungetNextChar();
					save = FALSE;
					state = DONE;
					currentToken = ID;
				}
				break;
    		case DONE: 
				break;
    		default: /* 当进入这个状态，就说明出 bug 了 */
				fprintf(listing, "Scanner Bug: state= %d\n", state);
				state = DONE;
				currentToken = ERROR;
				break;
    	}

		if ((save) && (tokenStringIndex <= MAXTOKENLEN)) {
			tokenString[tokenStringIndex++] = (char)c;
		}
		
		if (state == DONE) {
			tokenString[tokenStringIndex] = '\0';
			if (currentToken == ID) {
				currentToken = reservedLookup(tokenString);
			}
		}
  	}
	if (TraceScan) {
			fprintf(listing, "\t%d: ", lineno);
			printToken(currentToken, tokenString);
	}
  	return currentToken;
}
