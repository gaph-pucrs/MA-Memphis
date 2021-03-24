#!/usr/bin/env python2
import sys
import math
import os
import commands
import multiprocessing
from yaml_intf import *
from build_utils import *

## @package app_builder
#This scripts compiles the application source code of the testcase and generates the repository and appstart text files

#When defined, a main function must be called in the last line of the script, take a look at the end of file
def main():
	
	MEMPHIS_PATH = os.getenv("MEMPHIS_PATH", 0)
	MEMPHIS_HOME = os.getenv("MEMPHIS_HOME", 0)
	
	if MEMPHIS_PATH == 0:
		sys.exit("ENV PATH ERROR: MEMPHIS_PATH not defined")
	if MEMPHIS_HOME == 0:
		print "WARNING: MEMPHIS_HOME not defined, using as default testcase dir MEMPHIS_PATH/testcases"
		MEMPHIS_HOME = MEMPHIS_PATH + "/testcases"
		
	INPUT_TESTCASE_FILE_PATH    = sys.argv[1]
	
	#Extracts the testcase_dir:
	path_list       = INPUT_TESTCASE_FILE_PATH.split("/") #The testcase path can have severals '/' into its composition
	input_yaml_name = path_list[len(path_list)-1] #Gets the last element of the split list
	TESTCASE_PATH   = MEMPHIS_HOME + "/" + input_yaml_name.split(".")[0] #Creates a new path using the MEMPHIS_HOME + the YAML name
	  
	APP_PATH        = TESTCASE_PATH + "/management"
	
	#Gets the page size from the testcase file
	yaml_r = get_yaml_reader(INPUT_TESTCASE_FILE_PATH)
	page_size_KB = get_page_size_KB(yaml_r)
	page_size_bytes = get_page_size_KB(yaml_r) * 1024
	ma_task_list = get_ma_task_list(yaml_r)
	ma_id_list = get_ma_id_list(yaml_r)

	#Generate the APP id file
	generate_apps_id(APP_PATH, ma_id_list)

	#Copies the app make from MEMPHIS_PATH to the application folder
	copy_app_make(MEMPHIS_PATH, APP_PATH, page_size_KB, ma_task_list)
	NCPU = multiprocessing.cpu_count()
	#Compiles the application
	exit_status = os.system("cd "+APP_PATH+"; make "+"-j"+str(NCPU))

	if exit_status != 0:
		sys.exit("\nError compiling applications' source code\n");
	
	#Generate the repository.txt and repository_debug.txt files for each application
	generate_repository(yaml_r, TESTCASE_PATH, APP_PATH)
	
def copy_app_make(memphis_path, app_path, page_size_KB, ma_task_list):
	
	makes_source = memphis_path+"/build_env/makes/make_ma"
	
	for task in ma_task_list:
		make_app_path = app_path+"/"+task+"/makefile"
		copyfile(makes_source, make_app_path)
		line = "TARGET="+task+"\n"
		line += "PAGE_SP_INIT="+str((page_size_KB*1024) - 1)+"\n"
		prepend_line(make_app_path, line)

def indices(lst, element):
	result = []
	offset = -1
	while True:
		try:
			offset = lst.index(element, offset+1)
		except ValueError:
			return result
		result.append(offset)

def generate_apps_id(app_path, ma_task_list):
	
	file_lines = []
	file_lines.append("#pragma once\n\n")

	sizes = []
	tasks = []
	ids = []

	for task in ma_task_list:
		if task in tasks:
			continue

		tasks.append(task)
		sizes.append(ma_task_list.count(task))
		ids.append(indices(ma_task_list, task))

	task_id = 0
	for task in tasks:
		file_lines.append("#define "+task+"_size"+" "+str(sizes[task_id])+"\n")
		file_lines.append("static const int "+task+"[] = {")
		for id_seq in ids[task_id]:
			file_lines.append(str(id_seq)+",")
		task_id = task_id + 1
		file_lines.append("};\n")

	task_id_file_path = app_path + "/id_tasks.h"
	
	#Use this function to create any file into testcase, it automatically only updates the old file if necessary
	writes_file_into_testcase(task_id_file_path, file_lines)

