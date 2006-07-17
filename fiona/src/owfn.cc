#include "mynew.h"
#include "graph.h"
//#include "owfn.h"
//#include "owfnPlace.h"
//#include "owfnTransition.h"

#include "options.h"
#include "debug.h"

//#include<stdlib.h>
//#include<stdio.h>
//#include<unistd.h>

//#include <string>



using namespace std;


//! \fn oWFN::oWFN()
//! \brief constructor
oWFN::oWFN() : arcCnt(0), placeHashValue(0), placeCnt(0),
               placeInputCnt(0), placeOutputCnt(0), transCnt(0),
               transNrEnabled(0), BitVectorSize(0), currentState(0),
               startOfEnabledList(NULL), startOfQuasiEnabledList(NULL),
               FinalCondition(NULL) {
	startOfEnabledList = (owfnTransition *) 0;

	unsigned int i;
  	NonEmptyHash = 0;
	commDepth = 0;

  	try {
  		binHashTable = new binDecision * [HASHSIZE];
	} catch(bad_alloc) {
		char mess[] = "\nhash table too large!\n";
		//write(2,mess,sizeof(mess));
		cerr << mess;
		_exit(2);
	}
  	
  	for(i = 0; i < HASHSIZE; i++) {
          binHashTable[i] = (binDecision *) 0;
    } 
}


//! \fn oWFN::~oWFN()
//! \brief destructor
oWFN::~oWFN() {	
	delete[] inputPlacesArray;
	delete[] outputPlacesArray;
}


//! \fn unsigned int oWFN::getPlaceCnt()
//! \brief returns the number of all places of the net
unsigned int oWFN::getPlaceCnt() {
	return placeCnt;
}


//! \fn unsigned int oWFN::getTransitionCnt()
//! \brief returns the number of transitions of the net
unsigned int oWFN::getTransitionCnt() {
	return transCnt;
}


//! \fn unsigned int oWFN::getInputPlaceCnt()
//! \brief returns the number of input places of the net
unsigned int oWFN::getInputPlaceCnt() {
	return placeInputCnt;
}


//! \fn unsigned int oWFN::getOutputPlaceCnt()
//! \brief returns the number of output places of the net
unsigned int oWFN::getOutputPlaceCnt() {
	return placeOutputCnt;
}


//! \fn unsigned int oWFN::getTransitionCnt()
//! \brief returns the number of transitions of the net
unsigned int oWFN::getCommDepth() {
	return commDepth;
}


unsigned int oWFN::getCardFireList() {
	cout << "oWFN::getCardFireList() never used" << endl;
	return CardFireList;
}


//! \fn oWFN::initialize()
//! \brief initializes the owfn; is called right after the parsing of the net file is done
void oWFN::initialize() {
	int i;
	
	// initialize places hashes
	for(i = 0; i < placeCnt; i++) {
       Places[i]->set_hash(rand());
    }
    
    // initialize places
  	for(i=0; i < placeCnt; i++) {
		Places[i]->index = i;
        
        if ((Places[i]->type == INPUT) && (CurrentMarking[i] < Places[i]->capacity)) {
        	CurrentMarking[i] = Places[i]->capacity;
        } else {
        	CurrentMarking[i] = Places[i]->initial_marking;
        }
  	}
  	
	//initialize transitions
  	for(i = 0; i < transCnt; i++) {
		Transitions[i]->enabled = false;
		Transitions[i]->quasiEnabled = false;
  	}
  	
  	for(i = 0; i < transCnt; i++) {
		Transitions[i]->initialize(this);
  	}
  	
  	for(i=0; i < placeCnt; i++) {
        CurrentMarking[i] = Places[i]->initial_marking;
  	}
 
	for(int i = 0; i < transCnt; i++) {
		Transitions[i]->check_enabled(this);
  	}	
  	
//  	for(i = 0, BitVectorSize = 0; i < placeCnt; i++) {
//        BitVectorSize += Places[i]->nrbits;
//  	}
  		
	unsigned int ki = 0;
	unsigned int ko = 0;
	
	inputPlacesArray = new int [placeInputCnt];
	outputPlacesArray = new int [placeOutputCnt];
	
	// get the data for those arrays from the places of the net
	for (i = 0; i < placeCnt; i++) { // getPlaceCnt(); i++) {
		if (Places[i]->type == INPUT) {
			// current place is from type input
			inputPlacesArray[ki++] = i;
		} else if (Places[i]->type == OUTPUT) {
			// current place is from type output
			outputPlacesArray[ko++] = i;
		}	
	}
	
	for(i = 0, BitVectorSize = 0; i < placeCnt; i++) {
        Places[i]->startbit = BitVectorSize;
        BitVectorSize += Places[i]->nrbits;
  	}
	
	trace(TRACE_5, "oWFN initialized\n");
	
}



