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

    --signal led_aux : std_logic_vector(9 downto 0) ;
    signal led_out_k : std_logic_vector(7 downto 0) ;
    signal led_out_v : std_logic_vector(7 downto 0) ;

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
        if( ctrl_ce = '1') then
            row_shreg <= row_shreg(6 downto 0) & row_shreg(7);
        end if;
      end if ;
    end process ; -- row_cnt

    ROW <= row_shreg;
    -- End row counter


    -- LED Decoder
    --led_aux <= switch & row_shreg;

    with row_shreg select led_out_k <=
        "10111011" when "00000001",
        "10110111" when "00000010",
        "10101111" when "00000100",
        "10011111" when "00001000",
        "10011111" when "00010000",
        "10101111" when "00100000",
        "10110111" when "01000000",
        "10111011" when "10000000",
        (others => '1') when others;

    with row_shreg select led_out_v <=
        "01111110" when "00000001",
        "01111110" when "00000010",
        "10111101" when "00000100",
        "10111101" when "00001000",
        "11011011" when "00010000",
        "11011011" when "00100000",
        "11100111" when "01000000",
        "11100111" when "10000000",
        (others => '1') when others;

    -- END LED Decoder



    -- LED ON/OFF switch

    with switch select LED <=
        (others => '1') when "00",
        led_out_k       when "01",
        (others => '1') when "10",
        led_out_v       when "11",
        (others => '1') when others;

    -- END LED ON/OFF switch




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
