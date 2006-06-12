#include "graph.h"
#include "vertex.h"
#include "successorNodeList.h"
//#include "reachGraph.h"
//#include "graphEdge.h"
//#include "owfn.h"
//#include "stateList.h"
//#include "main.h"

//#include "enums.h"

#include "options.h"
#include "debug.h"

//#include <fstream>
//#include <iostream>
//#include <string>
//#include <set>


using namespace std;

//! \fn reachGraph::reachGraph(oWFN * _PN)
//! \param _PN
//! \brief constructor
reachGraph::reachGraph(oWFN * _PN) :
	numberOfVertices(0),
	numberOfEdges(0),
	actualDepth(0),
	numberBlueNodes(0),
	numberBlueEdges(0),
	numberOfStatesAllNodes(0) {
    
    PN = _PN;

    // get the root node
    calculateRootNode();
}

//! \fn reachGraph::~reachGraph()
//! \brief destructor !to be implemented!
reachGraph::~reachGraph() {
}

//! \fn vertex * reachGraph::getRoot() const
//! \return pointer to root
//! \brief returns a pointer to the root node of the graph
vertex * reachGraph::getRoot() const {
    return root;
}

//! \fn unsigned int reachGraph::NumberOfEdges () const
//! \return number of edges
//! \brief returns the number of edges of the graph
unsigned int reachGraph::getNumberOfEdges () const {
    return numberOfEdges;
}

//! \fn unsigned int reachGraph::NumberOfVertices () const
//! \return number of vertices
//! \brief returns the number of vertices of the graph
unsigned int reachGraph::getNumberOfVertices () const {
    return numberOfVertices;
}

//! \fn void reachGraph::calculateRootNode()
//! \brief calculates the root node of the graph
void reachGraph::calculateRootNode() {
    trace(TRACE_5, "void reachGraph::calculateRootNode(): start\n");

    // initialize graph => calcualte root node
    stateList * list = new stateList();
    vertex * v = new vertex(PN->placeInputCnt + PN->placeOutputCnt);

    // calc the reachable states from that marking
    if (parameters[P_CALC_ALL_STATES]) {
        PN->calculateReachableStatesFull(list, true);
    } else {
        PN->calculateReachableStates(list, true);
    }

    v->setStateList(list);

    root = v;
    numberOfVertices++;
    currentVertex = root;

   	numberOfStatesAllNodes += root->getStateList()->elementCount();

    trace(TRACE_5, "void reachGraph::calculateRootNode(): end\n");
}


//! \fn vertex * reachGraph::findVertexInSet(vertex * toAdd)
//! \param toAdd the vertex we are looking for in the graph
//! \brief this function uses the find method from the template set
vertex * reachGraph::findVertexInSet(vertex * toAdd) {
    vertexSet::iterator iter = setOfVertices.find(toAdd);
    if (iter != setOfVertices.end()) {
        return *iter;
    } else {
        return NULL;
    }
}


//! \fn int reachGraph::AddVertex (vertex * toAdd, unsigned int label, edgeType type)
//! \param toAdd a reference to the vertex that is to be added to the graph
//! \param label the label of the edge between the current vertex and the one to be added
//! \param type the type of the edge (sending, receiving)
//! \brief adding a new vertex/ edge to the graph
//!
//! if the graph already contains nodes, we first search the graph for a node that matches the new node
//! if we did not find a node, we add the new node to the graph (here we add the new node to the
//! successor node list of the current graph and add the current node to the list of predecessor nodes
//! of the new node; after that the current vertex becomes to be the new node)
//!
//! if we actually found a node matching the new one, we just create an edge between the current node
//! and the node we have just found, the found one gets the current node as a predecessor node

