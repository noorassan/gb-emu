makefile:

INC = -Iinclude/
BUILD_DIRS = src/*.cc src/memory_bank_controllers/*.cc

.DEFAULT_GOAL :=
all: main.o

main.o:
	g++ -Wall -std=c++17 -g ${INC} ${BUILD_DIRS}

clean:
	rm -f a.out
	rm -f log.txt
	rm -rf a.out.dSYM