void oWFN::initializeTransitions() {
	unsigned int i;
	 	
  	for(i = 0;i < transCnt; i++) {
		Transitions[i]->PrevEnabled = (i == 0 ? (owfnTransition *) 0 : Transitions[i-1]);
		Transitions[i]->NextEnabled = (i == transCnt - 1 ? (owfnTransition *) 0 : Transitions[i+1]);
 		Transitions[i]->enabled = true;
		Transitions[i]->PrevQuasiEnabled = (i == 0 ? (owfnTransition *) 0 : Transitions[i-1]);
		Transitions[i]->NextQuasiEnabled = (i == transCnt - 1 ? (owfnTransition *) 0 : Transitions[i+1]);
 		Transitions[i]->quasiEnabled = true;
  	}
  	startOfEnabledList = Transitions[0];
  	transNrEnabled = transCnt;

  	startOfQuasiEnabledList = Transitions[0];
  	transNrQuasiEnabled = transCnt; 
  	
	for(i = 0; i < transCnt; i++) {
		Transitions[i]->check_enabled(this);
  	}
}

void oWFN::removeisolated() {
	unsigned int i,j;

	owfnPlace * p;

	i=0;
	while(i<placeCnt) {
		if(Places[i]->references == 0) { // owfnPlace isolated
			p = Places[placeCnt - 1];
			Places[placeCnt - 1] = Places[i];
			Places[i] = p;
			placeCnt --;
		} else {
			i++;
		}
	}
	for(i=0;i<transCnt;i++) {
		Transitions[i]->nr = i;
	}
}


owfnTransition ** oWFN::firelist() {
	owfnTransition ** tl;
	owfnTransition * t;
	int i;
	tl = new owfnTransition * [transNrEnabled + 1];
	for(i=0, t = startOfEnabledList; t; t = t -> NextEnabled) {
		tl[i++] = t;
	}
	tl[i] = (owfnTransition *) 0;
	CardFireList = i;
	return tl;
}


owfnTransition ** oWFN::quasiFirelist() {
	owfnTransition ** tl;
	owfnTransition * t;
	int i;
	tl = new owfnTransition * [transNrQuasiEnabled + 1];
	for(i=0, t = startOfQuasiEnabledList; t; t = t -> NextQuasiEnabled) {
		tl[i++] = t;
	}
	tl[i] = (owfnTransition *) 0;
	CardQuasiFireList = i;
	return tl;
}



void oWFN::addStateToList(vertex * n, State * NewState, bool minimal) {

	if (parameters[P_CALC_ALL_STATES]
 		|| NewState->type == DEADLOCK
		|| minimal
		|| NewState->type == FINALSTATE) { 
		
		n->addState(NewState);
	}
	
	for(int i = 0; i < NewState->CardFireList; i++) {
		addStateToList(n, NewState->succ[i]);
	}
}

void oWFN::addSuccStatesToList(vertex * n, State * NewState) {
	if (NewState != NULL) {
		for(int i = 0; i < NewState->CardFireList; i++) {
			addStateToList(n, NewState->succ[i]);
		}
	}
}


