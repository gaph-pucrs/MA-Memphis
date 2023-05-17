------------------------------------------------------------------------------------------------
--
--  DISTRIBUTED MEMPHIS  - version 5.0
--
--  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
--
--  Distribution:  September 2013
--
--  Source name:  Hermes_switchcontrol.vhd
--
--  Brief description: Description of the states of the NoC, and executions for each state.
--
---------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_unsigned.all;
use work.standards.all;

entity SwitchControl is
port(
        clock :   in  std_logic;
        reset :   in  std_logic;
        h :       in  regNport;
        ack_h :   out regNport;
        address : in  regmetadeflit;
        data :    in  arrayNport_regflit;
        sender :  in  regNport;
        free :    out regNport;
        mux_in :  out arrayNport_reg3;
        mux_out : out arrayNport_reg3);
end SwitchControl;

architecture XY of SwitchControl is

type state is (S0,S1,S2,S3,S4,S5,S6,S7);
signal ES, PES: state;

-- sinais do arbitro
signal ask: std_logic := '0';
signal sel,prox: integer range 0 to (NPORT-1) := 0;
signal incoming: reg3 := (others=> '0');
signal header : regflit := (others=> '0');

-- sinais do controle
signal dirx,diry: integer range 0 to (NPORT-1) := 0;
signal lx,ly,tx,ty: regquartoflit := (others=> '0');
signal io_dir: std_logic_vector(3 downto 0); --Sinal io_dir (4 bits) guarda a direcao do pacote ao chegar na porta local
signal auxfree: regNport := (others=> '0');
signal source:  arrayNport_reg3 := (others=> (others=> '0'));
signal sender_ant: regNport := (others=> '0');

