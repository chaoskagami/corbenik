#!/usr/bin/env python2
# -*- encoding: utf8 -*-

# This assembler is very dumb and needs severe improvement.
# Maybe I should rewrite it as an LLVM backend. That would
# be much easier to maintain and far less hackish.

# Either way, expect this code to change, a lot. The bytecode
# format probably won't.

import os
import sys
import re
import struct

in_file  = ""
out_file = ""

def usage():
	print("Usage: " + sys.argv[0] + " <input.bas> <output.bco>")

lines = 0
def syn_err(x):
	print("error - " + str(line) + " - " + x)
	exit(2)

def rel_name(x):
	return {
		'native'   : "00",
		'agb'      : "01",
		'twl'      : "02",

		'native_p9': "03",
		'agb_p9'   : "04",
		'twl_p9'   : "05",

		'native_s0': "06",
		'native_s1': "07",
		'native_s2': "08",
		'native_s3': "09",

		'agb_s0'   : "0A",
		'agb_s1'   : "0B",
		'agb_s2'   : "0C",
		'agb_s3'   : "0D",

		'twl_s0'   : "0E",
		'twl_s1'   : "0F",
		'twl_s2'   : "10",
		'twl_s3'   : "11",
	}.get(x, "-1")

name = "NO NAME"
desc = "NO DESC"
title = []
ver = "01"
flags = []
uuid = ""
deps = []

def cat_list(list):
	retstr = ""
	for str in list:
		retstr += str + " "
	return retstr

def parse_op(token_list, instr_offs):
	global title
	global desc
	global name
	global ver
	global flags
	global uuid
	global deps
	s = len(token_list) # Get size.
	if s == 0:
		return bytearray() # Empty.

	if token_list[0] == "#": # Comment.
		if s < 3:
			return bytearray() # Nope.
		elif token_list[1] == "$name": # Meta: name
			name = cat_list(token_list[2:])
		elif token_list[1] == "$desc": # Description
			desc = cat_list(token_list[2:])
		elif token_list[1] == "$title": # Title list
			title = token_list[2:]
		elif token_list[1] == "$ver": # Version
			ver = token_list[2]
		elif token_list[1] == "$uuid": # UUID
			uuid = token_list[2]
		elif token_list[1] == "$flags": # Flags
			flags = token_list[2:]
		elif token_list[1] == "$deps": # Flags
			deps = token_list[2:]

		return bytearray()


	if token_list[0] == "nop": # Nop. Expects 0 args.
		return bytearray.fromhex("00")
	elif token_list[0] == "rel": # Rel. Expects one argument. Possibly requires mapping.
		if s != 2:
			syn_err("expected one argument")

		index = rel_name(token_list[1])
		if index == "-1":
			# TODO - Check if an integer was passed.
			syn_error("invalid argument")

		return bytearray.fromhex("01" + index)
	elif token_list[0] == "find":
		if s != 2:
			syn_err("invalid number of arguments")

		# We cut corners and calculate stuff manually.
		return bytearray.fromhex("02") + bytearray([len(token_list[1]) / 2]) + bytearray.fromhex(token_list[1])
	elif token_list[0] == "back":
		if s != 2:
			syn_err("invalid number of arguments")

		return bytearray.fromhex("03" + token_list[1])
	elif token_list[0] == "fwd":
		if s != 2:
			syn_err("invalid number of arguments")

		return bytearray.fromhex("04" + token_list[1])
	elif token_list[0] == "set":
		if s != 2:
			syn_err("invalid number of arguments")

		# We cut corners and calculate stuff manually.
		return bytearray.fromhex("05") + bytearray([len(token_list[1]) / 2]) + bytearray.fromhex(token_list[1])
	elif token_list[0] == "test":
		if s != 2:
			syn_err("invalid number of arguments")

		# We cut corners and calculate stuff manually.
		return bytearray.fromhex("06") + bytearray([len(token_list[1]) / 2]) + bytearray.fromhex(token_list[1])
	elif token_list[0] == "jmp":
		if s != 2:
			syn_err("invalid number of arguments")

		if instr_offs == None:
			return bytearray.fromhex("070000")
		else:
			tok = bytearray.fromhex(token_list[1])
			num = struct.unpack(">H", tok)[0]
			return bytearray.fromhex("07") + struct.pack(">H", instr_offs[num])
	elif token_list[0] == "rewind":
		return bytearray.fromhex("08")
	elif token_list[0] == "and":
		if s != 2:
			syn_err("invalid number of arguments")

		# We cut corners and calculate stuff manually.
		return bytearray.fromhex("09") + bytearray([len(token_list[1]) / 2]) + bytearray.fromhex(token_list[1])
	elif token_list[0] == "or":
		if s != 2:
			syn_err("invalid number of arguments")

		# We cut corners and calculate stuff manually.
		return bytearray.fromhex("0A") + bytearray([len(token_list[1]) / 2]) + bytearray.fromhex(token_list[1])
	elif token_list[0] == "xor":
		if s != 2:
			syn_err("invalid number of arguments")

		# We cut corners and calculate stuff manually.
		return bytearray.fromhex("0B") + bytearray([len(token_list[1]) / 2]) + bytearray.fromhex(token_list[1])
	elif token_list[0] == "not":
		if s != 2:
			syn_err("invalid number of arguments")

		return bytearray.fromhex("0C") + bytearray.fromhex(token_list[1])
	elif token_list[0] == "ver":
		if s != 2:
			syn_err("invalid number of arguments")

		return bytearray.fromhex("0D") + bytearray.fromhex(token_list[1])

def pad_zero_r(x, c):
	while len(x) < c:
		x = x + bytearray([0])
	return x


def pad_zero_l(x, c):
	while len(x) < c:
		x = bytearray([0]) + x
	return x

def flag_convert(x):
	flags = 0
	for f in x:
		if f == "require":
			flags &= 0x1
		if f == "devmode":
			flags &= 0x2
		if f == "noabort":
			flags &= 0x4
	return pad_zero_r(bytearray([flags]), 4)

try:
	# Read input and output files.
	in_file  = sys.argv[1]
	out_file = sys.argv[2]
except:
	usage()
	exit(1)

size = 0

offsets = []

with open(in_file, "r") as ins:
	with open(out_file, "wb") as writ:
		bytecode = bytearray()

		# We have to do two passes because of JMP.
		# One to figure out the opcode offsets, one
		# to actually parse everything.

		for line in ins:
			lines += 1
			tokens = re.split("\s+", line.strip("\n")) # Split by whitespace.
			bytes = parse_op(tokens, None) # Parse.
			if bytes:
				offsets += [size]
				size += len(bytes)

		offsets += [size+1] # So we can jump past the last instruction for 'exit' type behavior

		ins.seek(0)

		for line in ins:
			lines += 1
			tokens = re.split("\s+", line.strip("\n")) # Split by whitespace.
			bytes = parse_op(tokens, offsets) # Parse.
			if bytes:
				bytecode += bytes

		data  = bytearray("AIDA")
		data += bytearray.fromhex(ver)
		data += pad_zero_r(bytearray(name),         64)
		data += pad_zero_r(bytearray(desc),         256)
		data += pad_zero_r(bytearray.fromhex(uuid), 8)
		data += flag_convert(flags)
		data += struct.pack('I', len(title))
		data += struct.pack('I', len(deps))
		data += struct.pack('I', size)
		if title:
			for f in title:
				tid = bytearray.fromhex(f) # Endianness.
				tid.reverse()
				data += tid
		if deps:
			for f in deps:
				data += pad_zero_r(bytearray.fromhex(f), 8)
		data += bytecode
		writ.write(data)