void oWFN::addStateToList(vertex * n, State * currentState) {
		bool minimal = false;
		
//		unsigned int * myMarking;
		
		currentState->decode(this);
			
		for (int z = 0; z < placeCnt; z++) {
//			if (Places[z]->type == OUTPUT && myMarking[z] > 0) {
			if (Places[z]->type == OUTPUT && CurrentMarking[z] > 0) {
				minimal = true;
				break;
			}
		}
		
		if (currentState != NULL) {
			if (parameters[P_CALC_ALL_STATES] || currentState->type == DEADLOCK || minimal || currentState->type == FINALSTATE) { 
				n->addState(currentState);
			}
		} else {
			return ;
		}
	
	if (currentState != NULL) {
		for(int i = 0; i < currentState->CardFireList; i++) {
			addStateToList(n, currentState->succ[i]);
		}
	}
}

unsigned int * oWFN::copyCurrentMarking() {
	unsigned int * copy = new unsigned int [getPlaceCnt()];

	for (int i = 0; i < getPlaceCnt(); i++) {
		copy[i] = CurrentMarking[i];
	
	}	
	return copy;
}

void oWFN::copyMarkingToCurrentMarking(unsigned int * copy) {

	for (int i = 0; i < getPlaceCnt(); i++) {
		CurrentMarking[i] = copy[i];
	}	
	
	//initializeTransitions();
}


//! \fn void oWFN::calculateReachableStatesFull(vertex * listOfStates, bool minimal)
//! \param listOfStates list containing all reachable states from the current marking (after function is done)
//! \param minimal the current state is minimal in the vertex
//! \brief NO REDUCTION! calculate all reachable states from the current marking and store them in the list being passed 
//! as parameter (== vertex of reachGraph)
void oWFN::calculateReachableStates(vertex * n, bool minimal) {

	trace(TRACE_5, "oWFN::calculateReachableStates: start\n");

	State * CurrentState;
  	unsigned int i;
  	State * NewState;
  	stateType type;
  	
	CurrentState = binSearch(this);
	
	if (CurrentState == NULL) {
 	 	CurrentState = binInsert(this);
  		CurrentState->firelist = firelist();
  		CurrentState->CardFireList = CardFireList;
  		if (parameters[P_IG]) {
	  		CurrentState->quasiFirelist = quasiFirelist();
  		}
  	//	CurrentState->myMarking = copyCurrentMarking();
  		CurrentState->current = 0;
  		CurrentState->parent = (State *) 0;
  		CurrentState->succ = new State * [CardFireList+1];
  		CurrentState->placeHashValue = placeHashValue;
		CurrentState->type = typeOfState();
		n->addState(CurrentState);
	} else {
		if (CurrentState->type == DEADLOCK || minimal || CurrentState->type == FINALSTATE) { 
			if (n->addState(CurrentState)) {
				addSuccStatesToList(n, CurrentState);
			}
		}
	}
  	
	// building EG in a node
  	while(CurrentState) {
		if ((n->setOfStates.size() % 1000) == 0) {
			trace(TRACE_2, "\t current state count: " + intToString(n->setOfStates.size()) + "\n");
		}

		// no more transition to fire from current state?
		if (CurrentState->current < CurrentState->CardFireList) {
			//&& CurrentState->firelist[CurrentState->current] != NULL)

		//	copyMarkingToCurrentMarking(CurrentState->myMarking);
			CurrentState->decode(this);

//			placeHashValue = CurrentState->placeHashValue;

			// there is a next state that needs to be explored
	  		CurrentState->firelist[CurrentState->current]->fire(this);

			minimal = isMinimal();

	  		NewState = binSearch(this);
	  		
	  		if(NewState != NULL) {
		  		// Current marking already in bintree 
				if (NewState->type == DEADLOCK || minimal || NewState->type == FINALSTATE) { 
					if (n->addState(NewState)) {
						addSuccStatesToList(n, NewState);
					}
				}
				
		   		CurrentState -> succ[CurrentState -> current] = NewState;
	     		(CurrentState->current)++;
	    	} else {
      			NewState = binInsert(this);
      			
	      		NewState->firelist = firelist();
	      		NewState->CardFireList = CardFireList;
	      		if (parameters[P_IG]) {
		      		NewState->quasiFirelist = quasiFirelist();
	      		}
	      		NewState->current = 0;
	      	//	NewState->myMarking = copyCurrentMarking();
	      		NewState->parent = CurrentState;
	      		NewState->succ =  new State * [CardFireList+1];
	      		NewState->placeHashValue = placeHashValue;
	      		NewState->type = typeOfState();
	      		
	      		CurrentState->succ[CurrentState -> current] = NewState;
	      		CurrentState = NewState;
		      		
				if (NewState->type == DEADLOCK || minimal || NewState->type == FINALSTATE) { 
					n->addState(NewState);
				}
	    	}
		// no more transition to fire
		} else {
	  		// close state and return to previous state
	  		CurrentState = CurrentState->parent;

	  		if(CurrentState) {			// there is a father to further examine
	  			placeHashValue = CurrentState->placeHashValue;
	      		//copyMarkingToCurrentMarking(CurrentState->myMarking);
	      		CurrentState->decode(this);
	      		
	      		CurrentState->current++;
	    	}
		}
	}

	trace(TRACE_5, "end of function oWFN::calculateReachableStates\n");
}

