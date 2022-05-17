-----------------------------------------------------------------------------------------------
--
--  DISTRIBUTED HEMPS  - version 5.0
--
--  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
--
--  Distribution:  June 2016
--
--  Source name:  Hermes_buffer.vhd
--
--  Brief description: Description of queues of flits
--
---------------------------------------------------------------------------------------
--                                      BUFFER
--                         --------------
--                   RX ->|             |-> H
--              DATA_IN ->|             |<- ACK_H
--             CLOCK_RX ->|             |
--             CREDIT_O <-|             |-> DATA_AV
--                        |             |-> DATA
--                        |             |<- DATA_ACK
--                        |             |
--                        |             |   SENDER
--                        |             |=> de todas
--                        |             |   as portas
--                         --------------
--
--  Quando o algoritmo de chaveamento resulta no bloqueio dos flits de um pacote,
--  ocorre uma perda de desempenho em toda rede de interconexão, porque os flits são
--  bloqueados não somente na chave atual, mas em todas as intermediárias.
--  Para diminuir a perda de desempenho foi adicionada uma fila em cada porta de
--  entrada da chave, reduzindo as chaves afetadas com o bloqueio dos flits de um
--  pacote. É importante observar que quanto maior for o tamanho da fila menor será o
--  número de chaves intermediárias afetadas.
--  As filas usadas contêm dimensão e largura de flit parametrizáveis, para alterá-las
--  modifique as constantes TAM_BUFFER e TAM_FLIT no arquivo "packet.vhd".
--  As filas funcionam como FIFOs circulares. Cada fila possui dois ponteiros: first e
--  last. First aponta para a posição da fila onde se encontra o flit a ser consumido.
--  Last aponta para a posição onde deve ser inserido o próximo flit.
---------------------------------------------------------------------------------------
library IEEE;
use IEEE.std_logic_1164.all;  
use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;
use work.standards.all;
use work.hemps_pkg.all;

-- interface da Hermes_buffer
entity Hermes_buffer is
generic(
		TAM_BUFFER  : integer := 8;
		buffer_type : string := "SIM"
);
port(
		clock       	: in  std_logic;
		reset       	: in  std_logic;      
		rx          	: in  std_logic;
		eop_in      	: in  std_logic;
		data_in     	: in  regflit;
		credit_out  	: out std_logic;
		req_routing 	: out std_logic;
		ack_routing 	: in  std_logic; 
		tx          	: out std_logic;
		eop_out     	: out std_logic;
		data_out    	: out regflit;
		--data_header 	: out regflit_32;
		header_routing 	: out regflit_32;
		header_fixed 	: out regflit_32;
		credit_in   	: in  std_logic;
		sender      	: out std_logic;
		next_flit   	: in  std_logic;
		fail_in 		: in  std_logic
		);
end Hermes_buffer;

architecture Hermes_buffer of Hermes_buffer is

type fila_out is (S_INIT, S_PAYLOAD, S_SENDHEADER, S_HEADER_HIGH, S_HEADER_LOW, S_SIZE_HIGH, S_END, S_END2);

type state is (S0, SA, S1, S2, S3);
signal EA : state;

signal eop: std_logic_vector(TAM_BUFFER-1 downto 0);
signal first,last: pointer;
signal available_slot, one_flit_in_buffer, tx_sig: std_logic;
signal data_read: regflit;

--signal for controlling header_routing low or high flits
signal header_high 				: std_logic;

-- Flit Buffer implemented using array.
signal flit_buff: buff;

signal flit_counter : std_logic_vector(2 downto 0);

	component RAM16X1D is			
		generic (
			INIT : bit_vector(15 downto 0) := X"0000"
		);			
		port (
			DPO   : out std_ulogic;        
			SPO   : out std_ulogic;			        
			A0    : in std_ulogic;
			A1    : in std_ulogic;
			A2    : in std_ulogic;
			A3    : in std_ulogic;
			D     : in std_ulogic;        
			DPRA0 : in std_ulogic;
			DPRA1 : in std_ulogic;
			DPRA2 : in std_ulogic;
			DPRA3 : in std_ulogic;        
			WCLK  : in std_ulogic;
			WE    : in std_ulogic
		);
	end component;
							   
