------------------------------------------------------------------------------------------------
--
--  DISTRIBUTED HEMPS  - version 5.0
--
--  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
--
--  Distribution:  August 2016
--
--  Source name:  standards.vhd
--
--  Brief description:  Functions and constants for NoC generation.
--
------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.Std_Logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;
use IEEE.math_real.all;
use work.hemps_pkg.all;

package standards is

        --function log2( i : natural) return integer;

--------------------------------------------------------
-- CONSTANTS
--------------------------------------------------------
-- Memory map constants.
	constant DEBUG           : std_logic_vector(31 downto 0) := x"20000000";
	constant IRQ_MASK        : std_logic_vector(31 downto 0) := x"20000010";
	constant IRQ_STATUS_ADDR : std_logic_vector(31 downto 0) := x"20000020";
	constant TIME_SLICE_ADDR : std_logic_vector(31 downto 0) := x"20000060";
	constant FIFO_AVAIL      : std_logic_vector(31 downto 0) := x"20000040";
	constant END_SIM         : std_logic_vector(31 downto 0) := x"20000080";
	constant CLOCK_HOLD      : std_logic_vector(31 downto 0) := x"20000090";
	constant NET_ADDRESS     : std_logic_vector(31 downto 0) := x"20000140";
	
	-- Network interface mapping.
	constant NI_STATUS_READ   : std_logic_vector(31 downto 0) := x"20000100";
	constant NI_STATUS_SEND   : std_logic_vector(31 downto 0) := x"20000110";
	constant NI_READ          : std_logic_vector(31 downto 0) := x"20000120";
	constant NI_WRITE         : std_logic_vector(31 downto 0) := x"20000130";
	constant NI_CONFIGURATION : std_logic_vector(31 downto 0) := x"20000140";
	constant NI_ACK           : std_logic_vector(31 downto 0) := x"20000150";
	constant NI_NACK          : std_logic_vector(31 downto 0) := x"20000160";
	constant NI_END           : std_logic_vector(31 downto 0) := x"20000170";
	
	-- DMNI mapping.
	constant DMNI_SIZE   : std_logic_vector(31 downto 0) := x"20000200";
	constant DMNI_ADDR   : std_logic_vector(31 downto 0) := x"20000210";
	constant DMNI_SIZE_2 : std_logic_vector(31 downto 0) := x"20000204";
	constant DMNI_ADDR_2 : std_logic_vector(31 downto 0) := x"20000214";
	constant DMNI_OP     : std_logic_vector(31 downto 0) := x"20000220";
	constant START_DMNI  : std_logic_vector(31 downto 0) := x"20000230";
	constant DMNI_ACK    : std_logic_vector(31 downto 0) := x"20000240";
	
	constant DMNI_SEND_ACTIVE    : std_logic_vector(31 downto 0) := x"20000250";
        constant DMNI_RECEIVE_ACTIVE : std_logic_vector(31 downto 0) := x"20000260";
        constant DMA_SEND_KERNEL : std_logic_vector(31 downto 0) := x"20000264";
        constant DMA_WAIT_KERNEL : std_logic_vector(31 downto 0) := x"20000268";
	constant DMA_FAIL_KERNEL : std_logic_vector(31 downto 0) := x"20000272";
        	
	constant SCHEDULING_REPORT : std_logic_vector(31 downto 0) := x"20000270";
	
	constant TICK_COUNTER_ADDR : std_logic_vector(31 downto 0) := x"20000300";
	constant REQ_APP_REG       : std_logic_vector(31 downto 0) := x"20000350";
	constant ACK_APP_REG       : std_logic_vector(31 downto 0) := x"20000360";
	
        constant PENDING_SERVICE_INTR : std_logic_vector(31 downto 0) := x"20000400";
	
        constant CLOCK_HOLD_WAIT_KERNEL : std_logic_vector(31 downto 0) := x"20000410";
	
	
	constant SLACK_TIME_MONITOR : std_logic_vector(31 downto 0) := x"20000370";
	constant SLACK_MONITOR_WINDOW : integer :=	50000;
--------------------------------------------------------------------------------
-- Router position constants - FIXED - it is not a function of the NoC size
--------------------------------------------------------------------------------
        constant BL: integer := 0;
        constant BC: integer := 1;
        constant BR: integer := 2;
        constant CL: integer := 3;
        constant CC: integer := 4;
        constant CRX: integer := 5; 
        constant TL: integer := 6;
        constant TC: integer := 7;
        constant TR: integer := 8;

