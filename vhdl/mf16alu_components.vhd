---------------------------------------------------------------------------
-- 16 bit equality checker architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity comp16 is
	port (a, b: in std_ulogic_vector(15 downto 0);
			f: out std_ulogic);
end entity comp16;

architecture dataflow of comp16 is
begin
		f <= '1' when a = b else '0';
end architecture dataflow;
---------------------------------------------------------------------------
-- 1 bit adder architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity fullAdder1bit is
	port (a, b: in std_ulogic;
			cin: in std_ulogic;
			sum: out std_ulogic;
			cout: out std_ulogic);
end entity fullAdder1bit;

architecture dataflow of fullAdder1bit is
begin
	sum <= (a xor b) xor cin;
	cout <= (a and b) or (cin and (a xor b));
end architecture dataflow;
---------------------------------------------------------------------------
-- 16 bit adder structure built from fullAdder1bit
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use work.mf16alu_package.all;


entity add16 is
	port (a, b: in std_ulogic_vector(15 downto 0);
			cin: in std_ulogic;
			sum: out std_ulogic_vector(15 downto 0);
			cout: out std_ulogic);
end entity add16;

architecture structure of add16 is
	-- signals for ripple carry
	signal inner_cout: std_ulogic_vector(15 downto 0);
begin
	
	add0: fullAdder1bit port map (
			a => a(0),
			b => b(0),
			cin => cin,
			sum => sum(0),
			cout => inner_cout(0));
	
	gen_ripple: for i in 14 downto 1 generate
		add1to14: fullAdder1bit port map (
			a => a(i),
			b => b(i),
			cin => inner_cout(i-1),
			sum => sum(i),
			cout => inner_cout(i));
	end generate gen_ripple;
	
	add15: fullAdder1bit port map (
		a => a(15),
		b => b(15),
		cin => inner_cout(14),
		sum => sum(15),
		cout => cout);	
		
end architecture structure;
---------------------------------------------------------------------------
-- 1 bit shifter architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use work.mf16cpu_types.all;

entity shifter1bit is
	port (input: in std_ulogic_vector(15 downto 0);
			instruction: in instructionType;
			output: out std_ulogic_vector(15 downto 0)
			);
end entity shifter1bit;

architecture dataflow of shifter1bit is
begin
	
	-- re-arrange input signal bits into the output signal based on shift type
	with instruction select output <=
		input(14 downto 0) & '0' when i_sll,
		input(0)	& input(15 downto 1) when i_bsr,
		'0' & input(15 downto 1) when others;
		
end architecture dataflow;
---------------------------------------------------------------------------
-- 16 bit shifter architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.mf16cpu_types.all;
use work.mf16alu_package.all;


entity shift16 is
	port (input: in std_ulogic_vector(15 downto 0);
			instruction: in instructionType;
			shamt: in std_ulogic_vector(3 downto 0);
			output: out std_ulogic_vector(15 downto 0)
			);
end entity shift16;

architecture dataflow of shift16 is
	type sigArray is array(14 downto 0) of std_ulogic_vector(15 downto 0);
	
	signal shiftOut: sigArray; -- outputs of each shift stage
	
begin
	
	-- the first 1-bit shifter connected fed by the input
	shifter0: shifter1bit port map (
		input => input,
		instruction => instruction,
		output => shiftOut(0)
	);
	-- the remianing 14 1-bit shifters to the chain of shifters
	shift_chain: for i in 1 to 14 generate
		shifter1to14: shifter1bit port map (
			input => shiftOut(i-1),
			instruction => instruction,
			output => shiftOut(i)
	);
	end generate shift_chain;
	
	-- set final output from stage outputs based on shift amount
	with shamt select output <=
		input when x"0",
		shiftOut(0) when x"1",
		shiftOut(1) when x"2",
		shiftOut(2) when x"3",
		shiftOut(3) when x"4",
		shiftOut(4) when x"5",
		shiftOut(5) when x"6",
		shiftOut(6) when x"7",
		shiftOut(7) when x"8",
		shiftOut(8) when x"9",
		shiftOut(9) when x"a",
		shiftOut(10) when x"b",
		shiftOut(11) when x"c",
		shiftOut(12) when x"d",
		shiftOut(13) when x"e",
		shiftOut(14) when x"f";

end architecture dataflow;