begin

        ask <= '1' when h(LOCAL)='1' or h(EAST)='1' or h(WEST)='1' or h(NORTH)='1' or h(SOUTH)='1' else '0';
        incoming <= CONV_VECTOR(sel);
        header <= data(CONV_INTEGER(incoming));

        process(sel,h)
        begin
                case sel is
                        when LOCAL=>
                                if h(EAST)='1' then prox<=EAST;
                                elsif h(WEST)='1' then  prox<=WEST;
                                elsif h(NORTH)='1' then prox<=NORTH;
                                elsif h(SOUTH)='1' then prox<=SOUTH;
                                else prox<=LOCAL; end if;
                        when EAST=>
                                if h(WEST)='1' then prox<=WEST;
                                elsif h(NORTH)='1' then prox<=NORTH;
                                elsif h(SOUTH)='1' then prox<=SOUTH;
                                elsif h(LOCAL)='1' then prox<=LOCAL;
                                else prox<=EAST; end if;
                        when WEST=>
                                if h(NORTH)='1' then prox<=NORTH;
                                elsif h(SOUTH)='1' then prox<=SOUTH;
                                elsif h(LOCAL)='1' then prox<=LOCAL;
                                elsif h(EAST)='1' then prox<=EAST;
                                else prox<=WEST; end if;
                        when NORTH=>
                                if h(SOUTH)='1' then prox<=SOUTH;
                                elsif h(LOCAL)='1' then prox<=LOCAL;
                                elsif h(EAST)='1' then prox<=EAST;
                                elsif h(WEST)='1' then prox<=WEST;
                                else prox<=NORTH; end if;
                        when SOUTH=>
                                if h(LOCAL)='1' then prox<=LOCAL;
                                elsif h(EAST)='1' then prox<=EAST;
                                elsif h(WEST)='1' then prox<=WEST;
                                elsif h(NORTH)='1' then prox<=NORTH;
                                else prox<=SOUTH; end if;
                end case;
        end process;


        lx <= address((METADEFLIT - 1) downto QUARTOFLIT);
        ly <= address((QUARTOFLIT - 1) downto 0);

        tx <= header((METADEFLIT - 1) downto QUARTOFLIT);
        ty <= header((QUARTOFLIT - 1) downto 0);
        --io_dir pega os 4 primeiros bits mais significantes
        io_dir <= header(TAM_FLIT-1 downto TAM_FLIT-4);

        --dirx <= WEST when lx > tx else EAST;
        --diry <= NORTH when ly < ty else SOUTH;
        
        dirx <= WEST when (lx > tx) or (io_dir(3) = '1' and io_dir(2) = '0' and io_dir(1) = '1') else EAST;
        diry <= NORTH when (ly < ty ) or (io_dir(3) = '1' and io_dir(2) = '1' and io_dir(1) = '0') else SOUTH;
       

        process(reset,clock)
        begin
                if reset='1' then
                        ES<=S0;
                elsif clock'event and clock='0' then
                        ES<=PES;
                end if;
        end process;

        ------------------------------------------------------------------------------------------------------
        -- PARTE COMBINACIONAL PARA DEFINIR O PR�XIMO ESTADO DA M�QUINA.
        --
        -- SO -> O estado S0 � o estado de inicializa��o da m�quina. Este estado somente �
        --       atingido quando o sinal reset � ativado.
        -- S1 -> O estado S1 � o estado de espera por requisi��o de chaveamento. Quando o
        --       �rbitro recebe uma ou mais requisi��es o sinal ask � ativado fazendo a
        --       m�quina avan�ar para o estado S2.
        -- S2 -> No estado S2 a porta de entrada que solicitou chaveamento � selecionada. Se
        --       houver mais de uma, aquela com maior prioridade � a selecionada.
        -- S3 -> No estado S3 � realizado algoritmo de chaveamento XY. O algoritmo de chaveamento
        --       XY faz a compara��o do endere�o da chave atual com o endere�o da chave destino do
        --       pacote (armazenado no primeiro flit do pacote). O pacote deve ser chaveado para a
        --       porta Local da chave quando o endere�o xLyL* da chave atual for igual ao endere�o
        --       xTyT* da chave destino do pacote. Caso contr�rio, � realizada, primeiramente, a
        --       compara��o horizontal de endere�os. A compara��o horizontal determina se o pacote
        --       deve ser chaveado para o Leste (xL<xT), para o Oeste (xL>xT), ou se o mesmo j�
        --       est� horizontalmente alinhado � chave destino (xL=xT). Caso esta �ltima condi��o
        --       seja verdadeira � realizada a compara��o vertical que determina se o pacote deve
        --       ser chaveado para o Sul (yL<yT) ou para o Norte (yL>yT). Caso a porta vertical
        --       escolhida esteja ocupada, � realizado o bloqueio dos flits do pacote at� que o
        --       pacote possa ser chaveado.
        -- S4, S5 e S6 -> Nestes estados � estabelecida a conex�o da porta de entrada com a de
        --       de sa�da atrav�s do preenchimento dos sinais mux_in e mux_out.
        -- S7 -> O estado S7 � necess�rio para que a porta selecionada para roteamento baixe o sinal
        --       h.
        --
        process(ES,ask,h,lx,ly,tx,ty,auxfree,dirx,diry)
        begin
                case ES is
                        when S0 => PES <= S1;
                        when S1 => if ask='1' then PES <= S2; else PES <= S1; end if;
                        when S2 => PES <= S3;
                        when S3 => 
                        	----------- begin of special routing algorithm to a external component -------	
                        	if lx = tx and ly = ty and io_dir(3) = '1' then
                        		if io_dir(2) = '0' and auxfree(dirx)='1' then -- quando io_dir for igual a "100" or "101" entao redireciona para Leste ou Oeste
                        			PES<=S5;
                        		elsif io_dir(2) = '1' and auxfree(diry)='1' then --quando io_dir for igual a "110" or "111" entao redireciona para Norte ou Sul
                        			PES<=S6;
                        		else
                        			PES<=S1; 
                        		end if;
                        	----------- end of special routing algorithm to a external component -------	
                        	elsif lx = tx and ly = ty and auxfree(LOCAL)='1' then
                        		PES<=S4;
                    		elsif lx /= tx and auxfree(dirx)='1' then 
                    			PES<=S5;
                            elsif lx = tx and ly /= ty and auxfree(diry)='1' then 
                            	PES<=S6;
                            else 
                            	PES<=S1; 
                            end if;
                            
                        when S4 => PES<=S7;
                        when S5 => PES<=S7;
                        when S6 => PES<=S7;
                        when S7 => PES<=S1;
                end case;
        end process;

        ------------------------------------------------------------------------------------------------------
        -- executa as a��es correspondente ao estado atual da m�quina de estados
        ------------------------------------------------------------------------------------------------------
        process (clock)
        begin
                if clock'event and clock='1' then
                        case ES is
                                -- Zera vari�veis
                                when S0 =>
                                        sel <= 0;
                                        ack_h <= (others => '0');
                                        auxfree <= (others=> '1');
                                        sender_ant <= (others=> '0');
                                        mux_out <= (others=>(others=>'0'));
                                        source <= (others=>(others=>'0'));
                                -- Chegou um header
                                when S1=>
                                        ack_h <= (others => '0');
                                -- Seleciona quem tera direito a requisitar roteamento
                                when S2=>
                                        sel <= prox;
                                -- Estabelece a conex�o com a porta LOCAL
                                when S4 =>
                                        source(CONV_INTEGER(incoming)) <= CONV_VECTOR(LOCAL);
                                        mux_out(LOCAL) <= incoming;
                                        auxfree(LOCAL) <= '0';
                                        ack_h(sel)<='1';
                                -- Estabelece a conex�o com a porta EAST ou WEST
                                when S5 =>
                                        source(CONV_INTEGER(incoming)) <= CONV_VECTOR(dirx);
                                        mux_out(dirx) <= incoming;
                                        auxfree(dirx) <= '0';
                                        ack_h(sel)<='1';
                                -- Estabelece a conex�o com a porta NORTH ou SOUTH
                                when S6 =>
                                        source(CONV_INTEGER(incoming)) <= CONV_VECTOR(diry);
                                        mux_out(diry) <= incoming;
                                        auxfree(diry) <= '0';
                                        ack_h(sel)<='1';
                                when others => ack_h(sel)<='0';
                        end case;

                        sender_ant(LOCAL) <= sender(LOCAL);
                        sender_ant(EAST)  <= sender(EAST);
                        sender_ant(WEST)  <= sender(WEST);
                        sender_ant(NORTH) <= sender(NORTH);
                        sender_ant(SOUTH) <= sender(SOUTH);

                        if sender(LOCAL)='0' and  sender_ant(LOCAL)='1' then auxfree(CONV_INTEGER(source(LOCAL))) <='1'; end if;
                        if sender(EAST) ='0' and  sender_ant(EAST)='1'  then auxfree(CONV_INTEGER(source(EAST)))  <='1'; end if;
                        if sender(WEST) ='0' and  sender_ant(WEST)='1'  then auxfree(CONV_INTEGER(source(WEST)))  <='1'; end if;
                        if sender(NORTH)='0' and  sender_ant(NORTH)='1' then auxfree(CONV_INTEGER(source(NORTH))) <='1'; end if;
                        if sender(SOUTH)='0' and  sender_ant(SOUTH)='1' then auxfree(CONV_INTEGER(source(SOUTH))) <='1'; end if;

                end if;
        end process;


        mux_in <= source;
        free <= auxfree;

end XY;
