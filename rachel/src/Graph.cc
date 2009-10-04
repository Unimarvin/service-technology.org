/*****************************************************************************\
 Rachel -- Repairing Automata for Choreographies by Editing Labels
 
 Copyright (C) 2008, 2009  Niels Lohmann <niels.lohmann@uni-rostock.de>
 
 Rachel is free software; you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation; either version 3 of the License, or (at your option) any later
 version.
 
 Rachel is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along with
 Rachel (see file COPYING); if not, see http://www.gnu.org/licenses or write to
 the Free Software Foundation,Inc., 51 Franklin Street, Fifth
 Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#include <cassert>
#include <set>
#include <fstream>
#include <libgen.h>
#include "config.h"
#include "Graph.h"
#include "Formula.h"
#include "helpers.h"
#include "costfunction.h"

using std::vector;
using std::map;
using std::max;
using std::string;
using std::set;
using std::ofstream;


/**************************************
 * implementation of member functions *
 **************************************/

/**************
 * class Edge *
 **************/

/*!
 * \brief comparison operator: equality
 *
 * Compares edges for equality.
 */
bool Edge::operator == (const Edge &e) const {
    return (source == e.source &&
            target == e.target &&
            label == e.label);
}


/*!
 * \brief comparison operator: less
 *
 * Compares edges using a lexicographical order.
 */
bool Edge::operator < (const Edge &e) const {
    if (label < e.label) {
        return true;
    }

    if (label == e.label && source < e.source) {
        return true;
    }

    if (label == e.label && source == e.source && target < e.target) {
        return true;
    }
    
    return false;
}


/*!
 * \brief constructor
 *
 * Constructs an edge given source, target, and label.
 */
Edge::Edge(Node _source, Node _target, const Label _label) :
    source(_source), target(_target), label(_label), type() {
}


/*!
 * \brief constructor
 *
 * Constructs an empty edge, labeled with "" (epsilon).
 */
Edge::Edge() :
  source(0), target(0), label(""), type() {
}


/***************
 * class Graph *
 ***************/

/// returns outgoing edges of a node (not newly added nodes!)
Edges Graph::outEdges(Node q) {
    Edges result;
    
    for (size_t i = 0; i < edges[q].size(); ++i) {
        if ( !addedNodes[edges[q][i].target] )
            result.push_back(edges[q][i]);
    }
    
    return result;
}


/// returns outgoing send edges of a node (not newly added nodes!)
std::set<Label> Graph::sendLabels(Node q) {
    std::set<Label> result;
    
    for (size_t i = 0; i < edges[q].size(); ++i) {
        if ( !addedNodes[edges[q][i].target] && edges[q][i].label.substr(0,1) == "!" )
            result.insert(edges[q][i].label);
    }    
    
    return result;
}


/// returns outgoing receive edges of a node (not newly added nodes!)
std::set<Label> Graph::receiveLabels(Node q) {
    std::set<Label> result;
    
    for (size_t i = 0; i < edges[q].size(); ++i) {
        if ( !addedNodes[edges[q][i].target] && edges[q][i].label.substr(0,1) == "?" )
            result.insert(edges[q][i].label);
    }    
    
    return result;
}


/// returns (at most 1!) successor of a node with a given label
Edge Graph::successor(Node q, const Label &a) {
    for (size_t i = 0; i < edges[q].size(); ++i) {
        if (edges[q][i].label == a && addedNodes[edges[q][i].source] == false)
            return edges[q][i];
    }
    
    return Edge();
}


/*!
 * \brief add an edge to the graph
 *
 * \param q1   the source node of the edge
 * \param q2   the target node of the edge
 * \param l    the label of the edge
 * \param type the type of the edge if the graph represents the edit distance
 *             (initialized to NONE if none is given)
 */
Edge Graph::addEdge(Node q1, Node q2, const Label l, action_type type) {
    Edge result = Edge(q1,q2,l);
    result.type = type;
    edges[q1].push_back(result);

    return result;
}


