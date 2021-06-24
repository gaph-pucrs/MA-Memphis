#!/usr/bin/env python3
from os.path import exists
from filecmp import cmp
from os import makedirs
from shutil import copyfile
from distutils.dir_util import copy_tree, remove_tree
from yaml import safe_load
from libs import Libs
from kernel import Kernel
from bootloader import Bootloader
from management import Management
from hardware import Hardware

class Testcase:
	def __init__(self, platform_path, testcase_base):
		self.platform_path = platform_path
		self.path = platform_path
		self.base = testcase_base

		name = self.base.split(".yaml")[0].split("/")
		name = name[len(name) - 1]

		self.base_dir = name
		self.file = self.base_dir+"/"+name+".yaml"

		yaml = safe_load(open(self.base, "r"))

		self.libs = Libs(self.platform_path, self.base_dir)
		self.kernel = Kernel(yaml["sw"], yaml["hw"], self.platform_path, self.base_dir)
		self.bootloader = Bootloader(yaml["hw"], self.platform_path, self.base_dir)
		self.hardware = Hardware(yaml["hw"], self.platform_path, self.base_dir)

		self.PKG_N_PE_X = yaml["hw"]["mpsoc_dimension"][0]
		self.PKG_N_PE_Y = yaml["hw"]["mpsoc_dimension"][1]
		
	def copy(self):
		# If testcase has been updated, delete to rebuild everything
		if self.__is_obsolete():
			remove_tree(self.base_dir)

		makedirs(self.base_dir+"/applications", exist_ok=True)
		makedirs(self.base_dir+"/management", exist_ok=True)
		copy_tree(self.platform_path+"/include", self.base_dir+"/include", update=True)
		copyfile(self.base, self.file)

		self.__create_platform()
		self.__create_services()
		self.__create_cpu()

		self.libs.copy()
		self.kernel.copy()
		self.bootloader.copy()
		self.hardware.copy()

	def build(self):
		self.libs.build()
		
		self.kernel.build()

		self.bootloader.build()
		self.hardware.build()

		self.kernel.check_size()

	def __is_obsolete(self):
		if exists(self.file):
			if not cmp(self.base, self.file):
				return True
		
		return False

	def __create_platform(self):
		cfg = open(self.base_dir+"/platform.cfg", "w")

		cfg.write("router_addressing XY\n")
		cfg.write("channel_number {}\n".format(1))
		cfg.write("mpsoc_x {}\n".format(self.PKG_N_PE_X))
		cfg.write("mpsoc_y {}\n".format(self.PKG_N_PE_Y))
		cfg.write("flit_size {}\n".format(32))
		cfg.write("clock_period_ns {}\n".format(10))

		# Legacy lines. This will 'trick' the debugger into assigning no master
		cfg.write("cluster_x {}\n".format(self.PKG_N_PE_X))
		cfg.write("cluster_y {}\n".format(self.PKG_N_PE_Y))
		cfg.write("manager_position_x {}\n".format(self.PKG_N_PE_X))
		cfg.write("manager_position_y {}\n".format(self.PKG_N_PE_Y))

		cfg.close()
		
	def __create_services(self):
		services = open("{}/include/services.h".format(self.base_dir), "r")
		cfg = open("{}/services.cfg".format(self.base_dir), "w")

		for line in services:
			words = line.split()
			if "#define" in words and len(words) > 2:
				key = words[1]
				value = 0
				value = int(words[2], base=16)
				cfg.write("{} {:x}\n".format(key, value))

		services.close()

		cfg.write("\n")
		cfg.write("$TASK_ALLOCATION_SERVICE 40 221\n")
		cfg.write("$TASK_TERMINATED_SERVICE 70 221\n")

		cfg.close()

	def __create_cpu(self):
		cfg = open("{}/cpu.cfg".format(self.base_dir), "w")

		cfg.write("Interruption\t65536\n")
		cfg.write("Scheduler\t262144\n")
		cfg.write("Idle\t524288\n")

		cfg.close()
