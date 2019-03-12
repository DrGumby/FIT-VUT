#!/usr/bin/env bash

START=$(date +%s)

file='proj3.c'
binary='proj3'

# Output settings
TEXT_BOLD=`tput bold`
TEXT_RED=`tput setaf 1`
TEXT_GREEN=`tput setaf 2`
TEXT_BROWN=`tput setaf 3`
TEXT_BLUE=`tput setaf 4`
TEXT_RESET=`tput sgr0`

diff=$(which diff)


help() {
printf \\n%s\\n "Spusteni testu:
	$0 -h		Vypise tento help
	$0 -c		Uklid souboru vytvorenych v prubehu testovani
				maze soubory *-my a *-error	
Jak to funguje:
	Pro kazde JMENO testu (pouzivaji se cisla) vytvori nazvy souboru:
		testJMENO-out
		testJMENO-my
		testJMENO-error

	testJMENO-out existuje => skript ma vracet 0 a nove vytvoreny soubor
		testJMENO-my ma byt identicky
	testJMENO-out neexistuje => skript ma vracet nenulovou hodnotu a 
		zapsat neco na stderr, ktere se uklada do testJMENO-error"
}


# IO file names
errorFileName() {
	printf "test${1}-error"
}
inFileName() {
	printf "test${1}-in"
}
outFileName() {
	printf "test${1}-out"
}
myFileName() {
	printf "test${1}-my"
}

# Test initialization
initTest() {
	testNumber=$1
	testError=$(errorFileName $testNumber)
	testOut=$(outFileName $testNumber)
	testMy=$(myFileName $testNumber)
}

green() {
	printf %s "${TEXT_GREEN}${1}${TEXT_RESET}"
}

red() {
	printf %s "${TEXT_RED}${1}${TEXT_RESET}"
}

isOk() {
	testNumber=$1
	testExit=$2
	testError=$3

	printf \\t"..." "$1"

	if [ -e ${testOut} ] 
	then
		`$diff -q ${testOut} ${testMy} > /dev/null`
		diffResult=$?
		printf %s "isOK   ExitCode: "
		[ $testExit -eq 0 ] && green $testExit || red $testExit
		printf %s\\n ",       output: $([ $diffResult -eq 0 ] && green 'not diff' || red 'diff')"
		err=1
		[ $testExit -eq 0 ] && [ $diffResult -eq 0 ] && err=0
	else
		printf %s "isFail ExitCode: " 
		[ $testExit -eq 0 ] && red $testExit || green $testExit
		printf %s\\n ", error output: $([ -s $testError ] && green "found" || red "not found")"
		err=1
		[ $testExit -ne 0 ] && [ -s $testError ] && err=0 # True, if <FILE> exists and has size bigger than 0 (not empty).
		[ $err -eq 0 ] && printf "$TEXT_BROWN" && /bin/cat $testError
	fi

	[ $err -eq 0 ] && green "ok" || red "fail"
}


function testMem () {
    local testNumber=$1
    local testResult=$2

    printf \\t\\n"Valgrind... "
        
    cat tmp_error | grep "=="
        
    if [ $testResult == 0 ]
    then
        red "fail"
    else
        green "ok"
    fi
    echo
}


# Vstup: jmeno_testu parametry_pro_test
otestuj() {
    initTest ${1}
    printf %s\\n\\n "Test ${1} ${TEXT_BLUE}$info${TEXT_RESET}"

    `./${binary} $2 $3 > ${testMy} 2> ${testError}`
    isOk $testNumber $? $testError
    
    `valgrind -q --leak-check=full --show-leak-kinds=all ./${binary} $2 $3 >/dev/null 2>tmp_error`
    grep -q "==" "tmp_error"
    testMem $testNumber $?
    
    
}


file_exists() {
	if [ -e "$1" ]
	then
		printf "${TEXT_BOLD}Testing file ${1}${TEXT_RESET}\n"
	else
		printf "${TEXT_RED}Cannot run test without file ${1}.${TEXT_RESET}\n"
		exit 1
	fi
}


`valgrind --version > /dev/null 2>&1`
valgrindResult=$?
if [ ! $valgrindResult == 0 ]
then
    printf "${TEXT_RED}Valgrind not installed. Cannot check memmory leaks.${TEXT_RESET}\n"
    exit 1
fi


if [ "$1" = "-h" ] ; then	# Help
	help
	exit 0
elif [ "$1" = "-c" ] ; then	# Cleaning
	rm *-my *-error
	exit 0
fi

if [ $? -ne 0 ]
then
	printf "${TEXT_RED}Spatny parametr ${1}.${TEXT_RESET}\n"
	help
	exit 1
fi

file_exists "$file"

# Compile source
printf "Compile source code...\n"
`gcc -std=c99 -Wall -Wextra -pedantic -g -DNDEBUG ${file} -o ${binary} -lm`


printf %s\\n "Zakladni testy funkcnosti"

#  ------====== Zacatek testu =======------