//! \fn void oWFN::calculateReachableStatesFull(vertex * listOfStates, bool minimal)
//! \param listOfStates list containing all reachable states from the current marking (after function is done)
//! \param minimal the current state is minimal in the vertex
//! \brief NO REDUCTION! calculate all reachable states from the current marking and store them in the list being passed 
//! as parameter (== vertex of reachGraph)
void oWFN::calculateReachableStatesFull(vertex * n, bool minimal) {

	// calculates the EG starting at the current marking
	
	trace(TRACE_5, "start of function oWFN::calculateReachableStatesFull\n");

	State * CurrentState;
  	unsigned int i;
  	State * NewState;
  	stateType type;
	CurrentState = binSearch(this);
	
	unsigned int * tempCurrentMarking = NULL;
	unsigned int tempPlaceHashValue;
	
	
	if (options[O_BDD] == false && CurrentState != NULL) {
		// marking already has a state -> put it (and all its successors) into the node
		if (n->addState(CurrentState)) {
			addSuccStatesToList(n, CurrentState);
		}
		trace(TRACE_5, "end of function oWFN::calculateReachableStatesFull\n");
		return;
	}
	
	// the other case:
	// we have a marking which has not yet a state object assigned to it
	if (CurrentState == NULL) {
		CurrentState = binInsert(this);
	}
	
	CurrentState->firelist = firelist();
	CurrentState->CardFireList = CardFireList;
	if (parameters[P_IG]) {
  		CurrentState->quasiFirelist = quasiFirelist();
	}

	CurrentState->current = 0;
	CurrentState->parent = (State *) 0;
	CurrentState->succ = new State * [CardFireList+1];
	CurrentState->placeHashValue = placeHashValue;
	CurrentState->type = typeOfState();
	n->addState(CurrentState);
	  	
	// building EG in a node
  	while(CurrentState) {
 
		if ((n->setOfStates.size() % 1000) == 0) {
			trace(TRACE_2, "\t current state count: " + intToString(n->setOfStates.size()) + "\n");
		}
	  	
		// no more transition to fire from current state?
		if (CurrentState->current < CurrentState->CardFireList) {
			// there is a next state that needs to be explored

			if (tempCurrentMarking) {
				delete[] tempCurrentMarking;
				tempCurrentMarking = NULL;
			}
			
			tempCurrentMarking = copyCurrentMarking();
			tempPlaceHashValue = placeHashValue;
			  		
	  		trace(TRACE_5, "fire transition\n");

			CurrentState->firelist[CurrentState->current]->fire(this);
			minimal = isMinimal();
			NewState = binSearch(this);
			
	  		if(NewState != NULL) {
		  		// Current marking already in bintree 
				trace(TRACE_5, "Current marking already in bintree \n");
				if (n->addState(NewState)) {
					addSuccStatesToList(n, NewState);
				}
				
				copyMarkingToCurrentMarking(tempCurrentMarking);
				
				CurrentState->firelist[CurrentState->current]->backfire(this);
				
				placeHashValue = tempPlaceHashValue;
				
				delete[] tempCurrentMarking;
				tempCurrentMarking = NULL;
									
		   		CurrentState -> succ[CurrentState -> current] = NewState;
	     		(CurrentState->current)++;
	     		
	     //		CurrentState->decode(this);
	     		
	    	} else {
				trace(TRACE_5, "Current marking new\n");
      			NewState = binInsert(this);
      			NewState->firelist = firelist();
	      		NewState->CardFireList = CardFireList;
	      		if (parameters[P_IG]) {
		      		NewState->quasiFirelist = quasiFirelist();
	      		}
	      		NewState->current = 0;
	      		NewState->parent = CurrentState;
	      		NewState->succ =  new State * [CardFireList+1];
	      		NewState->placeHashValue = placeHashValue;
	      		NewState->type = typeOfState();
	      		
	      		CurrentState->succ[CurrentState -> current] = NewState;
	      		CurrentState = NewState;
		      		
				n->addState(NewState);
				
				if (tempCurrentMarking) {
					delete[] tempCurrentMarking;
					tempCurrentMarking = NULL;
				}
	    	}
		// no more transition to fire
		} else {
	  		// close state and return to previous state
			trace(TRACE_5, "close state and return to previous state\n");
	  		CurrentState = CurrentState->parent;

	  		if(CurrentState) {			// there is a father to further examine
	      		CurrentState->decode(this);
	      		CurrentState->current++;
	    	}
		}
	}
	if (tempCurrentMarking) {
		delete[] tempCurrentMarking;	
	}
}


