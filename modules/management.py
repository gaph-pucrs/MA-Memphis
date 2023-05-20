#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from os import environ, getcwd, listdir
from subprocess import run, check_output
from multiprocessing import cpu_count
from descriptor import Descriptor
from repository import Repository

class Management:
	def __init__(self, platform_path, testcase_path, page_size):
		self.testcase_path = testcase_path
		
		self.source_path = "{}/management".format(platform_path)
		self.base_path = "{}/management".format(testcase_path)
		self.page_size = page_size

	def copy(self):
		copy_tree(self.source_path, self.base_path, update=1)

	def build(self):
		NCPU = cpu_count()

		make_env = environ.copy()
		try:
			inc_path = make_env["C_INCLUDE_PATH"]
		except:
			inc_path = ""
		
		make_env["C_INCLUDE_PATH"] = "{}/{}/include:{}".format(getcwd(), self.testcase_path, inc_path)
		make_env["LDFLAGS"] = "-L{}/{}/lib --specs=nano.specs -Wl,-Ttext=0 -u _getpid".format(getcwd(), self.testcase_path)

		for task in listdir(self.base_path):
			make = run(["make", "-C", "{}/{}".format(self.base_path, task), "-j", str(NCPU)], env=make_env)
			if make.returncode != 0:
				raise Exception("Error build MA task {}.".format(task))

	def check_size(self):
		self.text_sizes = {}
		self.data_sizes = {}
		self.bss_sizes	= {}
		self.entry_points = {}

		for task in listdir(self.base_path):
			path = "{}/{}/{}.elf".format(self.base_path, task, task)

			out = check_output(["riscv64-elf-size", path]).split(b'\n')[1].split(b'\t')

			self.data_sizes[task] = int(out[1])
			self.text_sizes[task] = self.__get_txt_size(task)*4 - self.data_sizes[task]
			self.bss_sizes[task] = int(out[2])

			out = check_output(["riscv64-elf-readelf", path, "-h"]).split(b'\n')[10].split(b' ')[-1]
			self.entry_points[task] = int(out, 16)
			
		print("\n********************* MA page size report *********************")
		for task in listdir(self.base_path):
			size = self.text_sizes[task] + self.data_sizes[task] + self.bss_sizes[task]
			if size <= self.page_size:
				print("Task {} memory usage {}/{} bytes".format(task.rjust(25), str(size).rjust(6), str(self.page_size).ljust(6)))
			else:
				raise Exception("Task {} memory usage of {} is bigger than page size of {}".format(task, size, self.page_size))
		print("******************* End MA page size report *******************")

	def generate_repo(self, repodebug):
		for task in listdir(self.base_path):
			repo = Repository()

			descr = Descriptor("{}/{}/config.yaml".format(self.base_path, task), task)

			task_type = descr.get_type()
			repo.add(task_type, "Task type tag")

			repo.add(self.text_sizes[task], "txt size")
			repo.add(self.data_sizes[task], "data size")
			repo.add(self.bss_sizes[task], "bss size")
			repo.add(self.entry_points[task], "entry point")

			task_hex = open("{}/{}/{}.txt".format(self.base_path, task, task), "r")

			for line in task_hex:
				repo.add(int(line, 16), "")

			task_hex.close()

			repo.write("{}/{}.txt".format(self.base_path, task))
			if repodebug:
				repo.write_debug("{}/{}_debug.txt".format(self.base_path, task))		

	def __get_txt_size(self, task):
		return sum(1 for line in open("{}/{}/{}.txt".format(self.base_path, task, task)))

class ManagementIds:
	def __init__(self):
		self.lines = []
		self.lines.append("#pragma once\n")

	def add(self, task, size, ids):
		self.lines.append("#define\t"+task+"_size"+"\t"+str(size)+"\n")
		self.lines.append("static const int "+task+"[] = {")
		for id_seq in ids:
			self.lines.append(str(id_seq)+",")
		self.lines.append("};\n")

	def write(self, path):
		file = open(path, "w")
		file.writelines(self.lines)
		file.close()
