#include <math.h>

#include "tap.h"
#include "toml.h"

int main() {
  plan(26);

  note( "memory management" );

  { /** alloc_number **/
    note( "alloc_number" );
    TOMLNumber *number = TOML_anInt( 1 );
    ok( TOML_toInt( number ) == 1, "number is 1" );
    TOML_free( number );
  }

  { /** alloc_string **/
    note( "alloc_string" );
    TOMLString *string = TOML_aString( "Why, yes." );
    char string_buffer[256];
    TOML_copyString( string, 256, string_buffer );
    is( string_buffer, "Why, yes.", "string is correct" );
    TOML_free( string );
  }

  { /** alloc_array **/
    note( "alloc_array" );
    TOMLArray *array = TOML_anArray( TOML_NUMBER,
      TOML_anInt( 2 ),
      TOML_aDouble( 3.5 ),
      NULL
    );
    ok( TOML_toInt( TOMLArray_getIndex( array, 0 ) ) == 2, "array[0] is 2" );
    ok( TOML_toDouble( TOMLArray_getIndex( array, 1 ) ) == 3.5,
      "array[1] is 3.5"
    );
    TOML_free( array );
  }

  { /** alloc_table **/
    note( "alloc_table" );
    TOMLTable *table = TOML_aTable(
      TOML_aString( "key" ), TOML_aString( "value" ),
      NULL, NULL
    );
    TOMLString *table_value = TOMLTable_getKey( table, "key" );
    char value_buffer[256];
    TOML_copyString( table_value, 256, value_buffer );
    is( value_buffer, "value", "table \"key\" is \"value\"" );
    TOML_free( table );
  }

  { /** parse_entry_string **/
    note( "parse_entry_string" );
    TOMLTable *table = NULL;
    TOML_parse( "world = \"hello\"\n", &table );
    ok( table != NULL );
    TOMLRef world = TOMLTable_getKey( table, "world" );
    char *buffer = malloc( 256 );
    TOML_copyString( world, 256, buffer );
    is( buffer, "hello" );
    free( buffer );
    TOML_free( table );
  }

  { /** parse_entry_int **/
    note( "parse_entry_int" );
    TOMLTable *table = NULL;
    TOML_parse( "world = 123\n", &table );
    ok( table != NULL );
    ok( TOMLTable_getKey( table, "world" ) != NULL );
    ok( TOML_toInt( TOMLTable_getKey( table, "world" ) ) == 123 );
    TOML_free( table );
  }

  { /** parse_entry_double **/
    note( "parse_entry_double" );
    TOMLTable *table = NULL;
    TOML_parse( "world = 1.23\n", &table );
    ok( table != NULL );
    ok( TOMLTable_getKey( table, "world" ) != NULL );
    ok( TOML_toDouble( TOMLTable_getKey( table, "world" ) ) == 1.23 );
    TOML_free( table );
  }

  { /** parse_entry_array_numbers **/
    note( "parse_entry_array_numbers" );
    TOMLTable *table = NULL;
    TOML_parse( "world = [ 1, 2, 3 ]\n", &table );
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
    TOML_parse( "world = [ \"abc\" ]\n", &table );
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
    TOML_parse( "[world]\nplanet = \"pluto\"", &table );
    ok( table != NULL );
    ok( TOML_find( table, "world", "planet", NULL ) != NULL );
    TOML_free( table );
  }

  { /** parse_array_table **/
    note( "parse_array_table" );
    TOMLTable *table = NULL;
    TOML_parse( "[[world]]\nplanet = \"jupiter\"", &table );
    ok( table != NULL );
    TOMLArray *world = TOMLTable_getKey( table, "world" );
    ok( world != NULL );
    TOMLTable *index0 = TOMLArray_getIndex( world, 0 );
    ok( index0 != NULL );
    TOMLRef planet = TOMLTable_getKey( index0, "planet" );
    ok( planet != NULL );
    ok( TOML_find( table, "world", 0, "planet", NULL ) != NULL );
  }

  done_testing();
}
