---------------------------------------------------------------------------------------------------
--
-- Title       : Crossbars
-- Design      : QoS
-- Company     : GAPH
--
---------------------------------------------------------------------------------------------------
--
-- File        : Crossbars.vhd
-- Generated   : Thu Mar  6 17:08:38 2008
-- From        : interface description file
-- By          : Itf2Vhdl ver. 1.20
--
---------------------------------------------------------------------------------------------------
--							   
-- Description : Connects the input buffers to the router ports based on the Switch Control switching table.
--
---------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use work.standards.all;

entity Hermes_crossbar is
	port (
		table		: in matrixNportNport_std_logic;
	
		data		: in arrayNport_regflit;
		data_av		: in regNport;
		data_ack	: out regNport;
		eop_buffer	: in regNport;
		
		data_out	: out arrayNport_regflit;
		tx			: out regNport;
		eop_out		: out regNport;
		credit_i	: in regNport
	);
end Hermes_crossbar;

architecture Hermes_crossbar of Hermes_crossbar is
signal credit: regNport;

begin
-- Crossbar de dados
-- Liga barramentos de entrada 'data' a barramentos de saida 'data_out' baseado na tabela de roteamento 'table'.	
	data_out(EAST0) <= 	data(WEST0) when table(WEST0)(EAST0) = '1' else	(others=>'Z'); 
	data_out(EAST0) <= 	data(WEST1) when table(WEST1)(EAST0) = '1' else	(others=>'Z'); 
	data_out(EAST0) <= 	data(NORTH0) when table(NORTH0)(EAST0) = '1' else	(others=>'Z');
	data_out(EAST0) <= 	data(NORTH1) when table(NORTH1)(EAST0) = '1' else	(others=>'Z'); 
	data_out(EAST0) <= 	data(SOUTH0) when table(SOUTH0)(EAST0) = '1' else	(others=>'Z');
	data_out(EAST0) <= 	data(SOUTH1) when table(SOUTH1)(EAST0) = '1' else	(others=>'Z');	
	data_out(EAST0) <=	data(LOCAL0) when table(LOCAL0)(EAST0) = '1' else (others=>'Z');
	data_out(EAST0) <=	data(LOCAL1) when table(LOCAL1)(EAST0) = '1' else (others=>'Z');
	
	data_out(EAST1) <= 	data(WEST0) when table(WEST0)(EAST1) = '1' else	(others=>'Z'); 
	data_out(EAST1) <= 	data(WEST1) when table(WEST1)(EAST1) = '1' else	(others=>'Z'); 
	data_out(EAST1) <= 	data(NORTH0) when table(NORTH0)(EAST1) = '1' else	(others=>'Z');
	data_out(EAST1) <= 	data(NORTH1) when table(NORTH1)(EAST1) = '1' else	(others=>'Z'); 
	data_out(EAST1) <= 	data(SOUTH0) when table(SOUTH0)(EAST1) = '1' else	(others=>'Z');
	data_out(EAST1) <= 	data(SOUTH1) when table(SOUTH1)(EAST1) = '1' else	(others=>'Z');	
	data_out(EAST1) <=	data(LOCAL0) when table(LOCAL0)(EAST1) = '1' else (others=>'Z');
	data_out(EAST1) <=	data(LOCAL1) when table(LOCAL1)(EAST1) = '1' else (others=>'Z');
		 		
	data_out(WEST0) <= 	data(EAST0) when table(EAST0)(WEST0) = '1' else	(others=>'Z'); 
	data_out(WEST0) <= 	data(EAST1) when table(EAST1)(WEST0) = '1' else	(others=>'Z');
	data_out(WEST0) <= 	data(SOUTH0) when table(SOUTH0)(WEST0) = '1' else	(others=>'Z');
	data_out(WEST0) <= 	data(SOUTH1) when table(SOUTH1)(WEST0) = '1' else	(others=>'Z'); 
	data_out(WEST0) <= 	data(NORTH0) when table(NORTH0)(WEST0) = '1' else	(others=>'Z');
	data_out(WEST0) <= 	data(NORTH1) when table(NORTH1)(WEST0) = '1' else	(others=>'Z'); 
	data_out(WEST0) <=	data(LOCAL0) when table(LOCAL0)(WEST0) = '1' else (others=>'Z');
	data_out(WEST0) <=	data(LOCAL1) when table(LOCAL1)(WEST0) = '1' else (others=>'Z');  
	
	data_out(WEST1) <= 	data(EAST0) when table(EAST0)(WEST1) = '1' else	(others=>'Z'); 
	data_out(WEST1) <= 	data(EAST1) when table(EAST1)(WEST1) = '1' else	(others=>'Z');
	data_out(WEST1) <= 	data(SOUTH0) when table(SOUTH0)(WEST1) = '1' else	(others=>'Z');
	data_out(WEST1) <= 	data(SOUTH1) when table(SOUTH1)(WEST1) = '1' else	(others=>'Z'); 
	data_out(WEST1) <= 	data(NORTH0) when table(NORTH0)(WEST1) = '1' else	(others=>'Z');
	data_out(WEST1) <= 	data(NORTH1) when table(NORTH1)(WEST1) = '1' else	(others=>'Z'); 
	data_out(WEST1) <=	data(LOCAL0) when table(LOCAL0)(WEST1) = '1' else (others=>'Z');
	data_out(WEST1) <=	data(LOCAL1) when table(LOCAL1)(WEST1) = '1' else (others=>'Z');  
		
	data_out(SOUTH0) <=	data(EAST0) when table(EAST0)(SOUTH0) = '1' else (others=>'Z');	 
	data_out(SOUTH0) <=	data(EAST1) when table(EAST1)(SOUTH0) = '1' else (others=>'Z');	 
	data_out(SOUTH0) <=	data(WEST0) when table(WEST0)(SOUTH0) = '1' else (others=>'Z');
	data_out(SOUTH0) <=	data(WEST1) when table(WEST1)(SOUTH0) = '1' else (others=>'Z');	
	data_out(SOUTH0) <=	data(NORTH0) when table(NORTH0)(SOUTH0) = '1' else (others=>'Z');
	data_out(SOUTH0) <=	data(NORTH1) when table(NORTH1)(SOUTH0) = '1' else (others=>'Z');
	data_out(SOUTH0) <=	data(LOCAL0) when table(LOCAL0)(SOUTH0) = '1' else (others=>'Z');
	data_out(SOUTH0) <=	data(LOCAL1) when table(LOCAL1)(SOUTH0) = '1' else (others=>'Z'); 
	
	data_out(SOUTH1) <=	data(EAST0) when table(EAST0)(SOUTH1) = '1' else (others=>'Z');	 
	data_out(SOUTH1) <=	data(EAST1) when table(EAST1)(SOUTH1) = '1' else (others=>'Z');	 
	data_out(SOUTH1) <=	data(WEST0) when table(WEST0)(SOUTH1) = '1' else (others=>'Z');
	data_out(SOUTH1) <=	data(WEST1) when table(WEST1)(SOUTH1) = '1' else (others=>'Z');	
	data_out(SOUTH1) <=	data(NORTH0) when table(NORTH0)(SOUTH1) = '1' else (others=>'Z');
	data_out(SOUTH1) <=	data(NORTH1) when table(NORTH1)(SOUTH1) = '1' else (others=>'Z');
	data_out(SOUTH1) <=	data(LOCAL0) when table(LOCAL0)(SOUTH1) = '1' else (others=>'Z');
	data_out(SOUTH1) <=	data(LOCAL1) when table(LOCAL1)(SOUTH1) = '1' else (others=>'Z'); 
	
	data_out(NORTH0) <=	data(EAST0) when table(EAST0)(NORTH0) = '1' else (others=>'Z');
	data_out(NORTH0) <=	data(EAST1) when table(EAST1)(NORTH0) = '1' else (others=>'Z');
	data_out(NORTH0) <=	data(WEST0) when table(WEST0)(NORTH0) = '1' else (others=>'Z');
	data_out(NORTH0) <=	data(WEST1) when table(WEST1)(NORTH0) = '1' else (others=>'Z');	
	data_out(NORTH0) <=	data(SOUTH0) when table(SOUTH0)(NORTH0) = '1' else (others=>'Z');
	data_out(NORTH0) <=	data(SOUTH1) when table(SOUTH1)(NORTH0) = '1' else (others=>'Z'); 
	data_out(NORTH0) <=	data(LOCAL0) when table(LOCAL0)(NORTH0) = '1' else (others=>'Z');
	data_out(NORTH0) <=	data(LOCAL1) when table(LOCAL1)(NORTH0) = '1' else (others=>'Z'); 
		
	data_out(NORTH1) <=	data(EAST0) when table(EAST0)(NORTH1) = '1' else (others=>'Z');
	data_out(NORTH1) <=	data(EAST1) when table(EAST1)(NORTH1) = '1' else (others=>'Z');
	data_out(NORTH1) <=	data(WEST0) when table(WEST0)(NORTH1) = '1' else (others=>'Z');
	data_out(NORTH1) <=	data(WEST1) when table(WEST1)(NORTH1) = '1' else (others=>'Z');	
	data_out(NORTH1) <=	data(SOUTH0) when table(SOUTH0)(NORTH1) = '1' else (others=>'Z');
	data_out(NORTH1) <=	data(SOUTH1) when table(SOUTH1)(NORTH1) = '1' else (others=>'Z'); 
	data_out(NORTH1) <=	data(LOCAL0) when table(LOCAL0)(NORTH1) = '1' else (others=>'Z');
	data_out(NORTH1) <=	data(LOCAL1) when table(LOCAL1)(NORTH1) = '1' else (others=>'Z'); 
	
	data_out(LOCAL0) <=	data(EAST0) when table(EAST0)(LOCAL0) = '1' else (others=>'Z');
	data_out(LOCAL0) <=	data(EAST1) when table(EAST1)(LOCAL0) = '1' else (others=>'Z');	
	data_out(LOCAL0) <=	data(WEST0) when table(WEST0)(LOCAL0) = '1' else (others=>'Z');
	data_out(LOCAL0) <=	data(WEST1) when table(WEST1)(LOCAL0) = '1' else (others=>'Z');	
	data_out(LOCAL0) <=	data(SOUTH0) when table(SOUTH0)(LOCAL0) = '1' else (others=>'Z');
	data_out(LOCAL0) <=	data(SOUTH1) when table(SOUTH1)(LOCAL0) = '1' else (others=>'Z');
	data_out(LOCAL0) <=	data(NORTH0) when table(NORTH0)(LOCAL0) = '1' else (others=>'Z');
	data_out(LOCAL0) <=	data(NORTH1) when table(NORTH1)(LOCAL0) = '1' else (others=>'Z');
			
	data_out(LOCAL1) <=	data(EAST0) when table(EAST0)(LOCAL1) = '1' else (others=>'Z');
	data_out(LOCAL1) <=	data(EAST1) when table(EAST1)(LOCAL1) = '1' else (others=>'Z');	
	data_out(LOCAL1) <=	data(WEST0) when table(WEST0)(LOCAL1) = '1' else (others=>'Z');
	data_out(LOCAL1) <=	data(WEST1) when table(WEST1)(LOCAL1) = '1' else (others=>'Z');	
	data_out(LOCAL1) <=	data(SOUTH0) when table(SOUTH0)(LOCAL1) = '1' else (others=>'Z');
	data_out(LOCAL1) <=	data(SOUTH1) when table(SOUTH1)(LOCAL1) = '1' else (others=>'Z');
	data_out(LOCAL1) <=	data(NORTH0) when table(NORTH0)(LOCAL1) = '1' else (others=>'Z');
	data_out(LOCAL1) <=	data(NORTH1) when table(NORTH1)(LOCAL1) = '1' else (others=>'Z');
	
