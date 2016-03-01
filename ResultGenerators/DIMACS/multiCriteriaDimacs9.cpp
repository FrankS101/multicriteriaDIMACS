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
#include <Algorithms/namoaStar2.h>
#include <Algorithms/namoaStar.h>
#include <Algorithms/multicriteriaArc.h>
#include <Utilities/timer.h>
#include <boost/program_options.hpp>

#define NUM_CRITERIA 2

namespace po = boost::program_options;

class Results
{
public:
    Results( const std::vector< std::pair<unsigned int, unsigned int> >& queries):m_queries(queries)
    {
    }

    void add( const std::string& header, const std::vector< double >& results)
    {
        m_headers.push_back(header);
        m_data.push_back(results);
    }

    void print(std::ostream& out)
    {
        out << "source\tdestination";
        for( unsigned int i = 0; i < m_headers.size(); ++i)
        {
            out << "\t" << m_headers[i];
        }
        out << "\n";
        for( unsigned int i = 0; i < m_queries.size(); ++i)
        {
            out << m_queries[i].first << "\t" << m_queries[i].second;
            for( unsigned int j = 0; j < m_data.size(); ++j)
            {
                out << "\t" << m_data[j][i];
            }
            out << "\n";
        }
    }

private:
    std::vector< std::string> m_headers;
    std::vector< std::vector<double> > m_data;
    const std::vector< std::pair<unsigned int, unsigned int> >& m_queries;
};

template< class DijkstraVariant, typename GraphType>
void runQueries( GraphType& G, std::vector< std::pair<unsigned int,unsigned int> >& queries, std::vector<typename GraphType::NodeDescriptor>& ids, Results& results, const std::string& graphname, const std::string& algoname)
{
    std::vector<double> times;
    std::vector<double> paths;
    std::vector<double> generatedLabels;
    typedef typename GraphType::NodeIterator NodeIterator;
    typename GraphType::NodeIterator s,t;  
    unsigned int sourceId, targetId;
    unsigned int timestamp = 0;
    //create dijkstra
    DijkstraVariant dijkstra(G, NUM_CRITERIA, &timestamp);
    //create output message
    std::string message("Experiments at ");
    message.append( graphname + " " + algoname);
    ProgressBar show_progress( queries.size(), message);
    std::cout << message << std::endl;
    //run queries
    Timer timer; 
    unsigned int query_n = 1;   
    for( std::vector< std::pair<unsigned int,unsigned int> >::iterator it = queries.begin(); it != queries.end(); ++it)
    {
        //clear nodes
        NodeIterator u, lastnode;
        for( u = G.beginNodes(), lastnode = G.endNodes(); u != lastnode; ++u)
        {
            u->g_op.clear();
            u->g_cl.clear();
            u->labels.clear();
        }
        sourceId = it->first;
        targetId = it->second;
        std::cout << query_n << ") " << sourceId << "->" << targetId << std::endl;
        ++query_n;
        s = G.getNodeIterator( ids[sourceId]);
        t = G.getNodeIterator( ids[targetId]);
		std::cout << "Initializing...\n";
		timer.start();
        dijkstra.init( s, t, NUM_CRITERIA);
		std::cout << "Running query:\n";	
        dijkstra.runQuery( s, t);
        times.push_back(timer.getElapsedTime());
        paths.push_back(t->g_cl.size());
        //paths.push_back(t->labels.size());
        generatedLabels.push_back(dijkstra.getGeneratedLabels());
        std::cout << "\tTime:\t" << times[times.size()-1] << "sec\n";
        std::cout << "\tNon-dominated solutions:\t" << t->g_cl.size() << "\n";
		std::cout << "\tGenerated labels: " << dijkstra.getGeneratedLabels() << "\n\n";
    }
    results.add(graphname + " " + algoname,times);
    results.add(graphname + " " + algoname,generatedLabels);
    results.add(graphname + " " + algoname + " |C|",paths);
}

