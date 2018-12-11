-- cpu.vhd: Simple 8-bit CPU (BrainF*ck interpreter)
-- Copyright (C) 2018 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): DOPLNIT
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru

   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti

   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni z pameti (DATA_RDWR='1') / zapis do pameti (DATA_RDWR='0')
   DATA_EN    : out std_logic;                    -- povoleni cinnosti

   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA obsahuje stisknuty znak klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna pokud IN_VLD='1'
   IN_REQ    : out std_logic;                     -- pozadavek na vstup dat z klavesnice

   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- pokud OUT_BUSY='1', LCD je zaneprazdnen, nelze zapisovat,  OUT_WE musi byt '0'
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

   function ascii2hex(ascii_in : std_logic_vector(7 downto 0)) return std_logic_vector is
   begin
      case( ascii_in ) is

         when X"30" =>
            return std_logic_vector(X"0");
         when X"31" =>
            return std_logic_vector(X"1");
         when X"32" =>
            return std_logic_vector(X"2");
         when X"33" =>
            return std_logic_vector(X"3");
         when X"34" =>
            return std_logic_vector(X"4");
         when X"35" =>
            return std_logic_vector(X"5");
         when X"36" =>
            return std_logic_vector(X"6");
         when X"37" =>
            return std_logic_vector(X"7");
         when X"38" =>
            return std_logic_vector(X"8");
         when X"39" =>
            return std_logic_vector(X"9");
         when X"41" =>
            return std_logic_vector(X"a");
         when X"42" =>
            return std_logic_vector(X"b");
         when X"43" =>
            return std_logic_vector(X"c");
         when X"44" =>
            return std_logic_vector(X"d");
         when X"45" =>
            return std_logic_vector(X"e");
         when X"46" =>
            return std_logic_vector(X"f");
         when others =>
            return std_logic_vector(X"0");
      end case;

   end function;

   type fsm_state is (idle, fetch, store, decode, ptr_inc, ptr_dec, val_inc0, val_inc1, val_dec0, val_dec1,
                     while_begin0, while_begin1, while_skip_until_end, while_end, while_skip_until_begin, print,
                     getchar0, getchar1, comment, comment_skip, imm_op, halt);
   signal present_state   : fsm_state;
   signal next_state      : fsm_state;

   signal program_counter : std_logic_vector(11 downto 0);
   signal pc_inc          : std_logic;
   signal pc_dec          : std_logic;

   signal loop_ptr        : std_logic_vector(11 downto 0);
   signal loop_inc        : std_logic;
   signal loop_dec        : std_logic;
   signal loop_zero       : std_logic;

   signal ram_ptr         : std_logic_vector(9 downto 0);
   signal ram_inc         : std_logic;
   signal ram_dec         : std_logic;

   signal mux_sel         : std_logic_vector(1 downto 0);
   signal mux_zero        : std_logic;
   signal instr_decode    : std_logic_vector(7 downto 0);



   -- TODO FSM

 -- zde dopiste potrebne deklarace signalu

