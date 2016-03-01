#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <iostream>

typedef unsigned int PriorityQueueSizeType;

//-------------------------------- PRIORITY QUEUE STATISTICS --------------------------------

//queue statistics (can be set by compilerflag -DQUEUESTATS)
#ifdef QUEUESTATS
	#define _QUEUESTATS(x) x
#else
	#define _QUEUESTATS(x)
#endif


#ifdef MEMSTATS
	#define _MEMSTATS(x) x
#else
	#define _MEMSTATS(x)
#endif


#ifdef SHOWPROGRESS
    #define _SHOWPROGRESS(x) x
#else
	#define _SHOWPROGRESS(x)
#endif

//-------------------------------- CONFIGURATION TO PRINT DEBUG INFO ------------------------

#define DEBUG 0

static std::string filetrace;
static std::ofstream o_debug;
//static std::ostream& o_debug = std::cout;

//-------------------------------- CONFIGURATION FOR BENCHMARK PROBLEMS ---------------------

//const static std::string gridBenchmarksPath = std::string(getenv("HOME")) + "/Projects/Benchmarks/";
//const static std::string gridsPath = gridBenchmarksPath + "grids/";
//const static std::string gridQueriesPath = gridBenchmarksPath + "queries/";
//const static std::string gridSolutionsPath = gridBenchmarksPath + "solutions/";
const static unsigned int NQUERIES_2 = 15;
const static unsigned int NQUERIES_3 = 9;
const static unsigned int NGRIDS = 4;

#endif //CONFIGURATION_H
