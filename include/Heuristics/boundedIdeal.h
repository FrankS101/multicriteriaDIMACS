#ifndef BOUNDEDIDEAL_H
#define BOUNDEDIDEAL_H

#include <Structs/Trees/priorityQueue.h>
#include <Heuristics/heuristic.h>

/*
 *  IMPORTANT! This procedure will work only for bicriterion problems.
 * A generalization of the calculation is neccessary for the multicriteria problem.
 *
*/

template<class GraphType>
class BoundedTCHeuristic
{
public:
    
    typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
	typedef typename GraphType::InEdgeIterator  InEdgeIterator;
	typedef PriorityQueue< unsigned int, NodeIterator, HeapStorage>   PriorityQueueType;
	
    BoundedTCHeuristic( GraphType& graph):G(graph)
    {
        m_timestamp = new unsigned int();
        *m_timestamp = 0;
    } 
    
    ~BoundedTCHeuristic()
    {
        delete m_timestamp;
    }
     
    void buildTree( const typename GraphType::NodeIterator& t, unsigned int bound)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        
        secondary_pq.clear();
        t->heuristicList[1] = 0;
        t->marked = true;
        //t->succ = G.nilNodeDescriptor();
        t->dist = 0;
        secondary_pq.insert( t->heuristicList[1], t, &(t->secondary_pqitem));

        while( !secondary_pq.empty())
        {
            if( secondary_pq.minKey() > bound) break;
            u = secondary_pq.minItem();
            secondary_pq.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);
                if( !v->marked)
                {
                    //v->succ = u->getDescriptor();
                    v->heuristicList[1] = u->heuristicList[1] + k->criteriaList[1];
                    v->dist = u->dist + k->criteriaList[0];
                    v->marked = true;
                    secondary_pq.insert( v->heuristicList[1], v, &(v->secondary_pqitem));
                }
                else if( v->heuristicList[1] > u->heuristicList[1] + k->criteriaList[1] )
                {
                    //v->succ = u->getDescriptor();
                    v->heuristicList[1] = u->heuristicList[1] + k->criteriaList[1];
                    v->dist = u->dist + k->criteriaList[0];
                    secondary_pq.decrease( v->heuristicList[1], &(v->secondary_pqitem));
                }
            }
        }
    }    

    void buildSingleTree( const typename GraphType::NodeIterator& t, unsigned int criterionIndex, unsigned int bound)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        while( !pq.empty())
        {
            if( pq.minKey() > bound) break;
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
                u->marked = false;
            }
        }

        // stage 1: find the bounds
        runQuery( s, t, 0);
        unsigned int bound1 = s->heuristicList[1];

        buildTree(t,bound1);

        unsigned int bound0 = s->dist;
        buildSingleTree(t,0,bound0);
        
    }

    void runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t, unsigned int criterionIndex)
    {
        NodeIterator u,v;
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
            if( u == s) break;
            pq.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( v->timestamp < (*m_timestamp))
                {
                    //v->succ = u->getDescriptor();
                    v->heuristicList = u->heuristicList + k->criteriaList;
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->heuristicList[criterionIndex], v, &(v->pqitem));
                }
                else if( v->heuristicList[criterionIndex] > u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex] )
                {
                    //v->succ = u->getDescriptor();
                    v->heuristicList = u->heuristicList + k->criteriaList;
                    pq.decrease( v->heuristicList[criterionIndex], &(v->pqitem));
                }
            }
        }
    } 
    
private:
    GraphType& G;
    PriorityQueueType pq;
    PriorityQueueType secondary_pq;
    unsigned int* m_timestamp;
};

#endif // BOUNDEDIDEAL_H
