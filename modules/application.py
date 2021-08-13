#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from os import listdir, makedirs
from subprocess import run, check_output
from multiprocessing import cpu_count
from repository import Repository
from yaml import safe_load

class Application:
	def __init__(self, app_name, platform_path, testcase_path):
		self.app_name = app_name
		self.platform_path = platform_path
		self.testcase_path = testcase_path
		try:
			files = listdir(self.platform_path+"/applications/"+self.app_name)
		except:
			raise Exception("Application {} not found or has 0 tasks.".format(app_name))

		self.tasks = []
		for file in files:
			if file.endswith(".c"):
				task = file.split(".")[0]
				self.tasks.append(task)

		self.tasks.sort()

		self.ctg = {}
		try:
			self.ctg = safe_load(open("{}/applications/{}/config.yaml".format(self.platform_path, self.app_name)))["ctg"]
		except:
			pass

		self.tasks.sort()

	def copy(self):
		makedirs(self.testcase_path+"/applications", exist_ok=True)
		copy_tree(self.platform_path+"/applications/"+self.app_name, self.testcase_path+"/applications/"+self.app_name, update=1)
		self.generate_ids()

	def generate_ids(self):
		ids = ApplicationIds()

		ids.add(self.tasks)

		ids.write(self.testcase_path+"/applications/"+self.app_name+"/id_tasks.h")

	def build(self):
		NCPU = cpu_count()
		make = run(["make", "-C", self.testcase_path+"/applications/"+self.app_name, "-j", str(NCPU)])
		if make.returncode != 0:
			raise Exception("Error build application {}.".format(self.app_name))

	def check_count(self, max_tasks_app):
		if len(self.tasks) > max_tasks_app:
			raise Exception("Number of management tasks exceeds the maximum allowed (max_tasks_app).")

	def check_size(self, page_size, stack_size):
		self.text_sizes = {}
		self.data_sizes = {}
		self.bss_sizes	= {}

		for task in self.tasks:
			path = "{}/applications/{}/{}.elf".format(self.testcase_path, self.app_name, task)

			out = check_output(["mips-elf-size", path]).split(b'\n')[1].split(b'\t')

			self.data_sizes[task] = int(out[1])
			self.text_sizes[task] = self.__get_txt_size(task)*4 - self.data_sizes[task]
			self.bss_sizes[task] = int(out[2])

			
		print("\n************ {} page size report ************".format(self.app_name.center(20)))
		for task in self.tasks:
			size = self.text_sizes[task] + self.data_sizes[task] + self.bss_sizes[task] + stack_size
			if size <= page_size:
				print("Task {} memory usage {}/{} bytes".format(task.rjust(25), str(size).rjust(6), str(page_size).ljust(6)))
			else:
				raise Exception("Task {} memory usage of {} is bigger than page size of {}".format(task, size, page_size))
		print("********** End {} page size report **********".format(self.app_name.center(20)))

	def generate_repo(self, scenario_path):
		repo = Repository()

		repo.add(len(self.tasks), "Number of tasks of application {}".format(self.app_name))

		dep_list, dep_list_len = self.__get_dep_list()

		address = 4 + 4*len(self.tasks)*4 + dep_list_len*4
		for task in self.tasks:
			repo.add(self.text_sizes[task], "txt size")
			repo.add(self.data_sizes[task], "data size")
			repo.add(self.bss_sizes[task], "bss size")
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
			task_hex = open(self.testcase_path+"/applications/"+self.app_name+"/"+task+".txt", "r")

			for line in task_hex:
			 	repo.add(int(line, 16), "")

			task_hex.close()

		repo.write(scenario_path+"/applications/"+self.app_name+".txt")
		repo.write_debug(scenario_path+"/applications/"+self.app_name+"_debug.txt")

	def get_tasks(self):
		return self.tasks

	def __get_txt_size(self, task):
		return sum(1 for line in open(self.testcase_path+"/applications/"+self.app_name+"/"+task+".txt"))

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
