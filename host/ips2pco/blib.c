#define BLIB_NO_EXTERNS

#include "blib.h"

int            blib_filefd;
unsigned char* blib_buffer;
struct stat    blib_stat;

int copy_file(__READ char* dest, __READ char* src) {
	// Can you believe there's no standard method to copy files? Neither can I.
	// We use the generic POSIX way.

	int in_fd = open(src, O_RDONLY);
	if(in_fd <= 0)
		goto error_copy_file;

	int out_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (out_fd <= 0)
		goto error_copy_file;

	char buf[8192];

	while (1) {
		ssize_t result = read(in_fd, buf, sizeof(buf));

		if (!result) // Done?
			break;

		if(result < 0) {
			// ERROR!
			fprintf(stderr, "Negative bytes read?\n");
			goto error_copy_file;
		}

		if (write(out_fd, buf, result) != result) {
			// Short write? Out of disk, maybe. Either way, abort.
			fprintf(stderr, "Short write?\n");
			goto error_copy_file;
		}
	}

	close(in_fd);
	close(out_fd);

	return 0;

error_copy_file:
	if (in_fd) close(in_fd);
	if (out_fd) close(out_fd);

	return 1;
}

int hexdump_file(__READ uint8_t *buffer, __READ uint64_t len, __READ int format) {
	return hexdump_manual(0, buffer, len, format, stdout);
}

int hexdump_manual(__READ uint64_t offset, __READ uint8_t* buffer, __READ uint64_t len, __READ int format, FILE* output) {
	// Okay, hexdump.

	for (int i = 0; i < len;) {
		int length = 16;
		if (len - i < 16) // Incomplete line.
			length = len - i;

		// First, offsets.
		if (format & PRINT_OFFSET) {

			fprintf(output, "%08x", i);

			if (format & USE_COLON)
				fprintf(output, ":");
			else if (format & PIPE_OFFSET)
				fprintf(output, " | ");
			else
				fprintf(output, " ");
			fprintf(output, " ");
		}

		// Next, bytes.
		if (format & BYTE_A) { // One byte
			int copylen = length;
			for(int j=0; j < 16; j++) {
				if (copylen) {
					fprintf(output, "%02x ", buffer[i+j]);
					copylen--;
				} else {
					fprintf(output, "   ");
				}
				if (j == 7 && (format & CENTER_SPLIT) )
					fprintf(output, " ");
			}
		} else if (format & BYTE_B) { // Two byte
			int copylen = length;
			for(int j=0; j < 16; j++) {
				if (copylen) {
					fprintf(output, "%02x", buffer[i+j]);
					if (j % 2 == 1)
						fprintf(output, " ");
					copylen--;
				} else {
					fprintf(output, "  ");
					if (j % 2 == 1)
						fprintf(output, " ");
				}
				if (j == 7 && (format & CENTER_SPLIT) )
					fprintf(output, " ");
			}
		} else if (format & BYTE_C) { // Three byte
			int copylen = length;
			for(int j=0; j < 16; j++) {
				if (copylen) {
					fprintf(output, "%02x", buffer[i+j]);
					if (j % 4 == 3)
						fprintf(output, " ");
					copylen--;
				} else {
					fprintf(output, "  ");
					if (j % 4 == 3)
						fprintf(output, " ");
				}
				if (j == 7 && (format & CENTER_SPLIT) )
					fprintf(output, " ");
			}
		}

		if (format & WITH_ASCII) { // Print ascii
			fprintf(output, " ");

			if (format & PIPE_SEPARATE) {
				fprintf(output, "|");
			}

			for(int j=0; j < length; j++) {
				// We only print printables.
				int c = buffer[i+j];
				if (c > 0x1f && c < 0x7f) // Printable?
					fprintf(output, "%c", c);
				else {
					if (format & NONPRINT_PERIOD) {
						fprintf(output, ".");
					} else if (format & NONPRINT_UNDERS) {
						fprintf(output, "_");
					} else {
						fprintf(output, " ");
					}
				}
			}

			if (format & PIPE_SEPARATE) {
				fprintf(output, "|");
			}
		}

		i += 16;
		fprintf(output, "\n");
	}

	return 0;
}

uint8_t hexb_to_u8(char a, char b) {
	if (a >= 'a' && a <= 'f') {
		a -= 'a';
		a += 10;
	} else if (a >= 'A' && a <= 'F') {
		a -= 'A';
		a += 10;
	} else if (a >= '0' && a <= '9') {
		a -= '0';
	} else {
		return 0;
	}

	if (b >= 'a' && b <= 'f') {
		b -= 'a';
		b += 10;
	} else if (b >= 'A' && b <= 'F') {
		b -= 'A';
		b += 10;
	} else if (b >= '0' && b <= '9') {
		b -= '0';
	} else {
		return 0;
	}

	return ((a<<4)|b);
}

// Unhexdump
int unhexdump_buffer(__READ uint8_t* buffer, __READ uint64_t len, __WRITE uint8_t* output) {
	for(uint64_t i=0; i < (len/2); i++) {
		output[i] = hexb_to_u8(buffer[i*2], buffer[i*2+1]);
	}

	return 0;
}
