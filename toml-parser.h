#ifndef TOML_PARSER_H_Y469ZFLB
#define TOML_PARSER_H_Y469ZFLB

#include <string.h>

#include "toml.h"

typedef struct TOMLParser {} *pTOMLParser;

typedef struct TOMLParserState {
  TOMLTable *rootTable;
  TOMLTable *currentTable;
} TOMLParserState;

typedef struct TOMLToken {
  int token;
  char *marker;
  char *start;
  char *end;
} TOMLToken;

void * TOMLParserAlloc( void * (*malloc)( size_t ) );
void TOMLParserFree( void *, void (*free)( void * ) );
void TOMLParser( void *, int hTokenId, void *, TOMLParserState * );

int TOMLScan(char *p, int* token, TOMLToken * );

#endif /* end of include guard: TOML_PARSER_H_Y469ZFLB */
