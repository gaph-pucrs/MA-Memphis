------------------------------------------------------------------------------------------------
--
--  DISTRIBUTED MEMPHIS  - version 7.0
--
--  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
--
--  Distribution:  May 2015
--
--  Source name:  dmni.vhd
--
--  Brief description:  Implements a DMNI module.
--
--  OPERATIONS:
--    0 - Copy from memory
--    1 - Copy to memory
---------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use work.memphis_pkg.all;
use work.standards.all;

entity dmni is
    generic(address_router: regmetadeflit := (others=>'0'));
    port
    (  
        clock          : in  std_logic;
        reset          : in  std_logic;
        -- Configuration interface
        set_address    : in  std_logic;
        set_address_2  : in  std_logic;
        set_size       : in  std_logic;      
        set_size_2     : in  std_logic;
        set_op         : in  std_logic;
        start          : in  std_logic;
        config_data    : in  std_logic_vector(31 downto 0);
        -- Status outputs
        intr            : out  std_logic;
        send_active     : out  std_logic;
        receive_active  : out  std_logic;
        -- Memory interface
        mem_address    : out std_logic_vector(31 downto 0);
        mem_data_write : out std_logic_vector(31 downto 0);
        mem_data_read  : in  std_logic_vector(31 downto 0);
        mem_byte_we    : out std_logic_vector(3 downto 0);
        -- Noc Interface (Local port)
        tx              : out  std_logic;
        data_out        : out  regflit;
        credit_i        : in   std_logic;
        rx              : in   std_logic;
        data_in         : in   regflit;
        credit_o        : out  std_logic
    );  
end;

architecture dmni of dmni is

   constant DMNI_TIMER: std_logic_vector(4 downto 0):="10000";
   constant WORD_SIZE: std_logic_vector(4 downto 0):="00100";
   
   type dmni_state is (WAIT_state, LOAD, COPY_FROM_MEM, COPY_TO_MEM, FINISH);
   signal DMNI_Send: dmni_state;
   signal DMNI_Receive: dmni_state;
   
   type noc_state is (HEADER, PAYLOAD, DATA);
   signal SR: noc_state;
   
   type arbiter_state is (ROUND, SEND, RECEIVE);
   signal ARB: arbiter_state;
   
   signal bufferr: buff_dmni := (others=>(others=>'0'));
   subtype buffsizebool is std_logic_vector(0 to (TAM_BUFFER_DMNI-1)); 
   signal is_header: buffsizebool := (others=> '0');
   signal intr_count      : std_logic_vector(3 downto 0);

   signal first,last: pointer := (others=>'0');
   signal add_buffer      : std_logic;

   signal payload_size      : regflit;

   signal timer           : std_logic_vector(4 downto 0);
   signal address         : std_logic_vector(31 downto 0);
   signal address_2       : std_logic_vector(31 downto 0);
   signal size            : std_logic_vector(31 downto 0); 
   signal size_2          : std_logic_vector(31 downto 0);
   signal send_address     : std_logic_vector(31 downto 0);
   signal send_address_2   : std_logic_vector(31 downto 0);
   signal send_size        : std_logic_vector(31 downto 0); 
   signal send_size_2      : std_logic_vector(31 downto 0);
   signal recv_address     : std_logic_vector(31 downto 0);
   signal recv_size        : std_logic_vector(31 downto 0); 
   signal prio             : std_logic;
   signal operation        : std_logic;
   signal read_av          : std_logic;
   signal slot_available   : std_logic;
   signal read_enable      : std_logic;
   signal write_enable     : std_logic;

   signal send_active_2    : std_logic;
   signal receive_active_2 : std_logic;
   signal intr_counter_temp : std_logic_vector(3 downto 0);
