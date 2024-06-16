.PHONY: main parser clean

CXXFLAGS=--std=c++11 -Wall -Wextra -g

main: parser.o fault.o main.cpp
	g++ $(CXXFLAGS) -o $@ $^ 

parser.o: parser.cpp
	g++ $(CXXFLAGS) -c $<

fault.o: fault.cpp
	g++ $(CXXFLAGS) -c $<

clean:
	rm -rf main *.o
