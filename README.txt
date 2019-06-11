*************************************************
MEMPHIS - Many-core Modeling Platform in SoC	*	
												*
Version: 1.2 - jun/2019							*
												*
Authors: Marcelo Ruaro and Fernando Moraes      *
*************************************************

Prerequisites:
 - Ubuntu 16 +
 - Java
 - SystemC (only SystemC simmulations)
 - Python (with YAML plugin)
 - mips-cross compiler
 - Questa (only for VHDL simmulations)


To run MEMPHIS:
  
1. Download MEMPHIS

2. Set a MEMPHIS_PATH environment variable, pointing to the MEMPHIS root directory

3. Set a MEMPHIS_HOME (optional) pointing to any directory where you desire to store yours testcases. 
  If not defined, MEMPHIS will use as default the testcases directory present inside MEMPHIS_PATH.
  
4. Create a <my_testcase>.yaml file described in the yaml format. The file must have the following aspect:

hw:
   page_size_KB:  32        #(mandatory) specifies the page size, must be a value power of two, eg: 8, 16, 32, 64. Most commom value is 32
   tasks_per_PE: 2          #(mandatory) specifies the number of task per PE, must be a value higher than 0 and lower than 6. Most commom value is 2
   model_description: sc    #(mandatory) specifies the system model description: sc (gcc) | scmod (questa) | vhdl
   noc_buffer_size: 8       #(mandatory) must be power of 2.
   mpsoc_dimension: [2,2]   #(mandatory) [X,Y] size of MPSoC given by X times Y dimension
   cluster_dimension: [2,2] #(mandatory) [X,Y] size of a cluster given by X times Y dimension.
   Peripherals:             # Used to specify a external peripheral, MEMPHIS has by default one peripheral used to inject application from external world.
    - name: APP_INJECTOR    #(mandatory) Name of peripheral, this name must be the same that the macros and constant used by the platform to refer to peripheral
      pe: 1,1               #(mandatory) Edge of MPSoC where the peripheril is connected
      port: E               #(mandatory) Port (N-North, S-South, W-West, E-East) on the edge of MPSoC where the peripheril is connected 

5. The testcase file specifies hardware and kernel parameters, in order to execute application, you need to 
  create a scenario file to specifies application parameters. Thus, create a <my_scenario.yaml> file described in the yaml format. 
  The file must have the following aspect:
  
apps:                
  - name: mpeg        #(mandatory) name of application, must be equal to the application folder name
    cluster: 0        #(optinal) index of the statically mapped cluster - dynamic mapping by default
    start_time_ms: 15 #(optinal) any unsigned integer number - 0 by default
    static_mapping:   #(optinal) field, used to store static mapping information of tasks
      print: [1,1]    # Task print from app mpeg will be mapped as static at address X=1, Y=1
      start: [0,1]    # Task start from app mpeg will be mapped as static at address X=0, Y=1
  - name: prod_cons
  - name: dijkstra
  - name: dtw
      
6. Generate memphis testcase by calling memphis-gen, e.g.: memphis-gen <my_testcase>.yaml. A folder inside your MEMPHIS_HOME will be created with the testcase name.
   This folder containts the hardware and kernel source files and compiled files that enable the memphis execution
   
7. Compile your applications by calling memphis-app. This scripts enables two calling modes: 
      1. memphis-app <my_testcase>.yaml app1 app2 ...  				//specifying application per application
      2. memphis-app <my_testcase>.yaml -all <my_scenario.yaml> 	//specifying all the application present in the my_scenario.yaml
  You need to pass as application name the same application name that you create or a valid application name present inside the MEMPHIS_PATH/application directory

8. Generate the scenario and EXECUTE memphis by calling memphis-run, e.g.: memphis-run <my_testcase>.yaml <my_scenario.yaml> <sim_time>, where <sim_time> 
  is the simulation time (positive integer value)

*************************************************
