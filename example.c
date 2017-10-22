#include <stdio.h>
#include "scsp.h"

static int just_opened;

SCSP_SYSINT q_array_opened (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one) {
    printf("[");
    just_opened = 1;
    return 0;
}
SCSP_SYSINT q_array_item (SCSP_USERDATA userdata) {
    if (!just_opened) {
        printf(", ");
    }
    just_opened = 0;
    return 0;
}
SCSP_SYSINT q_array_closed (SCSP_USERDATA userdata) {
    printf("]");
    return 0;
}
SCSP_SYSINT q_map_opened (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one) {
    printf("{");
    just_opened = 1;
    return 0;
}
SCSP_SYSINT q_map_closed (SCSP_USERDATA userdata) {
    printf("}");
    return 0;
}
SCSP_SYSINT q_map_key (SCSP_USERDATA userdata) {
    if (!just_opened) {
        printf(", ");
    }
    just_opened = 0;
    return 0;
}
SCSP_SYSINT q_map_value (SCSP_USERDATA userdata) {
    printf(": ");
    return 0;
}
SCSP_SYSINT q_integer (SCSP_USERDATA userdata, SCSP_DATAINT value) {
    printf("%ld", value);
    return 0;
}
SCSP_SYSINT q_bytestring_open (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one) {
    printf("h'");
    return 0;
}
SCSP_SYSINT q_bytestring_chunk (SCSP_USERDATA userdata, uint8_t* buf, size_t len) {
    int i;
    for (i=0; i<len; ++i) {
        printf("%02X", (int)buf[i]);
    }
    return 0;
}
SCSP_SYSINT q_bytestring_close (SCSP_USERDATA userdata) {
    printf("'");
    return 0;
}
SCSP_SYSINT q_string_open (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one) {
    printf("\"");
    return 0;
}
SCSP_SYSINT q_string_chunk (SCSP_USERDATA userdata, uint8_t* buf, size_t len) {
    fwrite(buf, 1, len, stdout);
    return 0;
}
SCSP_SYSINT q_string_close (SCSP_USERDATA userdata) {
    printf("\"");
    return 0;
}
SCSP_SYSINT q_simple (SCSP_USERDATA userdata, char value) {
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
SCSP_SYSINT q_simple_other (SCSP_USERDATA userdata, SCSP_DATAINT value) {
    printf("simple(%ld)", value);
    return 0;
}
SCSP_SYSINT q_noninteger (SCSP_USERDATA userdata, double value) {
    printf("%lg", value);
    return 0;
}

struct scsp_callbacks sc = {
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
    uint8_t buf[1024];
    FILE* f = fopen(argv[1], "r");
    size_t ret = fread(buf, 1, 1024, f);
    
    struct scsp_state     ss;
    just_opened = 0;
    
    size_t cursor = 0;
    while(cursor < ret) {
        int ret2 = scsp_parse(&ss, &sc, NULL, buf+cursor, ret-cursor);
        //fprintf(stderr, "ret2=%d\n", ret2);
        if (ret2 == -1) break;
        cursor += ret2;
    }
    printf("\n");
    return 0;
}