// for OG
int reachGraph::AddVertex (vertex * toAdd, unsigned int label, edgeType type) {

	int offset = 0;

    if (numberOfVertices == 0) {                // graph contains no nodes at all
        root = toAdd;                           // the given node becomes the root node
        currentVertex = root;
        numberOfVertices++;
        setOfVertices.insert(toAdd);
    } else {
        vertex * found = findVertexInSet(toAdd);
        char * edgeLabel;

        if (type == sending) {
            edgeLabel = PN->Places[PN->inputPlacesArray[label]]->name;
        } else {
            edgeLabel = PN->Places[PN->outputPlacesArray[label]]->name;
        }

        if (found == NULL) {
//            cout << "with event " << label << " (type " << type << " ):" << endl;
            trace(TRACE_1, "\n\t new successor node computed:");
            toAdd->setNumber(numberOfVertices++);

            graphEdge * edgeSucc = new graphEdge(toAdd, edgeLabel, type);
            currentVertex->addSuccessorNode(edgeSucc);

            reachGraphStateSet::iterator iter;		// iterator over the stateList's elements

            for (iter = currentVertex->getStateList()->setOfReachGraphStates.begin(); iter != currentVertex->getStateList()->setOfReachGraphStates.end(); iter++) {
                if ((*iter)->state->type == DEADLOCK || (*iter)->state->type == FINALSTATE) {
                    (*iter)->setEdge(edgeSucc);
                }
            }

//            graphEdge * edgePred = new graphEdge(currentVertex, edgeLabel, type);
//            toAdd->addPredecessorNode(edgePred);

            for (int i = 0; i < (PN->placeInputCnt + PN->placeOutputCnt); i++) {
                toAdd->eventsUsed[i] = currentVertex->eventsUsed[i];
            }

            if (type == receiving) {
                offset = PN->placeInputCnt;
            }

            toAdd->eventsUsed[offset + label]++;

			// \begin{hack} for online shop example
/*            if (type == sending) {
                if (strcmp(PN->Places[PN->inputPlacesArray[label]]->name, "in.client.onlineshop.abort") == 0) {
                    for (int i = 0; i < (PN->placeInputCnt + PN->placeOutputCnt); i++) {
                        toAdd->eventsUsed[i]++;
                    }
                }
            }
*/			// \end{hack}

            currentVertex = toAdd;
            numberOfEdges++;

            setOfVertices.insert(toAdd);

           	numberOfStatesAllNodes += toAdd->getStateList()->elementCount();

            return 1;
        } else {
            trace(TRACE_1, "\t computed successor node already known: " + intToString(found->getNumber()) + "\n");

            graphEdge * edgeSucc = new graphEdge(found, edgeLabel, type);
            currentVertex->addSuccessorNode(edgeSucc);

            reachGraphStateSet::iterator iter;        // iterator over the stateList's elements

            for (iter = currentVertex->getStateList()->setOfReachGraphStates.begin(); iter != currentVertex->getStateList()->setOfReachGraphStates.end(); iter++) {
                if ((*iter)->state->type == DEADLOCK || (*iter)->state->type == FINALSTATE) {
                    (*iter)->setEdge(edgeSucc);
                }
            }

//            graphEdge * edgePred = new graphEdge(currentVertex, edgeLabel, type);
//            found->addPredecessorNode(edgePred);
            numberOfEdges++;

			if (type == receiving) {
                offset = PN->placeInputCnt;
            }

            found->eventsUsed[offset + label]++;

			// \begin{hack} for online shop example
/*            if (type == sending) {
                if (strcmp(PN->Places[PN->inputPlacesArray[label]]->name, "in.client.onlineshop.abort") == 0) {
                    for (int i = 0; i < (PN->placeInputCnt + PN->placeOutputCnt); i++) {
                         found->eventsUsed[i]++;
                    }
                }
            }
*/            // \end{hack}

            delete toAdd;
            return 0;
        }
    }
}


