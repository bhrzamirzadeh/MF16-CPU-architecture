---------------------------------------------------------------------------
-- package for components used in the MF16 CPU
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use work.mf16cpu_types.all;


package mf16cpu_package is

	component mf16alu
		port (input1, input2: in std_ulogic_vector(15 downto 0);
				instruction: in instructionType;
				result: out std_ulogic_vector(15 downto 0);
				jump_flag: out std_ulogic);
	end component;

	component led7seg
		port (hexval: in std_ulogic_vector(3 downto 0);
				segments: out std_ulogic_vector(6 downto 0));
	end component;
	
	component regstd
	port (clk, enable, reset: in std_ulogic;
			initValue: in std_ulogic_vector(15 downto 0);
			input: in std_ulogic_vector(15 downto 0);
			output: out std_ulogic_vector(15 downto 0));
	end component;

	component instrReg
		port (clk, enable: in std_ulogic;
				input: in std_ulogic_vector(15 downto 0);
				output0: out std_ulogic_vector(3 downto 0);
				output1: out std_ulogic_vector(3 downto 0);
				output2: out std_ulogic_vector(3 downto 0);
				output3: out std_ulogic_vector(3 downto 0)
				);
	end component;	

	component registerFile
		port (clk, reset: in std_ulogic;
				writeReg: in writeRegType;
				writeData: in std_ulogic_vector(15 downto 0);
				rs_num: in std_ulogic_vector(3 downto 0);
				rt_num: in std_ulogic_vector(3 downto 0);
				rd_num: in std_ulogic_vector(3 downto 0);
				rsData: out std_ulogic_vector(15 downto 0);
				rtData: out std_ulogic_vector(15 downto 0);
				rdData: out std_ulogic_vector(15 downto 0);
				-- debug outputs
				reg_out: inout regArrType
				);
	end component;
	
	component memUnit
		port (clk50mhz : in std_logic;	
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
				memRequest: inout std_ulogic;
				memDone: out std_ulogic
		);
	end component;
	
end mf16cpu_package;

