# scsp - simple_cbor_stream_parse

A simple low-level streamed callback-based [CBOR](https://cbor.io/) push parser in C.

License = MIT or Apache 2.0

## Features

* No memory allocations, embedded-friendly.
* Should work with 32-bit or 16-bit registers. Float support is also opt-out.
* No buffering at all in low-level mode. You are in charge for buffering. Parse function expects [write(2)](http://man7.org/linux/man-pages/man2/write.2.html) semantics and process less bytes than you have pushed into it (including 0 bytes if there is not enough data).
* Helper function to parse from a FD (with buffering) or from memory.
* There is C++ binding. istream version has unavoidable buffering, low-level version requires allocation.

## Concerns/Notes:

* Nesting depth is statically limited
* Unsigned numbers are represented as signed => can't handle access some extreme values
* Not validating. Bytestrings and strings are handled similarly.
* Strings and bytestrings are delivered in chunks which depend on how you push the data to the parser. String chunks may split UTF-8 characters in pieces.
* There is no CBOR generator here, only parser
* Not entire "Appenfix A" testsuite passes due to forced signed numbers

## How to use

### C

1. Copy source files (`scsp.c` and `scsp.h`) into your project
2. Define your callback functions (copy from some example). Note that strings may arrive in arbitrary chunks.
3. Fill in `scsp_callbacks` structure (copy from some example)
4. If you need to parse from entire file or from memory buffer, use `scsp_parse_from_fd` or `scsp_parse_from_memory`.
5. If you need flexible parsing, create `scsp_state` structure, initialize it with zeroes and call `scsp_parse_lowlevel` in a loop. It will consume data bit by bit. One call corresponds to a few callbacks.

### C++

1. Copy source files `scsp.c`, `scsp_cpp.cpp`, `scsp.h` and `scsp_cpp.hpp` into your project
2. Inherit from `scsp::Callbacks` interface or `scsp::CallbacksEmpty` class
3. Override events you need to listen to
4. For simple mode, use `parse_from_istream` / `parse_from_memory` / `parse_from_fd` functions. Note that istream version does buffering inside.
5. For flexible mode, create `scsp::State` object with `new_state`, use `parse_lowlevel` (see fifth point in "C" version of "How to use") in a loop, then delete state object with delete_state.

## Examples

* cbor_to_jsonesque - read cbor from a file or stdin and dump something similar to JSON or CBOR diagnostic.
* dump_scsp_events - dump each event in a separate line
* [simple_example](/simple_example.c) - parse from memory and assert it works
* [extract_strings](/extract_strings.cpp) - a C++ example. Prints all encountered strings.


## Configurable parameters


* SCSP_MAXDEPTH - maximum nesting depth of CBOR objects or strings. Should probably be 2-3 more than you plan to do. Defaults to 16.
* SCSP_DEBUG, SCSP_DEBUG_STDERR - enable debugging output
* SCSP_USERDATA - data passed to each callback. Defaults to `void*`
* SCSP_EXPORT - just resides in signature of user-facing functions
* SCSP_INT - primary data type for integers, lengths and return values. Defaults to int64_t.
* SCSP_ENABLE_HELPERS - Enable some helper functions.
* SCSP_ENABLE_FLOAT
* SCSP_ENABLE_32BIT
* SCSP_ENABLE_64BIT

## Example of an scsp event stream

`{"bytes": h'0102030405', 4: -6, true: 23.45, "array": [1, 1, 1]}`

```
00000000  a4 65 62 79 74 65 73 45  01 02 03 04 05 04 25 f5  |.ebytesE......%.|
00000010  fb 40 37 73 33 33 33 33  33 65 61 72 72 61 79 83  |.@7s33333earray.|
00000020  01 01 01                                          |...|
00000023
```

```
map_opened(4)
map_key
string_open(5)
string_chunk("\x62\x79\x74\x65\x73")
string_close
map_value
bytestring_open(5)
bytestring_chunk("\x01\x02\x03\x04\x05")
bytestring_close
map_key
integer(4)
map_value
integer(-6)
map_key
simple(T)
map_value
noninteger(23.45)
map_key
string_open(5)
string_chunk("\x61\x72\x72\x61\x79")
string_close
map_value
array_opened(3)
array_item
integer(1)
array_item
integer(1)
array_item
integer(1)
array_closed
map_closed
```