-------------------------------------------------------------------------
--      Header flit types
-----------------------------------------------------------------------
	constant PACKET_SWITCHING_XY            : std_logic_vector(3 downto 0) := x"0";
	constant PACKET_SWITCHING_SR            : std_logic_vector(3 downto 0) := x"7";
        --header flit to access the IO
        constant OUT_NORTH                      : std_logic_vector(3 downto 0) := x"5";
        constant OUT_EAST                       : std_logic_vector(3 downto 0) := x"4";
        constant OUT_WEST                       : std_logic_vector(3 downto 0) := x"3";
        constant OUT_SOUTH                      : std_logic_vector(3 downto 0) := x"2";

---------------------------------------------------------
-- CONSTANTS INDEPENDENTES
---------------------------------------------------------
        -- constant NPORT: integer := 5;

	-- constant EAST: integer := 0;
	-- constant WEST: integer := 1;
	-- constant NORTH : integer := 2;
	-- constant SOUTH : integer := 3;
	-- constant LOCAL : integer := 4;
	

	--duplicated channel
	constant NPORT  : integer := 10; 

	constant EAST0  : integer := 0;
	constant EAST1  : integer := 1;
	constant WEST0  : integer := 2;
	constant WEST1  : integer := 3;
	constant NORTH0 : integer := 4;
	constant NORTH1 : integer := 5;
	constant SOUTH0 : integer := 6;
	constant SOUTH1 : integer := 7;
	constant LOCAL0 : integer := 8;
	constant LOCAL1 : integer := 9;

---------------------------------------------------------
-- CONSTANT DEPENDENTE DA LARGURA DE BANDA DA REDE - FIXED FOR HEMPS
---------------------------------------------------------
	constant SOURCE_TARGET_SIZE_HEMPS 	: integer := 10;
        constant TAM_FLIT 				: integer range 1 to 64 := 16;
        constant METADEFLIT 			: integer range 1 to 32 := (TAM_FLIT/2);
        constant QUARTOFLIT 			: integer range 1 to 16 := (TAM_FLIT/4);

	constant TAM_FLIT_32   : integer range 1 to 64 := 32;
	constant METADEFLIT_32 : integer range 1 to 32 := (TAM_FLIT_32/2);
	constant FLIT16        : integer range 1 to 32 := 16;
	constant QUARTOFLIT_32 : integer range 1 to 16 := (TAM_FLIT_32/4);

---------------------------------------------------------
-- CONSTANTS DEPENDENTES DA PROFUNDIDADE DA FILA
---------------------------------------------------------
        constant TAM_BUFFER             : integer := 8;
        constant BUFFER_POINTER         : integer := integer(log2( real(TAM_BUFFER)));
        constant TAM_BUFFER_DMNI        : integer := 16;

---------------------------------------------------------
-- CONSTANTS DEPENDENTES DO NUMERO DE ROTEADORES
---------------------------------------------------------
        constant NROT: integer := NUMBER_PROCESSORS;

        constant MIN_X : integer := 0;
        constant MIN_Y : integer := 0;
        constant MAX_X : integer := NUMBER_PROCESSORS_X-1;
        constant MAX_Y : integer := NUMBER_PROCESSORS_Y-1;

---------------------------------------------------------
-- CONSTANT TB
---------------------------------------------------------
        constant TAM_LINHA : integer := 2; --4;

