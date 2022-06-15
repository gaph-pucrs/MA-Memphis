#!/usr/bin/env python3
from yaml import safe_load
from os import makedirs
from distutils.dir_util import copy_tree
from shutil import copyfile, copy
from management import Management
from application import Application
from repository import Start

class Scenario:
	def __init__(self, platform_path, testcase_path, scenario_file):
		self.platform_path = platform_path
		self.testcase_path = testcase_path

		self.base = scenario_file

		name = self.base.split(".yaml")[0].split("/")
		name = name[len(name) - 1]

		self.base_dir = self.testcase_path+"/"+name
		self.file = self.base_dir+"/"+name+".yaml"

		yaml = safe_load(open(self.base, "r"))

		tc_name = self.testcase_path.split("/")
		tc_name = tc_name[len(tc_name) - 1]

		tc_yaml = safe_load(open("{}/{}.yaml".format(self.testcase_path, tc_name), "r"))
		self.max_tasks_app	= tc_yaml["sw"]["max_tasks_app"]
		self.page_size		= tc_yaml["hw"]["page_size_KB"]*1024
		# self.stack_size		= tc_yaml["hw"]["stack_size"]

		self.management = Management(yaml["management"], self.platform_path, self.testcase_path)

		app_names = []
		self.app_instances = []
		self.applications = {}

		try:
			for app in yaml["apps"]:
				app_names.append(app["name"])

				start_time_ms = 0
				try:
					start_time_ms = int(app["start_time_ms"])
				except:
					pass

				self.app_instances.append((app, start_time_ms))

			self.app_instances.sort(key=lambda x: x[1])

			app_names = set(app_names)
		except:
			pass
		
		for app in app_names:
			self.applications[app] = Application(app, self.platform_path, self.testcase_path)
		
	def copy(self):
		makedirs(self.base_dir+"/debug/pipe", exist_ok=True)
		makedirs(self.base_dir+"/debug/request", exist_ok=True)
		makedirs(self.base_dir+"/log", exist_ok=True)
		makedirs(self.base_dir+"/management", exist_ok=True)
		makedirs(self.base_dir+"/applications", exist_ok=True)

		copy("{}/hardware/memphis".format(self.testcase_path), "{}/memphis".format(self.base_dir))
		copyfile("{}/kernel/kernel.txt".format(self.testcase_path), "{}/bootrom.txt".format(self.base_dir))

		open("{}/debug/traffic_router.txt".format(self.base_dir), "w").close()
		copyfile("{}/services.cfg".format(self.testcase_path), "{}/debug/services.cfg".format(self.base_dir))
		copyfile("{}/cpu.cfg".format(self.testcase_path), "{}/debug/cpu.cfg".format(self.base_dir))
		copyfile("{}/platform.cfg".format(self.testcase_path), "{}/debug/platform.cfg".format(self.base_dir))
		self.__append_platform()

		self.management.copy()

		for app in self.applications:
			self.applications[app].copy()

	def build(self):
		self.management.build()
		for app in self.applications:
			self.applications[app].build()

		self.management.check_count(self.max_tasks_app)
		for app in self.applications:
			self.applications[app].check_count(self.max_tasks_app)

		# self.management.check_size(self.page_size, self.stack_size)
		self.management.check_size(self.page_size, 0)
		for app in self.applications:
			self.applications[app].check_size(self.page_size, 0)
			# self.applications[app].check_size(self.page_size, self.stack_size)

		self.management.generate_repo(self.base_dir)

		for app in self.applications:
			self.applications[app].generate_repo(self.base_dir)

		self.management.generate_start(self.base_dir)
		self.__generate_app_start()		

	def __generate_app_start(self):
		start = Start()

		for app in self.app_instances:
			name = app[0]["name"]
			start.add(name, "App name")
			
			start.add(app[1], "Start time (ms)")

			tasks = self.applications[name].get_tasks()
			task_cnt = len(tasks)
			start.add(task_cnt, "Number of tasks")

			static_mapping = []
			try:
				static_mapping = app[0]["static_mapping"]
			except:
				pass # This means that the application not has any task mapped statically

			for task in static_mapping:
				if task not in tasks:
					raise Exception("Task {} in static_mapping list not present in application {}".format(task, name))
		
			for task in tasks:
				address = -1
				map_comment = ""
				if task in static_mapping:
					addr_x = static_mapping[task][0]
					addr_y = static_mapping[task][1]
					address = addr_x << 8 | addr_y
					map_comment = "statically mapped to PE {}x{}".format(addr_x, addr_y)
				else:
					map_comment = "dinamically mapped"
				
				start.add(str(address), "Task {} is {}".format(task, map_comment))
		
		start.write(self.base_dir+"/app_start.txt")
		start.write_debug(self.base_dir+"/app_start_debug.txt")

	def __append_platform(self):

		task_lines = []
		app_lines = []

		app_id = 1

		for app in self.app_instances:
			name = app[0]["name"]
			app_lines.append("{}\t{}\n".format(name, app_id))
			
			tasks = self.applications[name].get_tasks()
			for t in range(len(tasks)):
				task_lines.append("{} {}\n".format(tasks[t], app_id << 8 | t))

			app_id += 1


		cfg = open("{}/debug/platform.cfg".format(self.base_dir), "a")

		cfg.write("BEGIN_task_name_relation\n")

		ma_tasks = self.management.get_tasks()
		for t in range(len(ma_tasks)):
			cfg.write("{} {}\n".format(ma_tasks[t], t))

		cfg.writelines(task_lines)
		cfg.write("END_task_name_relation\n")

		cfg.write("BEGIN_app_name_relation\n")

		cfg.write("management\t0\n")
		cfg.writelines(app_lines)

		cfg.write("END_app_name_relation\n")

		cfg.close()		
