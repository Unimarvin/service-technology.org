#include <config.h>
#include <iostream>
#include <cstdlib>
#include <map>

#include "testFormula.h"
#include "testNode.h"
#include "testGraph.h"
#include "types.h"
#include "helpers.h"

using namespace std;
map<string, unsigned int> label2id;
map<unsigned int, string> id2label;

unsigned int firstLabelId; //all labels including tau
unsigned int firstInputId; //input labels
unsigned int firstOutputId;//output labels

unsigned int lastLabelId;
unsigned int lastInputId;
unsigned int lastOutputId;

int main() {

	//test clauseToString();
	Clause c;
	c.literal0 = 23;
	c.literal1 = 42;
	c.literal2 = -1;
	assert(clauseToString(c) == "  23 42 -1 ");


	testFormulaClass();
	testNodeClass();
	testGraphClass();
	cout << "tests finished" << endl << endl;
	exit(0);
}