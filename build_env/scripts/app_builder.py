#!/usr/bin/python
import sys
import math
import os
import commands
import Tkinter as tkinter
import tkMessageBox as mbox
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
    TESTCASE_NAME   = input_yaml_name.split(".")[0]
    
    APP_NAME        = sys.argv[2]   
    APP_PATH        = TESTCASE_PATH + "/applications/" + APP_NAME
    
    #Selects the application source code from the MEMPHIS_PATH or from MEMPHIS_HOME
    source_app_path = ""
    try:
        source_app_path = MEMPHIS_PATH + "/applications/" + APP_NAME
        if os.path.exists(APP_PATH) == False and os.path.exists(source_app_path) == False:
            raise Exception()
        elif os.path.exists(APP_PATH) == True:
            source_app_path = APP_PATH
        else:
            print "\nWARNING: App path not found at:\n-" + APP_PATH + "\nGetting app path from MEMPHIS_PATH="+source_app_path
    except:
        sys.exit("\n\nERROR: Invalid application name passed as 2nd argument: "+APP_NAME+"\nThe application was not found either in:\n- " + APP_PATH + "\n- "+ source_app_path)

    #Copies the application if it source path is not equal to the app path on testcase
    if (source_app_path != APP_PATH):
        generic_copy(source_app_path, APP_PATH, [".svn"])
    
    #Generate the APP id file
    generate_apps_id(APP_NAME, APP_PATH, TESTCASE_PATH)
    
    #Gets the page size from the testcase file
    yaml_r = get_yaml_reader(INPUT_TESTCASE_FILE_PATH)
    page_size_KB = get_page_size_KB(yaml_r)
    page_size_bytes = get_page_size_KB(yaml_r) * 1024
    
    #Copies the app make from MEMPHIS_PATH to the application folder
    copy_app_make(MEMPHIS_PATH, APP_PATH, page_size_KB)
    
    #Compiles the application
    exit_status = os.system("cd "+APP_PATH+"; make")

    if exit_status != 0:
        sys.exit("\nError compiling applications' source code\n");
    
    #Generate the repository.txt and repository_debug.txt files for each application
    generate_repository(yaml_r, TESTCASE_PATH, APP_PATH, APP_NAME);
    
def copy_app_make(memphis_path, app_path, page_size_KB):
    
    makes_source = memphis_path+"/build_env/makes/make_app"
    
    make_app_path = app_path+"/makefile"
    
    copyfile(makes_source, make_app_path)
    
    line = "PAGE_SP_INIT = "+ str((page_size_KB  *  1024) - 1) + "\n"
    
     #Append the PAGE_SP_INIT value
    append_lines_at_end_of_file(make_app_path, line)
        

def generate_apps_id(app_name, app_path, testacase_dir):
    
    task_id_file_path = app_path + "/id_tasks.h"
    
    file_lines = []
    
    task_name_list = get_app_task_name_list(testacase_dir, app_name)
    
    task_id = 0
    
    for task_name in task_name_list:
        file_lines.append("#define\t"+task_name+"\t"+str(task_id)+"\n")
        task_id = task_id + 1
    
    #Use this function to create any file into testcase, it automatically only updates the old file if necessary
    writes_file_into_testcase(task_id_file_path, file_lines)

