#!/usr/bin/python
import sys
import os
import filecmp
import commands
from shutil import copyfile, rmtree
from math import ceil

## @package build_utils
# This script contains utils function that are used by other python files

#------------ Python classes. See more in: http://www.tutorialspoint.com/python/python_classes_objects.htm
#This class is used to store the application info. (as start time) of applications, usefull into repository generation process
class ApplicationInfo:
    def __init__(self, name, start_time_ms, task_number, cluster_id, task_static_mapping_list):
        self.name = name
        self.start_time_ms = start_time_ms #Stores the application descriptor repository address
        self.task_number = task_number
        self.cluster_id = cluster_id #Store the cluster ID if the application was statically mapped to a cluster. Otherwise, stores -1;
        self.static_task_list = task_static_mapping_list #Stores a list of tuple={task_id, allocated proc address}

#This class stores the repository lines in a generic way, this class as well as StaticTask are used in the app_builder module
class RepoLine:
    def __init__(self, hex_string, commentary):
        self.hex_string = hex_string
        self.commentary = commentary

#Python class used for store Cluste's information. 
class Cluster:
    def __init__(self, master_x, master_y, leftbottom_x, leftbottom_y, topright_x, topright_y):
        self.master_x = master_x
        self.master_y = master_y
        self.leftbottom_x = leftbottom_x
        self.leftbottom_y = leftbottom_y
        self.topright_x = topright_x
        self.topright_y = topright_y 

#Functions -------------------------------------------------

def create_cluster_list(x_mpsoc_dim, y_mpsoc_dim, x_cluster_dim, y_cluster_dim, master_location):
   
    if  ( x_mpsoc_dim % x_cluster_dim ) != 0 or (y_mpsoc_dim % y_cluster_dim ) != 0:
        sys.exit('Error in YAML noc_dimension OR cluster_dimension - you must provide a compatible dimension')
        
    x_clusters_number = x_mpsoc_dim / x_cluster_dim;
    y_clusters_number = y_mpsoc_dim / y_cluster_dim;
    
    cluster_list = []
    
    for y in range(0, y_clusters_number):
        for x in range(0, x_clusters_number):
            
            leftbottom_x =   x  * x_cluster_dim
            leftbottom_y =   y  * y_cluster_dim
            topright_x   =   ( (x+1) *  x_cluster_dim ) - 1
            topright_y   =   ( (y+1) *  y_cluster_dim ) - 1
            
            if master_location == "LB":
               
                master_x =   x * x_cluster_dim
                master_y =   y * y_cluster_dim
                
            elif master_location == "RB":
                
                master_x =   ( (x+1) *  x_cluster_dim ) - 1
                master_y =   y * y_cluster_dim
                
            elif master_location == "LT":
                
                master_x =   x * x_cluster_dim
                master_y =   ( (y+1) *  y_cluster_dim ) - 1
                
            elif master_location == "RT":
                
                master_x =   ( (x+1) *  x_cluster_dim ) - 1
                master_y =   ( (y+1) *  y_cluster_dim ) - 1
                
            else:
                sys.exit("Error in YAML master_location - you must provide a compatible master position: [LB | RB | LT | RT]")
            
            cluster_list.append(Cluster(master_x, master_y, leftbottom_x, leftbottom_y, topright_x, topright_y))
    
    return cluster_list

def writes_file_into_testcase(file_path, file_lines):
    
    tmp_file_path = file_path + "tmp"
    
    file = open(tmp_file_path, 'w+')
    file.writelines(file_lines)
    file.close()
    
    if os.path.isfile(file_path):
        #If the file are equals remove the new file tmp and then do nothing
        if filecmp.cmp(file_path, tmp_file_path) == True:
            os.remove(tmp_file_path)
            return
       
    copyfile(tmp_file_path, file_path)
    os.remove(tmp_file_path)

#Check the page size, comparing the (code size + 50%) of the file_path
#with the page_size. 
def check_mem_size(file_path, mem_size_KB):
    
    program_memory_size = int (commands.getoutput("mips-elf-size "+file_path+" | tail -1 | sed 's/ //g' | sed 's/\t/:/g' | cut -d':' -f4"))
    
    file_size_KB = program_memory_size / 1024.0
    
    file_size_KB = ceil( file_size_KB + (file_size_KB * 0.2) )
    
    if file_size_KB >= mem_size_KB:
        sys.exit("ERROR: Insufficient memory size ("+str(mem_size_KB)+"KB) for file <"+file_path+"> size ("+str(file_size_KB)+"KB)")

    #This print needs more for: http://www.python-course.eu/python3_formatted_output.php
    #But currently I am very very very busy - so, if I is reading this, please...do it!!
    print ("Memory size ("+str(mem_size_KB)+"KB) OK for file\n-"+file_path+">\t with size ("+str(file_size_KB)+"KB)")

def get_app_task_name_list(testcase_path, app_name):
    
    source_file = testcase_path+"/applications/" + app_name
    
    task_name_list = []
    
    for file in os.listdir(source_file):
        if file.endswith(".c"):
            file_name = file.split(".")[0] #Splits the file name by '.' then gets the first element - [filename][.c]
            task_name_list.append(file_name)

    task_name_list.sort()
            
    return task_name_list

def conv_char_to_port(port_char):
    if port_char == 'E':
        return 0
    if port_char == 'W':
        return 1
    if port_char == 'N':
        return 2
    if port_char == 'S':
        return 3
    if port_char == 'L':
        return 4
    return 5

def delete_if_exists( path_dir ):
    if os.path.exists(path_dir):
        rmtree(path_dir, False, None)

def create_ifn_exists( path_dir ):
    if not os.path.exists(path_dir):
        os.mkdir(path_dir)

# This fucntion copies the source files in source_dir to target_dir
#If you desire to add especific copies test, for example, ignore some specific files names or extensions, 
#please includes those file name or extension into the 3rd argument (ignored_names_list), the name can be the file name or its extension
def generic_copy(source_dir, target_dir, ignored_extension_list):
    
    exclude_extensions = " --exclude=".join(ignored_extension_list)
    
    command_string = "rsync -u -r --exclude="+exclude_extensions+" "+source_dir+"/ "+target_dir+"/"
    
    status = os.system(command_string)
