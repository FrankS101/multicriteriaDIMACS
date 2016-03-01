#ifndef BLIND_H
#define BLIND_H

template<class GraphType>
class BlindHeuristic
{
public:
    
    typedef typename GraphType::NodeIterator    NodeIterator;
    BlindHeuristic( GraphType& graph):G(graph)
    {
    } 
    
    void init(const NodeIterator& s,const NodeIterator& t, const unsigned int nCriteria)
	{
		NodeIterator u, lastNode;
		for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
            for (unsigned int i = 0; i < nCriteria; ++i)
            {
                u->heuristicList[i] = 0;
            }
		}
	}
    
private:
    GraphType& G;
};


#endif // BLIND_H
