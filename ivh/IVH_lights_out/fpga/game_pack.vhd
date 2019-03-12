--
--	game_pack.vhd
-- FIT VUT projekt IVH: Solver hry Lights out
-- Úkol 1. : Implementace funkce pro urèení masky
--
-- Kamil Vojanec (xvojan00)
--
--	Purpose: This package defines supplemental types, subtypes, 
--		 constants, and functions 


library IEEE;
use IEEE.STD_LOGIC_1164.all;

package game_pack is

type mask_t is
  record
	top: std_logic;
	left: std_logic;
	right: std_logic;
	bottom: std_logic;
end record;

type game_matrix is array (0 to 4, 0 to 4) of std_logic;
-- Declare functions and procedure
--
function getmask (X,Y,COLUMNS,ROWS : integer) return mask_t;
function getneighbours(X, Y : integer; matrix : game_matrix) return std_logic_vector;
function getmatrix(input_vector : std_logic_vector(24 downto 0)) return game_matrix;
function getmyclick(X, Y : integer; matrix : game_matrix) return std_logic;
function getmystate(X, Y : integer; input_vector : std_logic_vector) return std_logic;

end game_pack;

package body game_pack is
--Implementation of functions

function getmask (X,Y,COLUMNS,ROWS : integer) return mask_t is 
	variable mask : mask_t;
	begin
		if X <= 0 then
			mask.left := '0';
		else
			mask.left := '1';
		end if;
		
		if X >= COLUMNS-1 then
			mask.right := '0';
		else
			mask.right := '1';
		end if;
		
		if Y <= 0 then
			mask.top := '0';
		else
			mask.top := '1';
		end if;
		
		if Y >= ROWS-1 then
			mask.bottom := '0';
		else
			mask.bottom := '1';
		end if;
		
		return mask;
	end;

function getmatrix(input_vector : std_logic_vector(24 downto 0)) return game_matrix is
variable matrix : game_matrix;
variable k : integer range 0 to 24 := 0;
begin
rows : for i in 0 to 4 loop
	cols : for j in 0 to 4 loop
		matrix(i, j) := input_vector(k);
		k := k+1;
	end loop ; -- cols
end loop ; -- rows
return matrix;
end;

function getneighbours(X, Y : integer; matrix : game_matrix) return std_logic_vector is
constant IDX_TOP    : NATURAL := 0; -- index sousedni bunky nachazejici se nahore v signalu NEIGHBOUR
constant IDX_LEFT   : NATURAL := 1; -- index sousedni bunky nachazejici se vlevo  v signalu NEIGHBOUR
constant IDX_RIGHT  : NATURAL := 2; -- index sousedni bunky nachazejici se vpravo v signalu NEIGHBOUR
constant IDX_BOTTOM : NATURAL := 3; -- index sousedni bunky nachazejici se dole   v signalu NEIGHBOUR
variable neigbours : std_logic_vector(3 downto 0);
variable mask : mask_t := getmask(X, Y, 5, 5);


begin
	if mask.left = '1' then
		neigbours(IDX_LEFT) := getmyclick(X-1, Y, matrix);
	else 
		neigbours(IDX_LEFT) := '0';
	end if ;

	if mask.right = '1' then
		neigbours(IDX_RIGHT) := getmyclick(X+1, Y, matrix);
	else 
		neigbours(IDX_RIGHT) := '0';
	end if ;

	if mask.bottom = '1' then
		neigbours(IDX_BOTTOM) := getmyclick(X, Y+1, matrix);
	else 
		neigbours(IDX_BOTTOM) := '0';
	end if ;
	if mask.top = '1' then
		neigbours(IDX_TOP) := getmyclick(X, Y-1, matrix);
	else 
		neigbours(IDX_TOP) := '0';
	end if ;

	return neigbours;
end;


function getmyclick(X, Y : integer; matrix : game_matrix) return std_logic is
variable click : std_logic;
begin

click := matrix(x, y);
return click;

end;

function getmystate(X, Y : integer; input_vector : std_logic_vector) return std_logic is
variable state : std_logic;
begin
	state := input_vector((5*x) + y);
	return state;
end;
 
end game_pack;
