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

struct _TOMLStringifyData {
  TOMLError *error;

  int bufferSize;
  int bufferIndex;
  char *buffer;
  int tableNameDepth;
  int tableNameStackSize;
  TOMLString **tableNameStack;
};

int _TOML_stringify( struct _TOMLStringifyData *self, TOMLRef src );

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
  self->content[ self->size ] = 0;
  strncpy( self->content, content, self->size );

  return self;
}

TOMLString * TOML_aStringN( char *content, int n ) {
  TOMLString *self = malloc( sizeof(TOMLString) );
  self->type = TOML_STRING;
  self->size = n;
  self->content = malloc( n + 1 );
  self->content[ n ] = 0;
  strncpy( self->content, content, n );

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

TOMLError * TOML_anError( int code ) {
  TOMLError *self = malloc( sizeof(TOMLError) );
  self->type = TOML_ERROR;
  self->code = code;
  self->lineNo = 0;
  self->line = NULL;
  self->message = NULL;
  self->fullDescription = NULL;

  return self;
}

char * _TOML_cstringCopy( char *str ) {
  if ( !str ) {
    return NULL;
  }

  int size = strlen( str );
  char *newstr = malloc( size + 1 );
  newstr[ size ] = 0;
  strncpy( newstr, str, size );

  return newstr;
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
  } else if ( basic->type == TOML_ERROR ) {
    TOMLError *error = (TOMLError *) self;
    TOMLError *newError = malloc( sizeof(TOMLError) );
    newError->type = TOML_ERROR;
    newError->code = error->code;
    newError->lineNo = error->lineNo;
    newError->line = _TOML_cstringCopy( error->line );
    newError->message = _TOML_cstringCopy( error->message );
    newError->fullDescription = _TOML_cstringCopy( error->fullDescription );
    return newError;
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
  } else if ( basic->type == TOML_ERROR ) {
    TOMLError *error = (TOMLError *) self;
    free( error->line );
    free( error->message );
    free( error->fullDescription );
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
      if ( key == NULL ) {
        break;
      }
      basic = self = TOMLTable_getKey( self, key );
    } else if ( basic->type == TOML_ARRAY ) {
      index = va_arg( args, int );
      if ( index == -1 ) {
        break;
      }
      basic = self = TOMLArray_getIndex( self, index );
    } else {
      break;
    }
  } while ( self );

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

TOMLToken * TOML_newToken( TOMLToken *token ) {
  TOMLToken *heapToken = malloc( sizeof(TOMLToken) );
  memcpy( heapToken, token, sizeof(TOMLToken) );
  return heapToken;
}

void TOML_copyString( TOMLString *self, int size, char *buffer ) {
  if ( self->type != TOML_STRING ) {
    buffer[0] = 0;
  } else {
    strncpy(
      buffer, self->content, size < self->size + 1 ? size : self->size + 1
    );
  }
}

char * _TOML_increaseBuffer( char *oldBuffer, int *size ) {
  int newSize = *size + 1024;
  char *newBuffer = malloc( newSize + 1 );
  // Always have a null terminator so TOMLScan can exit without segfault.
  newBuffer[ newSize ] = 0;

  if ( oldBuffer ) {
    strncpy( newBuffer, oldBuffer, *size + 1 );
    free( oldBuffer );
  }

  *size = newSize;

  return newBuffer;
}

int TOML_load( char *filename, TOMLTable **dest, TOMLError *error ) {
  assert( *dest == NULL );

  FILE *fd = fopen( filename, "r" );
  int bufferSize = 0;
  char * buffer = _TOML_increaseBuffer( NULL, &bufferSize );
  int read = fread( buffer, 1, bufferSize, fd );
  int incomplete = read == bufferSize;

  int hTokenId;
  TOMLToken token = { 0, NULL, NULL, buffer, 0, buffer };

  TOMLTable *topTable = *dest = TOML_aTable( NULL, NULL );
  TOMLParserState state = { topTable, topTable, 0, error, &token };

  pTOMLParser parser = TOMLParserAlloc( malloc );

  while ( state.errorCode == 0 && TOMLScan( token.end, &hTokenId, &token ) ) {
    while ( token.end == buffer + bufferSize && incomplete ) {
      int scanSize = token.end - token.start;

      if ( token.start == buffer ) {
        int oldBufferSize = bufferSize;
        buffer = _TOML_increaseBuffer( buffer, &bufferSize );

        strncpy( buffer, buffer + oldBufferSize - scanSize, scanSize );
        int read = fread( buffer + scanSize, 1, bufferSize - scanSize, fd );
        int incomplete = read == bufferSize - scanSize;
        TOMLScan( token.end, &hTokenId, &token );
      } else {
        strncpy( buffer, buffer + bufferSize - scanSize, scanSize );
        int read = fread( buffer + scanSize, 1, bufferSize - scanSize, fd );
        int incomplete = read == bufferSize - scanSize;
        TOMLScan( token.end, &hTokenId, &token );
      }
    }

    TOMLParser( parser, hTokenId, TOML_newToken( &token ), &state );
  }

  if ( state.errorCode == 0 ) {
    TOMLParser( parser, hTokenId, TOML_newToken( &token ), &state );
  }

  TOMLParserFree( parser, free );

  if ( state.errorCode != 0 ) {
    TOML_free( *dest );
    *dest = NULL;
    return state.errorCode;
  }

  fclose( fd );

  return 0;
}

