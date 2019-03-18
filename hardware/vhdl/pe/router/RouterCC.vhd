--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
-- Nesse arquivo vhdl existem dois hardware!!!!
-- 
-- traffic_monitor <=== Linha 28.
-- RouterCC <========== Linha 271.
--
-- O traffic_monitor serve para fazer o monitoramento das portas de entrada do 
-- router. Quando o sinal rx for igual a '1', esse hardware vai copia tudo que 
-- tiver em data_in para o arquivo traffic_router.txt, que se encontra na pasta 
-- debug do seu testcases.
-- Dependendo do nome de seu teste, sera necessário alterar a linha 135 com o 
-- caminho determinado para salvar o arquivo.
--
-- Caso seu objetivo seja realizar uma synthesis é aconselhável deletar esse 
-- hardware, e remover o mesmo da architecture RouterCC, que se encontra mais 
-- abaixo nesse arquivo.
--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use work.standards.all;
use ieee.std_logic_arith;
use IEEE.std_logic_textio.all;
use std.textio.all;
use IEEE.NUMERIC_STD.all;

entity traffic_monitor is
   generic( ID : integer );
port(
      clock :     in std_logic;
      reset :     in std_logic;
      data_in :   in regflit;
	  address :	in regmetadeflit;
      rx :        in std_logic;
      credit_o :  in std_logic );
end traffic_monitor;

architecture arq_traffic of traffic_monitor is
   type type_state is (Hheader, Ppayload, Sservice, task_allocation, task_menssage, cont_payload);
   signal SM_traffic_monitor : type_state;
   
   signal tick_counter          :  reg32;
	signal target_router         : regflit;
	signal header_time           : integer;
	signal bandwidth_allocation  : integer;
	signal payload               : integer;
	signal payload_counter       : regflit;
	signal service               : regflit;
	signal task_id               : regflit;
	signal consumer_id           : regflit;
	

begin
   process(clock,reset)
      variable fstatus      : FILE_OPEN_STATUS;        
      variable outline      : line;                                                                                                                  
      variable newAdress    : integer;
      file     traf_router  : TEXT;
      
   begin
      if reset = '1' then
         SM_traffic_monitor <= Hheader;
         bandwidth_allocation <= 0;
         tick_counter <= (others => '0');
         header_time <= 0;
      	target_router <= (others => '0');
      	payload <= 0;
      	payload_counter <= (others => '0');
      	service <= (others => '0');
      	task_id <= (others => '0');
      	consumer_id <= (others => '0');
      	
      elsif clock'event and clock = '1' then
      	
         if rx = '1' and credit_o = '1' then
            if payload_counter /= x"00000000" then -- <<<<<<< olhar depois
               payload_counter <= payload_counter - '1';
            end if;
            
            bandwidth_allocation <= bandwidth_allocation + 1;
            
            case( SM_traffic_monitor ) is
            
               when Hheader => 
                              if (rx = '1' and credit_o = '1') then
                                 SM_traffic_monitor <= Ppayload;
                              else
                                 SM_traffic_monitor <= Hheader;
                              end if;
                              target_router <= data_in and x"0000FFFF";
            						header_time <= conv_integer(tick_counter);
               
      			when Ppayload =>
      								payload <= conv_integer(data_in);
      								payload_counter <= data_in;
      								SM_traffic_monitor <= Sservice;
      								
      			when Sservice =>
      								service <= data_in;
      								
      								if data_in /= x"40" and data_in /= x"70" and data_in /= x"221" and
      								 	data_in /= x"10" and data_in /= x"20" then
      										SM_traffic_monitor <= cont_payload;
      								else
      										SM_traffic_monitor <= task_allocation;
      								end if;
      			
      			when task_allocation =>
      								task_id <= data_in;
      								
      								if  service = x"10" or service = x"20" then 
      									SM_traffic_monitor <= task_menssage;
      								else
      									SM_traffic_monitor <= cont_payload;
      								end if;
      								
      			when task_menssage =>
      								consumer_id <= data_in;
      								SM_traffic_monitor <= cont_payload;
      								
      			when cont_payload =>
      								SM_traffic_monitor <= cont_payload;

            end case; 
      		
      	elsif bandwidth_allocation > 0 then
      			bandwidth_allocation <= bandwidth_allocation + 1;         
         
         end if;
      	
         if rx = '0' and payload_counter = x"00000000" and SM_traffic_monitor = cont_payload then
            
               newAdress := to_integer(unsigned(address));

               file_open(fstatus, traf_router, "debug/traffic_router.txt", append_mode);
               
               if service /= x"40" and service /= x"70" and service /= x"221" and
                  service /= x"10" and service /= x"20" then
               
                     write(outline, header_time);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, newAdress);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, CONV_STRING_16BITS(service(15 downto 0)));
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, payload);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, bandwidth_allocation);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, ID);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, conv_integer(target_router));

               elsif  service = x"10" or service = x"20"  then
               
                     write(outline, header_time);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, newAdress);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, CONV_STRING_16BITS(service(15 downto 0)));
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, payload);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, bandwidth_allocation);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, ID);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, conv_integer(target_router));
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, conv_integer(task_id));
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, conv_integer(consumer_id));
               
               else
                  
                     write(outline, header_time);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, newAdress);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, CONV_STRING_16BITS(service(15 downto 0)));
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, payload);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, bandwidth_allocation);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, ID);
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, conv_integer(target_router));
                     write(outline, HT); -- <<< horizontal tab
                     write(outline, conv_integer(task_id));
                  
               end if;
               
               writeline(traf_router, outline);
               file_close(traf_router);
               bandwidth_allocation <= 0; 
               SM_traffic_monitor <= Hheader;
         
         end if;
         
         tick_counter <= tick_counter + 1;
            
      end if;
	end process;
   
