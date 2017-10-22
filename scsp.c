// Implemented by Vitaly "_Vi" Shukela in 2017; License = MIT or Apache 2.0

#include "scsp.h"

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

#if SCSP_ENABLE_HELPERS
    #include <unistd.h>
    #include <string.h>
    #include <errno.h>
#endif


#ifdef SCSP_ENABLE_FLOAT
    // snippet from RFC 7049
   #include <math.h>

   static double decode_half(const unsigned char *halfp) {
     int half = (halfp[0] << 8) + halfp[1];
     int exp = (half >> 10) & 0x1f;
     int mant = half & 0x3ff;
     double val;
     if (exp == 0) val = ldexp(mant, -24);
     else if (exp != 31) val = ldexp(mant + 1024, exp - 25);
     else val = mant == 0 ? INFINITY : NAN;
     return half & 0x8000 ? -val : val;
   }
#endif


#define CALLBACK(name, par...) \
    SCSP_DEBUG("callback: %s\n", #name); \
    if (callbacks->name) { \
        if ((*callbacks->name)(userdata, par) < 0) { \
            return -1; \
        } \
    }
#define CALLBACK0(name) \
    SCSP_DEBUG("callback: %s\n", #name); \
    if (callbacks->name) { \
        if ((*callbacks->name)(userdata) < 0) { \
            return -1; \
        } \
    }
SCSP_INT static scsp_closeelements(
                struct scsp_state* state, 
                struct scsp_callbacks* callbacks,
                SCSP_USERDATA userdata);


SCSP_INT scsp_parse_lowlevel(
            struct scsp_state* state, 
            struct scsp_callbacks* callbacks,
            SCSP_USERDATA userdata,
            const void* buf,
            size_t count)
{    
    if ( ((SCSP_INT)-1) > 0 ) {
        SCSP_DEBUG("SCSP_INT should be signed\n");
        return -1;
    }
        
    struct scsp_stack_entry* se = &state->stack[state->cur_depth];
    struct scsp_stack_entry* se1 = NULL;
    if (state->cur_depth > 0) {
        se1 = &state->stack[state->cur_depth - 1];
        SCSP_DEBUG("se1->type=%c se1->remaining=%ld\n", se1->type, se1->remaining);
    }
    
    if (count < 1) return 0;
    
    SCSP_INT ret = -2;
    
    const uint8_t*b = (const uint8_t*)buf;
    
    if (se1 && (se1->type == 's' || se1->type == 'b')) {
        if (se1->remaining <= 0) {
            SCSP_DEBUG("internal error: se1->remaining low\n");
            return -1;
        }
        size_t tobepr = se1->remaining;
        if (count < tobepr) tobepr=count;
        
        if (se1->type == 's') {
            CALLBACK(string_chunk, b, tobepr);
        } else {
            CALLBACK(bytestring_chunk, b, tobepr);
        }
        
        se1->remaining -= tobepr;
        
        if (se1->remaining == 0) {
            state->cur_depth -= 1;
            
            if (state->cur_depth > 0) {
                
                struct scsp_stack_entry* se2 = &state->stack[state->cur_depth - 1];
            
                if (se2->type == 'B' || se2->type == 'S') {
                    // inhibit close events in this case
                } else {
                    if (se1->type == 's') {
                        CALLBACK0(string_close);
                    } else {
                        CALLBACK0(bytestring_close);
                    }
                }
                
                if (-1 == scsp_closeelements(state, callbacks, userdata)) {
                    return -1;
                }
            } else {
                if (se1->type == 's') {
                    CALLBACK0(string_close);
                } else {
                    CALLBACK0(bytestring_close);
                }
            }
        }
        
        return tobepr;
    }
    
    uint8_t maj = (b[0] & 0xE0) >> 5;
    uint8_t add = (b[0] & 0x1F) >> 0;
    
    SCSP_DEBUG("maj=%d add=%d depth=%ld\n", maj, add, state->cur_depth);
    
    switch(maj) {
        case 0:
            se->type = '1';
            break;
        case 1:
            se->type = '-';
            break;
        case 2:
            se->type = 'b';
            break;
        case 3:
            se->type = 's';
            break;
        case 4:
            se->type = '[';
            break;
        case 5:
            se->type = '{';
            break;
        case 6:
            se->type = '_';
            break;
        case 7:
            se->type = '.';
            break;
    }
    
    if (add == 31) {
        switch (se->type) {
            case 'b': se->type = 'B'; break;
            case 's': se->type = 'S'; break;
            case '[': se->type = ']'; break;
            case '{': se->type = '}'; break;
            case '.': se->type = '!'; break;
            default: {
                SCSP_DEBUG("error: add=31 for the wrong thing\n");
                return -1;
            }
        }
    }
    else if (add == 24) { if ( count < 1+1) return 0; }
    else if (add == 25) { if ( count < 1+2) return 0; }
    else if (add == 26) { if ( count < 1+4) return 0; }
    else if (add == 27) { if ( count < 1+8) return 0; }
    else if (add < 24) { }
    else {
        SCSP_DEBUG("error: 'add' between 28 and 30 is not defined\n");
        return -1;
    }
    
    char t = se->type;
    char push_to_stack = '?';
    
    if (se1 && t != '!' && t != '_') {
        if (se1->type == '['  || se1->type == ']') {
            CALLBACK0(array_item);
        } else
        if (se1->type == '{'  || se1->type == '}') {
            if (se1->remaining == 0) {
                SCSP_DEBUG("internal error   rem=0?\n");
                return -1;
            } else
            if (se1->remaining > 0) {
                if ( (se1->remaining & 1) == 0) {
                    CALLBACK0(map_key);
                } else {
                    CALLBACK0(map_value);
                }
            } else {
                if (se1->remaining == -1) {
                    CALLBACK0(map_key);
                } else {
                    CALLBACK0(map_value);
                }
            }
        }
    }
   
    if (t == 'B') {
        CALLBACK(bytestring_open, -1);
        ret = 1;
        push_to_stack = 'Y';
        se->remaining = -1;
    } else
    if (t == 'S') {
        CALLBACK(string_open, -1);
        ret = 1;
        push_to_stack = 'Y';
        se->remaining = -1;
    } else
    if (t == ']') {
        CALLBACK(array_opened, -1);
        ret = 1;
        push_to_stack = 'Y';
        se->remaining = -1;
    } else
    if (t == '}') {
        CALLBACK(map_opened, -1);
        ret = 1;
        push_to_stack = 'Y';
        se->remaining = -1;
    } else
    if (t == '!') {
        ret = 1;
        push_to_stack = 'N';
    } else {
        SCSP_INT number;
        
        if (   (add == 25 && sizeof(SCSP_INT) < 2) ||
               (add == 26 && sizeof(SCSP_INT) < 4) ||
               (add == 27 && sizeof(SCSP_INT) < 8)) {
            SCSP_DEBUG("length overflow\n");
            return -1;
        }
        
        if (add<24) {
            number = add;
            ret = 1;
        } else
        if (add==24) {
            number = b[1];
            ret = 1+1;
        } else
        if (add==25) {
            if ((b[1]&0x80) && sizeof(SCSP_INT)<=2) {
                SCSP_DEBUG("number overflow 16-bit\n"); 
                return -1; 
            }
            number = (b[1] << 8) | b[2];
            ret = 1+2;
        } else
        if (add==26) {
#if SCSP_ENABLE_32BIT
            if ((b[1]&0x80) && sizeof(SCSP_INT)<=4) {
                SCSP_DEBUG("number overflow 32-bit\n"); 
                return -1; 
            }
            number = (((b[1] << 8) | b[2]) << 16)   |   ((b[3] << 8) | b[4]);
            ret = 1+4;
#else
            SCSP_DEBUG("32-bit numbers are not enabled\n");
            return -1;
#endif
        } else
        if (add==27) {
#if SCSP_ENABLE_64BIT
            if ((b[1]&0x80) && sizeof(SCSP_INT)<=8) {
                SCSP_DEBUG("number overflow 64-bit\n"); 
                return -1; 
            }
            number =  0;
            
            unsigned char i;
            for (i=1; i<9; ++i) {
                number <<= 8;
                number |= b[i];
            }
            
            ret = 1+8;
#else
            SCSP_DEBUG("64-bit numbers not enabled\n");
            return -1;
#endif
        } else {
            SCSP_DEBUG("internal error 3\n");
            return -1;
        }
    
        switch(t) {
            case '1':
                CALLBACK(integer, number);
                push_to_stack = 'N';
                break;
            case '-':
                number = -1 - number;
                CALLBACK(integer, number);
                push_to_stack = 'N';
                break;
            case 'b': {
                char inhibit_callback = se1 && se1->type == 'B';
                if (!inhibit_callback) {
                    CALLBACK(bytestring_open, number);
                }
                if (number == 0) {
                    push_to_stack = 'N';
                    if (!inhibit_callback) {
                        CALLBACK0(bytestring_close);
                    }
                } else {
                    push_to_stack = 'Y';
                    se->remaining = number;
                }
                break;
            }
            case 's': {
                char inhibit_callback = se1 && se1->type == 'S';
                if (!inhibit_callback) {
                    CALLBACK(string_open, number);
                }
                if (number == 0) {
                    push_to_stack = 'N';
                    if (!inhibit_callback) {
                        CALLBACK0(string_close);
                    }
                } else {
                    push_to_stack = 'Y';
                    se->remaining = number;
                }
                break;
            }
            case '[':
                se->remaining = number;
                CALLBACK(array_opened, number);
                if (number == 0) {
                    CALLBACK0(array_closed);
                    push_to_stack = 'N';
                } else {
                    push_to_stack = 'Y';
                }
                break;
            case '{':
                se->remaining = number;
                CALLBACK(map_opened, number);
                if (number == 0) {
                    CALLBACK0(map_closed);
                    push_to_stack = 'N';
                } else {
                    push_to_stack = 'Y';
                }
                break;
            case '_':
                // no tag support, just skipping them
                push_to_stack = 'N';
                break;
            case '.':
                switch (add) {
                    case 20: CALLBACK(simple, 'F'); break;
                    case 21: CALLBACK(simple, 'T'); break;
                    case 22: CALLBACK(simple, 'N'); break;
                    case 23: CALLBACK(simple, 'U'); break;
                    case 24: {
                        CALLBACK(simple_other, number); break;
                    }
#if SCSP_ENABLE_FLOAT
                    case 25: {
                        double x = decode_half(b+1);
                        CALLBACK(noninteger, x);
                        break;
                    }
                    case 26: {
                        uint32_t y = (b[1] << 24) | (b[2] << 16) | (b[3] << 8) | b[4];
                        double x = *(const float*)(&y);
                        CALLBACK(noninteger, x);
                        break;
                    }
                    case 27: {
#if SCSP_ENABLE_64BIT
                        unsigned char i;
                        uint64_t y = 0;
                        for (i=1; i<9; ++i) {
                            y <<= 8;
                            y |= b[i];
                        }
                        
                        double x = *(const double*)(&y);
                        CALLBACK(noninteger, x);
                        break;
#else
                        SCSP_DEBUG("64-bit things, including floats, are disabled in conf\n");
                        return -1;
#endif
                    }
#else // ! SCSP_ENABLE_FLOAT
                    case 25:
                    case 26:
                    case 27:
                        SCSP_DEBUG("float not supported\n");
                        return -1;
#endif // SCSP_ENABLE_FLOAT
                    case 28:
                    case 29:
                    case 30: {
                        SCSP_DEBUG("invalid misc value\n");
                        return -1;
                    }
                    case 31: {
                        SCSP_DEBUG("internal err\n");
                        return -1;
                    }
                    default: CALLBACK(simple_other, number);
                }
                push_to_stack = 'N';
                break;
            default:
                SCSP_DEBUG("internal error: t=%c ?\n", t);
                return -1;
        }
    }
    
    if (push_to_stack == 'N') {
        
        if (t == '!') {
            if (state->cur_depth == 0) {
                SCSP_DEBUG("top-level break\n");
                return -1;
            }
            struct scsp_stack_entry* se1 = &state->stack[state->cur_depth- 1];
            if (se1->remaining != -1) {
                SCSP_DEBUG("break in wrong place\n");
                return -1;
            }
            se1->remaining = 1;
        }
        
        if (t != '_') {
            if (-1 == scsp_closeelements(state, callbacks, userdata)) {
                return -1;
            }
        }
    } else 
    if (push_to_stack == 'Y') {
        
        if (t == '{') {
            if ( (se->remaining * 2) < se->remaining ) {
                SCSP_DEBUG("length overflow 3\n");
                return -1;
            }
            se->remaining *= 2;
        }
        
        SCSP_DEBUG("pushing to stack type %c with remaining %d\n", se->type, (int)se->remaining);
        
        if (state->cur_depth + 1 >= SCSP_MAXDEPTH) {
            SCSP_DEBUG("depth overflow\n");
            return -1;
        }
        if (se->remaining == 0) {
            SCSP_DEBUG("internal error: se->remaining == 0 v2\n");
            return -1;
        }
        
        state->cur_depth += 1;
        
        
    } else {
        SCSP_DEBUG("internal error: push_to_stack=%c\n", push_to_stack);
        return -1;
    }
    
    return ret;
}

SCSP_INT static scsp_closeelements(
                struct scsp_state* state, 
                struct scsp_callbacks* callbacks,
                SCSP_USERDATA userdata) {
    while (state->cur_depth > 0) {
        struct scsp_stack_entry* se1 = &state->stack[state->cur_depth- 1];
        if (se1->remaining >= 0) {
            if (se1->remaining == 0) {
                SCSP_DEBUG("internal error: remaining=0?\n");
                return -1;
            }
            se1->remaining -= 1;
            if (se1->remaining == 0) {
                switch(se1->type) {
                    case ']': 
                    case '[':
                         CALLBACK0(array_closed);
                         break;
                    case '{': 
                    case '}':
                        CALLBACK0(map_closed);
                        break;
                    case 'B':
                        CALLBACK0(bytestring_close);
                        break;
                    case 'S':
                        CALLBACK0(string_close);
                        break;
                    case 'b':
                    case 's':
                        SCSP_DEBUG("interal error 5\n");
                        return -1;
                    default: {
                        SCSP_DEBUG("internal error: se->type=%c ?\n", se1->type);
                        return -1;
                    }
                }
                state->cur_depth -= 1;
                continue;
            }
        } else {
            if (se1->type == '}') {
                if (se1->remaining == -1) {
                    se1->remaining = -2;
                } else {
                    se1->remaining = -1;
                }
            }
        }
        break;
    }
    return 0;
}

#if SCSP_ENABLE_HELPERS
SCSP_INT SCSP_EXPORT scsp_parse_from_fd(
            int fd, 
            struct scsp_callbacks* callbacks,
            SCSP_USERDATA userdata)
{
    struct scsp_state     ss;
    ss.cur_depth = 0;
    uint8_t buf[8092];
    
    size_t read_cursor = 0;
    for(;;) {
        size_t len = read(fd, buf+read_cursor, (sizeof buf) - read_cursor);
        if (len == -1) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) { usleep(100000); continue; }
            return -1;
        }
        if (len == 0) break;
        
        len += read_cursor;
        read_cursor=0;
        
        size_t write_cursor = 0;
        while(write_cursor < len) {
            int ret2 = scsp_parse_lowlevel(&ss, callbacks, userdata, buf+write_cursor, len-write_cursor);
            if (ret2 == -1) {
                return -1;
            }
            if (ret2 == 0) {
                memmove(buf, buf+write_cursor, len-write_cursor);
                read_cursor = len-write_cursor;
                break;
            }
            write_cursor += ret2;
        }
        //fflush(stdout);
    }
    return read_cursor;
}

#endif // SCSP_ENABLE_HELPERS

SCSP_INT SCSP_EXPORT scsp_parse_from_memory(
            const void *buffer,
            size_t count,
            struct scsp_callbacks* callbacks,
            SCSP_USERDATA userdata)
{
    struct scsp_state     ss;
    ss.cur_depth = 0;
    
    const uint8_t *buf = (const uint8_t*)buffer;
    
    size_t write_cursor = 0;
    while(write_cursor < count) {
        int ret2 = scsp_parse_lowlevel(&ss, callbacks, userdata, buf+write_cursor, count-write_cursor);
        if (ret2 == -1) {
            return -1;
        }
        if (ret2 == 0) {
            break;
        }
        write_cursor += ret2;
    }
    
    return count - write_cursor;
}

