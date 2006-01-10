/*****************************************************************************\
 * Copyright 2005, 2006 Niels Lohmann, Christian Gierds, Dennis Reinert      *
 *                                                                           *
 * This file is part of BPEL2oWFN.                                           *
 *                                                                           *
 * BPEL2oWFN is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or (at your    *
 * option) any later version.                                                *
 *                                                                           *
 * BPEL2oWFN is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for  *
 * more details.                                                             *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with BPEL2oWFN; if not, write to the Free Software Foundation, Inc., 51   *
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.                      *
\****************************************************************************/

/*!
 * \file petrinet.h
 *
 * \brief Functions for Petri nets (interface)
 *
 * This file contains the data structures and classes to store and organize
 * Petri nets generated from the abstract syntax tree. It contains the
 * following classes:
 *
 * - Node
 *   - Transition
 *   - Place
 * - Arc
 * - PetriNet
 * 
 *
 * \author
 *          - responsible: Niels Lohmann <nlohmann@informatik.hu-berlin.de>
 *          - last changes of: \$Author: nlohmann $
 * 
 * \date
 *          - created: 2005/10/18
 *          - last changed: \$Date: 2006/01/10 10:14:22 $
 * 
 * \note    This file is part of the tool BPEL2oWFN and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/forschung/projekte/tools4bpel
 *          for details.
 *          
 * \version \$Revision: 1.45 $
 */





#ifndef PETRINET_H
#define PETRINET_H

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include "bpel-kc-k.h"		// generated by kimwitu++
#include "bpel-kc-rk.h"		// generated by kimwitu++
#include "bpel-kc-unpk.h"	// generated by kimwitu++
#include "debug.h"		// debugging help
#include "helpers.h"		// helper functions
#include "exception.h"		// exception class

using namespace std;



/// Enumeration of the possible types of a place.
typedef enum
{
  INTERNAL,			///< low-level place (standard)
  IN,				///< input place of an open workflow net (oWFN)
  OUT				///< output place of an open workflow net (oWFN)
} place_type;





/// Enumeration of the possible types of an arc.
typedef enum
{
  STANDARD,			///< low-level arc (standard)
  READ,				///< read arc
  RESET				///< reset arc
} arc_type;





/// Enumeration of the possible types of a node.
typedef enum
{
  PLACE,			///< a place
  TRANSITION			///< a transition 
} node_type;





/*****************************************************************************/


/*!
 * \class  Node
 * \author Niels Lohmann <nlohmann@informatik.hu-berlin.de>
 *
 * \brief Nodes of the Petri net
 *
 * Class to represent nodes (i.e. places and transitions) of Petri nets. Each
 * node has an id and a history (i.e. the list of roles the node had during
 * the processing of a BPEL-file).
 * 
*/

class Node
{
public:
  /// the id of the node
  unsigned int id;

  /// the name of the type
  string nodeTypeName ();

  /// the type of the node
  node_type nodeType;

  /// the set of roles (i.e. the history) of the node
    vector < string > history;

  /// true if first role contains role
  bool firstMemberAs (string role);

  /// true if first role begins with role
  bool firstMemberIs (string role);
};





/*****************************************************************************/


/*!
 * \class  Transition
 * \author Niels Lohmann <nlohmann@informatik.hu-berlin.de>
 *
 * \brief Transitions of the Petri net
 *
 * Class to represent transitions of Petri nets. In addition to the inherited
 * variables #id and #history, each transition can have a transition #guard
 * stored as C++-string.
 * 
*/

class Transition:public Node
{
public:
  /// guard of the transition
  string guard;

  /// constructor which creates a transition and adds a first role to the history
  Transition (unsigned int id, string role, string guard);

  /// DOT-output of the transition (used by PetriNet::dotOut())
  string dotOut ();
};





/*****************************************************************************/


/*!
 * \class  Place
 * \author Niels Lohmann <nlohmann@informatik.hu-berlin.de>
 *
 * \brief Places of the Petri net
 *
 * Class to represent places of Petri nets. In addition to the inherited
 * variables #id and #history, each place has a type defined in the
 * enumeation #place_type.
 * 
*/

class Place:public Node
{
public:
  /// type of the place (as defined in #place_type)
  place_type type;

  /// constructor which creates a place and adds a first role to the history
  Place (unsigned int id, string role, place_type type);

  /// DOT-output of the place (used by PetriNet::dotOut())
  string dotOut ();
};





