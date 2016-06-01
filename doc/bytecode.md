Bytecode format
===================

Instructions are one byte and have a variable number of bytes afterwards.

Unless otherwise noted, if an instruction doesn't succeed, it will abort.

rel <mode> : 2 bytes : Opcode 0x01
	Chooses firmware relativity.

	<mode> : The location and size to operate in.
		0: NATIVE_FIRM (whole size)
		1: AGB_FIRM (whole size)
		2: TWL_FIRM (whole size)

		3: Native Proc9 ExeFS
		4: AGB Proc9 ExeFS
		5: TWL Proc9 ExeFS

		6: Native Section 0
		7: Native Section 1
		8: Native Section 2
		9: Native Section 3

		10: AGB Section 0
		11: AGB Section 1
		12: AGB Section 2
		13: AGB Section 3

		14: TWL Section 0
		15: TWL Section 1
		16: TWL Section 2
		17: TWL Section 3

find <size> <pattern...> : 2 + size bytes : opcode 0x02
	Finds a pattern in memory. On success, operations
	will be performed relative to the beginning of the found pattern.

	<size>    : 1 byte
		How many bytes the pattern is.
	<pattern> : <size> bytes
		data to find

back <count> : 5 bytes : opcode 0x03
	Moves back <count> bytes from current position.

	<count> : 4 bytes
		How many bytes to rewind.

fwd <count> : 5 bytes : opcode 0x04
	Moves forward <count> bytes from current position.

	<count> : 4 bytes
		How many bytes to rewind.

set <size> <data...> : 2 + size bytes : opcode 0x05
	Copies the bytes in <data> to the current location pointed to,
	and increments the current location by <size> bytes copied.

	<size> : 1 byte
		How many bytes to copy.
	<data> : <size> bytes
		Data to copy.

test <size> <data...> : 2 bytes : opcode 0x06
	Tests if the current location's data is equivalent to <data>.
	If equivalent, goes to the next instruction. If not, skips
	one operation.

	NO ABORT ON FAIL

	<size> : 1 byte
		Size of data to test against.
	<data> : <size> bytes
		Pattern to test.

jmp <offset> : 3 bytes : opcode 0x07
	Jumps to <offset> within the bytecode, and resumes execution from there.

	<offset> : 2 bytes
		Offset to jump to.

rewind : 1 byte : opcode 0x08
	Resets the location to the beginning of the space we're working off.

and <size> <data...> : 2 + <size> bytes : opcode 0x09
	Performs an AND operation bitwise using data as a mask.

	<size> : 1 byte
		Size of <data>.

	<data> : <size> bytes
		Data to bitwise and with relative data.
