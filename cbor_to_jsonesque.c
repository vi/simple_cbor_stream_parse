#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "scsp.h"

static int just_opened;

SCSP_INT q_integer (SCSP_USERDATA userdata, SCSP_INT value) {
    printf("%ld", value);
    return 0;
}
SCSP_INT q_bytestring_open (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("h'");
    return 0;
}
SCSP_INT q_bytestring_chunk (SCSP_USERDATA userdata, const uint8_t* buf, size_t len) {
    int i;
    for (i=0; i<len; ++i) {
        printf("%02X", (int)buf[i]);
    }
    return 0;
}
SCSP_INT q_bytestring_close (SCSP_USERDATA userdata) {
    printf("'");
    return 0;
}
SCSP_INT q_string_open (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("\"");
    return 0;
}
SCSP_INT q_string_chunk (SCSP_USERDATA userdata, const uint8_t* buf, size_t len) {
    fwrite(buf, 1, len, stdout);
    return 0;
}
SCSP_INT q_string_close (SCSP_USERDATA userdata) {
    printf("\"");
    return 0;
}
SCSP_INT q_array_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("[");
    just_opened = 1;
    return 0;
}
SCSP_INT q_array_item (SCSP_USERDATA userdata) {
    if (!just_opened) {
        printf(", ");
    }
    just_opened = 0;
    return 0;
}
SCSP_INT q_array_closed (SCSP_USERDATA userdata) {
    printf("]");
    return 0;
}
SCSP_INT q_map_opened (SCSP_USERDATA userdata, SCSP_INT size_or_minus_one) {
    printf("{");
    just_opened = 1;
    return 0;
}
SCSP_INT q_map_key (SCSP_USERDATA userdata) {
    if (!just_opened) {
        printf(", ");
    }
    just_opened = 0;
    return 0;
}
SCSP_INT q_map_value (SCSP_USERDATA userdata) {
    printf(": ");
    return 0;
}
SCSP_INT q_map_closed (SCSP_USERDATA userdata) {
    printf("}");
    return 0;
}
SCSP_INT q_simple (SCSP_USERDATA userdata, char value) {
    switch (value) {
        case 'T': printf("true"); break;    
        case 'F': printf("false"); break;    
        case 'N': printf("null"); break;    
        case 'U': printf("undefined"); break;    
        case '?': printf("???"); break;    
        default: printf("error");
    }
    return 0;
}
SCSP_INT q_simple_other (SCSP_USERDATA userdata, SCSP_INT value) {
    printf("simple(%ld)", value);
    return 0;
}
SCSP_INT q_noninteger (SCSP_USERDATA userdata, double value) {
    printf("%lg", value);
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
    NULL,
    &q_noninteger
};


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: cbor_to_jsonesque {filename|-}\n");
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
    
    just_opened = 0;
    
    if ( -1 == scsp_parse_from_fd(f, &q_callbacks, NULL) ) {
        fprintf(stderr, "Error reading CBOR\n");
        return 4;
    }
    
    printf("\n");
    return 0;
}
