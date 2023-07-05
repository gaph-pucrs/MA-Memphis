#!/usr/bin/env python3
from yaml import safe_load
from os import makedirs, listdir
from distutils.dir_util import remove_tree
from shutil import copyfile
from repository import Start
from os.path import exists
from filecmp import cmp

class Scenario:
	def __init__(self, platform_path, testcase_path, scenario_file):
		self.platform_path = platform_path
		self.testcase_path = testcase_path

		self.base = scenario_file

		name = self.base.split(".yaml")[0].split("/")
		name = name[len(name) - 1]

		self.base_dir = "{}/{}".format(self.testcase_path, name)
		self.file = "{}/{}.yaml".format(self.base_dir, name)

		yaml = safe_load(open(self.base, "r"))

		tc_name = self.testcase_path.split("/")
		tc_name = tc_name[len(tc_name) - 1]

		tc_yaml = safe_load(open("{}/{}.yaml".format(self.testcase_path, tc_name), "r"))
		self.page_size		= tc_yaml["hw"]["page_size_KB"]*1024

		self.ma_tasks = []

		for task in yaml["management"]:
			self.ma_tasks.append(task)

		self.applications = []
		for app in yaml["apps"]:
			app_name = app["name"]
			app_inst = app["instance"]
			try:
				start_ms = app["start_time_ms"]
			except:
				print("Using 0 ms as starting time for app {}".format(app_name))
				start_ms = 0
			try:
				mapping = app["static_mapping"]
			except:
				print("Using dynamic mapping for all tasks of app {}".format(app_name))
				mapping = []
			app_scen = ("{}_{}".format(app_name, app_inst), start_ms, mapping)
			self.applications.append(app_scen)

		self.applications.sort(key=lambda x: x[1])
				
	def copy(self, skipdebug):
		if self.__is_obsolete():
			remove_tree(self.base_dir)

		makedirs(self.base_dir+"/debug/pipe", exist_ok=True)
		makedirs(self.base_dir+"/debug/request", exist_ok=True)
		makedirs(self.base_dir+"/log", exist_ok=True)
		makedirs(self.base_dir+"/flit_sniffer", exist_ok=True)

		open("{}/debug/traffic_router.txt".format(self.base_dir), "w").close()
		copyfile(self.base, self.file)

		if not skipdebug:
			copyfile("{}/services.cfg".format(self.testcase_path), "{}/debug/services.cfg".format(self.base_dir))
			copyfile("{}/cpu.cfg".format(self.testcase_path), "{}/debug/cpu.cfg".format(self.base_dir))
			copyfile("{}/platform.cfg".format(self.testcase_path), "{}/debug/platform.cfg".format(self.base_dir))
			self.__append_platform()

	def build(self, repodebug):
		self.__generate_ma_start(repodebug)
		self.__generate_app_start(repodebug)

	def __generate_app_start(self, repodebug):
		start = Start()

		for app in self.applications:
			start.add(app[0], "App name")
			
			start.add(app[1], "Start time (ms)")

			source_path = "{}/applications/{}".format(self.testcase_path, app[0])

			try:
				files = listdir(source_path)
			except:
				raise Exception("Application {} not found or has 0 tasks.".format(app[0]))

			tasks = []
			for file in files:
				if file.endswith(".c"):
					task = file.split(".")[0]
					tasks.append(task)

			tasks.sort()
			task_cnt = len(tasks)
			start.add(task_cnt, "Number of tasks")

			for task in app[2]:
				if task not in tasks:
					raise Exception("Task {} in static_mapping list not present in application {}".format(task, app.app_name))
		
			for task in tasks:
				address = -1
				map_comment = ""
				if task in app[2]:
					addr_x = app[2][task][0]
					addr_y = app[2][task][1]
					address = addr_x << 8 | addr_y
					map_comment = "statically mapped to PE {}x{}".format(addr_x, addr_y)
				else:
					map_comment = "dynamically mapped"
				
				start.add(str(address), "Task {} is {}".format(task, map_comment))
		
		start.write(self.base_dir+"/app_start.txt")
		if repodebug:
			start.write_debug(self.base_dir+"/app_start_debug.txt")

	def __generate_ma_start(self, repodebug):
		start = Start()

		for task in self.ma_tasks:
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
		
		start.write("{}/ma_start.txt".format(self.base_dir))
		if repodebug:
			start.write_debug("{}/ma_start_debug.txt".format(self.base_dir))

	def __append_platform(self):
		task_lines = []
		app_lines = []

		app_id = 1

		for app in self.applications:
			app_lines.append("{}\t{}\n".format(app[0], app_id))
			
			source_path = "{}/applications/{}".format(self.testcase_path, app[0])

			try:
				files = listdir(source_path)
			except:
				raise Exception("Application {} not found or has 0 tasks.".format(app[0]))
			
			tasks = []
			for file in files:
				if file.endswith(".c"):
					task = file.split(".")[0]
					tasks.append(task)

			tasks.sort()
			
			for t in range(len(tasks)):
				task_lines.append("{} {}\n".format(tasks[t], app_id << 8 | t))

			app_id += 1


		cfg = open("{}/debug/platform.cfg".format(self.base_dir), "a")

		cfg.write("BEGIN_task_name_relation\n")

		for t in range(len(self.ma_tasks)):
			cfg.write("{} {}\n".format(self.ma_tasks[t]["task"], t))

		cfg.writelines(task_lines)
		cfg.write("END_task_name_relation\n")

		cfg.write("BEGIN_app_name_relation\n")

		cfg.write("management\t0\n")
		cfg.writelines(app_lines)

		cfg.write("END_app_name_relation\n")

		cfg.close()
  
	def __is_obsolete(self):
		if exists(self.file):
			if not cmp(self.base, self.file):
				return True
		
		return False