/*!
 * \brief add formula to a node
 *
 * \param q  a node
 * \param f  a formula
 *
 * \pre The pointer f is not NULL.
 */
void Graph::addFormula(Node q, Formula *f) {
    assert(f != NULL);    
    formulas[q] = f;
}


/// returns all combinations of edges that satisfy a node's annotation
Assignments Graph::sat(Node q) {
    Assignments result;
    
    // collect labels satisfying the annotation
    vector<Labels> satisfyingLabels = checkSat(q);
    
    // for each label, find the respective edge
    for (size_t i = 0; i < satisfyingLabels.size(); ++i) {
        Assignment temp;
        for (size_t j = 0; j < satisfyingLabels[i].size(); ++j) {
            temp.push_back(successor(q, satisfyingLabels[i][j]));
        }
        result.push_back(temp);
    }
    
    return result;    
}


/*!
 * \brief constructor
 *
 * Constructs a graph with a given identifyer.
 *
 * \param _id  identifier of the graph
 *
 */
Graph::Graph(const std::string& _id) :
    edges(), root(), finalNode(), max_value(0), insertionValue(),
    deletionValue(), formulas(), formulaBits(), id(_id),  nodes(),
    addedNodes() {
}


void Graph::createDotFile(bool reduced) {
    string dot_filename;

    // if no filename is given via command line, create it
    if (args_info.dot_arg == NULL) {
        switch (args_info.mode_arg) {
            case(mode_arg_matching):
            case(mode_arg_simulation): {
                dot_filename = std::string(basename(args_info.automaton_arg)) + "_" +
                    std::string(basename(args_info.og_arg)) + "_" +
                    cmdline_parser_mode_values[args_info.mode_arg] + ".dot";
                break;
            }
            
            case (mode_arg_og): {
                dot_filename = std::string(basename(args_info.og_arg)) + ".dot";
                break;
            }
            
            case (mode_arg_sa): {
                dot_filename = std::string(basename(args_info.automaton_arg)) + ".dot";
                break;
            }
        
            case (mode_arg_annotation): {
                if (!reduced)
                    dot_filename = std::string(basename(args_info.og_arg)) + ".bits1.dot";
                else
                    dot_filename = std::string(basename(args_info.og_arg)) + ".bits2.dot";                    
                break;
            }
        }
    } else {
        dot_filename = args_info.dot_arg;
    }


    // try to open the dot file for writing
    ofstream dot_file;
    dot_file.open(dot_filename.c_str());
    if (!dot_file) {
        fprintf(stderr, "could not create file '%s'\n", dot_filename.c_str());
        exit (EXIT_FAILURE);
    }


    // write header
    dot_file << "digraph G {\n";
    dot_file << "edge [fontname=Helvetica fontsize=10]\n";
    dot_file << "node [fontname=Helvetica fontsize=10]\n";
    
    
    // write dot graph according to chosen mode
    switch (args_info.mode_arg) {            
        case (mode_arg_og):
        case (mode_arg_matching):
        case (mode_arg_simulation): {
            dot_file << toDot() << "\n";
            break;
        }

        case (mode_arg_annotation): {
            dot_file << toDotAnnotated(reduced) << "\n";
            break;
        }
        
        case (mode_arg_sa): {
            dot_file << toDot(false) << "\n";
            break;
        }
    }
    
    
    // write footer and close file
    dot_file << "}\n";
    dot_file.close();
    
    
    // if dot found during configuration, executed it to create a PNG
    if (args_info.png_flag && !std::string(CONFIG_DOT).empty()) {
        std::string command = std::string(CONFIG_DOT) + " " + dot_filename + " -Tpng -O";
        system(command.c_str());
    }
}


