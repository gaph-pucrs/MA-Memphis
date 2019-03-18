#!/usr/bin/env python
import sys
import os
import subprocess
import filecmp
from shutil import copyfile, rmtree
from os.path import join
from yaml_intf import *
from build_utils import *
from wave_builder import generate_wave 

## @package testcase_builder
# This module generates a self-contained testcase directory with all source files, makes, waves, and scripts inside.
# If the testcase dir already exists it only copies the diferent files.
# This script requeris only one argument, that is the path to the yaml file.
# The path of the generated testcase directory will be the current path that is calling the script. 
#IMPORTANT: This module also copies the python file inside the directory scripts. 
#    Those scripts must be called inside the testcase dir. Their alread haas been called by the makefile of the testcase


#When defined, a main function must be called in the last line of the script, take a look at the end of file
def main():
   
    #This script copies and compile a given app
    MEMPHIS_PATH = os.getenv("MEMPHIS_PATH", 0)
    MEMPHIS_HOME = os.getenv("MEMPHIS_HOME", 0)
    print "\n"
    
     #Test if testcase file MEMPHIS_PATH is valid
    if MEMPHIS_HOME == 0:
        print "\nWARNING: MEMPHIS_HOME not defined, using as default testcase dir MEMPHIS_PATH/testcases\n"
        MEMPHIS_HOME = MEMPHIS_PATH + "/testcases"

    INPUT_TESTCASE_FILE_PATH = sys.argv[1]
       
    #Extracts the testcase_dir:
    path_list = INPUT_TESTCASE_FILE_PATH.split("/") #The testcase path can have severals '/' into its composition
    input_yaml_name = path_list[len(path_list)-1] #Gets the last element of the split list
    TESTCASE_PATH = MEMPHIS_HOME + "/" + input_yaml_name.split(".")[0] #Creates a new path using the MEMPHIS_HOME + the YAML name
    TESTCASE_NAME = input_yaml_name.split(".")[0]
        
    #print "Path Summary: "
    #print "MEMPHIS_PATH: ", MEMPHIS_PATH
    #print "MEMPHIS_HOME: ",MEMPHIS_HOME
    #print "TESTCASE_PATH:",TESTCASE_PATH
    #print "TESTCASE_NAME:",TESTCASE_NAME
    #print "INPUT_TESTCASE_FILE_PATH: ",INPUT_TESTCASE_FILE_PATH
    
    #Test if there are some differences between the input testcase file and the testcase file into testcase directory 
    #If there are differences, them deleted the testcase directory to rebuild a new testcase
    testcase_file_inside_dir = TESTCASE_PATH+"/"+TESTCASE_NAME+".yaml"
    if os.path.exists(testcase_file_inside_dir):
        testecase_changes = not filecmp.cmp(INPUT_TESTCASE_FILE_PATH, testcase_file_inside_dir)
        if testecase_changes:
            delete_if_exists(TESTCASE_PATH)
    
    
    #Create the testcase path if not exist
    create_ifn_exists(TESTCASE_PATH)
    
    #Reads some importats parameters from testcase
    yaml_reader = get_yaml_reader(INPUT_TESTCASE_FILE_PATH)
    model_description = get_model_description(yaml_reader)
    page_size_KB = get_page_size_KB(yaml_reader)
    memory_size_KB = get_memory_size_KB(yaml_reader)
    
    #Create the application dir
    create_ifn_exists(TESTCASE_PATH+"/applications")
    
    #Create the application dir
    delete_if_exists(TESTCASE_PATH+"/base_scenario")
    create_ifn_exists(TESTCASE_PATH+"/base_scenario")
    
    #Testcase generation: updates source files...
    copy_scripts ( MEMPHIS_PATH,  TESTCASE_PATH)
    copy_kernel( MEMPHIS_PATH,  TESTCASE_PATH)
    copy_hardware( MEMPHIS_PATH,  TESTCASE_PATH, model_description)
    copy_makefiles( MEMPHIS_PATH,  TESTCASE_PATH, page_size_KB, memory_size_KB, model_description)
    copy_testcase_file( testcase_file_inside_dir, INPUT_TESTCASE_FILE_PATH)
    
    #Create other important dirs
    create_ifn_exists(TESTCASE_PATH+"/include")
    
    #Calls the memphis-wave_gen script if
    generate_wave(TESTCASE_PATH, yaml_reader)
    
    #Compile the hw and kernel by calling hw_builder and kernel_builder    
    exit_status = os.system("make -C "+TESTCASE_PATH)
    
    if exit_status == 0:
        print "\n\nMemphis platform generated and compiled successfully at:\n-"+TESTCASE_PATH+"\n"
    else:
        print "\n*** ERROR: Memphis compilation not completed, process stopped!!!\n"
    
# ----------------------------------------- FUNCTIONS ---------------------------------------------

def copy_scripts(memphis_path, testcase_path):
    
    source_script_path = memphis_path+"/build_env/scripts"
    testcase_script_path = testcase_path+"/build"
    
    generic_copy(source_script_path, testcase_script_path, [".svn"] )

#This funtion copies the software source files to the testcase/software path. The copied files are kernel and apps
def copy_kernel(memphis_path, testcase_path):
    
    source_sw_path = memphis_path+"/software"
    testcase_sw_path = testcase_path+"/software"
    
    #--------------  COPIES ALL THE FILES .H AND .C FROM SOFTWARE DIRECTORY ----------------
    generic_copy(source_sw_path, testcase_sw_path, [".svn"] )
   
