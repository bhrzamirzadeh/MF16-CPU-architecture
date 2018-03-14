---------------------------------------------------------------------------
-- Sequencer top level entity for MF16 CPU
--
-- The CPU FSM changes state on the rising edge of the clock and has 5 states:
-- 	1) init: reset all registers to their default value (not part of the execution cycle)
--		2) fetch: setup to read next instruction from memory
--		3) decode: setup to write the fetched instruction into IR
--		4) executeAndMemAccess: setup ALU signals, branching signals and memory access signals
--		5) writeBack: setup to write PC and also write results to register file
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.mf16cpu_package.all;
use work.mf16alu_package.all;
use work.mf16cpu_types.all;

entity mf16cpu is
	port (
		-- memory controller signals for pins
		clk50mhz : in std_logic;
		ps2_clk : in std_logic;
		ps2_data : in std_logic;
		lcd_en : out std_logic;
		lcd_on : out std_logic;
		lcd_rs : out std_logic;
		lcd_rw : out std_logic;
		lcd_db : inout std_logic_vector(7 downto 0);
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
		-- signals for 7-segment LEDs to display MIF line/instruction/register content
		hex7: out std_ulogic_vector(6 downto 0);
		hex6: out std_ulogic_vector(6 downto 0);
		hex5: out std_ulogic_vector(6 downto 0);
		hex4: out std_ulogic_vector(6 downto 0);
		hex3: out std_ulogic_vector(6 downto 0);
		hex2: out std_ulogic_vector(6 downto 0);
		hex1: out std_ulogic_vector(6 downto 0);
		hex0: out std_ulogic_vector(6 downto 0);
		-- signals for red LEDs to display memory FSM state
		ledR: out std_ulogic_vector(17 downto 0);
		-- signals for green LEDs to display cpu FSM state
		ledG: out std_ulogic_vector(8 downto 0);
		-- signals for slide switches and push buttons
		sw: in std_ulogic_vector(17 downto 0);
		key: in std_ulogic_vector(3 downto 0)
	);
end;

