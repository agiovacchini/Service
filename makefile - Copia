CXX		=	g++
CXXFLAGS	=	-g -std=c++11 -Wall -L/usr/local/lib -I/usr/local/include 
NLIB		=	-lboost_system -lboost_filesystem -lboost_iostreams -lboost_thread -lboost_iostreams

BIN=promoCalculator

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -o $(BIN) $(NLIB) $^

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)

