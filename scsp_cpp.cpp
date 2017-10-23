#include "scsp_cpp.hpp"

extern "C" {
#include "scsp.h"
}

#include <istream>
#include <string.h>

namespace scsp {
    
static SCSP_INT tocpp_integer (SCSP_USERDATA userdata, SCSP_INT value) {
    ((class Callbacks*)(void*)userdata)->integer(value);
    return 0;
}
static SCSP_INT tocpp_bytestring_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    ((class Callbacks*)(void*)userdata)->bytestring_opened(size_or_minus_one);
    return 0;
}
static SCSP_INT tocpp_bytestring_chunk (SCSP_USERDATA userdata, const uint8_t* buf, size_t len) {
    ((class Callbacks*)(void*)userdata)->bytestring_chunk(buf, len);
    return 0;
}
static SCSP_INT tocpp_bytestring_closed (SCSP_USERDATA userdata) {
    ((class Callbacks*)(void*)userdata)->bytestring_closed();
    return 0;
}
static SCSP_INT tocpp_string_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    ((class Callbacks*)(void*)userdata)->string_opened(size_or_minus_one);
    return 0;
}
static SCSP_INT tocpp_string_chunk (SCSP_USERDATA userdata, const uint8_t* buf, size_t len) {
    ((class Callbacks*)(void*)userdata)->string_chunk(buf, len);
    return 0;
}
static SCSP_INT tocpp_string_closed (SCSP_USERDATA userdata) {
    ((class Callbacks*)(void*)userdata)->string_closed();
    return 0;
}
static SCSP_INT tocpp_array_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    ((class Callbacks*)(void*)userdata)->array_opened(size_or_minus_one);
    return 0;
}
static SCSP_INT tocpp_array_item (SCSP_USERDATA userdata) {
    ((class Callbacks*)(void*)userdata)->array_item();
    return 0;
}
static SCSP_INT tocpp_array_closed (SCSP_USERDATA userdata) {
    ((class Callbacks*)(void*)userdata)->array_closed();
    return 0;
}
static SCSP_INT tocpp_map_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    ((class Callbacks*)(void*)userdata)->map_opened(size_or_minus_one);
    return 0;
}
static SCSP_INT tocpp_map_key (SCSP_USERDATA userdata) {
    ((class Callbacks*)(void*)userdata)->map_key();
    return 0;
}
static SCSP_INT tocpp_map_value (SCSP_USERDATA userdata) {
    ((class Callbacks*)(void*)userdata)->map_value();
    return 0;
}
static SCSP_INT tocpp_map_closed (SCSP_USERDATA userdata) {
    ((class Callbacks*)(void*)userdata)->map_closed();
    return 0;
}
static SCSP_INT tocpp_simple (SCSP_USERDATA userdata, char value) {
    ((class Callbacks*)(void*)userdata)->simple(value);
    return 0;
}
static SCSP_INT tocpp_simple_other (SCSP_USERDATA userdata, SCSP_INT value) {
    ((class Callbacks*)(void*)userdata)->simple_other(value);
    return 0;
}
static SCSP_INT tocpp_tag (SCSP_USERDATA userdata, SCSP_INT value) {
    ((class Callbacks*)(void*)userdata)->tag(value);
    return 0;
}
#if SCSP_ENABLE_FLOAT
static SCSP_INT tocpp_noninteger (SCSP_USERDATA userdata, double value) {
    ((class Callbacks*)(void*)userdata)->noninteger(value);
    return 0;
}
#endif

static struct scsp_callbacks tocpp_callbacks = {
    &tocpp_integer,
    &tocpp_bytestring_opened,
    &tocpp_bytestring_chunk,
    &tocpp_bytestring_closed,
    &tocpp_string_opened,
    &tocpp_string_chunk,
    &tocpp_string_closed,
    &tocpp_array_opened,
    &tocpp_array_item,
    &tocpp_array_closed,
    &tocpp_map_opened,
    &tocpp_map_key,
    &tocpp_map_value,
    &tocpp_map_closed,
    &tocpp_simple,
    &tocpp_simple_other,
    &tocpp_tag
#if SCSP_ENABLE_FLOAT
    ,&tocpp_noninteger
#endif
};


bool SCSP_EXPORT parse_from_istream(
            std::istream& is,
            class Callbacks& callbacks)
{
    struct scsp_state     ss;
    ss.cur_depth = 0;
    uint8_t buf[8192];
    
    size_t read_cursor = 0;
    for(;;) {
        if (is.eof()) {
            if (read_cursor == 0) {
                return true;
            } else {
                return false;
            }
        }
        if (is.fail()) {
            return false;
        }
        is.read((char*)(buf+read_cursor), (sizeof buf) - read_cursor);
        size_t len = is.gcount();
       
        if (len == 0) {
            continue;
        }
        
        len += read_cursor;
        read_cursor=0;
        
        size_t write_cursor = 0;
        while(write_cursor < len) {
            int ret2 = scsp_parse_lowlevel(&ss, &tocpp_callbacks, (void*)&callbacks, buf+write_cursor, len-write_cursor);
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

SCSP_INT SCSP_EXPORT parse_from_memory(
            const void *buffer,
            size_t count,
            class Callbacks& callbacks)
{
    return scsp_parse_from_memory(buffer, count, &tocpp_callbacks, (void*)&callbacks);
}

SCSP_INT SCSP_EXPORT parse_from_fd(
            int fd, 
            class Callbacks& callbacks)
{
    return scsp_parse_from_fd(fd, &tocpp_callbacks, (void*)&callbacks);
}

class State {
    public: 
    struct scsp_state ss;
};

class State* new_state() { 
    class State* st = new State(); 
    st->ss.cur_depth = 0;
    return st;
}
void delete_state(class  State* state) {
    delete state;
}

SCSP_INT SCSP_EXPORT parse_lowlevel(
            class State& state,
            class Callbacks& callbacks,
            const void* buf,
            size_t count)
{
    return scsp_parse_lowlevel(&state.ss, &tocpp_callbacks, (void*)&callbacks, buf, count);
}

} // namespace scsp
