---------------------------------------------------------------------------
-- 16 bit register architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity regstd is
	port (clk, enable, reset: in std_ulogic;
			initValue: in std_ulogic_vector(15 downto 0); -- initial value for register
			input: in std_ulogic_vector(15 downto 0);
			output: out std_ulogic_vector(15 downto 0));
end entity regstd;

architecture behavior of regstd is
begin
	process is
	begin
		wait until falling_edge(clk);	-- on clock falling edge
		if reset = '1' then				-- if reset
			output <= initValue;			-- output initial value
		elsif enable = '1' then			-- otherwise if enable set
			output <= input;				-- just set register to input data
		end if;
  end process;
end architecture behavior;
---------------------------------------------------------------------------
-- 16 bit instruction register architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity instrReg is
	port (clk, enable: in std_ulogic;
			input: in std_ulogic_vector(15 downto 0);
			output0: out std_ulogic_vector(3 downto 0);
			output1: out std_ulogic_vector(3 downto 0);
			output2: out std_ulogic_vector(3 downto 0);
			output3: out std_ulogic_vector(3 downto 0)
			);
end entity instrReg;

architecture behavior of instrReg is
begin
	process is
	begin
		wait until falling_edge(clk);
		if enable = '1' then
			output0 <= input(3 downto 0);
			output1 <= input(7 downto 4);
			output2 <= input(11 downto 8);
			output3 <= input(15 downto 12);
		end if;
  end process;
end architecture behavior;
---------------------------------------------------------------------------
-- register file architecture
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.mf16cpu_types.all;
use work.mf16cpu_package.all;

entity registerFile is
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
end entity registerFile;

architecture behavior of registerFile is
	
	signal reg_enable: std_ulogic_vector(0 to 15) := (others => '0');
	
begin

	-- all registers except $sp
	gen_reg_array1: for i in 0 to 11 generate
		reg_array1: regstd port map (
			clk => clk,
			reset => reset,
			initValue => x"0000",
			enable => reg_enable(i),
			input => writeData,
			output => reg_out(i)
		);
	end generate gen_reg_array1;
	gen_reg_array2: for i in 13 to 15 generate
		reg_array1: regstd port map (
			clk => clk,
			reset => reset,
			initValue => x"0000",
			enable => reg_enable(i),
			input => writeData,
			output => reg_out(i)
		);
	end generate gen_reg_array2;
	
	-- $sp register
	sp_reg: regstd port map (
		clk => clk,
		reset => reset,
		initValue => x"7ffe", -- initial value for stack pointer
		enable => reg_enable(12),
		input => writeData,
		output => reg_out(12)
	);
	
	-- set outputs
	rsData <= reg_out( to_integer(unsigned(rs_num)) );
	rtData <= reg_out( to_integer(unsigned(rt_num)) );
	rdData <= reg_out( to_integer(unsigned(rd_num)) );

	-- set enable signals for individual registers
	process(writeReg) is
	begin
	
		reg_enable <= x"0000";
		
		case writeReg is
		
			when rs =>
				reg_enable( to_integer(unsigned(rs_num)) ) <= '1';
				
			when rt =>
				reg_enable( to_integer(unsigned(rt_num)) ) <= '1';
				
			when rd =>
				reg_enable( to_integer(unsigned(rd_num)) ) <= '1';
				
			when ra =>
				reg_enable(15) <= '1';
				
			when no =>
				reg_enable <= x"0000";
				
		end case;
		
  end process;	
	
end architecture behavior;