/*!
\file Place.h
\author Karsten
\status approved 27.01.2012
\brief Global data for place specific information

All data that describe attributes of places can be found here. General
information for a place in its role as a node, ar contained in Node.*
*/

#pragma once

#include "Dimensions.h"

/*!
\brief collection of information related to places
*/
struct Place
{
    public:
        /// number of signifcant places; significant places are such that comparing markings on the
        /// significant places suffices for checking equality
        static index_t CardSignificant;

        /// The hash factor for a place. We compute hash values for places as
        /// sum_{p in P} m(p)*hash(p)
        static hash_t* Hash;

        /// The maximum number of tokens that, according to input specification, can ever
        /// be present on this place. This is only used for calculating a dense representation of a
        /// marking, not as a blocker for transition activation.
        static capacity_t* Capacity;

        /// The number of bits needed for representing the numbers 0 .. Capacity[i]
        /// Used for handling dense representations of markings
        static cardbit_t* CardBits;

        /// Number of transitions for which this place is the distinguished insufficiently marked
        /// Pre-place (scapegoat)
        static index_t* CardDisabled;

        /// These are the transitions for which this place is the scapegoat
        /// The arrays Disabled[i] are allocated for the maximum number of such transitions
        /// which is card(p\bullet)
        static index_t** Disabled;

        /// Aufräumen der Knoten - Service für valgrind
        static void deletePlaces();

        /// Compute required nr of bits from max. nr of tokens
        static cardbit_t Capacity2Bits(capacity_t);
};
