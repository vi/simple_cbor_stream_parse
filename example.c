#include <stdio.h>

#include <stdint.h>

#define SCSP_DEBUG(x...) fprintf(stderr, x)

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
    SCSP_SYSINT (*array_opened) (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one);
    SCSP_SYSINT (*array_closed) (SCSP_USERDATA userdata);
    SCSP_SYSINT (*integer) (SCSP_USERDATA userdata, SCSP_DATAINT value);
};

/*
 * Returns 0 if there is too little data to be parsed. Expects buffering to be done by caller.
 * Returns -1 on error, including on error returned from callbacks.
 * Does not set errno.
 */
SCSP_SYSINT scsp_parse(
            struct scsp_state* state, 
            struct scsp_callbacks* callbacks,
            SCSP_USERDATA userdata,
            const void* buf,
            size_t count);

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

SCSP_SYSINT scsp_parse(
            struct scsp_state* state, 
            struct scsp_callbacks* callbacks,
            SCSP_USERDATA userdata,
            const void* buf,
            size_t count)
{
    if (sizeof(SCSP_DATAINT) < sizeof(SCSP_SYSINT)) {
        SCSP_DEBUG("SCSP_DATAINT should be not less than SCSP_SYSINT\n");
        return -1;
    }
    
    if ( ((SCSP_SYSINT)-1) > 0 ) {
        SCSP_DEBUG("SCSP_SYSINT should be signed\n");
        return -1;
    }
        
    struct scsp_stack_entry* se = &state->stack[state->cur_depth];
    if (count < 1) return 0;
    
    SCSP_SYSINT ret = -2;
    
    uint8_t*b = (uint8_t*)buf;
    uint8_t maj = (b[0] & 0xE0) >> 5;
    uint8_t add = (b[0] & 0x1F) >> 0;
    SCSP_DEBUG("maj=%d add=%d depth=%d\n", maj, add, state->cur_depth);
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
    else if (add == 24 && count < 1+1) return 0;
    else if (add == 25 && count < 1+2) return 0;
    else if (add == 26 && count < 1+4) return 0;
    else if (add == 27 && count < 1+8) return 0;
    else if (add < 24) { }
    else {
        SCSP_DEBUG("error: 'add' between 28 and 30 is not defined\n");
        return -1;
    }
    
    char t = se->type;
    char push_to_stack = '?';
    
    if (t == 'B') {
        // TODO
        ret = 1;
        push_to_stack = 'Y';
        se->remaining = -1;
    } else
    if (t == 'S') {
        // TODO
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
        // TODO
        ret = 1;
        push_to_stack = 'Y';
        se->remaining = -1;
    } else
    if (t == '!') {
        // TODO
        ret = 1;
        push_to_stack = 'N';
    } else {
        SCSP_DATAINT number;
        
        if (   (add == 25 && sizeof(SCSP_SYSINT) < 2) ||
               (add == 26 && sizeof(SCSP_SYSINT) < 4) ||
               (add == 27 && sizeof(SCSP_SYSINT) < 8)) {
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
            if ((b[1]&0x80) && sizeof(SCSP_SYSINT)<=2) {
                SCSP_DEBUG("length overflow 2\n"); 
                return -1; 
            }
            number = (b[1] << 8) | b[2];
            ret = 1+2;
        } else
        if (add==26) {
            if ((b[1]&0x80) && sizeof(SCSP_SYSINT)<=4) {
                SCSP_DEBUG("length overflow 2\n"); 
                return -1; 
            }
            number = (((b[1] << 8) | b[2]) << 16)   |   ((b[3] << 8) | b[4]);
            ret = 1+4;
        } else
        if (add==27) {
            if ((b[1]&0x80) && sizeof(SCSP_SYSINT)<=8) {
                SCSP_DEBUG("length overflow 2\n"); 
                return -1; 
            }
            number =   ((SCSP_DATAINT)((((b[1] << 8) | b[2]) << 16)   |   ((b[3] << 8) | b[4])) << 32)
                     |                 (((b[1] << 8) | b[2]) << 16)   |   ((b[3] << 8) | b[4]);
            ret = 1+8;
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
            case 'b':
                // TODO
                push_to_stack = 'Y';
                se->remaining = number;
                break;
            case 's':
                // TODO
                push_to_stack = 'Y';
                se->remaining = number;
                break;
            case '[':
                push_to_stack = 'Y';
                se->remaining = number;
                CALLBACK(array_opened, number);
                break;
            case '{':
                //TODO
                push_to_stack = 'Y';
                se->remaining = number;
                break;
            case '_':
                // TODO
                push_to_stack = 'N';
                break;
            case '.':
                // TODO
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
        
        while (state->cur_depth > 0) {
            struct scsp_stack_entry* se1 = &state->stack[state->cur_depth- 1];
            if (se1->remaining != -1) {
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
                            // TODO
                            break;
                        case 'B':
                        case 'b':
                        case 'S':
                        case 's':
                            // TODO
                            break;
                        default: {
                            SCSP_DEBUG("internal error: se->type=%c ?\n", se->type);
                            return -1;
                        }
                    }
                    state->cur_depth -= 1;
                    continue;
                }
            }
            break;
        }
    } else 
    if (push_to_stack == 'Y') {
        
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

int main(int argc, char* argv[]) {
    uint8_t buf[1024];
    FILE* f = fopen(argv[1], "r");
    size_t ret = fread(buf, 1, 1024, f);
    struct scsp_state     ss;
    struct scsp_callbacks sc = {0,};
    size_t cursor = 0;
    while(cursor < ret) {
        int ret2 = scsp_parse(&ss, &sc, NULL, buf+cursor, ret-cursor);
        fprintf(stderr, "ret2=%d\n", ret2);
        if (ret2 == -1) break;
        cursor += ret2;
    }
    return 0;
}
