INCLUDEDIR=../../include

all: compile

compile:
	g++ -std=c++11 multiCriteriaDimacs9.cpp -O2 -fno-strict-aliasing -I$(INCLUDEDIR) -DNDEBUG -lboost_program_options


debug:
	g++ -std=c++11 multiCriteriaDimacs9.cpp -O0 -g  -I$(INCLUDEDIR) -Wall -lboost_program_options

test:
	g++ -std=c++11 multiCriteriaFirst.cpp -O0 -g  -I$(INCLUDEDIR)-Wall -lboost_program_options

run:
	./a.out

profile: 
	./a.out
	gprof -ab > profile

clean: 
	rm *.out