template< typename GraphType>
void runExperimentsAt( GraphType& G, std::vector< std::pair<unsigned int,unsigned int> >& queries, std::vector<typename GraphType::NodeDescriptor>& ids, Results& results, const std::string& name, const unsigned int& algorithmVariant)
{
    switch( algorithmVariant)
    {
    case 1:
        // output message for all queries
        std::cout << "with NAMOA* (blind) ...\n\n";
        runQueries<NamoaStar2<GraphType,BlindHeuristic> >( G, queries, ids, results, name, "NAMOA*2_Blind");
        break;
    case 2:
        std::cout << "with NAMOA*_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,TCHeuristic> >( G, queries, ids, results, name, "NAMOA*2_TC");
        break;
    case 3:
        std::cout << "with NAMOA*_bound_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,BoundedTCHeuristic> >( G, queries, ids, results, name, "NAMOA*2_BTC");
        //std::cout << "with NAMOA* Arc Flags (" << name << ") ...\n\n";
        //runQueries<NamoaStarArc<GraphType,GreatCircleHeuristic> >( G, queries, ids, gChecker, showOnScreen);
        break;
    case 0: // default
        std::cout << "with NAMOA* (blind) ...\n\n";
        runQueries<NamoaStar2<GraphType,BlindHeuristic> >( G, queries, ids, results, name, "NAMOA*2_Blind");
        std::cout << "with NAMOA*_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,TCHeuristic> >( G, queries, ids, results, name, "NAMOA*2_TC");
        std::cout << "with NAMOA*_bound_tc ...\n\n";
        runQueries<NamoaStar2<GraphType,BoundedTCHeuristic> >( G, queries, ids, results, name, "NAMOA*2_BTC");
        break;
    }
}

void readQueries( std::vector< std::pair<unsigned int,unsigned int> >& queries, const std::string& filename, const unsigned int& numQueries)
{
	std::string token;
	unsigned int uID,vID;
	std::ifstream in;
    in.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    std::cout << "Reading queries from " << filename << std::endl;
	unsigned int readQueries = 0;

	try {
    	in.open( filename.c_str());
        assert( in.good());
        while ( ((readQueries < numQueries) || numQueries == 0) && getline(in,token)) 
        {
			std::stringstream graphinfo;                  
			graphinfo.str(token);   
        	graphinfo >> uID >> vID;
			queries.push_back( std::pair<unsigned int, unsigned int>( uID, vID));
			++readQueries;
        }
        in.close();
    }
    catch (std::ifstream::failure e) {
        std::cerr << "Exception opening/reading file '" << filename << "'\n";
        throw e;
    }
}

typedef DynamicGraph< AdjacencyListImpl, Node, Edge>       Graph;
typedef Graph::NodeIterator                                NodeIterator;
typedef Graph::EdgeIterator                                EdgeIterator;
typedef Graph::NodeDescriptor                              NodeDescriptor;

int main( int argc, char* argv[])
{
    std::string basePath = std::string(getenv("HOME")) + "/Projects/Graphs/DIMACS9/";
    // Set default parameters
    unsigned int numQueries = 50;
    unsigned int algorithmVariant = 0;
    std::string map ="NY";
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("size,s", po::value< unsigned int>(), "number of queries. Default:50")
        ("algorithm,a", po::value< unsigned int>(), "NAMOA* algorithm. All[0], NAMOA* blind[1], NAMOA* TC[2], NAMOA* Bounded TC[3]. Default:0")
        ("map,m", po::value< std::string>(), "Input map. The name of the map to read. Maps must be in '$HOME/Projects/Graphs/DIMACS9/");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    
    if (vm.empty()) {
        std::cout << desc << "\n";
        return 0;
    }
    if (vm.count("size"))       numQueries = vm["size"].as<unsigned int>();
    if (vm.count("algorithm"))  algorithmVariant = vm["algorithm"].as<unsigned int>();
    if (vm.count("map"))        map = vm["map"].as<std::string>();
    Graph G;
    DIMACS9DoubleReader<Graph> reader( basePath + map + "/" + map + "_dist.gr",
                                basePath + map + "/" + map + "_travel.gr", basePath + map + "/" + map + ".co");
    filetrace = map + "_trace.dat";
    o_debug.open(filetrace.c_str(), std::fstream::app);
    Timer timer;
    // read the input graph and store it in memory
    timer.start();
    G.read(&reader);
    std::cout << "Graph has " << (double)G.memUsage()/1048576 << " Mbytes. Time spent to read:\t" << timer.getElapsedTime() << "sec" << std::endl;
	std::vector< std::pair<unsigned int,unsigned int> > queries;
    readQueries( queries, basePath + map + "/" + map+ "_queries", numQueries);
    Results results(queries);
    runExperimentsAt( G, queries, reader.getIds(), results, "ADJ", algorithmVariant);
    map += "_results";
    // print results
    std::ofstream out(map.c_str(), std::fstream::app);
    results.print(out);
    out.close();
    o_debug.close();
    return 0;
}