architecture default of mf16cpu is

	-- Pin assignments
	attribute chip_pin : string;
		-- 7 segment LEDs
		attribute chip_pin of hex7: signal is "AA14,AG18,AF17,AH17,AG17,AE17,AD17";
		attribute chip_pin of hex6: signal is "AC17,AA15,AB15,AB17,AA16,AB16,AA17";
		attribute chip_pin of hex5: signal is "AH18,AF18,AG19,AH19,AB18,AC18,AD18";
		attribute chip_pin of hex4: signal is "AE18,AF19,AE19,AH21,AG21,AA19,AB19";
		attribute chip_pin of hex3: signal is "Y19,AF23,AD24,AA21,AB20,U21,V21";
		attribute chip_pin of hex2: signal is "W28,W27,Y26,W26,Y25,AA26,AA25";
		attribute chip_pin of hex1: signal is "U24,U23,W25,W22,W21,Y22,M24";
		attribute chip_pin of hex0: signal is "H22,J22,L25,L26,E17,F22,G18";
		-- slide switches and pushbuttons
		attribute chip_pin of sw: signal is "Y23,Y24,AA22,AA23,AA24,AB23,AB24,AC24,AB25,AC25,AB26,AD26,AC26,AB27,AD27,AC27,AC28,AB28";
		attribute chip_pin of key: signal is "R24,N21,M21,M23";
		-- red and green LEDs
		attribute chip_pin of ledR: signal is "H15,G16,G15,F15,H17,J16,H16,J15,G17,J17,H19,J19,E18,F18,F21,E19,F19,G19";
		attribute chip_pin of ledG: signal is "F17,G21,G22,G20,H21,E24,E25,E22,E21";
		-- memory controller pins
		attribute chip_pin of clk50mhz	: signal is "Y2";
		attribute chip_pin of ps2_clk		: signal is "G6";
		attribute chip_pin of ps2_data	: signal is "H5";
		attribute chip_pin of lcd_on		: signal is "L5";
		attribute chip_pin of lcd_en		: signal is "L4";
		attribute chip_pin of lcd_rw		: signal is "M1";
		attribute chip_pin of lcd_rs		: signal is "M2";
		attribute chip_pin of lcd_db		: signal is "M5,M3,K2,K1,K7,L2,L1,L3";
		attribute chip_pin of rs232_rxd	: signal is "G12";
		attribute chip_pin of rs232_txd	: signal is "G9";
		attribute chip_pin of rs232_cts	: signal is "G14";
		attribute chip_pin of sram_dq		: signal is "AG3,AF3,AE4,AE3,AE1,AE2,AD2,AD1,AF7,AH6,AG6,AF6,AH4,AG4,AF4,AH3";
		attribute chip_pin of sram_addr	: signal is "T8,AB8,AB9,AC11,AB11,AA4,AC3,AB4,AD3,AF2,T7,AF5,AC5,AB5,AE6,AB6,AC7,AE7,AD7,AB7";
		attribute chip_pin of sram_ce_N	: signal is "AF8";
		attribute chip_pin of sram_oe_N	: signal is "AD5";
		attribute chip_pin of sram_we_N	: signal is "AE8";
		attribute chip_pin of sram_ub_N	: signal is "AC4";
		attribute chip_pin of sram_lb_N	: signal is "AD4";

	-- 7 segment LED inputs
	signal hex7_in: std_ulogic_vector(3 downto 0);
	signal hex6_in: std_ulogic_vector(3 downto 0);
	signal hex5_in: std_ulogic_vector(3 downto 0);
	signal hex4_in: std_ulogic_vector(3 downto 0);
	signal hex3_in: std_ulogic_vector(3 downto 0);
	signal hex2_in: std_ulogic_vector(3 downto 0);
	signal hex1_in: std_ulogic_vector(3 downto 0);
	signal hex0_in: std_ulogic_vector(3 downto 0);
	
	-- control signals
	signal writePC: std_ulogic := '0';
	signal memAddrSrc: memAddrSrcType := pc;
	signal writeMem: std_ulogic := '0';
	signal writeIR: std_ulogic := '0';
	signal regFileWriteSrc: regFileWriteSrcType := alu;
	signal writeReg: writeRegType := no;
	signal doBranch: std_ulogic := '0';
	signal aluIn1Src: aluIn1SrcType := reg;
	signal aluOpcode: instructionType;
	signal memRequest: std_ulogic := '0';
	signal memDone: std_ulogic;
	signal init_sig: std_ulogic;
	
	-- data signals
		-- pc
		signal pc_out: std_ulogic_vector(15 downto 0) := x"0000";
		signal pc_in: std_ulogic_vector(15 downto 0);
		--memory
		signal sysclk1: std_logic;
		signal mem_data_read: std_logic_vector(15 downto 0);
		signal mem_state: memStateType;
		signal clk_div: std_logic_vector(19 downto 0);
		signal mem_addr: std_ulogic_vector(15 downto 0);
		signal serial_character_ready	: std_logic;
		-- ir
		signal ir_out_rd: std_ulogic_vector(3 downto 0);
		signal ir_out_rt: std_ulogic_vector(3 downto 0);
		signal ir_out_rs: std_ulogic_vector(3 downto 0);
		signal opcode: std_ulogic_vector(3 downto 0);
		-- register file
		signal reg_data_write: std_ulogic_vector(15 downto 0);
		signal rs_data_read: std_ulogic_vector(15 downto 0);
		signal rt_data_read: std_ulogic_vector(15 downto 0);
		signal rd_data_read: std_ulogic_vector(15 downto 0);
		signal reg_out: regArrType;
		-- alu
		signal alu_in1: std_ulogic_vector(15 downto 0);
		signal alu_in2: std_ulogic_vector(15 downto 0);
		signal instruction: instructionType;
		signal alu_result: std_ulogic_vector(15 downto 0);
		signal alu_jump_flag: std_ulogic;
		-- pc+1 adder
		signal pc_plus_1: std_ulogic_vector(15 downto 0);
		
		signal currCPUState: cpuStateType := init; -- initialize CPU state
		signal instrStep: std_ulogic := '0'; -- instruction step request
		signal key1pressed: std_ulogic := '0'; -- for KEY1 press detection
	
