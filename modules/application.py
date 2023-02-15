#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from os import listdir, environ, getcwd
from subprocess import run, check_output
from multiprocessing import cpu_count
from repository import Repository
from yaml import safe_load

class Application:
	def __init__(self, app, app_name, instance, platform_path, scenario_path, testcase_path):
		self.app_name = app_name
		self.testcase_path = testcase_path

		try:
			self.start_ms = app["start_time_ms"]
		except:
			print("Using 0 ms as starting time for app {}".format(self.app_name))
			self.start_ms = 0

		try:
			self.definitions = app["definitions"]
		except:
			self.definitions = []
   
		try:
			self.mapping = app["static_mapping"]
		except:
			print("Using pure dinamic mapping for app {}".format(self.app_name))
			self.mapping = []

		self.instance = instance
		self.source_path = "{}/applications/{}".format(platform_path, self.app_name)
		self.base_path = "{}/applications".format(scenario_path)
		self.app_path = "{}_{}".format(self.app_name, instance)

		try:
			files = listdir(self.source_path)
		except:
			raise Exception("Application {} not found or has 0 tasks.".format(self.app_name))

		self.tasks = []
		for file in files:
			if file.endswith(".c"):
				task = file.split(".")[0]
				self.tasks.append(task)

		self.tasks.sort()

		self.ctg = {}
		try:
			self.ctg = safe_load(open("{}/config.yaml".format(self.source_path)))["ctg"]
		except:
			print("CTG for app {} not provided".format(self.app_name))
			pass

	def copy(self):
		copy_tree(self.source_path, "{}/{}".format(self.base_path, self.app_path), update=1)
		self.generate_ids()

	def generate_ids(self):
		ids = ApplicationIds()

		ids.add(self.tasks)

		ids.write("{}/{}/id_tasks.h".format(self.base_path, self.app_path))

	def build(self):
		NCPU = cpu_count()
  
		# CFLAGS = ""
  
		# for definition in self.definitions:
			# CFLAGS = CFLAGS + "-D"+str(list(definition.keys())[0])+"="+str(list(definition.values())[0])+" "
  
		make_env = environ.copy()
		# make_env["CFLAGS"] = CFLAGS
		try:
			inc_path = make_env["C_INCLUDE_PATH"]
		except:
			inc_path = ""

		CFLAGS = ""
		for definition in self.definitions:
			CFLAGS += "-D"+str(list(definition.keys())[0])+"="+str(list(definition.values())[0])+" "

		make_env["C_INCLUDE_PATH"] = "{}/{}/include:{}".format(getcwd(), self.testcase_path, inc_path)
		make_env["CFLAGS"] = CFLAGS+"--include id_tasks.h"
		

		make_env["LDFLAGS"] = "-L{}/{}/lib --specs=nano.specs -Wl,-Ttext=0 -u _getpid".format(getcwd(), self.testcase_path)
  
		# print(make_env)
		make = run(["make", "-C", "{}/{}".format(self.base_path, self.app_path), "-j", str(NCPU)], env=make_env)
		if make.returncode != 0:
			raise Exception("Error building application {}.".format(self.app_name))

	def check_size(self, page_size, stack_size):
		self.text_sizes = {}
		self.data_sizes = {}
		self.bss_sizes	= {}
		self.entry_points = {}

		for task in self.tasks:
			path = "{}/{}/{}.elf".format(self.base_path, self.app_path, task)

			out = check_output(["riscv64-elf-size", path]).split(b'\n')[1].split(b'\t')

			self.data_sizes[task] = int(out[1])
			self.text_sizes[task] = self.__get_txt_size(task)*4 - self.data_sizes[task]
			self.bss_sizes[task] = int(out[2])

			out = check_output(["riscv64-elf-readelf", path, "-h"]).split(b'\n')[10].split(b' ')[-1]
			self.entry_points[task] = int(out, 16)

			
		print("\n************ {} page size report ************".format(self.app_name.center(20)))
		for task in self.tasks:
			size = self.text_sizes[task] + self.data_sizes[task] + self.bss_sizes[task] + stack_size
			if size <= page_size:
				print("Task {} memory usage {}/{} bytes".format(task.rjust(25), str(size).rjust(6), str(page_size).ljust(6)))
			else:
				raise Exception("Task {} memory usage of {} is bigger than page size of {}".format(task, size, page_size))
		print("********** End {} page size report **********".format(self.app_name.center(20)))

	def generate_repo(self):
		repo = Repository()

		repo.add(len(self.tasks), "Number of tasks of application {}".format(self.app_name))

		dep_list, dep_list_len = self.__get_dep_list()

		# First line is the number of tasks
		# Then, each task has 5 lines (txt, data, bss, entry, repo)
		# Finally, add the length of the dependence list
		address = 4 + 5*len(self.tasks)*4 + dep_list_len*4

		for task in self.tasks:
			repo.add(self.text_sizes[task], "txt size")
			repo.add(self.data_sizes[task], "data size")
			repo.add(self.bss_sizes[task], "bss size")
			repo.add(self.entry_points[task], "entry point")
			repo.add(address, "Repository address")

			address += self.text_sizes[task] + self.data_sizes[task]

		for t in range(len(self.tasks)):
			for c in range(len(dep_list[t])):
					successor = dep_list[t][c] + 1
					if c == len(dep_list[t]) - 1:
						successor *= -1	
					repo.add(successor, "Task {} is successor of task {}".format(self.tasks[dep_list[t][c]], self.tasks[t]))

			if len(dep_list[t]) == 0:
				repo.add(0, "Task {} has no successors".format(self.tasks[t]))

		for task in self.tasks:
			task_hex = open("{}/{}/{}.txt".format(self.base_path, self.app_path, task), "r")

			for line in task_hex:
				repo.add(int(line, 16), "")
	
			task_hex.close()

		repo.write("{}/{}.txt".format(self.base_path, self.get_full_name()))
		repo.write_debug("{}/{}_debug.txt".format(self.base_path, self.get_full_name()))

	def get_tasks(self):
		return self.tasks

	def get_full_name(self):
		return "{}_{}".format(self.app_name, self.instance)

	def __get_txt_size(self, task):
		return sum(1 for line in open("{}/{}/{}.txt".format(self.base_path, self.app_path, task)))

	def __get_dep_list(self):
		dep_list = []

		dep_list_len = 0
		for task in self.tasks:
			successors_list = []

			successors = {}
			try:
				successors = self.ctg[task]["successors"]
			except:
				pass
			
			for successor in successors:
				successors_list.append(self.tasks.index(successor))

			successors_list.sort()

			dep_list.append(successors_list)
			dep_list_len += len(successors_list)
			if len(successors_list) == 0:
				dep_list_len += 1

		return dep_list, dep_list_len

class ApplicationIds:
	def __init__(self):
		self.lines = []
		self.lines.append("#pragma once\n")

	def add(self, tasks):
		for id_seq in range(len(tasks)):
			self.lines.append("#define\t"+tasks[id_seq]+"\t"+str(id_seq)+"\n")

	def write(self, path):
		file = open(path, "w")
		file.writelines(self.lines)
		file.close()
