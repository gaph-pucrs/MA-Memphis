------------------------------------------------------------------------------------------------
-- Memphis Processing Element
------------------------------------------------------------------------------------------------
library ieee;
use work.mlite_pack.all;                
use work.standards.all;
use work.memphis_pkg.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_textio.all;
use ieee.std_logic_unsigned.all;
use ieee.math_real.all;

use std.textio.all;
library unisim;
use unisim.vcomponents.all;

entity pe is
    generic 
    (
        log_file            : string := "output.txt";
        router_address      : regmetadeflit:= (others=>'0');
        kernel_type			: kernel_str
    );
    port 
    (  
    	clock                   : in  std_logic;
        reset                   : in  std_logic;
		-- NoC
        rx                      : in  std_logic_vector(3 downto 0);
        data_in                 : in  arrayNPORT_1_regflit;
        credit_o                : out std_logic_vector(3 downto 0);
        tx                      : out std_logic_vector(3 downto 0);
        data_out                : out arrayNPORT_1_regflit;
        credit_i                : in  std_logic_vector(3 downto 0)
    );
end entity pe;

architecture structural of pe is
	
	type repo_state is (WAIT_state, COPY_FROM_REP);
    signal repo_FSM        : repo_state;
	
    signal cpu_mem_address_reg           : std_logic_vector(31 downto 0);
    signal cpu_mem_data_write_reg        : std_logic_vector(31 downto 0);
    signal cpu_mem_write_byte_enable_reg : std_logic_vector(3 downto 0);
    signal irq_mask_reg                  : std_logic_vector(7 downto 0);
    signal irq_status                    : std_logic_vector(7 downto 0);
    signal irq                           : std_logic;
    signal time_slice                    : std_logic_vector(31 downto 0);
    signal write_enable                  : std_logic;
    signal tick_counter_local            : std_logic_vector(31 downto 0);
    signal tick_counter                  : std_logic_vector(31 downto 0);
    signal current_page                  : std_logic_vector(7 downto 0);
    signal cpu_mem_address               : std_logic_vector(31 downto 0);
    signal cpu_mem_data_write            : std_logic_vector(31 downto 0);
    signal cpu_mem_data_read             : std_logic_vector(31 downto 0);
    signal cpu_mem_write_byte_enable     : std_logic_vector(3 downto 0);
    signal cpu_mem_pause                 : std_logic;
    signal cpu_enable_ram                : std_logic;
    signal cpu_set_size                  : std_logic;
    signal cpu_set_address               : std_logic;
    signal cpu_set_size_2                : std_logic;
    signal cpu_set_address_2             : std_logic;
    signal cpu_set_op                    : std_logic;
    signal cpu_start                     : std_logic;
    signal clock_aux                     : std_logic;
    signal clock_hold_s                  : std_logic;
    signal pending_service               : std_logic;
    
    --Router
    signal rx_router                     : regNport;
    signal data_in_router                : arrayNport_regflit;
    signal credit_o_router               : regNport;
    signal tx_router                     : regNport;
    signal data_out_router               : arrayNport_regflit;
    signal credit_i_router               : regNport;
    
    signal data_read_ram                 : std_logic_vector(31 downto 0);
    signal mem_data_read                 : std_logic_vector(31 downto 0);
    --signal debug_busy                    : std_logic;
    --signal debug_write_data              : std_logic;
    --signal debug_write_busy              : std_logic;
    --signal debug_data_avail              : std_logic;
    signal ni_intr                       : std_logic;
    signal dmni_mem_address              : std_logic_vector(31 downto 0);
    signal dmni_mem_write_byte_enable    : std_logic_vector(3 downto 0);
    signal dmni_mem_data_write           : std_logic_vector(31 downto 0);
    signal dmni_mem_data_read            : std_logic_vector(31 downto 0);
    signal dmni_data_read                : std_logic_vector(31 downto 0);
    signal dmni_enable_internal_ram      : std_logic;
    signal dmni_send_active_sig          : std_logic;
    signal dmni_receive_active_sig       : std_logic;
    signal addr_a                        : std_logic_vector(31 downto 2);
    signal addr_b                        : std_logic_vector(31 downto 2);
    signal data_av                       : std_logic;
    signal end_sim_reg                   : std_logic_vector(31 downto 0);
    signal uart_write_data               : std_logic;
    
    signal slack_update_timer			  : std_logic_vector(31 downto 0);
        