begin 
	
	-- Flit Buffer implemented using LUT RAM.	
	buffer_RTL: if buffer_type = "RTL" generate	
		flit_buff: for i in 0 to TAM_FLIT-1 generate
			RAM16X1D_inst : RAM16X1D
			port map (
				DPO 	=> data_read(i), 		
				SPO 	=> open, 		
				A0 		=> last(0), 		
				A1 		=> last(1), 		
				A2 		=> last(2), 		
				A3 		=> last(3), 		
				D 		=> data_in(i), 			
				DPRA0 	=> first(0), 		
				DPRA1 	=> first(1), 		
				DPRA2 	=> first(2), 		
				DPRA3 	=> first(3), 		
				WCLK 	=> clock, 		
				WE 		=> '1' 			
			);
		end generate flit_buff;
	end generate;
	
	buffer_SIM: if buffer_type = "SIM" generate
		data_read <= flit_buff(CONV_INTEGER(first));
	end generate;
  

	-- Controls flit storing and avaliable buffer slots. 
	process(reset, clock)
	variable last_var,first_var,count: pointer;	-- Used only to update 'available_slot'.
	begin
		if reset='1' then			 	
			last 			<= (others=>'0');
			eop 			<= (others=>'0');
			available_slot 	<= '1';
			first_var 		:= (others=>'0');
			last_var 		:= (others=>'0');
			count 	 		:= (others=>'0');
			flit_counter 	<= (others=>'0');
			header_fixed	<= (others=>'0');
			header_routing	<= (others=>'0');

		elsif rising_edge(clock) then
			-- Stores a flit.
			if rx = '1' and available_slot = '1' then
				if buffer_type = "SIM" then
					flit_buff(CONV_INTEGER(last)) <= data_in;
				end if;
				eop(CONV_INTEGER(last)) <= eop_in;

				if not(flit_counter = "10" and data_in = x"7FFF") then
					last <= last + 1;
					last_var := last + 1;
				end if;

				if eop_in = '1' then
					flit_counter <= (others=>'0');
				elsif not(flit_counter = "10" and data_in = x"7FFF") then
					flit_counter <= flit_counter + '1';
				end if;

				case flit_counter is
					when "000" =>--gets higher bits from header fixed
						header_fixed 	<= data_in & header_fixed(15 downto 0);
					when "001" =>--gets lower bits from header fixed
						header_fixed 	<= header_fixed(31 downto 16) & data_in;
					when "010" =>--gets higher bits from header routing
						if data_in /= x"7FFF" then
							header_routing 	<= data_in & header_routing(15 downto 0);
						end if;
					when "011" =>--gets lower bits from header routing
						header_routing 	<= header_routing(31 downto 16) & data_in;
					when others =>
				
				end case ;



			elsif fail_in = '1' then
				eop(CONV_INTEGER(last-1)) <= '1';
			end if;

			-- Verifies if a flit is transmited.
			if tx_sig = '1' and credit_in = '1' then
				first_var := first + 1;
			else
				first_var := first; 
			end if;
			
			-- Update the 'available_slot'.
			if (first_var = x"0" and last_var = TAM_BUFFER-1) or (first_var = last_var+1) then
				available_slot <= '0';
				count := "000";
			else
			--if ((last_var - first_var) = 2) or ((first_var - last_var) = TAM_BUFFER-2) then
				count := count + 1;
				if (count = x"5") then					
					available_slot <= '1';
				end if;	
			end if;
		end if;
	end process;
	
   
	-- Data ready to be transmited.
	data_out <= data_read;
	
	-- Indicates if the current flit is the last packet flit ('1').
	eop_out <= eop(CONV_INTEGER(first));
	
	--Indicates if exist at least one free slot in buffer ('1').
	credit_out <= available_slot;	
	
	-- Indicates one flit stored in the buffer.
	one_flit_in_buffer <= '1' when first+1 = last else '0';
		
	tx <= tx_sig;
	-- Routing request and packet flits transmition.
	process(reset, clock)
	begin
		if reset='1' then
			sender 			<='0';
			req_routing 	<= '0';
			tx_sig 			<= '0';
			first 			<= (others=>'0');
			EA 				<= S0;
			
		elsif rising_edge(clock) then
			case EA is
				-- waits one flit to be stored
				when S0 =>
					-- Waits for a stored flit.
					if first /= last then
						EA 				<= SA;
					else
						EA<= S0;
					end if;

				-- waits second flit
				when SA =>
					-- Waits for a stored flit.
					if rx = '1' then
							req_routing 	<= '1';
							EA 				<= S1;
					else
						EA<= SA;
					end if;
				
				-- Waits routing request acknolegment from Switch Contol.	
				when S1 => 	
					if ack_routing = '1' then
						req_routing <= '0';
						tx_sig 		<= '1';
						sender 		<= '1';
						EA 			<= S2 ;
					else
						EA 			<= S1;
					end if;
				 
				-- Send packet flits.
				when S2 =>
					if credit_in = '1' then
						first <= first + 1;
						
						-- Verifies if the current flit is the last packet flit.
						if eop(CONV_INTEGER(first)) = '1' then
							tx_sig <= '0';
							sender <= '0';

							if one_flit_in_buffer = '1' then
								EA <= S0;
							else
								EA 				<= SA;
							end if;

						
						-- Verifies if the current flit is the only stored flit.
						elsif one_flit_in_buffer = '1' then
							tx_sig <= '0';
							EA <= S3;
						else
							EA <= S2;
						end if;					
					
					else
						EA <= S2;
					end if;
					
				-- Waits for a stored flit.
				when S3 =>

					if first /= last then
						tx_sig <= '1';
						EA <= S2;	
					else
						EA <= S3;
					end if;
						
					
				when others =>
					EA <= S0;
			
			end case;
		end if;
	end process; 
end Hermes_buffer;