// int TOML_dump( char *filename, TOMLTable * );

int TOML_parse( char *buffer, TOMLTable **dest, TOMLError *error ) {
  assert( *dest == NULL );

  int hTokenId;
  TOMLToken token = { 0, NULL, NULL, buffer, 0, buffer };

  TOMLTable *topTable = *dest = TOML_aTable( NULL, NULL );
  TOMLParserState state = { topTable, topTable, 0, error, &token };

  pTOMLParser parser = TOMLParserAlloc( malloc );

  while ( state.errorCode == 0 && TOMLScan( token.end, &hTokenId, &token ) ) {
    TOMLParser( parser, hTokenId, TOML_newToken( &token ), &state );
  }

  if ( state.errorCode == 0 ) {
    TOMLParser( parser, hTokenId, TOML_newToken( &token ), &state );
  }

  TOMLParserFree( parser, free );

  if ( state.errorCode != 0 ) {
    TOML_free( *dest );
    *dest = NULL;
    return state.errorCode;
  }

  return 0;
}

TOMLString ** _TOML_increaseNameStack(
  TOMLString **nameStack, int *nameStackSize
) {
  TOMLString **oldStack = nameStack;
  int oldSize = *nameStackSize;
  *nameStackSize += 16;
  nameStack = malloc( *nameStackSize * sizeof(TOMLString *) );
  if ( oldStack ) {
    memcpy( nameStack, oldStack, oldSize );
    free( oldStack );
  }
  return nameStack;
}

void _TOML_stringifyPushName(
  struct _TOMLStringifyData *self, TOMLRef src
) {
  if ( self->tableNameDepth >= self->tableNameStackSize ) {
    self->tableNameStack = _TOML_increaseNameStack(
      self->tableNameStack,
      &( self->tableNameStackSize )
    );
  }
  self->tableNameStack[ self->tableNameDepth ] = src;
  self->tableNameDepth++;
}

void _TOML_stringifyPopName(
  struct _TOMLStringifyData *self
) {
  self->tableNameDepth--;
  self->tableNameStack[ self->tableNameDepth ] = NULL;
}

void _TOML_stringifyText( struct _TOMLStringifyData *self, char *text, int n ) {
  if ( self->bufferIndex + n + 1 >= self->bufferSize ) {
    self->buffer = _TOML_increaseBuffer( self->buffer, &self->bufferSize );
  }
  strncpy( self->buffer + self->bufferIndex, text, n );
  self->bufferIndex += n;
  self->buffer[ self->bufferIndex ] = 0;
}

void _TOML_stringifyTableHeader(
  struct _TOMLStringifyData *self, TOMLTable *table
) {
  TOMLBasic *first = TOMLArray_getIndex( table->values, 0 );
  if (
    first->type == TOML_TABLE || (
      first->type == TOML_ARRAY &&
      ((TOMLArray *) first)->memberType == TOML_TABLE
    )
  ) {
    return;
  }

  if ( self->bufferIndex != 0 ) {
    _TOML_stringifyText( self, "\n", 1 );
  }

  _TOML_stringifyText( self, "[", 1 );
  for ( int i = 0; i < self->tableNameDepth; ++i ) {
    TOMLString *tableName = self->tableNameStack[ i ];
    if ( i > 0 ) {
      _TOML_stringifyText( self, ".", 1 );
    }
    _TOML_stringifyText( self, tableName->content, tableName->size );
  }
  _TOML_stringifyText( self, "]\n", 2 );
}

void _TOML_stringifyArrayHeader( struct _TOMLStringifyData *self ) {
  if ( self->bufferIndex != 0 ) {
    _TOML_stringifyText( self, "\n", 1 );
  }

  _TOML_stringifyText( self, "[[", 2 );
  for ( int i = 0; i < self->tableNameDepth; ++i ) {
    TOMLString *tableName = self->tableNameStack[ i ];
    if ( i > 0 ) {
      _TOML_stringifyText( self, ".", 1 );
    }
    _TOML_stringifyText( self, tableName->content, tableName->size );
  }
  _TOML_stringifyText( self, "]]\n", 3 );
}

