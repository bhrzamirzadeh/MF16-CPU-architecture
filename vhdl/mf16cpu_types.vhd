---------------------------------------------------------------------------
-- type definitions for MF16 CPU
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

package mf16cpu_types is

	-- list of instructions
	type instructionType is (	i_add,
										i_and,
										i_beq,
										i_jr,
										i_jrl,
										i_lui,
										i_lw,
										i_bsr,
										i_nor,
										i_or,
										i_ori,
										i_sll,
										i_slt,
										i_xor,
										i_srl,
										i_sw);

	-- list of memory FSM states
	type memStateType is (	memReady4Next,
									memReady4Ctrl,
									memReady4Data);

	-- list of CPU FSM states
	type cpuStateType is (	init,
									fetch,
									decode,
									executeAndMemAccess,
									writeBack);
					
	type writeRegType is (rs, rt, rd, ra, no); -- values of writeReg signal for register file
	
	type memAddrSrcType is (pc, reg);
	
	type regFileWriteSrcType is (alu, mem, nextPC);
	
	type aluIn1SrcType is (ir, reg);
	
	type regArrType is array(0 to 15) of std_ulogic_vector(15 downto 0);
	
end package mf16cpu_types;