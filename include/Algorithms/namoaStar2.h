#ifndef NAMOASTAR2_H
#define NAMOASTAR2_H

#include <Structs/Trees/priorityQueue.h>
#include <Utilities/geographic.h>
#include <memory>

template<class GraphType, template <typename graphType> class HeuristicGraphType>
class NamoaStar2
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
    NamoaStar2( GraphType& graph, unsigned int numCriteria, unsigned int* timestamp):
                        G(graph), m_numCriteria(numCriteria), m_timestamp(timestamp), m_heuristicEngine(graph)
    {
    }


    void init(const NodeIterator& s, const NodeIterator& t, const unsigned int nCriteria)
    {
        NodeIterator u, lastNode;
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
            //u->labels.clear();
            u->g_op.clear();
            u->g_cl.clear();
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
        NodeIterator u,v;
        EdgeIterator e,lastEdge;

        m_generatedLabels = 0;
        assert( hasFeasiblePotentials(t));
        ++(*m_timestamp);

         unsigned int* pqitem = new unsigned int();
        // std::cout << "pqitem: " << *pqitem << std::endl;
        //s->labels.push_back(Label( CriteriaList(m_numCriteria), 0, pqitem));
        s->g_op.push_back(Label( CriteriaList(m_numCriteria), 0, pqitem));
        pq.insert( CriteriaList(m_numCriteria) + s->heuristicList, s, &(s->pqitem));

        while( !pq.empty())
        {
            // get the minimum lexicographic evaluation vector (f) from the PQ and its key
            CriteriaList minCriteria = pq.min().key;
            u = pq.minItem();
            // pop the element from the PQ
            pq.popMin();
            // calculate its cost vector (g)
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

            // move the label from G_op(u) to G_cl(u)
            moveToClosed( g_u, u);
            // whenever there are more labels in G_op(u), insert the best in PQ
            if ( !( u->g_op.empty()))
            {
                CriteriaList best_f_u = u->g_op.front().getCriteriaList() + u->heuristicList;
                pq.insert( best_f_u, u, &(u->pqitem));
                if (DEBUG == 1) {
                    o_debug << "  Inserting in PQ from G_op(u) |" << u->id << "| ";
                    o_debug << "f: (";
                    best_f_u.print( o_debug, ", ");
                    o_debug << ") g: (";
                    CriteriaList g_u = CriteriaList( u->g_op.front().getCriteriaList());
                    g_u.print(o_debug, ", ");
                    o_debug << ")";
                    o_debug << std::endl;
                }
            }
            if ( u == t)
            {
                continue;
            }
            if ( isDominatedBySolutions( t, minCriteria))
            {
                if (DEBUG == 1) {
                    o_debug << "  It is dominated by G_cl(u). \n";
                }
                continue;
            }
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);
                /*
                if ( v->timestamp != (*m_timestamp))
                {
                    v->labels.clear();
                    v->timestamp = (*m_timestamp);
                }
                */
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

                if ( isDominatedBySolutions( t, heuristicCost))
                {
                    if (DEBUG == 1) {
                        o_debug << "  It is dominated by the node labels (f). \n";
                    }
                    continue;
                }
                else
                {
                    if ( isDominatedByClosed( v, g_v))
                    {
                        if (DEBUG == 1) {
                            o_debug << "  It is dominated by G_cl(u). \n";
                        }
                        continue;
                    }
                    else
                    {
                        std::unique_ptr<CriteriaList> first;
                        if ( !( v->g_op.empty)())
                        {
                            first = std::unique_ptr<CriteriaList>( new CriteriaList( v->g_op.front().getCriteriaList()));
                        }
                        // check if g_v is dominated, if not, insert g_v in G_op(v) and remove the labels dominated by g_v
                        bool inserted = false;
                        unsigned int* pqitem = new unsigned int();
                        v->g_op = GopManagement( v, u, g_v, pqitem, inserted);
                        // if inserted, then insert in PQ as well (if applies)
                        if ( inserted)
                        {
                            if ( v->g_op.size() == 1)
                            {
                                if ( first == nullptr) // there wasn't any label before, insert in PQ
                                {
                                    pq.insert( heuristicCost, v, &(v->pqitem));
                                    if (DEBUG == 1) {
                                        o_debug << "  Inserting |" << v->id << "| ";
                                        o_debug << "f: (";
                                        heuristicCost.print( o_debug, ", ");
                                        o_debug << ") g: (";
                                        g_v.print(o_debug, ", ");
                                        o_debug << ")";
                                        o_debug << std::endl;
                                    }
                                }
                                else
                                {
                                    pq.decrease( heuristicCost, &(v->pqitem));
                                }
                            }
                            else if ( !( v->g_op.front().getCriteriaList() == *first))
                            {
                                pq.decrease( heuristicCost, &(v->pqitem));
                            }
                        }
                    }
                }
            }
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
    HeuristicGraphType<GraphType> m_heuristicEngine;

    void moveToClosed( const CriteriaList& g_u, const NodeIterator& u)
    {
        // insert into G_cl(u)
        u->g_cl.push_back( Label( g_u, u->getDescriptor(), 0));
        // remove from G_op(u)
        if ( !(u->g_op.empty()))
        {
            u->g_op.erase( u->g_op.begin());
        }
    }

    bool isDominatedBySolutions( const NodeIterator& t, const CriteriaList& f_v)
    { // performance of this should be further investigated // francis
        if ( ( t->g_op.empty()) && ( t->g_cl.empty()))
        {
            return false;
        }
        else if ( (!(t->g_op.empty())) && (f_v.isDominatedBy( t->g_op.back().getCriteriaList())))
        {
            return true;
        }
        else if ( (!(t->g_cl.empty())) && (f_v.isDominatedBy( t->g_cl.back().getCriteriaList())))
        {
            return true;
        }
        else return false;
    }

    bool isDominatedByClosed( const NodeIterator& v, const CriteriaList& g_v)
    {
        if ( v->g_cl.empty())
        {
            return false;
        }
        else
        {
            return g_v.isDominatedBy( v->g_cl.back().getCriteriaList());
        }
    }

    std::vector<Label> GopManagement( NodeIterator& v, NodeIterator& u, CriteriaList& g_v, unsigned int* pqitem, bool& inserted = false)
    {
        std::vector<Label> new_g_op;
        if ( v->g_op.empty()) {
            new_g_op.push_back( Label( g_v, u->getDescriptor(), pqitem));
            inserted = true;
            return new_g_op;
        }
        for ( std::vector<Label>::iterator it = v->g_op.begin(); it != v->g_op.end(); ++it)
        {
            if ( it->getCriteriaList().dominates(g_v) )
            {
                if (DEBUG == 1) {
                    o_debug << "  It is dominated by G_op(u). \n";
                }
                return v->g_op;
            }
            if ( g_v == it->getCriteriaList())
            {
                if (DEBUG == 1) {
                    o_debug << "  Same cost vector found in G_op(u). \n";
                }
                return v->g_op;
            }
            if ( !( inserted))
            {
                if ( g_v < it->getCriteriaList())
                {
                    new_g_op.push_back( Label( g_v, u->getDescriptor(), pqitem));
                    inserted = true;
                    if (DEBUG == 1) {
                        o_debug << "  Inserting in G_op |" << v->id << "| ";
                        o_debug << "g: (";
                        g_v.print(o_debug, ", ");
                        o_debug << ")";
                        o_debug << std::endl;
                    }
                    if ( !( it->getCriteriaList().isDominatedBy( g_v)))
                    {
                        new_g_op.push_back( *it);
                    }
                }
                else
                {
                    new_g_op.push_back( *it);
                }
            }
            else if ( !( it->getCriteriaList().isDominatedBy( g_v)))
            {
                new_g_op.push_back( *it);
            }
        }
        if ( !( inserted))
        {
            new_g_op.push_back( Label( g_v, u->getDescriptor(), pqitem));
            inserted = true;
            if (DEBUG == 1) {
                o_debug << "  Inserting in G_op |" << v->id << "| ";
                o_debug << "g: (";
                g_v.print(o_debug, ", ");
                o_debug << ")";
                o_debug << std::endl;
            }
        }
        return new_g_op;
    }


