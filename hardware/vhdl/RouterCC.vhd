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
use work.hemps_pkg.all;
use ieee.std_logic_arith;
use IEEE.std_logic_textio.all;
use std.textio.all;
use IEEE.NUMERIC_STD.all;

entity RouterCC is

    generic(
        address: regmetadeflit;
        EnablePacketFilter: boolean := FILTER_ENABLE(RouterIndex(address));
        XMax: regquartoflit := std_logic_vector(to_unsigned(NUMBER_PROCESSORS_X, QUARTOFLIT));
        YMax: regquartoflit := std_logic_vector(to_unsigned(NUMBER_PROCESSORS_Y, QUARTOFLIT));
        TimeoutMax: integer := TIMEOUT_MAX
    );

    port(
        clock:     in  std_logic;
        reset:     in  std_logic;
        --clock_rx:  in  regNport;
        rx:        in  regNport;
        data_in:   in  arrayNport_regflit;
        credit_o:  out regNport;    
        --clock_tx:  out regNport;
        tx:        out regNport;
        data_out:  out arrayNport_regflit;
        credit_i:  in  regNport
    );

end RouterCC;

architecture RouterCC of RouterCC is

    signal h, ack_h, data_av, sender, data_ack: regNport := (others=>'0');
    signal data: arrayNport_regflit := (others=>(others=>'0'));
    signal mux_in, mux_out: arrayNport_reg3 := (others=>(others=>'0'));
    signal free: regNport := (others=>'0');

    signal packetFilterDataOut: regflit;
    signal packetFilterTx: std_logic;
    signal packetFilterCreditI: std_logic;

    signal localBufferDataIn: regflit;
    signal localBufferRx: std_logic;
    signal localBufferCreditO: std_logic;

    --++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    -- Remova esse sinal quando for fazer a synthesis.
    -- Esse signal foi utilizado para enviar o valor do credit_o dos Hermes_buffer
    -- para o hardware traffic_monit.
    -- Sendo assim, é necessário fazer o Replace de credit_o_sig para credit_o, 
    -- que corresponde a porta de saída do RouterCC.
    signal credit_o_sig: regNport := (others=>'0');
    --++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

begin

      credit_o <= credit_o_sig;
-- --++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
-- -- Remova esse generate caso queira fazer uma synthesis.
-- -- Esse generate é usado para o traffic_monitor das portas de entrada do router.
-- --++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		-- traffic_router: for i in 0 to (NPORT-1) generate 
		-- 	 traffic_monit : entity work.traffic_monitor
		-- 		  generic map( ID  =>  i )
   		-- 	  port map(
   		-- 	  			 clock    =>  clock,
   		-- 	  			 reset    =>  reset,
   		-- 	  			 data_in  =>  data_in(i),
   		-- 			  	 address  =>  address,
   		-- 			  	 rx       =>  rx(i),
   		-- 			  	 credit_o =>  credit_o_sig(i));
		--   end generate traffic_router;
-- --++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++        

        FEast: entity work.Hermes_buffer

            port map(
                clock => clock,
                reset => reset,
                data_in => data_in(EAST),
                rx => rx(EAST),
                h => h(EAST),
                ack_h => ack_h(EAST),
                data_av => data_av(EAST),
                data => data(EAST),
                sender => sender(EAST),
                data_ack => data_ack(EAST),
                credit_o => credit_o_sig(EAST)
            );

        FWest: entity work.Hermes_buffer

            port map(
                clock => clock,
                reset => reset,
                data_in => data_in(WEST),
                rx => rx(WEST),
                h => h(WEST),
                ack_h => ack_h(WEST),
                data_av => data_av(WEST),
                data => data(WEST),
                sender => sender(WEST),
                data_ack => data_ack(WEST),
                credit_o => credit_o_sig(WEST)
            );

        FNorth: entity work.Hermes_buffer

            port map(
                clock => clock,
                reset => reset,
                data_in => data_in(NORTH),
                rx => rx(NORTH),
                h => h(NORTH),
                ack_h => ack_h(NORTH),
                data_av => data_av(NORTH),
                data => data(NORTH),
                sender => sender(NORTH),
                data_ack => data_ack(NORTH),
                credit_o => credit_o_sig(NORTH)
            );

        FSouth: entity work.Hermes_buffer

            port map(
                clock => clock,
                reset => reset,
                data_in => data_in(SOUTH),
                rx => rx(SOUTH),
                h => h(SOUTH),
                ack_h => ack_h(SOUTH),
                data_av => data_av(SOUTH),
                data => data(SOUTH),
                sender => sender(SOUTH),
                data_ack => data_ack(SOUTH),
                credit_o => credit_o_sig(SOUTH)
            );

        FLocal: entity work.Hermes_buffer

            port map(
                clock => clock,
                reset => reset,
                --data_in => data_in(LOCAL),
                --rx => rx(LOCAL),
                data_in => localBufferDataIn,
                rx => localBufferRx,
                h => h(LOCAL),
                ack_h => ack_h(LOCAL),
                data_av => data_av(LOCAL),
                data => data(LOCAL),
                sender => sender(LOCAL),
                data_ack => data_ack(LOCAL),
                --credit_o => credit_o_sig(LOCAL)
                credit_o => localBufferCreditO
            );

        PacketFilterGen: if EnablePacketFilter generate

            PacketFilter: entity work.PacketFilter

                generic map(
                    XMax => XMax,
                    YMax => YMax,
                    TimeoutMax => TimeoutMax
                )

                port map(
                    clk => clock,
                    reset => reset,
                    data_in => data_in(LOCAL),
                    rx => rx(LOCAL),
                    --credit_o => credit_o(LOCAL),
                    credit_o => credit_o_sig(LOCAL),
                    data_out => packetFilterDataOut,
                    tx => packetFilterTx,
                    --credit_i => localBufferCreditO
                    credit_i => packetFilterCreditI
                );
                
                localBufferDataIn <= packetFilterDataOut;
                localBufferRx <= packetFilterTx;
                packetFilterCreditI <= localBufferCreditO;

        end generate;

        NoPacketFilter: if not EnablePacketFilter generate

            localBufferDataIn <= data_in(LOCAL);
            localBufferRx <= rx(LOCAL);
            credit_o_sig(LOCAL) <= localBufferCreditO;

        end generate;

        SwitchControl: entity work.SwitchControl(XY)

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
                mux_out => mux_out
            );

        CrossBar: entity work.Hermes_crossbar

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
                credit_i => credit_i
            );

        --CLK_TX: for i in 0 to(NPORT-1) generate
        --    clock_tx(i) <= clock;
        --end generate CLK_TX;  

end RouterCC;
