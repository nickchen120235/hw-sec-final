.PHONY: main parser clean

CXXFLAGS=--std=c++11 -Wall -Wextra

main: parser.o main.cpp
	g++ -o $@ $^

parser.o: parser.cpp
	g++ -c $<

clean:
	rm -rf main *.o
