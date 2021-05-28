#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from os import listdir, makedirs
from subprocess import run
from multiprocessing import cpu_count
from repository import Repository

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

		task_type = 0x01
		address = 7*len(self.tasks)*4 + 4
		for t in range(len(self.tasks)):
			repo.add(t, self.tasks[t])
			repo.add(0, "Mapping [Legacy]")
			repo.add(task_type, "Task type tag")
			
			txt_size = self.__get_txt_size(self.tasks[t])
			repo.add(txt_size, "txt size")
			repo.add(0, "data size [Legacy]")
			repo.add(0, "bss size [Legacy]")
			repo.add(address, "Initial address")

			address += txt_size*4

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