begin
  --config
  proc_config: process(clock)
  begin 
    if(clock'event and clock = '1') then
      if (set_address = '1') then
        address <= config_data;
        address_2 <= (others => '0');
      elsif (set_address_2 = '1') then
        address_2 <= config_data;
      elsif (set_size = '1') then
        size <= config_data;
        size_2 <= (others => '0');
      elsif (set_size_2 = '1') then
        size_2 <= config_data;
      elsif (set_op = '1') then
        operation <= config_data(0);
      end if;
    end if;
  end process proc_config;

  mem_address <= send_address when write_enable = '1' else recv_address;
  credit_o <= slot_available;
  slot_available <= '0' when (first = last and add_buffer = '1') else '1';
  read_av <= '0' when (first = last and add_buffer = '0') else '1';
  send_active <= send_active_2;
  receive_active <= receive_active_2;

  arbiter: process (clock,reset)
  begin   
    if reset = '1' then
      read_enable <= '0';
      write_enable <= '0';
      timer <= "00000";
      prio <= '0';
      ARB <= ROUND;
      elsif (clock'event and clock = '1') then            
          case ARB is                
              when ROUND =>
                  if prio = '0' then
                    if DMNI_Receive = COPY_TO_MEM then
                       ARB <= RECEIVE;
                       read_enable <= '1';                        
                    elsif send_active_2 = '1' then
                       ARB <= SEND;
                       write_enable <= '1';
                    end if;
                  else
                    if send_active_2 = '1' then
                      ARB <= SEND;
                      write_enable <= '1';
                    elsif DMNI_Receive = COPY_TO_MEM then
                      ARB <= RECEIVE;
                      read_enable <= '1';
                    end if;
                  end if;

              when SEND =>
                  if DMNI_Send = FINISH or (timer = DMNI_TIMER and receive_active_2 = '1')  then   
                    timer <= "00000";
                    ARB <= ROUND;
                    write_enable <= '0';
                    prio <= not prio;
                  else
                      timer <= timer + '1';
                  end if;            
          
              when RECEIVE =>            
                  if DMNI_Receive = FINISH or (timer = DMNI_TIMER and send_active_2 = '1') then                   
                    timer <= "00000";
                    ARB <= ROUND;
                    read_enable <= '0';
                    prio <= not prio;
                  else
                      timer <= timer + '1';
                  end if;
          end case;   
      end if;
  end process arbiter; 

  proc_receive : process (clock, reset)
  begin 
    if (reset = '1') then
      first <= (others=> '0');
      last <= (others=> '0');
      payload_size <= (others=> '0');
      SR <= HEADER;
      add_buffer <= '0';
      receive_active_2 <= '0';
      DMNI_Receive <= WAIT_state;
      recv_address <= (others=> '0');
      recv_size <= (others=> '0');
      mem_data_write <= (others=> '0');
      is_header <= (others=> '0');
      intr_counter_temp <= (others=> '0');        
      mem_byte_we <= (others=> '0');  
    elsif (clock'event and clock = '1') then
      if (rx ='1' and slot_available = '1') then
        bufferr(CONV_INTEGER(last)) <= data_in;
        add_buffer <= '1';
        last <= last + 1;

        --Read from NoC
        case( SR ) is           
           when HEADER =>
             intr_counter_temp <= intr_counter_temp + 1;
             assert address_router = x"0000"
               report   "Master receiving msg"                     
             severity note;
             is_header(CONV_INTEGER(last)) <= '1';
             SR <= PAYLOAD;
           when PAYLOAD =>
             is_header(CONV_INTEGER(last)) <= '0';
             payload_size <= data_in - 1;
             SR <= DATA;
           when DATA =>
             is_header(CONV_INTEGER(last)) <= '0';
             if(payload_size = 0) then
                SR <= HEADER;
             else 
                payload_size <= payload_size - 1;
             end if;
         end case ; 
      end if; --(rx ='1' and slot_available = '1')

      --Write to memory
      case( DMNI_Receive ) is        
        when WAIT_state =>
          if (start = '1' and operation = '1') then
            recv_address <= address - WORD_SIZE;
            recv_size <= size - 1;
            if(is_header(CONV_INTEGER(first)) = '1' and intr_counter_temp > 0) then
              intr_counter_temp <= intr_counter_temp -1;
            end if;
            receive_active_2 <= '1';
            DMNI_Receive <= COPY_TO_MEM;
          end if;

        when COPY_TO_MEM =>
          if (read_enable = '1' and read_av = '1') then
            mem_byte_we <= "1111";
            mem_data_write <= bufferr(CONV_INTEGER(first));
            first <= first + 1;
            add_buffer <= '0';
            recv_address <= recv_address + WORD_SIZE;
            recv_size <= recv_size -1;
            if (recv_size = 0) then
              DMNI_Receive <= FINISH;
            end if ;
          else
            mem_byte_we <= "0000";
          end if;

        when FINISH =>
          receive_active_2 <= '0';
          mem_byte_we <= "0000";
          recv_address <= (others=> '0');
          recv_size <= (others=> '0');
          DMNI_Receive <= WAIT_state;
        when OTHERS => 
      end case;
    end if; --rising_edge(clock)
  end process proc_receive;

  intr_count <= intr_counter_temp;
  intr <= '1' when intr_counter_temp > 0 else '0';

  proc_send : process (clock, reset)
  begin 
    if(reset = '1') then
      DMNI_Send <= WAIT_state;
      send_active_2 <= '0';     
      tx <= '0';   
      send_size <= (others=> '0');
      send_size_2 <= (others=> '0');
      send_address <= (others=> '0');
      send_address_2 <= (others=> '0');
      data_out <= (others=> '0');
    elsif (clock'event and clock = '1') then
      case( DMNI_Send ) is        
        when WAIT_state =>
          if (start = '1' and operation = '0') then
            send_address <= address;
            send_address_2 <= address_2;
            send_size <= size;
            send_size_2 <= size_2;              
            send_active_2 <= '1';
            DMNI_Send <= LOAD;           
            assert address_router = x"0000"
              report   "Master sending msg"                     
            severity note;
          end if ;

        when LOAD =>
          if(credit_i = '1' and write_enable = '1') then
            send_address <= send_address + WORD_SIZE;
            DMNI_Send <= COPY_FROM_MEM;
          end if;

        when COPY_FROM_MEM =>
          if(credit_i = '1' and write_enable = '1') then
                  if(send_size > 0) then
                    tx <= '1';
                    data_out <= mem_data_read;
                    send_address <= send_address + WORD_SIZE;
                    send_size <= send_size -1;
                  elsif (send_size_2 > 0) then
                    send_size <= send_size_2;
                    send_size_2 <= (others=> '0');
                    tx <= '0';
                    if(send_address_2(30 downto 28) = "000") then
                      send_address <= send_address_2;
                    else
                      send_address <= send_address_2 - WORD_SIZE;
                    end if;
                    DMNI_Send <= LOAD;
                  else
                    tx <= '0';
                    DMNI_Send <= FINISH;
                  end if;
          else
                if (credit_i = '0') then
                  send_size <= send_size + 1;
                  send_address <= send_address - WORD_SIZE - WORD_SIZE; -- address back 2 positions 
                else
                  send_address <= send_address - WORD_SIZE; -- address back 1 position
                end if;
                tx <= '0';
                DMNI_Send <= LOAD;
          end if;

        when FINISH =>
          send_active_2     <= '0';
          send_address    <= (others=> '0');
          send_address_2  <= (others=> '0');
          send_size       <= (others=> '0');
          send_size_2     <= (others=> '0');
          DMNI_Send       <= WAIT_state;
        when OTHERS =>
      end case ;
    end if; --rising_edge(clock)
  end process proc_send;      
end dmni;
