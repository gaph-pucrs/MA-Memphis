#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from distutils.file_util import copy_file
from os import environ, getcwd
from subprocess import run, check_output
from multiprocessing import cpu_count
from descriptor import Descriptor
from repository import Repository, Start

class Management:
	def __init__(self, management, platform_path, scenario_path, testcase_path):
		self.management = management
		self.testcase_path = testcase_path

		self.tasks = []
		self.task_names = []
		self.unique_tasks = []
		for task in management:
			self.tasks.append(task)
			self.task_names.append(task["task"])
		
		self.unique_tasks = set(self.task_names)

		self.source_path = "{}/management".format(platform_path)
		self.base_path = "{}/management".format(scenario_path)

	def copy(self):
		for task in self.unique_tasks:
			copy_tree("{}/{}".format(self.source_path, task), "{}/{}".format(self.base_path, task), update=1)

		self.generate_ids()

	def task_index(self, lst, element):
		result = []
		offset = -1
		while True:
			try:
				offset = lst.index(element, offset+1)
			except ValueError:
				return result
			result.append(offset)

	def generate_ids(self):
		ids = ManagementIds()
		
		sizes = []
		task_ids = []
		output_tasks = []

		for task in self.tasks:
			name = task["task"]
			if name in output_tasks:
				continue

			output_tasks.append(name)
			sizes.append(self.tasks.count(name))
			task_ids.append(self.task_index(self.tasks, name))

		for i in range(len(output_tasks)):
			ids.add(output_tasks[i], sizes[i], task_ids[i])

		if output_tasks[0] != "mapper_task":
			raise Exception("Mapper task must be the first in the management list")

		ids.write("{}/id_tasks.h".format(self.base_path))

	def build(self):
		NCPU = cpu_count()

		make_env = environ.copy()
		# make_env["CFLAGS"] = CFLAGS
		try:
			inc_path = make_env["C_INCLUDE_PATH"]
		except:
			inc_path = ""
		
		CFLAGS = ""
		make_env["C_INCLUDE_PATH"] = "{}/{}/include:{}".format(getcwd(), self.testcase_path, inc_path)
		make_env["CFLAGS"] = CFLAGS+"--include ../id_tasks.h"

		make_env["LDFLAGS"] = "-L{}/{}/lib --specs=nano.specs -Wl,-Ttext=0 -u _getpid".format(getcwd(), self.testcase_path)

		for task in self.unique_tasks:
			make = run(["make", "-C", "{}/{}".format(self.base_path, task), "-j", str(NCPU)], env=make_env)
			if make.returncode != 0:
				raise Exception("Error build MA task {}.".format(task))

	def check_size(self, page_size, stack_size):
		self.text_sizes = {}
		self.data_sizes = {}
		self.bss_sizes	= {}
		self.entry_points = {}

		for task in self.unique_tasks:
			path = "{}/{}/{}.elf".format(self.base_path, task, task)

			out = check_output(["riscv64-elf-size", path]).split(b'\n')[1].split(b'\t')

			self.data_sizes[task] = int(out[1])
			self.text_sizes[task] = self.__get_txt_size(task)*4 - self.data_sizes[task]
			self.bss_sizes[task] = int(out[2])

			out = check_output(["riscv64-elf-readelf", path, "-h"]).split(b'\n')[10].split(b' ')[-1]
			self.entry_points[task] = int(out, 16)
			
		print("\n********************* MA page size report *********************")
		for task in self.unique_tasks:
			size = self.text_sizes[task] + self.data_sizes[task] + self.bss_sizes[task] + stack_size
			if size <= page_size:
				print("Task {} memory usage {}/{} bytes".format(task.rjust(25), str(size).rjust(6), str(page_size).ljust(6)))
			else:
				raise Exception("Task {} memory usage of {} is bigger than page size of {}".format(task, size, page_size))
		print("******************* End MA page size report *******************")

	def generate_repo(self, scenario_path):
		for task in self.unique_tasks:
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
			repo.write_debug("{}/{}_debug.txt".format(self.base_path, task))

	def generate_start(self, scenario_path):
		start = Start()

		for task in self.tasks:
			name = task["task"]
			start.add(name, "Task name")

			try:
				address = task["static_mapping"]
				addr_x = int(address[0])
				addr_y = int(address[1])
				address = addr_x << 8 | addr_y
				map_comment = "statically mapped to PE {}x{}".format(addr_x, addr_y)
				start.add(str(address), "Task {} is {}".format(name, map_comment))
			except:
				raise Exception("All management tasks must be STATICALLY MAPPED")			
		
		start.write("{}/ma_start.txt".format(scenario_path))
		start.write_debug("{}/ma_start_debug.txt".format(scenario_path))

	def get_tasks(self):
		return self.task_names

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