begin
	
-----------------------------------------------------------------------------------
-- PE COMPONENTS INSTANTIATION
-----------------------------------------------------------------------------------
	cpu : entity work.mlite_cpu
		port map(
			clk          => clock_hold_s,
			reset_in     => reset,
			intr_in      => irq,
			mem_address  => cpu_mem_address,
			mem_data_w   => cpu_mem_data_write,
			mem_data_r   => cpu_mem_data_read,
			mem_byte_we  => cpu_mem_write_byte_enable,
			mem_pause    => cpu_mem_pause,
			current_page => current_page
		);


master: if kernel_type = "mas" generate
	mem : entity work.ram_master
		port map(
			clk          => clock,
			enable_a     => cpu_enable_ram,
			wbe_a        => cpu_mem_write_byte_enable,
			address_a    => addr_a,
			data_write_a => cpu_mem_data_write,
			data_read_a  => data_read_ram,
			enable_b     => dmni_enable_internal_ram,
			wbe_b        => dmni_mem_write_byte_enable,
			address_b    => addr_b,
			data_write_b => dmni_mem_data_write,
			data_read_b  => mem_data_read
		);
end	generate;
		
slave: if kernel_type = "sla" generate
	mem : entity work.ram_slave
		port map(
			clk          => clock,
			enable_a     => cpu_enable_ram,
			wbe_a        => cpu_mem_write_byte_enable,
			address_a    => addr_a,
			data_write_a => cpu_mem_data_write,
			data_read_a  => data_read_ram,
			enable_b     => dmni_enable_internal_ram,
			wbe_b        => dmni_mem_write_byte_enable,
			address_b    => addr_b,
			data_write_b => dmni_mem_data_write,
			data_read_b  => mem_data_read
		);
end	generate;
	
	router : Entity work.RouterCC
		generic map(address => router_address)
		port map(
			clock    => clock,
			reset    => reset,
			rx       => rx_router,
			data_in  => data_in_router,
			credit_o => credit_o_router,
			tx       => tx_router,
			data_out => data_out_router,
			credit_i => credit_i_router
		);

        
    dmni : entity work.dmni
    	generic map(
    		address_router => router_address
    	)
    	port map(
    		clock          => clock,
    		reset          => reset,
    		--Configuration interface
    		set_address    => cpu_set_address,
    		set_address_2  => cpu_set_address_2,
    		set_size       => cpu_set_size,
    		set_size_2     => cpu_set_size_2,
    		set_op         => cpu_set_op,
    		start          => cpu_start,
    		config_data    => dmni_data_read,

    		-- Status outputs
    		intr           => ni_intr,
    		send_active    => dmni_send_active_sig,
    		receive_active => dmni_receive_active_sig,

    		-- Memory interface
    		mem_address    => dmni_mem_address,
    		mem_data_write => dmni_mem_data_write,
    		mem_data_read  => dmni_mem_data_read,
    		mem_byte_we    => dmni_mem_write_byte_enable,

    		--NoC Interface (Local port)
    		tx             => rx_router(LOCAL),
    		data_out       => data_in_router(LOCAL),
    		credit_i       => credit_o_router(LOCAL),
    		rx             => tx_router(LOCAL),
    		data_in        => data_out_router(LOCAL),
    		credit_o       => credit_i_router(LOCAL)
    	);

    uart : entity work.UartFile
    	generic map(
    		log_file => log_file
    	)
    	port map(
    		reset   => reset,
    		data_av => uart_write_data,
    		data_in => cpu_mem_data_write_reg
    	);
    	