info="Zadano bez parametru"
otestuj 1

info="Zadano spatne jmeno souboru"
otestuj 2 "neexistuje.txt"

info="Zadan zaporny pocet clusteru"
otestuj 3 "muni.txt -2"

info="Zadan nulovy pocet clusteru"
otestuj 4 "muni.txt 0"

info="Zadan stejny pocet vystupu jako ma vstup"
otestuj 5 "muni.txt 22"

info="Zadano o 1 mene na vystupu nez na vstupu"
otestuj 6 "muni.txt 21"

info="Zadano o 2 mene na vystupu nez na vstupu"
otestuj 7 "muni.txt 20"

info="Zadano o 3 mene na vystupu nez na vstupu"
otestuj 8 "muni.txt 19"

info="Zadano o 4 mene na vystupu nez na vstupu"
otestuj 9 "muni.txt 18"

info="Zadano o 5 mene na vystupu nez na vstupu"
otestuj 10 "muni.txt 17"

info="Zadano o 6 mene na vystupu nez na vstupu"
otestuj 11 "muni.txt 16"

info="Zadano o 7 mene na vystupu nez na vstupu"
otestuj 12 "muni.txt 15"

info="Zadano o 8 mene na vystupu nez na vstupu"
otestuj 13 "muni.txt 14"

info="Zadano o 9 mene na vystupu nez na vstupu"
otestuj 14 "muni.txt 13"

info="Zadano o 10 mene na vystupu nez na vstupu"
otestuj 15 "muni.txt 12"

info="Zadano o 11 mene na vystupu nez na vstupu"
otestuj 16 "muni.txt 11"

info="Zadano o 12 mene na vystupu nez na vstupu"
otestuj 17 "muni.txt 10"

info="Zadano o 13 mene na vystupu nez na vstupu"
otestuj 18 "muni.txt 9"

info="Zadano o 14 mene na vystupu nez na vstupu"
otestuj 19 'muni.txt 8'

info="Zadano o 15 mene na vystupu nez na vstupu"
otestuj 20 "muni.txt 7"

info="Zadano o 16 mene na vystupu nez na vstupu"
otestuj 21 "muni.txt 6"

info="Zadano o 17 mene na vystupu nez na vstupu"
otestuj 22 "muni.txt 5"

info="Zadano o 18 mene na vystupu nez na vstupu"
otestuj 23 "muni.txt 4"

info="Zadano o 19 mene na vystupu nez na vstupu"
otestuj 24 "muni.txt 3"

info="Zadano o 20 mene na vystupu nez na vstupu"
otestuj 25 "muni.txt 2"

info="Zadano o 21 mene na vystupu nez na vstupu"
otestuj 26 "muni.txt 1"

info="Nezadan pocet vystupnich clusteru, implicitne to ma byt 1"
otestuj 27 "muni.txt"

info="Zadan vetsi vystup nez ma vstup"
otestuj 28 "muni.txt 23"

info="Zadano count=100 a pritom je v souboru malo objektu"
otestuj 29 "test29-in"

info="Zadano misto ID slovo"
otestuj 30 "test30-in"

info="Zadano misto X slovo"
otestuj 31 "test31-in"

info="Zadano misto Y slovo"
otestuj 32 "test32-in"

info="Zadano zaporne X"
otestuj 33 "test33-in"

info="Zadano zaporne Y"
otestuj 34 "test34-in"

info="Zadano X = Y = 1000"
otestuj 35 "test35-in"

info="Zadano to same co predtim, ale prazdne radky na konci"
otestuj 36 "test36-in"

info="Zadano prekroceno X = 1001"
otestuj 37 "test37-in"

info="Zadano prekroceno Y = 1001"
otestuj 38 "test38-in"

info="Zadano necelociselne ID"
otestuj 39 "test39-in"

info="Zadano necelociselne X"
otestuj 40 "test40-in"

info="Zadano necelociselne Y"
otestuj 41 "test41-in"

info="Zadano duplicitni Id na radku 7"
otestuj 42 "test42-in"

info="Zadano prilis vysoke ID"
otestuj 43 "test43-in"

info="Zadano prilis vysoke ID, ale tak aby pri preteceni bylo rovno 2"
otestuj 44 "test44-in"

info="Slepen prvni a druhy radek"
otestuj 45 "test45-in"

info="Mezery a tabulatory pred koncem radku"
otestuj 46 "test46-in"

info="Zadano count = 0"
otestuj 47 "test47-in"

info="Zadano count = -1"
otestuj 48 "test48-in"

info="Zadano cunts misto count"
otestuj 49 "test49-in"

info="Zadano count=10.33"
otestuj 50 "test50-in"

info="Zadano v parametrech muni.txt 10.33"
otestuj 51 "muni.txt 10.33"

info="Chybi jedna polozka na radku"
otestuj 52 "test51-in"

END=$(date +%s)
printf "Test trval vterin: $(( $END - $START ))\n"