void _TOML_stringifyString(
  struct _TOMLStringifyData *self, TOMLString *string
) {
  char *cursor = string->content;
  while ( cursor != NULL ) {
    char *next = strpbrk( cursor, "\"\n\t" );

    if ( next ) {
      _TOML_stringifyText( self, cursor, next - cursor );
      if ( *next == '\"' ) {
        _TOML_stringifyText( self, "\\\"", 2 );
      } else if ( *next == '\n' ) {
        _TOML_stringifyText( self, "\\n", 2 );
      } else if ( *next == '\t' ) {
        _TOML_stringifyText( self, "\\t", 2 );
      }
      next++;
    } else {
      _TOML_stringifyText( self, cursor, strlen( cursor ) );
    }

    cursor = next;
  }
}

void _TOML_stringifyEntry(
  struct _TOMLStringifyData *self, TOMLString *key, TOMLBasic *value
) {
  _TOML_stringifyText( self, key->content, key->size );
  _TOML_stringifyText( self, " = ", 3 );

  if ( value->type == TOML_STRING ) {
    _TOML_stringifyText( self, "\"", 1 );
    _TOML_stringifyString( self, (TOMLString *) value );
    _TOML_stringifyText( self, "\"", 1 );
  } else {
    _TOML_stringify( self, value );
  }

  _TOML_stringifyText( self, "\n", 1 );
}

int _TOML_stringify(
  struct _TOMLStringifyData *self, TOMLRef src
) {
  // Cast to TOMLBasic to discover type.
  TOMLBasic *basic = src;

  // if table
  if ( basic->type == TOML_TABLE ) {
    TOMLTable *table = src;

    // loop keys
    for ( int i = 0; i < table->keys->size; ++i ) {
      TOMLRef key = TOMLArray_getIndex( table->keys, i );
      TOMLRef value = TOMLArray_getIndex( table->values, i  );
      TOMLBasic *basicValue = value;

      // if value is table, print header, recurse
      if ( basicValue->type == TOML_TABLE ) {
        TOMLTable *tableValue = value;
        _TOML_stringifyPushName( self, key );
        _TOML_stringifyTableHeader( self, value );
        _TOML_stringify( self, value );
        _TOML_stringifyPopName( self );
      // if value is array
      } else if ( basicValue->type == TOML_ARRAY ) {
        TOMLArray *array = value;

        // if value is object array
        if ( array->memberType == TOML_TABLE ) {
          // loop indices, print headers, recurse
          for ( int j = 0; j < array->size; ++j ) {
            _TOML_stringifyPushName( self, key );
            _TOML_stringifyArrayHeader( self );
            _TOML_stringify( self, TOMLArray_getIndex( array, j ) );
            _TOML_stringifyPopName( self );
          }
        } else {
          // print entry line with dense (no newlines) array
          _TOML_stringifyEntry( self, key, value );
        }
      } else {
        // if value is string or number, print entry
        _TOML_stringifyEntry( self, key, value );
      }
    }
  // if array
  } else if ( basic->type == TOML_ARRAY ) {
    TOMLArray *array = src;

    // print array densely
    _TOML_stringifyText( self, "[", 1 );
    for ( int i = 0; i < array->size; ++i ) {
      _TOML_stringifyText( self, " ", 1 );
      TOMLBasic *arrayValue = TOMLArray_getIndex( array, i );
      if ( arrayValue->type == TOML_STRING ) {
        _TOML_stringifyText( self, "\"", 1 );
        _TOML_stringifyString( self, (TOMLString *) arrayValue );
        _TOML_stringifyText( self, "\"", 1 );
      } else {
        _TOML_stringify( self, arrayValue );
      }
      if ( i != array->size - 1 ) {
        _TOML_stringifyText( self, ",", 1 );
      } else {
        _TOML_stringifyText( self, " ", 1 );
      }
    }
    _TOML_stringifyText( self, "]", 1 );
  // if string
  } else if ( basic->type == TOML_STRING ) {
    TOMLString *string = src;

    // print string
    _TOML_stringifyText( self, string->content, string->size );
  // if number
  } else if ( basic->type == TOML_NUMBER ) {
    TOMLNumber *number = src;
    char numberBuffer[ 16 ];
    memset( numberBuffer, 0, 16 );

    int size;
    if ( number->numberType == TOML_INT ) {
      size = snprintf( numberBuffer, 15, "%d", number->intValue );
    } else {
      size = snprintf( numberBuffer, 15, "%f", number->doubleValue );
    }

    // print number
    _TOML_stringifyText( self, numberBuffer, size );
  } else {
    assert( 0 );
  }
  // if error
    // print error

  return 0;
}

int TOML_stringify( char **buffer, TOMLRef src, TOMLError *error ) {
  int bufferSize = 0;
  char *output = _TOML_increaseBuffer( NULL, &bufferSize );

  int stackSize = 0;
  TOMLString **tableNameStack = _TOML_increaseNameStack( NULL, &stackSize );

  struct _TOMLStringifyData stringifyData = {
    error,

    bufferSize,
    0,
    output,
    0,
    stackSize,
    tableNameStack
  };

  int errorCode = _TOML_stringify( &stringifyData, src );

  free( tableNameStack );
  *buffer = stringifyData.buffer;

  return errorCode;
}
