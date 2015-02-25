#include <assert.h>



#include "lexer.h"

/* This lexer is UTF unaware on purpose, because characters that we
 are interested in are already in <128 range. Moreover, we are not
 removing or adding any bytes, only tagging relevant sections of data
 using pointers.
 
 Parser, on the other hand, might want to check for encoding.
 */

#define NULLSTATE (state_func)0
#define QUITLEX(LEXER) (LEXER)->state_func = NULLSTATE
#define LEXFUNC(NAME) static void lex_##NAME(Lexer *)
#define DATAEND(LEX) (LEX)->pos >= (LEX)->END
#define CHECKEND(LEX) if(DATAEND(LEX))

/* Define possible states */
LEXFUNC(initial);
LEXFUNC(tagopen);
LEXFUNC(comment);
LEXFUNC(whitespace);

static void token_emit(Lexer *lex, token_t type) {
	assert(lex->pos - lex->SOT);
	lex->token.type = type;
	lex->token.begin = lex->SOT;
	lex->token.end   = lex->pos;
	lex->token_eater(&lex->token, lex->args);
	lex->SOT = lex->pos;
}

/* If there is something that can be emitted,
   emit it as a string */
static __inline void _flush(Lexer *lex) {
	if (lex->pos - lex->SOT) {
		token_emit(lex, STRING);
	}
}

static void lex_initial(Lexer *lex) {
	/* This function pretty much handles most of the job.
	it increments the position until it reaches an interesting
	chracter, and either emits required token if it can, or it
	transfer control to other states.
	*/
	char current = lex->pos[0];
	CHECKEND(lex) {
		_flush(lex);
		QUITLEX(lex);
		return;
	}
	
	switch (current) {
		case '<':
			_flush(lex);
			lex->state_func = lex_tagopen;
			break;
		case '/':
			if(lex->pos[1] == '>') {
				_flush(lex);
				lex->pos+=2;
				token_emit(lex, TAGSELFCLOSE);
			} else {
				lex->pos++;
			}
			break;
		case '>':
			_flush(lex);
			lex->pos++;
			token_emit(lex, TAGCLOSE);
			break;
		case '=':
			_flush(lex);
			lex->pos++;
			token_emit(lex, EQUAL);
			break;
		case '"':
			_flush(lex);
			lex->pos++;
			token_emit(lex, DOUBLEQUOTE);
			break;
		case '\'':
			_flush(lex);
			lex->pos++;
			token_emit(lex, SINGLEQUOTE);
			break;
		case '\\':
			_flush(lex);
			lex->pos+=2;
			token_emit(lex, ESCAPE);
			break;
		case '-':
			if(lex->pos[1] == '-' && lex->pos[2] == '>') {
				_flush(lex);
				lex->pos+=3;
				token_emit(lex, COMMENTCLOSE);
			} else {
				lex->pos++;
			}
			break;
		case ']':
			if(lex->pos[1] == ']' && lex->pos[2] == '>') {
				_flush(lex);
				lex->pos+=3;
				token_emit(lex, CDATACLOSE);
			} else {
				lex->pos++;
			}
			break;
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			_flush(lex);
			lex->state_func = lex_whitespace;
			break;
		default:
			lex->pos++;
	}
}

static void lex_whitespace(Lexer *lex) {
	char current = lex->pos[0];
	
	switch(current) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			lex->pos++;
			break;
		default:
			token_emit(lex, WHITESPACE);
			lex->state_func = lex_initial;
	}
}

static void lex_tagopen(Lexer *lex) {
	/* If we are here, there must be a < character at lex->pos */
	lex->pos++;
	CHECKEND(lex) {
		/* Tag open character at the end of file
		We might have incomplete file or whatever. We emit
		it, rest is parser's problem.
		*/
		token_emit(lex, TAGOPEN);
		QUITLEX(lex);
		return;
	}
	
	switch (lex->pos[0]) {
	case '!':
		lex->state_func = lex_comment;;
		break;
	case '/':
		lex->pos++;
		token_emit(lex, CLOSETAGOPEN);
		lex->state_func = lex_initial;
		break;
	default:
		token_emit(lex, TAGOPEN);
		lex->state_func = lex_initial;
		break;
	}
}

static void lex_comment(Lexer *lex) {
	/* We have <! right now. */
	
	char success = 0; /* Did we find <!--, <![CDATA or <!DOCTYPE */

	
	lex->pos++;
	CHECKEND(lex) {
		/*We have <! at the end of the file, go back and emit 
		< as tag open and count ! as seperate string. */
		lex->pos--;
		token_emit(lex, TAGOPEN);
		lex->pos++;
		token_emit(lex, STRING);
		QUITLEX(lex);
		return;
	}
	
	switch (lex->pos[0]) {
		const char *cdata, *doctype;
		char iscdata, isdoctype;
		int i;
		
		case '-':
			if (lex->pos[1] == '-') {
				lex->pos+=2;
				token_emit(lex, COMMENTOPEN);
				lex->state_func = lex_initial;
				success = 1;
			}
			break;
		case '[': /* empty */;
			cdata = "[CDATA[";
			iscdata = 1;
			for (i = 0; i < 7; i++) {
				if(!(cdata[i] == lex->pos[i])) {
					iscdata = 0;
					break;
				}
			}
			if (iscdata) {
				lex->pos+=7;
				token_emit(lex, CDATAOPEN);
				lex->state_func = lex_initial;
				success = 1;
			}
			break;
		case 'D':
		case 'd': /* empty */ ;
			doctype = "doctype";
			isdoctype = 1;
			for (i = 0; i < 7; i++) {
				if(!(lex->pos[i] == doctype[i] ||
				     lex->pos[i] == (doctype[i] - TOUPPER))) {
						 isdoctype = 0;
						 break;
					 }
			}
			if (isdoctype) {
				lex->pos+=7;
				token_emit(lex, DOCTYPE);
				lex->state_func = lex_initial;
				success = 1;
			}
			break;	
	}
	
	/* we couldn't get <!--, <!DOCTYPE or <![CDATA[ */
	if (!success) {
		lex->pos--;
		token_emit(lex, TAGOPEN);
		lex->state_func = lex_initial;
	}
}

void run_lexer(Lexer *lex) {
	lex->state_func = lex_initial;
	
	while(lex->state_func) {
		lex->state_func(lex);
	}
}