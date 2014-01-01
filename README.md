# tomlc

### Dependencies

`libtoml` uses Lemon and re2c to generate its parser and lexer respectively.

- Lemon : http://www.hwaci.com/sw/lemon/
- re2c : http://re2c.org/

### Build Instructions

1. `./waf configure build install`

### Usage

```c
// TOML_parse and TOML_load create the root table for you.
TOMLTable *table = NULL;

// Load TOML from a file.
TOML_load( "test.toml", &table, NULL );

// Find the value we want.
TOMLRef value = TOML_find( table, "player", "size", NULL );
int size = TOML_int( value );

// Free the TOML hierarchy when you're done.
TOML_free( table );
```