/// returns a Dot representation of the graph
string Graph::toDot(bool drawFormulae) {
    string result;

    // an arrow indicating the initial state
    result += "  q0 [label=\"\" height=\"0.01\" width=\"0.01\" style=\"invis\"];\n";
    result += "  q0 -> q_" + toString(nodes[root]) + " [minlen=\"0.5\"];\n";

    for (size_t i = 0; i != nodes.size(); ++i) {
        result += "  q_" + toString(nodes[i]) + " [label=";

        if (drawFormulae && formulas[nodes[i]] != NULL) {
            result += "<<FONT>";
            result += formulas[nodes[i]]->toDot();
            result += "</FONT>>";
        } else {
            result += "\"\"";
        }
                
        if (addedNodes[nodes[i]])
            result += " style=dashed";
        
        if (finalNode[nodes[i]] && !drawFormulae) {
            result += " peripheries=\"2\"";
        }
        
        result += "];\n";

        for (size_t j = 0; j < edges[nodes[i]].size(); ++j) {
            result += "  q_" + toString(edges[nodes[i]][j].source);
            result += " -> q_" + toString(edges[nodes[i]][j].target);
            result += " [label=\"" + edges[nodes[i]][j].label + "\"";

            if (addedNodes[edges[nodes[i]][j].source] || addedNodes[edges[nodes[i]][j].target])
                result += " style=dashed";

            switch (edges[nodes[i]][j].type) {
                case(DELETE): result += " color=red"; break;
                case(INSERT): result += " color=green"; break;
                case(MODIFY): result += " color=orange"; break;
                case(NONE):
                case(KEEP):
                default: break;
            }
            
            result += "];";
        }
    }
    
    return result;
}


/// returns a Dot representation of the graph
string Graph::toDotAnnotated(bool reduced) {
    string result;

    // an arrow indicating the initial state
    result += "  q0 [label=\"\" height=\"0.01\" width=\"0.01\" style=\"invis\"];\n";
    result += "  q0 -> q_" + toString(nodes[root]) + " [minlen=\"0.5\"];\n";

    for (size_t i = 0; i != nodes.size(); ++i) {
        result += "  q_" + toString(nodes[i]) + "[label=\"\"";
        
        // draw node
        if (!reduced) {
            if (formulaBits[i].S)
                result += " style=filled, fillcolor=skyblue, label=\"S\"";
            else if (formulaBits[i].F)
                result += " style=filled, fillcolor=darkolivegreen2, label=\"F\"";
        } else {
            if (formulaBits[i].S_1)
                result += " style=filled, fillcolor=skyblue, label=\"S1\"";
            else if (formulaBits[i].S_2)
                result += " style=filled, fillcolor=purple, label=\"S2\"";
            else if (formulaBits[i].F_prime)
                result += " style=filled, fillcolor=darkolivegreen2, label=\"F'\"";
        }
        
        result += "];\n";

        // outgoing edges
        for (size_t j = 0; j < edges[nodes[i]].size(); ++j) {
            result += "  q_" + toString(edges[nodes[i]][j].source);
            result += " -> q_" + toString(edges[nodes[i]][j].target);
            result += " [label=\"" + edges[nodes[i]][j].label + "\"];\n";
        }
    }
    
    return result;
}


/// returns all satisfying label vectors
vector<Labels> Graph::checkSat(Node q) {
    vector<Labels> result;
    
    vector<unsigned int> index;
    vector<unsigned int> bounds;
    unsigned int count = 1;
    unsigned int degree = edges[q].size();
    for (size_t i = 0; i < degree; ++i) {
        index.push_back(0);
        bounds.push_back(2);
        count *= 2;
    }
    
    for (size_t j = 0; j < count; ++j) {
        set<string> tempSet;
        Labels tempLabels;
        for (size_t i = 0; i < degree; ++i) {
            if (index[i] != 0) {
                tempSet.insert(edges[q][i].label);
                tempLabels.push_back(edges[q][i].label);
            }
        }
        
        if (formulas[q] != NULL) {
            if (formulas[q]->sat(tempSet)) {
                result.push_back(tempLabels);
            }
        }
        
        // update the indices
        next_index(index, bounds);
    }
    
    return result;
}