// for IG
int reachGraph::AddVertex (vertex * toAdd, messageMultiSet messages, edgeType type) {
	trace(TRACE_5, "reachGraph::AddVertex (vertex * toAdd, messageMultiSet messages, edgeType type) : start\n");

    if (numberOfVertices == 0) {                // graph contains no nodes at all
        root = toAdd;                           // the given node becomes the root node
        currentVertex = root;
        numberOfVertices++;

        setOfVertices.insert(toAdd);
    } else {
        vertex * found = findVertexInSet(toAdd); //findVertex(toAdd);

        char * label = new char[256];
        char * actualMessage;
        bool comma = false;

        strcpy(label, "");

        for (messageMultiSet::iterator iter = messages.begin(); iter != messages.end(); iter++) {
            if (comma) {
                strcat(label, ", ");
            }
            actualMessage = new char[strlen(PN->Places[*iter]->name)];
            strcpy(actualMessage, PN->Places[*iter]->name);
            strcat(label, actualMessage);
            comma = true;
        }

        if (found == NULL) {

//            cout << "with event " << label << " (type " << type << " ):" << endl;
            trace(TRACE_1, "\n\t new successor node computed:");

            toAdd->setNumber(numberOfVertices++);

            graphEdge * edgeSucc = new graphEdge(toAdd, label, type);
            currentVertex->addSuccessorNode(edgeSucc);

            reachGraphStateSet::iterator iter;                      // iterator over the stateList's elements

            for (iter = currentVertex->getStateList()->setOfReachGraphStates.begin(); 
            			iter != currentVertex->getStateList()->setOfReachGraphStates.end(); 
            			iter++) {
            				
                if ((*iter)->state->type == DEADLOCK || (*iter)->state->type == FINALSTATE) {
                    (*iter)->setEdge(edgeSucc);
                }
            }
//            graphEdge * edgePred = new graphEdge(currentVertex, label, type);
//            toAdd->addPredecessorNode(edgePred);

cout << "added pred" << endl;

            currentVertex = toAdd;
            numberOfEdges++;

            setOfVertices.insert(toAdd);

           	numberOfStatesAllNodes += toAdd->getStateList()->elementCount();

			trace(TRACE_5, "reachGraph::AddVertex (vertex * toAdd, messageMultiSet messages, edgeType type) : end\n");
           	
            return 1;
        } else {
            trace(TRACE_1, "\t successor node already known: " + intToString(found->getNumber()) + "\n");

            graphEdge * edgeSucc = new graphEdge(found, label, type);
            currentVertex->addSuccessorNode(edgeSucc);

            reachGraphStateSet::iterator iter;		// iterator over the stateList's elements

            for (iter = currentVertex->getStateList()->setOfReachGraphStates.begin(); iter != currentVertex->getStateList()->setOfReachGraphStates.end(); iter++) {
                if ((*iter)->state->type == DEADLOCK || (*iter)->state->type == FINALSTATE) {
                    (*iter)->setEdge(edgeSucc);
                }
            }

//            graphEdge * edgePred = new graphEdge(currentVertex, label, type);
//            found->addPredecessorNode(edgePred);
            numberOfEdges++;

            delete toAdd;

			trace(TRACE_5, "reachGraph::AddVertex (vertex * toAdd, messageMultiSet messages, edgeType type) : end\n");

            return 0;
        }
    }
}


//! \fn stateList * reachGraph::calculateSuccStatesInput(unsigned int input, vertex * node)
//! \param input set of input messages
//! \param node the node for which the successor states are to be calculated
//! \brief calculates the set of successor states in case of an input message
stateList * reachGraph::calculateSuccStatesInput(unsigned int input, vertex * node) {

	trace(TRACE_5, "reachGraph::calculateSuccStatesInput(unsigned int input, vertex * node) : start\n");
	stateList * newStateList = new stateList();		// the new list of states for the next node

	reachGraphStateSet::iterator iter;				// iterator over the stateList's elements

	for (iter = node->getStateList()->setOfReachGraphStates.begin();
		 iter != node->getStateList()->setOfReachGraphStates.end(); iter++) {

		PN->setCurrentMarkingFromState((*iter));	// set the net to the marking of the state being considered
		PN->addInputMessage(input);					// add the input message to the current marking
        if (parameters[P_CALC_ALL_STATES]) {
            PN->calculateReachableStatesFull(newStateList, (*iter)->isMinimal());	// calc the reachable states from that marking
        } else {
            PN->calculateReachableStates(newStateList, (*iter)->isMinimal());		// calc the reachable states from that marking
        }
    }
	trace(TRACE_5, "reachGraph::calculateSuccStatesInput(unsigned int input, vertex * node) : end\n");
	return newStateList;							// return the new state list
}


