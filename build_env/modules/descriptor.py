#!/usr/bin/env python3
from yaml import safe_load

class Descriptor:
	def __init__(self, file, name):
		self.yaml = safe_load(open(file, "r"))
		self.name = name

	def get_type(self):
		ttt = 0

		try:
			act = self.yaml["act"]
			ttt |= 0x02

			for cap in act:
				if cap == "migration":
					ttt |= 0x01000000
				else:
					print("Management task {} unknown capability {}".format(self.name, cap))
		except:
			pass

		try:
			decide = self.yaml["decide"]
			ttt |= 0x04

			for cap in decide:
				if cap == "qos":
					ttt |= 0x010000
				else:
					print("Management task {} unknown capability {}".format(self.name, cap))
		except:
			pass

		try:
			observe = self.yaml["observe"]
			ttt |= 0x08

			for cap in observe:
				if cap == "qos":
					ttt |= 0x0100
				else:
					print("Management task {} unknown capability {}".format(self.name, cap))
		except:
			pass

		return ttt
		