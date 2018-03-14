---------------------------------------------------------------------------
-- Memory unit for problem set 6 question 2
-- 
-- This enitity is meant to sit between the cscie93 memory controller and MF16 CPU.
-- It controlls the memory finite state machine and performs the required handshake
-- with cscie93 memory controller.
-- This FSM changes state on the falling edge of the clock and has 3 states:
-- 	1) memReady4Next: idle or ready for next read/write request or finsihbed write data
--		2) memReady4Ctrl: memory is ready to accept control signals or is providing data
--		3) memReady4Data: memory is ready to accpet write data
--
-- It also passes the memory FSM state out to the CPU.
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use work.mf16cpu_package.all;
use work.mf16cpu_types.all;
library cscie93;
-- This file should be used for the DE2-115 board ONLY
entity memUnit is
	port (
		clk50mhz : in std_logic;	
		mem_addr : in std_logic_vector(15 downto 0);
		mem_data_write : in std_logic_vector(15 downto 0);
		mem_rw : in std_logic;	
		mem_reset : in std_logic;	
		ps2_clk : in std_logic;
		ps2_data : in std_logic;	
		clock_hold : in std_logic;
		clock_step : in std_logic;
		clock_divide_limit : in std_logic_vector(19 downto 0);
		mem_suspend : in std_logic;	
		lcd_en : out std_logic;
		lcd_on : out std_logic;
		lcd_rs : out std_logic;
		lcd_rw : out std_logic;
		lcd_db : inout std_logic_vector(7 downto 0);
		mem_data_read : out std_logic_vector(15 downto 0);
		sysclk1 : inout std_logic;
		rs232_rxd : in std_logic;
		rs232_txd : out std_logic;
		rs232_cts : out std_logic;
		sram_dq : inout std_logic_vector (15 downto 0);
		sram_addr : out std_logic_vector(19 downto 0);
		sram_ce_N : out std_logic;
		sram_oe_N : out std_logic;
		sram_we_N : out std_logic;
		sram_ub_N : out std_logic;
		sram_lb_N : out std_logic;
		serial_character_ready : out std_logic;
		-- memory FSM state
		mem_state : out memStateType;
		memRequest: in std_ulogic;
		memDone: out std_ulogic := '1'
	);
end;

architecture default of memUnit is

	signal mem_addressready		: std_logic;
	signal mem_dataready_inv	: std_logic;
	signal sig_mem_addr 			: std_logic_vector(15 downto 0);
	signal sig_mem_rw				: std_logic;
	signal sig_mem_data_write	: std_logic_vector(15 downto 0);
	signal sig_mem_data_read	: std_logic_vector(31 downto 0);
	
	signal currentState			: memStateType := memReady4Next; -- start with memory ready
	
begin
	memDev : cscie93.memory_controller port map (
	
		clk50mhz						=> clk50mhz,
		
		mem_addr 					=> "00000" & sig_mem_addr,
		mem_data_write				=> x"0000" & sig_mem_data_write,
		mem_rw						=> sig_mem_rw,
		mem_sixteenbit				=> '1',
		mem_thirtytwobit			=> '0',
		mem_addressready			=> mem_addressready,
		mem_reset					=> mem_reset,
		
		ps2_clk						=> ps2_clk,
		ps2_data						=> ps2_data,
		
		clock_hold					=> clock_hold,
		clock_step					=> clock_step,
		clock_divide_limit		=> clock_divide_limit,
		mem_suspend					=> mem_suspend,
		
		lcd_en 						=> lcd_en,
		lcd_on 						=> lcd_on,
		lcd_rs 						=> lcd_rs,
		lcd_rw 						=> lcd_rw,
		lcd_db 						=> lcd_db,
		
		mem_data_read				=> sig_mem_data_read,
		mem_dataready_inv 		=> mem_dataready_inv,
		
		sysclk1 						=> sysclk1,
		
		rs232_rxd 					=> rs232_rxd,
		rs232_txd 					=> rs232_txd,
		rs232_cts 					=> rs232_cts,
		-- SSRAM interface
		sram_dq						=> sram_dq,
		sram_addr					=> sram_addr,
		sram_ce_N 					=> sram_ce_N,
		sram_oe_N					=> sram_oe_N,
		sram_we_N					=> sram_we_N,
		sram_ub_N					=> sram_ub_N,
		sram_lb_N					=> sram_lb_N,
		-- usable as interrupts for char ready
		serial_character_ready 	=> serial_character_ready

	);
	
	-- memUnit FSM to do handshake with memory controller
	memFSM: process is
		variable nextState: memStateType;
	Begin
	
		wait until falling_edge(sysclk1);
		
			case currentState is
				-----------------
				when memReady4Next =>
					if memRequest = '1' then
						memDone <= '0';
						if mem_dataready_inv = '1' then
							nextState := memReady4Ctrl;
							sig_mem_addr <= mem_addr;
							sig_mem_rw <= mem_rw;
							mem_addressready <= '1';
						end if;
					end if;
				-----------------
				when memReady4Ctrl =>
					if mem_dataready_inv = '0' then
						nextState := memReady4Data;
						if mem_rw = '0' then
							mem_data_read <= sig_mem_data_read(15 downto 0);
						else
							sig_mem_data_write <= mem_data_write;
						end if;
						mem_addressready <= '0';
					end if;
				-----------------
				when memReady4Data =>
					if mem_dataready_inv = '1' then
						nextState := memReady4Next;
						memDone <= '1';
					end if;
				
			end case;
			
			currentState <= nextState;
			mem_state <= currentState;
		
	end process memFSM;

end;