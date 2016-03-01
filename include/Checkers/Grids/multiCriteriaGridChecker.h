#ifndef MULTICRITERIAGRIDCHECKER_H
#define MULTICRITERIAGRIDCHECKER_H

#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <assert.h>
#include <iostream>
#include <Algorithms/multicriteriaDijkstra.h>
#include <Checkers/multiCriteriaChecker.h>

class GridChecker : public MultiCriteriaChecker
{
public:
    typedef typename std::pair<unsigned int, unsigned int> Coords;
    typedef typename std::pair<Coords, Coords> ProblemCoords;

    GridChecker(unsigned int nqueries, unsigned int ncriteria, std::string QueriesPath, std::string SolutionsPath,
                unsigned int gridDimensionSize) : MultiCriteriaChecker( nqueries, ncriteria, QueriesPath, SolutionsPath),
                m_gridDimension(gridDimensionSize)
    {           
        readQueries( m_queriesFilepath);
        readSolutions( m_solutionsPath, m_nqueries);
    }

    QueriesNodeIDs getQueries(unsigned int benchmarkVariant) // note: it should be based on the queries read from file!
    {
        if (!((2 <= benchmarkVariant) && (benchmarkVariant <= 3)))
            std::cerr << "Error related to benchmarkVariant provided '" << benchmarkVariant << "'\n";
        QueriesNodeIDs queries;
        std::vector<unsigned int> yProblemCoords;
        unsigned int center = (GRIDDIMSIZE / 2) - 1;

        if (benchmarkVariant == 3)
        {
            yProblemCoords = {center, center + 1, center + 2, center + 3, center + 4, center + 5,
                center + 10, center + 15, center + 20};
        }
        else if (benchmarkVariant == 2)
        {
            yProblemCoords = {center, center + 1, center + 2, center + 3, center + 4, center + 5,
                center + 10, center + 15, center + 20, center + 25, center + 30,
                center + 35, center + 40, center + 45, center + 50};
        }

        unsigned int xNode = (center * GRIDDIMSIZE) + center; // node-id of the grid's center
        for (unsigned int yCoord : yProblemCoords)
        {
            unsigned int yNode = (yCoord * GRIDDIMSIZE) + yCoord;
            queries.push_back(std::pair< unsigned int, unsigned int>(xNode, yNode));
        }
        return queries;
    }
/*
    std::vector<CriteriaList>& getSolutions(unsigned int prob_n)
    {
        auto it = m_solutions.find(prob_n);
        if (it != m_solutions.end())
        {
            return it->second;
        }
        else std::cout << "No encontrado\n";
    }
*/
    void show(unsigned int nProbs)
    {
        for (unsigned int i = 0; i < nProbs; ++i)
        {
            std::cout << "p" << i << ": ";
            auto itQ = m_queries.find( i);
            std::cout << "((" << itQ->second.first.first << " " << itQ->second.first.second <<
                         ")->(" << itQ->second.second.first << " " << itQ->second.second.second << "))\n";
            std::cout << "{\n";
            
            std::vector<CriteriaList> solutions = this->getSolutions( i);
            for (auto itS = solutions.begin(); itS != solutions.end(); ++itS)
            {
                std::cout << " ";
                const_cast<CriteriaList&>(*itS).print(std::cout, ", ");
                std::cout << std::endl;
            }
            std::cout << "}\n\n";
        }
    }

private:

    void readQueries(const std::string filepath)
    {
        std::ifstream in;
        in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            in.open(filepath);
            std::string token;
            std::stringstream data;
            unsigned int prob_n, nprobs, ncosts, src_x, src_y, tar_x, tar_y;

            // read problem source and target coordinates
            getline(in, token); // discard the comment on the first line
            // second line
            getline(in, token);
            data.str(token);
            data >> ncosts;
            assert(ncosts == m_ncriteria);
            // third line
            getline(in, token);
            data.str(token);
            data >> nprobs;
            unsigned int counter = nprobs;
            assert(nprobs == m_nqueries);
            // rest
            while (counter > 0 && getline(in, token))
            {
                data.str(token);
                data >> prob_n;
                data >> src_x;
                data >> src_y;
                data >> tar_x;
                data >> tar_y;
                data.clear();
                assert(0 <= src_x && src_x <= m_gridDimension);
                assert(0 <= src_y && src_y <= m_gridDimension);
                assert(0 <= tar_x && tar_x <= m_gridDimension);
                assert(0 <= tar_y && tar_y <= m_gridDimension);
                Coords srcCoords(src_x, src_y);
                Coords tarCoords(tar_x, tar_y);
                ProblemCoords pCoords(srcCoords, tarCoords);
                std::pair<unsigned int, ProblemCoords> query(prob_n, pCoords);
                m_queries.insert(query);
                --counter;
            }
        }
        catch (std::ifstream::failure e)
        {
            std::cerr << "Exception opening/reading file '" << m_queriesFilepath << "'\n";
            throw e;
        }
    }

    void readSolutions(const std::string filepath, const unsigned int nprobs)
    {
        std::ifstream in;
        in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            in.open(filepath);
            std::string token;
            std::stringstream data;
            unsigned int nSols, nCriteria, prob_n;
            unsigned int cont = 0;
            unsigned int counter = nprobs;

            // read problem source and target coordinates
            getline(in, token); // discard the comment on the first line
            while (counter > 0 && getline(in, token))
            {
                data.str(token);
                data >> prob_n;
                data >> nCriteria;

                assert(prob_n == cont && nCriteria == m_ncriteria);

                getline(in, token);
                data.str(token);
                data >> nSols;
                std::vector<CriteriaList> query_solutions;

                for (unsigned int i = 0; i < nSols; ++i)
                {
                    getline(in, token);
                    data.str(token);
                    std::vector<WeightType> paretoSolution;
                    for (unsigned int j = 0; j < nCriteria; ++j)
                    {
                        WeightType elem;
                        data >> elem;
                        paretoSolution.push_back(elem);
                    }
                    CriteriaList solution( paretoSolution);
                    query_solutions.push_back(solution);
                }
                auto it = m_queries.find(prob_n);
                if (it != m_queries.end())
                {
                    std::pair<unsigned int, std::vector<CriteriaList> > solution(prob_n, query_solutions);
                    m_solutions.insert(solution);
                }
                else std::cerr << "Found solution to a query not specified, n: " << prob_n << std::endl;

                query_solutions.clear();
                ++cont;
                data.clear();
                --counter;
            }
        }
        catch (std::ifstream::failure e)
        {
            std::cerr << "Exception opening/reading file '" << m_queriesFilepath << "'\n";
            throw e;
        }
    }

    unsigned int m_gridDimension;
    std::unordered_map<unsigned int, ProblemCoords> m_queries;
};

#endif // MULTICRITERIAGRIDCHECKER_H

