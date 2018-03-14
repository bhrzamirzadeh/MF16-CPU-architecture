---------------------------------------------------------------------------
-- MF16 ALU architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

-- include mf16alu components package
use work.mf16alu_package.all;
use work.mf16cpu_types.all;

entity mf16alu is
	port (input1, input2: in std_ulogic_vector(15 downto 0);
			instruction: in instructionType;
			result: out std_ulogic_vector(15 downto 0);
			jump_flag: out std_ulogic);
end entity mf16alu;

architecture dataflow of mf16alu is
	signal add_out, and_out, or_out, nor_out, sub_out, sign, shift_out: std_ulogic_vector(15 downto 0);
	signal is_equal: std_ulogic;

begin
	-- component instatiations
	
	-- 16-bit full adder instance for input1+input2
	adder: add16 port map (
		a => input1,
		b => input2,
		cin => '0',
		sum => add_out);
		
	-- is input1=input2 (for beq instruction)
	comparator16: comp16 port map (
		a => input1,
		b => input2,
		f => is_equal);
		
	-- 16-bit full adder instance to calculate input2-input1 (for slt instruction)
	subtractor: add16 port map (
		a => not input1,
		b => input2,
		cin => '1',
		sum => sub_out);
		
	-- if sign bit from the result of the above subtraction was 1 then set result=1
	sign <= x"0001" when sub_out(15) = '1' else x"0000";

	-- 16-bit shifter instance
	shifter: shift16 port map (
		input => input1,
		shamt => input2(3 downto 0),
		instruction => instruction,
		output => shift_out);	
	
	-- 16-bit 5:1 mux to select which one of the above is ALU result
	with instruction select result <=
		add_out when i_add,
		input1 and input2 when i_and,
		input1(7 downto 0) & x"00" when i_lui,
		shift_out when i_bsr | i_sll | i_srl,
		input1 xor input2 when i_xor,
		input1 nor input2 when i_nor,
		input1 or input2 when i_or | i_ori,
		sign when i_slt,
		x"0000" when others;

	-- set ALU jump flag
	with instruction select jump_flag <=
		is_equal when i_beq,
		'1' when i_jr,
		'1' when i_jrl,
		'0' when others;
		
end architecture dataflow;
