-- Crossbar do controle de fluxo
-- Liga os sinais de entrada 'credit_i' aos sinais 'credit' baseado na tabela de roteamento.
	credit(EAST0)	<=	credit_i(WEST0) when credit_i(WEST0) = '0' and table (EAST0)(WEST0)   = '1' else
						credit_i(WEST1) when credit_i(WEST1) = '0' and table (EAST0)(WEST1)   = '1' else   
						credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(EAST0)(SOUTH0) = '1' else
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(EAST0)(SOUTH1) = '1' else  
						credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(EAST0)(NORTH0) = '1' else
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(EAST0)(NORTH1) = '1' else 	
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(EAST0)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(EAST0)(LOCAL1) = '1' else
						'1';
						
	credit(EAST1)	<=	credit_i(WEST0) when credit_i(WEST0) = '0' and table (EAST1)(WEST0)   = '1' else
						credit_i(WEST1) when credit_i(WEST1) = '0' and table (EAST1)(WEST1)   = '1' else   
						credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(EAST1)(SOUTH0) = '1' else
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(EAST1)(SOUTH1) = '1' else  
						credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(EAST1)(NORTH0) = '1' else
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(EAST1)(NORTH1) = '1' else 	
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(EAST1)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(EAST1)(LOCAL1) = '1' else
						'1';
							
	credit(WEST0) <= 	credit_i(EAST0) when credit_i(EAST0) = '0' and table (WEST0)(EAST0)   = '1' else
						credit_i(EAST1) when credit_i(EAST1) = '0' and table (WEST0)(EAST1)   = '1' else  
						credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(WEST0)(SOUTH0) = '1' else 
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(WEST0)(SOUTH1) = '1' else 
						credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(WEST0)(NORTH0) = '1' else 	
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(WEST0)(NORTH1) = '1' else 
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(WEST0)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(WEST0)(LOCAL1) = '1' else
						'1';
						
	credit(WEST1) <= 	credit_i(EAST0) when credit_i(EAST0) = '0' and table (WEST1)(EAST0)   = '1' else
						credit_i(EAST1) when credit_i(EAST1) = '0' and table (WEST1)(EAST1)   = '1' else  
						credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(WEST1)(SOUTH0) = '1' else 
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(WEST1)(SOUTH1) = '1' else 
						credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(WEST1)(NORTH0) = '1' else 	
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(WEST1)(NORTH1) = '1' else 
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(WEST1)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(WEST1)(LOCAL1) = '1' else
						'1';
							
	credit(SOUTH0)<= 	credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(SOUTH0)(NORTH0) = '1' else 
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(SOUTH0)(NORTH1) = '1' else  
						credit_i(EAST0) when credit_i(EAST0) = '0' and table(SOUTH0)(EAST0) = '1' else	 
						credit_i(EAST1) when credit_i(EAST1) = '0' and table(SOUTH0)(EAST1) = '1' else
						credit_i(WEST0) when credit_i(WEST0) = '0' and table(SOUTH0)(WEST0) = '1' else	
						credit_i(WEST1) when credit_i(WEST1) = '0' and table(SOUTH0)(WEST1) = '1' else
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(SOUTH0)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(SOUTH0)(LOCAL1) = '1' else
						'1';
						
	credit(SOUTH1)<= 	credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(SOUTH1)(NORTH0) = '1' else 
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(SOUTH1)(NORTH1) = '1' else  
						credit_i(EAST0) when credit_i(EAST0) = '0' and table(SOUTH1)(EAST0) = '1' else	 
						credit_i(EAST1) when credit_i(EAST1) = '0' and table(SOUTH1)(EAST1) = '1' else
						credit_i(WEST0) when credit_i(WEST0) = '0' and table(SOUTH1)(WEST0) = '1' else	
						credit_i(WEST1) when credit_i(WEST1) = '0' and table(SOUTH1)(WEST1) = '1' else
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(SOUTH1)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(SOUTH1)(LOCAL1) = '1' else
						'1';
							
	credit(NORTH0)<= 	credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(NORTH0)(SOUTH0) = '1' else
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(NORTH0)(SOUTH1) = '1' else 
						credit_i(EAST0) when credit_i(EAST0) = '0' and table(NORTH0)(EAST0) = '1' else	
						credit_i(EAST1) when credit_i(EAST1) = '0' and table(NORTH0)(EAST1) = '1' else
						credit_i(WEST0) when credit_i(WEST0) = '0' and table(NORTH0)(WEST0) = '1' else	
						credit_i(WEST1) when credit_i(WEST1) = '0' and table(NORTH0)(WEST1) = '1' else
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(NORTH0)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(NORTH0)(LOCAL1) = '1' else
						'1';
						
	credit(NORTH1)<= 	credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(NORTH1)(SOUTH0) = '1' else
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(NORTH1)(SOUTH1) = '1' else 
						credit_i(EAST0) when credit_i(EAST0) = '0' and table(NORTH1)(EAST0) = '1' else	
						credit_i(EAST1) when credit_i(EAST1) = '0' and table(NORTH1)(EAST1) = '1' else
						credit_i(WEST0) when credit_i(WEST0) = '0' and table(NORTH1)(WEST0) = '1' else	
						credit_i(WEST1) when credit_i(WEST1) = '0' and table(NORTH1)(WEST1) = '1' else
						credit_i(LOCAL0) when credit_i(LOCAL0) = '0' and table(NORTH1)(LOCAL0) = '1' else
						credit_i(LOCAL1) when credit_i(LOCAL1) = '0' and table(NORTH1)(LOCAL1) = '1' else
						'1';
							
	credit(LOCAL0)<= 	credit_i(EAST0) when credit_i(EAST0) = '0' and table (LOCAL0)(EAST0)   = '1' else
						credit_i(EAST1) when credit_i(EAST1) = '0' and table (LOCAL0)(EAST1)   = '1' else 
						credit_i(WEST0) when credit_i(WEST0) = '0' and table (LOCAL0)(WEST0)   = '1' else 
						credit_i(WEST1) when credit_i(WEST1) = '0' and table (LOCAL0)(WEST1)   = '1' else
						credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(LOCAL0)(SOUTH0) = '1' else 
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(LOCAL0)(SOUTH1) = '1' else
						credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(LOCAL0)(NORTH0) = '1' else
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(LOCAL0)(NORTH1) = '1' else  
						'1';
						
	credit(LOCAL1)<= 	credit_i(EAST0) when credit_i(EAST0) = '0' and table (LOCAL1)(EAST0)   = '1' else
						credit_i(EAST1) when credit_i(EAST1) = '0' and table (LOCAL1)(EAST1)   = '1' else 
						credit_i(WEST0) when credit_i(WEST0) = '0' and table (LOCAL1)(WEST0)   = '1' else 
						credit_i(WEST1) when credit_i(WEST1) = '0' and table (LOCAL1)(WEST1)   = '1' else
						credit_i(SOUTH0) when credit_i(SOUTH0) = '0' and table(LOCAL1)(SOUTH0) = '1' else 
						credit_i(SOUTH1) when credit_i(SOUTH1) = '0' and table(LOCAL1)(SOUTH1) = '1' else
						credit_i(NORTH0) when credit_i(NORTH0) = '0' and table(LOCAL1)(NORTH0) = '1' else
						credit_i(NORTH1) when credit_i(NORTH1) = '0' and table(LOCAL1)(NORTH1) = '1' else  
						'1';
			  	   					
						