//! \fn int oWFN::addInputMessage(unsigned int message)
//! \param message name of message to be added to the currentmarking
//! \brief adds input message to the current marking
int oWFN::addInputMessage(unsigned int message) {

	// found that place
	CurrentMarking[message]++;

	placeHashValue += Places[message]->hash_factor;
	placeHashValue %= HASHSIZE;

	for (int k = 0; k < Places[message]->NrOfLeaving; k++) {
		((owfnTransition *) Places[message]->LeavingArcs[k]->Destination)->check_enabled(this);
	}
	return 0;		// place found
}			


//! \fn int oWFN::addInputMessage(char * messages)
//! \param message name of message to be added to the currentmarking
//! \brief adds input message to the current marking
int oWFN::addInputMessage(messageMultiSet messages) {
	for (messageMultiSet::iterator iter = messages.begin(); iter != messages.end(); iter++) {
		if (Places[*iter]->type == INPUT) {
			// found that place
			CurrentMarking[*iter]++;
			placeHashValue += Places[*iter]->hash_factor;
			placeHashValue %= HASHSIZE;

			// TODO: check_enabled!!!!!!!! so richtig?!
			for (int k = 0; k < Places[*iter]->NrOfLeaving; k++) {
				((owfnTransition *) Places[*iter]->LeavingArcs[k]->Destination)->check_enabled(this);
			}
			return 0;
		}
	}
	return 1;		// place not found
}