/*****************************************************************************/


/*!
 * \class  Arc
 * \author Niels Lohmann <nlohmann@informatik.hu-berlin.de>
 *
 * \brief Arcs of the Petri net
 *
 * Class to represent arcs of Petri nets. An arc written as a tupel (n1,n2)
 * has n1 as #source and n2 as #target. Moreover each arc has a type defined
 * in the enumeration #arc_type and an optional #inscription for high-level
 * Petri nets to be evaluated by guarded transitions.
 * 
*/

class Arc
{
public:
  /// source node of the arc
  Node * source;

  /// target node of the arc
  Node *target;

  /// type of the arc (as defined in #arc_type)
  arc_type type;

  /// inscription of the arc for high-level Petri nets
  string inscription;

  /// Constructor to create an arc of certain type and inscription.
    Arc (Node * source, Node * target, arc_type type, string inscription);

  /// DOT-output of the arc (used by PetriNet::dotOut())
  string dotOut ();
};





/*****************************************************************************/


/*!
 * \class  PetriNet
 * \author Niels Lohmann <nlohmann@informatik.hu-berlin.de>
 *
 * \brief A Petri net
 *
 * Class to represent Petri nets. The net is consists of places of class
 * #Place, transitions of class #Transition and arcs of class #Arc. The sets
 * are saved in three lists #P, #T and #F.
 *
 */


class PetriNet
{
public:
  /// Adds a place with a given role and type.
  Place * newPlace (string role, place_type type = INTERNAL);

  /// Adds a transition with a given role and guard.
  Transition *newTransition (string role, string guard = "");

  /// Adds an arc given source and target node and an inscription.
  Arc *newArc (Node * source, Node * target, string inscription = "");

  /// Adds an arc given source and target node and an inscription.
  Arc *newArc (Node * source, Node * target, kc::casestring inscription);

  /// Adds an arc given source and target node, and arc type and an inscription.
  Arc *newArc (Node * source, Node * target, arc_type type,
	       string inscription = "");

  /// Information about the net including histories of all nodes.
  void printInformation ();

  /// DOT (Graphviz) output.
  void dotOut ();

  /// LoLA-output.
  void lolaOut ();

  /// oWFN-output.
  void owfnOut ();

  /// Merges places given two places.
  void mergePlaces (Place * p1, Place * p2);

  /// Merges places given two roles.
  void mergePlaces (string role1, string role2);

  /// Merges places given two activities with roles.
  void mergePlaces (kc::impl_activity * act1, string role1,
		    kc::impl_activity * act2, string role2);

  /// Merges transitions given two transitions.
  void mergeTransitions (Transition * t1, Transition * t2);

  /// Finds place given a role.
  Place *findPlace (string role);

  /// Finds place given an activity with a role.
  Place *findPlace (kc::impl_activity * activity, string role);

  /// Finds transition given a role.
  Transition *findTransition (string role);

  /// Simplifies the Petri net.
  void simplify ();

  /// Converts net to low-level net.
  void makeLowLevel ();

  /// Constructor to create an empty Petri net.
    PetriNet ();

private:
  /// Adds a place without an initial role.
    Place * newPlace ();

  /// Removes a place from the net.
  void removePlace (Place * p);

  /// Adds a transition without an initial role.
  Transition *newTransition ();

  /// Removes a transition from the net.
  void removeTransition (Transition * t);

  /// Removes an arc from the net.
  void removeArc (Arc * f);

  /// Removes all ingoing and outgoing arcs of a node.
  void detachNode (Node * n);

  /// Calculates the preset of a node.
    set < pair < Node *, arc_type > >preset (Node * n);

  /// Calculates the postset of a node.
    set < pair < Node *, arc_type > >postset (Node * n);

  /// the list of places of the Petri net
    set < Place * >P;

  /// the list of transitions of the Petri net
    set < Transition * >T;

  /// the list of arcs of the Petri net
    set < Arc * >F;

  /// Statistical output.
  string information ();

  /// Remove interface places (for LoLA-output)
  void removeInterface ();

  /// Returns an id for new nodes.
  unsigned int getId ();

  /// Returns current id.
  unsigned int id ();

  /// the id that will be assigned to the next node
  unsigned int nextId;

  /// true if function #PetriNet::makeLowLevel() was called
  bool lowLevel;

  /// Mapping of roles to nodes of the Petri net.
    map < string, Node * >roleMap;
};

#endif
