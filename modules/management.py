#!/usr/bin/env python3
from distutils.dir_util import copy_tree
from distutils.file_util import copy_file
from os import makedirs
from subprocess import run, check_output
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

		if output_tasks[0] != "mapper_task":
			raise Exception("Mapper task must be the first in the management list")

		ids.write(self.testcase_path+"/management/id_tasks.h")

	def build(self):
		NCPU = cpu_count()
		for task in self.unique_tasks:
			make = run(["make", "-C", self.testcase_path+"/management/"+task, "-j", str(NCPU)])
			if make.returncode != 0:
				raise Exception("Error build MA task {}.".format(task))

	def check_count(self, max_tasks_app):
		if len(self.tasks) > max_tasks_app:
			raise Exception("Number of management tasks exceeds the maximum allowed (max_tasks_app).")

	def check_size(self, page_size, stack_size):
		self.text_sizes = {}
		self.data_sizes = {}
		self.bss_sizes	= {}

		for task in self.unique_tasks:
			path = "{}/management/{}/{}.elf".format(self.testcase_path, task, task)

			out = check_output(["mips-elf-size", path]).split(b'\n')[1].split(b'\t')

			self.data_sizes[task] = int(out[1])
			self.text_sizes[task] = self.__get_txt_size(task)*4 - self.data_sizes[task]
			self.bss_sizes[task] = int(out[2])

			
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

			descr = Descriptor(self.testcase_path+"/management/"+task+"/config.yaml", task)

			task_type = descr.get_type()
			repo.add(task_type, "Task type tag")

			repo.add(self.text_sizes[task], "txt size")
			repo.add(self.data_sizes[task], "data size")
			repo.add(self.bss_sizes[task], "bss size")

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

			try:
				address = task["static_mapping"]
				addr_x = int(address[0])
				addr_y = int(address[1])
				address = addr_x << 8 | addr_y
				map_comment = "statically mapped to PE {}x{}".format(addr_x, addr_y)
				start.add(str(address), "Task {} is {}".format(name, map_comment))
			except:
				raise Exception("All management tasks must be STATICALLY MAPPED")			
		
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