// prints the CurrentMarking
void oWFN::printmarking() {
	bool comma = false;
	cout << "---------------------------------------------------------------" << endl;
	cout << "\t(" << CurrentMarking << ")" << "\t[";
	
	for (int i = 0; i < placeCnt; i++) {
		if (CurrentMarking[i] > 0) {
			if (CurrentMarking[i] > 5) {
				if (comma) {
					cout << ", ";
				}
				cout << Places[i]->name;
				cout << ":";
				cout << CurrentMarking[i];
				comma = true;
			} else {
				for (int k = 0; k < CurrentMarking[i]; k++) {
					if (comma) {
						cout << ", ";
					}
					cout << Places[i]->name;
					comma = true;
				}
			}
		}	
	}
	cout << "]\n---------------------------------------------------------------" << endl;
}

// prints a given marking
void oWFN::printmarking(unsigned int * marking) {
	bool comma = false;
	cout << "---------------------------------------------------------------" << endl;
	cout << "\t(" << marking << ")" << "\t[";
	
	for (int i = 0; i < placeCnt; i++) {
		if (marking[i] > 0) {
			if (marking[i] > 5) {
				if (comma) {
					cout << ", ";
				}
				cout << Places[i]->name;
				cout << ":";
				cout << marking[i];
				comma = true;
			} else {
				for (int k = 0; k < marking[i]; k++) {
					if (comma) {
						cout << ", ";
					}
					cout << Places[i]->name;
					comma = true;
				}
			}
		}	
	}
	cout << "]\n---------------------------------------------------------------" << endl;
}


//! \fn char * oWFN::printMarking(unsigned int * myMarking) 
//! \param myMarking the marking to be printed out
//! \brief returns the label of the given marking, that means the label consists of the names of the 
//! places of the net that have tokens (is a multiset => occurance of name == number of tokens
//
// used in dotFile creation (reachGraph::printGraphToDot)
char * oWFN::printMarking(unsigned int * marking) {
	bool comma = false;

	try {
		char * buffer = new char[256];
		strcpy(buffer, "");
		for (int i = 0; i < placeCnt; i++) {
			if (marking[i] > 5) {
				if (comma) {
					strcat(buffer, ", ");
				}
				strcat(buffer, Places[i]->name);
				strcat(buffer, ":");
				sprintf(buffer, "%s%d", buffer, marking[i]);
				comma = true;
			} else {
				for (int k = 0; k < marking[i]; k++) {
					if (comma) {
						strcat(buffer, ", ");
					}
					strcat(buffer, Places[i]->name);
					comma = true;
				}
			}
		}
		return buffer;
	} catch(bad_alloc) {
		char mess[] = "\n! oWFN::printMarking failed !\n";
		//write(2,mess,sizeof(mess));
		cerr << mess;
		_exit(2);
	}

}

char * oWFN::printCurrentMarkingForDot() {
	bool comma = false;

	try {
		char * buffer = new char[256];
		strcpy(buffer, "");
		for (int i = 0; i < placeCnt; i++) {
			if (CurrentMarking[i] > 5) {
				if (comma) {
					strcat(buffer, ", ");
				}
				strcat(buffer, Places[i]->name);
				strcat(buffer, ":");
				sprintf(buffer, "%s%d", buffer, CurrentMarking[i]);
				comma = true;
			} else {
				for (int k = 0; k < CurrentMarking[i]; k++) {
					if (comma) {
						strcat(buffer, ", ");
					}
					strcat(buffer, Places[i]->name);
					comma = true;
				}
			}
		}
		return buffer;
	} catch(bad_alloc) {
		char mess[] = "\n! oWFN::printMarking failed !\n";
		//write(2,mess,sizeof(mess));
		cerr << mess;
		_exit(2);
	}

}