begin

   instr_decode <= CODE_DATA;

   pc_cnt : process(CLK, RESET)
   begin
      if RESET = '1' then
         program_counter <= (others => '0');
      elsif rising_edge(CLK) then
         if EN = '1' then
            if pc_inc = '1' then
               program_counter <= program_counter + 1;
            elsif pc_dec = '1' then
               program_counter <= program_counter - 1;
            end if;
         end if;
      end if;
   end process;

   loop_cnt : process(CLK, RESET)
   begin
      if RESET = '1' then
         loop_ptr <= (others => '0');
      elsif rising_edge(CLK) then
         if EN = '1' then
            if loop_inc = '1' then
               loop_ptr <= loop_ptr + 1;
            elsif loop_dec = '1' then
               loop_ptr <= loop_ptr - 1;
            end if;
         end if;
      end if;
   end process;

   loop_zero <= '1' when loop_ptr = (others => '0') else '0';

   ram_cnt : process(CLK, RESET)
   begin
      if RESET = '1' then
         ram_ptr <= (others => '0');
      elsif rising_edge(CLK) then
         if EN = '1' then
            if ram_inc = '1' then
               ram_ptr <= ram_ptr + 1;
            elsif ram_dec = '1' then
               ram_ptr <= ram_ptr - 1;
            end if;
         end if;
      end if;
   end process;


   -- Output multiplexer
   with mux_sel select DATA_WDATA <=
      IN_DATA           when "00",
      CODE_DATA         when "01",
      DATA_RDATA - 1    when "10",
      DATA_RDATA + 1    when "11",
      null              when others;
   -- End output multiplexer

   mux_zero <= '1' when DATA_RDATA = (others => '0') else '0';


   fsm_present_state : process(CLK, RESET)
   begin
      if RESET = '1' then
         present_state <= idle;
      elsif rising_edge(CLK) then
         if EN = '1' then
            present_state <= next_state;
         end if;
      end if;
   end process;



   fsm_next_state : process(present_state, instr_decode)
   begin
      pc_inc <= '0';
      pc_dec <= '0';

      loop_inc <= '0';
      loop_dec <= '0';

      ram_inc <= '0';
      ram_dec <= '0';

      IN_REQ <= '0';
      OUT_WE <= '0';

      CODE_EN <= '1';

      mux_sel <= (others => '0');

      DATA_RDWR <= '1';
      DATA_EN <= '0';

      case( present_state ) is

         when idle =>
            next_state <= fetch;
            CODE_EN <= '1';
         when fetch =>
            next_state <= decode;

         --DECODE INSTRUCTION
         when decode =>
            case( instr_decode ) is

               when X"3E" =>
                  next_state <= ptr_inc;
               when X"3C" =>
                  next_state <= ptr_dec;
               when  X"2B" =>
                  next_state <= val_inc0;
               when X"2D" =>
                  next_state <= val_dec0;
               when X"5B" =>
                  next_state <= while_begin0;
               when X"5D" =>
                  next_state <= while_end;
               when X"2E" =>
                  next_state <= print;
               when X"2C" =>
                  next_state <= getchar0;
               when X"23" =>
                  next_state <= comment;
               when X"30" =>
                  next_state <= imm_op;
               when X"31" =>
                  next_state <= imm_op;
               when X"32" =>
                  next_state <= imm_op;
               when X"33" =>
                  next_state <= imm_op;
               when X"34" =>
                  next_state <= imm_op;
               when X"35" =>
                  next_state <= imm_op;
               when X"36" =>
                  next_state <= imm_op;
               when X"37" =>
                  next_state <= imm_op;
               when X"38" =>
                  next_state <= imm_op;
               when X"39" =>
                  next_state <= imm_op;
               when X"41" =>
                  next_state <= imm_op;
               when X"42" =>
                  next_state <= imm_op;
               when X"43" =>
                  next_state <= imm_op;
               when X"44" =>
                  next_state <= imm_op;
               when X"45" =>
                  next_state <= imm_op;
               when X"46" =>
                  next_state <= imm_op;
               when X"00" =>
                  next_state <= halt;
               when others =>
                  next_state <= halt;
            end case;

         -- INSTRUCTION EXECUTE
         -- INCREMENT RAM POINTER
         when ptr_inc =>
            ram_inc <= '1';
            next_state <= fetch;
            pc_inc <= '1';

         --DECREMENT RAM POINTER
         when ptr_dec =>
            ram_dec <= '1';
            next_state <= fetch;
            pc_inc <= '1';

         --INCREMENT RAM VALUE
         when val_inc0 =>
            DATA_RDWR <= '1';
            DATA_EN <= '1';
            next_state <= val_inc1;
         when val_inc1 =>
            DATA_RDWR <= '0';
            DATA_EN <= '1';
            mux_sel <= "11";
            pc_inc <= '1';
            next_state <= fetch;

         -- DECREMENT RAM VALUE
         when val_dec0 =>
            DATA_RDWR <= '1';
            DATA_EN <= '1';
            next_state <= val_dec1;
         when val_dec1 =>
            DATA_RDWR <= '0';
            DATA_EN <= '1';
            mux_sel <= "10";
            pc_inc <= '1';
            next_state <= fetch;

         -- PRINT
         when print =>
            if OUT_BUSY = '1' then
               next_state <= print;
            else
               DATA_RDWR <= '1';
               DATA_EN <= '1';
               OUT_WE <= '1';
               OUT_DATA <= DATA_RDATA;
               pc_inc <= '1';
               next_state <= fetch;
            end if;

         -- GETCHAR
         when getchar0 =>
            IN_REQ <= '1';
            next_state <= getchar1;
         when getchar1 =>
            if  (not IN_VLD) then
               next_state <= getchar1;
            else
               DATA_RDWR <= '0';
               DATA_EN <= '1';
               DATA_WDATA <= IN_DATA;
               pc_inc <= '1';
               next_state <= fetch;
            end if;

         -- WHILE
         when while_begin0 =>
            pc_inc <= '1';
            DATA_RDWR <= '1';
            DATA_EN <= '1';
            next_state <= while_begin1;
         when while_begin1 =>
            if DATA_RDATA = (others => '0') then
               loop_ptr <= (0 => '1', others => '0');
               next_state <= while_skip_until_end;
            else
               next_state <= fetch;
            end if;
         when while_skip_until_end =>
            if loop_zero = '0' then
               if instr_decode = X"5B" then
                  loop_inc <= '1';
               elsif instr_decode = X"5D" then
                  loop_dec <= '1';
               end if;
               pc_inc <= '1';
               next_state <= while_skip_until_end;
            else
               pc_inc <= '1';
               next_state <= fetch;
            end if;

         -- WHILE END
         when while_end =>
            if DATA_RDATA = (others => '0') then
               pc_inc <= '1';
               next_state <= fetch;
            else
               loop_ptr <= (0 => '1', others => '0');
               pc_dec <= '1';
               next_state <= while_skip_until_begin;
            end if;
         when while_skip_until_begin =>
            if loop_zero = '0' then
               if instr_decode = X"5D" then
                  loop_inc <= '1';
               elsif instr_decode = X"5B" then
                  loop_dec <= '1';
               end if;
               if loop_zero = '1' then
                  pc_inc <= '1';
               else
                  pc_dec <= '0';
               end if;
            end if;

         -- COMMENT
         when comment =>
            pc_inc <= '1';
            next_state <= comment_skip;
         when comment_skip =>
            if instr_decode /= X"23" then
               pc_inc <= '1';
               next_state <= comment_skip;
            else
               pc_inc <= '1';
               next_state <= fetch;
            end if;

         -- IMMEDIATE OPERAND
         when imm_op =>
            DATA_WDATA <= (7 downto 3 => ascii2hex(instr_decode), others => '0');
            DATA_RDWR <= '0';
            DATA_EN <= '1';
            pc_inc <= '1';
            next_state <= fetch;

         when halt =>
            null;
         when others =>
            pc_inc <= '1';
      end case;
   end process;


 -- zde dopiste vlastni VHDL kod dle blokoveho schema

 -- inspirujte se kodem procesoru ze cviceni

end behavioral;
