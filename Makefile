.PHONY: main parser clean

CXXFLAGS=--std=c++11 -Wall -Wextra

main: parser.o main.cpp
	g++ $(CXXFLAGS) -o $@ $^ 

parser.o: parser.cpp
	g++ $(CXXFLAGS) -c $<

clean:
	rm -rf main *.o