void oWFN::printstate(char * c, unsigned int * st) {
    int i,j;
    if(Sflg) {
		cout << "STATE"; cout << c ;
		for(i=0,j=0;i<placeCnt;i++) {
        	if(st[i]) {
            	if(st[i] == VERYLARGE) {
                	cout << (j++ ? ",\n" : "\n") << Places[i]->name << " : oo" ;
            	} else {
					cout << (j++ ? ",\n" : "\n") << Places[i]->name << " : " << st[i];
            	}
        	}
    	}
    	cout << "\n\n";
    	return;
    }
    
    if(sflg) {
    	ofstream statestream(statefile);
    	if(! statestream) {  
        	cerr << "Cannot open state output file: " << statefile <<
        	"\n no output written\n";
        	return;
    	}
    	statestream << "STATE";
    	for(i=0,j=0;i<placeCnt;i++) {
        	if(st[i]) {
            	if(st[i] == VERYLARGE) {
                	statestream << (j++ ? ",\n" : "\n") << Places[i]->name << " : oo" ;
            	} else {
	                statestream << (j++ ? ",\n" : "\n") << Places[i]->name << " : " << st[i];
				}
        	}
		}
    	statestream << "\n\n";
    	return;
    }
}


void oWFN::print_binDec(int h) {

	for(int i=0; i < placeCnt; i++) {
		cout << Places[i] -> name << ": " << Places[i] -> nrbits;
	}
	cout << endl;
	
	print_binDec(binHashTable[h], 0);
}
	
	
void oWFN::print_binDec(binDecision * d, int indent) {
	int i;
	// print bin decision table at hash entry h

	for(i=0; i<indent; i++) cout << ".";
	
	if(!d) {
		cout << " nil " << endl;
		return;
	}
	
	cout << "b " << d -> bitnr << " v ";
	for(i=0; i< (BitVectorSize - (d -> bitnr + 1)) ; i+=8) {
		cout << (unsigned int) (d -> vector)[i/8] << " " ;
	}

	for(i=0; i<BitVectorSize - (d -> bitnr+1); i++) {
		cout << (((d->vector)[i/8] >> (7-i%8))%2);
	}
	cout << endl;
	print_binDec(d -> nextold, indent+1);
	print_binDec(d -> nextnew, indent+1);
}


unsigned int oWFN::getPlaceHashValue() {
	return placeHashValue;
}


void oWFN::addTransition(unsigned int i, owfnTransition * transition) {
	transCnt++;
	Transitions[i] = transition;
	if(!(transCnt % REPORTFREQUENCY)) {
		cerr << "\n" << transCnt << "transitions parsed\n";
  	}
}


void oWFN::addArc(Arc *arc) {
}


void oWFN::addPlace(unsigned int i, owfnPlace * place) {
	placeCnt++;
	Places[i] = place;
	place->index = i;
	
	if(!(placeCnt % REPORTFREQUENCY)) {
		cerr << "\n" << placeCnt << "places parsed\n";
  	}
}

//! \fn int oWFN::setCurrentMarkingFromState(reachGraphState * s)
//! \param s state to be explored
//! \brief extracts from the give state the marking and sets the currentmarking appropriatly
//int oWFN::setCurrentMarkingFromState(reachGraphState * s) {
//
//	trace(TRACE_5, "oWFN::setCurrentMarkingFromState(reachGraphState * state): start\n");
//
////	copyMarkingToCurrentMarking(s->state->myMarking);
//	
////	placeHashValue = s->state->placeHashValue;
//	
////	copyMarkingToCurrentMarking(s->state->myMarking);
//	
////	unsigned int * markingvonkarsten = new unsigned int [placeCnt];
//	s->state->decode(CurrentMarking, this);
//}


void oWFN::RemoveGraph() {
	int i;

	for(i=0;i<HASHSIZE;i++) {
		if(binHashTable[i]) delete binHashTable[i];
		binHashTable[i] = (binDecision *) 0;
	}
}


//! \fn int oWFN::removeOutputMessage(unsigned int message)
//! \param messageName name of message to be deleted from currentmarking
//! \brief removes an output message from the current marking, if an output place was found the marking on that
//! place is decreased by one and the function returns 1
int oWFN::removeOutputMessage(unsigned int message) {

	if (CurrentMarking[message] > 0) {
		CurrentMarking[message]--;
		placeHashValue -= Places[message]->hash_factor;
		placeHashValue %= HASHSIZE;
		
		// TODO: check_enabled!!!!!!!! so richtig?!
		for (int k = 0; k < Places[message]->NrOfLeaving; k++) {
			((owfnTransition *) Places[message]->LeavingArcs[k]->Destination)->check_enabled(this);
		}
		return 1;	
	} 
	return 0;
}	

