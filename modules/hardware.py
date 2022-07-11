#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from subprocess import run
from multiprocessing import cpu_count

class Hardware:
	def __init__(self, hw, platform_path, testcase_path):
		self.PKG_PAGE_SIZE 			= hw["page_size_KB"]
		self.PKG_MAX_LOCAL_TASKS 	= hw["tasks_per_PE"]
		self.PKG_N_PE_X 			= hw["mpsoc_dimension"][0]
		self.PKG_N_PE_Y 			= hw["mpsoc_dimension"][1]
		self.peripherals			= hw["Peripherals"]

		self.filterEnable           = hw["filter_enable"]
		if isinstance(self.filterEnable, bool):
				self.filterEnable = [self.filterEnable for i in range(0, PKG_N_PE_X*PKG_N_PE_Y)]

		self.timeoutMax             = hw["timeout_max"]

		self.memory_size = self.PKG_PAGE_SIZE*(self.PKG_MAX_LOCAL_TASKS + 1)  # 1 page for kernel

		self.PKG_N_PE = self.PKG_N_PE_X * self.PKG_N_PE_Y

		self.platform_path = platform_path
		self.testcase_path = testcase_path

	def copy(self):
		copy_tree(self.platform_path+"/hardware", self.testcase_path+"/hardware", update=1)
		self.generate_definitions()
		self.generate_pkg()

	def generate_definitions(self):
		definitions = HardwareDefinitions()
		definitions.define("PAGE_SIZE_BYTES", str(self.PKG_PAGE_SIZE*1024))
		definitions.define("MEMORY_SIZE_BYTES", str(self.memory_size*1024))
		definitions.define("N_PE_X", str(self.PKG_N_PE_X))
		definitions.define("N_PE_Y", str(self.PKG_N_PE_Y))
		definitions.define("N_PE", str(self.PKG_N_PE))

		io_list = []
		for peripheral in self.peripherals:
			x = int(peripheral["pe"][0])
			y = int(peripheral["pe"][2])
			seq_addr = y*self.PKG_N_PE_X + x
			port = self.port_code(peripheral["port"])
			definitions.add_peripheral(peripheral["name"], seq_addr)
			io_list.append((seq_addr, port))

		definitions.add_io_list(io_list, self.PKG_N_PE)

		definitions.write(self.testcase_path+"/hardware/definitions.h")

	def generate_pkg(self):
		definitions = PkgDefinitions()
		definitions.define("PAGE_SIZE_BYTES", str(self.PKG_PAGE_SIZE*1024))
		definitions.define("MEMORY_SIZE_BYTES", str(self.memory_size*1024))
		definitions.define("NUMBER_PROCESSORS_X", str(self.PKG_N_PE_X))
		definitions.define("NUMBER_PROCESSORS_Y", str(self.PKG_N_PE_Y))
		definitions.define("NUMBER_PROCESSORS", str(self.PKG_N_PE))

		definitions.define("FILTER_ENABLE", "(" + "".join([str(en) + ", " if i != len(self.filterEnable)-1 else str(en) for i, en in enumerate(self.filterEnable)]) + ")", "boolean_vector(0 to NUMBER_PROCESSORS_X*NUMBER_PROCESSORS_Y - 1)")
		definitions.define("TIMEOUT_MAX", str(self.timeoutMax))

		definitions.write(self.testcase_path+"/hardware/src/hemps_pkg.vhd")

	def port_code(self, char):
		if char == 'E':
			return 0
		if char == 'W':
			return 1
		if char == 'N':
			return 2
		if char == 'S':
			return 3
		if char == 'L':
			return 4
		return 5

	def build(self):
		NCPU = cpu_count()
		make = run(["make", "-C", self.testcase_path+"/hardware", "-j", str(NCPU)])
		if make.returncode != 0:
			raise Exception("Error building hardware.")

class HardwareDefinitions:
	def __init__(self):
		self.lines = []
		self.lines.append("#pragma once\n")

	def define(self, key, value):
		self.lines.append("#define\t"+key+"\t"+value+"\n")

	def add_peripheral(self, peripheral, pe):
		self.lines.append("#define\t"+peripheral+"\t"+str(pe)+"\n")

	def add_io_list(self, io_list, n_pe):
		io_list = sorted(io_list, key = lambda x: x[0])
		self.lines.append("const char io_port[N_PE] = {")

		io_pe = 0

		for pe in range(n_pe):
			if pe == io_list[io_pe][0]:
				self.lines.append(str(io_list[io_pe][1])+",")
				io_pe = io_pe + 1
			else:
				self.lines.append(str(5)+",")

		self.lines.append("};\n")

	def write(self, path):
		file = open(path, "w")
		file.writelines(self.lines)
		file.close()

class PkgDefinitions:
	def __init__(self):
		self.lines = []
		self.lines.append("library IEEE;\n")
		self.lines.append("use IEEE.Std_Logic_1164.all;\n")
		self.lines.append("use IEEE.std_logic_unsigned.all;\n")
		self.lines.append("use IEEE.std_logic_arith.all;\n")
		self.lines.append("package hemps_pkg is\n\n")

	#def define(self, key, value):
	#	self.lines.append("constant {} : integer := {};\n".format(key, value))

	def define(self, key, value, value_type = "integer"):
		self.lines.append("    constant {} : {} := {};\n".format(key, value_type, value))

	def write(self, path):
		self.lines.append("\nend hemps_pkg;\n")
		file = open(path, "w")
		file.writelines(self.lines)
		file.close()
