# tomlc [![Build Status](https://travis-ci.org/mzgoddard/tomlc.png?branch=master)](https://travis-ci.org/mzgoddard/tomlc)

This implementation targets [https://github.com/mojombo/toml](https://github.com/mojombo/toml) commit [4f23be43e4](https://github.com/mojombo/toml/commit/4f23be43e42775493f142e7dd025b6227e037dd9).

Out of order tables are not yet supported:

```toml
[a.b]
value = 1

[a]
value = 2
```

### License

MIT

### Dependencies

`tomlc` uses Lemon and re2c to generate its parser and lexer respectively.

- Lemon : http://www.hwaci.com/sw/lemon/
- re2c : http://re2c.org/

### Build Instructions

1. Install `re2c`.
1. `./waf configure build install`  
  By default, this installs as `toml`.

1. Test with `build/toml-test`

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

Part of `tomlc` is a tool called `toml-lookup` that can be used to access parts of a toml file. For example `toml-lookup test.toml "en.text[0].characterImage"` prints `text-only` to stdout.
