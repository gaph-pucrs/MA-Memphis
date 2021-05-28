#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from subprocess import run
from multiprocessing import cpu_count

class Libs:
	def __init__(self, platform_path, testcase_path):
		self.platform_path = platform_path
		self.testcase_path = testcase_path

	def copy(self):
		copy_tree(self.platform_path+"/lib", self.testcase_path+"/lib", update=1)

	def build(self):
		NCPU = cpu_count()
		run(["make", "-C", self.testcase_path+"/lib", "-j", str(NCPU)])
