/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 5 "toml-lemon.lemon"

#include <assert.h>
#include <stdlib.h>
#include "toml-parser.h"

typedef struct table_id_node {
  char *name;
  struct table_id_node *first;
  struct table_id_node *next;
} table_id_node;

char * _TOML_newstr( TOMLToken *token ) {
  if ( token->tokenStr ) {
    char *buffer = token->tokenStr;
    token->tokenStr = NULL;
    return buffer;
  }

  int size = token->end - token->start;
  char *buffer = malloc( size + 1 );
  strncpy( buffer, token->start, size );
  buffer[ size ] = 0;
  return buffer;
}

char * _TOML_getline( TOMLToken *token ) {
  char *endOfLine = strchr( token->lineStart, '\n' );
  if ( endOfLine == NULL ) {
    endOfLine = strchr( token->lineStart, 0 );
  }

  int size = endOfLine - token->lineStart;
  char *buffer = malloc( size + 1 );
  strncpy( buffer, token->lineStart, size );
  buffer[ size ] = 0;

  return buffer;
}

void _TOML_fillError(
  TOMLToken *token, TOMLParserState *state, int errorCode
) {
  state->errorCode = errorCode;

  TOMLError *error = state->errorObj;
  if ( error ) {
    error->code = errorCode;
    error->lineNo = token->line;
    error->line = _TOML_getline( state->token );

    int messageSize = strlen( TOMLErrorDescription[ errorCode ] );
    error->message = malloc( messageSize + 1 );
    strncpy( error->message, TOMLErrorDescription[ errorCode ], messageSize );
    error->message[ messageSize ] = 0;

    char *longMessage = malloc(
      strlen( error->line ) +
      strlen( error->message ) +
      (int) ( error->lineNo / 10 ) +
      20
    );
    sprintf(
      longMessage,
      "Error on line %d. %s: %s",
      error->lineNo,
      error->message,
      error->line
    );
    error->fullDescription = longMessage;
  }
}

