#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <Structs/Graphs/dynamicGraph.h>
#include <Structs/Graphs/adjacencyListImpl.h>
#include <Algorithms/multicriteriaDijkstra.h>
#include <Algorithms/multicriteriaGraph.h>
#include <Algorithms/namoaStar.h>
#include <Algorithms/namoaStar2.h>
#include <Algorithms/multicriteriaArc.h>
#include <Heuristics/blind.h>
#include <Heuristics/ideal.h>
#include <Heuristics/boundedIdeal.h>
#include <Checkers/multiCriteriaChecker.h>
#include <Checkers/multiCriteriaResults.h>
#include <Checkers/Grids/multiCriteriaGridChecker.h>
#include <Utilities/timer.h>
#include <Utilities/colormod.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

template< class algorithmVariant, typename GraphType>
void runQueries( GraphType& G, std::vector< std::pair<unsigned int,unsigned int> >& queries,
                 std::vector<typename GraphType::NodeDescriptor>& ids, GridChecker& gChecker, const unsigned int showOnScreen)
{
    typedef typename GraphType::NodeIterator NodeIterator;
    NodeIterator s,t;
    unsigned int sourceId, targetId;
    unsigned int timestamp = 0;
    //create algorithm
    algorithmVariant algorithm( G, NUM_CRITERIA, &timestamp);
    //run queries
    unsigned int query_n = 0;
    double numLabels = 0;
    double totalTime = 0;
    for( std::vector< std::pair<unsigned int,unsigned int> >::iterator it = queries.begin();
         it != queries.end(); ++it)
    {
        //clear nodes
        NodeIterator u, lastnode;
        for( u = G.beginNodes(), lastnode = G.endNodes(); u != lastnode; ++u)
        {
            u->g_op.clear();
            u->g_cl.clear();
        }
        sourceId = it->first;
        targetId = it->second;
        s = G.getNodeIterator( ids[sourceId]);
        t = G.getNodeIterator( ids[targetId]);
        Timer timer;
        timer.start();
        algorithm.init( s, t, NUM_CRITERIA);
        auto heuristicTime = 1000 * timer.getElapsedTime();
        timer.start();
        algorithm.runQuery( s, t);
        auto runtime = 1000 * timer.getElapsedTime();
        totalTime += runtime;
        // add Pareto efficient solutions to the results class
        MulticriteriaResults mResults( queries);
        mResults.addResults( t->g_cl);
        numLabels += algorithm.getGeneratedLabels();
        // Two things are checked: First, the set of solutions is the same; second, the solutions have been found in the same order.
        bool order = true;
        bool correctness = mResults.checkParetoCosts( gChecker, query_n, order);
        Color::Modifier def(Color::FG_DEFAULT);
        Color::Modifier red(Color::FG_RED);
        Color::Modifier green(Color::FG_GREEN);
        Color::Modifier yellow(Color::FG_YELLOW);
        Color::Modifier b_blue(Color::BG_BLUE);
        Color::Modifier b_def(Color::BG_DEFAULT);

        std::cout << b_blue << "(" << sourceId << "->" << targetId << ")" << b_def << "\t\t";

        if ( showOnScreen)
        {
            Color::Modifier blue(Color::FG_BLUE);
            Color::Modifier lblue(Color::FG_LIGHT_BLUE);
            Color::Modifier cyan(Color::FG_CYAN);
            Color::Modifier lcyan(Color::FG_LIGHT_CYAN);
            std::cout << blue << t->g_cl.size() << def << " efficient paths were found.\n";
            std::cout << "\t\t\t" << lblue << algorithm.getGeneratedLabels() << def << " labels were scanned.\n";
            std::cout << "\t\t\t" << cyan << heuristicTime << def << " msec. - calculation of heuristic.\n";
            std::cout << "\t\t\t" << lcyan << runtime << def << " msec. - algorithm runtime.\n";
        }
        std::cout << "Solutions ... ->\t";
        if ( !( correctness))
        {
            std::cout << red << "Different!!!\n" << def;
            t->printLabels( std::cout, G);
            exit (EXIT_FAILURE);
        }
        else
        {
            std::cout << green << "OK" << def << "\n";
            if (! (order)) std::cout << yellow << "Warning!" << def << " The same set of solutions was found, but in different order.\n";
        }
        std::cout << "---------------------------------------------------------------------\n";
        ++query_n;
    }

    if ( showOnScreen)
    {
        std::cout << "\tAlgorithm Runtime:\t" << totalTime << " msec. ( " << (totalTime / queries.size()) << " msec. per query)\n";
        std::cout << "\tGenerated labels:\t " << numLabels << std::endl;
    }
}

