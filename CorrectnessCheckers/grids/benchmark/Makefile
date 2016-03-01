INCLUDEDIR=../../../include

all: compile

compile:
	g++ -std=c++11 -o checker.out correctnessChecker.cpp -O2 -fno-strict-aliasing  -I$(INCLUDEDIR) -DNDEBUG -lboost_program_options

debug:
	g++ -std=c++11 -o checker.out correctnessChecker.cpp -O0 -g -I$(INCLUDEDIR) -Wall -lboost_program_options

run:
	./checker.out -b 2 -a 2 -g 1

clean: 
	rm *.out 
