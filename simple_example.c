#include <string.h>
#include <assert.h>

#include "scsp.h"

int64_t array_opened(void* userdata, int64_t nelem) {
    assert(nelem == -1 || nelem == 4);
    return 0;
}

int64_t array_closed(void* userdata) {
    assert(userdata == (void*)0x5645);
    return 0;
}

int64_t integer(void* userdata, int64_t value) {
    assert(value == 5 || value == -10 || value==1234567);
    return 0;
}

int64_t string_chunk(void* userdata, const uint8_t* buf, size_t len) {
    assert(len == 3);
    assert(!memcmp((const char*)buf, "ABC", 3));
    return 0;
}

int main() {
    
    struct scsp_callbacks cb;
    memset(&cb, 0, sizeof(cb));
    
    cb.array_opened = &array_opened;
    cb.array_closed = &array_closed;
    cb.integer = &integer;
    cb.string_chunk = &string_chunk;
    
    const char *cbor_data = 
        "\x9F"                        // [
            "\x1A" "\x00\x12\xD6\x87" //     1234567,
            "\x63" "ABC"              //     "ABC",
            "\x84"                    //     [
                "\x05"                //         5,
                "\x05"                //         5,
                "\x29"                //         -10,
                "\x29"                //         -10,
                                      //     ]
        "\xFF"                        // ]
    ;
    size_t cbor_data_len = 2+5+4+1+4;
    
    assert(scsp_parse_from_memory(cbor_data, cbor_data_len, &cb, (void*)0x5645) == 0);
    
    return 0;
}