/// returns true iff the node's annotation is fulfilled by "final"
bool Graph::isFinal(Node q) {
    set<Label> test;
    test.insert("FINAL");

    if (formulas[q] != NULL) {
        if (formulas[q]->sat(test))
            return true;
    }
    
    return false;
}


/// gets the root node
Node Graph::getRoot() const {
    return root;
}


/// sets the root node
void Graph::setRoot(Node q) {
    root = q;
}


/// adds this node to the final nodes (IG only)
void Graph::setFinal(Node q) {
    finalNode[q] = true;
}


/// add a node to the graph
Node Graph::addNode(Node q) {
    // if node is present, return it
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i] == q)
            return q;
    }
    
    this->max_value = max(q, max_value);
    this->nodes.push_back(q);
    this->addedNodes[q] = false;
    
    return q;
}


/*!
 * \brief add a new node and connect it with a new edge
 *
 * \note If the node to add is already present (either already inserted or
 *       present from the start) that node is returned instead.
 *
 * \todo Remove this function: adding of nodes is not necessary to calculate
 *       simulation or matching.
 */
Node Graph::addNewNode(Node q, const Label l) {
    // if node was already added here, return it
    for (size_t i = 0; i < edges[q].size(); ++i) {
        if ( edges[q][i].label == l ) //&& addedNodes[ edges[q][i].target ] )
            return edges[q][i].target;
    }
    
    // add a new node and a corresponding edge
    Node new_q = this->max_value + 1;
    addNode(new_q);
    addedNodes[new_q] = true;
    addEdge(q, new_q, l);
    
    return new_q;
}


/// preprocess the graph with deletion values (helper)
Value Graph::preprocessDeletionRecursively(Node q) {
    if (deletionValue[q] != 0)
        return deletionValue[q];

    if (edges[q].empty()) {
        deletionValue[q] = 1;
        return 1;
    }
    
    Value result = 0;

    // iterate the successor nodes and collect their preprocess value
    for (unsigned i  = 0; i < edges[q].size(); ++i) {
        result += L(edges[q][i].label, "") * preprocessDeletionRecursively(edges[q][i].target);
    }
    
    return result;
}


/// preprocess the graph with insertion values (helper)
Value Graph::preprocessInsertionRecursively(Node q) {
    if (insertionValue[q] != 0)
        return insertionValue[q];
    
    // If the node's formula can be satisfied with "FINAL", any subsequent
    // addition of nodes would be sub-optimal.
    // What about nodes without successors?
    if (isFinal(q)) {
        insertionValue[q] = 1;
        return 1;
    }    
    
    Value bestValue = 0;
    
    // get and iterate the satisfying assignments
    Assignments assignments = sat(q);
    for (size_t k = 0; k < assignments.size(); ++k) {
        
        Assignment beta = assignments[k];
        
        Value newValue = 0;
        
        // iterate the successor nodes and collect their preprocess value
        for (unsigned i  = 0; i < beta.size(); ++i) {
            newValue += L("", beta[i].label) * preprocessInsertionRecursively(beta[i].target);
        }
        
        // divide by size of the assignment and discount
        newValue *= (discount() / beta.size());       
        
        bestValue = max(bestValue, newValue);
    }
    
    bestValue += (1-discount());    
    insertionValue[q] = bestValue;
    
    return bestValue;
}


/// preprocess the graph with deletion values (for service automata)
void Graph::preprocessDeletion() {
    preprocessDeletionRecursively(root);
}


/// preprocess the graph with insertion values (for OGs)
void Graph::preprocessInsertion() {
    preprocessInsertionRecursively(root);
}


