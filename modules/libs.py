#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from subprocess import run
from multiprocessing import cpu_count

class Libs:
	def __init__(self, platform_path, testcase_path):
		self.platform_path = platform_path
		self.testcase_path = testcase_path

	def copy(self):
		copy_tree(self.platform_path+"/libmemphis", self.testcase_path+"/libmemphis", update=1)
		copy_tree(self.platform_path+"/libmutils", self.testcase_path+"/libmutils", update=1)

	def build(self):
		NCPU = cpu_count()
		make = run(["make", "-C", self.testcase_path+"/libmemphis", "-j", str(NCPU)])
		if make.returncode != 0:
			raise Exception("Error building libraries.")

		make = run(["make", "-C", self.testcase_path+"/libmutils", "-j", str(NCPU)])
		if make.returncode != 0:
			raise Exception("Error building libraries.")

	def install(self):
		make = run(["make", "-C", self.testcase_path+"/libmemphis", "install"])
		if make.returncode != 0:
			raise Exception("Error building libraries.")

		make = run(["make", "-C", self.testcase_path+"/libmutils", "install"])
		if make.returncode != 0:
			raise Exception("Error installing libraries.")
