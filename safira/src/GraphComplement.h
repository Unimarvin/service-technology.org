/*****************************************************************************\
 Safira -- Implementing a Set Algebra for Service Behavior

 Copyright (c) 2010 Kathrin Kaschner

 Safira is free software: you can redistribute it and/or modify it under the
 terms of the GNU Affero General Public License as published by the Free
 Software Foundation, either version 3 of the License, or (at your option)
 any later version.

 Safira is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
 more details.

 You should have received a copy of the GNU Affero General Public License
 along with Safira.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _GRAPHCOMPLEMENT_H
#define _GRAPHCOMPLEMENT_H

#include <map>
#include "FormulaTree.h"
//#include "Node.h"
#include "Graph.h"

using namespace std;

class GraphComplement : public Graph {
	private:
		map<int, Node*> addedNodes;
		Node* trap;
		FormulaTree* root;
        Node* getNode(Formula *f);
        Node* getNode_stupid(Formula *f);
        bool formulaFound(Formula *f, Formula *g);
        Node* searchNode(Formula *f, FormulaTree *n);
        void toDot_Nodes(FILE* out) const;
        //	void toDot_addedNodes(FILE* out) const;
        //  void printAddedNodes(ostream& o) const;

    public:

    	///constructor
    	GraphComplement();

    	///destructor
    	~GraphComplement();

    	/// computes the complement for the extended annotated graph
    	void complement();

        /// complete the extended annotated graph - fast algorithm, but unreduced result
        void makeComplete_fast();

        /// complete the extended annotated graph - stupid algorithm (simple reduction, slow)
        void makeComplete_stupid();

        /// complete the extended annotated graph - efficient algorithm (intelligent reduction, fast)
        void makeComplete_efficient();

        /// make the extended annotated graph total
        void makeTotal();

        /// Graphviz dot output
        void toDot(FILE* out, string title = "") const;

        // Graph output as complement (it is required that makeTotal and makeComplete was executed before)
        //void print(ostream& o) const;

        //TODO: diese Funktion private?
        //void getGlobalFormulaForComplement(ostream& o) const;

        /// get the number of new nodes in the complement
        int getSizeOfAddedNodes();

        //TODO: diese Funktion private?
        void generateGlobalFormula();

        void negateGlobalFormula();

        //TODO: diese Funktion private?
        void appandAddedNodes();
};

#endif /* GRAPHCOMPLEMENT_H_ */