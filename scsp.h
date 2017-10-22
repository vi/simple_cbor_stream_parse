// A simple low-level streamed callback-based CBOR push parser in C.

// Implemented by Vitaly "_Vi" Shukela in 2017; License = MIT or Apache 2.0

#ifndef _SCSP_H
#define _SCSP_H

#include <stdint.h>
#include <stddef.h>


#ifdef SCSP_DEBUG_STDERR
    #define SCSP_DEBUG(x...) fprintf(stderr, x)
    #include <stdio.h>
#endif

#ifndef SCSP_INT
#define SCSP_INT int64_t
#endif


#ifndef SCSP_USERDATA
#define SCSP_USERDATA void*
#endif

#ifndef SCSP_MAXDEPTH
#define SCSP_MAXDEPTH 16
#endif

#ifndef SCSP_DEBUG
#define SCSP_DEBUG(x...)
#endif

#ifndef SCSP_EXPORT
#define SCSP_EXPORT
#endif

#ifndef SCSP_ENABLE_32BIT
#define SCSP_ENABLE_32BIT 1
#endif

#ifndef SCSP_ENABLE_64BIT
#define SCSP_ENABLE_64BIT 1
#endif

#ifndef SCSP_ENABLE_FLOAT
#define SCSP_ENABLE_FLOAT 1
#endif

/* 
types:

'1' - integer
'-' - negative integer
'b' - byte string
'B' - byte string streamed
's' - UTF-8 string
'S' - UTF-8 string streamed
'[' - array
']' - array streamed
'{' - map
'}' - map streamed
'_' - tag
'.' - others
'!' - break

*/

struct scsp_stack_entry {
    char type;
    SCSP_INT remaining;
};

struct scsp_state {
    SCSP_INT cur_depth;
    struct scsp_stack_entry stack[SCSP_MAXDEPTH];
};

struct scsp_callbacks {
    SCSP_INT (*integer) (SCSP_USERDATA userdata, SCSP_INT value);
    
    SCSP_INT (*bytestring_open) (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one);
    SCSP_INT (*bytestring_chunk) (SCSP_USERDATA userdata, uint8_t* buf, size_t len);
    SCSP_INT (*bytestring_close) (SCSP_USERDATA userdata);
    
    SCSP_INT (*string_open)  (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one);
    SCSP_INT (*string_chunk) (SCSP_USERDATA userdata, uint8_t* buf, size_t len);
    SCSP_INT (*string_close) (SCSP_USERDATA userdata);
    
    SCSP_INT (*array_opened) (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one);
    SCSP_INT (*array_item)   (SCSP_USERDATA userdata);
    SCSP_INT (*array_closed) (SCSP_USERDATA userdata);
    
    SCSP_INT (*map_opened) (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one);
    SCSP_INT (*map_key) (SCSP_USERDATA userdata);
    SCSP_INT (*map_value) (SCSP_USERDATA userdata);
    SCSP_INT (*map_closed) (SCSP_USERDATA userdata);
    
    // 'T' - true, 'F' - false, 'N' - null, 'U' - undefined
    SCSP_INT (*simple) (SCSP_USERDATA userdata, char value);
    SCSP_INT (*simple_other) (SCSP_USERDATA userdata, SCSP_INT value);
#if SCSP_ENABLE_FLOAT
    SCSP_INT (*noninteger) (SCSP_USERDATA userdata, double value);
#endif
};

/*
 * Returns 0 if there is too little data to be parsed. Expects buffering to be done by caller.
 * Returns -1 on error, including on error returned from callbacks.
 * Does not set errno.
 */
SCSP_INT SCSP_EXPORT scsp_parse(
            struct scsp_state* state, 
            struct scsp_callbacks* callbacks,
            SCSP_USERDATA userdata,
            const void* buf,
            size_t count);

#endif // _SCSP_H