//! \fn stateList * reachGraph::calculateSuccStatesInput(messageMultiSet input, vertex * node)
//! \param input set of input messages
//! \param node the node for which the successor states are to be calculated
//! \brief calculates the set of successor states in case of an input message
stateList * reachGraph::calculateSuccStatesInput(messageMultiSet input, vertex * node) {
	trace(TRACE_5, "reachGraph::calculateSuccStatesInput(messageMultiSet input, vertex * node) : start\n");

	stateList * newStateList = new stateList();		// the new list of states for the next node

    reachGraphStateSet::iterator iter;				// iterator over the stateList's elements

    for (iter = node->getStateList()->setOfReachGraphStates.begin(); iter != node->getStateList()->setOfReachGraphStates.end(); iter++) {
		PN->setCurrentMarkingFromState((*iter));	// set the net to the marking of the state being considered
        PN->addInputMessage(input);					// add the input message to the current marking
        if (parameters[P_CALC_ALL_STATES]) {
            PN->calculateReachableStatesFull(newStateList, (*iter)->isMinimal());	// calc the reachable states from that marking
        } else {
            PN->calculateReachableStates(newStateList, (*iter)->isMinimal());		// calc the reachable states from that marking
        }
    }
	trace(TRACE_5, "reachGraph::calculateSuccStatesInput(messageMultiSet input, vertex * node) : end\n");

    return newStateList;							// return the new state list
}

//! \fn stateList * reachGraph::calculateSuccStatesOutput(unsigned int output, vertex * node)
//! \param output the output messages that are taken from the marking
//! \param node the node for which the successor states are to be calculated
//! \brief calculates the set of successor states in case of an output message
stateList * reachGraph::calculateSuccStatesOutput(unsigned int output, vertex * node) {
	trace(TRACE_5, "reachGraph::calculateSuccStatesOutput(unsigned int output, vertex * node) : start\n");

    stateList * newStateList = new stateList();     // the new list of states for the next node

    reachGraphStateSet::iterator iter;                      // iterator over the stateList's elements

    for (iter = node->getStateList()->setOfReachGraphStates.begin(); iter != node->getStateList()->setOfReachGraphStates.end(); iter++) {

        PN->setCurrentMarkingFromState(*iter);      // set the net to the marking of the state being considered
        if (PN->removeOutputMessage(output)) {      // remove the output message from the current marking
            // if there is a state for which an output event was activated, catch that state
            if (parameters[P_CALC_ALL_STATES]) {
                PN->calculateReachableStatesFull(newStateList, true);   // calc the reachable states from that marking
            } else {
                PN->calculateReachableStates(newStateList, true);   // calc the reachable states from that marking
            }
        }
    }
	trace(TRACE_5, "reachGraph::calculateSuccStatesOutput(unsigned int output, vertex * node) : end\n");
    return newStateList;                            // return the new state list
}

//! \fn stateList * reachGraph::calculateSuccStatesOutput(unsigned int output, vertex * node)
//! \param output the output messages that are taken from the marking
//! \param node the node for which the successor states are to be calculated
//! \brief calculates the set of successor states in case of an output message
stateList * reachGraph::calculateSuccStatesOutput(messageMultiSet output, vertex * node) {
	trace(TRACE_5, "reachGraph::calculateSuccStatesOutput(messageMultiSet output, vertex * node) : start\n");

    stateList * newStateList = new stateList();     // the new list of states for the next node

    reachGraphStateSet::iterator iter;                      // iterator over the stateList's elements

    for (iter = node->getStateList()->setOfReachGraphStates.begin(); iter != node->getStateList()->setOfReachGraphStates.end(); iter++) {

        PN->setCurrentMarkingFromState(*iter);      // set the net to the marking of the state being considered
        if (PN->removeOutputMessage(output)) {      // remove the output message from the current marking
            // if there is a state for which an output event was activated, catch that state
            if (parameters[P_CALC_ALL_STATES]) {
                PN->calculateReachableStatesFull(newStateList, true);   // calc the reachable states from that marking
            } else {
                PN->calculateReachableStates(newStateList, true);   // calc the reachable states from that marking
            }
        }
	}
	trace(TRACE_5, "reachGraph::calculateSuccStatesOutput(messageMultiSet output, vertex * node) : end\n");

    return newStateList;                            // return the new state list
}