void TOML_freeToken( TOMLToken *token ) {
  free( token->tokenStr );
  free( token );
}
#line 85 "toml-lemon.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    TOMLParserTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is TOMLParserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    TOMLParserARG_SDECL     A static variable declaration for the %extra_argument
**    TOMLParserARG_PDECL     A parameter declaration for the %extra_argument
**    TOMLParserARG_STORE     Code to store %extra_argument into yypParser
**    TOMLParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 31
#define YYACTIONTYPE unsigned char
#define TOMLParserTOKENTYPE void*
typedef union {
  int yyinit;
  TOMLParserTOKENTYPE yy0;
  TOMLArray * yy2;
  table_id_node * yy44;
  TOMLNumber * yy45;
  int yy61;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define TOMLParserARG_SDECL  TOMLParserState *state ;
#define TOMLParserARG_PDECL , TOMLParserState *state 
#define TOMLParserARG_FETCH  TOMLParserState *state  = yypParser->state 
#define TOMLParserARG_STORE yypParser->state  = state 
#define YYNSTATE 49
#define YYNRULE 38
#define YYERRORSYMBOL 11
#define YYERRSYMDT yy61
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    88,    2,   30,   32,   34,   35,   39,   47,    6,   25,
 /*    10 */    26,   27,   19,   10,   13,    9,   31,   32,   34,   35,
 /*    20 */    28,   12,    6,   33,    4,   29,    3,   42,   39,   36,
 /*    30 */    43,   44,   45,   46,   17,   18,   40,   12,   12,    1,
 /*    40 */     5,   16,   33,    4,   39,   22,   24,   39,   49,    1,
 /*    50 */    21,   37,    8,   15,   40,   22,   24,   23,   38,   24,
 /*    60 */    20,    7,    1,   48,   11,   12,   41,   22,   14,   89,
 /*    70 */     8,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    12,   13,   14,   15,   16,   17,    7,    4,   20,   22,
 /*    10 */    23,   24,   25,   26,   27,   28,   14,   15,   16,   17,
 /*    20 */    11,    1,   20,    2,    3,   11,    6,   11,    7,    4,
 /*    30 */    21,   22,   23,   24,   18,   19,   20,    1,    1,    3,
 /*    40 */     3,    1,    2,    3,    7,    9,   10,    7,    0,    3,
 /*    50 */    23,    4,    5,   19,   20,    9,   10,   24,   20,   10,
 /*    60 */     8,   29,    3,   22,   29,    1,   11,    9,   29,   30,
 /*    70 */     5,
};
#define YY_SHIFT_USE_DFLT (-2)
#define YY_SHIFT_MAX 19
static const signed char yy_shift_ofst[] = {
 /*     0 */    21,   46,   40,   36,   37,   -1,   20,   49,   -1,   52,
 /*    10 */    52,   59,   64,   52,   58,   47,   48,   25,   65,    3,
};
#define YY_REDUCE_USE_DFLT (-14)
#define YY_REDUCE_MAX 14
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -12,  -13,    2,    9,   16,   34,   14,   33,   38,   32,
 /*    10 */    35,   41,   55,   39,   27,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    87,   70,   87,   87,   87,   87,   87,   78,   87,   69,
 /*    10 */    67,   72,   87,   68,   75,   87,   87,   87,   58,   87,
 /*    20 */    80,   74,   81,   77,   82,   73,   76,   79,   85,   86,
 /*    30 */    50,   51,   52,   53,   54,   55,   56,   57,   59,   62,
 /*    40 */    60,   83,   84,   61,   63,   64,   65,   66,   71,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  TOMLParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void TOMLParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "EOF",           "COMMENT",       "LEFT_SQUARE", 
  "RIGHT_SQUARE",  "ID_DOT",        "EQ",            "ID",          
  "COMMA",         "STRING",        "NUMBER",        "error",       
  "file",          "line",          "line_and_comment",  "line_content",
  "table_header",  "entry",         "table_header_2",  "table_id",    
  "id",            "value",         "array",         "string",      
  "number",        "members",       "array_members",  "string_members",
  "number_members",  "comma",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "file ::= line EOF",
 /*   1 */ "line ::= line_and_comment",
 /*   2 */ "line ::= line line_and_comment",
 /*   3 */ "line_and_comment ::= line_content",
 /*   4 */ "line_and_comment ::= COMMENT",
 /*   5 */ "line_content ::= table_header",
 /*   6 */ "line_content ::= entry",
 /*   7 */ "table_header ::= LEFT_SQUARE table_header_2 RIGHT_SQUARE",
 /*   8 */ "table_header_2 ::= LEFT_SQUARE table_id RIGHT_SQUARE",
 /*   9 */ "table_header_2 ::= table_id",
 /*  10 */ "table_id ::= table_id ID_DOT id",
 /*  11 */ "table_id ::= id",
 /*  12 */ "entry ::= id EQ value",
 /*  13 */ "id ::= ID",
 /*  14 */ "value ::= array",
 /*  15 */ "value ::= string",
 /*  16 */ "value ::= number",
 /*  17 */ "array ::= LEFT_SQUARE members RIGHT_SQUARE",
 /*  18 */ "members ::= array_members",
 /*  19 */ "members ::= string_members",
 /*  20 */ "members ::= number_members",
 /*  21 */ "members ::=",
 /*  22 */ "array_members ::= array_members comma array",
 /*  23 */ "array_members ::= array_members comma",
 /*  24 */ "array_members ::= array",
 /*  25 */ "string_members ::= string_members comma string",
 /*  26 */ "string_members ::= string_members comma",
 /*  27 */ "string_members ::= string",
 /*  28 */ "number_members ::= number_members comma number",
 /*  29 */ "number_members ::= number_members comma",
 /*  30 */ "number_members ::= number",
 /*  31 */ "comma ::= COMMA",
 /*  32 */ "string ::= STRING",
 /*  33 */ "number ::= NUMBER",
 /*  34 */ "error ::= EOF error",
 /*  35 */ "table_header ::= LEFT_SQUARE error",
 /*  36 */ "entry ::= id EQ error",
 /*  37 */ "entry ::= id error",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to TOMLParser and TOMLParserFree.
