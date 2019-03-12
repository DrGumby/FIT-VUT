CC=gcc
CFLAGS= -O2 -std=c99 -Wall -Wextra -pedantic
CXX=g++
CPPFLAGS= -std=c++11 -pedantic -Wall
LIB_FILES=htab_hash_function.o htab_init.o htab_move.o htab_size.o htab_bucket_count.o htab_lookup_add.o htab_find.o htab_foreach.o htab_remove.o htab_clear.o htab_free.o

all: tail wordcount_link wordcount_dynamic_link

tail:
	$(CC) $(CFLAGS) tail.c -o tail
	$(CXX) $(CPPFLAGS) tail2.cc -o tail2

wordcount_link: wordcount_static_lib
	$(CC) wordcount.o io.o ./libhtab.a -o wordcount -lc
wordcount_dynamic_link: wordcount_dynamic_lib
	$(CC) wordcount.o io.o -L. -Wl,-rpath=. -lhtab -o wordcount-dynamic


wordcount_static_lib: wordcount_object
	ar rcs libhtab.a $(LIB_FILES)

wordcount_dynamic_lib: wordcount_object
	$(CC) -shared $(LIB_FILES) -o libhtab.so

wordcount_object:
	$(CC) $(CFLAGS) -fPIC htab_hash_function.c -o htab_hash_function.o -c
	$(CC) $(CFLAGS) -fPIC htab_init.c -o htab_init.o -c
	$(CC) $(CFLAGS) -fPIC htab_move.c -o htab_move.o -c
	$(CC) $(CFLAGS) -fPIC htab_size.c -o htab_size.o -c
	$(CC) $(CFLAGS) -fPIC htab_bucket_count.c -o htab_bucket_count.o -c
	$(CC) $(CFLAGS) -fPIC htab_lookup_add.c -o htab_lookup_add.o -c
	$(CC) $(CFLAGS) -fPIC htab_find.c -o htab_find.o -c
	$(CC) $(CFLAGS) -fPIC htab_foreach.c -o htab_foreach.o -c
	$(CC) $(CFLAGS) -fPIC htab_remove.c -o htab_remove.o -c
	$(CC) $(CFLAGS) -fPIC htab_clear.c -o htab_clear.o -c
	$(CC) $(CFLAGS) -fPIC htab_free.c -o htab_free.o -c
	$(CC) $(CFLAGS) wordcount.c -o wordcount.o -c
	$(CC) $(CFLAGS) io.c -o io.o -c
clean:
	rm -f *.o *.a *.so tail tail2 wordcount wordcount_dynamic
