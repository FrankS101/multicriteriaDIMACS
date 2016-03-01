#ifndef NAMOASTAR_H
#define NAMOASTAR_H

#include <Structs/Trees/priorityQueue.h>

template<class GraphType, template <typename graphType> class HeuristicType>
class NamoaStarDijkstra
{
public:
	typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
	typedef typename GraphType::SizeType        SizeType;
	typedef typename GraphType::NodeData        NodeData;

	typedef PriorityQueue< CriteriaList, NodeIterator, HeapStorage> PriorityQueueType;
	typedef typename PriorityQueueType::PQItem PQItem;   
	
    /**
     * @brief Constructor
     *
     * @param graph The graph to run the algorithm on
     * @param timestamp An address containing a timestamp. A timestamp must be given in order to check whether a node is visited or not
     */
    NamoaStarDijkstra( GraphType& graph, unsigned int numCriteria, unsigned int* timestamp):
                        G(graph), m_numCriteria(numCriteria), m_timestamp(timestamp), m_heuristicEngine(graph)
    {
    }
    
    void init(const NodeIterator& s, const NodeIterator& t, const unsigned int nCriteria)
	{
		NodeIterator u, lastNode;
		for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
		    u->labels.clear();
		}
        m_heuristicEngine.init( s, t, nCriteria);
		pq.clear();
	}

    /**
     * @brief Builds a shortest path tree routed on a source node
     *
     * @param s The source node
     */
    void runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {
		NodeIterator u,v,lastNode;
		EdgeIterator e,lastEdge;

        m_generatedLabels = 0;
        //assert( hasFeasiblePotentials(t));
		++(*m_timestamp);
		
		unsigned int* pqitem = new unsigned int();
        //o_debug << "pqitem: " << *pqitem << std::endl;
		s->labels.push_back(Label( CriteriaList(m_numCriteria), 0, pqitem));
		pq.insert( CriteriaList(m_numCriteria) + s->heuristicList, s, pqitem);

		while( !pq.empty())
		{
		    CriteriaList minCriteria = pq.min().key;
		    u = pq.minItem();
		    pq.popMin();

			CriteriaList g_u = minCriteria - u->heuristicList;
            ++m_generatedLabels;

            if (DEBUG == 1) {
                o_debug << "\nIT: " << m_generatedLabels << "\nExtracting |" << u->id << "| ";
                o_debug << "f: (";
                minCriteria.print( o_debug, ", ");
                o_debug << ") g: (";
                g_u.print(o_debug, ", ");
                o_debug << ")";
                o_debug << std::endl;
            }
            /*
			if( u == t)
            {
                eraseAllDominatedLabels( G, t, g_u);
            }
            */
			moveToClosed( g_u, u);

            if ( isDominatedByNodeLabels( t, minCriteria))
            {
                if (DEBUG == 1) {
                    o_debug << "  Filtering |" << v->id << "| ";
                    o_debug << "f: (";
                    minCriteria.print( o_debug, ", ");
                    o_debug << ") g: (";
                    g_u.print(o_debug, ", ");
                    o_debug << ")";
                    o_debug << std::endl;
                }
                continue;
            }

		  	for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
		    {
				v = G.target(e);

				if ( v->timestamp != (*m_timestamp))
				{
					v->labels.clear();
					v->timestamp = (*m_timestamp);
				}
				
				CriteriaList g_v = g_u + e->criteriaList;
				CriteriaList heuristicCost = g_v + v->heuristicList;

                if (DEBUG == 1) {
                    o_debug << " Expanding |" << v->id << "| ";
                    o_debug << "f: (";
                    heuristicCost.print( o_debug, ", ");
                    o_debug << ") g: (";
                    g_v.print(o_debug, ", ");
                    o_debug << ")";
                    o_debug << std::endl;
                }

				if ( distanceExistsInNode( v, g_v))
				{
                    if (DEBUG == 1) {
                        o_debug << "  Distance Exists in node. \n";
                    }
                    continue;
                    //v->labels.push_back( Label( g_v, u->getDescriptor(), 0) );
                    //++m_generatedLabels;
				}
				else	
				{
                    if( isDominatedByNodeLabels( v, g_v))
                    {
                        if (DEBUG == 1) {
                            o_debug << "  It is dominated by G_cl(u). \n";
                        }
                        continue;
                    }
					eraseDominatedLabels( G, v, g_v);
                    if( isDominatedByNodeLabels(t, heuristicCost))
                    {
                        if (DEBUG == 1) {
                            o_debug << "  It is dominated by the node labels (f). \n";
                        }
                        continue;
                    }
					unsigned int* pqitem = new unsigned int();
                    // o_debug << "pqitem: " << *pqitem << std::endl;
					v->labels.push_back( Label( g_v, u->getDescriptor(), pqitem) );
                    //std::cout << "PQ item(1) = " << *(v->labels.back().getPQitem()) << std::endl;
                    if (DEBUG == 1) {
                        o_debug << "  Inserting |" << v->id << "| ";
                        o_debug << "f: (";
                        heuristicCost.print( o_debug, ", ");
                        o_debug << ") g: (";
                        g_v.print(o_debug, ", ");
                        o_debug << ")";
                        o_debug << std::endl;
                    }
					pq.insert( heuristicCost, v, pqitem);
                    //std::cout << "PQ item(2) = " << *pqitem << std::endl;
                }
            }
            /*
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);
                std::cout << "PQ item = " << *(v->labels.front().getPQitem()) << std::endl;
            }
            */
		}
    }

    const unsigned int& getGeneratedLabels()
    {
        return m_generatedLabels;
    }
    
    /**
     * @brief Runs a shortest path query between a source node s and a target node t
     *
     * @param s The source node
     * @param t The target node
     * @return The distance of the target node
     *
    WeightType runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {

    }*/

