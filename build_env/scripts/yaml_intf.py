#!/usr/bin/python
import yaml
import sys
from operator import attrgetter
from build_utils import ApplicationInfo, get_app_task_name_list

## @package yaml_intf
#IMPORTANT: This file encapsulates the yaml reading process, abstracting this process to another modules
#For this reason, modification in the yaml sintax are only reflected here
#USE THIS FILE TO PROVIDE FUNCIONS TO ALL MODULES
#If you need to modify the yaml syntax, modify only this file, in a way to make
#the yaml reading process transparent to the other modules. This process is done by creating new functions in this module
#that can be called in the other modules

def get_yaml_reader(yaml_path):
    try:
        file = open(yaml_path, 'r')
    except:
        sys.exit('ERROR: No such following testcase file or directory (%s)!' % yaml_path)
    try:
        yaml_reader = yaml.load(file)
    except:
        sys.exit('ERROR: Incorrent YAML sintax!!\nThe YAML file does not support the character tab (\\t) (%s)!\n' % yaml_path)
    
    return yaml_reader

def get_page_size_KB(yaml_reader):
    return yaml_reader["hw"]["page_size_KB"]

def get_tasks_per_PE(yaml_reader):
    return yaml_reader["hw"]["tasks_per_PE"]

def get_memory_size_KB(yaml_reader):
    #page size number plus one (kernel page) x page_size_KB
    memory_size_KB = (get_tasks_per_PE(yaml_reader) + 1) * get_page_size_KB(yaml_reader)
    
    return memory_size_KB

def get_model_description(yaml_reader):
    return yaml_reader["hw"]["model_description"]

def get_noc_buffer_size(yaml_reader):
    return yaml_reader["hw"]["noc_buffer_size"]

def get_mpsoc_x_dim(yaml_reader):
    return yaml_reader["hw"]["mpsoc_dimension"][0]

def get_mpsoc_y_dim(yaml_reader):
    return yaml_reader["hw"]["mpsoc_dimension"][1]

def get_cluster_x_dim(yaml_reader):
    return yaml_reader["hw"]["cluster_dimension"][0]

def get_cluster_y_dim(yaml_reader):
    return yaml_reader["hw"]["cluster_dimension"][1]

def get_master_location(yaml_reader):
    try:
        return yaml_reader["hw"]["master_location"]
    except:
        return "LB"

def get_physical_channels(yaml_reader):
    to_ret = 1
    
    try:
        to_ret = yaml_reader["hw"]["physical_channels"]
    except:
        pass
    return to_ret

def get_mapping_algorithm(yaml_reader):
    return yaml_reader["sw"]["mapping_algorithm"]

#Return a sorted app name list according the start time
def get_apps_name_list(yaml_reader):
   
    apps_list = yaml_reader["apps"]

    apps_name_list = []

    for app in apps_list:
        start_time = 0
        try:
            start_time = app["start_time_ms"]
        except:
            pass
        apps_name_list.append((start_time, app["name"]))
        
    sorted_list = sorted(apps_name_list, key=lambda app: app[0])
    
    apps_name_list = []
    
    for app in sorted_list:
        apps_name_list.append(app[1])
        
    return apps_name_list

def get_apps_number(yaml_reader):
    number = len(get_apps_name_list(yaml_reader))
    return number

def get_task_scheduler(yaml_reader):
    return yaml_reader["sw"]["task_scheduler"]

def get_app_repo_size(yaml_reader):
    return 1000

#Returns a list of objects ApplicationInfo containing the app name, num of tasks, start time and cluster id
#However repo address return -1, it is filled inside app_builder during the repositoriy generation
def get_app_info_list(yaml_reader, testcase_path):
    
    #Start of processing to extracts the information of task statically mapped
    app_start_list = []  
    
    yaml_app_index = 0
    
    app_id = 0
    
    for app_reader in yaml_reader["apps"]:
        app_name = app_reader["name"]
        
        #Then, list all tasks from app_name
        app_task_list = get_app_task_name_list(testcase_path, app_name)
        
        task_number = len(app_task_list)
        
        #If the time is not configured - default is zero
        start_time_ms = 0
        cluster = -1
        try:
            start_time_ms = int(app_reader["start_time_ms"])
        except:
            pass
        try:
            cluster = app_reader["cluster"]
        except:
            pass
        
        #Test if a given app have the static_mapping tag
        static_mapping_tasks = []
        try:
            static_mapping_tasks = yaml_reader["apps"][app_id]["static_mapping"]
        except:
            pass # This means that the application not has any task mapped statically
        
        task_id = 0
        
        #Checks for typing error in the task name
        for static_task in static_mapping_tasks:
            if static_task not in app_task_list: 
                print "[WARNING]: Static task name ["+static_task+"] does not belong to application [" + app_name+ "], it will be ignored in static mapping\n"
        
        static_task_list = []
        #Walk over all tasks of app
        for task_name in app_task_list:
            
            #Walk over all tasks signaled within static mapping flag
            if task_name in static_mapping_tasks:
                
                x_address = int( static_mapping_tasks[task_name][0] ) # Gets the x address value
                y_address = int( static_mapping_tasks[task_name][1] ) # Gets the y address value
                
                task_static_map = x_address << 8 | y_address
                
                static_task_list.append([task_id, task_static_map])
                
            else:
                static_task_list.append([task_id, -1]) #Signals a dynamic mapping
                
            task_id = task_id + 1
        
        #Create a new object of Application info gathering all information previously extracted from yaml
        app_start_list.append( ApplicationInfo(app_name, start_time_ms, task_number, cluster, static_task_list) )
        
        app_id = app_id + 1

    # Sort the list in place.  See more in : https://wiki.python.org/moin/HowTo/Sorting
    app_start_list.sort(key=lambda app: app.start_time_ms)
    
    return app_start_list

def get_IO_peripherals(yaml_reader):
    to_return = []
    
    try:
        to_return = yaml_reader["hw"]["Peripherals"]
    except:
        pass
    return to_return
        
    



