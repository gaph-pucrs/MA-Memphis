#!/usr/bin/python
import sys
import math
import os
from yaml_intf import *
from build_utils import *

#This script compiles the hardware part inside the testcase dir and also generate the hardware includes memphis.h or memphis.vhd in the include directory inside the testcase

def main():
    
    testcase_name = sys.argv[1]
    
    yaml_r = get_yaml_reader(testcase_name)

    generate_hw_pkg( yaml_r )
    
    #compile_hw
    exit_status = os.system("cd hardware/; make")
    
    if exit_status != 0:
        sys.exit("\nError compiling hardware source code\n");
    

def generate_hw_pkg( yaml_r ):
    
    #Variables from yaml used into this function
    x_mpsoc_dim =       get_mpsoc_x_dim(yaml_r)
    y_mpsoc_dim =       get_mpsoc_y_dim(yaml_r)
    x_cluster_dim =     get_cluster_x_dim(yaml_r)
    y_cluster_dim =     get_cluster_y_dim(yaml_r)
    master_location =   get_master_location(yaml_r)
    system_model_desc = get_model_description(yaml_r)
    IO_peripherals =    get_IO_peripherals(yaml_r)
    
    #-------Gets the PEs that are connected to IO peripherals------
    pe_number = 0
    io_list = []
    io_name_list = []
    #Walk over x and y address
    for y in range(0, y_mpsoc_dim):
       
        for x in range(0, x_mpsoc_dim):
            #Flag that stores if the PE have IO connections or not
            pe_not_connected_to_IO = True
            
            for io_connection in IO_peripherals:
                
                x_addr = int(io_connection["pe"][0])
                y_addr = int(io_connection["pe"][2])
                
                if (x_addr == x and y_addr == y):
                    
                    pe_not_connected_to_IO = False
                    
                    port_char = io_connection["port"]
                    
                    io_list.append(conv_char_to_port(port_char))
                    
                    io_name_list.append([io_connection["name"],pe_number])
                
            
            if pe_not_connected_to_IO == True:
                io_list.append(5)
                
            pe_number = pe_number + 1
    
            
    cluster_list = create_cluster_list(x_mpsoc_dim, y_mpsoc_dim, x_cluster_dim, y_cluster_dim, master_location)
    
    pe_type = []
    #----------- Generates PEs types ----------------
    is_master_list = []
    #Walk over x and y address
    for y in range(0, y_mpsoc_dim):
        
        for x in range(0, x_mpsoc_dim):
            #By default is a slave PE
            master_pe = False
            
            for cluster_obj in cluster_list:
                if x == cluster_obj.master_x and y == cluster_obj.master_y:
                    master_pe = True
                    break
     
            if master_pe == True:
                is_master_list.append(1) # master
            else:
                is_master_list.append(0) # slave
                
    #------------  Updates the include path -----------
    include_dir_path = "include"
    
    #Creates the include path if not exists
    if os.path.exists(include_dir_path) == False:
        os.mkdir(include_dir_path)
    
    
    if system_model_desc == "sc" or system_model_desc == "scmod":
        
        generate_to_systemc(is_master_list, io_list, io_name_list, yaml_r )
    
    elif system_model_desc == "vhdl":
       
        generate_to_vhdl(is_master_list, io_list, io_name_list, yaml_r )
        


def generate_to_systemc(is_master_list, io_list, io_name_list, yaml_r):
   
    #Variables from yaml used into this function
    page_size_KB =      get_page_size_KB(yaml_r)
    memory_size_KB =    get_memory_size_KB(yaml_r)
    x_mpsoc_dim =       get_mpsoc_x_dim(yaml_r)
    y_mpsoc_dim =       get_mpsoc_y_dim(yaml_r)
    
    string_pe_type_sc = ""
    
    #Walk over is master list
    for pe_type in is_master_list:
        
        string_pe_type_sc = string_pe_type_sc + str(pe_type) + ", "
    
    #Remove the lost ',' in the string_pe_type_vhdl
    string_pe_type_sc = string_pe_type_sc[0:len(string_pe_type_sc)-2] # 2 because the space, otherelse will be 1
    

    string_io_connections_sc = ""
    
    for io_connection in io_list:
        string_io_connections_sc = string_io_connections_sc + str(io_connection) + ", "
    
    #Remove the lost ',' in the string_pe_type_vhdl
    string_io_connections_sc = string_io_connections_sc[0:len(string_io_connections_sc)-2]


    file_lines = []
    #---------------- SYSTEMC SINTAX ------------------
    file_lines.append("#ifndef _MEMPHIS_PKG_\n")
    file_lines.append("#define _MEMPHIS_PKG_\n\n")
    
    file_lines.append("#define PAGE_SIZE_BYTES           "+str(page_size_KB*1024)+"\n")
    file_lines.append("#define MEMORY_SIZE_BYTES      "+str(memory_size_KB*1024)+"\n")
    file_lines.append("#define N_PE_X              "+str(x_mpsoc_dim)+"\n")
    file_lines.append("#define N_PE_Y              "+str(y_mpsoc_dim)+"\n")
    file_lines.append("#define N_PE                "+str(x_mpsoc_dim*y_mpsoc_dim)+"\n\n")
    
    file_lines.append("//Peripheral Position\n")
    for io_peripheral in io_name_list:
        file_lines.append("#define "+io_peripheral[0]+"\t\t\t"+str(io_peripheral[1])+"\n")
    
    file_lines.append("\nconst int pe_type[N_PE] = {"+string_pe_type_sc+"};\n")
    file_lines.append("const char io_port[N_PE]= {"+string_io_connections_sc+"};\n\n")
    file_lines.append("#endif\n")
    
    #Use this function to create any file into testcase, it automatically only updates the old file if necessary
    writes_file_into_testcase("include/memphis_pkg.h", file_lines)
    
    

