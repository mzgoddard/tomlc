#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
      return TOML_allocTable( NULL, NULL );
    case TOML_ARRAY:
      return TOML_allocArray( TOML_NOTYPE );
    case TOML_STRING:
      return TOML_allocString( "" );
    case TOML_INT:
      return TOML_allocInt( 0 );
    case TOML_DOUBLE:
      return TOML_allocDouble( 0 );
    case TOML_BOOLEAN:
      return TOML_allocBoolean( 0 );
    case TOML_DATE:
      return TOML_allocEpochDate( 0 );
    default:
      return NULL;
  }
}

TOMLTable * TOML_allocTable( TOMLString *key, TOMLRef value, ... ) {
  TOMLTable *self = malloc( sizeof(TOMLTable) );
  self->type = TOML_TABLE;
  self->keys = TOML_allocArray( TOML_STRING, NULL );
  self->values = TOML_allocArray( TOML_NOTYPE, NULL );

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

TOMLArray * TOML_allocArray( TOMLType memberType, ... ) {
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

TOMLString * TOML_allocString( char *content ) {
  int size = strlen( content );
  TOMLString *self = malloc( sizeof(TOMLString) + size + 1 );
  self->type = TOML_STRING;
  self->size = size;
  self->content[ self->size ] = 0;
  strncpy( self->content, content, size );

  return self;
}

TOMLString * TOML_allocStringN( char *content, int n ) {
  TOMLString *self = malloc( sizeof(TOMLString) + n + 1 );
  self->type = TOML_STRING;
  self->size = n;
  self->content[ n ] = 0;
  strncpy( self->content, content, n );

  return self;
}

TOMLNumber * TOML_allocInt( int value ) {
  TOMLNumber *self = malloc( sizeof(TOMLNumber) );
  self->type = TOML_INT;
  // self->numberType = TOML_INT;
  self->intValue = value;

  return self;
}

TOMLNumber * TOML_allocDouble( double value ) {
  TOMLNumber *self = malloc( sizeof(TOMLNumber) );
  self->type = TOML_DOUBLE;
  // self->numberType = TOML_DOUBLE;
  self->doubleValue = value;

  return self;
}

TOMLBoolean * TOML_allocBoolean( int truth ) {
  TOMLBoolean *self = malloc( sizeof(TOMLBoolean) );
  self->type = TOML_BOOLEAN;
  self->isTrue = truth;
  return self;
}

int _TOML_isLeapYear( int year ) {
  if ( year % 400 == 0 ) {
    return 1;
  } else if ( year % 100 == 0 ) {
    return 0;
  } else if ( year % 4 == 0 ) {
    return 1;
  } else {
    return 0;
  }
}

TOMLDate * TOML_allocDate(
  int year, int month, int day, int hour, int minute, int second
) {
  TOMLDate *self = malloc( sizeof(TOMLDate) );
  self->type = TOML_DATE;

  self->year = year;
  self->month = month;
  self->day = day;

  self->hour = hour;
  self->minute = minute;
  self->second = second;

  struct tm _time = {
    second,
    minute,
    hour,
    day,
    month,
    year - 1900
  };

  // local time
  time_t localEpoch = mktime( &_time );
  // gm time
  _time = *gmtime( &localEpoch );
  time_t gmEpoch = mktime( &_time );

  double diff = difftime( localEpoch, gmEpoch );

  // Adjust the localEpock made by mktime to a gmt epoch.
  self->sinceEpoch = localEpoch + diff;

  return self;
}

TOMLDate * TOML_allocEpochDate( time_t stamp ) {
  TOMLDate *self = malloc( sizeof(TOMLDate) );
  self->type = TOML_DATE;
  self->sinceEpoch = stamp;

  struct tm _time = *gmtime( &stamp );

  self->second = _time.tm_sec;
  self->minute = _time.tm_min;
  self->hour = _time.tm_hour;
  self->day = _time.tm_mday;
  self->month = _time.tm_mon;
  self->year = _time.tm_year + 1900;

  return self;
}

TOMLError * TOML_allocError( int code ) {
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
    TOMLString *newString = malloc( sizeof(TOMLString) + string->size + 1 );
    newString->type = TOML_STRING;
    newString->size = string->size;
    strncpy( newString->content, string->content, string->size + 1 );
    return newString;
  } else if ( basic->type == TOML_INT || basic->type == TOML_DOUBLE ) {
    TOMLNumber *number = (TOMLNumber *) self;
    TOMLNumber *newNumber = malloc( sizeof(TOMLNumber) );
    newNumber->type = number->type;
    // newNumber->numberType = number->numberType;
    memcpy( newNumber->bytes, number->bytes, 8 );
    return newNumber;
  } else if ( basic->type == TOML_BOOLEAN ) {
    TOMLBoolean *boolean = (TOMLBoolean *) self;
    TOMLBoolean *newBoolean = malloc( sizeof(TOMLBoolean) );
    newBoolean->type = boolean->type;
    newBoolean->isTrue = boolean->isTrue;
    return newBoolean;
  } else if ( basic->type == TOML_DATE ) {
    TOMLDate *date = (TOMLDate *) self;
    TOMLDate *newDate = malloc( sizeof(TOMLDate) );
    *newDate = *date;
    return newDate;
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

int TOML_isNumber( TOMLRef self ) {
  TOMLBasic *basic = (TOMLBasic *) self;
  return basic->type == TOML_INT || basic->type == TOML_DOUBLE;
}

TOMLRef TOML_find( TOMLRef self, ... ) {
  TOMLBasic *basic = self;
  va_list args;
  va_start( args, self );

  char *key;

  do {
    if ( basic->type == TOML_TABLE ) {
      key = va_arg( args, char * );
      if ( key == NULL ) {
        break;
      }
      basic = self = TOMLTable_getKey( self, key );
    } else if ( basic->type == TOML_ARRAY ) {
      key = va_arg( args, char * );
      if ( key == NULL ) {
        break;
      }
      basic = self = TOMLArray_getIndex( self, atoi( key ) );
    } else {
      break;
    }
  } while ( self );

  va_end( args );
  return self;
}

TOMLRef TOMLTable_getKey( TOMLTable *self, char *key ) {
  int keyLength = strlen( key );
  int i;
  for ( i = 0; i < self->keys->size; ++i ) {
    TOMLString *tableKey = TOMLArray_getIndex( self->keys, i );
    int minSize = keyLength < tableKey->size ? keyLength : tableKey->size;
    if ( strncmp( tableKey->content, key, minSize + 1 ) == 0 ) {
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

  TOMLArray_append( self->keys, TOML_allocString( key ) );
  TOMLArray_append( self->values, value );
}

TOMLRef TOMLArray_getIndex( TOMLArray *self, int index ) {
  return self->members && self->size > index ? self->members[ index ] : NULL;
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

char * TOML_toString( TOMLString *self ) {
  char *string = malloc( self->size + 1 );
  TOML_copyString( self, self->size + 1, string );
  return string;
}

#define RETURN_VALUE switch ( self->type ) { \
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

struct tm TOML_toTm( TOMLDate *self ) {
  return *gmtime( &self->sinceEpoch );
}

int TOML_toBoolean( TOMLBoolean *self ) {
  return self->isTrue;
}

TOMLToken * TOML_newToken( TOMLToken *token ) {
  TOMLToken *heapToken = malloc( sizeof(TOMLToken) );
  memcpy( heapToken, token, sizeof(TOMLToken) );

  int size = token->end - token->start;
  heapToken->tokenStr = malloc( size + 1 );
  heapToken->tokenStr[ size ] = 0;
  strncpy( heapToken->tokenStr, token->start, size );

  return heapToken;
}

void TOML_strcpy( char *buffer, TOMLString *self, int size ) {
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
  if ( fd == NULL ) {
    if ( error ) {
      error->code = TOML_ERROR_FILEIO;
      error->lineNo = -1;
      error->line = NULL;

      int messageSize = strlen( TOMLErrorDescription[ error->code ] );
      error->message =
        malloc( messageSize + 1 );
      strcpy( error->message, TOMLErrorDescription[ error->code ] );
      error->message[ messageSize ] = 0;

      int fullDescSize = messageSize + strlen( filename ) + 8;
      error->fullDescription = malloc( fullDescSize + 1 );
      snprintf(
        error->fullDescription,
        fullDescSize,
        "%s File: %s",
        error->message,
        filename
      );
    }

    return TOML_ERROR_FILEIO;
  }

  int bufferSize = 0;
  char * buffer = _TOML_increaseBuffer( NULL, &bufferSize );
  int copyBufferSize = 0;
  char * copyBuffer = _TOML_increaseBuffer( NULL, &copyBufferSize );
  int read = fread( buffer, 1, bufferSize, fd );
  int incomplete = read == bufferSize;

  int hTokenId;
  TOMLToken token = { 0, NULL, NULL, buffer, 0, buffer, NULL };
  TOMLToken lastToken = token;

  TOMLTable *topTable = *dest = TOML_allocTable( NULL, NULL );
  TOMLParserState state = { topTable, topTable, 0, error, &token };

  pTOMLParser parser = TOMLParserAlloc( malloc );

  while (
    state.errorCode == 0 && (
      TOMLScan( token.end, &hTokenId, &token ) || incomplete
    )
  ) {
    while ( token.end >= buffer + bufferSize && incomplete ) {
      int lineSize = buffer + bufferSize - lastToken.lineStart;

      if ( lastToken.lineStart == buffer ) {
        int oldBufferSize = bufferSize;
        strncpy( copyBuffer, lastToken.lineStart, lineSize );
        buffer = _TOML_increaseBuffer( buffer, &bufferSize );
        copyBuffer = _TOML_increaseBuffer( copyBuffer, &copyBufferSize );
        strncpy( buffer, copyBuffer, lineSize );
      } else {
        strncpy( copyBuffer, lastToken.lineStart, lineSize );
        strncpy( buffer, copyBuffer, lineSize );
      }

      int read = fread( buffer + lineSize, 1, bufferSize - lineSize, fd );
      incomplete = read == bufferSize - lineSize;
      if ( !incomplete ) {
        buffer[ lineSize + read ] = 0;
      }

      token = lastToken;
      token.end = buffer + ( token.end - token.lineStart );
      token.lineStart = buffer;
      lastToken = token;
      TOMLScan( token.end, &hTokenId, &token );
    }

    lastToken = token;

    int tmpSize = token.end - token.start;
    char *tmp = malloc( tmpSize + 1 );
    strncpy( tmp, token.start, tmpSize );
    tmp[ tmpSize ] = 0;
    free( tmp );

    TOMLParser( parser, hTokenId, TOML_newToken( &token ), &state );
  }

  if ( state.errorCode == 0 ) {
    TOMLParser( parser, hTokenId, TOML_newToken( &token ), &state );
  }

  TOMLParserFree( parser, free );

  free( copyBuffer );
  free( buffer );
  fclose( fd );

  if ( state.errorCode != 0 ) {
    TOML_free( *dest );
    *dest = NULL;
    return state.errorCode;
  }

  return 0;
}

// int TOML_dump( char *filename, TOMLTable * );

int TOML_parse( char *buffer, TOMLTable **dest, TOMLError *error ) {
  assert( *dest == NULL );

  int hTokenId;
  TOMLToken token = { 0, NULL, NULL, buffer, 0, buffer, NULL };

  TOMLTable *topTable = *dest = TOML_allocTable( NULL, NULL );
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
    !first ||
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
    // Scan for escapable character or unicode.
    char *next = cursor;
    unsigned int ch = *next;
    for ( ;
      !(
        ch == 0 ||
          ch == '\b' ||
          ch == '\t' ||
          ch == '\f' ||
          ch == '\n' ||
          ch == '\r' ||
          ch == '"' ||
          ch == '/' ||
          ch == '\\' ||
          ch > 0x7f
      );
      next++, ch = *next
    ) {}

    if ( *next == 0 ) {
      next = NULL;
    }

    // Copy text up to character and then insert escaped character.
    if ( next ) {
      _TOML_stringifyText( self, cursor, next - cursor );

      #define REPLACE( match, value ) \
      if ( *next == match ) { \
        _TOML_stringifyText( self, value, 2 ); \
      }
      REPLACE( '\b', "\\b" )
      else REPLACE( '\t', "\\t" )
      else REPLACE( '\f', "\\f" )
      else REPLACE( '\n', "\\n" )
      else REPLACE( '\r', "\\r" )
      else REPLACE( '"', "\\\"" )
      else REPLACE( '/', "\\/" )
      else REPLACE( '\\', "\\\\" )
      #undef REPLACE
      else if ( ((unsigned int) *next ) > 0x7f ) {
        int num = 0;
        int chsize;

        // Decode the numeric representation of the utf8 character
        if ( ( *next & 0xe0 ) == 0xe0 ) {
          chsize = 3;
          num =
            ( ( next[0] & 0x0f ) << 12 ) |
              ( ( next[1] & 0x3f ) << 6 ) |
              ( next[2] & 0x3f );
        } else if ( ( *next & 0xc0 ) == 0xc0 ) {
          chsize = 2;
          num =
            ( ( next[0] & 0x1f ) << 6 ) |
              ( next[1] & 0x3f );
        } else {
          assert( 0 );
        }

        // Stringify \uxxxx
        char utf8Buffer[5];
        snprintf( utf8Buffer, 5, "%04x", num );
        _TOML_stringifyText( self, "\\u", 2 );
        _TOML_stringifyText( self, utf8Buffer, 4 );

        next += chsize - 1;
      }

      next++;
    // Copy everything up to the end.
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

  // if null
  if ( src == NULL ) {
    _TOML_stringifyText( self, "(null)", 6 );
  // if table
  } else if ( basic->type == TOML_TABLE ) {
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
  } else if ( TOML_isNumber( basic ) ) {
    TOMLNumber *number = src;
    char numberBuffer[ 16 ];
    memset( numberBuffer, 0, 16 );

    int size;
    if ( number->type == TOML_INT ) {
      size = snprintf( numberBuffer, 15, "%d", number->intValue );
    } else if ( fmod( number->doubleValue, 1 ) == 0 ) {
      size = snprintf( numberBuffer, 15, "%.1f", number->doubleValue );
    } else {
      size = snprintf( numberBuffer, 15, "%g", number->doubleValue );
    }

    // print number
    _TOML_stringifyText( self, numberBuffer, size );
  } else if ( basic->type == TOML_BOOLEAN ) {
    TOMLBoolean *boolean = (TOMLBoolean *) basic;

    if ( boolean->isTrue ) {
      _TOML_stringifyText( self, "true", 4 );
    } else {
      _TOML_stringifyText( self, "false", 5 );
    }
  } else if ( basic->type == TOML_DATE ) {
    TOMLDate *date = (TOMLDate *) basic;
    char numberBuffer[ 16 ];
    int size;

    #define STRINGIFY_DATE_SECTION( format, part, spacer ) \
      size = snprintf( numberBuffer, 15, format, date->part ); \
      _TOML_stringifyText( self, numberBuffer, size ); \
      _TOML_stringifyText( self, spacer, 1 )

    STRINGIFY_DATE_SECTION( "%d", year, "-" );
    STRINGIFY_DATE_SECTION( "%0.2d", month, "-" );
    STRINGIFY_DATE_SECTION( "%0.2d", day, "T" );
    STRINGIFY_DATE_SECTION( "%0.2d", hour, ":" );
    STRINGIFY_DATE_SECTION( "%0.2d", minute, ":" );
    STRINGIFY_DATE_SECTION( "%0.2d", second, "Z" );

    #undef STRINGIFY_DATE_SECTION
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
