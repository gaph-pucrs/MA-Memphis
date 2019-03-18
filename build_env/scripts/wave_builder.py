#!/usr/bin/env python
import sys
import os
from yaml_intf import *

## @package wave_generator
#This scripts generates the wave.do inside the testcase dir. Modify this scripts as you need to improve debuggability in your project

def generate_wave(testcase_path, yaml_reader):
    
    system_model = get_model_description(yaml_reader)
    
    if system_model == "scmod" or system_model == "vhdl":
    
        wave_lines = generate_wave_generic(yaml_reader, system_model)
    
        wave_path = testcase_path + "/base_scenario/wave.do"
        try:
            if not os.path.isfile(wave_path):
                f = open(wave_path, "w+")
                f.writelines(wave_lines)
                f.close()
        except:
            sys.exit("\n[WARN] wave.do not created, testcase directory not created yet!!\n")
            pass

def generate_wave_generic(yaml_r, system_model):
    
    mpsoc_x_dim =       get_mpsoc_x_dim(yaml_r)
    mpsoc_y_dim =       get_mpsoc_y_dim(yaml_r)
    
    wave_lines = []
    wave_lines.append("onerror {resume}\n")
    wave_lines.append("quietly WaveActivateNextPane {} 0\n") 
    
    wave_lines.append("add wave -noupdate /test_bench/Memphis/clock\n")
    
    
    app_injector_send = "add wave -noupdate -group {App Injector} -group send -radix hexadecimal /test_bench/App_Injector/"
    app_injector_receive = "add wave -noupdate -group {App Injector} -group receive -radix hexadecimal /test_bench/App_Injector/"
    app_injector_monitor = "add wave -noupdate -group {App Injector} -group monitor -radix hexadecimal /test_bench/App_Injector/"
    
    wave_lines.append(app_injector_monitor + "EA_new_app_monitor\n")
    wave_lines.append(app_injector_monitor + "app_cluster_id\n")
    wave_lines.append(app_injector_monitor + "app_task_number\n")
    wave_lines.append(app_injector_monitor + "app_start_time\n")
    wave_lines.append(app_injector_monitor + "app_name\n")
    wave_lines.append(app_injector_send + "tx\n")
    wave_lines.append(app_injector_send + "data_out\n")
    wave_lines.append(app_injector_send + "credit_in\n")
    wave_lines.append(app_injector_send + "EA_send_packet\n")
    wave_lines.append(app_injector_send + "packet_size\n")
    wave_lines.append(app_injector_receive + "EA_receive_packet\n")
    wave_lines.append(app_injector_receive + "rx\n")
    wave_lines.append(app_injector_receive + "data_in\n")
    wave_lines.append(app_injector_receive + "credit_out\n")
                    
    pe_index = 0
    
    for x in range(0, mpsoc_x_dim):
        for y in range(0, mpsoc_y_dim):
            
            pe_index = (y * mpsoc_x_dim) + x
            
            if system_model == "scmod":
                pe = str(x) + "x" + str(y)
                pe_group = "add wave -noupdate -group {PE "+pe+"} "
                signal_path = "-radix hexadecimal /test_bench/Memphis/PE"+pe+"/"
            else: #vhdl
                pe = str(pe_index)
                pe_group = "add wave -noupdate -group {PE"+str(x) + "x" + str(y)+"} "
                signal_path = "-radix hexadecimal /test_bench/Memphis/proc("+pe+")/PE/"
            
            
            cpu_group =  "-group CPU_"+pe+" " 
            
            wave_lines.append(pe_group + cpu_group + signal_path + "/current_page\n")
            wave_lines.append(pe_group + cpu_group + signal_path + "/irq_mask_reg\n")
            wave_lines.append(pe_group + cpu_group + signal_path + "/irq_status\n") 
            wave_lines.append(pe_group + cpu_group + signal_path + "/irq\n") 
            wave_lines.append(pe_group + cpu_group + signal_path + "/cpu_mem_address_reg\n") 
            wave_lines.append(pe_group + cpu_group + signal_path + "/cpu_mem_data_write_reg\n") 
            wave_lines.append(pe_group + cpu_group + signal_path + "/cpu_mem_data_read\n") 
            wave_lines.append(pe_group + cpu_group + signal_path + "/cpu_mem_write_byte_enable\n") 
            
            router_config = "-group router_config_"+pe+" "
            
            dmni_group = "-group {DMNI "+pe+"} "
            pe_path = signal_path
            signal_path = pe_path+"/dmni"
            send_ps = "-group send_"+pe+"_PS "
            receive_ps = "-group receive_"+pe+"_PS "
            
            wave_lines.append(pe_group + dmni_group + signal_path + "/operation\n");
            
                    
            wave_lines.append(pe_group + dmni_group + send_ps + signal_path + "/tx\n") 
            wave_lines.append(pe_group + dmni_group + send_ps + signal_path + "/data_out\n")
            wave_lines.append(pe_group + dmni_group + send_ps + signal_path + "/credit_i\n") 
            
            wave_lines.append(pe_group + dmni_group + receive_ps + signal_path + "/rx\n") 
            wave_lines.append(pe_group + dmni_group + receive_ps + signal_path + "/data_in\n") 
            wave_lines.append(pe_group + dmni_group + receive_ps + signal_path + "/credit_o\n") 
                
            config = "-group config_"+pe+" "
            
            wave_lines.append(pe_group + dmni_group + config + signal_path + "/set_address\n") 
            wave_lines.append(pe_group + dmni_group + config + signal_path + "/set_address_2\n") 
            wave_lines.append(pe_group + dmni_group + config + signal_path + "/set_size\n")
            wave_lines.append(pe_group + dmni_group + config + signal_path + "/set_size_2\n") 
            wave_lines.append(pe_group + dmni_group + config + signal_path + "/set_op\n") 
            wave_lines.append(pe_group + dmni_group + config + signal_path + "/start\n") 
            wave_lines.append(pe_group + dmni_group + config + signal_path + "/config_data\n") 
            
            
            input_name = "EAST", "WEST", "NORTH", "SOUTH", "LOCAL"
            signal_path = pe_path+"router/"
            router_group = "-group {router "+pe+"} "
            wave_lines.append(pe_group + router_group + "-divider receive\n") 
            count=0
            for input in input_name:
                p = str(count)
                i_name = "input_"+input;
                input_group = "-group {"+i_name+" "+pe+"} "
                wave_lines.append(pe_group + router_group + input_group + signal_path + "rx("+p+")\n") 
                wave_lines.append(pe_group + router_group + input_group + signal_path + "data_in("+p+")\n")  
                wave_lines.append(pe_group + router_group + input_group + signal_path + "credit_o("+p+")\n")
                count= count+1
                
            wave_lines.append(pe_group + router_group + "-divider send\n")
            count=0
            for input in input_name:
                p = str(count)
                i_name = "output_"+input;
                input_group = "-group {"+i_name+" "+pe+"} "
                wave_lines.append(pe_group + router_group + input_group + signal_path + "tx("+p+")\n") 
                wave_lines.append(pe_group + router_group + input_group + signal_path + "data_out("+p+")\n")  
                wave_lines.append(pe_group + router_group + input_group + signal_path + "credit_i("+p+")\n")  
                count = count+1;
            
    wave_lines.append("configure wave -signalnamewidth 1\n")
    wave_lines.append("configure wave -namecolwidth 217\n")
    wave_lines.append("configure wave -timelineunits ns\n")
    
    return  wave_lines
    