def generate_to_vhdl(is_master_list, io_list, io_name_list, yaml_r):
    
    #Variables from yaml used into this function
    page_size_KB =      get_page_size_KB(yaml_r)
    memory_size_KB =    get_memory_size_KB(yaml_r)
    noc_buffer_size =   get_noc_buffer_size(yaml_r)
    x_mpsoc_dim =       get_mpsoc_x_dim(yaml_r)
    y_mpsoc_dim =       get_mpsoc_y_dim(yaml_r)
    
    
    #These lines compute the logical memory addresses used by the mlite to index the pages at the local memory
    page_size_h_index   = int( math.log ( (page_size_KB * 1024),           2 ) - 1 )
    #The ceil is used to support floating point log results
    page_number_h_index = int( math.ceil( math.log ( (memory_size_KB/ page_size_KB) , 2 ) )  + page_size_h_index )
    
    #Look if a PE is slave or master
    string_pe_type_vhdl = ""
    
    #Walk over 'is master list'
    for pe_type in is_master_list:
        
        if pe_type == 0: #slave
            string_pe_type_vhdl = string_pe_type_vhdl + "\"sla\","
        else:
            string_pe_type_vhdl = string_pe_type_vhdl + "\"mas\","
    
    #Remove the lost ',' in the string_pe_type_vhdl
    string_pe_type_vhdl = string_pe_type_vhdl[0:len(string_pe_type_vhdl)-1]
    
    
    string_io_connections_vhdl = ""
    
    for io_connection in io_list:
        string_io_connections_vhdl = string_io_connections_vhdl + str(io_connection) + ", "
    
    #Remove the lost ',' in the string_pe_type_vhdl
    string_io_connections_vhdl = string_io_connections_vhdl[0:len(string_io_connections_vhdl)-2]
    
   
    file_lines = []
    #---------------- VHDL SINTAX ------------------
    file_lines.append("library IEEE;\n")
    file_lines.append("use IEEE.Std_Logic_1164.all;\n")
    file_lines.append("use IEEE.std_logic_unsigned.all;\n")
    file_lines.append("use IEEE.std_logic_arith.all;\n\n")
    file_lines.append("package memphis_pkg is\n\n")
    file_lines.append("    -- paging definitions\n")
    file_lines.append("    constant PAGE_SIZE_BYTES          : integer := "+str(page_size_KB*1024)+";\n")
    file_lines.append("    constant MEMORY_SIZE_BYTES        : integer := "+str(memory_size_KB*1024)+";\n")
    file_lines.append("    constant PAGE_SIZE_H_INDEX        : integer := "+str(page_size_h_index)+";\n")
    file_lines.append("    constant PAGE_NUMBER_H_INDEX      : integer := "+str(page_number_h_index)+";\n\n")
    file_lines.append("    -- Memphis top definitions\n")
    file_lines.append("    constant NUMBER_PROCESSORS_X      : integer := "+str(x_mpsoc_dim)+";\n")
    file_lines.append("    constant NUMBER_PROCESSORS_Y      : integer := "+str(y_mpsoc_dim)+";\n")
    file_lines.append("    constant TAM_BUFFER               : integer := "+str(noc_buffer_size)+";\n")
    file_lines.append("    constant NUMBER_PROCESSORS        : integer := "+str(x_mpsoc_dim*y_mpsoc_dim)+";\n\n")
    
    file_lines.append("    -- IO Peripheral Position\n")
    
    for io_peripheral in io_name_list:
        file_lines.append("    constant "+io_peripheral[0]+"\t\t\t: integer := "+str(io_peripheral[1])+";\n")
        
    file_lines.append("\n    -- types\n")
    file_lines.append("    subtype kernel_str is string(1 to 3);\n")
    file_lines.append("    subtype io_link is integer range 0 to 5;\n")
    
    #type NumericCodeType is array (7 downto 0) of Integer range 0 to 9;
#constant EntryCode : NumericCodeType := (2,6,4,8,0,0,1,3);
    
    file_lines.append("    type pe_type_t is array(0 to NUMBER_PROCESSORS-1) of kernel_str;\n")
    file_lines.append("    type io_link_t is array(0 to NUMBER_PROCESSORS-1) of io_link;\n")
    file_lines.append("    constant pe_type : pe_type_t := ("+string_pe_type_vhdl+");\n")
    file_lines.append("    constant io_port : io_link_t := ("+string_io_connections_vhdl+");\n\n")
    file_lines.append("end memphis_pkg;\n")
    
    #Use this function to create any file into testcase, it automatically only updates the old file if necessary
    writes_file_into_testcase("include/memphis_pkg.vhd", file_lines)
    
main()