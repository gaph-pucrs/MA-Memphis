//------------------------------------------------------------------------------------------------
//
//  DISTRIBUTED MEMPHIS -  5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2013
//
//  Source name:  ram_generator.c
//
//  Brief description:  This source receives parametrizable memory and creates the files related to memory
//
//------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#define BUF_SIZE (1024*1024)

void vhd_ram_parametrized(unsigned int* code, int count, unsigned int ram_size, char *type)
{
    unsigned char init[128][32];
    unsigned char mask;
    char *type_ram;
    int j, i, k, line, index, max_mem = 1024, max_loop = 16, ram_size_flow;
    ram_size_flow = ram_size;

    //Definition of how many times the loop will be executed
    for(i=0; i<5; i++){
    	if(ram_size == max_mem){
    		max_loop = max_loop; // :)
    	}else{
    		max_mem = (max_mem/2);
    		max_loop = (max_loop/2);
    	}
    }
	
	printf("library ieee;\n");
	printf("use ieee.std_logic_1164.all;\n");
	printf("use ieee.std_logic_misc.all;\n");
	printf("use ieee.std_logic_arith.all;\n");
	printf("use ieee.std_logic_unsigned.all;\n");
	//printf("use work.mlite_pack.all;\n\n");

	printf("library UNISIM;\n");
	printf("use UNISIM.vcomponents.all;\n\n");

	//printf("%d, %d", max_loop, max_mem);

	printf("entity ram_%s is\n",type);
	printf("	port(clk             : in  std_logic;\n");        
	printf("        address_a       : in  std_logic_vector(31 downto 2);\n");
	printf("        enable_a        : in  std_logic;\n");
	printf("        wbe_a           : in  std_logic_vector(3 downto 0);\n");
	printf("        data_write_a    : in  std_logic_vector(31 downto 0);\n");
	printf("        data_read_a     : out std_logic_vector(31 downto 0);\n\n");

	printf("        address_b       : in  std_logic_vector(31 downto 2);\n");
	printf("        enable_b        : in  std_logic;\n");
	printf("        wbe_b           : in  std_logic_vector(3 downto 0);\n");
	printf("        data_write_b    : in  std_logic_vector(31 downto 0);\n");
	printf("        data_read_b     : out std_logic_vector(31 downto 0));\n");
	printf("end; --entity ram     \n\n");

	printf("architecture ram_%s of ram_%s is\n",type,type);
	if(ram_size == 64)
		printf("begin\n");

	if(ram_size > 64){
		//sinais para 128
		printf("signal enable_a_lo       : std_logic;\n");
	    printf("signal wbe_a_lo          : std_logic_vector(3 downto 0);\n");
	    printf("signal data_write_a_lo   : std_logic_vector(31 downto 0);\n");
	    printf("signal data_read_a_lo    : std_logic_vector(31 downto 0);\n");
	    
	    printf("signal enable_b_lo       : std_logic;\n");
	    printf("signal wbe_b_lo          : std_logic_vector(3 downto 0);\n");
	    printf("signal data_read_b_lo    : std_logic_vector(31 downto 0);\n");
	    
	    printf("signal enable_a_hi       : std_logic;\n");
	    printf("signal wbe_a_hi          : std_logic_vector(3 downto 0);\n");
	    printf("signal data_read_a_hi   : std_logic_vector(31 downto 0);\n");
	        
	    printf("signal enable_b_hi       : std_logic;\n");
	    printf("signal wbe_b_hi          : std_logic_vector(3 downto 0);\n");
	    printf("signal data_read_b_hi    : std_logic_vector(31 downto 0);\n");
	    
	    printf("signal address_a_reg     : std_logic_vector(31 downto 2);\n");
	    printf("signal address_b_reg     : std_logic_vector(31 downto 2);\n");

	    if(ram_size > 128){
		    //sinais para 256
		    printf("signal enable_a_lo_256       : std_logic;\n");
		    printf("signal wbe_a_lo_256          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_write_a_lo_256   : std_logic_vector(31 downto 0);\n");
		    printf("signal data_read_a_lo_256    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_b_lo_256       : std_logic;\n");
		    printf("signal wbe_b_lo_256          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_lo_256    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_a_hi_256       : std_logic;\n");
		    printf("signal wbe_a_hi_256          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_a_hi_256   : std_logic_vector(31 downto 0);\n");
		        
		    printf("signal enable_b_hi_256       : std_logic;\n");
		    printf("signal wbe_b_hi_256          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_hi_256    : std_logic_vector(31 downto 0);\n");
		    

		if(ram_size > 256){
			//sinais para 512
		    printf("signal enable_a_lo_512       : std_logic;\n");
		    printf("signal wbe_a_lo_512          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_write_a_lo_512   : std_logic_vector(31 downto 0);\n");
		    printf("signal data_read_a_lo_512    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_b_lo_512       : std_logic;\n");
		    printf("signal wbe_b_lo_512          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_lo_512    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_a_hi_512       : std_logic;\n");
		    printf("signal wbe_a_hi_512          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_a_hi_512   : std_logic_vector(31 downto 0);\n");
		        
		    printf("signal enable_b_hi_512       : std_logic;\n");
		    printf("signal wbe_b_hi_512          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_hi_512    : std_logic_vector(31 downto 0);\n");

		    printf("signal enable_a_lo_512_2       : std_logic;\n");
		    printf("signal wbe_a_lo_512_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_write_a_lo_512_2   : std_logic_vector(31 downto 0);\n");
		    printf("signal data_read_a_lo_512_2    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_b_lo_512_2       : std_logic;\n");
		    printf("signal wbe_b_lo_512_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_lo_512_2    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_a_hi_512_2       : std_logic;\n");
		    printf("signal wbe_a_hi_512_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_a_hi_512_2   : std_logic_vector(31 downto 0);\n");
		        
		    printf("signal enable_b_hi_512_2       : std_logic;\n");
		    printf("signal wbe_b_hi_512_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_hi_512_2    : std_logic_vector(31 downto 0);\n");
		
			if(ram_size > 512){
				//sinais para 1024
		    printf("signal enable_a_lo_1024       : std_logic;\n");
		    printf("signal wbe_a_lo_1024          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_write_a_lo_1024   : std_logic_vector(31 downto 0);\n");
		    printf("signal data_read_a_lo_1024    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_b_lo_1024       : std_logic;\n");
		    printf("signal wbe_b_lo_1024          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_lo_1024    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_a_hi_1024       : std_logic;\n");
		    printf("signal wbe_a_hi_1024          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_a_hi_1024   : std_logic_vector(31 downto 0);\n");
		        
		    printf("signal enable_b_hi_1024       : std_logic;\n");
		    printf("signal wbe_b_hi_1024          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_hi_1024    : std_logic_vector(31 downto 0);\n");

		    printf("signal enable_a_lo_1024_2       : std_logic;\n");
		    printf("signal wbe_a_lo_1024_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_write_a_lo_1024_2   : std_logic_vector(31 downto 0);\n");
		    printf("signal data_read_a_lo_1024_2    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_b_lo_1024_2       : std_logic;\n");
		    printf("signal wbe_b_lo_1024_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_lo_1024_2    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_a_hi_1024_2       : std_logic;\n");
		    printf("signal wbe_a_hi_1024_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_a_hi_1024_2   : std_logic_vector(31 downto 0);\n");
		        
		    printf("signal enable_b_hi_1024_2       : std_logic;\n");
		    printf("signal wbe_b_hi_1024_2          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_hi_1024_2    : std_logic_vector(31 downto 0);\n");

		    //sinais para 1024
		    printf("signal enable_a_lo_1024_3       : std_logic;\n");
		    printf("signal wbe_a_lo_1024_3          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_write_a_lo_1024_3   : std_logic_vector(31 downto 0);\n");
		    printf("signal data_read_a_lo_1024_3    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_b_lo_1024_3       : std_logic;\n");
		    printf("signal wbe_b_lo_1024_3          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_lo_1024_3    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_a_hi_1024_3       : std_logic;\n");
		    printf("signal wbe_a_hi_1024_3          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_a_hi_1024_3   : std_logic_vector(31 downto 0);\n");
		        
		    printf("signal enable_b_hi_1024_3       : std_logic;\n");
		    printf("signal wbe_b_hi_1024_3          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_hi_1024_3    : std_logic_vector(31 downto 0);\n");

		    printf("signal enable_a_lo_1024_4       : std_logic;\n");
		    printf("signal wbe_a_lo_1024_4          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_write_a_lo_1024_4   : std_logic_vector(31 downto 0);\n");
		    printf("signal data_read_a_lo_1024_4    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_b_lo_1024_4       : std_logic;\n");
		    printf("signal wbe_b_lo_1024_4          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_lo_1024_4    : std_logic_vector(31 downto 0);\n");
		    
		    printf("signal enable_a_hi_1024_4       : std_logic;\n");
		    printf("signal wbe_a_hi_1024_4          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_a_hi_1024_4   : std_logic_vector(31 downto 0);\n");
		        
		    printf("signal enable_b_hi_1024_4       : std_logic;\n");
		    printf("signal wbe_b_hi_1024_4          : std_logic_vector(3 downto 0);\n");
		    printf("signal data_read_b_hi_1024_4    : std_logic_vector(31 downto 0);\n");
			}
		}
	}

		printf("begin\n");
	    printf("process(clk)\n");
	    printf("begin\n");
	    printf("if rising_edge(clk) then\n");
	    printf("	address_a_reg <= address_a;\n");
	    printf("	address_b_reg <= address_b;\n");
	    printf("	end if;\n");
	    printf("end process;\n");
	}

	if(ram_size == 128){
	   
	    printf("data_read_a <= data_read_a_lo when (address_a_reg < x\"0001000\"&\"00\") else\n");
	    printf("data_read_a_hi when ((address_a_reg >= x\"0001000\"&\"00\") and (address_a_reg < x\"0002000\"&\"00\"));\n");
	    printf("data_read_b <= data_read_b_lo when (address_b_reg < x\"0001000\"&\"00\") else\n");
	    printf("data_read_b_hi when ((address_b_reg >= x\"0001000\"&\"00\") and (address_b_reg < x\"0002000\"&\"00\"));\n");
	   
	    printf("enable_a_lo <= enable_a when (address_a < x\"0001000\"&\"00\") else '0';\n");
	    printf("enable_b_lo <= enable_b when (address_b < x\"0001000\"&\"00\") else '0';\n");	    
	    printf("enable_a_hi <= enable_a when ((address_a >= x\"0001000\"&\"00\") and (address_a < x\"0002000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi <= enable_b when ((address_b >= x\"0001000\"&\"00\") and (address_b < x\"0002000\"&\"00\")) else '0';\n");
	               
	    printf("wbe_a_lo <= wbe_a when  enable_a_lo='1' else x\"0\";\n");
	    printf("wbe_a_hi <= wbe_a when  enable_a_hi='1' else x\"0\";\n");
	    printf("wbe_b_lo <= wbe_b when  enable_b_lo='1' else x\"0\";\n");
	    printf("wbe_b_hi <= wbe_b when  enable_b_hi='1' else x\"0\";\n");
	    printf("\n\n\n");
	}

    if(ram_size == 256){
    	printf("data_read_a <= data_read_a_lo when (address_a_reg < x\"0001000\"&\"00\") else\n");
   		printf("data_read_a_hi when ((address_a_reg >= x\"0001000\"&\"00\") and (address_a_reg < x\"0002000\"&\"00\")) else \n");
	    printf("data_read_a_lo_256 when ((address_a_reg >= x\"0002000\"&\"00\") and (address_a_reg < x\"0003000\"&\"00\")) else \n");
	    printf("data_read_a_hi_256 when ((address_a_reg >= x\"0003000\"&\"00\") and (address_a_reg < x\"0004000\"&\"00\"));\n");

	    printf("data_read_b <= data_read_b_lo when (address_b_reg < x\"0001000\"&\"00\") else\n");
    	printf("data_read_b_hi when ((address_b_reg >= x\"0001000\"&\"00\") and (address_b_reg < x\"0002000\"&\"00\")) else\n");
	    printf("data_read_b_lo_256 when ((address_b_reg >= x\"0002000\"&\"00\") and (address_a_reg < x\"0003000\"&\"00\")) else \n");
	    printf("data_read_b_hi_256 when ((address_b_reg >= x\"0003000\"&\"00\") and (address_b_reg< x\"0004000\"&\"00\"));\n");

	    printf("enable_a_lo <= enable_a when (address_a < x\"0001000\"&\"00\") else '0';\n");
	    printf("enable_b_lo <= enable_b when (address_b < x\"0001000\"&\"00\") else '0';\n");
	    printf("enable_a_hi <= enable_a when ((address_a >= x\"0001000\"&\"00\") and (address_a < x\"0002000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi <= enable_b when ((address_b >= x\"0001000\"&\"00\") and (address_b < x\"0002000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_256 <= enable_a when ((address_a >= x\"0002000\"&\"00\") and (address_a < x\"0003000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_256 <= enable_b when ((address_b >= x\"0002000\"&\"00\") and (address_b < x\"0003000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_256 <= enable_a when ((address_a >= x\"0003000\"&\"00\") and (address_a < x\"0004000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_256 <= enable_b when ((address_b >= x\"0003000\"&\"00\") and (address_b < x\"0004000\"&\"00\")) else '0';\n");

	    printf("wbe_a_lo <= wbe_a when  enable_a_lo='1' else x\"0\";\n");
	    printf("wbe_a_hi <= wbe_a when  enable_a_hi='1' else x\"0\";\n");
	    printf("wbe_b_lo <= wbe_b when  enable_b_lo='1' else x\"0\";\n");
	    printf("wbe_b_hi <= wbe_b when  enable_b_hi='1' else x\"0\";\n");
	    printf("wbe_a_lo_256 <= wbe_a when  enable_a_lo_256='1' else x\"0\";\n");
	    printf("wbe_a_hi_256 <= wbe_a when  enable_a_hi_256='1' else x\"0\";\n");
	    printf("wbe_b_lo_256 <= wbe_b when  enable_b_lo_256='1' else x\"0\";\n");
	    printf("wbe_b_hi_256 <= wbe_b when  enable_b_hi_256='1' else x\"0\";\n");
	    printf("\n\n\n");
	}

	if(ram_size == 512){
		printf("data_read_a <= data_read_a_lo when (address_a_reg < x\"0001000\"&\"00\") else\n");
   		printf("data_read_a_hi when ((address_a_reg >= x\"0001000\"&\"00\") and (address_a_reg < x\"0002000\"&\"00\")) else \n");
	    printf("data_read_a_lo_256 when ((address_a_reg >= x\"0002000\"&\"00\") and (address_a_reg < x\"0003000\"&\"00\")) else \n");
	    printf("data_read_a_hi_256 when ((address_a_reg >= x\"0003000\"&\"00\") and (address_a_reg < x\"0004000\"&\"00\")) else \n");
	    printf("data_read_a_lo_512 when ((address_a_reg >= x\"0004000\"&\"00\") and (address_a_reg < x\"0005000\"&\"00\")) else \n");
	    printf("data_read_a_hi_512 when ((address_a_reg >= x\"0005000\"&\"00\") and (address_a_reg < x\"0006000\"&\"00\")) else \n");
		printf("data_read_a_lo_512_2 when ((address_a_reg >= x\"0006000\"&\"00\") and (address_a_reg < x\"0007000\"&\"00\")) else \n");
	    printf("data_read_a_hi_512_2 when ((address_a_reg >= x\"0007000\"&\"00\") and (address_a_reg < x\"0008000\"&\"00\")); \n");

	    printf("data_read_b <= data_read_b_lo when (address_b_reg < x\"0001000\"&\"00\") else\n");
    	printf("data_read_b_hi when ((address_b_reg >= x\"0001000\"&\"00\") and (address_b_reg < x\"0002000\"&\"00\")) else\n");
	    printf("data_read_b_lo_256 when ((address_b_reg >= x\"0002000\"&\"00\") and (address_a_reg < x\"0003000\"&\"00\")) else \n");
	    printf("data_read_b_hi_256 when ((address_b_reg >= x\"0003000\"&\"00\") and (address_b_reg< x\"0004000\"&\"00\")) else \n");
	    printf("data_read_b_lo_512 when ((address_b_reg >= x\"0004000\"&\"00\") and (address_b_reg< x\"0005000\"&\"00\")) else \n");
	    printf("data_read_b_hi_512 when ((address_b_reg >= x\"0005000\"&\"00\") and (address_b_reg< x\"0006000\"&\"00\")) else \n");
		printf("data_read_b_lo_512_2 when ((address_b_reg >= x\"0006000\"&\"00\") and (address_b_reg< x\"0007000\"&\"00\")) else \n");
	    printf("data_read_b_hi_512_2 when ((address_b_reg >= x\"0007000\"&\"00\") and (address_b_reg< x\"0008000\"&\"00\")); \n");

	    printf("enable_a_lo <= enable_a when (address_a < x\"0001000\"&\"00\") else '0';\n");
	    printf("enable_b_lo <= enable_b when (address_b < x\"0001000\"&\"00\") else '0';\n");
	    printf("enable_a_hi <= enable_a when ((address_a >= x\"0001000\"&\"00\") and (address_a < x\"0002000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi <= enable_b when ((address_b >= x\"0001000\"&\"00\") and (address_b < x\"0002000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_256 <= enable_a when ((address_a >= x\"0002000\"&\"00\") and (address_a < x\"0003000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_256 <= enable_b when ((address_b >= x\"0002000\"&\"00\") and (address_b < x\"0003000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_256 <= enable_a when ((address_a >= x\"0003000\"&\"00\") and (address_a < x\"0004000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_256 <= enable_b when ((address_b >= x\"0003000\"&\"00\") and (address_b < x\"0004000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_512 <= enable_a when ((address_a >= x\"0004000\"&\"00\") and (address_a < x\"0005000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_512 <= enable_b when ((address_b >= x\"0004000\"&\"00\") and (address_b < x\"0005000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_512 <= enable_a when ((address_a >= x\"0005000\"&\"00\") and (address_a < x\"0006000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_512 <= enable_b when ((address_b >= x\"0005000\"&\"00\") and (address_b < x\"0006000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_512_2 <= enable_a when ((address_a >= x\"0006000\"&\"00\") and (address_a < x\"0007000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_512_2 <= enable_b when ((address_b >= x\"0006000\"&\"00\") and (address_b < x\"0007000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_512_2 <= enable_a when ((address_a >= x\"0007000\"&\"00\") and (address_a < x\"0008000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_512_2 <= enable_b when ((address_b >= x\"0007000\"&\"00\") and (address_b < x\"0008000\"&\"00\")) else '0';\n");

	    printf("wbe_a_lo <= wbe_a when  enable_a_lo='1' else x\"0\";\n");
	    printf("wbe_a_hi <= wbe_a when  enable_a_hi='1' else x\"0\";\n");
	    printf("wbe_b_lo <= wbe_b when  enable_b_lo='1' else x\"0\";\n");
	    printf("wbe_b_hi <= wbe_b when  enable_b_hi='1' else x\"0\";\n");
	    printf("wbe_a_lo_256 <= wbe_a when  enable_a_lo_256='1' else x\"0\";\n");
	    printf("wbe_a_hi_256 <= wbe_a when  enable_a_hi_256='1' else x\"0\";\n");
	    printf("wbe_b_lo_256 <= wbe_b when  enable_b_lo_256='1' else x\"0\";\n");
	    printf("wbe_b_hi_256 <= wbe_b when  enable_b_hi_256='1' else x\"0\";\n");
	    printf("wbe_a_lo_512 <= wbe_a when  enable_a_lo_512='1' else x\"0\";\n");
	    printf("wbe_a_hi_512 <= wbe_a when  enable_a_hi_512='1' else x\"0\";\n");
	    printf("wbe_b_lo_512 <= wbe_b when  enable_b_lo_512='1' else x\"0\";\n");
	    printf("wbe_b_hi_512 <= wbe_b when  enable_b_hi_512='1' else x\"0\";\n");
	    printf("wbe_a_lo_512_2 <= wbe_a when  enable_a_lo_512_2='1' else x\"0\";\n");
	    printf("wbe_a_hi_512_2 <= wbe_a when  enable_a_hi_512_2='1' else x\"0\";\n");
	    printf("wbe_b_lo_512_2 <= wbe_b when  enable_b_lo_512_2='1' else x\"0\";\n");
	    printf("wbe_b_hi_512_2 <= wbe_b when  enable_b_hi_512_2='1' else x\"0\";\n");
	    printf("\n\n\n");
	}

		if(ram_size == 1024){
		printf("data_read_a <= data_read_a_lo when (address_a_reg < x\"0001000\"&\"00\") else\n");
   		printf("data_read_a_hi when ((address_a_reg >= x\"0001000\"&\"00\") and (address_a_reg < x\"0002000\"&\"00\")) else \n");
	    printf("data_read_a_lo_256 when ((address_a_reg >= x\"0002000\"&\"00\") and (address_a_reg < x\"0003000\"&\"00\")) else \n");
	    printf("data_read_a_hi_256 when ((address_a_reg >= x\"0003000\"&\"00\") and (address_a_reg < x\"0004000\"&\"00\")) else \n");
	    printf("data_read_a_lo_512 when ((address_a_reg >= x\"0004000\"&\"00\") and (address_a_reg < x\"0005000\"&\"00\")) else \n");
	    printf("data_read_a_hi_512 when ((address_a_reg >= x\"0005000\"&\"00\") and (address_a_reg < x\"0006000\"&\"00\")) else \n");
		printf("data_read_a_lo_512_2 when ((address_a_reg >= x\"0006000\"&\"00\") and (address_a_reg < x\"0007000\"&\"00\")) else \n");
	    printf("data_read_a_hi_512_2 when ((address_a_reg >= x\"0007000\"&\"00\") and (address_a_reg < x\"0008000\"&\"00\")) else \n");
	    printf("data_read_a_lo_1024 when ((address_a_reg >= x\"0008000\"&\"00\") and (address_a_reg < x\"0009000\"&\"00\")) else \n");
	    printf("data_read_a_hi_1024 when ((address_a_reg >= x\"0009000\"&\"00\") and (address_a_reg < x\"000A000\"&\"00\")) else \n");
	    printf("data_read_a_lo_1024_2 when ((address_a_reg >= x\"000A000\"&\"00\") and (address_a_reg < x\"000B000\"&\"00\")) else \n");
	    printf("data_read_a_hi_1024_2 when ((address_a_reg >= x\"000B000\"&\"00\") and (address_a_reg < x\"000C000\"&\"00\")) else \n");
	    printf("data_read_a_lo_1024_3 when ((address_a_reg >= x\"000C000\"&\"00\") and (address_a_reg < x\"000D000\"&\"00\")) else \n");
	    printf("data_read_a_hi_1024_3 when ((address_a_reg >= x\"000D000\"&\"00\") and (address_a_reg < x\"000E000\"&\"00\")) else \n");
	    printf("data_read_a_lo_1024_4 when ((address_a_reg >= x\"000E000\"&\"00\") and (address_a_reg < x\"000F000\"&\"00\")) else \n");
	    printf("data_read_a_hi_1024_4 when ((address_a_reg >= x\"000F000\"&\"00\") and (address_a_reg < x\"0010000\"&\"00\")); \n");

	    printf("data_read_b <= data_read_b_lo when (address_b_reg < x\"0001000\"&\"00\") else\n");
    	printf("data_read_b_hi when ((address_b_reg >= x\"0001000\"&\"00\") and (address_b_reg < x\"0002000\"&\"00\")) else\n");
	    printf("data_read_b_lo_256 when ((address_b_reg >= x\"0002000\"&\"00\") and (address_a_reg < x\"0003000\"&\"00\")) else \n");
	    printf("data_read_b_hi_256 when ((address_b_reg >= x\"0003000\"&\"00\") and (address_b_reg< x\"0004000\"&\"00\")) else \n");
	    printf("data_read_b_lo_512 when ((address_b_reg >= x\"0004000\"&\"00\") and (address_b_reg< x\"0005000\"&\"00\")) else \n");
	    printf("data_read_b_hi_512 when ((address_b_reg >= x\"0005000\"&\"00\") and (address_b_reg< x\"0006000\"&\"00\")) else \n");
		printf("data_read_b_lo_512_2 when ((address_b_reg >= x\"0006000\"&\"00\") and (address_b_reg< x\"0007000\"&\"00\")) else \n");
	    printf("data_read_b_hi_512_2 when ((address_b_reg >= x\"0007000\"&\"00\") and (address_b_reg< x\"0008000\"&\"00\")) else \n");
	    printf("data_read_b_lo_1024 when ((address_b_reg >= x\"0008000\"&\"00\") and (address_b_reg< x\"0009000\"&\"00\")) else \n");
	    printf("data_read_b_hi_1024 when ((address_b_reg >= x\"0009000\"&\"00\") and (address_b_reg< x\"000A000\"&\"00\")) else \n");
	    printf("data_read_b_lo_1024_2 when ((address_b_reg >= x\"000A000\"&\"00\") and (address_b_reg< x\"000B000\"&\"00\")) else \n");
	    printf("data_read_b_hi_1024_2 when ((address_b_reg >= x\"000B000\"&\"00\") and (address_b_reg< x\"000C000\"&\"00\")) else \n");
	    printf("data_read_b_lo_1024_3 when ((address_b_reg >= x\"000C000\"&\"00\") and (address_b_reg< x\"000D000\"&\"00\")) else \n");
	    printf("data_read_b_hi_1024_3 when ((address_b_reg >= x\"000D000\"&\"00\") and (address_b_reg< x\"000E000\"&\"00\")) else \n");
	    printf("data_read_b_lo_1024_4 when ((address_b_reg >= x\"000E000\"&\"00\") and (address_b_reg< x\"000F000\"&\"00\")) else \n");
	    printf("data_read_b_hi_1024_4 when ((address_b_reg >= x\"000F000\"&\"00\") and (address_b_reg< x\"0010000\"&\"00\")); \n");

	    printf("enable_a_lo <= enable_a when (address_a < x\"0001000\"&\"00\") else '0';\n");
	    printf("enable_b_lo <= enable_b when (address_b < x\"0001000\"&\"00\") else '0';\n");
	    printf("enable_a_hi <= enable_a when ((address_a >= x\"0001000\"&\"00\") and (address_a < x\"0002000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi <= enable_b when ((address_b >= x\"0001000\"&\"00\") and (address_b < x\"0002000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_256 <= enable_a when ((address_a >= x\"0002000\"&\"00\") and (address_a < x\"0003000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_256 <= enable_b when ((address_b >= x\"0002000\"&\"00\") and (address_b < x\"0003000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_256 <= enable_a when ((address_a >= x\"0003000\"&\"00\") and (address_a < x\"0004000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_256 <= enable_b when ((address_b >= x\"0003000\"&\"00\") and (address_b < x\"0004000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_512 <= enable_a when ((address_a >= x\"0004000\"&\"00\") and (address_a < x\"0005000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_512 <= enable_b when ((address_b >= x\"0004000\"&\"00\") and (address_b < x\"0005000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_512 <= enable_a when ((address_a >= x\"0005000\"&\"00\") and (address_a < x\"0006000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_512 <= enable_b when ((address_b >= x\"0005000\"&\"00\") and (address_b < x\"0006000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_512_2 <= enable_a when ((address_a >= x\"0006000\"&\"00\") and (address_a < x\"0007000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_512_2 <= enable_b when ((address_b >= x\"0006000\"&\"00\") and (address_b < x\"0007000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_512_2 <= enable_a when ((address_a >= x\"0007000\"&\"00\") and (address_a < x\"0008000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_512_2 <= enable_b when ((address_b >= x\"0007000\"&\"00\") and (address_b < x\"0008000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_1024 <= enable_b when ((address_a >= x\"0008000\"&\"00\") and (address_a < x\"0009000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_1024 <= enable_b when ((address_b >= x\"0008000\"&\"00\") and (address_b < x\"0009000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_1024 <= enable_b when ((address_a >= x\"0009000\"&\"00\") and (address_a < x\"000A000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_1024 <= enable_b when ((address_b >= x\"0009000\"&\"00\") and (address_b < x\"000A000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_1024_2 <= enable_b when ((address_a >= x\"000A000\"&\"00\") and (address_a < x\"000B000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_1024_2 <= enable_b when ((address_b >= x\"000A000\"&\"00\") and (address_b < x\"000B000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_1024_2 <= enable_b when ((address_a >= x\"000B000\"&\"00\") and (address_a < x\"000C000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_1024_2 <= enable_b when ((address_b >= x\"000B000\"&\"00\") and (address_b < x\"000C000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_1024_3 <= enable_b when ((address_a >= x\"000C000\"&\"00\") and (address_a < x\"000D000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_1024_3 <= enable_b when ((address_b >= x\"000C000\"&\"00\") and (address_b < x\"000D000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_1024_3 <= enable_b when ((address_a >= x\"000D000\"&\"00\") and (address_a < x\"000E000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_1024_3 <= enable_b when ((address_b >= x\"000D000\"&\"00\") and (address_b < x\"000E000\"&\"00\")) else '0';\n");
	    printf("enable_a_lo_1024_4 <= enable_b when ((address_a >= x\"000E000\"&\"00\") and (address_a < x\"000F000\"&\"00\")) else '0';\n");
	    printf("enable_b_lo_1024_4 <= enable_b when ((address_b >= x\"000E000\"&\"00\") and (address_b < x\"000F000\"&\"00\")) else '0';\n");
	    printf("enable_a_hi_1024_4 <= enable_b when ((address_a >= x\"000F000\"&\"00\") and (address_a < x\"0010000\"&\"00\")) else '0';\n");
	    printf("enable_b_hi_1024_4 <= enable_b when ((address_b >= x\"000F000\"&\"00\") and (address_b < x\"0010000\"&\"00\")) else '0';\n");

	    printf("wbe_a_lo <= wbe_a when  enable_a_lo='1' else x\"0\";\n");
	    printf("wbe_a_hi <= wbe_a when  enable_a_hi='1' else x\"0\";\n");
	    printf("wbe_b_lo <= wbe_b when  enable_b_lo='1' else x\"0\";\n");
	    printf("wbe_b_hi <= wbe_b when  enable_b_hi='1' else x\"0\";\n");
	    printf("wbe_a_lo_256 <= wbe_a when  enable_a_lo_256='1' else x\"0\";\n");
	    printf("wbe_a_hi_256 <= wbe_a when  enable_a_hi_256='1' else x\"0\";\n");
	    printf("wbe_b_lo_256 <= wbe_b when  enable_b_lo_256='1' else x\"0\";\n");
	    printf("wbe_b_hi_256 <= wbe_b when  enable_b_hi_256='1' else x\"0\";\n");
	    printf("wbe_a_lo_512 <= wbe_a when  enable_a_lo_512='1' else x\"0\";\n");
	    printf("wbe_a_hi_512 <= wbe_a when  enable_a_hi_512='1' else x\"0\";\n");
	    printf("wbe_b_lo_512 <= wbe_b when  enable_b_lo_512='1' else x\"0\";\n");
	    printf("wbe_b_hi_512 <= wbe_b when  enable_b_hi_512='1' else x\"0\";\n");
	    printf("wbe_a_lo_512_2 <= wbe_a when  enable_a_lo_512_2='1' else x\"0\";\n");
	    printf("wbe_a_hi_512_2 <= wbe_a when  enable_a_hi_512_2='1' else x\"0\";\n");
	    printf("wbe_b_lo_512_2 <= wbe_b when  enable_b_lo_512_2='1' else x\"0\";\n");
	    printf("wbe_b_hi_512_2 <= wbe_b when  enable_b_hi_512_2='1' else x\"0\";\n");
	    printf("wbe_a_lo_1024 <= wbe_b when  enable_a_lo_1024 ='1' else x\"0\";\n");
	    printf("wbe_a_hi_1024 <= wbe_b when  enable_a_hi_1024 ='1' else x\"0\";\n");
	    printf("wbe_b_lo_1024 <= wbe_b when  enable_b_lo_1024 ='1' else x\"0\";\n");
	    printf("wbe_b_hi_1024 <= wbe_b when  enable_b_hi_1024 ='1' else x\"0\";\n");
	    printf("wbe_a_lo_1024_2 <= wbe_b when  enable_a_lo_1024_2='1' else x\"0\";\n");
	    printf("wbe_a_hi_1024_2 <= wbe_b when  enable_a_hi_1024_2='1' else x\"0\";\n");
	    printf("wbe_b_lo_1024_2 <= wbe_b when  enable_b_lo_1024_2='1' else x\"0\";\n");
	    printf("wbe_b_hi_1024_2 <= wbe_b when  enable_b_hi_1024_2='1' else x\"0\";\n");
	    printf("wbe_a_lo_1024_3 <= wbe_b when  enable_a_lo_1024_3='1' else x\"0\";\n");
	    printf("wbe_a_hi_1024_3 <= wbe_b when  enable_a_hi_1024_3='1' else x\"0\";\n");
	    printf("wbe_b_lo_1024_3 <= wbe_b when  enable_b_lo_1024_3='1' else x\"0\";\n");
	    printf("wbe_b_hi_1024_3 <= wbe_b when  enable_b_hi_1024_3='1' else x\"0\";\n");
	    printf("wbe_a_lo_1024_4 <= wbe_b when  enable_a_lo_1024_4='1' else x\"0\";\n");
	    printf("wbe_a_hi_1024_4 <= wbe_b when  enable_a_hi_1024_4='1' else x\"0\";\n");
	    printf("wbe_b_lo_1024_4 <= wbe_b when  enable_b_lo_1024_4='1' else x\"0\";\n");
	    printf("wbe_b_hi_1024_4 <= wbe_b when  enable_b_hi_1024_4='1' else x\"0\";\n");
	    printf("\n\n\n");
		}

	while(max_loop != 0){

		//printf("max_loop: %d, ram_size: %d, ram_size_flow: %d\n\n", max_loop, ram_size, ram_size_flow);

		 

		if(max_loop == 16 && ram_size == 1024){
		 	type_ram = "_lo";
		 }else if(max_loop == 15 && ram_size == 1024){
		 	type_ram = "_hi";
		 }else if(max_loop == 14 && ram_size == 1024){
		 	type_ram = "_lo_256";
		 }else if(max_loop == 13 && ram_size == 1024){
		 	type_ram = "_hi_256";
		 }else if(max_loop == 12 && ram_size == 1024){
		 	type_ram = "_lo_512";
		 }else if(max_loop == 11 && ram_size == 1024){
		 	type_ram = "_hi_512";
		 }else if(max_loop == 10 && ram_size == 1024){
		 	type_ram = "_lo_512_2";
		 }else if(max_loop == 9 && ram_size == 1024){
		 	type_ram = "_hi_512_2";
		 }else if(max_loop == 8 && ram_size == 1024){
		 	type_ram = "_lo_1024";
		 }else if(max_loop == 7 && ram_size == 1024){
		 	type_ram = "_hi_1024";
		 }else if(max_loop == 6 && ram_size == 1024){
		 	type_ram = "_lo_1024_2";
		 }else if(max_loop == 5 && ram_size == 1024){
		 	type_ram = "_hi_1024_2";
		 }else if(max_loop == 4 && ram_size == 1024){
		 	type_ram = "_lo_1024_3";
		 }else if(max_loop == 3 && ram_size == 1024){
		 	type_ram = "_hi_1024_3";
		 }else if(max_loop == 2 && ram_size == 1024){
		 	type_ram = "_lo_1024_4";
		 }else if(max_loop == 1 && ram_size == 1024){
		 	type_ram = "_hi_1024_4";
		 }else if(max_loop == 8 && ram_size == 512){
		 	type_ram = "_lo";
		 }else if(max_loop == 7 && ram_size == 512){
		 	type_ram = "_hi";
		 }else if(max_loop == 6 && ram_size == 512){
		 	type_ram = "_lo_256";
		 }else if(max_loop == 5 && ram_size == 512){
		 	type_ram = "_hi_256";
		 }else if(max_loop == 4 && ram_size == 512){
		 	type_ram = "_lo_512";
		 }else if(max_loop == 3 && ram_size == 512){
		 	type_ram = "_hi_512";
		 }else if(max_loop == 2 && ram_size == 512){
		 	type_ram = "_lo_512_2";
		 }else if(max_loop == 1 && ram_size == 512){
		 	type_ram = "_hi_512_2";
		 }else if(max_loop == 4 && ram_size == 256){
    	 	type_ram = "_lo";
    	 }else if(max_loop == 3 && ram_size == 256){
    	 	type_ram = "_hi";
    	 }else if(max_loop == 2 && ram_size == 256){
    	 	type_ram = "_lo_256";
    	 }else if(max_loop == 1 && ram_size == 256){
    	 	type_ram = "_hi_256";
    	 }else if(max_loop == 2 && ram_size == 128){
    	 	type_ram = "_lo";
    	 }else if(max_loop == 1 && ram_size == 128){
    	 	type_ram = "_hi";
    	 }

		for (j=0; j<32; j++)
		    {
		    	 

		        printf("ram_bit_%d_%d : RAMB16_S1_S1\n",j, max_loop);
		        printf("generic map (\n");

		        for (k=0; k<64; k++)         // initializes vectors with zeros
		            for (i = 0; i < 32; i++)
		                init[k][i] = 0;

		        for (i=0; i<count; i++)
		        {
		            mask = 1 << (i % 8);          // masks the init bit that should be evaluated
		            line = i / 256;               // init line number being generated
		            index = 31 - (i % 256) / 8;   // init index being generated

		            if (code[i] & (1 << j))       // sets an init bit
		                init[line][index] = init[line][index] | mask;
		        }

		        for (k = 0; k < 64; k++)
		        {
		            printf("INIT_%2.2X => X\"",k);
		            for (i = 0; i < 32; i++)
		                printf("%2.2X",init[k][i]);

		            printf("\"");
		            if (k != 63)
		                printf(",\n");
		        }


	 	        printf(")\n");
				printf("port map (\n");

		        if(ram_size == 64){
		        printf("CLKA     => clk, 					-- Port A clock input\n");
		        printf("ADDRA    => address_a(15 downto 2), -- Port A address input\n");
		        printf("ENA      => enable_a,               -- Port A enable input\n");
		        printf("WEA      => wbe_a(%d),               -- Port A write enable input\n",j/8);
		        printf("DIA(0)   => data_write_a(%d),        -- Port A data input\n",j);
		        printf("DOA(0)   => data_read_a(%d),         -- Port A data output\n",j);
		        printf("SSRA     => '0',                    -- Port A reset input\n");

		        printf("CLKB     => clk,                    -- Port B clock input\n");
		        printf("ADDRB    => address_b(15 downto 2), -- Port B address input\n");
		        printf("ENB      => enable_b,               -- Port B enable input\n");
		        printf("WEB      => wbe_b(%d),               -- Port B write enable input\n",j/8);
		        printf("DIB(0)   => data_write_b(%d),        -- Port B data input\n",j);
		        printf("DOB(0)   => data_read_b(%d),         -- Port B data output\n",j);
		        printf("SSRB     => '0'                     -- Port B reset input\n");
		        printf(");\n\n\n");
		    }else{
		    	printf("CLKA     => clk, 					-- Port A clock input\n");
		        printf("ADDRA    => address_a(15 downto 2), -- Port A address input\n");
		        printf("ENA      => enable_a%s,               -- Port A enable input\n",type_ram);
		        printf("WEA      => wbe_a%s(%d),               -- Port A write enable input\n",type_ram,j/8);
		        printf("DIA(0)   => data_write_a(%d),        -- Port A data input\n",j);
		        printf("DOA(0)   => data_read_a%s(%d),         -- Port A data output\n",type_ram,j);
		        printf("SSRA     => '0',                    -- Port A reset input\n");

		        printf("CLKB     => clk,                    -- Port B clock input\n");
		        printf("ADDRB    => address_b(15 downto 2), -- Port B address input\n");
		        printf("ENB      => enable_b%s,               -- Port B enable input\n",type_ram);
		        printf("WEB      => wbe_b%s(%d),               -- Port B write enable input\n",type_ram,j/8);
		        printf("DIB(0)   => data_write_b(%d),        -- Port B data input\n",j);
		        printf("DOB(0)   => data_read_b%s(%d),         -- Port B data output\n",type_ram,j);
		        printf("SSRB     => '0'                     -- Port B reset input\n");
		        printf(");\n\n\n");
		    }
	    }
	    max_loop--;
	    ram_size_flow = (ram_size_flow/2);
	}
	printf("end;");
}