#Function that generates a new repository from the dir: /applications
#Please, behold this following peace of art:     
def generate_repository(yaml_r, testcase_dir, app_path, app_name):
    
    TASK_DESCRIPTOR_SIZE = 6 #6 is number of lines to represent a task description 

    print "\n***************** Task page size report ***********************"
    
    task_name_list = get_app_task_name_list(testcase_dir, app_name)
    
    app_tasks_number = len(task_name_list)
    
    ######################################################################################
    ######### Check if task number is compatible to kernel max task number ###############
    ######################################################################################
    check_application_task_number(testcase_dir, app_tasks_number, app_name)
    ######################################################################################
    
    #Used for point the next free address to fill with a given task code (task.txt file)
    initial_address = 0 
    
    #repolines is a list of RepoLine objetcts, the ideia is keep a list generic to be easily converted to any systax
    repo_lines = []
    
    #First line of app descriptor is the app task number
    repo_lines.append( RepoLine(toX( app_tasks_number), "task number to application "+app_name+"" ) ) 
    
    #This variable point to the end of the application header descriptor, this address represent where the 
    #app tasks code can be inserted
    initial_address = initial_address + ( (TASK_DESCRIPTOR_SIZE * app_tasks_number) * 4) + 4#plus 4 because the first word is the app task number
    
    task_id = 0
    
    #Walk for all app tasks into /applications/app dir to fills the task headers
    for task_name in task_name_list:
        
        txt_size = get_task_txt_size(app_path, task_name)
        
        #Points next_free_address to a next free repo space after considering the task code of <task_name>.txt
        repo_lines.append( RepoLine(toX(task_id)                                    , task_name+".c") )
        repo_lines.append( RepoLine(toX(0)                             , "static mapped PE address, filled only during application injection") )
        repo_lines.append( RepoLine(toX(txt_size)                                   , "txt size") )
        repo_lines.append( RepoLine(toX(get_task_DATA_size(app_path, task_name))    , "data size") )
        repo_lines.append( RepoLine(toX(get_task_BSS_size(app_path, task_name))     , "bss size") )
        repo_lines.append( RepoLine(toX(initial_address)                            , "initial_address") )
        
        initial_address = initial_address + (txt_size * 4)
        
        dependenc_list = get_task_dependence_list(app_name, task_name)
        
        task_id = task_id + 1
        
    #After fills the task header, starts to insert the tasks code
    for task_name in task_name_list:
        
        txt_source_file = app_path + "/" + task_name + ".txt"
        
        bin_source_file = app_path + "/" + task_name + ".bin"
        
        check_mem_size(bin_source_file, get_page_size_KB(yaml_r) )
        
        comment = task_name+".c"
        
        task_txt_file = open(txt_source_file, "r")
        
        for line in task_txt_file:
			file_line = line[0:len(line)-1] # removes the \n from end of file
			repo_lines.append( RepoLine(file_line  , comment) )
			comment = ""
                
        task_txt_file.close()
    
    #Here the repository for each task must be writed
    generate_app_repository_file(app_name, app_path, repo_lines, get_model_description(yaml_r))
    
    ################Finally, generates the repository file (main and debug files) ##########################
    print "***************** End task page size report ********************\n"


#Receives a int, convert to string and fills to a 32 bits word
def toX(input):
    hex_string = "%x" % input
    #http://stackoverflow.com/questions/339007/nicest-way-to-pad-zeroes-to-string
    return hex_string.zfill(8) # 8 is the lenght of chars to represent 32 bits (repo word) in hexa

#This fucntion receives the repo_lines filled above and generates two files: the repository itself and a repository_debug
def generate_app_repository_file(app_name, app_path, repo_lines, system_model_description):    
    
    repo_file_path = app_path + "/repository.txt"
    repo_debug_file_path = app_path + "/repository_debug.txt"
    
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
    
    source_file = app_path + "/" + task_name + ".txt"
    
    #http://stackoverflow.com/questions/845058/how-to-get-line-count-cheaply-in-python/1019572#1019572
    num_lines = sum(1 for line in open(source_file))
    
    return num_lines
    
def get_task_DATA_size(app_path, task_name):
    
    source_file = app_path + "/" + task_name + ".bin"
    
    #This command gets the bss data and size of the binary file
    #https://www.quora.com/What-is-a-convenient-way-to-execute-a-shell-command-in-Python-and-retrieve-its-output
    data_size = int (commands.getoutput("mips-elf-size "+source_file+" | tail -1 | sed 's/ //g' | sed 's/\t/:/g' | cut -d':' -f2"))
    
    while data_size % 4 != 0:
        data_size = data_size + 1
        
    data_size = data_size / 4
    
    return data_size
    
def get_task_BSS_size(app_path, task_name):
    
    source_file = app_path + "/" + task_name + ".bin"
    
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

def get_task_comp_load(app_name, task_name):
    return 0 #TO DO

def get_task_dependence_list(app_name, task_name):
    return [] #TO DO


#This function compares the number of tasks of the application with the number of the MAXIMUM task for one application that is supported by the kernel
def check_application_task_number(testcase_dir, app_task_number, app_name):
    
    kernel_max_app = 0
    
    #Useful function that serach a value of a #define is for a given file_path
    with open(testcase_dir+"/include/kernel_pkg.h") as search:
        for line in search:
            line = line.rstrip()  # remove '\n' at end of line
            if "#define MAX_TASKS_APP" in line:
                kernel_max_app = int(line.split(" ")[16])
                break
    if kernel_max_app == 0:
        sys.exit("ERROR in app_builder: impossible to determine the max number of application task\n")
    
    if app_task_number > kernel_max_app:
        tkinter.Tk().wm_withdraw()
        mbox.showinfo('ERROR', "Applicaiton: "+app_name+" has "+str(app_task_number)+" tasks but kernel support MAX_TASKS_APP of "+str(kernel_max_app)+". Please change this kernel constraint inside the file build_env/scripts/kernel_builder.py")
        sys.exit("\nMAX_TASKS_APP boundary exceeded\n")

main()
