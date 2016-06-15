#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char font_data[] = {
#include "font.h"
};
#include "font_prop.h"

int main() {
	FILE* f = fopen("host/termfont.bin", "wb");
    fwrite(&font_width, 1, 4, f);
    fwrite(&font_height, 1, 4, f);
    fwrite(font_data, 1, sizeof(font_data), f);
    fclose(f);
    return 0;
}