//! \fn void reachGraph::printGraphToDot(vertex * v, fstream& os, bool visitedNodes[])
//! \param v current node in the iteration process
//! \param os output stream
//! \param visitedNodes[] array of bool storing the nodes that we have looked at so far
//! \brief breadthsearch through the graph printing each node and edge to the output stream
void reachGraph::printGraphToDot(vertex * v, fstream& os, bool visitedNodes[]) {

    if (v->getColor() == BLUE) {
        numberBlueNodes++;
    }

    if (parameters[P_SHOW_ALL_NODES]
            || (parameters[P_SHOW_NO_RED_NODES] && (v->getColor() != RED))
            || (parameters[P_SHOW_BLUE_NODES_ONLY] && (v->getColor() == BLUE))
            || (v == root)) {
	
	    if (parameters[P_SHOW_EMPTY_NODE]
	        || v->getStateList()->setOfReachGraphStates.size() != 0) {
	
		    os << "p" << v->getNumber() << " [label=\"# " << v->getNumber() << "\\n";
		
		    string CNF = "<";
		    bool mult = false;  // more than one clause -> true
		
			reachGraphStateSet::iterator iter;                      // iterator over the stateList's elements
		
	        for (iter = v->getStateList()->setOfReachGraphStates.begin(); iter != v->getStateList()->setOfReachGraphStates.end(); iter++) {
	
	            if (parameters[P_SHOW_STATES_PER_NODE]) {
	                os << "[" << PN->printMarking((*iter)->state->myMarking) << "]\\n";
	            }
//				os << "(";
				if (v->getColor() != RED) {
					switch ((*iter)->state->type) {
						case DEADLOCK:  if (mult) {
	                                        CNF += " * ";
	                                    }
	                                    //os << "DL";
	                                    CNF += "("; CNF += (*iter)->getClause(); CNF += ")"; mult=true;
	                                    break;
	                    case FINALSTATE: if (mult) {
	                                    	CNF += " * ";
	                                	}
//										os << "FS";
	                                	CNF += "(true)"; mult=true;
	                                	break;
	                    default:		
//										os << "TR";
//										CNF += "true";
	                            		break;
	                };
				} //else {
//					switch ((*iter)->state->type) {
//						case DEADLOCK:  os << "DL"; break;
//						case FINALSTATE: os << "FS"; break;
//						default: os << "TR"; break;
//					};
//				}
//				os << ", " << (*iter)->isMinimal();
//				os << ") " << (*iter)->state->index << "\\n";
			}
		
		    if (parameters[P_OG]) {
			    if (v->getColor() == RED) {
					CNF += "(false)";
			    } else {
			        if (v->getStateList()->setOfReachGraphStates.size() == 0) {
			            CNF += "(true)";
			        }
			    }
			    CNF += ">";
				os << CNF;
		    }
		
		    os << "\", fontcolor=black, color=";
		
		    switch(v->getColor()) {
		        case BLUE: os << "blue"; break;
		        case RED: os << "red, style=dashed"; break;
		        default: os << "black"; break;
		    }
		    os << "];\n";
		
		    v->resetIteratingSuccNodes();
		    visitedNodes[v->getNumber()] = 1;
		    graphEdge * element;
		    string label;
		
		    while ((element = v->getNextEdge()) != NULL) {
		        vertex * vNext = element->getNode();
		
			    if (parameters[P_SHOW_ALL_NODES]
			            || (parameters[P_SHOW_NO_RED_NODES] && vNext->getColor() != RED)
			            || (parameters[P_SHOW_BLUE_NODES_ONLY] && vNext->getColor() == BLUE)) {
			
				    if (parameters[P_SHOW_EMPTY_NODE] || vNext->getStateList()->setOfReachGraphStates.size() != 0) {
				
				        if (vNext != NULL) {
				            if (element->getType() == receiving) {
				                label = "?";
				            } else {
				                label = "!";
				            }
				            os << "p" << v->getNumber() << "->" << "p" << vNext->getNumber() << " [label=\"" << label << element->getLabel() << "\", fontcolor=black, color=";
				            switch (vNext->getColor()) {
				                case RED: os << "red"; break;
				                case BLUE: os << "blue"; numberBlueEdges++; break;
				                default: os << "black"; break;
				            }
				            os << "];\n";
				            if ((vNext != v) && !visitedNodes[vNext->getNumber()]) {
				                printGraphToDot(vNext, os, visitedNodes);
				            }
				        }
				    }
			    }
		    } // while
	    }
    }
}


