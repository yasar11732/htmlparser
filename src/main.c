#include <stdlib.h>
#include <stdio.h>

#include "lexer.h"
#include "file_read.h"


/* Normally, a parser will handle tokens, but for now 
	this function will print them.
*/
void token_print(const Token *t,const void *asdf) {
	int strlen = t->end - t->begin;
	char *mystring = (char *)malloc(strlen + 1);
	char *tokentype;
	
	
	memcpy(mystring, t->begin, strlen);
	mystring[strlen] = '\0';
	
	switch(t->type) {
	case DOCTYPE:
		tokentype = "doctype";
		break;
	case TAGOPEN:
		tokentype = "tagopen";
		break;
	case TAGCLOSE:
		tokentype = "tagclose";
		break;
	case TAGSELFCLOSE:
		tokentype = "tagselfclose";
		break;
	case SINGLEQUOTE:
		tokentype = "singlequote";
		break;
	case DOUBLEQUOTE:
		tokentype = "double quote";
		break;
	case ESCAPE:
		tokentype = "escape";
		break;
	case COMMENTOPEN:
		tokentype = "comment open";
		break;
	case COMMENTCLOSE:
		tokentype = "comment close";
		break;
	case CDATAOPEN:
		tokentype = "cdata open";
		break;
	case CDATACLOSE:
		tokentype = "cdata close";
		break;
	case STRING:
		tokentype = "string";
		break;
	case WHITESPACE:
		tokentype = "whitespace";
		break;
	case EQUAL:
		tokentype = "equal";
		break;
	case CLOSETAGOPEN:
		tokentype = "closetagopen";
		break;
	default:
		/* This sholdn't happen */
		tokentype = "other";
		break;
	}
	printf("token type: %s, token value: {%s}\n", tokentype, mystring);
}

int main() {
	struct file_data fd = read_file("deneme.html");
	Lexer lex;
	lex_init(&lex, fd.data, fd.data + fd.numChars, token_print, NULL);
	run_lexer(&lex);
	
	return 0;
}