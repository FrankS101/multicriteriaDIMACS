#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <Structs/Graphs/dynamicGraph.h>
#include <Structs/Graphs/adjacencyListImpl.h>
#include <Heuristics/blind.h>
#include <Heuristics/ideal.h>
#include <Heuristics/boundedIdeal.h>
#include <Algorithms/multicriteriaDijkstra.h>
#include <Algorithms/multicriteriaGraph.h>
#include <Algorithms/namoaStar.h>
#include <Algorithms/namoaStar2.h>
#include <Algorithms/multicriteriaArc.h>
#include <Checkers/multiCriteriaChecker.h>
#include <Checkers/multiCriteriaResults.h>
#include <Checkers/Grids/multiCriteriaGridChecker.h>
#include <Utilities/timer.h>
#include <Utilities/colormod.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

template< class algorithmVariant, typename GraphType>
void runQuery( GraphType& G, std::vector< std::pair<unsigned int,unsigned int> >& queries, const unsigned int query_n,
                 std::vector<typename GraphType::NodeDescriptor>& ids, GridChecker& gChecker)
{
    typedef typename GraphType::NodeIterator NodeIterator;
    NodeIterator s,t;
    unsigned int sourceId, targetId;
    unsigned int timestamp = 0;
    //clear nodes
    NodeIterator u, lastnode;
    for( u = G.beginNodes(), lastnode = G.endNodes(); u != lastnode; ++u)
    {
        u->g_op.clear();
        u->g_cl.clear();
    }
    //create algorithm
    algorithmVariant algorithm( G, NUM_CRITERIA, &timestamp);
    // double numLabels = 0;
    std::vector< std::pair<unsigned int,unsigned int> >::iterator it = queries.begin() + query_n;
    sourceId = it->first;
    targetId = it->second;
    s = G.getNodeIterator( ids[sourceId]);
    t = G.getNodeIterator( ids[targetId]);
    algorithm.init( s, t, NUM_CRITERIA);
    algorithm.runQuery( s, t);
    // add Pareto efficient solutions to the results class
    MulticriteriaResults mResults( queries);
    mResults.addResults( t->g_cl);
    std::cout << std::endl;
    t->printLabels( std::cout, G);
    // Two things are checked: First, the set of solutions is the same; second, the solutions have been found in the same order.
    bool order = true;
    bool correctness = mResults.checkParetoCosts( gChecker, query_n, order);
    if ( !( correctness))
    {
        exit (EXIT_FAILURE);
    }
}

typedef DynamicGraph< AdjacencyListImpl, Node, Edge>       Graph;
typedef DynamicGraph< AdjacencyListImpl>                   graph;
typedef Graph::NodeIterator                                NodeIterator;
typedef Graph::EdgeIterator                                EdgeIterator;
typedef Graph::NodeDescriptor                              NodeDescriptor;

int main( int argc, char* argv[])
{
    std::string gridBenchmarksPath = "/home/francis/Projects/Benchmarks/";
    std::string gridsPath = gridBenchmarksPath + "grids/";
    std::string gridQueriesPath = gridBenchmarksPath + "queries/";
    std::string gridSolutionsPath = gridBenchmarksPath + "solutions/";

    unsigned int criteriaVariant = 2;
    unsigned int gridVariant = 0;
    unsigned int queryVariant = 0;
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("criteria,c", po::value< unsigned int>(), "Number of criteria. Two[2], Three[3]. Default:2")
            ("grid,g", po::value< unsigned int>(), "Grid number. Default:0")
            ("query,q", po::value< unsigned int>(), "Query number. Default:0");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.empty()) {
        std::cout << desc << "\n";
        return 0;
    }
    if (vm.count("criteria"))
    {
        criteriaVariant = vm["criteria"].as<unsigned int>();
    }
    if (vm.count("grid"))
    {
        gridVariant = vm["grid"].as<unsigned int>();
    }
    if (vm.count("query"))
    {
        queryVariant = vm["query"].as<unsigned int>();
    }
    unsigned int nqueries;
    if ( criteriaVariant == 2)
    {
        NUM_CRITERIA = 2;
        nqueries = NQUERIES_2;
    }
    else if ( criteriaVariant == 3)
    {
        NUM_CRITERIA = 3;
        nqueries = NQUERIES_3;
    }
    else
    {
        std::cerr << "The benchmark with the number of criteria provided has not been implemented yet\n";
    }
    GridChecker gChecker( nqueries, criteriaVariant,
                          gridQueriesPath + "queries" + std::to_string( criteriaVariant) + ".txt",
                          gridSolutionsPath + "p" + std::to_string( gridVariant) + "/" +
                          std::to_string( criteriaVariant) + ".txt",
                          GRIDDIMSIZE);
    std::vector< std::pair< unsigned int ,unsigned int> > queries;
    queries = gChecker.getQueries ( criteriaVariant);
    Graph G;
    std::string gridProblemPath = gridsPath + "Grid" + std::to_string( gridVariant)+ ".txt";
    std::cout << "\nBenchmark at " << gridProblemPath;
    GridReader<Graph> reader( gridProblemPath, criteriaVariant);
    G.read(&reader);
    filetrace = "trace.dat";
    o_debug.open(filetrace.c_str(), std::fstream::trunc);
    //o_debug = std::cout;
    runQuery<NamoaStar2<Graph, TCHeuristic> >( G, queries, queryVariant, reader.getIds(), gChecker);
    o_debug.close();
    return 0;
}