# This function copies the source files in hardware dir according with the system model description
#For example, to a SytemC description, all files in the hardware dir with the extension .ccp and .h will be copied
#If you desire to add specific copies test, for example, ignore some specific vhd files, please include those file name or extension
#into the 3rd argument (ignored_names_list), the name can be the file name or its extension
def copy_hardware(memphis_path, testcase_path, system_model_description):
    
    source_hw_path = memphis_path+"/hardware"
    testcase_hw_path = testcase_path+"/hardware"
    
    #Creates the directory into testcase path
    create_ifn_exists(testcase_hw_path)
    
    if system_model_description == "sc" or system_model_description == "scmod":
        
        delete_if_exists(testcase_hw_path+"/vhdl")
        source_hw_path = source_hw_path+"/sc"
        testcase_hw_path = testcase_hw_path+"/sc"
        ignored_names_list = [".svn" , ".vhd"]
        
    elif system_model_description == "vhdl":
        
        delete_if_exists(testcase_hw_path+"/sc")
        
        #--New code: In case of vhdl description, copies the peripherals files (that are implemented in Systemc) from sc directory
        source_perif_path = source_hw_path+"/sc/peripherals"
        testcase_perif_path = testcase_hw_path+"/sc"
        create_ifn_exists(testcase_perif_path)
        testcase_perif_path = testcase_hw_path+"/sc/peripherals"
        create_ifn_exists(testcase_perif_path)
            
        ignored_names_list = [".svn"]
        generic_copy(source_perif_path, testcase_perif_path, ignored_names_list)
        #------------------------End new code------------------------------------------------------------------
        
        source_hw_path = source_hw_path+"/vhdl"
        testcase_hw_path = testcase_hw_path+"/vhdl"
        ignored_names_list = [".svn" , ".h", ".cpp"]
        
    else:
        sys.exit('Error in system_model_description - you must provide a compatible system model description')
    
    generic_copy(source_hw_path, testcase_hw_path, ignored_names_list)

def copy_makefiles(memphis_path, testcase_path, page_size_KB, memory_size_KB, system_model_description):
     #--------------  COPIES THE MAKEFILE TO SOFTWARE DIR ----------------------------------
   
    makes_dir = memphis_path+"/build_env/makes"
    
    if system_model_description == "sc":
        
        copyfile(makes_dir+"/make_systemc", testcase_path+"/hardware/makefile")
        
        if os.path.isfile(testcase_path+"/base_scenario/sim.do"):
            os.remove(testcase_path+"/base_scenario/sim.do") 
        
    elif system_model_description == "vhdl":
        
         copyfile(makes_dir+"/make_vhdl", testcase_path+"/hardware/makefile")
         
         copyfile(makes_dir+"/sim.do", testcase_path+"/base_scenario/sim.do")
         
        
    elif system_model_description == "scmod":
        
        copyfile(makes_dir+"/make_systemc_mod", testcase_path+"/hardware/makefile")

        copyfile(makes_dir+"/sim.do", testcase_path+"/base_scenario/sim.do")  
    #Changes the sim.do exit mode according the system model
    if system_model_description == "scmod":
        
        sim_do_path = testcase_path+"/base_scenario/sim.do"
        sim_file = open(sim_do_path, "a")
        sim_file.write("\nwhen -label end_of_simulation { Memphis/PE0x0/end_sim_reg == x\"00000000\" } {echo \"End of simulation\" ; quit ;}")
        sim_file.close()
    
    elif system_model_description == "vhdl":
        
        sim_do_path = testcase_path+"/base_scenario/sim.do"
        sim_file = open(sim_do_path, "a")
        sim_file.write("\nwhen -label end_of_simulation { Memphis/proc(0)/PE/end_sim_reg == x\"00000000\" } {echo \"End of simulation\" ; quit ;}")
        sim_file.close()
        
        
    copyfile(makes_dir+"/make_testcase", testcase_path+"/makefile")
    
    copyfile(makes_dir+"/make_kernel", testcase_path+"/software/makefile")
    
     #Open the file (closing after scope) to append the PAGE_SP_INIT and MEM_SP_INIT value
    make_file_path = testcase_path + "/software/makefile"
    
    lines = []

    lines.append("PAGE_SP_INIT = "+ str((page_size_KB  *  1024) - 1) + "\n")
    lines.append("MEM_SP_INIT  = "+ str((memory_size_KB * 1024) - 1) + "\n")
    
    append_lines_at_end_of_file(make_file_path, lines)
    
    copyfile(makes_dir+"/make_all_apps", testcase_path+"/applications/makefile")
    
        
#This function copies the input testcase file to the testcase path.
#If the input testcase file is equal to the current testcase file, the copy is suspended. Further, the package generation is not fired 
#The function return if the testcase are equals = True or not equals = False
def copy_testcase_file(current_testcase_file_path, input_testcase_file_path):
    
    if os.path.isfile(current_testcase_file_path):
    
        #If the file are equals then do nothing
        if filecmp.cmp(input_testcase_file_path, current_testcase_file_path) == True:
            return True
    
        
    copyfile(input_testcase_file_path, current_testcase_file_path)
    return False

def append_lines_at_end_of_file(file_path, lines):
    
    f = open(file_path, "a")
    
    f.writelines(lines)
    
    f.close()
    
#Call of function main, this aproaches enables to call a fucntion before it declaration
main()