*/
void *TOMLParserAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  TOMLParserARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
      /* TERMINAL Destructor */
    case 1: /* EOF */
    case 2: /* COMMENT */
    case 3: /* LEFT_SQUARE */
    case 4: /* RIGHT_SQUARE */
    case 5: /* ID_DOT */
    case 6: /* EQ */
    case 7: /* ID */
    case 8: /* COMMA */
    case 9: /* STRING */
    case 10: /* NUMBER */
{
#line 1 "toml-lemon.lemon"
 TOML_freeToken((yypminor->yy0)); 
#line 502 "toml-lemon.c"
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from TOMLParserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void TOMLParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int TOMLParserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_MAX ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_MAX );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_SZ_ACTTAB );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   TOMLParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   TOMLParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 12, 2 },
  { 13, 1 },
  { 13, 2 },
  { 14, 1 },
  { 14, 1 },
  { 15, 1 },
  { 15, 1 },
  { 16, 3 },
  { 18, 3 },
  { 18, 1 },
  { 19, 3 },
  { 19, 1 },
  { 17, 3 },
  { 20, 1 },
  { 21, 1 },
  { 21, 1 },
  { 21, 1 },
  { 22, 3 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 0 },
  { 26, 3 },
  { 26, 2 },
  { 26, 1 },
  { 27, 3 },
  { 27, 2 },
  { 27, 1 },
  { 28, 3 },
  { 28, 2 },
  { 28, 1 },
  { 29, 1 },
  { 23, 1 },
  { 24, 1 },
  { 11, 2 },
  { 16, 2 },
  { 17, 3 },
  { 17, 2 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  TOMLParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* file ::= line EOF */
#line 85 "toml-lemon.lemon"
{
  yy_destructor(yypParser,1,&yymsp[0].minor);
}
#line 828 "toml-lemon.c"
        break;
      case 4: /* line_and_comment ::= COMMENT */
#line 89 "toml-lemon.lemon"
{
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 835 "toml-lemon.c"
        break;
      case 7: /* table_header ::= LEFT_SQUARE table_header_2 RIGHT_SQUARE */
#line 93 "toml-lemon.lemon"
{
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,4,&yymsp[0].minor);
}
#line 843 "toml-lemon.c"
        break;
      case 8: /* table_header_2 ::= LEFT_SQUARE table_id RIGHT_SQUARE */
#line 95 "toml-lemon.lemon"
{
  table_id_node *first = yymsp[-1].minor.yy44->first;
  table_id_node *node = first;
  table_id_node *next = node->next;
  TOMLTable *table = state->rootTable;

  for ( ; node; node = next ) {
    TOMLTable *tmpTable = TOMLTable_getKey( table, node->name );
    TOMLBasic *tmpBasic = (TOMLBasic *) tmpTable;

    if ( tmpTable && tmpBasic->type == TOML_ARRAY && node->next ) {
      TOMLArray *tmpArray = (TOMLArray *) tmpBasic;
      tmpTable = TOMLArray_getIndex( tmpArray, tmpArray->size - 1 );
    }

    if ( !tmpTable ) {
      TOMLRef nextValue;
      if ( node->next ) {
        nextValue = TOML_aTable( NULL, NULL );
      } else {
        nextValue = TOML_anArray( TOML_TABLE, NULL );
      }
      TOMLTable_setKey( table, node->name, nextValue );
      tmpTable = nextValue;
    }
    table = tmpTable;
    next = node->next;
    free( node->name );
    free( node );
  }

  TOMLArray *array = (TOMLArray *) table;
  table = TOML_aTable( NULL, NULL );
  TOMLArray_append( array, table );

  state->currentTable = table;
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,4,&yymsp[0].minor);
}
#line 886 "toml-lemon.c"
        break;
      case 9: /* table_header_2 ::= table_id */
#line 132 "toml-lemon.lemon"
{
  table_id_node *first = yymsp[0].minor.yy44->first;
  table_id_node *node = first;
  table_id_node *next = node->next;
  TOMLTable *table = state->rootTable;

  for ( ; node; node = next ) {
    TOMLTable *tmpTable = TOMLTable_getKey( table, node->name );
    TOMLBasic *tmpBasic = (TOMLBasic *) tmpTable;

    if ( tmpTable && tmpBasic->type == TOML_ARRAY ) {
      TOMLArray *tmpArray = (TOMLArray *) tmpBasic;
      tmpTable = TOMLArray_getIndex( tmpArray, tmpArray->size - 1 );
    }

    if ( tmpTable && node->next == NULL ) {
      _TOML_fillError( state->token, state, TOML_ERROR_TABLE_DEFINED );
    } else if ( !tmpTable ) {
      tmpTable = TOML_aTable( NULL, NULL );
      TOMLTable_setKey( table, node->name, tmpTable );
    }
    table = tmpTable;
    next = node->next;
    free( node->name );
    free( node );
  }

  state->currentTable = table;
}
#line 919 "toml-lemon.c"
        break;
      case 10: /* table_id ::= table_id ID_DOT id */
#line 163 "toml-lemon.lemon"
{
  table_id_node *node = malloc( sizeof(table_id_node) );
  node->name = yymsp[0].minor.yy0;
  node->first = yymsp[-2].minor.yy44->first;
  node->next = NULL;
  yymsp[-2].minor.yy44->next = node;
  yygotominor.yy44 = node;
  yy_destructor(yypParser,5,&yymsp[-1].minor);
}
#line 932 "toml-lemon.c"
        break;
      case 11: /* table_id ::= id */