begin

	-- CPU component instantiations

	-- PC register
	pcReg: regstd port map (
		clk => sysclk1,
		enable => writePC,
		reset => init_sig,
		initValue => x"0000",
		input => pc_in,
		output => pc_out
	);
	
	-- PC adder
	pcAdder: add16 port map (
		a => pc_out,
		b => x"0002",
		cin => '0',
		sum => pc_plus_1
	);
	
	-- Instruction register
	ir_dev: instrReg port map (
		clk => sysclk1,
		enable => writeIR,
		input => to_stdulogicvector(mem_data_read),
		output0 => ir_out_rd,
		output1 => ir_out_rt,
		output2 => ir_out_rs,
		output3 => opcode
	);
	
	-- Register file
	regFile: registerFile port map (
		clk => sysclk1,
		reset => init_sig,
		writeReg => writeReg,
		writeData => reg_data_write,
		rs_num => ir_out_rs,
		rt_num => ir_out_rt,
		rd_num => ir_out_rd,
		rsData => rs_data_read,
		rtData => rt_data_read,
		rdData => rd_data_read,
		reg_out => reg_out
	);
	
	-- ALU
	alu_dev: mf16alu port map (
		input1 => alu_in1,
		input2 => rs_data_read,
		instruction => instruction,
		result => alu_result,
		jump_flag => alu_jump_flag
	);

	-- Memory unit
	memory: memUnit port map (
	
		clk50mhz						=> clk50mhz,
		mem_addr 					=> to_stdlogicvector(mem_addr) and x"fffe",
		mem_data_write				=> to_stdlogicvector(rs_data_read),
		mem_rw						=> writeMem,
		mem_reset					=> not key(3),
		ps2_clk						=> ps2_clk,
		ps2_data						=> ps2_data,
		clock_hold					=> sw(16),
		clock_step					=> not key(2),
		clock_divide_limit		=> clk_div,
		mem_suspend					=> sw(17),
		lcd_en 						=> lcd_en,
		lcd_on 						=> lcd_on,
		lcd_rs 						=> lcd_rs,
		lcd_rw 						=> lcd_rw,
		lcd_db 						=> lcd_db,
		mem_data_read				=> mem_data_read,
		sysclk1 						=> sysclk1,
		rs232_rxd 					=> rs232_rxd,
		rs232_txd 					=> rs232_txd,
		rs232_cts 					=> rs232_cts,
		sram_dq						=> sram_dq,
		sram_addr					=> sram_addr,
		sram_ce_N 					=> sram_ce_N,
		sram_oe_N					=> sram_oe_N,
		sram_we_N					=> sram_we_N,
		sram_ub_N					=> sram_ub_N,
		sram_lb_N					=> sram_lb_N,
		serial_character_ready 	=> serial_character_ready,
		mem_state					=> mem_state,
		memRequest					=> memRequest,
		memDone						=> memDone
	);
		
	-- 7 segment LEDs
	h7: led7seg port map (
		hexval => hex7_in,
		segments => hex7);
	h6: led7seg port map (
		hexval => hex6_in,
		segments => hex6);
	h5: led7seg port map (
		hexval => hex5_in,
		segments => hex5);
	h4: led7seg port map (
		hexval => hex4_in,
		segments => hex4);
	h3: led7seg port map (
		hexval => hex3_in,
		segments => hex3);
	h2: led7seg port map (
		hexval => hex2_in,
		segments => hex2);	
	h1: led7seg port map (
		hexval => hex1_in,
		segments => hex1);
	h0: led7seg port map (
		hexval => hex0_in,
		segments => hex0);	
		
	-- convert opcode number to named type
	with opcode select instruction <=
		i_add when x"0",
		i_and when x"1",
		i_beq when x"2",
		i_jr when x"3",
		i_jrl when x"4",
		i_lui when x"5",
		i_lw when x"6",
		i_bsr when x"7",
		i_nor when x"8",
		i_or when x"9",
		i_ori when x"a",
		i_sll when x"b",
		i_slt when x"c",
		i_xor when x"d",
		i_srl when x"e",
		i_sw when x"f";
		
	-- memory address mux
	with memAddrSrc select mem_addr <=
		pc_out when pc,
		rt_data_read when reg;
	
	-- register file write data mux
	with regFileWriteSrc select reg_data_write <=
		alu_result when alu,
		to_stdulogicvector(mem_data_read) when mem,
		pc_plus_1 when nextPC;
	
	-- alu input1 mux
	with aluIn1Src select alu_in1 <=
		x"00" & ir_out_rt & ir_out_rd when ir,
		rt_data_read when reg;
	
	-- PC register input mux
	with doBranch and alu_jump_flag select pc_in <=
		rd_data_read when '1',
		pc_plus_1 when '0';
	
	-- clock speed with SW15
	with sw(15) select clk_div <=
		x"00004" when '0', -- 6.25 MHz
		x"fffff" when '1'; -- 23 Hz
	
	-- Memory unit state on rightmost red LEDs
	with mem_state select ledR(2 downto 0) <=
		"100" when memReady4Next,
		"010" when memReady4Ctrl,
		"001" when memReady4Data;
	
	-- CPU state on leftmost green LEDs
	with currCPUState select ledG(7 downto 4) <=
		"1111" when init,
		"1000" when fetch,
		"0100" when decode,
		"0010" when executeAndMemAccess,
		"0001" when writeBack;

