/*****************************************************************************
 * Copyright 2005, 2006, 2007 Jan Bretschneider, Peter Massuthe, Leonard Kern*
 *                                                                           *
 * This file is part of Fiona.                                               *
 *                                                                           *
 * Fiona is free software; you can redistribute it and/or modify it          *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or (at your    *
 * option) any later version.                                                *
 *                                                                           *
 * Fiona is distributed in the hope that it will be useful, but WITHOUT      *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for  *
 * more details.                                                             *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with Fiona; if not, write to the Free Software Foundation, Inc., 51       *
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.                      *
 *****************************************************************************/

/*!
 * \file    OGFromFile.h
 *
 * \brief   Holds an Operating Guidelines (OG) read from a OG file. We do not
 *          use the class OG for storing OGs read from a file because the class
 *          OG is tailored for computing an OG and therefore heavily depends
 *          on the existence of an underlying oWFN. If we used the class OG for
 *          storing an already computed OG, we would have to reimplement most
 *          methods anyway and the possibility of reuse would be minimal.
 *
 * \author  responsible: Jan Bretschneider <bretschn@informatik.hu-berlin.de>
 *
 * \note    This file is part of the tool Fiona and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 */

#ifndef OGFROMFILE_H_
#define OGFROMFILE_H_

#include <fstream>
#include <string>
#include <set>
#include <map>
#include <list>
#include "GraphNode.h"

using namespace std;

template<typename T> class GraphEdge;


class GraphFormula;
class GraphFormulaAssignment;
class GraphFormulaCNF;

class OGFromFileNode : public GraphNodeCommon<OGFromFileNode> {
//
//    public:
//        OGFromFileNode(const std::string& name_,
//                       GraphFormula* annotation_,
//                       GraphNodeColor color = BLUE);
//
};


class oWFN;

class OGFromFile {
    public:
        /**
         * Type of container passed to OGFromFile::product().
         */
        typedef std::list<OGFromFile*> ogs_t;

        /**
         * Type of container passed to OGFromFile::getProductOGFilePrefix().
         */
        typedef std::list<std::string> ogfiles_t;

    private:
        GraphNodeCommon<OGFromFileNode>* root;
        typedef std::set<GraphNodeCommon<OGFromFileNode>*> nodes_t;
        nodes_t nodes; // needed for proper deletion of OG.
        typedef nodes_t::const_iterator nodes_const_iterator;
        typedef nodes_t::iterator nodes_iterator;

        void removeTransitionsToNodeFromAllOtherNodes(const GraphNodeCommon<OGFromFileNode>* nodeToDelete);

        /**
         * Recursive coordinated dfs through OG and rhs OG.
         * \param currentOGNode the current node of the OG
         * \param currentRhsNode the current node of the rhs OG
         * \param productOG the resulting product OG
         */
        void buildProductOG(GraphNodeCommon<OGFromFileNode>* currentOGNode,
                            GraphNodeCommon<OGFromFileNode>* currentRhsNode,
                            OGFromFile* productOG);

        bool simulatesRecursive(GraphNodeCommon<OGFromFileNode> *myNode,
                                set<GraphNodeCommon<OGFromFileNode>*> *myVisitedNodes,
                                GraphNodeCommon<OGFromFileNode> *simNode,
                                set<GraphNodeCommon<OGFromFileNode>*> *simVisitedNodes);
    public:

        OGFromFile();
        ~OGFromFile();
        void addNode(GraphNodeCommon<OGFromFileNode>* node);
        GraphNodeCommon<OGFromFileNode>* addNode(const std::string& nodeName,
                                GraphFormula* annotation,
                                GraphNodeColor color = BLUE);
        void addTransition(const std::string& srcName,
                           const std::string& dstName,
                           const std::string& label);
        bool hasNodeWithName(const std::string& nodeName) const;
        GraphNodeCommon<OGFromFileNode>* getRoot() const;
        void setRoot(GraphNodeCommon<OGFromFileNode>* newRoot);
        void setRootToNodeWithName(const std::string& nodeName);
        GraphNodeCommon<OGFromFileNode>* getNodeWithName(const std::string& nodeName) const;
        bool hasNoRoot() const;

        /**
         * @todo Remove unreachable nodes after (or during) removal of false
         * nodes.
         */
        void removeFalseNodes();

        /**
         * Returns the product OG of all given OGs. The caller has to delete the
         * returned OGFromFile.
         * \param ogs Given OGs. Must contain at least two OG.
         */
        static OGFromFile* product(const ogs_t& ogs);

        /**
         * Returns the product OG of this OG and the passed one. The caller has to
         * delete the returned OGFromFile.
         */
        OGFromFile* product(const OGFromFile* rhs);

        /**
         * Creates and returns the annotation for the product node of the given two
         * nodes. The caller is responsible for deleting the returned formula.
         */
        GraphFormulaCNF* createProductAnnotation(const GraphNodeCommon<OGFromFileNode>* lhs,
                                          const GraphNodeCommon<OGFromFileNode>* rhs) const;

        /**
         * Produces from the given OG file names the default prefix of the
         * product OG output file.
         */
        static std::string getProductOGFilePrefix(const ogfiles_t& ogfiles);

        /**
         * Strips the OG file suffix from filename and returns the result.
         */
        static std::string stripOGFileSuffix(const std::string& filename);

        void printGraphToDot(GraphNodeCommon<OGFromFileNode>* v,
                             fstream& os,
                             std::map<GraphNodeCommon<OGFromFileNode>*, bool>&) const;

        void printDotFile(const std::string& filenamePrefix) const;
        void printDotFile(const std::string& filenamePrefix,
                          const std::string& dotGraphTitle) const;

        /**
         * Prints this OG in OG file format to a file with the given prefix. The
         * suffix is added automatically by this method.
         */
        void printOGFile(const std::string& filenamePrefix) const;

        /**
         * Adds the suffix for OG files to the given file name prefix and returns
         * the result. The suffix includes the . (dot).
         */
        static std::string addOGFileSuffix(const std::string& filePrefix);

        bool simulates(OGFromFile *smallerOG);

        /**
         * computes the number of Services determined by this OG
         */
        unsigned int numberOfServices();
        unsigned int numberOfServicesRecursively(set<GraphNodeCommon<OGFromFileNode>*> activeNodes,
                                            map<GraphNodeCommon<OGFromFileNode>*, unsigned int>& followers,
                                            map<GraphNodeCommon<OGFromFileNode>*, list <set<GraphNodeCommon<OGFromFileNode>*> > >& validFollowerCombinations,
                                            map<set<GraphNodeCommon<OGFromFileNode>*>, unsigned int>& eliminateRedundantCounting);
        unsigned int processAssignmentsRecursively(set<string> labels,
                                              GraphFormulaAssignment possibleAssignment,
                                              GraphNodeCommon<OGFromFileNode>* testNode,
                                              list<GraphFormulaAssignment>& assignmentList);

        //! Tests, if this OG is acyclic
        bool isAcyclic();
};

#endif