template< typename GraphType>
void runBenchmarks( GraphType& G, std::vector< std::pair<unsigned int,unsigned int> >& queries,
                    std::vector<typename GraphType::NodeDescriptor>& ids, GridChecker& gChecker,
                    const std::string& name, const unsigned int& algorithmVariant, const unsigned int showOnScreen)
{
    switch( algorithmVariant)
    {
    case 1:
        // output message for all queries
        std::cout << "with NAMOA* (blind) ...\n\n";
        runQueries<NamoaStar2<GraphType,BlindHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        break;
    case 2:
        std::cout << "with NAMOA*_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,TCHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        break;
    case 3:
        std::cout << "with NAMOA*_bound_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,BoundedTCHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        //std::cout << "with NAMOA* Arc Flags (" << name << ") ...\n\n";
        //runQueries<NamoaStarArc<GraphType,GreatCircleHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        break;
    case 0: // default
        std::cout << "with NAMOA* (blind) ...\n\n";
        runQueries<NamoaStar2<GraphType,BlindHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        std::cout << "with NAMOA*_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,TCHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        std::cout << "with NAMOA*_bound_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,BoundedTCHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        break;
    }
}

typedef DynamicGraph< AdjacencyListImpl, Node, Edge>       Graph;
//typedef DynamicGraph< AdjacencyListImpl, LWNode, Edge>     LWGraph;
typedef Graph::NodeIterator                                NodeIterator;
typedef Graph::EdgeIterator                                EdgeIterator;
typedef Graph::NodeDescriptor                              NodeDescriptor;

int main( int argc, char* argv[])
{
    std::string gridBenchmarksPath = "/home/francis/Projects/Benchmarks/";
    std::string gridsPath = gridBenchmarksPath + "grids/";
    std::string gridQueriesPath = gridBenchmarksPath + "queries/";
    std::string gridSolutionsPath = gridBenchmarksPath + "solutions/";

    unsigned int benchmarkVariant = 2;
    unsigned int algorithmVariant = 0;
    unsigned int showOnScreen = 1;

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("benchmark,b", po::value< unsigned int>(), "Benchmark to run. Bicriteria[2],  Tri-criterion[3]. Default:2")
        ("algorithm,a", po::value< unsigned int>(), "Multicriteria Heuristic. All[0], Blind[1], Ideal Point[2], Bounded Ideal point[3]. Default:0")
        ("showOnScreen,s", po::value< unsigned int>(), "Display stats on screen. Yes[1], No[0]. Default:1");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.empty()) {
        std::cout << desc << "\n";
        return 0;
    }
    if (vm.count("benchmark"))
    {
        benchmarkVariant = vm["benchmark"].as<unsigned int>();
    }
    if (vm.count("algorithm"))
    {
        algorithmVariant = vm["algorithm"].as<unsigned int>();
    }
    if (vm.count("showOnScreen"))
    {
        showOnScreen = vm["showOnScreen"].as<unsigned int>();
    }
    unsigned int nqueries;
    if ( benchmarkVariant == 2)
    {
        NUM_CRITERIA = 2;
        nqueries = NQUERIES_2;
    }
    else if ( benchmarkVariant == 3)
    {
        NUM_CRITERIA = 3;
        nqueries = NQUERIES_3;
    }
    else
    {
        std::cerr << "BenchmarkVariant provided not implemented yet\n";
    }

    for ( unsigned int grid_n = 0; grid_n < NGRIDS; ++grid_n)
    {
        GridChecker gChecker( nqueries, benchmarkVariant,
                              gridQueriesPath + "queries" + std::to_string( benchmarkVariant) + ".txt",
                              gridSolutionsPath + "p" + std::to_string( grid_n) + "/" +
                              std::to_string( benchmarkVariant) + ".txt",
                              GRIDDIMSIZE);
        std::vector< std::pair< unsigned int ,unsigned int> > queries;
        queries = gChecker.getQueries (benchmarkVariant);
        Graph G;
        std::string gridProblemPath = gridsPath + "Grid" + std::to_string( grid_n)+ ".txt";
        std::cout << "\nBenchmark at " << gridProblemPath;
        GridReader<Graph> reader( gridProblemPath, benchmarkVariant);
        Timer timer;
        timer.start();
        G.read(&reader);
        std::cout << "Graph has " << (double)G.memUsage()/1048576 << " Mbytes. Time spent to read:\t" << timer.getElapsedTime() << "sec" << std::endl;
        std::cout << "Checking correctness of " << queries.size() << " queries \n";
        timer.start();
        runBenchmarks( G, queries, reader.getIds(), gChecker, "ADJ", algorithmVariant, showOnScreen);
        std::cout << "\tBenchmark runtime: \t" << timer.getElapsedTime() << " sec" << std::endl;
    }
    return 0;
}