void fill_c_model(unsigned int *code, unsigned int code_size, unsigned int ram_size, char *type) {

	unsigned int i;

	printf("/*** RAM memory initialized with kernel object code ***/\n\n");

	printf("#include <systemc.h>\n\n");

	printf("#define RAM_SIZE\t%d*1024\n\n",ram_size);

	printf("SC_MODULE(ram_%s) {\n\n",type);

	printf("\tsc_in< bool >\t\t\tclk;\n");

	printf("\tsc_in< sc_uint<30> >\taddress_a;\n");
	printf("\tsc_in< bool >\t\t\tenable_a;\n");
	printf("\tsc_in < sc_uint<4> >\twbe_a;\n");
	printf("\tsc_in < sc_uint<32> >\tdata_write_a;\n");
	printf("\tsc_out < sc_uint<32> >\tdata_read_a;\n\n");

	printf("\tsc_in< sc_uint<30> >\taddress_b;\n");
	printf("\tsc_in< bool >\t\t\tenable_b;\n");
	printf("\tsc_in < sc_uint<4> >\twbe_b;\n");
	printf("\tsc_in < sc_uint<32> >\tdata_write_b;\n");
	printf("\tsc_out < sc_uint<32> >\tdata_read_b;\n\n");

	printf("\tunsigned long ram[RAM_SIZE];\n");
	printf("\tunsigned long byte[4];\n");
	printf("\tunsigned long half_word[2];\n\n");

	printf("\tSC_CTOR(ram_%s) {\n\n",type);

	printf("\t\tSC_METHOD(read_a);\n");
	printf("\t\tsensitive << clk.pos();\n\n");

	printf("\t\tSC_METHOD(write_a);\n");
	printf("\t\tsensitive << clk.pos();\n\n");

	printf("\t\tSC_METHOD(read_b);\n");
	printf("\t\tsensitive << clk.pos();\n\n");

	printf("\t\tSC_METHOD(write_b);\n");
	printf("\t\tsensitive << clk.pos();\n\n");

	printf("\t\t// Byte masks.\n");

	printf("\t\tbyte[0] = 0x000000FF;\n");
	printf("\t\tbyte[1] = 0x0000FF00;\n");
	printf("\t\tbyte[2] = 0x00FF0000;\n");
	printf("\t\tbyte[3] = 0xFF000000;\n\n");

	printf("\t\t// Half word masks.\n");
	printf("\t\thalf_word[0] = 0x0000FFFF;\n");
	printf("\t\thalf_word[1] = 0xFFFF0000;\n\n");

	printf("\t\t// Initializes RAM memory with kernel object code.\n");
	for(i=0; i<code_size; i++)
		printf("\t\tram[0x%X] = 0x%08X;\n",i,code[i]);

	printf("\t}\n\n");

	printf("\t/*** Process functions ***/\n");
	printf("\tvoid read_a();\n");
	printf("\tvoid write_a();\n\n");
	printf("\tvoid read_b();\n");
	printf("\tvoid write_b();\n};\n");

}

