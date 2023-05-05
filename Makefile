
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall -g

all: correctness persistence my_test

correctness: kvstore.o correctness.o SSTable.o

persistence: kvstore.o persistence.o SSTable.o

my_test: kvstore.o my_test.o SSTable.o

clean:
	-rm -f correctness persistence my_test SSTable *.o
