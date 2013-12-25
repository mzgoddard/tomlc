#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <antlr3.h>

#include "toml.h"
#include "toml-parser.h"
// #include "tomlParser.h"
// #include "tomlLexer.h"

TOMLRef TOML_alloc( TOMLType type ) {
  switch ( type ) {
    case TOML_TABLE:
      return TOML_aTable( NULL, NULL );
    case TOML_ARRAY:
      return TOML_anArray( TOML_NOTYPE );
    case TOML_STRING:
      return TOML_aString( "" );
    case TOML_NUMBER:
      return TOML_anInt( 0 );
    default:
      return NULL;
  }
}

TOMLTable * TOML_aTable( TOMLString *key, TOMLRef value, ... ) {
  TOMLTable *self = malloc( sizeof(TOMLTable) );
  self->type = TOML_TABLE;
  self->keys = TOML_anArray( TOML_STRING, NULL );
  self->values = TOML_anArray( TOML_NOTYPE, NULL );

  if ( key != NULL ) {
    TOMLArray_append( self->keys, key );
    TOMLArray_append( self->values, value );
  } else {
    return self;
  }

  va_list args;
  va_start( args, value );

  key = va_arg( args, TOMLString * );
  while ( key != NULL ) {
    value = va_arg( args, TOMLRef );
    TOMLArray_append( self->keys, key );
    TOMLArray_append( self->values, value );
    key = va_arg( args, TOMLString * );
  }

  va_end( args );

  return self;
}

TOMLArray * TOML_anArray( TOMLType memberType, ... ) {
  TOMLArray *self = malloc( sizeof(TOMLArray) );
  self->type = TOML_ARRAY;
  self->memberType = memberType;
  self->size = 0;
  self->members = NULL;

  va_list args;
  va_start( args, memberType );

  TOMLRef member = va_arg( args, TOMLRef );
  while ( member != NULL ) {
    TOMLArray_append( self, member );
    member = va_arg( args, TOMLRef );
  }

  va_end( args );

  return self;
}

TOMLString * TOML_aString( char *content ) {
  TOMLString *self = malloc( sizeof(TOMLString) );
  self->type = TOML_STRING;
  self->size = strlen( content );
  self->content = malloc( self->size + 1 );
  strncpy( self->content, content, self->size + 1 );

  return self;
}

TOMLString * TOML_aStringN( char *content, int n ) {
  TOMLString *self = malloc( sizeof(TOMLString) );
  self->type = TOML_STRING;
  self->size = n;
  self->content = malloc( self->size + 1 );
  strncpy( self->content, content, self->size );
  self->content[ n ] = 0;

  return self;
}

TOMLNumber * TOML_anInt( int value ) {
  TOMLNumber *self = malloc( sizeof(TOMLNumber) );
  self->type = TOML_NUMBER;
  self->numberType = TOML_INT;
  self->intValue = value;

  return self;
}

TOMLNumber * TOML_aDouble( double value ) {
  TOMLNumber *self = malloc( sizeof(TOMLNumber) );
  self->type = TOML_NUMBER;
  self->numberType = TOML_DOUBLE;
  self->doubleValue = value;

  return self;
}

TOMLRef TOML_copy( TOMLRef self ) {
  TOMLBasic *basic = (TOMLBasic *) self;

  if ( basic->type == TOML_TABLE ) {
    TOMLTable *table = (TOMLTable *) self;
    TOMLTable *newTable = malloc( sizeof(TOMLTable) );
    newTable->type = TOML_TABLE;
    newTable->keys = TOML_copy( table->keys );
    newTable->values = TOML_copy( table->values );
    return newTable;
  } else if ( basic->type == TOML_ARRAY ) {
    TOMLArray *array = (TOMLArray *) self;
    TOMLArray *newArray = malloc( sizeof(TOMLArray) );
    newArray->type = TOML_ARRAY;
    newArray->memberType = array->memberType;
    int i;
    for ( i = 0; i < array->size; ++i ) {
      TOMLArray_append(
        newArray,
        TOML_copy( TOMLArray_getIndex( array, i ) )
      );
    }
    return newArray;
  } else if ( basic->type == TOML_STRING ) {
    TOMLString *string = (TOMLString *) self;
    TOMLString *newString = malloc( sizeof(TOMLString) );
    newString->type = TOML_STRING;
    newString->size = string->size;
    newString->content = malloc( string->size + 1 );
    strncpy( newString->content, string->content, string->size + 1 );
    return newString;
  } else if ( basic->type == TOML_NUMBER ) {
    TOMLNumber *number = (TOMLNumber *) self;
    TOMLNumber *newNumber = malloc( sizeof(TOMLNumber) );
    newNumber->type = TOML_NUMBER;
    newNumber->numberType = number->numberType;
    memcpy( newNumber->bytes, number->bytes, 8 );
    return newNumber;
  } else {
    return NULL;
  }
}

void TOML_free( TOMLRef self ) {
  TOMLBasic *basic = (TOMLBasic *) self;

  if ( basic->type == TOML_TABLE ) {
    TOMLTable *table = (TOMLTable *) self;
    TOML_free( table->keys );
    TOML_free( table->values );
  } else if ( basic->type == TOML_ARRAY ) {
    TOMLArray *array = (TOMLArray *) self;
    int i;
    for ( i = 0; i < array->size; ++i ) {
      TOML_free( array->members[ i ] );
    }
    free( array->members );
  } else if ( basic->type == TOML_STRING ) {
    TOMLString *string = (TOMLString *) self;
    free( string->content );
  }

  free( self );
}

