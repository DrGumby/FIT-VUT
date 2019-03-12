#!/bin/bash

echo "Testing make"

make >makelog

if [[ -f tail && -f tail2 && -f wordcount && -f wordcount-dynamic && -f libhtab.a && -f libhtab.so ]]
then
    echo "Make OK"
    rm -f makelog
else
    echo "Make nevytvořil všechny potřebné soubory nebo jsou špatně pojmenovány. Viz vytvořený soubor makelog"
fi

g++ -std=c++11 -O2 wordcount.cc -o wordcountcpp

./wordcountcpp <testfile | sort  >wordcount_cpp_output

./wordcount-dynamic <testfile | sort >wordcount-dynamic_c_output

./wordcount <testfile | sort >wordcount_c_output

echo "Testuji shodnost výstupů (diff)"

diff -s wordcount_cpp_output wordcount-dynamic_c_output

diff -s wordcount_cpp_output wordcount_c_output

diff -s wordcount_c_output wordcount-dynamic_c_output

echo "Testuji Valgrind"

valgrind --error-exitcode=1 ./wordcount <testfile >valgrind_out 2>valgrind_out
if [[ $? == 0 ]]; then
    echo "wordcount: Valgrind wordcount OK"
elif [[ $(grep "no leaks are possible" valgrind_out) ]]; then
    echo "wordcount: Žádný leak, ale jsou tam warningy, viz soubor valgrind_out"
else
    echo "wordcount: Chyba, memory leak, viz soubor valgrind_out"
fi

valgrind --error-exitcode=1 ./wordcount-dynamic <testfile >valgrind_dynamic_out 2>valgrind_dynamic_out
if [[ $? == 0 ]]; then
    echo "wordcount-dynamic: Valgrind wordcount OK"
elif [[ $(grep "no leaks are possible" valgrind_dynamic_out) ]]; then
    echo "wordcount-dynamic: Žádný leak, ale jsou tam warningy, viz soubor valgrind_dynamic_out"
else
    echo "wordcount-dynamic: Chyba, memory leak, viz soubor valgrind_dynamic_out"
fi

echo "Testuji příliš dlouhé řádky"

head /dev/urandom | tr -dc A-Za-z0-9 | head -c130 >testfile2
cat testfile >> testfile2





./wordcount <testfile2 >wordcount_c_output 2>wordcount_c_stderr
./wordcount-dynamic <testfile2 >wordcount-dynamic_c_output 2>wordcount-dynamic_c_stderr

grep "`head -n1 testfile2 | head -c126`" wordcount_c_output >/dev/null
if [[ $? == 0 && ! -z wordcount_c_stderr ]]; then 
	echo "Wordcount : OK"
fi

grep "`head -n1 testfile2 | head -c126`" wordcount-dynamic_c_output >/dev/null
if [[ $? == 0 && ! -z wordcount-dynamic_c_stderr ]]; then 
	echo "Wordcount-dynamic : OK"
fi

rm -f testfile2
rm -f wordcount_c_stderr
rm -f wordcount-dynamic_c_stderr
rm -f wordcount_c_output
rm -f wordcount-dynamic_c_output
rm -f wordcount_cpp_output
rm -f wordcountcpp