//! \fn int oWFN::removeOutputMessage(messageMultiSet messages)
//! \param message index of message to be deleted from currentmarking
//! \brief removes an output message from the current marking, if an output place was found the marking on that
//! place is decreased by one and the function returns 1
int oWFN::removeOutputMessage(messageMultiSet messages) {
	unsigned int found = 0;
	
	for (messageMultiSet::iterator iter = messages.begin(); iter != messages.end(); iter++) {
		if (Places[*iter]->type == OUTPUT) {
			if (CurrentMarking[*iter] > 0) {
				found++;
				// found that place
				CurrentMarking[*iter]--;
				placeHashValue -= Places[*iter]->hash_factor;
				placeHashValue %= HASHSIZE;
				
				// TODO: check_enabled!!!!!!!! so richtig?!
				for (int k = 0; k < Places[*iter]->NrOfLeaving; k++) {
					((owfnTransition *) Places[*iter]->LeavingArcs[k]->Destination)->check_enabled(this);
				}	
			}
		} 
	}
	return messages.size() == found;	
}


void oWFN::deletePlace(owfnPlace * place) {
	delete place;
	placeCnt--;
}

void oWFN::deleteTransition(owfnTransition * transition) {
	delete transition;
	transCnt--;
}


//! \fn stateType oWFN::typeOfState()
//! \brief returns true, if current state is transient; that means the number of enabled transitions is > 0
stateType oWFN::typeOfState() {
	if (isFinal()) {			// state is final
		return FINALSTATE;
	} 
	if (transNrEnabled == 0) {
		return DEADLOCK;		// state is an internal deadlock, no transition is enabled or quasi-enabled
	} else if (transNrEnabled != 0) {
		return TRANS;		// state is transient since there are transitions which are enabled
	}
	return NN;			
}

//! \fn bool oWFN::isMinimal()
//! \brief returns true, if current state is minimal
bool oWFN::isMinimal() {
	
	int i;
	int k = 0;
	for (i = 0; i < placeCnt; i++) {
		if (Places[i]->type == OUTPUT && CurrentMarking[i] > 0) {
			return true;
		}
	}
	return false;
}

//! \fn bool oWFN::isMaximal()
//! \brief returns true, if current state is maximal
bool oWFN::isMaximal() {
	return transNrEnabled == 0;			
}

//! \fn bool oWFN::isFinalMarking(unsigned int * marking) 
//! \param marking marking to be checked
//! \brief checks if the marking given as parameter is the final marking of the net
bool oWFN::isFinalMarking(unsigned int * marking) {
	for (int i = 0; i < getPlaceCnt(); i++) {
		if (Places[i]->type != INTERNAL && marking[i] > 0) {
			return false;
		}
		if (FinalMarking[i] != 0 && marking[i] != FinalMarking[i]) {
			return false;	
		}	
	}

	return true;
}

bool oWFN::isFinal() {
	if(FinalCondition) 
	{
		return FinalCondition -> value;
	}
	else
	{
		return isFinalMarking(CurrentMarking);
	}
}

char * oWFN::createLabel(messageMultiSet m) {
	char * label = new char[256];
	char * actualMessage;
	bool comma = false;
	
	strcpy(label, "");
	
	for (messageMultiSet::iterator iter1 = m.begin(); iter1 != m.end(); iter1++) {
		if (comma) {
			strcat(label, ", ");
		}
		actualMessage = new char[strlen(Places[*iter1]->name)];
		strcpy(actualMessage, Places[*iter1]->name);
		strcat(label, actualMessage);
		comma = true;
	}
	
	return label;
}