---------------------------------------------------------
-- SUBTIPOS, TIPOS E FUNCOES
---------------------------------------------------------

        subtype reg3 is std_logic_vector(2 downto 0);
        subtype reg8 is std_logic_vector(7 downto 0);
		subtype reg16 is std_logic_vector(15 downto 0);
        subtype reg30 is std_logic_vector(29 downto 0); 
        subtype reg32 is std_logic_vector(31 downto 0); 
        subtype regNrot is std_logic_vector((NROT-1) downto 0);
        subtype regNport is std_logic_vector((NPORT-1) downto 0); 
        subtype regflit is std_logic_vector((TAM_FLIT-1) downto 0); 
	subtype regflit_32 is std_logic_vector((TAM_FLIT_32-1) downto 0);
		
        subtype regmetadeflit is std_logic_vector((METADEFLIT-1) downto 0); 
        subtype regquartoflit is std_logic_vector((QUARTOFLIT-1) downto 0);
		
	subtype regmetadeflit_32 is std_logic_vector((METADEFLIT_32-1) downto 0); 
	subtype regquartoflit_32 is std_logic_vector((QUARTOFLIT_32-1) downto 0);
	subtype	regNsource_target_hemps is std_logic_vector((SOURCE_TARGET_SIZE_HEMPS-1) downto 0);
		
	--depth of the buffers in NoC
        subtype pointer is std_logic_vector((BUFFER_POINTER-1) downto 0);
        subtype regNPe is std_logic_vector((NUMBER_PROCESSORS - 1) downto 0);

        type buff is array(0 to TAM_BUFFER-1) of regflit;
        type buff_dmni is array(0 to TAM_BUFFER_DMNI-1) of regflit;


        type arrayNport_reg3 is array((NPORT-1) downto 0) of reg3;
        type arrayNport_reg8 is array((NPORT-1) downto 0) of reg8;
        type arrayNport_regflit is array((NPORT-1) downto 0) of regflit; 
		type arrayNport_regflit_32 is array((NPORT-1) downto 0) of regflit_32; 
        type arrayNrot_reg3 is array((NROT-1) downto 0) of reg3;
        type arrayNrot_regflit is array((NROT-1) downto 0) of regflit;
        type arrayNrot_regmetadeflit is array((NROT-1) downto 0) of regmetadeflit; 
        type arrayNPe_reg32 is array((NUMBER_PROCESSORS -1) downto 0) of reg32;
        type arrayNPe_reg30 is array((NUMBER_PROCESSORS -1) downto 0) of reg30;

        -- number of ports of the processing_element - 4 - north/south/west/east
        type arrayNPORT_1_regflit is array(3 downto 0) of regflit;

	--constants for the DUPLICATED PORT
	type matrixNportNport_std_logic is array((NPORT-1) downto 0) of std_logic_vector((NPORT-1) downto 0);

---------------------------------------------------------
-- HERMES FUCTIONS 
---------------------------------------------------------
        Function CONV_VECTOR( int: integer ) return std_logic_vector;
        function CONV_VECTOR( letra : string(1 to TAM_LINHA);  pos: integer ) return std_logic_vector;
        function CONV_HEX( int : integer ) return string;
        function CONV_STRING_4BITS( dado : std_logic_vector(3 downto 0)) return string; 
        function CONV_STRING_8BITS( dado : std_logic_vector(7 downto 0)) return string; 
        function CONV_STRING_16BITS( dado : std_logic_vector(15 downto 0)) return string;
        function CONV_STRING_32BITS( dado : std_logic_vector(31 downto 0)) return string;

---------------------------------------------------------
-- HEMPS FUCTIONS
---------------------------------------------------------
        function RouterPosition(router: integer) return integer;
        function RouterAddress(router: integer) return std_logic_vector; 
        function log_filename(i: integer) return string;
end standards;

package body standards is 
        --
        -- converte um inteiro em um std_logic_vector(2 downto 0) 
        --
        function CONV_VECTOR( int: integer ) return std_logic_vector is
                variable bin: reg3;
        begin 
                case(int) is
                        when 0 => bin := "000";
                        when 1 => bin := "001";
                        when 2 => bin := "010";
                        when 3 => bin := "011";
                        when 4 => bin := "100";
                        when 5 => bin := "101";
                        when 6 => bin := "110";
                        when 7 => bin := "111";
                        when others => bin := "000";
                end case;
                return bin; 
        end CONV_VECTOR;
        --------------------------------------------------------- 
        -- FUNCOES TB
        --------------------------------------------------------- 
        --
        -- converte um caracter de uma dada linha em um std_logic_vector 
        --
        function CONV_VECTOR( letra:string(1 to TAM_LINHA);pos: integer ) return std_logic_vector is
                variable bin: std_logic_vector(3 downto 0); 
        begin 
                case (letra(pos)) is
                        when '0' => bin := "0000";
                        when '1' => bin := "0001";
                        when '2' => bin := "0010";
                        when '3' => bin := "0011";
                        when '4' => bin := "0100";
                        when '5' => bin := "0101";
                        when '6' => bin := "0110";
                        when '7' => bin := "0111";
                        when '8' => bin := "1000";
                        when '9' => bin := "1001";
                        when 'A' => bin := "1010";
                        when 'B' => bin := "1011";
                        when 'C' => bin := "1100";
                        when 'D' => bin := "1101";
                        when 'E' => bin := "1110";
                        when 'F' => bin := "1111";
                        when others =>bin := "0000";
                end case;
                return bin; 
        end CONV_VECTOR;

