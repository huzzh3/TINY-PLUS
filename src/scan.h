#ifndef _SCAN_H_
#define _SCAN_H_

/* MAXTOKENLEN 是 token 的最大范围 */
#define MAXTOKENLEN 40

/* tokenString 是 lexeme, 也是 token 的实际值 */
extern char tokenString[MAXTOKENLEN+1];

/* getToken 函数能够获取下一个 token,
 * 它只返回 TokenType
 */
TokenType getToken(void);

#endif