//! \fn void reachGraph::printDotFile()
//! \brief creates a dot file of the graph
void reachGraph::printDotFile() {
    if (numberOfVertices < 200000) {
        vertex * tmp = root;
        bool visitedNodes[numberOfVertices];

        for (int i = 0; i < numberOfVertices; i++) {
            visitedNodes[i] = 0;
        }

        char buffer[256];
        if (parameters[P_OG]) {
            if (parameters[P_CALC_ALL_STATES]) {
                sprintf(buffer, "%s.a.OG.out", netfile);
            } else {
                sprintf(buffer, "%s.OG.out", netfile);
            }
        } else {
            if (parameters[P_CALC_ALL_STATES]) {
                sprintf(buffer, "%s.a.IG.out", netfile);
            } else {
                sprintf(buffer, "%s.IG.out", netfile);
            }
        }

        fstream dotFile(buffer, ios_base::out | ios_base::trunc);
        dotFile << "digraph g1 {\n";
        dotFile << "graph [fontname=\"Helvetica\", label=\"";
        parameters[P_OG] ? dotFile << "OG of " : dotFile << "IG of ";
        dotFile << netfile;
        dotFile << "\"];\n";
        dotFile << "node [fontname=\"Helvetica\" fontsize=10];\n";
        dotFile << "edge [fontname=\"Helvetica\" fontsize=10];\n";

		numberBlueNodes = 0;

        printGraphToDot(tmp, dotFile, visitedNodes);
        dotFile << "}";
        dotFile.close();
        trace(TRACE_0, "    number of blue nodes: " + intToString(numberBlueNodes) + "\n");
        trace(TRACE_0, "    number of blue edges: " + intToString(numberBlueEdges) + "\n");
        trace(TRACE_0, "    number of states stored in nodes: " + intToString(numberOfStatesAllNodes) + "\n");
        

        if (numberOfVertices < 900) {
            trace(TRACE_0, "creating the dot file of the graph...\n");
            if (parameters[P_OG]) {
                if (parameters[P_CALC_ALL_STATES]) {
                    sprintf(buffer, "dot -Tpng %s.a.OG.out -o %s.a.OG.png", netfile, netfile);
                } else {
                    sprintf(buffer, "dot -Tpng %s.OG.out -o %s.OG.png", netfile, netfile);
                }
            } else {
                if (parameters[P_CALC_ALL_STATES]) {
                    sprintf(buffer, "dot -Tpng %s.a.IG.out -o %s.a.IG.png", netfile, netfile);
                } else {
                    sprintf(buffer, "dot -Tpng %s.IG.out -o %s.IG.png", netfile, netfile);
                }
            }
            trace(TRACE_0, buffer); trace(TRACE_0, "\n");
            system(buffer);

        } else {
            trace(TRACE_0, "graph is too big to have dot create the graphics\n");
        }
    } else {
        trace(TRACE_0, "graph is too big to create dot file\n");
    }
}


//! \fn bool reachGraph::stateActivatesOutputEvents(reachGraphState * s)
//! \param s the state that is checked for activating output events
//! \brief returns true, if the given state activates at least one output event
bool reachGraph::stateActivatesOutputEvents(reachGraphState * s) {
    int i;
    unsigned int * marking = s->state->myMarking;

    for (i = 0; i < PN->getPlaceCnt(); i++) {

        if (PN->Places[i]->getType() == OUTPUT && marking[i] > 0) {
            return true;
        }
    }
    return false;
}


