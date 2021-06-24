#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from shutil import copyfile
from subprocess import run
from multiprocessing import cpu_count

class Bootloader:
	def __init__(self, hw, platform_path, testcase_path):
		self.platform_path = platform_path
		self.testcase_path = testcase_path
		self.PKG_PAGE_SIZE = hw["page_size_KB"]

	def copy(self):
		copy_tree(self.platform_path+"/bootloader", self.testcase_path+"/bootloader", update=1)
		copyfile(self.platform_path+"/bootloader/mipsi/Makefile", self.testcase_path+"/bootloader/mipsi/Makefile")
		self.update_stack()

	def update_stack(self):
		file = open(self.testcase_path+"/bootloader/mipsi/Makefile", "a")
		file.write("PAGE_SP_INIT = "+str(self.PKG_PAGE_SIZE*1024)+"\n")
		file.close()

	def build(self):
		NCPU = cpu_count()
		make = run(["make", "-C", self.testcase_path+"/bootloader/mipsi", "-j", str(NCPU)])
		if make.returncode != 0:
			raise Exception("Error assemblying bootloader.")
