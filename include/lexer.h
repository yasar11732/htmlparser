#ifndef _LEXER_H
#define _LEXER_H

/* For case insensitive comparision */
#define TOUPPER ('a' - 'A')

/* We will use these token types */
typedef enum {
	DOCTYPE,      /* <!DOCTYPE */
	TAGOPEN,      /* <          */
	TAGCLOSE,     /* >          */
	TAGSELFCLOSE, /* />         */
	CLOSETAGOPEN, /* </         */
	SINGLEQUOTE,  /* '          */
	DOUBLEQUOTE,  /* "          */
	ESCAPE,       /* \x         */
	COMMENTOPEN,  /* <!--       */
	COMMENTCLOSE, /* -->        */
	CDATAOPEN,    /* <![CDATA[  */
	CDATACLOSE,   /* ]]>        */
	STRING,       /* anything else */
	WHITESPACE,   /* any combination of " ", "\r", "\n", "\t" */
	EQUAL         /* = */
} token_t;

/* Lexer will produce these. */
typedef struct {
	/* begin is included but end is exluded. For example;
	Memory locations: 1 | 2 | 3 | 4 | 5 | 6 |
	Chars:          : t | o | k | e | n |
	
	Begin will be 1, end will be 6.
	
	CAUTION! This is not a null terminated string.
	*/
	token_t type;
	const char *begin;
	const char *end;
} Token;

typedef void (*token_eater_t)(const Token *, const void *);

/* This is the data used by our lexer. */
typedef struct lexer {
	char *SOF;                          /* Start of file */
	char *END;                          /* End of file */
	char *SOT;                          /* Start of current token */
	char *pos;                          /* Current position of lexer */
	
	const void *args;                  /* Any arguments for token eater. It is callback
	                                       function's duty to make any sense of it. */
	
	/* Lexer work as a state machine, this function denotes the current 
	state. */
	void (*state_func)(struct lexer *);
	
	
	/*  This function gets a pointer to current token.
	
		This is where lexer communicates with possible parsers. This
		way, multiple parsers can be built upon this lexer.
		
		Each time a new token is found, this function will be called.
	*/
	token_eater_t token_eater;
	
	/* Tokens that we emit are also kept here, so we don't need to malloc - free
	each token. Parsers can do that if they require. */
	Token token;
} Lexer;

typedef void (*state_func)(struct lexer *);

static __inline void lex_init(Lexer *lex,
                            char *SOF,
							char *END,
							token_eater_t eater,
							const void *args
							) {
								
	lex->SOF = SOF;
	lex->END = END;
	lex->SOT = SOF;
	lex->pos   = SOF;
	lex->args  = args;
	lex->token_eater = eater;
}

void run_lexer(Lexer *);

#endif