scsp - simple_cbor_stream_parse
----

A simple low-level streamed callback-based [CBOR](https://cbor.io/) push parser in C.

License = MIT or Apache 2.0

* No memory allocations, embedded-friendly.
* Should work with 32-bit or 16-bit numbers. Float support is also opt-out.
* No buffering at all. You are in charge for buffering. Parse function expects [write(2)](http://man7.org/linux/man-pages/man2/write.2.html) semantics and process less bytes than you have pushed into it (including 0 bytes if there is not enough data).

Concerns/Notes:

* Nesting depth is statically limited
* Unsigned numbers are represented as signed => can't handle access some extreme values
* Not validating. Bytestrings and strings are handled similarly.
* Strings and bytestrings are delivered in chunks which depend on how you push the data to the parser. String chunks may split UTF-8 characters in pieces.
* There is no CBOR generator here, only parser
* Tags are ignored
* Not entire "Appenfix A" testsuite passes due to forced signed numbers

Examples:

* cbor_to_jsonesque - read cbor from a file or stdin and dump something similar to JSON or CBOR diagnostic.
* dump_scsp_events - dump each event in a separate line
* simple_example - parse from memory and assert it works

Configurable parameters:

* SCSP_MAXDEPTH - maximum nesting depth of CBOR objects or strings. Should probably be 2-3 more than you plan to do. Defaults to 16.
* SCSP_DEBUG, SCSP_DEBUG_STDERR - enable debugging output
* SCSP_USERDATA - data passed to each callback. Defaults to `void*`
* SCSP_EXPORT - just resides in signature of user-facing functions
* SCSP_INT - primary data type for integers, lengths and return values. Defaults to int64_t.
* SCSP_ENABLE_HELPERS - Enable some helper functions.
* SCSP_ENABLE_FLOAT
* SCSP_ENABLE_32BIT
* SCSP_ENABLE_64BIT
