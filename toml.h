#ifndef TOML_H_N6JCXECL
#define TOML_H_N6JCXECL

// Values identifying what the TOML object is.
typedef enum {
  TOML_NOTYPE,
  TOML_TABLE,
  TOML_ARRAY,
  TOML_STRING,
  TOML_NUMBER,
  TOML_ERROR
} TOMLType;

// Values identifying what the underlying number type is.
typedef enum {
  TOML_INT,
  TOML_DOUBLE
} TOMLNumberType;

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
  TOMLNumberType numberType;
  union {
    int intValue;
    double doubleValue;
    char bytes[8];
  };
} TOMLNumber;

typedef struct TOMLError {
  TOMLType type;
  int lineNo;
  char * line;
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

TOMLRef TOML_copy( TOMLRef );
void TOML_free( TOMLRef );

/*****************
 ** Interaction **
 *****************/

int TOML_isType( TOMLRef, TOMLType );

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
// Returns 1 if there was an error.
int TOML_load( char *filename, TOMLTable ** );

// Writes a stringified table to the indicated file.
// Returns 1 if there was an error.
int TOML_dump( char *filename, TOMLTable * );

// Allocates a table filled with the parsed content of the buffer.
// Returns 1 if there was an error.
int TOML_parse( char *buffer, TOMLTable ** );

// Allocates a string filled a string version of the table.
// Returns 1 if there was an error.
int TOML_stringify( char **buffer, TOMLTable * );

#endif /* end of include guard: TOML_H_N6JCXECL */