/// return deletion value (for service automata)
Value Graph::getDeletionValue(Node q) {
    return deletionValue[q];
}


/// return insertion value (for OGs)
Value Graph::getInsertionValue(Node q) {
    return insertionValue[q];
}


/*!
 * \brief calculate the number of characterized services (helper)
 *
 * The algorithm traverses the satisfying assignments of the given node's
 * formula and recursively checks the successor nodes. A mapping implements a
 * cache for dynammic programming to avoid repetitive calculations of already
 * seen nodes.
 *
 * \param q  a node
 *
 * \return   the number of services represented by the subgraph starting here
 */
long double Graph::countServicesRecursively(Node q) {
    // a cache for dynamic programming
    static map<Node, long double> cache;
    if (cache[q] != 0)
        return cache[q];
    
    Assignments assignments = sat(q);
    
    // the number of services represented by the subgraph starting here
    long double result = 0;
    
    // traverse the satisfying assignments of the node's formula
    for (size_t k = 0; k < assignments.size(); ++k) {
        Assignment beta = assignments[k];

        // the number of services represented by this assignment
        long double temp = 1;

        // multiply all recursively reachable services
        for (size_t i = 0; i < beta.size(); ++i) {
            temp *= countServicesRecursively(beta[i].target);
        }

        // add up the numbers of each satisfying assignment        
        result += temp;
    }

    // in case no assignment was found, this node would be red!
    assert (result != 0);
    
    cache[q] = result;
    return result;
}


/*!
 * \brief calculate the number of characterized services
 *
 * Calculates the number of the acyclic deterministic services represented by
 * the graph (up to tree isomorphism).
 *
 * \return the number of characterized services or 'inf' if this number is
 *         greater than 10**4932
 */
long double Graph::countServices() {
    return countServicesRecursively(root);
}


/// check if graph is cyclic (helper)
bool Graph::isCyclicRecursively(Node q) {
    // call stack to detect cyclic calls
    static vector<Node> call_stack;
    
    // the result so far
    static bool result = false;
    
    if (result)
        return true;
    
    // check if this combination is already on the call stack
    for (size_t i = 0; i < call_stack.size(); ++i) {
        if (call_stack[i] == q) {
            result = true;
            return true;
        }
    }
    
    call_stack.push_back(q);

    for (size_t i = 0; i < edges[q].size(); ++i) {
        result = result || isCyclicRecursively(edges[q][i].target);
    }
    
    call_stack.pop_back();

    return result;
}


/// check if graph is cyclic
bool Graph::isCyclic() {
    return isCyclicRecursively(root);
}


/// coherently re-enumerate the graph's nodes
void Graph::reenumerate() {
    map<Node, Node> translation1;
    map<Node, Node> translation2;
    std::map<Node, Edges> new_edges;
    std::map<Node, Formula*> new_formulas;
    Nodes new_nodes;
    
    addedNodes.clear();

    // re-indexing nodes
    for (size_t i = 0; i < nodes.size(); ++i) {
        translation1[i] = nodes[i];
        translation2[nodes[i]] = i;
        new_formulas[i] = formulas[nodes[i]];
        addedNodes[i] = false;
        new_nodes.push_back(i);
    }
    
    // using the new index for the edges
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < edges[nodes[i]].size(); ++j) {
            Edge e;
            e.source = translation2[ edges[nodes[i]][j].source ];
            e.target = translation2[ edges[nodes[i]][j].target ];
            e.label = edges[nodes[i]][j].label;
            new_edges[i].push_back(e);
        }
    }
    
    root = translation2[root];
    max_value = nodes.size()-1;
    
    nodes = new_nodes;
    edges = new_edges;
    formulas = new_formulas;
}


/// return the average size of the satisfying assignments
double Graph::averageSatSize() {
    double result = 0;
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        result += sat(nodes[i]).size();
    }
    
    return (result / static_cast<double>(nodes.size()));
}