#Function that generates a new repository from the dir: /applications
#Please, behold this following peace of art:     
def generate_repository(yaml_r, testcase_dir, app_path):
	print "\n***************** Task page size report ***********************"
	
	task_name_list = get_ma_task_list(yaml_r)
	
	app_tasks_number = len(task_name_list)
	
	######################################################################################
	######### Check if task number is compatible to kernel max task number ###############
	######################################################################################
	check_application_task_number(testcase_dir, app_tasks_number, "management")
	######################################################################################
		
	#repolines is a list of RepoLine objetcts, the ideia is keep a list generic to be easily converted to any systax
	repo_lines = []
	
	# Walk for all tasks into /management dir to fills the task headers
	for task_name in task_name_list:
		txt_size = get_task_txt_size(app_path, task_name)

		task_reader = get_yaml_reader(app_path + "/" + task_name + "/config.yaml")
		task_type_tag = get_task_type_tag(task_reader, task_name)
		
		#Points next_free_address to a next free repo space after considering the task code of <task_name>.txt
		repo_lines.append(RepoLine(toX(task_type_tag), "task type tag"))
		repo_lines.append(RepoLine(toX(txt_size), "txt size") )
		repo_lines.append(RepoLine(toX(get_task_DATA_size(app_path, task_name)), "data size") )
		repo_lines.append(RepoLine(toX(get_task_BSS_size(app_path, task_name)), "bss size") )

		txt_source_file = app_path + "/" + task_name +"/"+task_name+".txt"
		bin_source_file = app_path + "/" + task_name +"/"+task_name+".elf"
		check_mem_size(bin_source_file, get_page_size_KB(yaml_r) )
		
		comment = task_name

		task_txt_file = open(txt_source_file, "r")
		
		for line in task_txt_file:
			file_line = line[0:len(line)-1] # removes the \n from end of file
			repo_lines.append( RepoLine(file_line, comment) )
			comment = ""
				
		task_txt_file.close()
		generate_task_repository_file(task_name, app_path, repo_lines, get_model_description(yaml_r))

		repo_lines = []
	
	################Finally, generates the repository file (main and debug files) ##########################
	print "***************** End task page size report ********************\n"


def get_task_type_tag(reader, task):
	ttt = 0
	try:
		act = reader["act"]
		ttt |= 0x02

		for capability in act:
			if capability == "migration":
				ttt |= 0x01000000
			else:
				print "Management task "+ task +": unknown act capability '"+capability+"'\n"
	except:
		pass

	try:
		decide = reader["decide"]
		ttt |= 0x04

		for capability in decide:
			if capability == "qos":
				ttt |= 0x010000
			else:
				print "Management task "+ task +": unknown decide capability '"+capability+"'\n"
	except:
		pass

	try:
		observe = reader["observe"]
		ttt |= 0x04

		for capability in observe:
			if capability == "qos":
				ttt |= 0x0100
			else:
				print "Management task "+ task +": unknown observe capability '"+capability+"'\n"
	except:
		pass

	# print "Management task " + task + " ttt = " + str(ttt) + "\n"
	return ttt

#Receives a int, convert to string and fills to a 32 bits word
def toX(input):
	hex_string = "%x" % input
	#http://stackoverflow.com/questions/339007/nicest-way-to-pad-zeroes-to-string
	return hex_string.zfill(8) # 8 is the lenght of chars to represent 32 bits (repo word) in hexa

#This fucntion receives the repo_lines filled above and generates two files: the repository itself and a repository_debug
def generate_task_repository_file(task_name, app_path, repo_lines, system_model_description):    
	
	repo_file_path = app_path +"/"+ task_name + "/repository.txt"
	repo_debug_file_path = app_path +"/"+ task_name + "/repository_debug.txt"
	
	file_lines = []
	file_debug_lines = []
	
	address = 0
	
	for repo_obj in repo_lines:
		
		file_lines.append(repo_obj.hex_string+"\n")
		
		debug_line = hex(address)+"\t\t0x"+repo_obj.hex_string+"\t"+repo_obj.commentary+"\n"
		
		address = address + 4
		
		file_debug_lines.append(debug_line)
		
	writes_file_into_testcase(repo_file_path, file_lines)
	writes_file_into_testcase(repo_debug_file_path, file_debug_lines)

