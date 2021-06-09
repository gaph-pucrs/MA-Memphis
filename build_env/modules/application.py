#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from os import listdir, makedirs
from subprocess import run
from multiprocessing import cpu_count
from repository import Repository
from yaml import safe_load

class Application:
	def __init__(self, app_name, platform_path, testcase_path):
		self.app_name = app_name
		self.platform_path = platform_path
		self.testcase_path = testcase_path

		files = listdir(self.platform_path+"/applications/"+self.app_name)

		self.tasks = []
		for file in files:
			if file.endswith(".c"):
				task = file.split(".")[0]
				self.tasks.append(task)

		self.tasks.sort()

		self.communication = {}
		try:
			self.communication = safe_load(open("{}/applications/{}/config.yaml".format(self.platform_path, self.app_name)))["communication"]
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
		run(["make", "-C", self.testcase_path+"/applications/"+self.app_name, "-j", str(NCPU)])

	def generate_repo(self, scenario_path):
		repo = Repository()

		repo.add(len(self.tasks), "Number of tasks of application {}".format(self.app_name))

		dep_list, dep_list_len = self.__get_dep_list()

		address = 4 + 4*len(self.tasks)*4 + dep_list_len*4
		for task in self.tasks:
			txt_size = self.__get_txt_size(task)
			repo.add(txt_size, "txt size")
			repo.add(0, "data size")
			repo.add(0, "bss size")
			repo.add(address, "Repository address")

			address += txt_size*4

		for t in range(len(self.tasks)):
			for c in range(len(dep_list[t])):
					consumer = dep_list[t][c] + 1
					if c == len(dep_list[t]) - 1:
						consumer *= -1	
					repo.add(consumer, "Task {} is producer of task {}".format(self.tasks[t], self.tasks[dep_list[t][c]]))

			if len(dep_list[t]) == 0:
				repo.add(0, "Task {} has no consumers".format(self.tasks[t]))

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
			sucessors = []

			consumers = {}
			try:
				consumers = self.communication[task]["consumers"]
			except:
				pass
			
			for consumer in consumers:
				sucessors.append(self.tasks.index(consumer))

			dep_list.append(sucessors)
			dep_list_len += len(sucessors)
			if len(sucessors) == 0:
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
