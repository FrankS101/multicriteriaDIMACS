#ifndef IDEAL_H
#define IDEAL_H

#include <Structs/Trees/priorityQueue.h>
#include <Heuristics/heuristic.h>

template<class GraphType>
class TCHeuristic
{
public:
    typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
	typedef typename GraphType::InEdgeIterator  InEdgeIterator;
	typedef PriorityQueue< unsigned int, NodeIterator, HeapStorage>   PriorityQueueType;
	
    TCHeuristic( GraphType& graph):G(graph)
    {
        m_timestamp = new unsigned int();
        *m_timestamp = 0;
    } 
    
    ~TCHeuristic()
    {
        delete m_timestamp;
    }

    void init( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t, const unsigned int nCriteria)
    {
        NodeIterator u,lastNode;
        *m_timestamp = 1;
        unsigned int EMPTY_HEURISTIC = std::numeric_limits<unsigned int>::max();
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
            for (unsigned int i = 0; i < nCriteria; ++i)
            {
                u->heuristicList[i] = EMPTY_HEURISTIC;
                u->timestamp = 0;
            }
		}
        for (unsigned int i = 0; i < nCriteria; ++i)
        {
            buildTree(t,i);
            ++(*m_timestamp);
        }
    }

    void buildTree( const typename GraphType::NodeIterator& t, unsigned int criterionIndex)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        pq.clear();
        ++(*m_timestamp);
        t->heuristicList[criterionIndex] = 0;
        t->timestamp = (*m_timestamp);
        //t->succ = G.nilNodeDescriptor();
        pq.insert( t->heuristicList[criterionIndex], t, &(t->pqitem));
        while( !pq.empty())
        {
            u = pq.minItem();
            pq.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);
                if( v->timestamp < (*m_timestamp))
                {
                   //v->succ = u->getDescriptor();
                    v->heuristicList[criterionIndex] = u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex];
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->heuristicList[criterionIndex], v, &(v->pqitem));
                }
                else if( v->heuristicList[criterionIndex] > u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex] )
                {
                    //v->succ = u->getDescriptor();
                    v->heuristicList[criterionIndex] = u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex];
                    pq.decrease( v->heuristicList[criterionIndex], &(v->pqitem));
                }
            }
        }
    }    
    
private:
    GraphType& G;
    PriorityQueueType pq;
    unsigned int* m_timestamp;
};

#endif // IDEAL_H