-----------------------------------------------------------------------------------
-- COMBINATIONAL LOGIC AND WIRING
-----------------------------------------------------------------------------------

    --Router external wiring
    rx_router		(3 downto 0)	<= rx;
    credit_i_router	(3 downto 0)	<= credit_i;
    data_in_router	(EAST)			<= data_in(EAST);
    data_in_router	(WEST)			<= data_in(WEST);
    data_in_router	(NORTH)			<= data_in(NORTH);
    data_in_router	(SOUTH)			<= data_in(SOUTH);
    
    tx							<= tx_router		(3 downto 0);
    credit_o					<= credit_o_router	(3 downto 0);
    data_out(EAST)				<= data_out_router	(EAST);
    data_out(WEST)				<= data_out_router	(WEST);
    data_out(NORTH)				<= data_out_router	(NORTH);
    data_out(SOUTH)				<= data_out_router	(SOUTH);
    	
    -- UART 
    uart_write_data <= '1' when cpu_mem_address_reg = DEBUG and write_enable = '1' else '0';
    --debug_busy       <= '0';
    --debug_write_busy <= '0';
    --debug_data_avail <= '0';

	-- CPU data read mux
    MUX_CPU : cpu_mem_data_read <=  ZERO(31 downto 8) & irq_mask_reg when cpu_mem_address_reg = IRQ_MASK else
                                    ZERO(31 downto 8) & irq_status when cpu_mem_address_reg = IRQ_STATUS_ADDR else
                                    time_slice when cpu_mem_address_reg = TIME_SLICE_ADDR else
                                    ZERO(31 downto 16) & router_address when cpu_mem_address_reg = NET_ADDRESS else
                                    tick_counter when cpu_mem_address_reg = TICK_COUNTER_ADDR else  
                                    ZERO(31 downto 1) & dmni_send_active_sig when cpu_mem_address_reg = DMNI_SEND_ACTIVE else                                    
                                    ZERO(31 downto 1) & dmni_receive_active_sig when cpu_mem_address_reg = DMNI_RECEIVE_ACTIVE else
                                    data_read_ram;
    
    --Comb assignments
    addr_a(31 downto 28)                    <= cpu_mem_address(31 downto 28);
    addr_a(27 downto PAGE_SIZE_H_INDEX + 1) <= ZERO(27 downto PAGE_SIZE_H_INDEX + 9) & current_page when current_page /= "00000000" and cpu_mem_address(31 downto PAGE_SIZE_H_INDEX + 1) /= ZERO(31 downto PAGE_SIZE_H_INDEX + 1)
    	else cpu_mem_address(27 downto PAGE_SIZE_H_INDEX + 1);
    addr_a(PAGE_SIZE_H_INDEX downto 2) <= cpu_mem_address(PAGE_SIZE_H_INDEX downto 2);

    addr_b                   <= dmni_mem_address(31 downto 2);
    data_av                  <= '1' when cpu_mem_address_reg = DEBUG and write_enable = '1' else '0';
    --write_enable_debug       <= '1' when cpu_mem_address_reg = DEBUG and write_enable = '1' else '0';
    --data_out_debug           <= cpu_mem_data_write_reg;
    --debug_write_busy         <= busy_debug;
    --debug_busy               <= '1' when cpu_mem_address_reg = DEBUG and write_enable = '1' and busy_debug = '1' else '0';
    cpu_mem_pause            <= '0';
    irq                      <= '1' when (irq_status /= x"00" and irq_mask_reg /= x"00") else '0';
    dmni_data_read           <= cpu_mem_data_write_reg;
    dmni_mem_data_read       <= mem_data_read;
    cpu_enable_ram           <= '1' when cpu_mem_address(30 downto 28) = "000" else '0';
    dmni_enable_internal_ram <= '1' when dmni_mem_address(30 downto 28) = "000" else '0';
    end_sim_reg              <= x"00000000" when cpu_mem_address_reg = END_SIM and write_enable = '1' else x"00000001";
    irq_status(7 downto 4)   <= "00" & ni_intr & '0';
    irq_status(3)            <= '1' when time_slice = x"00000001" else '0';
    irq_status(2)   		 <= '0';
    irq_status(1)   		 <= '1' when dmni_send_active_sig = '0' and slack_update_timer = SLACK_MONITOR_WINDOW else '0';
    irq_status(0)            <= (not dmni_send_active_sig and pending_service);

    cpu_set_size      <= '1' when cpu_mem_address_reg = DMNI_SIZE and write_enable = '1' else '0';
    cpu_set_address   <= '1' when cpu_mem_address_reg = DMNI_ADDR and write_enable = '1' else '0';
    cpu_set_size_2    <= '1' when cpu_mem_address_reg = DMNI_SIZE_2 and write_enable = '1' else '0';
    cpu_set_address_2 <= '1' when cpu_mem_address_reg = DMNI_ADDR_2 and write_enable = '1' else '0';
    cpu_set_op        <= '1' when (cpu_mem_address_reg = DMNI_OP and write_enable = '1') else '0';
    cpu_start         <= '1' when (cpu_mem_address_reg = START_DMNI and write_enable = '1') else '0';
    
    write_enable <= '1' when cpu_mem_write_byte_enable_reg /= "0000" else '0';


