
#ifndef GRAPHINTERSECTION_H_
#define GRAPHINTERSECTION_H_

#include "Graph.h"
#include "stack"

using namespace std;

class GraphIntersection : public Graph {
	private:
		list<Intpair> initialNodepairs;
		stack<long> myStack;
		stack<Nodepair*> unproc;

        void printNodepairs(ostream& o) const;
        /// prints the global Formula of the complement (it is required that makeTotal and makeComplete was executed before)

        void printGlobalFormula(ostream& o) const;
        void product(Nodepair* qp);

    public:

        /// the nodes indexed by an identifier
        map<Intpair, Nodepair*, Cmp> nodepairs;

    	///constructor
        GraphIntersection();

    	///destructor
        ~GraphIntersection();

    	/// cumputes the intersection for the given graphs
    	void intersection(Graph *g1, Graph *g2);

        /// Graphviz dot output
        void toDot(FILE* out, string title = "") const;

        /// Graph output
        void print(ostream& o) const;

        void printIntersection(ostream& o) const;
};

#endif /* GRAPHINTERSECTION_H_ */
