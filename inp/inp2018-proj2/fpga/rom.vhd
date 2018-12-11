-- rom.vhd : ROM memory
-- Copyright (C) 2018 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL; 
use ieee.std_logic_arith.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity rom is
 generic (
   INIT  : string := ""
 );
 port (
   CLK   : in std_logic; -- clock

   EN    : in  std_logic;
   ADDR  : in std_logic_vector(11 downto 0); -- address
   DATA  : out std_logic_vector(7 downto 0) -- data
 );
end rom;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of rom is
   type t_ram is array (0 to 2**12-1) of std_logic_vector(7 downto 0);

   -- prevod retezce typu string delky size na pole 8-bitovych polozek typu t_ram
   function str2vec(arg: string; size: integer) return t_ram is
      variable result: t_ram := (others => (others => '0'));
    begin
      for i in arg'range loop
          result(i-1) := conv_std_logic_vector(character'pos(arg(i)), 8);
      end loop;
      return result;
    end; 

   -- deklarace signalu realizujici pamet ROM (pole konstant)
   constant mem: t_ram := str2vec(INIT, 2*2048);
   signal dout : std_logic_vector(7 downto 0) := (others => '0');
begin

   process (CLK)  -- sekvencni prvek, reaguje pouze na zmeny CLK
   begin
      -- pri kazde vzestupne hrane
      if (CLK'event) and (CLK = '1') then
         -- pokud je EN nastaven na logickou 1
         if (EN = '1') then
            -- aktualizovat hodnotu signalu dout
            dout <= mem(conv_integer(ADDR));
         end if;
      end if;
   end process;

   DATA <= dout;
   
end behavioral;

