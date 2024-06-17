.PHONY: main parser clean

CXXFLAGS=--std=c++11 -Wall -Wextra -g

main: parser.o main.cpp
	g++ $(CXXFLAGS) -o $@ $^ 

parser.o: parser.cpp
	g++ $(CXXFLAGS) -c $<

interference: interference.o
	g++ $(CXXFLAGS) -o interference.o interference.cpp parser.cpp interference_test.cpp

clean:
	rm -rf main *.o
