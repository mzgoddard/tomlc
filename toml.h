#ifndef TOML_H_N6JCXECL
#define TOML_H_N6JCXECL

#include <stdlib.h>

// Values identifying what the TOML object is.
typedef enum {
  TOML_NOTYPE,
  TOML_TABLE,
  TOML_ARRAY,
  TOML_STRING,
  TOML_INT,
  TOML_DOUBLE,
  TOML_BOOLEAN,
  TOML_DATE,
  TOML_ERROR
} TOMLType;

// Values identifying what the underlying number type is.
// typedef enum {
//   TOML_INT,
//   TOML_DOUBLE
// } TOMLNumberType;

typedef enum {
  TOML_SUCCESS,
  TOML_ERROR_FILEIO,
  TOML_ERROR_FATAL,
  TOML_ERROR_TABLE_DEFINED,
  TOML_ERROR_ENTRY_DEFINED,
  TOML_ERROR_NO_VALUE,
  TOML_ERROR_NO_EQ,
  TOML_ERROR_INVALID_HEADER,
  TOML_ERROR_ARRAY_MEMBER_MISMATCH
} TOMLErrorType;

static char *TOMLErrorStrings[] = {
  "TOML_SUCCESS",
  "TOML_ERROR_FILEIO",
  "TOML_ERROR_FATAL",
  "TOML_ERROR_TABLE_DEFINED",
  "TOML_ERROR_ENTRY_DEFINED",
  "TOML_ERROR_NO_VALUE",
  "TOML_ERROR_NO_EQ",
  "TOML_ERROR_INVALID_HEADER",
  "TOML_ERROR_ARRAY_MEMBER_MISMATCH"
};

static char *TOMLErrorDescription[] = {
  NULL,
  "Error reading from/writing to file.",
  "Fatal error.",
  "Table is already defined.",
  "Entry is already defined.",
  "Missing valid value.",
  "Missing equal sign in table entry.",
  "Incomplete table header.",
  "Array member must be the same type as other members."
};

// Arbitrary pointer to a TOML object.
typedef void * TOMLRef;

// Struct defining the common part of all TOML objects, giving access to
// the type.
typedef struct TOMLBasic {
  TOMLType type;
} TOMLBasic;

// A TOML array.
typedef struct TOMLArray {
  TOMLType type;
  TOMLType memberType;
  int size;
  TOMLRef *members;
} TOMLArray;

// A TOML table.
typedef struct TOMLTable {
  TOMLType type;
  TOMLArray *keys;
  TOMLArray *values;
} TOMLTable;

// A TOML string.
typedef struct TOMLString {
  TOMLType type;
  int size;
  char *content;
} TOMLString;

// A TOML number.
typedef struct TOMLNumber {
  TOMLType type;
  union {
    int intValue;
    double doubleValue;
    char bytes[8];
  };
} TOMLNumber;

// A TOML boolean.
typedef struct TOMLBoolean {
  TOMLType type;
  int isTrue;
} TOMLBoolean;

// A TOML date.
typedef struct TOMLDate {
  TOMLType type;
  long int sinceEpoch;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} TOMLDate;

typedef struct TOMLError {
  TOMLType type;
  TOMLErrorType code;
  int lineNo;
  char * line;
  char * message;
  char * fullDescription;
} TOMLError;

/**********************
 ** Memory Functions **
 **********************/

TOMLRef TOML_alloc( TOMLType );

// Allocates a table and assigns key, value pairs. Table takes ownership of any
// keys and values given and they will be freed if overwritten with setKey or
// freed by TOML_free.
TOMLTable * TOML_aTable( TOMLString *key, TOMLRef value, ... );

// Allocates an array and appends any values given. Array takes ownership of
// any keys and values given and they will be freed if overwritten with
// setIndex or freed by TOML_free.
TOMLArray * TOML_anArray( TOMLType memberType, ... );

// TOML_aString creates a copy of the given string.
TOMLString * TOML_aString( char *content );
TOMLString * TOML_aStringN( char *content, int n );
TOMLNumber * TOML_anInt( int value );
TOMLNumber * TOML_aDouble( double value );
TOMLBoolean * TOML_aBoolean( int truth );
TOMLDate * TOML_aDate( int, int, int, int, int, int );
TOMLDate * TOML_anEpochDate( long int stamp );
TOMLError * TOML_anError( int code );

TOMLRef TOML_copy( TOMLRef );
void TOML_free( TOMLRef );

/*****************
 ** Interaction **
 *****************/

int TOML_isType( TOMLRef, TOMLType );
int TOML_isNumber( TOMLRef );

TOMLRef TOML_find( TOMLRef, ... );

TOMLRef TOMLTable_find( TOMLTable *, char *, ... );
TOMLRef TOMLTable_getKey( TOMLTable *, char * );
void TOMLTable_setKey( TOMLTable *, char *, TOMLRef );

TOMLRef TOMLArray_getIndex( TOMLArray *, int index );

// Set index of array to the given value. If the index is greater than or equal
// to the current size of the array, the value will be appended to the end.
void TOMLArray_setIndex( TOMLArray *, int index, TOMLRef );

void TOMLArray_append( TOMLArray *, TOMLRef );

int TOML_toInt( TOMLNumber * );
double TOML_toDouble( TOMLNumber * );
void TOML_copyString( TOMLString *, int, char * );

/************************
 ** Loading and Saving **
 ************************/

// Allocates a table filled with the parsed content of the file.
// Returns non-zero if there was an error.
int TOML_load( char *filename, TOMLTable **, TOMLError * );

// Writes a stringified table to the indicated file.
// Returns non-zero if there was an error.
// TODO: Implement TOML_dump.
// int TOML_dump( char *filename, TOMLTable *, TOMLError * );

// Allocates a table filled with the parsed content of the buffer.
// Returns non-zero if there was an error.
int TOML_parse( char *buffer, TOMLTable **, TOMLError * );

// Allocates a string filled a string version of the table.
// Returns non-zero if there was an error.
int TOML_stringify( char **buffer, TOMLRef, TOMLError * );

#endif /* end of include guard: TOML_H_N6JCXECL */
