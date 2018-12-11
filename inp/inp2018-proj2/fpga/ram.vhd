-- ram.vhd : RAM memory
-- Copyright (C) 2018 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL; 
use IEEE.std_logic_ARITH.ALL;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity ram is
 port (
   CLK   : in std_logic; -- hodiny

   ADDR  : in std_logic_vector(9 downto 0);  -- adresa bunky
   WDATA : in std_logic_vector(7 downto 0);  -- data pro zapis
   RDATA : out std_logic_vector(7 downto 0); -- nactena data (v dalsim taktu, pokud EN=1)
   RDWR  : in std_logic;                     -- 1 - cteni, 0 - zapis
   EN    : in std_logic                      -- 1 - povoleni prace s pameti
 );
end ram;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of ram is

   type t_ram is array (0 to 2**10-1) of std_logic_vector (7 downto 0);
   signal ram: t_ram := (others => X"00"); -- pocatecni obsah pameti = same nuly
   signal rd : std_logic_vector (7 downto 0) := (others => '0');
begin
   RDATA <= rd;
   
   -- RAM rd / wr
   sram_mem: process (CLK) -- sekvencni prvek, reaguje pouze na zmeny CLK
   begin
      --pri kazde vzestupne hrane
      if (CLK'event) and (CLK = '1') then
         -- pri ktere je nastaven signal EN na hodnotu logicka 1
         if (EN = '1') then
            if (RDWR = '0') then
               -- pokud se zapisuje do pameti, prepsat polozku na adrese ADDR hodnotou WDATA
               ram(conv_integer(ADDR)) <= WDATA;
               -- aktualizovat hodnotu signalu rd zapisovanou hodnotou
               rd <= WDATA;
            else
               -- pokud se cte z pameti, aktualizovat hodnotu signalu rd polozkou na adrese ADDR
               rd <= ram(conv_integer(ADDR));
            end if;
         end if;
      end if;
   end process;
   
end behavioral;

