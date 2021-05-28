#!/usr/bin/env python3

class Repository:
	def __init__(self):
		self.lines = []
		self.commentary = []
		self.addr = 0

	def add(self, value, commentary):
		str_value = "{0:0{1}X}".format(value, 8)
		self.lines.append("{}\n".format(str_value))
		self.commentary.append("{:x}\t{}\t{}\n".format(self.addr, str_value, commentary))
		self.addr += 4

	def write(self, path):
		file = open(path, "w")
		file.writelines(self.lines)
		file.close()

	def write_debug(self, path):
		file = open(path, "w")
		file.writelines(self.commentary)
		file.close()

class Start:
	def __init__(self):
		self.lines = []
		self.commentary = []

	def add(self, line, commentary):
		self.lines.append("{}\n".format(line))
		self.commentary.append("{}\t{}\n".format(line, commentary))

	def write(self, path):
		file = open(path, "w")
		file.writelines(self.lines)
		file.close()

	def write_debug(self, path):
		file = open(path, "w")
		file.writelines(self.commentary)
		file.close()
