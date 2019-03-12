----------------------------------------------------------------------------------
-- Rok 2018
-- Author: Kamil Vojanec (xvojan00)
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.game_pack.ALL; -- reseni prvniho ukolu - bude pouzito spravne reseni

entity cell is
   GENERIC (
      MASK              : mask_t := (others => '1') -- mask_t definovano v baliku math_pack
   );
   Port ( 
      NEIGHBOUR         : in   STD_LOGIC_VECTOR (3 downto 0); -- '1' na urcite pozici urcuje, ze bylo kliknuto na urcitou sousedni bunku
      CLICK             : in   STD_LOGIC; -- '1' urcuje, ze bylo kliknuto na tuto bunku
      
      STATE             : in   STD_LOGIC; -- aktualni stav bunky
      
      LIGHT             : out  STD_LOGIC -- novy stav bunky
   );
end cell;

architecture Behavioral of cell is
  constant IDX_TOP    : NATURAL := 0; -- index sousedni bunky nachazejici se nahore v signalu NEIGHBOUR
  constant IDX_LEFT   : NATURAL := 1; -- index sousedni bunky nachazejici se vlevo  v signalu NEIGHBOUR
  constant IDX_RIGHT  : NATURAL := 2; -- index sousedni bunky nachazejici se vpravo v signalu NEIGHBOUR
  constant IDX_BOTTOM : NATURAL := 3; -- index sousedni bunky nachazejici se dole   v signalu NEIGHBOUR
  
begin

process(NEIGHBOUR, CLICK, STATE)
variable mask_vec : std_logic_vector(3 downto 0) := (others => '0');
variable parity : STD_LOGIC := '1';
variable parity_vector : std_logic_vector(3 downto 0) := (others => '0');
begin
	if MASK.top = '1' then
		mask_vec(0) := '1';
	end if;
	if MASK.left = '1' then
		mask_vec(1) := '1';
	end if;
	if MASK.right = '1' then
		mask_vec(2) := '1';
	end if;
	if MASK.bottom = '1' then
		mask_vec(3) := '1';
	end if;
	
	parity_vector := mask_vec and NEIGHBOUR;

	
	for i in parity_vector'range loop
		parity := parity xor parity_vector(i);
	end loop;


	if CLICK = '1' then
		parity := not parity;
	end if;
	
	if parity = '0' then
		LIGHT <= not STATE;
	else
		LIGHT <= STATE;
	end if;
	parity := '1';
end process;
-- Pozadavky na funkci (pouze kombinacni logika, nesmi byt zadny latch!)
--   pri urceni vystupni hodnoty LIGHT je nutne zohlednit aktualni stav bunky (signal STATE) a dale zda-li bylo kliknuto na bunku (CLICKED) a pripadne na nejake bunky z okoli (NEIGHBOUR)
--   pocet kliknuti (tzn. lichy nebo sudy) timpadem urci novy stav bunky (signal LIGHT)   
--   hodnota bunky z okoli vstupuje do vypoctu jen pokud maska je na prislusne pozici v '1'

end Behavioral;