void usage()
{
    printf("Usage: ram_generator <-128|-256|-512|-1024> <-vhd|-ucf> <model.txt>  <kernel_master.txt> <kernel_slave.txt> <repository.txt>\n\n");
    printf("  model.txt must have the tags $ram_master$ and $ram_slave$ when used\n ");
    printf("  with -vhd option and also $repository$ when used with -ucf \n");
    printf("  option.\n\n");
}

int main(int argc, char* argv[]){

    FILE *file;
    char *buf, *ram;
    unsigned int *code;
    int size;
	int i;

    buf = (char*)malloc(BUF_SIZE);
    code = (unsigned int*) malloc(BUF_SIZE);

    if(argc < 2){
        usage();
}
	//for(i=2;i<argc;i++){
		file = fopen(argv[3], "r");
		if(file == NULL){
			printf("Can't open file %s!\n",argv[3]);
			exit(-1);
		}
		i = atoi(argv[1]);

		for(size = 0; size < (i*1024); ++size){
			if(feof(file))
				break;
			fscanf(file, "%x", &code[size]);
		}
		fclose(file);
	
		if(size > ((i*1024)/4)){
			printf("ERROR! Kernel Size\n");
			exit(-1);
		}

			if (strcmp(argv[2],"-rtl") == 0){
				if (strcmp(argv[3],"kernel_master.txt") == 0){
					vhd_ram_parametrized(code, size, i, "master");
				}
				else if(strcmp(argv[3],"kernel_local.txt") == 0){
					vhd_ram_parametrized(code, size, i, "local");
				}
				else if(strcmp(argv[3],"kernel_mblite.txt") == 0){
					vhd_ram_parametrized(code, size, i, "mblite");
				}
				else if(strcmp(argv[3],"kernel_slave.txt") == 0){
					vhd_ram_parametrized(code, size, i, "slave");
				}
			}
			else if(strcmp(argv[2],"-h") == 0){
				if (strcmp(argv[3],"kernel_master.txt") == 0){
					fill_c_model(code, size, i, "master");
				}
				else if(strcmp(argv[3],"kernel_local.txt") == 0){
					fill_c_model(code, size, i, "local");
				}
				else if(strcmp(argv[3],"kernel_mblite.txt") == 0){
					fill_c_model(code, size, i, "mblite");
				}
				else if(strcmp(argv[3],"kernel_slave.txt") == 0){
					fill_c_model(code, size, i, "slave");
				}
			}
		else{
			usage();
		}
    return 0;
}
