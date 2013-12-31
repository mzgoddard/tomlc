#include <stdio.h>
#include <string.h>

#include "toml.h"

struct option {
  char *shortFlag;
  char *longFlag;
  char *argName;
  char *helpText;
  struct option *next;
  int argumentIndex;
  int argumentCount;
};

void add_option( struct option **head, struct option *option ) {
  if ( *head == NULL ) {
    *head = option;
    return;
  }

  struct option *prev = *head;
  while ( prev->next != NULL ) {
    prev = prev->next;
  }

  prev->next = option;
}

int available_option( struct option *head, int i ) {
  struct option *ptr = head;
  while ( ptr ) {
    if (
      ptr->argumentIndex >= i &&
        ptr->argumentIndex < i + ptr->argumentCount
    ) {
      return 0;
    }
    ptr = ptr->next;
  }
  return 1;
}

#define USAGE( usageText ) char *usageHelpText = usageText;

#define FLAG( shortOpt, longOpt, optionName, helpText ) \
  int optionName = 0; \
  struct option optionName ## Option = { \
    shortOpt, longOpt, #optionName, helpText, NULL, -1, 0 \
  }; \
  { \
  add_option( &optionhead, &optionName ## Option ); \
  \
  for ( int i = 1; i < argc; ++i ) { \
    if ( \
      strcmp( argv[ i ], shortOpt ) == 0 || \
        strcmp( argv[ i ], longOpt ) == 0 \
    ) { \
      optionName = 1; \
      optionName ## Option.argumentIndex = i; \
      optionName ## Option.argumentCount = 1; \
      break; \
    } \
  } \
}

#define STROPTION( shortOpt, longOpt, optionName, helpText ) \
  char *optionName = NULL; \
  struct option optionName ## Option = { \
    shortOpt, longOpt, #optionName, helpText, NULL, -1, 0 \
  }; \
  { \
  add_option( &optionhead, &optionName ## Option ); \
  \
  for ( int i = 1; i < argc; ++i ) { \
    if ( \
      ( \
        shortOpt && \
          strcmp( argv[ i ], shortOpt ) == 0 \
      ) || ( \
        longOpt && \
          strcmp( argv[ i ], longOpt ) == 0 \
      ) \
    ) { \
      optionName = argv[ i + 1 ]; \
      optionName ## Option.argumentIndex = i; \
      optionName ## Option.argumentCount = 2; \
      break; \
    } else if ( \
      !shortOpt && !longOpt && available_option( optionhead, i ) \
    ) { \
      optionName = argv[ i ]; \
      optionName ## Option.argumentIndex = i; \
      optionName ## Option.argumentCount = 1; \
      break; \
    } \
  } \
}

int main( int argc, char **argv ) {
  struct option *optionhead = NULL;

  USAGE( "toml [options] filepath [members]" );
  FLAG( "-h", "--help", help, "print help and exit" );
  FLAG( "-c", "--check", check, "check that given source is valid toml" );
  FLAG( "-v", "--version", version, "print version and exit" );
  STROPTION( NULL, NULL, filepath, "path to toml file" );
  STROPTION( NULL, NULL, members, "key names to lookup" );

  if ( help || filepath == NULL ) {
    printf( "%s\n", usageHelpText );
    struct option *tmp = optionhead;
    while ( tmp ) {
      if ( !tmp->shortFlag && !tmp->longFlag ) {
        printf( "\t%-15s %s\n", tmp->argName, tmp->helpText );
      } else {
        printf(
          "\t%s, %s\t%s\n", tmp->shortFlag, tmp->longFlag, tmp->helpText
        );
      }
      tmp = tmp->next;
    }
    return 0;
  }

  if ( version ) {
    printf( "toml v0.0.0\n" );
    return 0;
  }

  TOMLTable *table = NULL;
  TOMLError *error = TOML_anError( TOML_SUCCESS );

  // Load and exit if there is an error.
  if ( TOML_load( filepath, &table, error ) != TOML_SUCCESS ) {
    printf( "%s\n", error->fullDescription );
    return error->code;
  }

  TOML_free( error );

  if ( check ) {
    printf( "Ok TOML.\n" );
    return 0;
  }

  // Parse path argument.
  TOMLRef ref = table;
  TOMLBasic *basic = ref;
  {
    int tokenSize = 0;
    char *token = NULL;
    char *tokenStart = members;
    char *tokenEnd = tokenStart;

    while ( tokenEnd ) {
      tokenEnd = strpbrk( tokenStart, ".[]" );

      if ( tokenEnd ) {
        if ( tokenEnd - tokenStart > tokenSize ) {
          tokenSize = tokenEnd - tokenStart;
          free( token );
          token = malloc( tokenSize + 1 );
        }

        strncpy( token, tokenStart, tokenEnd - tokenStart );
        token[ tokenEnd - tokenStart ] = 0;

        basic = ref = TOML_find( ref, token, NULL );

        tokenEnd++;
        if ( *tokenEnd == '.' || *tokenEnd == '[' ) {
          tokenEnd++;
        }
        tokenStart = tokenEnd;
        if ( *tokenEnd == 0 ) {
          tokenEnd = NULL;
        }
      } else {
        basic = ref = TOML_find( ref, tokenStart, NULL );
      }
    }
  }

  error = TOML_anError( TOML_SUCCESS );

  char *output;
  if ( TOML_stringify( &output, ref, error ) != TOML_SUCCESS ) {
    printf( "%s\n", error->fullDescription );
    return error->code;
  }

  TOML_free( error );

  printf( "%s\n", output );
  free( output );

  return 0;
}