private:
    GraphType& G;
    PriorityQueueType pq;
    unsigned int m_generatedLabels;
	unsigned int m_numCriteria;
	unsigned int* m_timestamp;
    HeuristicType<GraphType> m_heuristicEngine;

	bool distanceExistsInNode( const NodeIterator& v, const CriteriaList& g_v)
	{
		for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
		{
			if ( it->getCriteriaList() == g_v )
		    {
				return true;
		    }
		}
		return false;		
	}

	void eraseDominatedLabels( GraphType& G, const NodeIterator& v, const CriteriaList& g_v)
	{
        EdgeIterator e, lastEdge;
        NodeIterator w;
		std::vector<Label>::iterator it = v->labels.begin();
		while ( it != v->labels.end() )
		{
			if ( it->getCriteriaList().isDominatedBy(g_v) )
		    {
				if( it->isInQueue())
				{
                    // std::cout << *(it->getPQitem()) << std::endl;
					pq.remove( it->getPQitem());
					moveToClosed( it->getCriteriaList(), v);
				}
				else
				{
                    // I still have to investigate the utility of this --> (IMPORTANT)
                    // GO FORWARD IN SEARCH SPACE TO FIND MORE DOMINATED LABELS
                    /*
				    for( e = G.beginEdges(v), lastEdge = G.endEdges(v); e != lastEdge; ++e)
		            {
                        //std::cout << "Reducing search space...\n";
				        w = G.target(e);
                        eraseDominatedLabels( G, w, g_v + e->criteriaList);
			        }
                    */
			    }
				it = v->labels.erase( it );
		    }
		    else 
		    {
			    ++it;
		    }
		}
	}
	
	void eraseAllDominatedLabels(GraphType& G, const NodeIterator& t, const CriteriaList& g_v)
	{
        NodeIterator u, lastNode;
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
            if( u->labels.empty()) continue;
            if( u == t) continue;
            eraseDominatedLabels( G, u, g_v);
        }
    }


    bool hasFeasiblePotentials( const typename GraphType::NodeIterator& t)
    {
        NodeIterator u,v,lastNode;
        EdgeIterator e,lastEdge;

        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);
				CriteriaList c_e = e->criteriaList;
				CriteriaList c_v = v->heuristicList;
				CriteriaList c_u = u->heuristicList;
                if( e->criteriaList + v->heuristicList < u->heuristicList)
                {
                    return false;
                }
            }       
        }
        return true;
    }

	

	bool isDominatedByNodeLabels( const NodeIterator& v, const CriteriaList& g_v)
	{
		for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
		{
			if ( it->getCriteriaList().dominates(g_v) )
		    {
				return true;
		    }
		}
		return false;
	}

	void moveToClosed( const CriteriaList& g_u, const NodeIterator& u)
	{
		for ( std::vector<Label>::iterator it = u->labels.begin(); it != u->labels.end(); ++it)
		{
			if ( (it->isInQueue()) && (it->getCriteriaList() == g_u) )
		    {
				it->deletePQitem();
		    }
		}		
	}
	
};


#endif//NAMOASTAR_H