-- converte um inteiro em um string 
        function CONV_HEX( int: integer ) return string is
                variable str: string(1 to 1);
        begin 
                case(int) is
                        when 0 => str := "0";
                        when 1 => str := "1";
                        when 2 => str := "2";
                        when 3 => str := "3";
                        when 4 => str := "4";
                        when 5 => str := "5";
                        when 6 => str := "6";
                        when 7 => str := "7";
                        when 8 => str := "8";
                        when 9 => str := "9";
                        when 10 => str := "A";
                        when 11 => str := "B";
                        when 12 => str := "C";
                        when 13 => str := "D";
                        when 14 => str := "E";
                        when 15 => str := "F";
                        when others =>str := "U";
                end case;
                return str; 
        end CONV_HEX;

        function CONV_STRING_4BITS(dado : std_logic_vector(3 downto 0)) return string is
                variable str: string(1 to 1);
        begin 
                str := CONV_HEX(CONV_INTEGER(dado));
                return str; 
        end CONV_STRING_4BITS; 

        function CONV_STRING_8BITS(dado : std_logic_vector(7 downto 0)) return string is
                variable str1,str2: string(1 to 1);
                variable str: string(1 to 2);
        begin 
                str1 := CONV_STRING_4BITS(dado(7 downto 4));
                str2 := CONV_STRING_4BITS(dado(3 downto 0));
                str := str1 & str2;
                return str; 
        end CONV_STRING_8BITS; 

        function CONV_STRING_16BITS(dado : std_logic_vector(15 downto 0)) return string is
                variable str1,str2: string(1 to 2);
                variable str: string(1 to 4);
        begin 
                str1 := CONV_STRING_8BITS(dado(15 downto 8));
                str2 := CONV_STRING_8BITS(dado(7 downto 0));
                str := str1 & str2;
                return str; 
        end CONV_STRING_16BITS;

        function CONV_STRING_32BITS(dado : std_logic_vector(31 downto 0)) return string is
                variable str1,str2: string(1 to 4);
                variable str: string(1 to 8);
        begin 
                str1 := CONV_STRING_16BITS(dado(31 downto 16));
                str2 := CONV_STRING_16BITS(dado(15 downto 0));
                str := str1 & str2;
                return str; 
        end CONV_STRING_32BITS;

                -- Returns the router position in the mesh
        -- BR: Botton Right
        -- BL: Botton Left
        -- TR: Top Right
        -- TL: Top Left 
        -- CRX: Center Right 
        -- CL: Center Left
        -- CC: Center
        -- 4x4 positions exemple
        --              TL TC TC TR
        --              CL CC CC CRX 
        --              CL CC CC CRX 
        --              BL BC BC BR
        function RouterPosition(router: integer) return integer is
                variable pos: integer range 0 to TR;
                variable line, column: integer;
                begin
                        
                        --line := router/NUMBER_PROCESSORS_Y;
                        column := router mod NUMBER_PROCESSORS_X;
                        
                        if router >= NUMBER_PROCESSORS-NUMBER_PROCESSORS_X then --TOP 
                                if column = NUMBER_PROCESSORS_X-1 then --RIGHT
                                        pos := TR;
                                elsif column = 0 then--LEFT
                                        pos := TL;
                                else--CENTER_X
                                        pos := TC;
                                end if;
                        elsif router < NUMBER_PROCESSORS_X then --BOTTOM
                                if column = NUMBER_PROCESSORS_X-1 then --RIGHT
                                        pos := BR;
                                elsif column = 0 then--LEFT
                                        pos := BL;
                                else--CENTER_X
                                        pos := BC;
                                end if;
                        else --CENTER_Y
                                if column = NUMBER_PROCESSORS_X-1 then --RIGHT
                                        pos := CRX; 
                                elsif column = 0 then--LEFT
                                        pos := CL;
                                else--CENTER_X
                                        pos := CC;
                                end if;
                        end if; 
                        
                        return pos;
                        
        end RouterPosition;

        function RouterAddress(router: integer) return std_logic_vector is
				variable pos_x, pos_y			: reg8;
				--size of the address in hemps = 16 bits
				variable addr 					: reg16;
                variable aux                    : integer;
        begin 
                aux := (router/NUMBER_PROCESSORS_X);
				pos_x := conv_std_logic_vector((router mod NUMBER_PROCESSORS_X),8);
				pos_y := conv_std_logic_vector(aux,8); 
                
                addr := pos_x & pos_y;
                return addr;
        end RouterAddress;
        
        function log_filename(i: integer) return string is
                variable filename               : string(1 to 14);
                variable aux_x                  : integer;
                variable aux_y                  : integer;
        begin
                aux_x := (i mod NUMBER_PROCESSORS_X);
                aux_y := (i/NUMBER_PROCESSORS_X);
                filename := "log/log" & CONV_HEX(aux_x) & "x" & CONV_HEX(aux_y)  & ".txt";
                return filename;
        end log_filename;    
       
end standards;