-----------------------------------------------------------------------------------
-- SYNCHRONOUS PROCESSES
-----------------------------------------------------------------------------------
    sequential_attr: process(clock, reset)
    begin            
        if reset = '1' then
            cpu_mem_address_reg <= ZERO;
            cpu_mem_data_write_reg <= ZERO;
            cpu_mem_write_byte_enable_reg <= ZERO(3 downto 0);
            irq_mask_reg <= ZERO(7 downto 0);
            time_slice <= ZERO;
            tick_counter <= ZERO;
            pending_service <= '0';
            slack_update_timer <= ZERO;
        elsif (clock'event and clock = '1') then
            if cpu_mem_pause = '0' then
                cpu_mem_address_reg <= cpu_mem_address;
                cpu_mem_data_write_reg <= cpu_mem_data_write;
                cpu_mem_write_byte_enable_reg <= cpu_mem_write_byte_enable;
        
                if cpu_mem_address_reg = IRQ_MASK and write_enable = '1' then
                    irq_mask_reg <= cpu_mem_data_write_reg(7 downto 0);
                end if;     
               -- Decrements the time slice when executing a task (current_page /= x"00") or handling a syscall (syscall = '1')
                if time_slice > 1 then
                    time_slice <= time_slice - 1;
                end if;  

                if(cpu_mem_address_reg = PENDING_SERVICE_INTR and write_enable = '1') then
                    if cpu_mem_data_write_reg = ZERO then
                        pending_service <= '0';
                    else
                        pending_service <= '1';
                    end if;
                end if; 
            end if;
            
            if cpu_mem_address_reg = SLACK_TIME_MONITOR and write_enable = '1' then
            	slack_update_timer <= cpu_mem_data_write_reg;
            elsif slack_update_timer < SLACK_MONITOR_WINDOW then
            	slack_update_timer <= slack_update_timer + 1;
            end if;
                                    
            if cpu_mem_address_reg = TIME_SLICE_ADDR and write_enable = '1' then
                time_slice <= cpu_mem_data_write_reg;
            end if;
                
            tick_counter <= tick_counter + 1;   
        end if;
    end process sequential_attr;

    clock_stop: process(reset,clock)
    begin
        if(reset = '1') then
            tick_counter_local <= (others=> '0');
            clock_aux <= '1';
        else
            if cpu_mem_address_reg = CLOCK_HOLD and write_enable = '1' then
                clock_aux <= '0';
            --elsif tx_router(LOCAL) = '1' or ni_intr = '1' or time_slice = x"00000001" then 
            elsif ni_intr = '1' or time_slice = x"00000001" or irq_status(1) = '1' then 
                clock_aux <= '1';
            end if;

            if(clock_aux ='1' and clock ='1') then
                clock_hold_s <= '1';
                tick_counter_local <= tick_counter_local + 1;
            else
                clock_hold_s <= '0';
            end if;
        end if;
    end process clock_stop;

end architecture structural;

  