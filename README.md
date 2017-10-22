scsp - simple_cbor_stream_parse
----

A simple low-level streamed callback-based [CBOR](https://cbor.io/) push parser in C.

License = MIT or Apache 2.0

* No memory allocations, embedded-friendly.
* Should work with 32-bit or 16-bit numbers. Float support is also opt-out.
* No buffering at all. You are in charge for buffering. Parse function expects [write(2)](http://man7.org/linux/man-pages/man2/write.2.html) semantics and process less bytes than you have pushed into it (including 0 bytes if there is not enough data).

Concerns:

* Nesting depth is statically limited
* Unsigned numbers are represented as signed => can't handle access some extreme values
* Not validating. Bytestrings and strings are handled similarly.
* Strings and bytestrings are delivered in chunks which depend on how you push the data to the parser. String chunks may split UTF-8 characters in pieces.
