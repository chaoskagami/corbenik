The stdlib/ directory is a self-contained, mostly conformant implementation of the C Standard Library for 3DS in ARM9 code. It supports a good chunk of standard functions and interfaces which follow:

FILE* fopen(const char* name, const char* mode)

   Opens a file @name@ with access mode @mode@ and returns an opaque FILE pointer.

size_t fread(void* buf, size_t size, size_t elem, FILE* handle)

   Reads @elem@ elements of size @size@ from the file @handle@, storing read data to @buf@.

size_t fread(void* buf, size_t size, size_t elem, FILE* handle)

   Writes @elem@ elements of size @size@ to the file @handle@, reading data to write from @buf@.

void fseek(FILE* file, );