def get_task_txt_size(app_path, task_name):
	
	source_file = app_path + "/" + task_name + "/"+task_name+".txt"
	
	#http://stackoverflow.com/questions/845058/how-to-get-line-count-cheaply-in-python/1019572#1019572
	num_lines = sum(1 for line in open(source_file))
	
	return num_lines
	
def get_task_DATA_size(app_path, task_name):
	
	source_file = app_path + "/" + task_name +"/"+task_name+".elf"
	
	#This command gets the bss data and size of the binary file
	#https://www.quora.com/What-is-a-convenient-way-to-execute-a-shell-command-in-Python-and-retrieve-its-output
	data_size = int (commands.getoutput("mips-elf-size "+source_file+" | tail -1 | sed 's/ //g' | sed 's/\t/:/g' | cut -d':' -f2"))
	
	while data_size % 4 != 0:
		data_size = data_size + 1
		
	data_size = data_size / 4
	
	return data_size
	
def get_task_BSS_size(app_path, task_name):
	
	source_file = app_path + "/" + task_name +"/"+task_name+ ".elf"
	
	#This command gets the bss data and size of the binary file
	#https://www.quora.com/What-is-a-convenient-way-to-execute-a-shell-command-in-Python-and-retrieve-its-output
	bss_size = int(commands.getoutput("mips-elf-size "+source_file+" | tail -1 | sed 's/ //g' | sed 's/\t/:/g' | cut -d':' -f3"))
	
	while bss_size % 4 != 0:
		bss_size = bss_size + 1
		
	bss_size = bss_size / 4
	
	return bss_size

# This fucntion copies the source files in source_dir to target_dir
#If you desire to add especific copies test, for example, ignore some specific files names or extensions, 
#please includes those file name or extension into the 3rd argument (ignored_names_list), the name can be the file name or its extension
def  generic_copy(source_dir, target_dir, ignored_extension_list):
	
	exclude_extensions = " --exclude=".join(ignored_extension_list)
	
	command_string = "rsync -u -r --exclude="+exclude_extensions+" "+source_dir+"/ "+target_dir+"/"
	
	status = os.system(command_string) 
	
def append_lines_at_end_of_file(file_path, lines):
	
	f = open(file_path, "a")
	
	f.writelines(lines)
	
	f.close()

# https://stackoverflow.com/a/4454598
def prepend_line(file_path, lines):
	with file(file_path, 'r') as original: data = original.read()
	with file(file_path, 'w') as modified: modified.write(lines+data)

def get_task_comp_load(app_name, task_name):
	return 0 #TO DO

def get_task_dependence_list(app_name, task_name):
	return [] #TO DO

#This function compares the number of tasks of the application with the number of the MAXIMUM task for one application that is supported by the kernel
def check_application_task_number(testcase_dir, app_task_number, app_name):
	
	kernel_max_app = 0
	
	#Useful function that serach a value of a #define is for a given file_path
	with open(testcase_dir+"/software/kernel/pkg.h") as search:
		for line in search:
			line = line.rstrip()  # remove '\n' at end of line
			if "#define PKG_MAX_TASKS_APP" in line:
				kernel_max_app = int(line.split(" ")[2])
				break
	if kernel_max_app == 0:
		sys.exit("ERROR in app_builder: impossible to determine the max number of application task\n")
	
	if app_task_number > kernel_max_app:
		print "ERROR: Applicaiton: "+app_name+" has "+str(app_task_number)+" tasks but kernel support MAX_TASKS_APP of "+str(kernel_max_app)+". Please change this in testcase yaml file\n"
		sys.exit("\nMAX_TASKS_APP boundary exceeded\n")

main()