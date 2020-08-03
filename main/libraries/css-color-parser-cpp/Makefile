CXXFLAGS = -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter

build: test

test: csscolorparser.o test.o
	$(CXX) $(CXXFLAGS) -o $@ $^

fuzz: csscolorparser.o fuzz.o
	afl-clang-fast++ $(CXXFLAGS) -o $@ $^

fuzz.o: fuzz.cpp
	afl-clang-fast++ $(CXXFLAGS) -c -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -rf *.o test fuzz

.PHONY: clean
