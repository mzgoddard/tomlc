#include <math.h>
#include <stdlib.h>

#include "tap.h"
#include "toml.h"

int main() {
  ansicolor( getenv( "ANSICOLOR" ) != NULL );
  plan( 75 );

  note( "\n** memory management **" );

  { /** alloc_number **/
    note( "alloc_number" );
    TOMLNumber *number = TOML_allocInt( 1 );
    ok( TOML_toInt( number ) == 1, "number is 1" );
    TOML_free( number );
  }

  { /** alloc_string **/
    note( "alloc_string" );
    TOMLString *string = TOML_allocString( "Why, yes." );
    char string_buffer[256];
    TOML_copyString( string, 256, string_buffer );
    is( string_buffer, "Why, yes.", "string is correct" );
    TOML_free( string );
  }

  { /** alloc_date **/
    note( "alloc_date" );
    TOMLDate *date = TOML_allocDate( 2013, 11, 20, 14, 30, 0 );
    ok( date->sinceEpoch == 1387549800 );
    TOML_free( date );

    date = TOML_allocEpochDate( 1387549800 );
    ok( date->year == 2013 );
    ok( date->month == 11 );
    ok( date->day == 20 );
    ok( date->hour == 14 );
    ok( date->minute == 30 );
    ok( date->second == 0 );
    TOML_free( date );
  }

  { /** alloc_array **/
    note( "alloc_array" );
    TOMLArray *array = TOML_allocArray( TOML_INT,
      TOML_allocInt( 2 ),
      TOML_allocInt( 3 ),
      NULL
    );
    ok( TOML_toInt( TOMLArray_getIndex( array, 0 ) ) == 2, "array[0] is 2" );
    ok( TOML_toInt( TOMLArray_getIndex( array, 1 ) ) == 3, "array[1] is 3" );
    TOML_free( array );
  }

  { /** alloc_table **/
    note( "alloc_table" );
    TOMLTable *table = TOML_allocTable(
      TOML_allocString( "key" ), TOML_allocString( "value" ),
      NULL, NULL
    );
    TOMLString *table_value = TOMLTable_getKey( table, "key" );
    char value_buffer[256];
    TOML_copyString( table_value, 256, value_buffer );
    is( value_buffer, "value", "table \"key\" is \"value\"" );
    TOML_free( table );
  }

  note( "\n** parse **" );

  { /** parse_entry_string **/
    note( "parse_entry_string" );
    TOMLTable *table = NULL;
    TOML_parse( "world = \"hello\"\n", &table, NULL );
    ok( table != NULL, "table created" );
    TOMLRef world = TOMLTable_getKey( table, "world" );
    char *buffer = malloc( 256 );
    TOML_copyString( world, 256, buffer );
    is( buffer, "hello", "string contained hello" );
    free( buffer );
    TOML_free( table );

    // escapes
    table = NULL;
    TOML_parse( "escapes = \"\\b\\t\\f\\n\\r\\\"\\/\\\\\"", &table, NULL );
    ok( table != NULL, "table created" );
    ok( table->keys->size == 1, "table has a member" );
    ok(
      TOML_find( table, "escapes", NULL ) != NULL,
      "table contained escapes"
    );
    is(
      ((TOMLString *) TOML_find( table, "escapes", NULL ))->content,
      "\b\t\f\n\r\"/\\",
      "member contained escaped characters"
    );
    TOML_free( table );
  }

  { /** parse_entry_int **/
    note( "parse_entry_int" );
    TOMLTable *table = NULL;
    TOML_parse( "world = 123\n", &table, NULL );
    ok( table != NULL );
    ok( TOMLTable_getKey( table, "world" ) != NULL );
    ok( TOML_toInt( TOMLTable_getKey( table, "world" ) ) == 123 );
    TOML_free( table );
  }

  { /** parse_entry_double **/
    note( "parse_entry_double" );
    TOMLTable *table = NULL;
    TOML_parse( "world = 1.23\n", &table, NULL );
    ok( table != NULL );
    ok( TOMLTable_getKey( table, "world" ) != NULL );
    ok( TOML_toDouble( TOMLTable_getKey( table, "world" ) ) == 1.23 );
    TOML_free( table );
  }

  { /** parse_entry_boolean **/
    note( "parse_entry_boolean" );
    TOMLTable *table = NULL;
    TOML_parse( "world = true\nmoon = false", &table, NULL );
    ok( table != NULL );
    ok( TOML_find( table, "world", NULL ) != NULL );
    ok( TOML_isType( TOML_find( table, "world", NULL ), TOML_BOOLEAN ) );
    ok( TOML_find( table, "moon", NULL ) != NULL );
    ok( TOML_isType( TOML_find( table, "moon", NULL ), TOML_BOOLEAN ) );
    TOML_free( table );
  }

  { /** parse_entry_date **/
    note( "parse_entry_date" );
    TOMLTable *table = NULL;
    TOML_parse( "start = 2013-12-20T14:30:00Z", &table, NULL );
    ok( table != NULL );
    ok( TOML_find( table, "start", NULL ) != NULL );
    TOML_free( table );
  }

  { /** parse_entry_array_numbers **/
    note( "parse_entry_array_numbers" );
    TOMLTable *table = NULL;
    TOML_parse( "world = [ 1, 2, 3 ]\n", &table, NULL );
    ok( table != NULL );
    ok( TOMLTable_getKey( table, "world" ) != NULL );
    ok( TOML_toInt(
      TOMLArray_getIndex( TOMLTable_getKey( table, "world" ), 1 )
    ) == 2 );
    TOML_free( table );
  }

  { /** parse_entry_array_strings **/
    note( "parse_entry_array_strings" );
    TOMLTable *table = NULL;
    TOML_parse( "world = [ \"abc\" ]\n", &table, NULL );
    ok( table != NULL );
    ok( TOMLTable_getKey( table, "world" ) != NULL );
    char *buffer = malloc( 256 );
    TOML_copyString(
      TOMLArray_getIndex( TOMLTable_getKey( table, "world" ), 0 ), 256, buffer
    );
    is( buffer, "abc" );
    free( buffer );
    TOML_free( table );
  }

  { /** parse_table **/
    note( "parse_table" );
    TOMLTable *table = NULL;
    TOML_parse( "[world]\nplanet = \"pluto\"", &table, NULL );
    ok( table != NULL );
    ok( TOML_find( table, "world", "planet", NULL ) != NULL );
    TOML_free( table );
  }

  { /** parse_array_table **/
    note( "parse_array_table" );
    TOMLTable *table = NULL;
    TOML_parse( "[[world]]\nplanet = \"jupiter\"", &table, NULL );
    ok( table != NULL );
    TOMLArray *world = TOMLTable_getKey( table, "world" );
    ok( world != NULL );
    TOMLTable *index0 = TOMLArray_getIndex( world, 0 );
    ok( index0 != NULL );
    TOMLRef planet = TOMLTable_getKey( index0, "planet" );
    ok( planet != NULL );
    ok( TOML_find( table, "world", "0", "planet", NULL ) != NULL );
    TOML_free( table );
  }

  { /** parse_array_table_2 **/
    note( "parse_array_table_2" );
    TOMLTable *table = NULL;
    TOML_parse(
      "[[world]]\nplanet = \"jupiter\"\n[[world]]\nplanet = \"saturn\"",
      &table,
      NULL
    );
    ok( table != NULL );
    is(
      ((TOMLString *) TOML_find( table, "world", "1", "planet", NULL ))->content,
      "saturn"
    );
    TOML_free( table );
  }

  { /** parse_entry_array **/
    note( "parse_entry_array" );
    TOMLTable *table = NULL;
    TOML_parse(
      "planets = [ \"mercury\", \"venus\", \"earth\" ]",
      &table,
      NULL
    );
    ok( table != NULL );
    ok( ((TOMLArray *) TOMLTable_getKey( table, "planets" ) )->size == 3 );
    TOML_free( table );
  }

  note( "\n** errors **" );

  { /** parse_incomplete_string **/
    note( "parse_incomplete_string" );
    TOMLTable *table = NULL;
    TOMLError *error = TOML_allocError( TOML_SUCCESS );
    ok( TOML_parse(
      "world = \"planet\"\nmoon = \nothermoon = \"daedulus",
      &table,
      error
    ) != 0 );
    ok( table == NULL );
    ok( error->code != 0 );
    TOML_free( error );
  }

  { /** parse_incomplete_table_header **/
    note( "parse_incomplete_table_header" );
    TOMLTable *table = NULL;
    TOMLError *error = TOML_allocError( TOML_SUCCESS );
    ok( TOML_parse( "[world]]\nhello = \"world\"", &table, error ) != 0 );
    ok( table == NULL );
    ok( error->code != 0 );
    TOML_free( error );
  }

  { /** parse_repeated_table_header **/
    note( "parse_repeated_table_header" );
    TOMLTable *table = NULL;
    TOMLError *error = TOML_allocError( TOML_SUCCESS );
    ok( TOML_parse( "[world]\n[world]", &table, error ) != 0 );
    ok( table == NULL );
    ok( error->code != 0 );
    TOML_free( error );
  }

  { /** parse_repeated_entry **/
    note( "parse_repeated_entry" );
    TOMLTable *table = NULL;
    TOMLError *error = TOML_allocError( TOML_SUCCESS );
    ok( TOML_parse( "planet = 1\nplanet = 2", &table, error ) != 0 );
    ok( table == NULL );
    ok( error->code != 0 );
    TOML_free( error );
  }

  note( "\n** stringify **" );

  { /** stringify_string **/
    note( "stringify_string" );
    TOMLTable *table = NULL;
    TOML_parse( "word = \"some words\"", &table, NULL );
    char *buffer;
    TOML_stringify( &buffer, TOMLTable_getKey( table, "word" ), NULL );
    is( buffer, "some words" );
    free( buffer );
    TOML_free( table );

    // escapes
    table = NULL;
    TOML_parse( "escapes = \"\\b\\t\\f\\n\\r\\\"\\/\\\\\"", &table, NULL );
    ok( table != NULL );
    ok( TOML_find( table, "escapes", NULL ) != NULL );
    buffer = NULL;
    TOML_stringify( &buffer, table, NULL );
    is( buffer, "escapes = \"\\b\\t\\f\\n\\r\\\"\\/\\\\\"\n" );
    free( buffer );
    TOML_free( table );
  }

  { /** stringify_boolean **/
    note( "stringify_boolean" );
    TOMLTable *table = NULL;
    TOML_parse( "world = true\nmoon = false", &table, NULL );
    char *buffer;
    TOML_stringify( &buffer, table, NULL );
    is( buffer, "world = true\nmoon = false\n" );
    free( buffer );
    TOML_free( table );
  }

  { /** stringify_date **/
    note( "stringify_date" );
    TOMLTable *table = NULL;
    TOML_parse( "date = 2013-12-20T14:30:00Z", &table, NULL );
    char *buffer;
    TOML_stringify( &buffer, table, NULL );
    is( buffer, "date = 2013-12-20T14:30:00Z\n" );
    free( buffer );
    TOML_free( table );
  }

  { /** stringify_table_simple **/
    note( "stringify_table_simple" );
    TOMLTable *table = NULL;
    TOML_parse( "word = \"some \\\"words\\\"\"", &table, NULL );
    char *buffer;
    TOML_stringify( &buffer, table, NULL );
    is( buffer, "word = \"some \\\"words\\\"\"\n" );
    free( buffer );
    TOML_free( table );
  }

  { /** stringify_table_child **/
    note( "stringify_table_child" );
    TOMLTable *table = NULL;
    TOML_parse( "[table]\nchairs = 4", &table, NULL );
    char *buffer;
    TOML_stringify( &buffer, table, NULL );
    is( buffer, "[table]\nchairs = 4\n" );
    free( buffer );
    TOML_free( table );
  }

  { /** stringify_arrays **/
    note( "stringify_arrays" );
    TOMLTable *table = NULL;
    TOML_parse(
      "[[planets]]\nmoons = [ \"io\" ]\n[[planets]]\n[[planets]]\nmoons = []", &table, NULL
    );
    char *buffer;
    TOML_stringify( &buffer, table, NULL );
    is(
      buffer,
      "[[planets]]\nmoons = [ \"io\" ]\n\n[[planets]]\n\n[[planets]]\nmoons = []\n"
    );
    free( buffer );
    TOML_free( table );
  }

  { /** stringify_nested_tables **/
    note( "stringify_nested_tables" );
    TOMLTable *table = NULL;
    TOML_parse(
      "[top.bottom]\nmiddle = \"no\"\n"
        "[top.middle.bottom]\nmiddle = \"infact, yes\"",
      &table,
      NULL
    );
    ok( TOML_find( table, "top", "middle", "bottom", "middle", NULL ) != NULL );
    char *buffer;
    TOML_stringify( &buffer, table, NULL );
    is(
      buffer,
      "[top.bottom]\nmiddle = \"no\"\n\n"
        "[top.middle.bottom]\nmiddle = \"infact, yes\"\n"
    );
    free( buffer );
    TOML_free( table );
  }

  note( "** unicode **" );

  { /** parse_utf8 **/
    note( "parse_utf8" );
    TOMLTable *table = NULL;
    TOML_parse( "utf8 = \"\\u00a0\\u07ff\\u0800\\uffff\"", &table, NULL );
    ok( table != NULL );
    TOMLString *str = TOML_find( table, "utf8", NULL );
    is( str->content, "\u00a0\u07ff\u0800\uffff" );
    TOML_free( table );
  }

  { /** stringify_utf8 **/
    note( "stringify_utf8" );
    TOMLTable *table = NULL;
    TOML_parse( "utf8 = \"\\u00a0\\u07ff\\u0800\\uffff\"", &table, NULL );
    ok( table != NULL );
    char *buffer;
    TOML_stringify( &buffer, table, NULL );
    is( buffer, "utf8 = \"\\u00a0\\u07ff\\u0800\\uffff\"\n" );
    free( buffer );
    TOML_free( table );
  }

  done_testing();
}
