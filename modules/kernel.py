#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from distutils.file_util import copy_file
from subprocess import run, check_output
from multiprocessing import cpu_count

class Kernel:
	def __init__(self, sw, hw, platform_path, testcase_path):
		self.PKG_MAX_TASKS_APP		= sw["max_tasks_app"]
		self.qos_interval			= 500
		self.qos_rate				= 1

		self.PKG_PAGE_SIZE 			= hw["page_size_KB"]
		self.PKG_MAX_LOCAL_TASKS 	= hw["tasks_per_PE"]
		self.PKG_N_PE_X 			= hw["mpsoc_dimension"][0]
		self.PKG_N_PE_Y 			= hw["mpsoc_dimension"][1]
		self.peripherals			= hw["Peripherals"]
		# self.stack_size				= hw["stack_size"]

		self.PKG_N_PE = self.PKG_N_PE_X * self.PKG_N_PE_Y

		self.platform_path = platform_path
		self.testcase_path = testcase_path

	def copy(self):
		copy_tree(self.platform_path+"/kernel", self.testcase_path+"/kernel", update=1)
		copy_file(self.platform_path+"/kernel/Makefile", self.testcase_path+"/kernel/Makefile")
		self.update_stack()
		self.generate_definitions()

	def update_stack(self):
		file = open(self.testcase_path+"/kernel/Makefile", "a")
		file.write("PAGE_SP_INIT = "+str(self.PKG_PAGE_SIZE*1024)+"\n")
		file.close()

	def generate_definitions(self):
		definitions = KernelDefinitions()
		definitions.define("PKG_MAX_LOCAL_TASKS", str(self.PKG_MAX_LOCAL_TASKS))
		definitions.define("PKG_PAGE_SIZE", str(self.PKG_PAGE_SIZE*1024))
		definitions.define("PKG_MAX_TASKS_APP", str(self.PKG_MAX_TASKS_APP))
		definitions.define("PKG_PENDING_SVC_MAX", str(10))
		definitions.define("PKG_SLACK_TIME_WINDOW", str(50000))
		definitions.define("PKG_MONITOR_INTERVAL_QOS", str(self.qos_interval*100))
		definitions.define("PKG_MONITOR_RATE_QOS", str(self.qos_rate))
		definitions.define("PKG_N_PE", str(self.PKG_N_PE))
		definitions.define("PKG_N_PE_X", str(self.PKG_N_PE_X))
		definitions.define("PKG_N_PE_Y", str(self.PKG_N_PE_Y))

		definitions.define("PKG_PERIPHERALS_NUMBER", str(len(self.peripherals)))
		for peripheral in self.peripherals:
			name = peripheral["name"]
			addr = int(peripheral["pe"][0]) << 8 | int(peripheral["pe"][2])
			port = peripheral["port"]
			encoding = 0x80000000
			if port == "E":
				encoding = encoding | 0x00000000
			elif port == "W":
				encoding = encoding | 0x20000000
			elif port == "N":
				encoding = encoding | 0x40000000
			elif port == "S":
				encoding = encoding | 0x60000000
			else:
				raise Exception("Invalid port "+port+" for peripheral "+name) 

			definitions.define(name, hex(encoding | addr))

		definitions.write(self.testcase_path+"/lib/src/include/kernel_definitions.h")

	def build(self):
		NCPU = cpu_count()
		make = run(["make", "-C", self.testcase_path+"/kernel", "-j", str(NCPU)])
		if make.returncode != 0:
			raise Exception("Error building kernel.")

	def check_size(self):
		path = "{}/kernel/kernel.elf".format(self.testcase_path)

		out = check_output(["riscv64-elf-size", path]).split(b'\n')[1].split(b'\t')

		size = self.__get_txt_size()*4 + int(out[2])# + self.stack_size
					
		print("\n******************* Kernel page size report *******************")
		if size <= self.PKG_PAGE_SIZE*1024:
			print("Kernel memory usage {}/{} bytes".format(str(size).rjust(30), str(self.PKG_PAGE_SIZE*1024).ljust(6)))
		else:
			raise Exception("Kernel memory usage of {} is bigger than page size of {}".format(size, self.PKG_PAGE_SIZE*1024))
		print("***************** End kernel page size report *****************")

	def __get_txt_size(self):
		return sum(1 for line in open(self.testcase_path+"/kernel/kernel.txt"))
		
class KernelDefinitions:
	def __init__(self):
		self.lines = []
		self.lines.append("#pragma once\n")

	def define(self, key, value):
		self.lines.append("#define\t"+key+"\t"+value+"\n")

	def write(self, path):
		file = open(path, "w")
		file.writelines(self.lines)
		file.close()