/*
    std::vector<Label> GopManagement( NodeIterator& v, NodeIterator& u, CriteriaList& g_v, unsigned int* pqitem, bool& inserted = false)
    {
        std::vector<Label> new_g_op;
        if ( v->g_op.empty()) {
            //unsigned int* pqitem = new unsigned int();
            new_g_op.push_back( Label( g_v, u->getDescriptor(), pqitem));
            //std::cout << "pqitem(1): " << *pqitem << std::endl;
            inserted = true;
            return new_g_op;
        }
        for ( std::vector<Label>::iterator it = v->g_op.begin(); it != v->g_op.end(); ++it)
        {
            if ( it->getCriteriaList().dominates(g_v) )
            {
                if (DEBUG == 1) {
                    o_debug << "  It is dominated by G_op(u). \n";
                }
                return v->g_op;
            }
            if ( !( inserted))
            {
                if ( g_v < it->getCriteriaList())
                {
                    //unsigned int* pqitem = new unsigned int();
                    //pqitem = new unsigned int();
                    new_g_op.push_back( Label( g_v, u->getDescriptor(), pqitem));
                    //std::cout << "pqitem(2): " << *pqitem << std::endl;
                    inserted = true;
                    if ( !( it->getCriteriaList().isDominatedBy( g_v)))
                    {
                        new_g_op.push_back( *it);
                    }
                }
                else
                {
                    new_g_op.push_back( *it);
                }
            }
            else if ( !( it->getCriteriaList().isDominatedBy( g_v)))
            {
                new_g_op.push_back( *it);
            }
        }
        if ( !( inserted))
        {
            new_g_op.push_back( Label( g_v, u->getDescriptor(), pqitem));
            inserted = true;
        }
        return new_g_op;
    }
*/

    bool distanceExistsInNode( const NodeIterator& v, const CriteriaList& g_v)
    {
        /*
        for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
        {
            if ( it->getCriteriaList() == g_v )
            {
                return true;
            }
        }*/
        for ( std::vector<Label>::iterator it = v->g_op.begin(); it != v->g_op.end(); ++it)
        {
            if ( it->getCriteriaList() == g_v )
            {
                return true;
            }
        }
        for ( std::vector<Label>::iterator it = v->g_cl.begin(); it != v->g_cl.end(); ++it)
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
        /*
        std::vector<Label>::iterator it = v->labels.begin();
        while ( it != v->labels.end() )
        {
            if ( it->getCriteriaList().isDominatedBy(g_v) )
            {
                if( it->isInQueue())
                {
                    pq.remove( it->getPQitem());
                    moveToClosed( it->getCriteriaList(), v);
                }
                else
                {
                    // GO FORWARD IN SEARCH SPACE TO FIND MORE DOMINATED LABELS
                    for( e = G.beginEdges(v), lastEdge = G.endEdges(v); e != lastEdge; ++e)
                    {
                        //std::cout << "Reducing search space...\n";
                        w = G.target(e);
                        eraseDominatedLabels( G, w, g_v + e->criteriaList);
                    }
                }
                it = v->labels.erase( it );
            }
            else
            {
                ++it;
            }
        }
        */
        std::vector<Label>::iterator it = v->g_op.begin();
        while ( it != v->g_op.end() )
        {
            if ( it->getCriteriaList().isDominatedBy(g_v) )
            {
                if( it->isInQueue())
                {
                    pq.remove( it->getPQitem());
                    moveToClosed( it->getCriteriaList(), v);
                }
                else
                {
                    // GO FORWARD IN SEARCH SPACE TO FIND MORE DOMINATED LABELS
                    for( e = G.beginEdges(v), lastEdge = G.endEdges(v); e != lastEdge; ++e)
                    {
                        //std::cout << "Reducing search space...\n";
                        w = G.target(e);
                        eraseDominatedLabels( G, w, g_v + e->criteriaList);
                    }
                }
                it = v->g_op.erase( it );
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
        /*
        for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
        {
            if ( it->getCriteriaList().dominates(g_v) )
            {
                return true;
            }
        }
        */
        for ( std::vector<Label>::iterator it = v->g_op.begin(); it != v->g_op.end(); ++it)
        {
            if ( it->getCriteriaList().dominates(g_v) )
            {
                return true;
            }
        }
        for ( std::vector<Label>::iterator it = v->g_cl.begin(); it != v->g_cl.end(); ++it)
        {
            if ( it->getCriteriaList().dominates(g_v) )
            {
                return true;
            }
        }
        return false;
    }


};

#endif // NAMOASTAR2_H

