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
