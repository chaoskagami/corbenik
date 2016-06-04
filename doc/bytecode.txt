Bytecode format
===================

Instructions are one byte and have a variable number of bytes afterwards.

Note that this file describes the instruction set AS THE VM INTERPRETS IT,
e.g. not as you'd write it for the assembler.

Any integers of greater than one byte are not re-ordered to fit endianness.
If you specify 0001 as a value, the vm will read this as:

   uint16_t val = *((uint16_t*){0x00, 0x01})

Be aware of this.

Unless otherwise noted, if an instruction doesn't succeed, it will abort.

nop : 1 byte : Opcode 0x00
	Does nothing. Not actually treated as an instruction,
    rather just skipped over. This is mainly just for compatibility.

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

		18: Loader title (see info)

find <size> <pattern...> : 2 + size bytes : opcode 0x02
	Finds a pattern in memory. On success, operations
	will be performed relative to the beginning of the found pattern.

	<size>    : 1 byte
		How many bytes the pattern is.
	<pattern> : <size> bytes
		data to find

back <count> : 2 bytes : opcode 0x03
	Moves back <count> bytes from current position.

	<count> : 1 byte
		How many bytes to rewind.

fwd <count> : 2 bytes : opcode 0x04
	Moves forward <count> bytes from current position.

	<count> : 1 byte
		How many bytes to rewind.

set <size> <data...> : 2 + size bytes : opcode 0x05
	Copies the bytes in <data> to the current location pointed to,
	and increments the current location by <size> bytes copied.

	<size> : 1 byte
		How many bytes to copy.
	<data> : <size> bytes
		Data to copy.

test <size> <data...> : 2 + size bytes : opcode 0x06
	Tests if the current location's data is equivalent to <data>.
	If equivalent, goes to the next instruction. If not, skips
	one operation.

	NO ABORT ON FAIL

	<size> : 1 byte
		Size of data to test against.
	<data> : <size> bytes
		Pattern to test.

jmp <offset> : 3 bytes : opcode 0x07
	Jumps to offset instruction within the bytecode, and
    resumes execution from there.

	Note that the assembler should be passed the number of instrcution
	and will automatically calculate the offset needed.

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

title <count> <title> : 2 + <count> * 8 bytes : 0x0A
	Specifies that the following code is applicable only when being applied to
	a list of titles.

	This allows the creation of generic patches which can be used with multiple
	titles and share common parts.

	The default state is to apply code on any titleID matches within the header,
	so unless you have specialized needs you'll almost never need this.

	<count> : 1 byte
		How many titleIDs to read.
	<title> : 8 * <count> bytes
		List of titleIDs as u64.

next : 1 byte : 0xFF
	Resets state to default, and changes the base of code to the next instruction.
	This opcode is not meant to be used directly - it's emitted when generating
	caches.
