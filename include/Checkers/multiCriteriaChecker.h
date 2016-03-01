#ifndef MULTICRITERIACHECKER_H
#define MULTICRITERIACHECKER_H

#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <assert.h>
#include <iostream>
#include <Algorithms/multicriteriaDijkstra.h>

typedef unsigned int WeightType;
typedef std::vector<WeightType> ParetoSolution;
typedef std::vector< std::pair< unsigned int, unsigned int> > QueriesNodeIDs;

class MultiCriteriaChecker
{
public:
    MultiCriteriaChecker(unsigned int nqueries, unsigned int ncriteria, std::string QueriesPath, std::string SolutionsPath) :
    m_nqueries(nqueries), m_ncriteria(ncriteria), m_queriesFilepath(QueriesPath), m_solutionsPath(SolutionsPath)
    {
    }

    std::vector<CriteriaList>& getSolutions(unsigned int query_n)
    {
        auto it = m_solutions.find(query_n);
        if (it != m_solutions.end())
        {
            return it->second;
        }
        else
        {
            std::cerr << "Provided query number not found.\n";
            return it->second;
        }
    }

protected:
    unsigned int m_nqueries;
    unsigned int m_ncriteria;
    std::string m_queriesFilepath;
    std::string m_solutionsPath;
    std::unordered_map<unsigned int, std::vector<CriteriaList> > m_solutions;
};

#endif // MULTICRITERIACHECKER_H

