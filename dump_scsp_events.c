#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "scsp.h"


SCSP_INT q_integer (SCSP_USERDATA userdata, SCSP_INT value) {
    printf("integer(%ld)\n", value); fflush(stdout);
    return 0;
}
SCSP_INT q_bytestring_open (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("bytestring_open(%ld)\n", size_or_minus_one); fflush(stdout);
    return 0;
}
SCSP_INT q_bytestring_chunk (SCSP_USERDATA userdata, const uint8_t* buf, size_t len) {
    int i;
    printf("bytestring_chunk(\"");
    for (i=0; i<len; ++i) {
        printf("\\x%02X", (int)buf[i]);
    }
    printf("\")\n");
    fflush(stdout);
    return 0;
}
SCSP_INT q_bytestring_close (SCSP_USERDATA userdata) {
    printf("bytestring_close\n"); fflush(stdout);
    return 0;
}
SCSP_INT q_string_open (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("string_open(%ld)\n", size_or_minus_one); fflush(stdout);
    return 0;
}
SCSP_INT q_string_chunk (SCSP_USERDATA userdata, const uint8_t* buf, size_t len) {
    int i;
    printf("string_chunk(\"");
    for (i=0; i<len; ++i) {
        printf("\\x%02X", (int)buf[i]);
    }
    printf("\")\n");
    fflush(stdout);
    return 0;
}
SCSP_INT q_string_close (SCSP_USERDATA userdata) {
    printf("string_close\n"); fflush(stdout);
    return 0;
}
SCSP_INT q_array_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("array_opened(%ld)\n", size_or_minus_one); fflush(stdout);
    return 0;
}
SCSP_INT q_array_item (SCSP_USERDATA userdata) {
    printf("array_item\n"); fflush(stdout);
    return 0;
}
SCSP_INT q_array_closed (SCSP_USERDATA userdata) {
    printf("array_closed\n"); fflush(stdout);
    return 0;
}
SCSP_INT q_map_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("map_opened(%ld)\n", size_or_minus_one); fflush(stdout);
    return 0;
}
SCSP_INT q_map_key (SCSP_USERDATA userdata) {
    printf("map_key\n"); fflush(stdout);
    return 0;
}
SCSP_INT q_map_value (SCSP_USERDATA userdata) {
    printf("map_value\n"); fflush(stdout);
    return 0;
}
SCSP_INT q_map_closed (SCSP_USERDATA userdata) {
    printf("map_closed\n"); fflush(stdout);
    return 0;
}
SCSP_INT q_simple (SCSP_USERDATA userdata, char value) {
    printf("simple(%c)\n", value); fflush(stdout);
    return 0;
}
SCSP_INT q_simple_other (SCSP_USERDATA userdata, SCSP_INT value) {
    printf("simple_other(%ld)\n", value); fflush(stdout);
    return 0;
}
SCSP_INT q_noninteger (SCSP_USERDATA userdata, double value) {
    printf("noninteger(%lg)\n", value); fflush(stdout);
    return 0;
}

struct scsp_callbacks q_callbacks = {
    &q_integer,
    &q_bytestring_open,
    &q_bytestring_chunk,
    &q_bytestring_close,
    &q_string_open,
    &q_string_chunk,
    &q_string_close,
    &q_array_opened,
    &q_array_item,
    &q_array_closed,
    &q_map_opened,
    &q_map_key,
    &q_map_value,
    &q_map_closed,
    &q_simple,
    &q_simple_other,
    &q_noninteger
};


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: dump_scsp_events {filename|-}\n");
        return 1;
    }
    int f;
    if (argv[1][0] == '-' && argv[1][1] == '\0') {
        f = 0;
    } else {
        f = open(argv[1], O_RDONLY);
        if (f == -1) {
            perror("open");
            return 2;
        }
    }
    
    if ( -1 == scsp_parse_from_fd(f, &q_callbacks, NULL) ) {
        fprintf(stderr, "Error reading CBOR\n");
        return 4;
    }
    
    return 0;
}