int TOML_isType( TOMLRef self, TOMLType type ) {
  TOMLBasic *basic = (TOMLBasic *) self;
  return basic->type == type;
}

TOMLRef TOML_find( TOMLRef self, ... ) {
  TOMLBasic *basic = self;
  va_list args;
  va_start( args, self );

  char *key;
  int index;

  do {
    if ( basic->type == TOML_TABLE ) {
      key = va_arg( args, char * );
      index = -1;
      basic = self = TOMLTable_getKey( self, key );
    } else if ( basic->type == TOML_ARRAY ) {
      key = NULL;
      index = va_arg( args, int );
      basic = self = TOMLArray_getIndex( self, index );
    } else {
      key = NULL;
      index = -1;
    }
  } while ( ( key != NULL || index != -1 ) && self );

  va_end( args );
  return self;
}

TOMLRef TOMLTable_find( TOMLTable *self, char *key, ... ) {
  va_list args;
  va_start( args, key );

  while ( key != NULL && self ) {
    self = TOMLTable_getKey( self, key );
    key = va_arg( args, char * );
  }

  va_end( args );

  return self;
}

TOMLRef TOMLTable_getKey( TOMLTable *self, char *key ) {
  int keyLength = strlen( key );
  int i;
  for ( i = 0; i < self->keys->size; ++i ) {
    TOMLString *tableKey = TOMLArray_getIndex( self->keys, i );
    int minSize = keyLength < tableKey->size ? keyLength : tableKey->size;
    if ( strncmp( tableKey->content, key, minSize ) == 0 ) {
      return TOMLArray_getIndex( self->values, i );
    }
  }
  return NULL;
}

void TOMLTable_setKey( TOMLTable *self, char *key, TOMLRef value ) {
  int keyLength = strlen( key );
  int i;
  for ( i = 0; i < self->keys->size; ++i ) {
    TOMLString *tableKey = TOMLArray_getIndex( self->keys, i );
    int minSize = keyLength < tableKey->size ? keyLength : tableKey->size;
    if ( strncmp( tableKey->content, key, minSize ) == 0 ) {
      TOMLArray_setIndex( self->values, i, value );
      return;
    }
  }

  TOMLArray_append( self->keys, TOML_aString( key ) );
  TOMLArray_append( self->values, value );
}

TOMLRef TOMLArray_getIndex( TOMLArray *self, int index ) {
  return self->members[ index ];
}

void TOMLArray_setIndex( TOMLArray *self, int index, TOMLRef value ) {
  if ( index < self->size ) {
    TOML_free( self->members[ index ] );
    self->members[ index ] = value;
  } else {
    TOMLArray_append( self, value );
  }
}

void TOMLArray_append( TOMLArray *self, TOMLRef value ) {
  TOMLRef *oldMembers = self->members;

  self->members = malloc( ( self->size + 1 ) * sizeof(TOMLRef) );
  int i = 0;
  for ( ; i < self->size; ++i ) {
    self->members[ i ] = oldMembers[ i ];
  }
  // memcpy( self->members, oldMembers, self->size * sizeof(TOMLRef) );
  self->members[ self->size ] = value;
  self->size++;

  free( oldMembers );
}

#define RETURN_VALUE switch ( self->numberType ) { \
    case TOML_INT: \
      return self->intValue; \
    case TOML_DOUBLE: \
      return self->doubleValue; \
    default: \
      return 0; \
  }

int TOML_toInt( TOMLNumber *self ) {
  RETURN_VALUE;
}

double TOML_toDouble( TOMLNumber *self ) {
  RETURN_VALUE;
}

#undef RETURN_VALUE

void TOML_copyString( TOMLString *self, int size, char *buffer ) {
  if ( self->type != TOML_STRING ) {
    buffer[0] = 0;
  } else {
    strncpy( buffer, self->content, size < self->size ? size : self->size );
  }
}

// int TOML_load( char *filename, TOMLTable **dest ) {
//   FILE *fd = fopen( filename, "r" );
//   char *buffer = malloc( 1024 );
//   int read = fread( buffer, 1, 1024, fd );
// }

// int TOML_dump( char *filename, TOMLTable * );

int TOML_parse( char *buffer, TOMLTable **dest ) {
  assert( *dest == NULL );

  TOMLTable *topTable = TOML_aTable( NULL, NULL );
  TOMLTable *currentTable = topTable;
  *dest = topTable;

  int hTokenId;
  TOMLToken token;
  token.end = buffer;

  TOMLParserState state = {
    topTable,
    topTable
  };

  pTOMLParser parser = TOMLParserAlloc( malloc );

  char tmp[4096];

  while ( TOMLScan( token.end, &hTokenId, &token ) ) {
    TOMLToken *heapToken = malloc( sizeof(TOMLToken) );
    memcpy( heapToken, &token, sizeof(TOMLToken) );

    TOMLParser( parser, hTokenId, heapToken, &state );
  }

  TOMLToken *heapToken = malloc( sizeof(TOMLToken) );
  memcpy( heapToken, &token, sizeof(TOMLToken) );

  TOMLParser( parser, hTokenId, heapToken, &state );

  TOMLParserFree( parser, free );

  return 0;
}

// int TOML_stringify( char **buffer, TOMLRef src ) {
//
// }