/// calculate a compact represenation of the OG's formulae
void Graph::calculateCompactAnnotations() {
    unsigned int F_count = 0;
    unsigned int F_prime_count = 0;
    unsigned int S_count = 0;
    unsigned int S1_count = 0;
    unsigned int S2_count = 0;
    
    // traverse the OG's nodes
    for (size_t i = 0; i < nodes.size(); ++i) {
        bool in_S = false;
        bool in_F = false;
        bool in_S1 = false;
        bool in_S2 = false;
        bool in_FP = false;
        
        // STEP 1: treat the "final" assignment
        // if "final" satisfies the formula, this node belongs to F
        if (formulas[nodes[i]]->hasFinal()) {
            F_count++;
            in_F = true;
            if (args_info.verbose_flag)
                fprintf(stderr, "node %d is in F\n", nodes[i]);
            
            // if the node has no outgoing edges, its implicitly in F
            if (!edges[nodes[i]].empty()) {
                F_prime_count++;
                in_FP = true;
                if (args_info.verbose_flag)
                    fprintf(stderr, "node %d is in F'\n", nodes[i]);
            }
        }
        

        // STEP 2: check if annotation can only be fulfilled by sending
        // (i.e., not by receiving or final)
        if (formulas[nodes[i]] != NULL) {
            if (!formulas[nodes[i]]->sat(receiveLabels(nodes[i]))) {
                S_count++;
                in_S = true;
                if (args_info.verbose_flag)
                    fprintf(stderr, "node %d is in S\n", nodes[i]);
            }
        }

        // check for an alternative representation of S
        // additional check?: !edges[nodes[i]].empty()
        if (receiveLabels(nodes[i]).empty()) {
            if (!in_S && !in_F) {
                S2_count++;
                in_S2 = true;
                if (args_info.verbose_flag)
                    fprintf(stderr, "node %d is in S2\n", nodes[i]);
            }
        } else {
            if (in_S) {
                S1_count++;
                in_S1 = true;
                if (args_info.verbose_flag)
                    fprintf(stderr, "node %d is in S1\n", nodes[i]);
            }
        }
        
        // store the bit annotation
        formulaBits[i] = FormulaBits(in_S, in_F, in_S1, in_S2, in_FP);
    }
        
    if (args_info.verbose_flag)
        fprintf(stderr, "\n");

    fprintf(stderr, "# nodes with final annotation:     %5d = |F|\n", F_count);
    fprintf(stderr, "  ~ without successor:             %5d\n", F_count - F_prime_count);
    fprintf(stderr, "  ~ with successor:                %5d = |F'|\n", F_prime_count);
    fprintf(stderr, "  ~ effect of implicit storage:    %5.2f %%\n",
        (double(F_prime_count) / double(F_count)) * 100.0);
    fprintf(stderr, "# nodes with must send annotation: %5d = |S|\n", S_count);
    fprintf(stderr, "  ~ with ?-successor:              %5d = |S1|\n", S1_count);
    fprintf(stderr, "  * without ?-successor:           %5d = |S2|\n", S2_count);
    fprintf(stderr, "  ~ effect of implicit storage:    %5.2f %%\n",
        (double(S1_count + S2_count) / double(S_count)) * 100.0);
    fprintf(stderr, "# nodes of OG:                     %5lu\n", (unsigned long)nodes.size());
    fprintf(stderr, "  ~ with set bits (explicit)       %5d (%6.2f %%)\n",
        F_count + S_count, ((double(F_count + S_count) / double(nodes.size())) * 100.0) );
    fprintf(stderr, "  ~ with set bits (implicit)       %5d (%6.2f %%)\n",
        F_prime_count + S1_count + S2_count,
        ((double(F_prime_count + S1_count + S2_count) / double(nodes.size())) * 100.0) );    
}


