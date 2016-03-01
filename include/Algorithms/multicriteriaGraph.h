#ifndef MULTICRITERIAGRAPH_H
#define MULTICRITERIAGRAPH_H

#include <Structs/Graphs/nodeSelection.h>

static unsigned int NUM_CRITERIA = 2;

class CriteriaList
{
public:
    typedef unsigned int WeightType;
    typedef std::vector< WeightType>::iterator Iterator;
    typedef std::vector< WeightType>::const_iterator ConstIterator;

    CriteriaList( const unsigned int& numCriteria = 0, const unsigned int& defaultValue = 0): m_criteria( numCriteria, defaultValue)
    {
    }

    CriteriaList( const CriteriaList& other): m_criteria( other.m_criteria)
    {
    }

    CriteriaList( const std::vector<WeightType>& other)
    {
        for( unsigned int i = 0; i < other.size(); ++i)
        {
            m_criteria.push_back( other[i]);
        }
    }

    void clear()
    {
        for ( Iterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(); criterion != endCriterion; ++criterion)
        {
            *criterion = 0;
        }
    }

    bool dominates(const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        if ( *this == other) return true; // comment Francis: ciertamente mejorable, se están recorriendo dos veces el vector.

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( (*criterion) > (*otherCriterion))
            {
                return false;
            }
        }
        return true;
    }

    bool dominatesTight(const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        assert( m_criteria.size() > 1);

        //std::cout << (double) m_criteria[0]/other.m_criteria[0] << std::endl;
        if( (double) (1/0.999) * m_criteria[0] < (double)other.m_criteria[0]) return true;
        return false;

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        WeightType mysum = 0;
        WeightType othersum = 0;
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( criterion == m_criteria.begin()) continue;
            mysum += (*criterion);
            othersum += (*criterion);
        }
        criterion = m_criteria.begin();
        otherCriterion = other.m_criteria.begin();
        return ((double)othersum / mysum) > ( (double) (*criterion)/ (*otherCriterion)) * gamma;
    }

    bool isDominatedBy(const CriteriaList& other) const
    {
        return other.dominates(*this);
    }

    WeightType& operator [] ( unsigned int pos)
    {
        assert ( 0 <= pos);
        assert ( pos < m_criteria.size());
        Iterator it = m_criteria.begin();
        std::advance( it, pos);
        return *it;
    }

    bool operator < (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        if ( *this == other) return false;

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( (*criterion) < (*otherCriterion))
            {
                return true;
            }
            if ( (*criterion) > (*otherCriterion))
            {
                return false;
            }
        }
        return false;
    }

    bool operator > (const CriteriaList& other) const
    {
        return other < (*this);
    }

    bool operator == (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        if ( this == &other) return true;

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( (*criterion) != (*otherCriterion))
            {
                return false;
            }
        }
        return true;
    }

    CriteriaList operator + (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        CriteriaList sum( m_criteria.size());

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        Iterator sumCriterion = sum.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion, ++ sumCriterion)
        {
            *sumCriterion = *criterion + *otherCriterion;
        }
        return sum;
    }

    CriteriaList operator - (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        CriteriaList diff( m_criteria.size());

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        Iterator diffCriterion = diff.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion, ++ diffCriterion)
        {
            assert( *criterion >= *otherCriterion);
            *diffCriterion = *criterion - *otherCriterion;
        }
        return diff;
    }

    void print (std::ostream& out, const std::string& delimiter = ", " )
    {
        for ( Iterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(); criterion != endCriterion; ++criterion)
        {
            if ( criterion != m_criteria.begin()) out << delimiter;
            out << *criterion;
        }
    }
private:
    std::vector< WeightType> m_criteria;
    static constexpr double gamma = 1.1;
    static constexpr double epsilon = 0.005;
};

class Label
{
public:

    Label(): m_criteriaList( 0), m_pred(0), m_data(0)
    {
    }

    Label( const unsigned int& numCriteria): m_criteriaList( numCriteria), m_pred(0), m_data(0)
    {
    }

    Label( const CriteriaList& criteriaList, void* pred, void* data):
                                                    m_criteriaList( criteriaList),
                                                    m_pred(pred),
                                                    m_data(data)
    {
    }

