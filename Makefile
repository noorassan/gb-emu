makefile:

BUILD_DIRS = src/*.cc src/memory_bank_controllers/*.cc

all: main.o

main.o:
	g++ -Wall -std=c++17 -g ${BUILD_DIRS}

clean:
	rm a.out
	rm -rf a.out.dSYM
