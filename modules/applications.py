#!/usr/bin/env python3
from yaml import safe_load
from os import makedirs
from distutils.dir_util import remove_tree
from shutil import copyfile
from application import Application
from os.path import exists
from filecmp import cmp

class Applications:
	def __init__(self, platform_path, testcase_path, applications_file):
		self.platform_path = platform_path
		self.testcase_path = testcase_path

		self.base = applications_file

		name = self.base.split(".yaml")[0].split("/")
		name = name[len(name) - 1]

		self.base_dir = "{}/applications".format(testcase_path)
		self.file = "{}/{}.yaml".format(self.base_dir, name)

		yaml = safe_load(open(self.base, "r"))

		self.applications = {}

		try:
			for app in yaml["apps"]:
				app_name = app["name"]
				app_inst = app["instance"]
				app_key = (app_name, app_inst)
				try:
					if self.applications[app_key] is not None:
						raise Exception("App {} has multiple {} instances".format(app_name, app_inst))
				except:
					self.applications[app_key] = Application(app, app_name, app_inst, self.platform_path, self.base_dir, self.testcase_path)
		except:
			pass

		tc_name = self.testcase_path.split("/")
		tc_name = tc_name[len(tc_name) - 1]

		tc_yaml = safe_load(open("{}/{}.yaml".format(self.testcase_path, tc_name), "r"))
		self.page_size		= tc_yaml["hw"]["page_size_KB"]*1024

	def copy(self):
		if self.__is_obsolete():
			remove_tree(self.base_dir)

		makedirs(self.base_dir, exist_ok=True)
		copyfile(self.base, self.file)

		for app in self.applications:
			self.applications[app].copy()

	def build(self, repodebug):
		for app in self.applications:
			self.applications[app].build()

		for app in self.applications:
			self.applications[app].check_size(self.page_size)

		for app in self.applications:
			self.applications[app].generate_repo(repodebug)

	def __is_obsolete(self):
		if exists(self.file):
			if not cmp(self.base, self.file):
				return True
		
		return False
