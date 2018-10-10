library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity ledc8x8 is
port ( -- Sem doplnte popis rozhrani obvodu.
    SMCLK : in std_logic;
    RESET : in std_logic;

    ROW : out std_logic_vector(7 downto 0) ;
    LED : out std_logic_vector(7 downto 0)
);
end ledc8x8;

architecture main of ledc8x8 is

    -- Sem doplnte definice vnitrnich signalu.

    signal ce_cnt : std_logic_vector(23 downto 0) ;
    signal switch : std_logic_vector(1 downto 0);
    signal ctrl_ce : std_logic;

    signal row_shreg : std_logic_vector(7 downto 0) ;

    signal led_aux : std_logic_vector(9 downto 0) ;
    signal led_out : std_logic_vector(7 downto 0) ;

begin

    
    --  Control block
    ctrl_cnt : process( SMCLK, RESET )
    begin
      if( RESET = '1' ) then
        ce_cnt <= (others => '0');
      elsif( rising_edge(SMCLK) ) then
        ce_cnt <= ce_cnt + 1;
      end if ;
    end process ; -- ctrl_cnt

    switch <= ce_cnt(23 downto 22);
    ctrl_ce <= '1' when ce_cnt(7 downto 0) = X"FF" else '0';
    -- End control block

    -- Row counter
    row_cnt : process( SMCLK, RESET )
    begin
      if( RESET = '1' ) then
        row_shreg <= (0 => '1', others => '0');
      elsif( rising_edge(SMCLK) ) then
        row_shreg <= row_shreg(6 downto 0) & row_shreg(7);
      end if ;
    end process ; -- row_cnt

    ROW <= row_shreg;
    -- End row counter


    -- LED Decoder
    led_aux <= switch & row_shreg;

    led_dec : process( led_aux )
    begin
        led_out <= (others => '0');
    end process ; -- led_dec

    -- END LED Decoder




    -- Sem doplnte popis obvodu. Doporuceni: pouzivejte zakladni obvodove prvky
    -- (multiplexory, registry, dekodery,...), jejich funkce popisujte pomoci
    -- procesu VHDL a propojeni techto prvku, tj. komunikaci mezi procesy,
    -- realizujte pomoci vnitrnich signalu deklarovanych vyse.

    -- DODRZUJTE ZASADY PSANI SYNTETIZOVATELNEHO VHDL KODU OBVODOVYCH PRVKU,
    -- JEZ JSOU PROBIRANY ZEJMENA NA UVODNICH CVICENI INP A SHRNUTY NA WEBU:
    -- http://merlin.fit.vutbr.cz/FITkit/docs/navody/synth_templates.html.

    -- Nezapomente take doplnit mapovani signalu rozhrani na piny FPGA
    -- v souboru ledc8x8.ucf.

end main;
