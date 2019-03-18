------------------------------------------------------------------------------------------------
--
--  DISTRIBUTED MEMPHIS  - version 5.0
--
--  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
--
--  Distribution:  September 2013
--
--  Source name:  test_bench.vhd
--
--  Brief description:  Test bench.
--
------------------------------------------------------------------------------------------------

library IEEE;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_textio.all;
use std.textio.all;
use work.memphis_pkg.all;
use work.standards.all;

--! @file
--! @ingroup vhdl_group
--! @{
--! @}

--! @brief entity brief description
 
--! @detailed detailed description
entity test_bench is
        
        --constant	log_file            : string := "output_master.txt"; --! port description
        constant	mlite_description   : string := "RTL";
     	constant	ram_description     : string := "RTL";
     	constant	router_description  : string := "RTL";
end;

architecture test_bench of test_bench is
	
	
        signal clock                      : std_logic := '0';
        signal reset                      : std_logic;
        
		-- IO signals connecting App Injector and Memphis
		signal memphis_injector_tx 		: std_logic;
 		signal memphis_injector_credit_i 	: std_logic;
	 	signal memphis_injector_data_out 	: regflit;
	 	
	 	signal memphis_injector_rx 		: std_logic;
 		signal memphis_injector_credit_o 	: std_logic;
	 	signal memphis_injector_data_in 	: regflit;
	 	
	 	-- Create the signals of your IO component here:
		
begin

	-- Peripheral 1 - Instantiation of App Injector 
	App_Injector : entity work.app_injector
	port map(
		clock        => clock,
		reset        => reset,
		
		rx			 => memphis_injector_tx,
		data_in		 => memphis_injector_data_out,
		credit_out   => memphis_injector_credit_i,
		
		tx			 => memphis_injector_rx,
		data_out	 => memphis_injector_data_in,
		credit_in	 => memphis_injector_credit_o
	);
	
	-- Peripheral 2 - Instantiate your IO component here:


   --
   --  Memphis instantiation 
   --
   	Memphis : entity work.Memphis
	port map(
		clock 				=> clock,
		reset 				=> reset,
		
		-- Peripheral 1 - App Injector
		memphis_app_injector_tx 		=> memphis_injector_tx,
		memphis_app_injector_credit_i => memphis_injector_credit_i,
		memphis_app_injector_data_out => memphis_injector_data_out,
		
		memphis_app_injector_rx		=> memphis_injector_rx,
		memphis_app_injector_credit_o	=> memphis_injector_credit_o,
		memphis_app_injector_data_in 	=> memphis_injector_data_in
		
		-- Peripheral 2 - Connect your IO component to Memphis here: 
			
	);
	   
	   
	   
	reset     <= '1', '0' after 100 ns;
	-- 100 MHz
	clock     <= not clock after 5 ns;
	

end test_bench;
