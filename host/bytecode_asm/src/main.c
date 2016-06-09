#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/******************************************
 * -a          Assemble file to bytecode.
 * -d          Disassemble file to bytecode.
 * -m          Print metadata for assembled file.
 * -o <file>   Output assembled/disassembled code to file
 * --          Terminate parsing options.
 */

void usage(char* name) {
	printf("Usage:\n"
           "  Assemble:      %s -s [-o <file>] <file>\n"
           "  Disassemble:   %s -d [-o <file>] <file>\n"
           "  Show metadata: %s -m <file>\n",
           name, name, name
           );
}

int main(int argc, char **argv) {
	FILE* f = fopen("test.pco", "rb"); // The 'b' is only required if on Windows - ugh.
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	rewind(f);
	char* data = calloc(1, size + 1);

	fread(data, 1, size, f);

	fclose(f);

	tokenize(data, size);
}
