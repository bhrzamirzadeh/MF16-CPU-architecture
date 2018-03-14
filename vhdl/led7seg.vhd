---------------------------------------------------------------------------
-- 4 bit binary number to 7-segment Hex display decoder
---------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity led7seg is
	-- input 4 bit value
	port (hexval: in std_ulogic_vector(3 downto 0);
			-- signals for 7 seqments of the LED
			segments: out std_ulogic_vector(6 downto 0));
end entity led7seg;

architecture dataflow of led7seg is
begin
	process(hexval) is
		begin
			case hexval is -- assign 0 thorugh F
				when x"0" => segments <= "1000000";
				when x"1" => segments <= "1111001";
				when x"2" => segments <= "0100100";
				when x"3" => segments <= "0110000";
				when x"4" => segments <= "0011001";
				when x"5" => segments <= "0010010";
				when x"6" => segments <= "0000010";
				when x"7" => segments <= "1111000";
				when x"8" => segments <= "0000000";
				when x"9" => segments <= "0010000";
				when x"a" => segments <= "0001000";
				when x"b" => segments <= "0000011";
				when x"c" => segments <= "1000110";
				when x"d" => segments <= "0100001";
				when x"e" => segments <= "0000110";
				when x"f" => segments <= "0001110";
			end case;
	end process;
end architecture dataflow;