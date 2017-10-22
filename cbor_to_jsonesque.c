#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
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
    uint8_t buf[8192];
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
    
    struct scsp_state     ss;
    just_opened = 0;
    
    size_t read_cursor = 0;
    for(;;) {
        size_t len = read(f, buf+read_cursor, (sizeof buf) - read_cursor);
        if (len == -1) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) { usleep(100000); continue; }
            perror("read");
            return 4;
        }
        if (len == 0) break;
        
        len += read_cursor;
        read_cursor=0;
        
        size_t write_cursor = 0;
        while(write_cursor < len) {
            int ret2 = scsp_parse(&ss, &sc, NULL, buf+write_cursor, len-write_cursor);
            //fprintf(stderr, "ret2=%d\n", ret2);
            if (ret2 == -1) {
                fprintf(stderr, "Error parsing CBOR\n");
                return 3;
            }
            if (ret2 == 0) {
                memmove(buf, buf+write_cursor, len-write_cursor);
                read_cursor = len-write_cursor;
                break;
            }
            write_cursor += ret2;
        }
        fflush(stdout);
    }
    printf("\n");
    return 0;
}