end arq_traffic;
--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


------------------------------------------------------------------------------------------------
--
--  DISTRIBUTED MEMPHIS  - version 5.0
--
--  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
--
--  Distribution:  September 2013
--
--  Source name:  RouterCC.vhd
--
--  Brief description: Top module of the NoC - the NoC is built using only this module
--
---------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------- 
--                                              ROUTER
--
--                                              NORTH               LOCAL
--                       ---------------------------------------------
--                      |                       ******         ****** |
--                      |                       *FILA*         *FILA* |
--                      |                       ******         ****** |
--                      |                   *************             |
--                      |                   *  ARBITRO  *             |
--                      | ******            *************      ****** |
--                 WEST | *FILA*            *************      *FILA* | EAST
--                      | ******            *  CONTROLE *      ****** |
--                      |                   *************             |
--                      |                       ******                |
--                      |                       *FILA*                |
--                      |                       ******                |
--                      -----------------------------------------------
--                                              SOUTH
--
--  As chaves realizam a transfer�ncia de mensagens entre n�cleos. 
--  A chave possui uma l�gica de controle de chaveamento e 5 portas bidirecionais:
--  East, West, North, South e Local. Cada porta possui uma fila para o armazenamento 
--  tempor�rio de flits. A porta Local estabelece a comunica��o entre a chave e seu 
--  n�cleo. As demais portas ligam a chave �s chaves vizinhas.
--  Os endere�os das chaves s�o compostos pelas coordenadas XY da rede de interconex�o, 
--  onde X � a posi��o horizontal e Y a posi��o vertical. A atribui��o de endere�os �s 
--  chaves � necess�ria para a execu��o do algoritmo de chaveamento.
--  Os m�dulos principais que comp�em a chave s�o: fila, �rbitro e l�gica de 
--  chaveamento implementada pelo controle_mux. Cada uma das filas da chave (E, W, N, 
--  S e L), ao receber um novo pacote requisita chaveamento ao �rbitro. O �rbitro 
--  seleciona a requisi��o de maior prioridade, quando existem requisi��es simult�neas, 
--  e encaminha o pedido de chaveamento � l�gica de chaveamento. A l�gica de 
--  chaveamento verifica se � poss�vel atender � solicita��o. Sendo poss�vel, a conex�o
--  � estabelecida e o �rbitro � informado. Por sua vez, o �rbitro informa a fila que 
--  come�a a enviar os flits armazenados. Quando todos os flits do pacote foram 
--  enviados, a conex�o � conclu�da pela sinaliza��o, por parte da fila, atrav�s do 
--  sinal sender.
---------------------------------------------------------------------------------------
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use work.standards.all;
use ieee.std_logic_arith;
use IEEE.std_logic_textio.all;
use std.textio.all;
use IEEE.NUMERIC_STD.all;

