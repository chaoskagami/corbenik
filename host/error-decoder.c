#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const static char* get_desc(unsigned int index) {
    const static char *strs[] = {
        "Not found",
        "Exists already",
        "Out of space",
        "Invalidated archive",
        "Unacceptable",
        "Verification Failure",
        "Not supported",
        "Unknown",
        "Success,"
    };

    if (index == 0)
        return strs[8];

    if (index >= 100 && index < 180)
        return strs[0];
    else if (index >= 180 && index < 200)
        return strs[1];
    else if (index >= 200 && index < 220)
        return strs[2];
    else if (index >= 220 && index < 230)
        return strs[3];
    else if (index >= 230 && index < 340)
        return strs[4];
    else if (index >= 390 && index < 400)
        return strs[5];
    else if (index >= 760 && index < 780)
        return strs[6];
    return strs[7];
}



int main(int c, char** v) {
    unsigned int error, desc, module, summary, level;
    sscanf(v[1], "%08x", &error);
    printf("Code: %08x\n", error);

    desc = error & 0b1111111111;
    module = (error >> 10) & 0b11111111;
    summary = (error >> 21) & 0b111111;
    level = (error >> 27) & 0b11111;

    printf("Desc: %s (%u)\n", get_desc(desc), desc);

    return 0;
}