---------------------
-- CPU state controller
	cpuFSM: process is
		variable nextCPUState: cpuStateType;
	Begin
		
		wait until rising_edge(sysclk1);
		
		case currCPUState is
			-----------------
			when init =>
				init_sig <= '1'; -- reset all registers to their default values on the next falling edge
				nextCPUState := fetch;
			-----------------
			when fetch =>
				init_sig <= '0'; -- we're cycling now so no more register resetting
				
				if memDone = '1' then -- if memory is idle
					nextCPUState := decode;
					writePC <= '0';
					doBranch <= '0';
					writeReg <= no;
					memAddrSrc <= pc;
					writeMem <= '0';
					memRequest <= '1'; -- ask memory for to read an instruction on next falling edge
				end if;
			-----------------
			when decode =>
				if memDone = '1' then -- if memory is idle
					nextCPUState := executeAndMemAccess;
					memRequest <= '0'; -- relieve memory
					writeIR <= '1'; -- latch instruction in IR on the next falling edge
				end if;
			-----------------		
			when executeAndMemAccess =>
				-- we didnt request memory to do anything in the previous state
				-- so no need to check if it is idle here
				
				-- handle instruction stepping
				-- 
				if sw(14) = '1' then -- if in instruction stepping mode
					if instrStep = '1' then -- and if a step was reuqested
						nextCPUState := writeBack; -- increment state
						instrStep <= '0'; -- acknowledge step request
					end if;
				else -- if NOT in instruction stepping mode
					nextCPUState := writeBack; -- just increment state
				end if;
				
				writeIR <= '0';
				
				-- set some execution signals based on the instruction
				case instruction is
					when i_add | i_and | i_or | i_nor | i_slt | i_sll | i_srl | i_xor | i_bsr =>
						aluIn1Src <= reg;
					when i_ori | i_lui =>
						aluIn1Src <= ir;
					when i_beq | i_jr | i_jrl =>
						aluIn1Src <= reg;
						doBranch <='1';
					when i_lw =>
						memAddrSrc <= reg;
						writeMem <= '0';
						memRequest <= '1';
					when i_sw =>
						memAddrSrc <= reg;
						writeMem <= '1';
						memRequest <= '1';
				end case;
			-----------------	
			when writeBack =>
				if memDone = '1' then -- if memory is idle
					nextCPUState := fetch;
					memRequest <= '0'; -- relieve memory if needed
					writePC <= '1';-- latch PC on the next falling edge
					
					-- latch stuff in register file based on instruction
					case instruction is
						when i_add | i_and | i_or | i_nor | i_sll | i_srl | i_xor | i_bsr | i_slt =>
							regFileWriteSrc <= alu;
							writeReg <= rd;
						when i_ori | i_lui =>
							regFileWriteSrc <= alu;
							writeReg <= rs;
						when i_jrl =>
							regFileWriteSrc <= nextPC;
							writeReg <= ra;
						when i_lw =>
							regFileWriteSrc <= mem;
							writeReg <= rs;
						when i_beq | i_sw | i_jr =>
							null;
					end case;
				end if;
				
		end case;
			
		currCPUState <= nextCPUState;
		
		-- reset program if KEY0 is pressed
		if not key(0) = '1' then
			currCPUState <= init;
		end if;
		
		
		if not key(1) = '1' then -- if KEY1 is physically pressed
			if key1pressed = '0' then -- and its previous state is not pressed
				instrStep <= '1'; -- set instruction step request
				key1pressed <= '1'; -- set key state to pressed
			end if;
		else -- if KEY1 is not physically pressed
			instrStep <= '0';
			key1pressed <= '0';
		end if;
				
	end process cpuFSM;
	
-------------
-- updating LEDs based on SW14, SW(3 to 0) states
	debug: process(sysclk1) is
	Begin
		
		case sw(13) is
		
			when '0' => -- show PC and instruction when off
				-- PC = hex 7 to 4
				hex7_in <= '0' & pc_out(15 downto 13);
				hex6_in <= pc_out(12 downto 9);
				hex5_in <= pc_out(8 downto 5);
				hex4_in <= pc_out(4 downto 1);
				-- Instruction = hex 3 to 0
				hex3_in <= opcode;
				hex2_in <= ir_out_rs;
				hex1_in <= ir_out_rt;
				hex0_in <= ir_out_rd;
				
			when '1' => -- show MIF line and register content
				-- MIF line = hex 7 to 4
				hex7_in <= '0' & pc_out(15 downto 13);
				hex6_in <= pc_out(12 downto 9);
				hex5_in <= pc_out(8 downto 5);
				hex4_in <= pc_out(4 downto 1);
				-- Register content = hex 3 to 0
				-- Register number comes from SW(3 to 0)
				hex3_in <= reg_out( to_integer(unsigned( sw(3 downto 0) )) )(15 downto 12);
				hex2_in <= reg_out( to_integer(unsigned( sw(3 downto 0) )) )(11 downto 8);
				hex1_in <= reg_out( to_integer(unsigned( sw(3 downto 0) )) )(7 downto 4);
				hex0_in <= reg_out( to_integer(unsigned( sw(3 downto 0) )) )(3 downto 0);	
				
		end case;
		
	end process debug;
end;