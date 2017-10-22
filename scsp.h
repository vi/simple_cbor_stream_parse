#ifndef _SCSP_H
#define _SCSP_H

#include <stdint.h>
#include <stddef.h>

#define SCSP_DEBUG(x...) fprintf(stderr, x)
#include <stdio.h>

#ifndef SCSP_SYSINT
#define SCSP_SYSINT int64_t
#endif

#ifndef SCSP_DATAINT
#define SCSP_DATAINT int64_t
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
    SCSP_SYSINT remaining;
};

struct scsp_state {
    SCSP_SYSINT cur_depth;
    struct scsp_stack_entry stack[SCSP_MAXDEPTH];
};

struct scsp_callbacks {
    SCSP_SYSINT (*integer) (SCSP_USERDATA userdata, SCSP_DATAINT value);
    
    SCSP_SYSINT (*bytestring_open) (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one);
    SCSP_SYSINT (*bytestring_chunk) (SCSP_USERDATA userdata, uint8_t* buf, size_t len);
    SCSP_SYSINT (*bytestring_close) (SCSP_USERDATA userdata);
    
    SCSP_SYSINT (*string_open)  (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one);
    SCSP_SYSINT (*string_chunk) (SCSP_USERDATA userdata, uint8_t* buf, size_t len);
    SCSP_SYSINT (*string_close) (SCSP_USERDATA userdata);
    
    SCSP_SYSINT (*array_opened) (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one);
    SCSP_SYSINT (*array_item)   (SCSP_USERDATA userdata);
    SCSP_SYSINT (*array_closed) (SCSP_USERDATA userdata);
    
    SCSP_SYSINT (*map_opened) (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one);
    SCSP_SYSINT (*map_key) (SCSP_USERDATA userdata);
    SCSP_SYSINT (*map_value) (SCSP_USERDATA userdata);
    SCSP_SYSINT (*map_closed) (SCSP_USERDATA userdata);
    
    SCSP_SYSINT (*simple) (SCSP_USERDATA userdata, char value);
};

/*
 * Returns 0 if there is too little data to be parsed. Expects buffering to be done by caller.
 * Returns -1 on error, including on error returned from callbacks.
 * Does not set errno.
 */
SCSP_SYSINT SCSP_EXPORT scsp_parse(
            struct scsp_state* state, 
            struct scsp_callbacks* callbacks,
            SCSP_USERDATA userdata,
            const void* buf,
            size_t count);

#endif // _SCSP_H

