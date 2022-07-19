.PHONY: all clean
CXX=gcc
CPP=g++
FLAGS=-Wall -g

all:test_myfs test_mystdio
test_myfs: test_myfs.c libmyfs.so
	$(CXX) $(FLAGS) -o test_myfs test_myfs.c ./libmyfs.so
libmyfs.so: myfs.c myfs.h
	$(CXX) --shared -fPIC -o libmyfs.so myfs.c myfs.h
test_mystdio: test_mystdio.c libmylibc.so libmyfs.so
	$(CXX) $(FLAGS) -o test_mystdio test_mystdio.c ./libmylibc.so ./libmyfs.so -lm
libmylibc.so: mystdio.c mystdio.h
	$(CXX) --shared -fPIC -o libmylibc.so mystdio.c mystdio.h
clean:
	rm -f *.o test_myfs lib.so test_mystdio libmylibc
