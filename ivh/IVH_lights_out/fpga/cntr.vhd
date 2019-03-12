----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    07:39:24 03/27/2018 
-- Design Name: 
-- Module Name:    cntr - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.all;
use ieee.numeric_std.all;
use ieee.std_logic_arith.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity cntr is
	generic(
		INITIAL_VALUE : natural := 2**25 - 100
	);
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           ENABLE : in  STD_LOGIC;
           VALUE: out  STD_LOGIC_VECTOR(24 downto 0);
			  DONE : out STD_LOGIC
			  );
end cntr;

architecture Behavioral of cntr is
signal value_tmp : natural range 0 to 2**25 := INITIAL_VALUE;
signal done_tmp :std_logic := '0';
begin
VALUE <= conv_std_logic_vector(value_tmp, 25);
DONE <= done_tmp;
process (CLK, RESET, value_tmp) 
begin
	if RESET='1' then 
         value_tmp <= INITIAL_VALUE;
			done_tmp <= '0';
	elsif CLK='1' and CLK'event then
      if RESET='1' then 
         value_tmp <= 0;
			done_tmp <= '0';
      elsif ENABLE='1' and done_tmp='0' then
         value_tmp <= value_tmp + 1;
      end if;
		
		if value_tmp = 2**25-1 then
			done_tmp <= '1';
		end if;
   end if;
	
end process; 



end Behavioral;
