---------------------------------------------------------------------------
-- package for components used in MF16 ALU
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use work.mf16cpu_types.all;


package mf16alu_package is

	component fullAdder1bit
		port (a, b: in std_ulogic;
				cin: in std_ulogic;
				sum: out std_ulogic;
				cout: out std_ulogic);
	end component;	

	-- 16 bit full adder
	component add16
		port (a, b: in std_ulogic_vector(15 downto 0);
				cin: in std_ulogic;
				sum: out std_ulogic_vector(15 downto 0);
				cout: out std_ulogic);
	end component;
	
	-- 16 bit equality checker
	component comp16
	port (a, b: in std_ulogic_vector(15 downto 0);
			f: out std_ulogic);
	end component;
	
	component shifter1bit
		port (input: in std_ulogic_vector(15 downto 0);
				instruction: in instructionType;
				output: out std_ulogic_vector(15 downto 0)
				);
	end component;

	component shift16
		port (input: in std_ulogic_vector(15 downto 0);
				instruction: in instructionType;
				shamt: in std_ulogic_vector(3 downto 0);
				output: out std_ulogic_vector(15 downto 0)
				);
	end component;
	
end mf16alu_package;

