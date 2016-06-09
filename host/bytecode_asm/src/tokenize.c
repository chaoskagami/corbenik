#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct token_s {
	enum {
		opcode  = 0,
		meta    = 1,
		integer = 2,
		bytes   = 3,
		string  = 4,
	} token_type;

	union {
		// Opcode
		uint8_t opcode;

        // Metadata
		struct {
			char *field;
			char *data;
		} meta;

		// Integer value
		uint32_t integer;

		// Data buffer
		struct {
			uint8_t len;
			uint8_t* data;
		} bytes;

		// String
		struct {
			char* str;
		} string;
	} token_data;
};

void tokenize(char* data, size_t len) {
	// Okay, first thing is first. In tokenizing, pass 1 - we want to strip all comments out of the data.
	// It'll clog us up for no good reason.

	// We also want to get rid of newlines, etc so we can compact spaces
	// afterwards.

	// Comments always begin with '#'.
	for(size_t i=0; i < len; i++) {
		if ( data[i] == '#' ) {
			// We want to remove this, so replace with
			// all spaces until end of line.
			while(data[i] != '\n' && i < len) {
				data[i] = ' ';
				i++;
			}
			data[i] = ' ';
		}
		else if (data[i] == '\n' || data[i] == '\r' || data[i] == ',')
			data[i] = ' ';
	}

	char* lstrip = data;

	// We want to trim front and back, first.
	for(size_t i=0; i < len - 1; i++) {
		if (data[i] != ' ') {
			lstrip = &data[i];
			len -= i;
			break;
		}
	}

	for(size_t i=len-1; i > 0; i--) {
		if (lstrip[i] == ' ' && lstrip[i-1] != ' ') {
			lstrip[i] = 0;
			len -= ((len-1) - i);
			break;
		}
	}

	for(size_t i=0; i < len; i++) {
		if (lstrip[i] == ' ') {
			size_t sta = i + 1;
			while(lstrip[i] == ' ') i++;
			size_t end = i;

			memmove(&lstrip[sta], &lstrip[end], len - end);

			i = sta;
			len -= (end - sta);
		}
	}

	printf("%s\n", lstrip);
}
