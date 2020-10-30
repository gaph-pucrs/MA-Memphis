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
    max_tasks_app   =   get_max_tasks_app(yaml_r)
    x_mpsoc_dim =       get_mpsoc_x_dim(yaml_r)
    y_mpsoc_dim =       get_mpsoc_y_dim(yaml_r)
    IO_peripherals =    get_IO_peripherals(yaml_r)

    mpsoc_dim = x_mpsoc_dim * y_mpsoc_dim
    
    file_lines = []
    #---------------- C SINTAX ------------------
    file_lines.append("#pragma once\n")
    file_lines.append("#define PKG_MAX_LOCAL_TASKS "+str(max_local_tasks)+" //!> Max task allowed to execute into a single processor\n")
    file_lines.append("#define PKG_PAGE_SIZE "+str(page_size_KB*1024)+" //!> The page size each task will have (inc. kernel)\n")
    file_lines.append("#define PKG_MAX_TASKS_APP "+str(max_tasks_app)+" //!> Max number of tasks for the APPs described into testcase file\n")
    file_lines.append("#define PKG_PENDING_SVC_MAX "+str(20)+" //!< Pending service array size\n")
    file_lines.append("#define PKG_SLACK_TIME_WINDOW "+str(50000)+" //!< Half millisecond\n")
    file_lines.append("#define PKG_MAX_KERNEL_MSG_LEN "+str(10)+"   //!< Size of the kernel output pending service\n")

    file_lines.append("#define PKG_N_PE "+str(mpsoc_dim)+"  //!< Number of PEs\n")
    file_lines.append("#define PKG_N_PE_X "+str(x_mpsoc_dim)+" //!< Number of PEs in X dimension\n")

    file_lines.append("#define PKG_PERIPHERALS_NUMBER "+str(len(IO_peripherals))+"      //max number of peripherals\n")
    for io_peripheral in IO_peripherals:
        pe_addr = int(io_peripheral["pe"][0]) << 8 | int(io_peripheral["pe"][2])
        port = io_peripheral["port"]
        port_encoded = 0 #Stores the port where the peripheral is connected to PE. Such information is used by XY routing algorithm (See SwitchControl.vhd of router implementation)
        if (port == "E"):
            port_encoded = 2147483648 # == 1000 0000 0000 0000 0000 0000 0000 0000 == IO routing On through port East
        elif (port == "W"):
            port_encoded = 2684354560 # == 1010 0000 0000 0000 0000 0000 0000 0000 == IO routing On through port West
        elif (port == "N"):
            port_encoded = 3221225472 # == 1100 0000 0000 0000 0000 0000 0000 0000 == IO routing On through port North
        elif (port == "S"):
            port_encoded = 3758096384 # == 1110 0000 0000 0000 0000 0000 0000 0000 == IO routing On through port South
        file_lines.append("#define "+str(io_peripheral["name"])+" "+hex(port_encoded|pe_addr)+" //This number is hot encoded (1 k bit + 2 n bits + 13 zeros + 8 x bits + 8 y bits). k bit when on enable routing to external peripheral, n bits signals the port between peripheral and PE, x bits stores the X address of PE, y bits stores the Y address of PE\n")
    
    #Use this function to create any file into testcase, it automatically only updates the old file if necessary
    writes_file_into_testcase("software/kernel/pkg.h", file_lines)

#Generates the memory symbolic link
def generate_memory( yaml_r ):
    
    #Variables from yaml used into this function
    x_mpsoc_dim     =   get_mpsoc_x_dim(yaml_r)
    y_mpsoc_dim     =   get_mpsoc_y_dim(yaml_r)
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
        for x in range(0, x_mpsoc_dim):
            for y in range(0, y_mpsoc_dim):                
                dst_ram_file = memory_path+"/ram"+str(x)+"x"+str(y)+".txt"
                os.symlink("../../software/kernel.txt", dst_ram_file)
              
      
main()
