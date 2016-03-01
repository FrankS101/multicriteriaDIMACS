#ifndef MULTICRITERIARESULTS_H
#define MULTICRITERIARESULTS_H

#include <Checkers/multiCriteriaChecker.h>
#include <Algorithms/multicriteriaDijkstra.h>

typedef typename std::pair<unsigned int, unsigned int> QueryCoords;
typedef typename std::vector<QueryCoords> Queries;

class MulticriteriaResults
{
public:

    MulticriteriaResults(const Queries& queries) : m_queries(queries)
    {
    }

    ~MulticriteriaResults()
    {
        m_paretoSolutions.erase( m_paretoSolutions.begin(), m_paretoSolutions.end() );
    }

    void addResults(const std::vector<Label> labels)
    {
        for (std::vector<Label>::const_iterator it = labels.begin(); it != labels.end(); ++it)
        {
            m_paretoSolutions.push_back(it->getCriteriaList());
        }
    }

    void print(std::ostream& out);

    bool checkParetoCosts(MultiCriteriaChecker& checker, unsigned int query_n, bool& order)
    {
        std::vector<CriteriaList> correctSolutions = checker.getSolutions(query_n);

        if (correctSolutions == m_paretoSolutions)
        {
            order = true;
            return true;
        }

        std::sort(m_paretoSolutions.begin(), m_paretoSolutions.end());

        if (correctSolutions == m_paretoSolutions)
        {
            order = false;
            return true;
        }
        correctSolutions.clear();
        return false;
    }

private:

    //std::vector< std::string> m_headers;
    //std::vector< std::vector<double> > m_data;
    const Queries& m_queries;
    //std::vector< std::string> m_headers;
    //std::vector< std::vector<double> > m_data;
    //const std::vector< std::pair<unsigned int, unsigned int> >& m_query;
    unsigned int m_nExpandedLabels;
    std::vector<CriteriaList> m_paretoSolutions;
};

#endif // MULTICRITERIARESULTS_H

