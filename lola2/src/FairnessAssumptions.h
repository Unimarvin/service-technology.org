/*!

\author Karsten
\file FairnessAssumptions.h
\status new

This is just a type definition for the possible fairness assumptions
that can be specified for each transition.
We consider weak and strong fairness as well as the absence of fairness.
*/


#pragma once

typedef enum 
{
	NO_FAIRNESS,
	WEAK_FAIRNESS,
	STRONG_FAIRNESS
}
tFairnessAssumption;