/// BPMN output of a service automaton
/// \bug: if two edges [x,a,y] and [x,b,y] exists, only the first one is handled
/// \bug: strict termination has to be enforced
void Graph::bpmnOutput() {
    // BPMN shapes
    double scale = 0.6;
    string shape_xor = "shape=\"diamond\" label=\"\" regular=\"true\" height=\"" + toString(scale+0.1) + "\" image=\"" + string(args_info.shapedir_arg) + "xor.png\"";
    string shape_mul = "shape=\"diamond\" label=\"\" regular=\"true\" height=\"" + toString(scale+0.1) + "\" image=\"" + string(args_info.shapedir_arg) + "multiple.png\"";
    string shape_mix = "shape=\"diamond\" label=\"\" regular=\"true\" height=\"" + toString(scale+0.1) + "\"";
    string shape_start = "shape=\"circle\" label=\"\" height=\"" + toString(scale) + "\"";
    string shape_stop = "shape=\"circle\" label=\"\" penwidth=\"4\" height=\"" + toString(scale) + "\"";
    string shape_send = "shape=\"circle\" peripheries=\"2\" height=\"" + toString(scale-0.1) + "\" image=\"" + string(args_info.shapedir_arg) + "send.png\"";
    string shape_receive = "shape=\"circle\" peripheries=\"2\" height=\"" + toString(scale-0.1) + "\" image=\"" + string(args_info.shapedir_arg) + "receive.png\"";    
    string shape_time = "shape=\"circle\" label=\"\" peripheries=\"2\" height=\"" + toString(scale-0.1) + "\" image=\"" + string(args_info.shapedir_arg) + "time.png\"";
    
    // dot header
    fprintf(stdout, "digraph G {\n");
    fprintf(stdout, "  graph [rankdir=\"LR\"];\n");
    fprintf(stdout, "  node [fixedsize=\"true\", fontname=\"Helvetica\" fontsize=\"10\"];\n");
    fprintf(stdout, "  edge [fontname=\"Helvetica\" fontsize=\"10\"];\n\n");

    // start node
    fprintf(stdout, "  n_%d_START [%s];\n\n", root, shape_start.c_str());

    fprintf(stdout, "  subgraph cluster0 {\n    style=invis\n");
    
    // traverse the SA's nodes and collect predecessors
    std::map<Node, Nodes> pred;
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < edges[i].size(); ++j) {
            pred[edges[i][j].target].push_back(edges[i][j].source);
            
            // create events
            if (edges[i][j].label.substr(0,1) == "!") {
                fprintf(stdout, "    n_%d_%d [%s label=\"\\n\\n\\n\\n\\n%s\"];\n",
                    edges[i][j].source, edges[i][j].target,
                    shape_send.c_str(), edges[i][j].label.c_str());
            } else {
                fprintf(stdout, "    n_%d_%d [%s label=\"\\n\\n\\n\\n\\n%s\"];\n",
                    edges[i][j].source, edges[i][j].target,
                    shape_receive.c_str(), edges[i][j].label.c_str());
            }
        }        
    }
    
    std::map<Node, string> in;
    std::map<Node, string> out;
    for (size_t i = 0; i < nodes.size(); ++i) {

        // start node
        if (i == root) {
            // if this node has incoming edges, add an XOR join
            if (pred[i].size() > 0) {
                fprintf(stdout, "    n_%lu_JOIN [%s];\n", (unsigned long)i, shape_xor.c_str());
                fprintf(stdout, "    n_%lu_START -> n_%lu_JOIN;\n", (unsigned long)i, (unsigned long)i);
                in[i] = "n_" + toString(i) + "_JOIN"; }
            else {
                in[i] = "n_" + toString(i) + "_START";
            }
        }
        
        // more than one incoming edge: join
        if (pred[i].size() > 1) {
            fprintf(stdout, "    n_%lu_JOIN [%s];\n", (unsigned long)i, shape_xor.c_str());
            in[i] = "n_" + toString(i) + "_JOIN";                
        }

        // end node
        if (finalNode[i]) {
            fprintf(stdout, "    n_%lu_END [%s];\n", (unsigned long)i, shape_stop.c_str());

            // if this node has outgoing edges, add an XOR (or maybe something else) split
            if (edges[i].size() > 0) {
                fprintf(stdout, "    n_%lu_SPLIT [%s];\n", (unsigned long)i, shape_mix.c_str());
                fprintf(stdout, "    n_%lu_SPLIT -> n_%lu_END;\n", (unsigned long)i, (unsigned long)i);
                out[i] = "n_" + toString(i) + "_SPLIT";
            } else {
                out[i] = "n_" + toString(i) + "_END";
            }
        }

        // more than one outgoing edge: split
        if (edges[i].size() > 1) {

            if (!receiveLabels(i).empty() && !sendLabels(i).empty()) {
                fprintf(stderr, "%lu is a mixed split -- CANNOT HANDLE THIS YET!\n", (unsigned long)i);
                fprintf(stdout, "    n_%lu_SPLIT [%s];\n", (unsigned long)i, shape_mix.c_str());
                out[i] = "n_" + toString(i) + "_SPLIT";
            }
            else if (receiveLabels(i).empty()) {
                fprintf(stdout, "    n_%lu_SPLIT [%s];\n", (unsigned long)i, shape_xor.c_str());
                out[i] = "n_" + toString(i) + "_SPLIT";
            }
            else if (sendLabels(i).empty()) {
                fprintf(stdout, "    n_%lu_SPLIT [%s];\n", (unsigned long)i, shape_mul.c_str());
                out[i] = "n_" + toString(i) + "_SPLIT";
            }            
        }
    }

    fprintf(stdout, "  }\n\n");

    // add the control flow edges to the BPMN model
    for (size_t i = 0; i < nodes.size(); ++i) {

/*        
        string edge_source = (in[i] == "") ?
            "n_" + toString(pred[i][0]) + "_" + toString(i) :
            in[i];

        string edge_target = (out[i] == "") ?
            "n_" + toString(i) + "_" + toString(edges[i][0].target) :
            out[i];
        
        fprintf(stdout, "  %s -> %s;\n", edge_source.c_str(), edge_target.c_str());
*/
        
        // normal -> normal
        if (in[i] == "" && out[i] == "") {
            fprintf(stdout, "  n_%d_%lu -> n_%lu_%d;\n",
                pred[i][0], (unsigned long)i, (unsigned long)i, edges[i][0].target);
        }

        // normal -> !normal
        if (in[i] == "" && out[i] != "") {
            fprintf(stdout, "  n_%d_%lu -> %s;\n",
                pred[i][0], (unsigned long)i, out[i].c_str());            
        }

        // !normal -> normal
        if (in[i] != "" && out[i] == "") {
            fprintf(stdout, "  %s -> n_%lu_%d;\n",
                in[i].c_str(), (unsigned long)i, edges[i][0].target);            
        }
        
        // !normal -> !normal
        if (in[i] != "" && out[i] != "") {
            fprintf(stdout, "  %s -> %s;\n",
                in[i].c_str(), out[i].c_str());            
        }
        
        // egdes from or to added nodes
        for (size_t j = 0; j < edges[i].size(); ++j) {
            // output of event is not normal (join or exit)
            if (in[edges[i][j].target] != "") {
                fprintf(stdout, "  n_%d_%d -> %s;\n",
                    edges[i][j].source, edges[i][j].target,
                    in[edges[i][j].target].c_str());
            }
            
            // input of event is not normal (split or standard)
            if (out[edges[i][j].source] != "") {
                fprintf(stdout, "  %s -> n_%d_%d;\n",
                    out[edges[i][j].source].c_str(),
                    edges[i][j].source, edges[i][j].target);
            }
        }
    }
    
    fprintf(stdout, "}\n");
}

