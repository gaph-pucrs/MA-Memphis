#!/usr/bin/env python2
import sys
import math
import os
import multiprocessing
from yaml_intf import *
from build_utils import *

## @package kernel_builder
#This scripts compile the kernels and generates the include files kernel_pkg.h and kernel_pkg.c
#Additionally, this scripts generate the processors' memory text files

def main():
    
    testcase_name = sys.argv[1]
    
    yaml_r = get_yaml_reader(testcase_name)
    
    generate_sw_pkg( yaml_r )
    NCPU = multiprocessing.cpu_count()
    #compile_kernel master and slave and if exit status is equal to 0 (ok) then check page_size
    exit_status = os.system("cd software/; make "+"-j"+str(NCPU))
        
    if exit_status != 0:
        sys.exit("\nError compiling kernel source code\n");
        
    print "\n***************** kernel page size report ***********************"
    check_mem_size("software/kernel.elf", get_page_size_KB(yaml_r) )        
    print "***************** end kernel page size report *********************\n"
    
    generate_memory( yaml_r )

def generate_sw_pkg( yaml_r ):
    
    #Variables from yaml used into this function
    page_size_KB =      get_page_size_KB(yaml_r)
    max_local_tasks =   get_tasks_per_PE(yaml_r)    
    
    file_lines = []
    #---------------- C SINTAX ------------------
    file_lines.append("#pragma once\n")
    file_lines.append("#define PKG_MAX_LOCAL_TASKS "+str(max_local_tasks)+" //!> Max task allowed to execute into a single processor\n")
    file_lines.append("#define PKG_PAGE_SIZE "+str(page_size_KB*1024)+" //!> The page size each task will have (inc. kernel)\n")
    file_lines.append("#define PKG_MAX_TASKS_APP "+str(10)+" //!> Max number of tasks for the APPs described into testcase file\n")
    file_lines.append("#define PKG_PENDING_SVC_MAX "+str(20)+" //!< Pending service array size\n")
    file_lines.append("#define PKG_SLACK_TIME_WINDOW "+str(50000)+" //!< Half millisecond\n")
    
    #Use this function to create any file into testcase, it automatically only updates the old file if necessary
    writes_file_into_testcase("software/kernel/pkg.h", file_lines)

#Generates the memory symbolic link
def generate_memory( yaml_r ):
    
    #Variables from yaml used into this function
    x_mpsoc_dim     =   get_mpsoc_x_dim(yaml_r)
    y_mpsoc_dim     =   get_mpsoc_y_dim(yaml_r)
    x_cluster_dim   =   get_cluster_x_dim(yaml_r)
    y_cluster_dim   =   get_cluster_y_dim(yaml_r)
    model_descr     =   get_model_description(yaml_r)
    master_location =   get_master_location(yaml_r)
    mem_size_KB     =   get_memory_size_KB(yaml_r)
    
    
    memory_path = "base_scenario/ram_pe"
    delete_if_exists(memory_path)
    os.mkdir(memory_path)
  
    if model_descr == "vhdl":
        
        #This part of code set the apropriated memory size as input to the ram_generator
        gen_compatible_mem_size = 0;
        if mem_size_KB <= 64:
            gen_compatible_mem_size = 64;
        elif mem_size_KB <= 128:
            gen_compatible_mem_size = 128
        elif mem_size_KB <= 256:
            gen_compatible_mem_size = 256;
        elif mem_size_KB <= 512:
            gen_compatible_mem_size = 512;
        elif(mem_size_KB <= 1024):
            gen_compatible_mem_size = 1024;
    
        ext_slave  = os.system("cd software; ram_generator " + str(gen_compatible_mem_size) + " -rtl kernel_slave.txt > ../base_scenario/ram_pe/ram_slave.vhd")
        
        ext_master = os.system("cd software; ram_generator " + str(gen_compatible_mem_size) + " -rtl kernel_master.txt > ../base_scenario/ram_pe/ram_master.vhd")
        
        if ext_master != 0 or ext_slave != 0 or gen_compatible_mem_size == 0:
            sys.exit("ERROR: Error in the ram_generation process")
        
        if gen_compatible_mem_size > 256:
            sys.exit("ERROR: Memory size for VHDL description not allowed, please reduce page_size_KB or tasks_per_PE in yaml file until this message disappear :(")
        
    else:
        cluster_list = create_cluster_list(x_mpsoc_dim, y_mpsoc_dim, x_cluster_dim, y_cluster_dim, master_location)
        
        for x in range(0, x_mpsoc_dim):
            for y in range(0, y_mpsoc_dim):
                
                master_pe = False
                for cluster_obj in cluster_list:
                    if x == cluster_obj.master_x and y == cluster_obj.master_y:
                        master_pe = True
                        break
                
                dst_ram_file = memory_path+"/ram"+str(x)+"x"+str(y)+".txt"
                
                if master_pe == True:
                    os.symlink("../../software/kernel_master.txt", dst_ram_file)
                else:
                    os.symlink("../../software/kernel_slave.txt", dst_ram_file)
              
      
main()