-- Crossbar que liga os sinais de entrada 'data_av' aos sinais de saida 'tx' 
-- baseado na tabela de roteamento 'table'.
-- 'tx' depende dos sinais 'credit' devido ao multicast.
	tx(EAST0) <=	data_av(WEST0) when table(WEST0)(EAST0) = '1'  and credit(WEST0) = '1' else
					data_av(WEST1) when table(WEST1)(EAST0) = '1'  and credit(WEST1) = '1' else 	
					data_av(NORTH0) when table(NORTH0)(EAST0) = '1' and credit(NORTH0) = '1' else 
					data_av(NORTH1) when table(NORTH1)(EAST0) = '1' and credit(NORTH1) = '1' else
					data_av(SOUTH0) when table(SOUTH0)(EAST0) = '1' and credit(SOUTH0) = '1' else 
					data_av(SOUTH1) when table(SOUTH1)(EAST0) = '1' and credit(SOUTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(EAST0) = '1' and credit(LOCAL0) = '1' else
					data_av(LOCAL1) when table(LOCAL1)(EAST0) = '1' and credit(LOCAL1) = '1' else 
					'0';
					
	tx(EAST1) <=	data_av(WEST0) when table(WEST0)(EAST1) = '1'  and credit(WEST0) = '1' else
					data_av(WEST1) when table(WEST1)(EAST1) = '1'  and credit(WEST1) = '1' else 	
					data_av(NORTH0) when table(NORTH0)(EAST1) = '1' and credit(NORTH0) = '1' else 
					data_av(NORTH1) when table(NORTH1)(EAST1) = '1' and credit(NORTH1) = '1' else
					data_av(SOUTH0) when table(SOUTH0)(EAST1) = '1' and credit(SOUTH0) = '1' else 
					data_av(SOUTH1) when table(SOUTH1)(EAST1) = '1' and credit(SOUTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(EAST1) = '1' and credit(LOCAL0) = '1' else 
					data_av(LOCAL1) when table(LOCAL1)(EAST1) = '1' and credit(LOCAL1) = '1' else 
					'0';
			
	tx(WEST0) <= 	data_av(EAST0) when table(EAST0)(WEST0) = '1' and credit(EAST0) = '1'  else
					data_av(EAST1) when table(EAST1)(WEST0) = '1' and credit(EAST1) = '1'  else	 
					data_av(SOUTH0) when table(SOUTH0)(WEST0) = '1' and credit(SOUTH0) = '1' else
					data_av(SOUTH1) when table(SOUTH1)(WEST0) = '1' and credit(SOUTH1) = '1' else
					data_av(NORTH0) when table(NORTH0)(WEST0) = '1' and credit(NORTH0) = '1' else	
					data_av(NORTH1) when table(NORTH1)(WEST0) = '1' and credit(NORTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(WEST0) = '1' and credit(LOCAL0) = '1' else
					data_av(LOCAL1) when table(LOCAL1)(WEST0) = '1' and credit(LOCAL1) = '1' else 
					'0';
					
	tx(WEST1) <= 	data_av(EAST0) when table(EAST0)(WEST1) = '1' and credit(EAST0) = '1'  else
					data_av(EAST1) when table(EAST1)(WEST1) = '1' and credit(EAST1) = '1'  else	 
					data_av(SOUTH0) when table(SOUTH0)(WEST1) = '1' and credit(SOUTH0) = '1' else
					data_av(SOUTH1) when table(SOUTH1)(WEST1) = '1' and credit(SOUTH1) = '1' else
					data_av(NORTH0) when table(NORTH0)(WEST1) = '1' and credit(NORTH0) = '1' else	
					data_av(NORTH1) when table(NORTH1)(WEST1) = '1' and credit(NORTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(WEST1) = '1' and credit(LOCAL0) = '1' else
					data_av(LOCAL1) when table(LOCAL1)(WEST1) = '1' and credit(LOCAL1) = '1' else 
					'0';
							
	tx(SOUTH0) <=	data_av(EAST0) when table(EAST0)(SOUTH0) = '1' and credit(EAST0) = '1' else
					data_av(EAST1) when table(EAST1)(SOUTH0) = '1' and credit(EAST1) = '1' else 
					data_av(WEST0) when table(WEST0)(SOUTH0) = '1' and credit(WEST0) = '1' else  
					data_av(WEST1) when table(WEST1)(SOUTH0) = '1' and credit(WEST1) = '1' else
					data_av(NORTH0) when table(NORTH0)(SOUTH0) = '1' and credit(NORTH0) = '1' else 	
					data_av(NORTH1) when table(NORTH1)(SOUTH0) = '1' and credit(NORTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(SOUTH0) = '1' and credit(LOCAL0) = '1' else
					data_av(LOCAL1) when table(LOCAL1)(SOUTH0) = '1' and credit(LOCAL1) = '1' else 	
					'0';
					
	tx(SOUTH1) <=	data_av(EAST0) when table(EAST0)(SOUTH1) = '1' and credit(EAST0) = '1' else
					data_av(EAST1) when table(EAST1)(SOUTH1) = '1' and credit(EAST1) = '1' else 
					data_av(WEST0) when table(WEST0)(SOUTH1) = '1' and credit(WEST0) = '1' else  
					data_av(WEST1) when table(WEST1)(SOUTH1) = '1' and credit(WEST1) = '1' else
					data_av(NORTH0) when table(NORTH0)(SOUTH1) = '1' and credit(NORTH0) = '1' else 	
					data_av(NORTH1) when table(NORTH1)(SOUTH1) = '1' and credit(NORTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(SOUTH1) = '1' and credit(LOCAL0) = '1' else
					data_av(LOCAL1) when table(LOCAL1)(SOUTH1) = '1' and credit(LOCAL1) = '1' else 	
					'0';
		
	tx(NORTH0) <=	data_av(EAST0) when table(EAST0)(NORTH0) = '1' and credit(EAST0) = '1' else
					data_av(EAST1) when table(EAST1)(NORTH0) = '1' and credit(EAST1) = '1' else 
					data_av(WEST0) when table(WEST0)(NORTH0) = '1' and credit(WEST0) = '1' else  
					data_av(WEST1) when table(WEST1)(NORTH0) = '1' and credit(WEST1) = '1' else
					data_av(SOUTH0) when table(SOUTH0)(NORTH0) = '1' and credit(SOUTH0) = '1' else 	
					data_av(SOUTH1) when table(SOUTH1)(NORTH0) = '1' and credit(SOUTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(NORTH0) = '1' and credit(LOCAL0) = '1' else 
					data_av(LOCAL1) when table(LOCAL1)(NORTH0) = '1' and credit(LOCAL1) = '1' else 
					'0';
					
	tx(NORTH1) <=	data_av(EAST0) when table(EAST0)(NORTH1) = '1' and credit(EAST0) = '1' else
					data_av(EAST1) when table(EAST1)(NORTH1) = '1' and credit(EAST1) = '1' else 
					data_av(WEST0) when table(WEST0)(NORTH1) = '1' and credit(WEST0) = '1' else  
					data_av(WEST1) when table(WEST1)(NORTH1) = '1' and credit(WEST1) = '1' else
					data_av(SOUTH0) when table(SOUTH0)(NORTH1) = '1' and credit(SOUTH0) = '1' else 	
					data_av(SOUTH1) when table(SOUTH1)(NORTH1) = '1' and credit(SOUTH1) = '1' else
					data_av(LOCAL0) when table(LOCAL0)(NORTH1) = '1' and credit(LOCAL0) = '1' else
					data_av(LOCAL1) when table(LOCAL1)(NORTH1) = '1' and credit(LOCAL1) = '1' else 
					'0';
			
	tx(LOCAL0) <=	data_av(EAST0) when table(EAST0)(LOCAL0) = '1' and credit(EAST0) = '1' else
					data_av(EAST1) when table(EAST1)(LOCAL0) = '1' and credit(EAST1) = '1'else 	
					data_av(WEST0) when table(WEST0)(LOCAL0) = '1' and credit(WEST0) = '1' else 	
					data_av(WEST1) when table(WEST1)(LOCAL0) = '1' and credit(WEST1) = '1' else
					data_av(SOUTH0) when table(SOUTH0)(LOCAL0) = '1' and credit(SOUTH0) = '1' else 
					data_av(SOUTH1) when table(SOUTH1)(LOCAL0) = '1' and credit(SOUTH1) = '1' else
					data_av(NORTH0) when table(NORTH0)(LOCAL0) = '1'and credit(NORTH0) = '1'  else 
					data_av(NORTH1) when table(NORTH1)(LOCAL0) = '1'and credit(NORTH1) = '1'  else
					'0';
					
	tx(LOCAL1) <=	data_av(EAST0) when table(EAST0)(LOCAL1) = '1' and credit(EAST0) = '1' else
					data_av(EAST1) when table(EAST1)(LOCAL1) = '1' and credit(EAST1) = '1' else 	
					data_av(WEST0) when table(WEST0)(LOCAL1) = '1' and credit(WEST0) = '1' else 	
					data_av(WEST1) when table(WEST1)(LOCAL1) = '1' and credit(WEST1) = '1' else
					data_av(SOUTH0) when table(SOUTH0)(LOCAL1) = '1' and credit(SOUTH0) = '1' else 
					data_av(SOUTH1) when table(SOUTH1)(LOCAL1) = '1' and credit(SOUTH1) = '1' else
					data_av(NORTH0) when table(NORTH0)(LOCAL1) = '1'and credit(NORTH0) = '1'  else 
					data_av(NORTH1) when table(NORTH1)(LOCAL1) = '1'and credit(NORTH1) = '1'  else
					'0'; 


--eop_out
	eop_out(EAST0) <=	eop_buffer(WEST0) when table(WEST0)(EAST0) = '1'  and credit(WEST0) = '1' else
						eop_buffer(WEST1) when table(WEST1)(EAST0) = '1'  and credit(WEST1) = '1' else 	
						eop_buffer(NORTH0) when table(NORTH0)(EAST0) = '1' and credit(NORTH0) = '1' else 
						eop_buffer(NORTH1) when table(NORTH1)(EAST0) = '1' and credit(NORTH1) = '1' else
						eop_buffer(SOUTH0) when table(SOUTH0)(EAST0) = '1' and credit(SOUTH0) = '1' else 
						eop_buffer(SOUTH1) when table(SOUTH1)(EAST0) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(EAST0) = '1' and credit(LOCAL0) = '1' else
						eop_buffer(LOCAL1) when table(LOCAL1)(EAST0) = '1' and credit(LOCAL1) = '1' else 
						'0';
					
	eop_out(EAST1) <=	eop_buffer(WEST0) when table(WEST0)(EAST1) = '1'  and credit(WEST0) = '1' else
						eop_buffer(WEST1) when table(WEST1)(EAST1) = '1'  and credit(WEST1) = '1' else 	
						eop_buffer(NORTH0) when table(NORTH0)(EAST1) = '1' and credit(NORTH0) = '1' else 
						eop_buffer(NORTH1) when table(NORTH1)(EAST1) = '1' and credit(NORTH1) = '1' else
						eop_buffer(SOUTH0) when table(SOUTH0)(EAST1) = '1' and credit(SOUTH0) = '1' else 
						eop_buffer(SOUTH1) when table(SOUTH1)(EAST1) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(EAST1) = '1' and credit(LOCAL0) = '1' else 
						eop_buffer(LOCAL1) when table(LOCAL1)(EAST1) = '1' and credit(LOCAL1) = '1' else 
						'0';
			
	eop_out(WEST0) <= 	eop_buffer(EAST0) when table(EAST0)(WEST0) = '1' and credit(EAST0) = '1'  else
						eop_buffer(EAST1) when table(EAST1)(WEST0) = '1' and credit(EAST1) = '1'  else	 
						eop_buffer(SOUTH0) when table(SOUTH0)(WEST0) = '1' and credit(SOUTH0) = '1' else
						eop_buffer(SOUTH1) when table(SOUTH1)(WEST0) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(NORTH0) when table(NORTH0)(WEST0) = '1' and credit(NORTH0) = '1' else	
						eop_buffer(NORTH1) when table(NORTH1)(WEST0) = '1' and credit(NORTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(WEST0) = '1' and credit(LOCAL0) = '1' else
						eop_buffer(LOCAL1) when table(LOCAL1)(WEST0) = '1' and credit(LOCAL1) = '1' else 
						'0';
					
	eop_out(WEST1) <= 	eop_buffer(EAST0) when table(EAST0)(WEST1) = '1' and credit(EAST0) = '1'  else
						eop_buffer(EAST1) when table(EAST1)(WEST1) = '1' and credit(EAST1) = '1'  else	 
						eop_buffer(SOUTH0) when table(SOUTH0)(WEST1) = '1' and credit(SOUTH0) = '1' else
						eop_buffer(SOUTH1) when table(SOUTH1)(WEST1) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(NORTH0) when table(NORTH0)(WEST1) = '1' and credit(NORTH0) = '1' else	
						eop_buffer(NORTH1) when table(NORTH1)(WEST1) = '1' and credit(NORTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(WEST1) = '1' and credit(LOCAL0) = '1' else
						eop_buffer(LOCAL1) when table(LOCAL1)(WEST1) = '1' and credit(LOCAL1) = '1' else 
						'0';
							
	eop_out(SOUTH0) <=	eop_buffer(EAST0) when table(EAST0)(SOUTH0) = '1' and credit(EAST0) = '1' else
						eop_buffer(EAST1) when table(EAST1)(SOUTH0) = '1' and credit(EAST1) = '1' else 
						eop_buffer(WEST0) when table(WEST0)(SOUTH0) = '1' and credit(WEST0) = '1' else  
						eop_buffer(WEST1) when table(WEST1)(SOUTH0) = '1' and credit(WEST1) = '1' else
						eop_buffer(NORTH0) when table(NORTH0)(SOUTH0) = '1' and credit(NORTH0) = '1' else 	
						eop_buffer(NORTH1) when table(NORTH1)(SOUTH0) = '1' and credit(NORTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(SOUTH0) = '1' and credit(LOCAL0) = '1' else
						eop_buffer(LOCAL1) when table(LOCAL1)(SOUTH0) = '1' and credit(LOCAL1) = '1' else 	
						'0';
					
	eop_out(SOUTH1) <=	eop_buffer(EAST0) when table(EAST0)(SOUTH1) = '1' and credit(EAST0) = '1' else
						eop_buffer(EAST1) when table(EAST1)(SOUTH1) = '1' and credit(EAST1) = '1' else 
						eop_buffer(WEST0) when table(WEST0)(SOUTH1) = '1' and credit(WEST0) = '1' else  
						eop_buffer(WEST1) when table(WEST1)(SOUTH1) = '1' and credit(WEST1) = '1' else
						eop_buffer(NORTH0) when table(NORTH0)(SOUTH1) = '1' and credit(NORTH0) = '1' else 	
						eop_buffer(NORTH1) when table(NORTH1)(SOUTH1) = '1' and credit(NORTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(SOUTH1) = '1' and credit(LOCAL0) = '1' else
						eop_buffer(LOCAL1) when table(LOCAL1)(SOUTH1) = '1' and credit(LOCAL1) = '1' else 	
						'0';
		
	eop_out(NORTH0) <=	eop_buffer(EAST0) when table(EAST0)(NORTH0) = '1' and credit(EAST0) = '1' else
						eop_buffer(EAST1) when table(EAST1)(NORTH0) = '1' and credit(EAST1) = '1' else 
						eop_buffer(WEST0) when table(WEST0)(NORTH0) = '1' and credit(WEST0) = '1' else  
						eop_buffer(WEST1) when table(WEST1)(NORTH0) = '1' and credit(WEST1) = '1' else
						eop_buffer(SOUTH0) when table(SOUTH0)(NORTH0) = '1' and credit(SOUTH0) = '1' else 	
						eop_buffer(SOUTH1) when table(SOUTH1)(NORTH0) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(NORTH0) = '1' and credit(LOCAL0) = '1' else 
						eop_buffer(LOCAL1) when table(LOCAL1)(NORTH0) = '1' and credit(LOCAL1) = '1' else 
						'0';
					
	eop_out(NORTH1) <=	eop_buffer(EAST0) when table(EAST0)(NORTH1) = '1' and credit(EAST0) = '1' else
						eop_buffer(EAST1) when table(EAST1)(NORTH1) = '1' and credit(EAST1) = '1' else 
						eop_buffer(WEST0) when table(WEST0)(NORTH1) = '1' and credit(WEST0) = '1' else  
						eop_buffer(WEST1) when table(WEST1)(NORTH1) = '1' and credit(WEST1) = '1' else
						eop_buffer(SOUTH0) when table(SOUTH0)(NORTH1) = '1' and credit(SOUTH0) = '1' else 	
						eop_buffer(SOUTH1) when table(SOUTH1)(NORTH1) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(LOCAL0) when table(LOCAL0)(NORTH1) = '1' and credit(LOCAL0) = '1' else
						eop_buffer(LOCAL1) when table(LOCAL1)(NORTH1) = '1' and credit(LOCAL1) = '1' else 
						'0';
			
	eop_out(LOCAL0) <=	eop_buffer(EAST0) when table(EAST0)(LOCAL0) = '1' and credit(EAST0) = '1' else
						eop_buffer(EAST1) when table(EAST1)(LOCAL0) = '1' and credit(EAST1) = '1'else 	
						eop_buffer(WEST0) when table(WEST0)(LOCAL0) = '1' and credit(WEST0) = '1' else 	
						eop_buffer(WEST1) when table(WEST1)(LOCAL0) = '1' and credit(WEST1) = '1' else
						eop_buffer(SOUTH0) when table(SOUTH0)(LOCAL0) = '1' and credit(SOUTH0) = '1' else 
						eop_buffer(SOUTH1) when table(SOUTH1)(LOCAL0) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(NORTH0) when table(NORTH0)(LOCAL0) = '1'and credit(NORTH0) = '1'  else 
						eop_buffer(NORTH1) when table(NORTH1)(LOCAL0) = '1'and credit(NORTH1) = '1'  else
						'0';
					
	eop_out(LOCAL1) <=	eop_buffer(EAST0) when table(EAST0)(LOCAL1) = '1' and credit(EAST0) = '1' else
						eop_buffer(EAST1) when table(EAST1)(LOCAL1) = '1' and credit(EAST1) = '1' else 	
						eop_buffer(WEST0) when table(WEST0)(LOCAL1) = '1' and credit(WEST0) = '1' else 	
						eop_buffer(WEST1) when table(WEST1)(LOCAL1) = '1' and credit(WEST1) = '1' else
						eop_buffer(SOUTH0) when table(SOUTH0)(LOCAL1) = '1' and credit(SOUTH0) = '1' else 
						eop_buffer(SOUTH1) when table(SOUTH1)(LOCAL1) = '1' and credit(SOUTH1) = '1' else
						eop_buffer(NORTH0) when table(NORTH0)(LOCAL1) = '1'and credit(NORTH0) = '1'  else 
						eop_buffer(NORTH1) when table(NORTH1)(LOCAL1) = '1'and credit(NORTH1) = '1'  else
						'0'; 

	data_ack <= credit;
end Hermes_crossbar;




	
