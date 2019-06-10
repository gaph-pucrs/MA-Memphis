#!/usr/bin/python
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))+"/scripts")
from yaml_intf import *
from build_utils import *

## @package deloream_env
#This script generate all Debugging Enviroment to the testcase direcotory
#Basically, this module is encharged for to create and fills the /debug directory
#It is called by the testcase_builder script


def generate_deloream_env(testcase_path, yaml_testcase_r, scenario_path, yaml_scenario_r):
    
    debug_path = scenario_path+"/debug"
    
    delete_if_exists(debug_path)
    
    os.mkdir(debug_path)
    
    #Create traffic_router file inside debug directory
    with open(os.path.join(dir, debug_path+"/traffic_router.txt"), 'w'):
        pass
    
    os.mkdir(debug_path+"/pipe")
    
    os.mkdir(debug_path+"/request")
    
    services_cfg(testcase_path, debug_path)
    
    platform_cfg(testcase_path, debug_path, yaml_testcase_r, yaml_scenario_r)
    
    cpu_cfg(debug_path)

def cpu_cfg(debug_path):
    cpu_cfg_lines = []
    
    cpu_cfg_lines.append("Interruption\t65536\n")
    cpu_cfg_lines.append("Scheduler\t262144\n")
    cpu_cfg_lines.append("Idle\t524288\n")
    
    writes_file_into_testcase(debug_path+"/cpu.cfg", cpu_cfg_lines)
    
def platform_cfg(testcase_path, debug_path, yaml_testcase_r, yaml_scenario_r):
    
    mpsoc_x = get_mpsoc_x_dim(yaml_testcase_r)
    mpsoc_y = get_mpsoc_y_dim(yaml_testcase_r)
    cluster_x = get_cluster_x_dim(yaml_testcase_r)
    cluster_y = get_cluster_y_dim(yaml_testcase_r)
    manager_position_x = "0"
    manager_position_y = "0"
    global_manager_cluster = "0"
    flit_size = "32"
    clock_period_ns = "10"
    
    platform_cfg_lines = []
    
    platform_cfg_lines.append("router_addressing XY\n")
    platform_cfg_lines.append("channel_number "+str(get_physical_channels(yaml_testcase_r))+"\n")
    platform_cfg_lines.append("mpsoc_x "+str(mpsoc_x)+"\n")
    platform_cfg_lines.append("mpsoc_y "+str(mpsoc_y)+"\n")
    platform_cfg_lines.append("cluster_x "+str(cluster_x)+"\n")
    platform_cfg_lines.append("cluster_y "+str(cluster_y)+"\n")
    platform_cfg_lines.append("manager_position_x "+manager_position_x+"\n")
    platform_cfg_lines.append("manager_position_y "+manager_position_y+"\n")
    platform_cfg_lines.append("global_manager_cluster "+global_manager_cluster+"\n")
    platform_cfg_lines.append("flit_size "+flit_size+"\n")
    platform_cfg_lines.append("clock_period_ns "+clock_period_ns+"\n")
    
    platform_cfg_lines.append("BEGIN_task_name_relation\n")
    
    apps_name_list = get_apps_name_list(yaml_scenario_r)
    
    apps_name_relation = []
    
    apps_name_relation.append("BEGIN_app_name_relation\n")
    
    app_id = 0
    
    for app_name in apps_name_list:
        
        task_name_list = get_app_task_name_list(testcase_path, app_name)
        
        task_id = 0
        
        apps_name_relation.append(app_name+"\t"+str(app_id)+"\n");
        
        for task_name in task_name_list:
            
            relative_id =  app_id << 8 | task_id
            
            platform_cfg_lines.append(task_name+" "+str(relative_id)+"\n")
            
            task_id = task_id + 1
        
        app_id = app_id + 1
    
    apps_name_relation.append("END_app_name_relation\n")
    
    platform_cfg_lines.append("END_task_name_relation\n")
    
    platform_cfg_lines = platform_cfg_lines + apps_name_relation;
    
    writes_file_into_testcase(debug_path+"/platform.cfg", platform_cfg_lines)

def services_cfg(testcase_path, debug_path):
    
    fp = open(testcase_path+"/software/include/services.h", "r")
    
    service_cfg_lines = []
    
    for line in fp:
        split_line = line.split()
        #Test if the string is in split_line and split line leght > 2
        if "#define" in split_line and len(split_line) > 2:
            value = int(split_line[2], 0)
            #value_hex = format(value, 'x') #Incompatible below version 2.6
            value_hex = "%x" % value
            service_cfg_lines.append(split_line[1]+" "+value_hex+"\n")
    fp.close()
    
    service_cfg_lines.append("\n")
    service_cfg_lines.append("$TASK_ALLOCATION_SERVICE 40 221\n")
    service_cfg_lines.append("$TASK_TERMINATED_SERVICE 70 221\n")
    
    writes_file_into_testcase(debug_path+"/services.cfg", service_cfg_lines)
    
    
    
    