    Label( const Label& other):
                        m_criteriaList( other.m_criteriaList),
                        m_pred(other.m_pred),
                        m_data(other.m_data)
    {
    }

    void deletePQitem()
    {
        delete m_data.pqitem;
        m_data.pqitem = 0; // comment Francis: no sería NULLPTR?
    }

    bool dominates(const Label& other) const
    {
        return m_criteriaList.dominates( other.m_criteriaList);
    }

    bool dominatesUnique(const Label& other) const
    {
        if( m_data.boundaryNode == other.m_data.boundaryNode) return m_criteriaList.dominates( other.m_criteriaList);
        return false;
    }

    void* getPredecessor() const
    {
        return m_pred;
    }

    const CriteriaList& getCriteriaList() const
    {
        return m_criteriaList;
    }

    unsigned int* getPQitem() const
    {
        return m_data.pqitem;
    }

    bool isDominatedBy(const Label& other) const
    {
        return other.dominates(*this);
    }

    bool isDominatedUniqueBy(const Label& other) const
    {
        if( m_data.boundaryNode == other.m_data.boundaryNode) return other.dominates(*this);
        return false;
    }

    bool isInQueue() const
    {
        return m_data.pqitem != 0;
    }

    bool operator < (const Label& other) const
    {
        return m_criteriaList < other.m_criteriaList;
    }

    bool operator > (const Label& other) const
    {
        return other < (*this);
    }

    template <class GraphType>
    void print (std::ostream& out, GraphType& G)
    {
        out << "(";
        m_criteriaList.print(out, ", ");

        if ( m_pred )
        {
            typename GraphType::NodeIterator u = G.getNodeIterator((typename GraphType::NodeDescriptor)m_pred);
            //out <<  ", " << u->id;
        }
        else
        {
            out << ", nil";
        }
        out << ")";
    }

private:
    CriteriaList m_criteriaList;
    void* m_pred; // comment Francis: puntero a .. nulo?

    union extra_info
    {
        extra_info(void* init):boundaryNode(init)
        {
        }
        unsigned int* pqitem;
        void * boundaryNode;  // comment Francis: boundaryNode ?? necesario ??
    };

    extra_info m_data;
};

class Node: DefaultGraphItem
{
public:
    Node( unsigned int data = 0): timestamp(0), heuristicList( NUM_CRITERIA)
    {
    }

    template <class GraphType>
    void printLabels( std::ostream& out, GraphType& G)
    {
        out << "G_op(" << id << ") = {\n";
        for ( std::vector<Label>::iterator it = g_op.begin(); it != g_op.end(); ++it)
        {
            out << "\t";
            it->print(out, G);
            out << "\n";
        }
        out << "}\n";
        out << "G_cl(" << id << ") = {\n";
        for ( std::vector<Label>::iterator it = g_cl.begin(); it != g_cl.end(); ++it)
        {
            out << "\t";
            it->print(out, G);
            out << "\n";
        }
        out << "}\n";
    }

    unsigned int x, y;           // x and y coordinates of the node in the grid
    std::vector<Label> labels;
    std::vector<Label> g_op;
    std::vector<Label> g_cl;
    unsigned int timestamp;
    CriteriaList heuristicList;
    unsigned int id;
    unsigned int pqitem, secondary_pqitem;
    bool marked;
    unsigned int dist;
    //void* succ;
    //unsigned int selectionID;
};

class LWNode: Node  // class extending node to calculate the bounded lower bound in the ideal point
{
public:
    LWNode( )
    {
    }

    bool marked;
    unsigned int dist;
};

class GridNode: Node
{
public:
    GridNode( )
    {
    }

    unsigned int x, y;           // x-axis and y-axis coordinates of the node in the grid
};

class node: DefaultGraphItem
{
public:
    node ( unsigned int data = 0): timestamp(0), heuristicList( NUM_CRITERIA)
    {
    }

    unsigned int timestamp;
    CriteriaList heuristicList;
    unsigned int pqitem;
    void* succ;
    unsigned int dist;
    //unsigned int selectionID;
};

struct Edge: DefaultGraphItem
{
    Edge( unsigned int data = 0)
        : criteriaList( NUM_CRITERIA)  // aquí hay que cambiar para nobjetivos
    {
    }

    CriteriaList criteriaList;
    //unsigned int flags;
};


#endif // MULTICRITERIAGRAPH_H

