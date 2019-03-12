library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.game_pack.all;



architecture main of tlv_gp_ifc is
signal matrix : game_matrix;
type light_array_t is array (0 to 4, 0 to 4) of std_logic;
signal LIGHT : light_array_t;
signal init_vector : std_logic_vector(24 downto 0) := conv_std_logic_vector(0,25);
signal solution : std_logic_vector(24 downto 0) := conv_std_logic_vector(0,25);
signal done : std_logic := '0';
signal enable : std_logic := '0';
signal neighbour : std_logic_vector(3 downto 0) := conv_std_logic_vector(0,4);
signal click : std_logic := '0';
signal state : std_logic := '0';
signal data_addr : std_logic_vector(1 downto 0);
signal data_out : std_logic_vector(24 downto 0);
signal data_in : std_logic_vector(24 downto 0);
signal data_we: std_logic;
signal data_re: std_logic;



begin

spidecc: entity work.SPI_adc generic map
(
	ADDR_WIDTH => 8,--Address width 8 bits
	DATA_WIDTH => 25,--sirka dat 32bitu
	ADDR_OUT_WIDTH => 2,--sirka adresy na vystupu(2 bity)
	BASE_ADDR  => 16#80# --adresovy prostor od 0x80
)
port map
(
	CLK      => CLK,
	CS       => SPI_CS,
	DO       => SPI_DO,
	DO_VLD   => SPI_DO_VLD,
	DI       => SPI_DI,
	DI_REQ   => SPI_DI_REQ,
	ADDR     => data_addr,
	DATA_OUT => data_out,
	DATA_IN  => data_in,
	WRITE_EN => data_we,
	READ_EN  => data_re  
);

counter : entity work.cntr 		GENERIC MAP (0)
								PORT MAP (CLK, RESET, ENABLE, solution, DONE);

process (CLK) 
begin
	if clk'event and clk='1' then
		if (data_we='1') then
			init_vector <= data_out;
			enable <= '1';
		end if;
	end if;
end process;

matrix <= getmatrix(solution);


conditional_gen: if enable = '1' generate
	rows : for i in 0 to 4 generate
		cols : for j in 0 to 4 generate
		
		
		
			cell : entity work.cell  	GENERIC MAP (MASK => getmask(i, j, 5, 5))
										PORT MAP (NEIGHBOUR => getneighbours(i,j, matrix), CLICK => matrix(i, j), STATE => getmystate(i, j, init_vector), LIGHT => LIGHT(i,j));
		end generate ; -- cols
	
	end generate ; -- rows
end generate;
end main;

