#include <stdio.h>
#include "scsp.h"

SCSP_SYSINT q_array_opened (SCSP_USERDATA userdata, SCSP_SYSINT size_or_minus_one) {
    printf("[");
    return 0;
}
SCSP_SYSINT q_array_closed (SCSP_USERDATA userdata) {
    printf("],");
    return 0;
}
SCSP_SYSINT q_integer (SCSP_USERDATA userdata, SCSP_DATAINT value) {
    printf("%lld, ", value);
    return 0;
}

struct scsp_callbacks sc = {
    &q_array_opened,
    &q_array_closed,
    &q_integer
};


int main(int argc, char* argv[]) {
    uint8_t buf[1024];
    FILE* f = fopen(argv[1], "r");
    size_t ret = fread(buf, 1, 1024, f);
    struct scsp_state     ss;
    size_t cursor = 0;
    while(cursor < ret) {
        int ret2 = scsp_parse(&ss, &sc, NULL, buf+cursor, ret-cursor);
        fprintf(stderr, "ret2=%d\n", ret2);
        if (ret2 == -1) break;
        cursor += ret2;
    }
    printf("\n");
    return 0;
}