#line 171 "toml-lemon.lemon"
{
  table_id_node *node = malloc( sizeof(table_id_node) );
  node->name = yymsp[0].minor.yy0;
  node->first = node;
  node->next = NULL;
  yygotominor.yy44 = node;
}
#line 943 "toml-lemon.c"
        break;
      case 12: /* entry ::= id EQ value */
#line 179 "toml-lemon.lemon"
{
  if ( yymsp[-2].minor.yy0 != NULL || yymsp[0].minor.yy0 != NULL ) {
    TOMLRef oldValue = TOMLTable_getKey( state->currentTable, yymsp[-2].minor.yy0 );
    if ( oldValue != NULL ) {
      _TOML_fillError( state->token, state, TOML_ERROR_ENTRY_DEFINED );
    } else {
      TOMLTable_setKey( state->currentTable, yymsp[-2].minor.yy0, yymsp[0].minor.yy0 );
    }
  }
  free( yymsp[-2].minor.yy0 );
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
#line 959 "toml-lemon.c"
        break;
      case 13: /* id ::= ID */
#line 191 "toml-lemon.lemon"
{
  yygotominor.yy0 = _TOML_newstr( yymsp[0].minor.yy0 );
}
#line 966 "toml-lemon.c"
        break;
      case 14: /* value ::= array */
      case 15: /* value ::= string */ yytestcase(yyruleno==15);
      case 18: /* members ::= array_members */ yytestcase(yyruleno==18);
      case 19: /* members ::= string_members */ yytestcase(yyruleno==19);
#line 195 "toml-lemon.lemon"
{ yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 974 "toml-lemon.c"
        break;
      case 16: /* value ::= number */
#line 197 "toml-lemon.lemon"
{ yygotominor.yy0 = yymsp[0].minor.yy45; }
#line 979 "toml-lemon.c"
        break;
      case 17: /* array ::= LEFT_SQUARE members RIGHT_SQUARE */
#line 199 "toml-lemon.lemon"
{
  yygotominor.yy0 = yymsp[-1].minor.yy0;
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,4,&yymsp[0].minor);
}
#line 988 "toml-lemon.c"
        break;
      case 20: /* members ::= number_members */
#line 204 "toml-lemon.lemon"
{ yygotominor.yy0 = yymsp[0].minor.yy2; }
#line 993 "toml-lemon.c"
        break;
      case 21: /* members ::= */
#line 205 "toml-lemon.lemon"
{ yygotominor.yy0 = TOML_anArray( TOML_NOTYPE, NULL ); }
#line 998 "toml-lemon.c"
        break;
      case 22: /* array_members ::= array_members comma array */
      case 25: /* string_members ::= string_members comma string */ yytestcase(yyruleno==25);
#line 207 "toml-lemon.lemon"
{
  yygotominor.yy0 = yymsp[-2].minor.yy0;
  TOMLArray_append( yygotominor.yy0, yymsp[0].minor.yy0 );
}
#line 1007 "toml-lemon.c"
        break;
      case 23: /* array_members ::= array_members comma */
      case 26: /* string_members ::= string_members comma */ yytestcase(yyruleno==26);
#line 211 "toml-lemon.lemon"
{
  yygotominor.yy0 = yymsp[-1].minor.yy0;
}
#line 1015 "toml-lemon.c"
        break;
      case 24: /* array_members ::= array */
#line 214 "toml-lemon.lemon"
{
  yygotominor.yy0 = TOML_anArray( TOML_ARRAY, yymsp[0].minor.yy0, NULL );
}
#line 1022 "toml-lemon.c"
        break;
      case 27: /* string_members ::= string */
#line 225 "toml-lemon.lemon"
{
  yygotominor.yy0 = TOML_anArray( TOML_STRING, yymsp[0].minor.yy0, NULL );
}
#line 1029 "toml-lemon.c"
        break;
      case 28: /* number_members ::= number_members comma number */
#line 230 "toml-lemon.lemon"
{
  if ( yymsp[-2].minor.yy2->memberType != yymsp[0].minor.yy45->type ) {
    _TOML_fillError( state->token, state, TOML_ERROR_ARRAY_MEMBER_MISMATCH );
  }
  yygotominor.yy2 = yymsp[-2].minor.yy2;
  TOMLArray_append( yygotominor.yy2, yymsp[0].minor.yy45 );
}
#line 1040 "toml-lemon.c"
        break;
      case 29: /* number_members ::= number_members comma */
#line 237 "toml-lemon.lemon"
{
  yygotominor.yy2 = yymsp[-1].minor.yy2;
}
#line 1047 "toml-lemon.c"
        break;
      case 30: /* number_members ::= number */
#line 240 "toml-lemon.lemon"
{
  yygotominor.yy2 = TOML_anArray( yymsp[0].minor.yy45->type, yymsp[0].minor.yy45, NULL );
}
#line 1054 "toml-lemon.c"
        break;
      case 31: /* comma ::= COMMA */
#line 244 "toml-lemon.lemon"
{
  yy_destructor(yypParser,8,&yymsp[0].minor);
}
#line 1061 "toml-lemon.c"
        break;
      case 32: /* string ::= STRING */
#line 246 "toml-lemon.lemon"
{
  TOMLToken *token = yymsp[0].minor.yy0;
  int size = token->end - token->start;

  char *tmp = _TOML_newstr( token );

  char *dest = malloc( size + 1 );
  strncpy( dest, tmp, size );
  dest[ size ] = 0;

  char *tmpCursor = tmp;
  char *destCursor = dest;

  // replace \\\" with "
  // replace \\n with \n
  // replace \\t with \t
  while ( tmpCursor != NULL ) {
    char *next = strchr( tmpCursor, '\\' );
    if ( next && next[1] ) {
      char *nextDest = destCursor + ( (int) next - (int) tmpCursor );
      if ( next[1] == '"' ) { // '"'
        strcpy( nextDest, next + 1 );
      } else if ( next[1] == 'n' ) {
        *nextDest = '\n';
        strcpy( nextDest + 1, next + 2 );
      } else if ( next[1] == 't' ) {
        *nextDest = '\t';
        strcpy( nextDest + 1, next + 2 );
      }
      size--;
      tmpCursor = next + 2;
      destCursor = nextDest + 1;
    } else {
      tmpCursor = next;
    }
  }

  yygotominor.yy0 = TOML_aStringN( dest + 1, size - 2 );

  free( dest );
  free( tmp );
}
#line 1107 "toml-lemon.c"
        break;
      case 33: /* number ::= NUMBER */
#line 290 "toml-lemon.lemon"
{
  char *tmp = _TOML_newstr( yymsp[0].minor.yy0 );

  if ( strchr( tmp, '.' ) != NULL ) {
    yygotominor.yy45 = TOML_aDouble( atof( tmp ) );
  } else {
    yygotominor.yy45 = TOML_anInt( atoi( tmp ) );
  }

  free( tmp );
}
#line 1122 "toml-lemon.c"
        break;
      case 34: /* error ::= EOF error */
#line 306 "toml-lemon.lemon"
{ yygotominor.yy61 = yymsp[0].minor.yy61;   yy_destructor(yypParser,1,&yymsp[-1].minor);
}
#line 1128 "toml-lemon.c"
        break;
      case 35: /* table_header ::= LEFT_SQUARE error */
#line 308 "toml-lemon.lemon"
{
  _TOML_fillError( yymsp[-1].minor.yy0, state, TOML_ERROR_INVALID_HEADER );
}
#line 1135 "toml-lemon.c"
        break;
      case 36: /* entry ::= id EQ error */
#line 312 "toml-lemon.lemon"
{
  _TOML_fillError( state->token, state, TOML_ERROR_NO_VALUE );
  free( yymsp[-2].minor.yy0 );
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
#line 1144 "toml-lemon.c"
        break;
      case 37: /* entry ::= id error */
#line 317 "toml-lemon.lemon"
{
  _TOML_fillError( state->token, state, TOML_ERROR_NO_EQ );
  free( yymsp[-1].minor.yy0 );
}
#line 1152 "toml-lemon.c"
        break;
      default:
      /* (1) line ::= line_and_comment */ yytestcase(yyruleno==1);
      /* (2) line ::= line line_and_comment */ yytestcase(yyruleno==2);
      /* (3) line_and_comment ::= line_content */ yytestcase(yyruleno==3);
      /* (5) line_content ::= table_header */ yytestcase(yyruleno==5);
      /* (6) line_content ::= entry */ yytestcase(yyruleno==6);
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  TOMLParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 3 "toml-lemon.lemon"
 _TOML_fillError( state->token, state, TOML_ERROR_FATAL ); 
#line 1207 "toml-lemon.c"
  TOMLParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  TOMLParserARG_FETCH;
#define TOKEN (yyminor.yy0)
  TOMLParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  TOMLParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  TOMLParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "TOMLParserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void TOMLParser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  TOMLParserTOKENTYPE yyminor       /* The value for the token */
  TOMLParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  TOMLParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
