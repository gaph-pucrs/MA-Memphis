#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from distutils.file_util import copy_file
from os import makedirs
from subprocess import run
from multiprocessing import cpu_count
from descriptor import Descriptor
from repository import Repository, Start

class Management:
	def __init__(self, management, platform_path, testcase_path):
		self.management = management
		self.platform_path = platform_path
		self.testcase_path = testcase_path

		self.tasks = []
		self.task_names = []
		self.unique_tasks = []
		for task in management:
			self.tasks.append(task)
			self.task_names.append(task["task"])
		
		self.unique_tasks = set(self.task_names)

	def copy(self):
		makedirs(self.testcase_path+"/management", exist_ok=True)
		copy_file(self.platform_path+"/management/tag.h", self.testcase_path+"/management/tag.h", update=1)

		for task in self.unique_tasks:
			copy_tree(self.platform_path+"/management/"+task, self.testcase_path+"/management/"+task, update=1)

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

		ids.write(self.testcase_path+"/management/id_tasks.h")

	def build(self):
		NCPU = cpu_count()
		for task in self.unique_tasks:
			run(["make", "-C", self.testcase_path+"/management/"+task, "-j", str(NCPU)])

	def generate_repo(self, scenario_path):
		for task in self.unique_tasks:
			repo = Repository()

			descr = Descriptor(self.testcase_path+"/management/"+task+"/config.yaml", task)

			task_type = descr.get_type()
			repo.add(task_type, "Task type tag")

			txt_size = self.__get_txt_size(task)
			repo.add(txt_size, "txt size")

			repo.add(0, "data size [Legacy]")
			repo.add(0, "bss size [Legacy]")

			task_hex = open(self.testcase_path+"/management/"+task+"/"+task+".txt", "r")

			for line in task_hex:
				repo.add(int(line, 16), "")

			task_hex.close()

			repo.write(scenario_path+"/management/"+task+".txt")
			repo.write_debug(scenario_path+"/management/"+task+"_debug.txt")

	def generate_start(self, scenario_path):
		start = Start()

		for task in self.tasks:
			name = task["task"]
			start.add(name, "Task name")

			address = -1
			map_comment = ""
			try:
				address = task["static_mapping"]
				addr_x = int(address[0])
				addr_y = int(address[1])
				address = addr_x << 8 | addr_y
				map_comment = "statically mapped to PE {}x{}".format(addr_x, addr_y)
			except:
				address = -1
				map_comment = "dinamically mapped"
			
			start.add(str(address), "Task {} is {}".format(name, map_comment))
		
		start.write(scenario_path+"/ma_start.txt")
		start.write_debug(scenario_path+"/ma_start_debug.txt")

	def get_tasks(self):
		return self.task_names

	def __get_txt_size(self, task):
		return sum(1 for line in open(self.testcase_path+"/management/"+task+"/"+task+".txt"))

	
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