entity RouterCC is
generic( address: regmetadeflit);
port(
        clock:     in  std_logic;
        reset:     in  std_logic;
        rx:        in  regNport;
        data_in:   in  arrayNport_regflit;
        credit_o:  out regNport;    
        tx:        out regNport;
        data_out:  out arrayNport_regflit;
        credit_i:  in  regNport);
end RouterCC;

architecture RouterCC of RouterCC is

signal h, ack_h, data_av, sender, data_ack: regNport := (others=>'0');
signal data: arrayNport_regflit := (others=>(others=>'0'));
signal mux_in, mux_out: arrayNport_reg3 := (others=>(others=>'0'));
signal free: regNport := (others=>'0');

--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
-- Remova esse sinal quando for fazer a synthesis.
-- Esse signal foi utilizado para enviar o valor do credit_o dos Hermes_buffer
-- para o hardware traffic_monit.
-- Sendo assim, é necessário fazer o Replace de credit_o_sig para credit_o, 
-- que corresponde a porta de saída do RouterCC.
signal credit_o_sig :                           regNport := (others=>'0');
--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
begin

--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
-- Remova esse generate caso queira fazer uma synthesis.
-- Esse generate é usado para o traffic_monitor das portas de entrada do router.
--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      credit_o <= credit_o_sig;
		traffic_router: for i in 0 to (NPORT-1) generate 
			 traffic_monit : entity work.traffic_monitor
				  generic map( ID  =>  i )
   			  port map(
   			  			 clock    =>  clock,
   			  			 reset    =>  reset,
   			  			 data_in  =>  data_in(i),
   					  	 address  =>  address,
   					  	 rx       =>  rx(i),
   					  	 credit_o =>  credit_o_sig(i));
		  end generate traffic_router;
--++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++        

        FEast : Entity work.Hermes_buffer
        port map(
                clock => clock,
                reset => reset,
                data_in => data_in(0),
                rx => rx(0),
                h => h(0),
                ack_h => ack_h(0),
                data_av => data_av(0),
                data => data(0),
                sender => sender(0),
                data_ack => data_ack(0),
                credit_o => credit_o_sig(0));

        FWest : Entity work.Hermes_buffer
        port map(
                clock => clock,
                reset => reset,
                data_in => data_in(1),
                rx => rx(1),
                h => h(1),
                ack_h => ack_h(1),
                data_av => data_av(1),
                data => data(1),
                sender => sender(1),
                data_ack => data_ack(1),
                credit_o => credit_o_sig(1));

        FNorth : Entity work.Hermes_buffer
        port map(
                clock => clock,
                reset => reset,
                data_in => data_in(2),
                rx => rx(2),
                h => h(2),
                ack_h => ack_h(2),
                data_av => data_av(2),
                data => data(2),
                sender => sender(2),
                data_ack => data_ack(2),
                credit_o => credit_o_sig(2));

        FSouth : Entity work.Hermes_buffer
        port map(
                clock => clock,
                reset => reset,
                data_in => data_in(3),
                rx => rx(3),
                h => h(3),
                ack_h => ack_h(3),
                data_av => data_av(3),
                data => data(3),
                sender => sender(3),
                data_ack => data_ack(3),
                credit_o => credit_o_sig(3));

        FLocal : Entity work.Hermes_buffer
        port map(
                clock => clock,
                reset => reset,
                data_in => data_in(4),
                rx => rx(4),
                h => h(4),
                ack_h => ack_h(4),
                data_av => data_av(4),
                data => data(4),
                sender => sender(4),
                data_ack => data_ack(4),
                credit_o => credit_o_sig(4));

        SwitchControl : Entity work.SwitchControl(XY)
        port map(
                clock => clock,
                reset => reset,
                h => h,
                ack_h => ack_h,
                address => address,
                data => data,
                sender => sender,
                free => free,
                mux_in => mux_in,
                mux_out => mux_out);

        CrossBar : Entity work.Hermes_crossbar
        port map(
                data_av => data_av,
                data_in => data,
                data_ack => data_ack,
                sender => sender,
                free => free,
                tab_in => mux_in,
                tab_out => mux_out,
                tx => tx,
                data_out => data_out,
                credit_i => credit_i);

end RouterCC;