//! \fn analysisResult reachGraph::analyseNode(vertex * node, bool finalAnalysis)
//! \param node the node to be analysed
//! \brief analyses the node and sets its color, if the node gets to be red, then TERMINATE is returned
analysisResult reachGraph::analyseNode(vertex * node, bool finalAnalysis) {

	trace(TRACE_2, "\t\t\t analysing node ");
	trace(TRACE_2, intToString(node->getNumber()) + ": ");

    if (node->getColor() != RED) {			// red nodes stay red forever
        if (node->getStateList()->setOfReachGraphStates.size() == 0) {
        	// we analyse an empty node; it becomes blue
            if (node->getColor() != BLUE) {
                numberBlueNodes++;
            }
            node->setColor(BLUE);
			trace(TRACE_2, "node analysed blue (empty node)");
			trace(TRACE_2, "\t ...terminate\n");
            return TERMINATE;
        } else {
        	// we analyse a non-empty node

            vertexColor c = BLACK;						// the color of the current node            
            vertexColor cTmp = BLACK;					// the color of a state of the current node
            string clause;
            bool finalState = false;

            reachGraphStateSet::iterator iter;			// iterator over the stateList's elements

			// iterate over all states and determine color for each state
            for (iter = node->getStateList()->setOfReachGraphStates.begin(); iter != node->getStateList()->setOfReachGraphStates.end(); iter++) {

                // we just consider the maximal states only
                if ((*iter)->state->type == DEADLOCK || (*iter)->state->type == FINALSTATE) {
                    if ((*iter)->state->type == FINALSTATE) {
                        c = BLUE;					// final states potentially make node blue
                        finalState = true;
                    } else {
                    	// state is a deadlock
                        finalState = false;
                        if (node->eventsToBeSeen == 0 || finalAnalysis) {
                            cTmp = (*iter)->calcColor();
				            switch (cTmp) {
			                case RED:	// found a red state; so node becomes red
			                	if (node->getColor() == BLACK) {
	                            	// node was black
	                            	trace(TRACE_2, "node analysed red \t");
	                            	node->setColor(RED);
			                	} else if (node->getColor() == RED) {
			                		// this should not be the case!
			                		trace(TRACE_2, "analyseNode called when node already red!!!");
			                	} else if (node->getColor() == BLUE) {
			                		// this should not be the case!
                                	trace(TRACE_2, "analyseNode called when node already blue!!!");
                                	numberBlueNodes--;
                            	}
                            	trace(TRACE_2, "\t\t ...terminate\n");
                            	return TERMINATE;
								break;
			                case BLUE:	// found a blue state (i.e. deadlock is resolved)
								c = BLUE;
			                	break;
			                case BLACK:	// no definite result of state analysis
                                if (finalAnalysis) {
                                    node->setColor(RED);
	                            	trace(TRACE_2, "node analysed red (final analysis)");
	                                trace(TRACE_2, "\t ...terminate\n");
	                            	return TERMINATE; 			// <---------------------???
                                } else {
                                    node->setColor(BLACK);
	                            	trace(TRACE_2, "node still indefinite \t\t ...continue\n");
	                            	return CONTINUE;
                                }
                                break;
				            }
                        } else {
                        	// still events left to resolve deadlocks...
                            node->setColor(BLACK);
                            trace(TRACE_2, "node still indefinite \t\t ...continue\n");
                            return CONTINUE;
                        }
                    }
                }
            }
            // all states considered
            
            if (node->getColor() != BLUE && c == BLUE) {
                numberBlueNodes++;
            } else if (node->getColor() == BLUE && c != BLUE) {
                numberBlueNodes--;
            }

            trace(TRACE_2, "all states checked, node becomes ");     	
            if (c == BLACK)
            	trace(TRACE_2, "black");
            else if (c == RED)
            	trace(TRACE_2, "red");
            else if (c == BLUE)
            	trace(TRACE_2, "blue");
            
            node->setColor(c);
            if (finalState) {
            	trace(TRACE_2, " ...terminate\n");
                return TERMINATE;
            } else {
            	trace(TRACE_2, " ...continue\n");
                return CONTINUE;
            }
        }
    }
    return TERMINATE;
}
