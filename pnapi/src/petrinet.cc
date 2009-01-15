/*!
 * \file    petrinet.cc
 *
 * \brief   Petri net basics
 *
 * \author  Niels Lohmann <nlohmann@informatik.hu-berlin.de>,
 *          Christian Gierds <gierds@informatik.hu-berlin.de>,
 *          Martin Znamirowski <znamirow@informatik.hu-berlin.de>,
 *          Robert Waltemath <robert.waltemath@uni-rostock.de>,
 *          last changes of: $Author$
 *
 * \since   2005-10-18
 *
 * \date    $Date$
 *
 * \version $Revision$
 */

#include <cassert>
#include <iostream>
#include <algorithm>

#include "util.h"
#include "parser.h"
#include "component.h"
#include "petrinet.h"

using std::cerr;
using std::min;


namespace pnapi
{

  /****************************************************************************
   *** Class Marking Function Definitions
   ***************************************************************************/

  /*!
   * \brief   Constructor for marking class
   */
  Marking::Marking()
  {
  }


  /*!
   * \brief   Constructor for marking class
   *
   * Reads marking from the Petri net.
   *
   * \param   PetriNet &n
   */
  Marking::Marking(PetriNet &n)
  {
    for (set<Place *>::const_iterator p = n.getPlaces().begin(); p != n.getPlaces().end(); p++)
      m[*p] = (*p)->getTokenCount();
  }


  /*!
   * \brief   Returns the map..
   */
  map<Place *, unsigned int> Marking::getMap() const
  {
    return m;
  }


  /*!
   * \brief   Returns the size of the Marking
   */
  unsigned int Marking::size()
  {
    return m.size();
  }


  /*!
   * \brief   overloaded operator [] for Markings
   */
  unsigned int & Marking::operator [](Place *offset)
  {
    return m[offset];
  }


  /*!
   * \brief   overloaded operator == for Markings
   */
  bool Marking::operator ==(const Marking &mm) const
  {
    return m == mm.getMap();
  }



  /****************************************************************************
   *** Class ComponentObserver Function Defintions
   ***************************************************************************/

  /*!
   */
  ComponentObserver::ComponentObserver(PetriNet & net) :
    net_(net)
  {
    assert(&net.observer_ == this);
  }


  /*!
   */
  PetriNet & ComponentObserver::getPetriNet() const
  {
    return net_;
  }


  /*!
   */
  void ComponentObserver::updateNodeNameHistory(Node & node,
					       const deque<string> & oldHistory)
  {
    assert(net_.containsNode(node));

    finalizeNodeNameHistory(node, oldHistory);
    initializeNodeNameHistory(node);
  }


  /*!
   */
  void ComponentObserver::updatePlaceType(Place & place, Node::Type type)
  {
    assert(net_.containsNode(place));

    finalizePlaceType(place, type);
    initializePlaceType(place);
  }


  void ComponentObserver::updateArcCreated(Arc & arc)
  {
    assert(&arc.getPetriNet() == &net_);
    assert(net_.arcs_.find(&arc) == net_.arcs_.end());
    assert(net_.findArc(arc.getSourceNode(), arc.getTargetNode()) == NULL);

    net_.arcs_.insert(&arc);
    arc.getTargetNode().preset_.insert(&arc.getSourceNode());
    arc.getSourceNode().postset_.insert(&arc.getTargetNode());
  }


  void ComponentObserver::updateArcRemoved(Arc & arc)
  {
    assert(&arc.getPetriNet() == &net_);
    assert(net_.arcs_.find(&arc) == net_.arcs_.end());
    assert(net_.findArc(arc.getSourceNode(), arc.getTargetNode()) == NULL);

    net_.arcs_.erase(&arc);
    arc.getTargetNode().preset_.erase(&arc.getSourceNode());
    arc.getSourceNode().postset_.erase(&arc.getTargetNode());
  }


  void ComponentObserver::updatePlaces(Place & place)
  {
    updateNodes(place);
    net_.places_.insert(&place);
    initializePlaceType(place);
  }


  void ComponentObserver::updateTransitions(Transition & trans)
  {
    updateNodes(trans);
    net_.transitions_.insert(&trans);
  }


  void ComponentObserver::updateNodes(Node & node)
  {
    assert(&node.getPetriNet() == &net_);
    assert(!net_.containsNode(node));
    assert(net_.nodesByName_.find(node.getName()) == net_.nodesByName_.end());

    net_.nodes_.insert(&node);
    initializeNodeNameHistory(node);
  }

  
  void ComponentObserver::updateNodesMerged(Node & node1, Node & node2)
  {
    assert(node2.getNameHistory().empty());

    // delete node2 from net
    Place * place = dynamic_cast<Place *>(&node2);
    if (place != NULL)
      net_.deletePlace(*place);
    else
      net_.deleteTransition(*dynamic_cast<Transition *>(&node2));

    // FIXME: update final condition (node2 was merged into node1)
  }


  void ComponentObserver::initializeNodeNameHistory(Node & node)
  {
    deque<string> history = node.getNameHistory();
    for (deque<string>::iterator it = history.begin(); it != history.end();
	 ++it)
      {
	assert((net_.nodesByName_.find(*it))->second == &node ||
	       net_.nodesByName_.find(*it) == net_.nodesByName_.end());
	net_.nodesByName_[*it] = &node;
      }
  }


  void ComponentObserver::finalizeNodeNameHistory(Node & node,
						  const deque<string> & history)
  {
    for (deque<string>::const_iterator it = history.begin();
	 it != history.end(); ++it)
      net_.nodesByName_.erase(*it);
  }


  void ComponentObserver::initializePlaceType(Place & place)
  {
    switch (place.getType())
      {
      case Node::INTERNAL:
	net_.internalPlaces_.insert(&place); break;
      case Node::INPUT:
	net_.inputPlaces_.insert(&place);
	net_.interfacePlaces_.insert(&place);
	break;
      case Node::OUTPUT:
	net_.outputPlaces_.insert(&place);
	net_.interfacePlaces_.insert(&place);
	break;
      }
  }


  void ComponentObserver::finalizePlaceType(Place & place, Node::Type type)
  {
    switch (type)
      {
      case Node::INTERNAL:
	net_.internalPlaces_.erase(&place); break;
      case Node::INPUT:
	net_.inputPlaces_.erase(&place);
	net_.interfacePlaces_.erase(&place);
	break;
      case Node::OUTPUT:
	net_.outputPlaces_.erase(&place);
	net_.interfacePlaces_.erase(&place);
	break;
      }
  }


  /****************************************************************************
   *** Class PetriNet Function Defintions
   ***************************************************************************/

  /*!
   * Reads a Petri net from a stream (in most cases backed by a file). The
   * format
   * of the stream data is not determined automatically. You have to set it
   * explicitly using PetriNet::setFormat().
   */
  istream & operator>>(istream & is, PetriNet & net)
  {
    switch (net.format_)
      {
      case PetriNet::FORMAT_OWFN:
	{
	  parser::owfn::Parser parser;
	  parser::owfn::Visitor visitor;
	  parser.parse(is).visit(visitor);
	  net = visitor.getPetriNet();
	  break;
	}

      default:
	assert(false);  // unsupported input format
      }

    return is;
  }


  /*!
   */
  PetriNet::PetriNet() :
    format_(FORMAT_OWFN), observer_(*this)
  {
  }


  /*!
   * The copy constructor with deep copy.
   */
  PetriNet::PetriNet(const PetriNet & net) :
    format_(net.format_), observer_(*this)
  {
    *this += net;
  }


  /*!
   * The destructor of the PetriNet class.
   */
  PetriNet::~PetriNet()
  {
    // delete all places
    set<Place *> places = places_;
    for (set<Place *>::iterator it = places.begin(); it != places.end(); ++it)
      deletePlace(**it);

    // delete all transitions
    set<Transition *> transitions = transitions_;
    for (set<Transition *>::iterator it = transitions.begin();
	 it != transitions.end(); ++it)
      deleteTransition(**it);

    // FIXME: possibly delete final condition
    
    assert(nodes_.empty());
    assert(nodesByName_.empty());
    assert(transitions_.empty());
    assert(places_.empty());
    assert(internalPlaces_.empty());
    assert(inputPlaces_.empty());
    assert(outputPlaces_.empty());
    assert(interfacePlaces_.empty());
    assert(interfacePlacesByPort_.empty());
    assert(arcs_.empty());
  }


  /*!
   * The "=" operator.
   */
  PetriNet & PetriNet::operator=(const PetriNet & net)
  {
    assert(this != &net);

    this->~PetriNet();
    return *new (this) PetriNet(net);
  }


  /*!
   * Adds all nodes and arcs of the second net and combines final conditions.
   *
   * \pre   the node names are unique (disjoint name sets); use 
   *        prefixNodeNames() on <em>both</em> nets to achieve this
   */
  PetriNet & PetriNet::operator+=(const PetriNet & net)
  {
    // add all transitions of the net
    for (set<Transition *>::iterator it = net.transitions_.begin();
	 it != net.transitions_.end(); ++it)
      {
	assert(!containsNode((*it)->getName()));
	new Transition(*this, observer_, **it);
      }

    // add all places
    for (set<Place *>::iterator it = net.places_.begin();
	 it != net.places_.end(); ++it)
      {
	assert(!containsNode((*it)->getName()));
	new Place(*this, observer_, **it);
      }

    // create arcs according to the arcs in the net
    for (set<Arc *>::iterator it = net.arcs_.begin(); it != net.arcs_.end();
	 ++it)
      new Arc(*this, observer_, **it);

    // FIXME: combine final conditions

    return *this;
  }


  /*!
   * Given a second Petri net #net, the internal structure is added and input
   * and output places are connected appropriatly (if an input and an output
   * place name of the two nets match).
   */
  void PetriNet::compose(const PetriNet & net, const string & prefix,
			 const string & netPrefix)
  {
    assert(prefix != netPrefix);

    PetriNet tmpNet = net;  // copy the net to prefix it

    // find the interface places to be merged
    vector<pair<Place *, Place *> > mergePlaces;
    const set<Place *> & interface = tmpNet.getInterfacePlaces();
    for (set<Place *>::iterator it = interface.begin(); it != interface.end();
	 ++it)
      {
	Place * place = findPlace((*it)->getName());
	if (place != NULL && place->isComplementType(**it))
	  mergePlaces.push_back(pair<Place *, Place *>(place, *it));
      }

    // prefix and combine the nets
    prefixNodeNames(prefix);
    tmpNet.prefixNodeNames(netPrefix);
    *this += tmpNet;

    // translate references and merge places
    for (vector<pair<Place *, Place *> >::iterator it = mergePlaces.begin();
	 it != mergePlaces.end(); ++it)
      {
	if (&it->first->getPetriNet() != this)
	  it->first = findPlace(it->first->getName());
	else if (&it->second->getPetriNet() != this)
	  it->second = findPlace(it->second->getName());
	it->first->merge(*it->second);
      }
  }


  /*!
   * \param   source  the source Node
   * \param   target  the target Node
   * \param   weight  weight of the Arc
   * \return  the newly created Arc
   */
  Arc & PetriNet::createArc(Node & source, Node & target, int weight)
  {
    return *new Arc(*this, observer_, source, target, weight);
  }


  /*!
   * If an empty name is given, one is generated using getUniqueNodeName().
   *
   * \param   name  the (initial) name of the place
   * \param   type  communication type of the place (internal or interface)
   * \return  the newly created place
   */
  Place & PetriNet::createPlace(const string & name, Node::Type type)
  {
    return *new Place(*this, observer_,
		      name.empty() ? getUniqueNodeName("p") : name, type);
  }


  /*!
   * If an empty name is given, one is generated using getUniqueNodeName().
   */
  Transition & PetriNet::createTransition(const string & name)
  {
    return *new Transition(*this, observer_, name.empty() ? getUniqueNodeName("t") : name);
  }


  /*!
   */
  void PetriNet::deleteInterfacePlaces()
  {
    set<Place *> interface = interfacePlaces_;
    for(set<Place *>::iterator it = interface.begin(); it != interface.end(); 
	++it)
      deletePlace(**it);
  }


  /*!
   */
  bool PetriNet::containsNode(Node & node) const
  {
    return nodes_.find(&node) != nodes_.end();
  }


  /*!
   */
  bool PetriNet::containsNode(const string & name) const
  {
    return findNode(name) != NULL;
  }


  /*!
   */
  Node * PetriNet::findNode(const string & name) const
  {
    map<string, Node *>::const_iterator result = nodesByName_.find(name);

    if (result != nodesByName_.end())
      return result->second;

    return NULL;
  }


  /*!
   * \return  a pointer to the place or a NULL pointer if the place was not
   *          found.
   */
  Place * PetriNet::findPlace(const string & name) const
  {
    // if we had a map placesByName we could avoid the dynamic cast here
    return dynamic_cast<Place *>(findNode(name));
  }


  /*!
   * \return  a pointer to the transition or a NULL pointer if the transition
   *          was not found.
   */
  Transition * PetriNet::findTransition(const string & name) const
  {
    // if we had a map transitionsByName we could avoid the dynamic cast here
    return dynamic_cast<Transition *>(findNode(name));
  }


  /*!
   */
  Arc * PetriNet::findArc(const Node & source, const Node & target) const
  {
    // use an arc cache to make this more efficient
    for (set<Arc *>::iterator it = arcs_.begin(); it != arcs_.end(); ++it)
      if (&(*it)->getSourceNode() == &source &&
	  &(*it)->getTargetNode() == &target)
	return *it;

    return NULL;
  }


  /*!
   */
  const set<Node *> & PetriNet::getNodes() const
  {
    return nodes_;
  }


  /*!
   */
  const set<Place *> & PetriNet::getPlaces() const
  {
    return places_;
  }


  /*!
   */
  const set<Place *> & PetriNet::getInputPlaces() const
  {
    return inputPlaces_;
  }


  /*!
   */
  const set<Place *> & PetriNet::getOutputPlaces() const
  {
    return outputPlaces_;
  }


  /*!
   */
  const set<Place *> & PetriNet::getInterfacePlaces() const
  {
    return interfacePlaces_;
  }


  /*!
   */
  const set<Transition *> & PetriNet::getTransitions() const
  {
    return transitions_;
  }


  /*!
   * \param   base  base name
   * \return  a string to be used as a name for a new node
   */
  string PetriNet::getUniqueNodeName(const string & base) const
  {
    int i = 1;
    string name;

    // use a "mutable" cache to make this more efficient
    do name = base + util::toString(i++);
    while (nodesByName_.find(name) != nodesByName_.end());
      
    return name;
  }


  /*!
   * \brief   checks whether the Petri net is free choice
   *
   *          A Petri net is free choice iff
   *            all two transitions have either disjoint or equal presets
   *
   * \return  true if the criterion is fulfilled and false if not
   */
  bool PetriNet::isFreeChoice() const
  {
    for (set<Transition *>::iterator t = transitions_.begin(); t != transitions_.end(); t++)
      for (set<Transition *>::iterator tt = transitions_.begin(); tt != transitions_.end(); tt++)
	{
	  set<Node *> t_pre  = (*t)->getPreset();
	  set<Node *> tt_pre = (*tt)->getPreset();
	  if ((t_pre != tt_pre) && !(util::setIntersection(t_pre, tt_pre).empty()))
	    return false;
	}
    return true;
  }


  /*!
   * \brief   normalizes the given Petri net
   *
   *          A Petri net (resp. open net) is called normal if
   *          each transition has only one interface place in its neighborhood.
   */
  void PetriNet::normalize()
  {

    std::string suffix = "_normalized";
    set<Place *> temp;
    set<Place *> interface = getInterfacePlaces();

    for (set<Place *>::iterator place = interface.begin(); place
	   != interface.end(); place++)
      {
	set<Node *> neighbors = util::setUnion((*place)->getPreset(), (*place)->getPostset());
	for (set<Node *>::iterator transition = neighbors.begin();
	     transition != neighbors.end(); transition++)
	  {
	    Transition *t = static_cast<Transition *>(*transition);
	    while (!t->isNormal())
	      {
		// create new internal place from interface place
		Place &newP = createPlace((*place)->getName() + suffix);

		// create new interface place
		Place &newPin = createPlace("i_" + (*place)->getName(), (*place)->getType());

		// create new transition
		Transition &newT = createTransition("t_" + (*place)->getName());

		// set arcs (1, 2 & 3)
		if ((*place)->getType() == Node::INPUT)
		  {
		    createArc(newPin, newT);
		    createArc(newT, newP);
		    for (set<Arc *>::iterator f = arcs_.begin(); f != arcs_.end(); f++)
		      {
			if (&(*f)->getSourceNode() == (*place))
			  {
			    createArc(newP, (*f)->getTargetNode());
			  }
		      }
		  }
		else
		  {
		    createArc(newT, newPin);
		    createArc(newP, newT);
		    for (set<Arc *>::iterator f = arcs_.begin(); f != arcs_.end(); f++)
		      {
			if (&(*f)->getTargetNode() == (*place))
			  {
			    createArc((*f)->getSourceNode(), newP);
			  }
		      }
		  }

		temp.insert(*place);
	      }
	  }
      }

    // remove the old interface places
    for (set<Place *>::iterator place = temp.begin(); place != temp.end(); place++)
      deletePlace(**place);
  }



  /****************************************************************************
   *** Private PetriNet Function Definitions
   ***************************************************************************/


  /*!
   * All Places and Transitions of the net are prefixed.
   *
   * \param   prefix  the prefix to be added
   */
  PetriNet & PetriNet::prefixNodeNames(const string & prefix)
  {
    for (set<Node *>::iterator it = nodes_.begin(); it != nodes_.end(); ++it)
      (*it)->prefixNameHistory(prefix);
    return *this;
  }


  void PetriNet::deletePlace(Place & place)
  {
    observer_.finalizePlaceType(place, place.getType());
    places_.erase(&place);
    deleteNode(place);
  }


  void PetriNet::deleteTransition(Transition & trans)
  {
    transitions_.erase(&trans);
    deleteNode(trans);
  }


  /*!
   * \note  Never call this method directly! Use deletePlace() or
   *        deleteTransition() instead!
   */
  void PetriNet::deleteNode(Node & node)
  {
    assert(containsNode(node));
    assert(dynamic_cast<Place *>(&node) == NULL ? true :
	   places_.find(dynamic_cast<Place *>(&node)) == places_.end());
    assert(dynamic_cast<Transition *>(&node) == NULL ? true :
	   transitions_.find(dynamic_cast<Transition *>(&node)) ==
	   transitions_.end());

    while (!node.getPreset().empty())
      deleteArc(*findArc(**node.getPreset().begin(), node));
    while (!node.getPostset().empty())
      deleteArc(*findArc(node, **node.getPostset().begin()));

    observer_.finalizeNodeNameHistory(node, node.getNameHistory());
    nodes_.erase(&node);

    delete &node;
  }


  /*!
   */
  void PetriNet::deleteArc(Arc & arc)
  {
    assert(arcs_.find(&arc) != arcs_.end());

    arcs_.erase(&arc);
    delete &arc;
  }



  /***************************** NOT YET REFACTORED *************************/


/*!
 * \brief   returns true iff all arcs connecting to n have the same weight
 *
 *          Returns true only if the given node's incoming and outgoing arcs
 *          all have the same weight.
 *
 * \param   n  node to be examined
 *
 * \pre     n != NULL
 *
 * \todo    Znamirowski: Set brackets for for-loop. The code is not readable
 *          this way.
 */
bool PetriNet::sameweights(Node *n) const
{
  /*
  assert(n != NULL);
  bool first = true;
  unsigned int w = 0;

  for (set<Arc*>::iterator f = F.begin(); f != F.end(); f++)
  {
    if ((&(*f)->getSourceNode() == n) || (&(*f)->getTargetNode() == n) )
    {
      if (first)
      {
        first=false;
        w = (*f)->getWeight();
      }
      else
      {
        if ( (*f)->getWeight() != w)
        {
          return false;
        }
      }
    }
  }
  */
  return true;
}


/*!
 * \brief   swaps input and output places
 *
 *          Swaps the input and output places, i.e. swaps the sets #inputPlaces_ and
 *          #outputPlaces_ and the adjacent arcs.
 *
 *  \test	This method has been tested in tests/test_mirror.cc.
 */
void PetriNet::mirror()
{
  /*
  // swap arcs
  for (set<Arc*>::iterator f = arcs_.begin(); f != arcs_.end(); f++)
  {
    if ( inputPlaces_.find( findPlace( ((*f)->getSourceNode()).getName()) ) != inputPlaces_.end() )
      (*f)->mirror();
    else if ( outputPlaces_.find( findPlace( ((*f)->getTargetNode()).getName()) ) != outputPlaces_.end() )
      (*f)->mirror();
  }

  // swap input and output places
  set<Place *> inputPlaces__old = inputPlaces_;
  set<Place *> outputPlaces__old = outputPlaces_;
  inputPlaces_ = outputPlaces__old;
  outputPlaces_ = inputPlaces__old;
  for (set<Place *>::iterator p = inputPlaces_.begin(); p != inputPlaces_.end(); p++)
    ;//(*p)->setType(Node::INPUT);
  for (set<Place *>::iterator p = outputPlaces_.begin(); p != outputPlaces_.end(); p++)
    ;//(*p)->setType(Node::OUTPUT);

  // swap the transitions
  for (set<Transition *>::iterator t = transitions_.begin(); t != transitions_.end(); t++)
  {
    if ( (*t)->getType() == Node::OUTPUT )
      ;//(*t)->setType(Node::INPUT);
    else if ( (*t)->getType() == Node::INPUT )
      ;//(*t)->setType(Node::OUTPUT);
  }
  */
}



/*!
 * \brief   produces a second constraint oWFN
 *
 * \param   a constraint oWFN
 *
 * \test    This method has been tested in tests/test_produce.cc.
 */
void PetriNet::produce(const PetriNet &net)
{
  /* FIXME
  // the constraint oWFN must have an empty interface
  assert(net.inputPlaces_.empty());
  assert(net.outputPlaces_.empty());


  // copy the constraint oWFN's places to the oWFN
  for (set<Place *>::iterator p = net.places_.begin(); p != net.places_.end(); p++)
  {
    Place *p_new = newPlace((*p)->getName());

    p_new->isFinal = (*p)->isFinal;
    p_new->tokens = (*p)->tokens;
  }


  // copy the constraint oWFN's unlabeled transitions to the oWFN
  for (set<Transition *>::iterator t = net.transitions_.begin(); t != net.transitions_.end(); t++)
  {
    if ( (*t)->getLabels().empty())
    {
      //Transition *t_new = newTransition((*t)->getName());

      // copy the arcs of the constraint oWFN
      for (set< Arc * >::iterator arc = net.arcs_.begin(); arc != net.arcs_.end(); arc ++)
      {
        if ( ( (*arc)->getSourceNode()->nodeType == PLACE) && ( (*arc)->getTargetNode() == static_cast<Node *>(*t) ) )
          newArc( findPlace( (*arc)->getSourceNode()->getName() ), t_new, STANDARD, (*arc)->getWeight() );
        if ( ( (*arc)->getTargetNode()->nodeType == PLACE) && ( (*arc)->getSourceNode() == static_cast<Node *>(*t) ) )
          newArc( t_new, findPlace( (*arc)->getTargetNode()->getName() ), STANDARD, (*arc)->getWeight() );
      }
    }
  }


  // transitions of the oWFN that are used as labels in the constraint oWFN
  set<string> used_labels;
  set<pair<Transition *, Transition *> > transition_pairs;


  // Traverse the used labels and store pairs of constrainted and labeled
  // transitions. If a label does not identify a transition, look for a plce
  // with the same label instead. According to its communication type, either
  // add the transitions in the preset or the postset to the transition pairs.
  // For example, if "visa" is not found as transition, but there exists an
  // input place with this name, add all transitions in the postset of this
  // place, together with the transition labeled "visa" to the transition
  // pairs.
  for (set<Transition *>::iterator t = net.transitions_.begin(); t != net.transitions_.end(); t++)
  {
    set<string> labels = (*t)->getLabels();
    for (set<string>::iterator label = labels.begin(); label != labels.end(); label++)
    {
      Transition *t_l = findTransition(*label);

      // specified transition not found -- trying places instead
      if (t_l == NULL)
      {
        Place *p = findPlace(*label);
        if (p != NULL)
        {
          set<Node *> transitions_p;

          if (p->getType() == Node::INPUT)
            transitions_p = p->getPostset();
          else
            transitions_p = p->getPreset();

          for (set<Node *>::iterator pre_transition = transitions_p.begin(); pre_transition != transitions_p.end(); pre_transition++)
          {
            used_labels.insert((*pre_transition)->getName());
            transition_pairs.insert(pair<Transition *, Transition *>(static_cast<Transition *>(*pre_transition), *t));
          }
        }
        else
        {
          std::cerr << "label " << *label << " neither describes a transition nor a place" << std::endl;
          assert(false);
        }
      }
      else
      {
        used_labels.insert(*label);
        transition_pairs.insert(pair<Transition *, Transition *>(t_l, *t));
      }
    }
  }


  // create pair transitions with their arcs
  for (set<pair<Transition *, Transition *> >::iterator tp = transition_pairs.begin(); tp != transition_pairs.end(); tp++)
  {
    // I have to comment the next line as Fiona cannot read node names with brackets
    // Transition *t_new = newTransition("(" + tp->first->getName() + "," + tp->second->getName() + ")");
    //Transition *t_new = newTransition(tp->first->getName() + "_" + tp->second->getName());

    // copy the arcs of the constraint oWFN
    for (set< Arc * >::iterator arc = net.arcs_.begin(); arc != net.arcs_.end(); arc ++)
    {
      if ( ( (*arc)->getSourceNode()->nodeType == PLACE) && ( (*arc)->getTargetNode() == static_cast<Node *>(tp->second) ) )
        newArc( findPlace( (*arc)->getSourceNode()->getName() ), t_new, STANDARD, (*arc)->getWeight() );
      if ( ( (*arc)->getTargetNode()->nodeType == PLACE) && ( (*arc)->getSourceNode() == static_cast<Node *>(tp->second) ) )
        newArc( t_new, findPlace( (*arc)->getTargetNode()->getName() ), STANDARD, (*arc)->getWeight() );
    }

    // copy the arcs of the oWFN
    for (set< Arc * >::iterator arc = arcs_.begin(); arc != arcs_.end(); arc ++)
    {
      if ( ( (*arc)->getSourceNode()->nodeType == PLACE) && ( (*arc)->getTargetNode() == static_cast<Node *>(tp->first) ) )
        newArc( findPlace( (*arc)->getSourceNode()->getName() ), t_new, STANDARD, (*arc)->getWeight() );
      if ( ( (*arc)->getTargetNode()->nodeType == PLACE) && ( (*arc)->getSourceNode() == static_cast<Node *>(tp->first) ) )
        newArc( t_new, findPlace( (*arc)->getTargetNode()->getName() ), STANDARD, (*arc)->getWeight() );
    }
  }


  // remove transitions that are used as labels
  for (set<string>::iterator t = used_labels.begin(); t != used_labels.end(); t++)
    deleteTransition(*findTransition(*t));
  */
}






/*!
 * \brief   checks a marking m for final condition
 *
 * \param   Marking m
 *
 * \return  TRUE iff m fulfills the final condition
 */
bool PetriNet::checkFinalCondition(Marking &m) const
{
  //FIXME: return finalcondition->evaluate(m);
  return false;
}





/*!
 * \brief   checks whether the Petri net is a workflow net or not
 *
 *          A Petri net N is a workflow net iff
 *            (1) there is exactly one place with empty preset
 *            (2) there is exactly one place with empty postset
 *            (3) all other nodes are situated on a path between the places described in (1) and (2)
 *
 * \return  true iff (1), (2) and (3) are fulfilled
 * \return  false in any other case
 */
bool PetriNet::isWorkflow() const
{
  Place *first = NULL;
  Place *last  = NULL;

  // finding places described in condition (1) & (2)
  for (set<Place *>::const_iterator p = places_.begin(); p != places_.end(); p++)
  {
    if ((*p)->getPreset().empty())
    {
      if (first == NULL)
        first = *p;
      else
      {
        std::cerr << "This net is no workflow net because there are more than one place with empty preset!\n";
        return false;
      }
    }
    if ((*p)->getPostset().empty())
    {
      if (last == NULL)
        last = *p;
      else
      {
        std::cerr << "This net is no workflow net because there are more than one place with empty postset!\n";
        return false;
      }
    }
  }
  if (first == NULL || last == NULL)
  {
    std::cerr << "No workflow net! Either no place with empty preset or no place with empty postset found.\n";
    return false;
  }

  // insert new transition which consumes from last and produces on first to form a cycle
  //Transition *tarjan = &createTransition("tarjan");
  //FIXME
  //newArc(last, tarjan);
  //  newArc(tarjan, first);

  // each 2 nodes \f$x,y \in P \cup T\f$ are in a directed cycle
  // (strongly connected net using tarjan's algorithm)
  unsigned int i = 0; ///< count index
  map<Node *, int> index; ///< index property for nodes
  map<Node *, unsigned int> lowlink; ///< lowlink property for nodes
  set<Node *> stacked; ///< the stack indication for nodes
  stack<Node *> S; ///< stack used by Tarjan's algorithm

  // set all nodes' index values to ``undefined''
  for (set<Place *>::const_iterator p = places_.begin(); p != places_.end(); p++)
    index[*p] = (-1);
  for (set<Transition *>::const_iterator t = transitions_.begin(); t != transitions_.end(); t++)
    index[*t] = (-1);

  // getting the number of strongly connected components reachable from first
  unsigned int sscCount = dfsTarjan(first, S, stacked, i, index, lowlink);

  std::cout << "\nstrongly connected components: " << sscCount << "\n\n";

  //deleteTransition(*tarjan);

  // check set $P \cup T$
  set<Node *> nodeSet;
  for (set<Place *>::const_iterator p = places_.begin(); p != places_.end(); p++)
    nodeSet.insert(*p);
  for (set<Transition *>::const_iterator t = transitions_.begin(); t != transitions_.end(); t++)
    nodeSet.insert(*t);

  /* true iff only one strongly connected component found from first and all nodes
   * of $\mathcal{N}$ are members of this component
   */
  if (sscCount == 1 && util::setDifference(nodeSet, stacked).empty())
    return true;
  else
  {
    cerr << "No workflow net! Some places are not between the the preset-less and the postset-less place.\n";
    return false;
  }
}




/*!
 * \brief   DFS on the net using Tarjan's algorithm.
 *
 * \param   Node n which is to check
 * \param   Stack S is the stack used in the Tarjan algorithm
 * \param   Set stacked which is needed to identify a node which is already stacked
 * \param   int \f$i \in \fmathbb{N}\f$ which is the equivalent to Tarjan's index variable
 * \param   Map index which describes the index property of a node
 * \param   Map lowlink which describes the lowlink property of a node
 *
 * \return  the number of strongly connected components reachable from the start node.
 *
 * \note    Used by method isWorkflowNet() to check condition (3) - only working for this method.
 */
unsigned int PetriNet::dfsTarjan(Node *n, stack<Node *> &S, set<Node *> &stacked, unsigned int &i, map<Node *, int> &index, map<Node *, unsigned int> &lowlink) const
{
  unsigned int retVal = 0;

  index[n] = i;
  lowlink[n] = i;
  i++;
  S.push(n);
  stacked.insert(n);
  std::cout << n->getName() << " stacked, ";
  for (set<Node *>::const_iterator nn = n->getPostset().begin(); nn != n->getPostset().end(); nn++)
  {
    if (index[*nn] < 0)
    {
      retVal += dfsTarjan(*nn, S, stacked, i, index, lowlink);
      lowlink[n] = min(lowlink[n], lowlink[*nn]);
    }
    else
    {
      if (stacked.count(*nn) > 0)
        lowlink[n] = min(lowlink[n], lowlink[*nn]);
    }
  }
  if (static_cast<int>(lowlink[n]) == index[n])
  {
    retVal++;
    std::cout << "\nSCC: ";
    while (!S.empty() && lowlink[S.top()] == lowlink[n])
    {
      std::cout << S.top()->getName() << ", ";
      S.pop();
    };
  }

  return retVal;
}







/*!
 * \brief   calculates the current marking m
 *
 * \return  Marking m which represents the current Marking m
 *          if the net
 *
 * \note    method uses the information in the property token of
 *          each place.
 */
Marking PetriNet::calcCurrentMarking() const
{
  /* FIXME
  unsigned int placeCount = initMarking(); // initializes the places and returns the number of places
  Marking *m = new Marking(placeCount, 0);

  for (set<Place *>::const_iterator p = places_.begin(); p != places_.end(); p++)
    (*m)[(*p)->marking_id] = (*p)->tokens;
  for (set<Place *>::const_iterator p = inputPlaces_.begin(); p != inputPlaces_.end(); p++)
    (*m)[(*p)->marking_id] = (*p)->tokens;
  for (set<Place *>::const_iterator p = outputPlaces_.begin(); p != outputPlaces_.end(); p++)
    (*m)[(*p)->marking_id] = (*p)->tokens;

  return *m;
  */

  return *new Marking();
}




/*!
 * \brief   recalculates a marking m to its corresponding place tokens
 *
 * An initial markings was described by setting the tokens variable of each place
 * to one specific value. Now, we can describe Markings without using those variables,
 * so probably it is necessary to translate the new markings to the old fashion.
 *
 * \param   Marking m which represents the new markings
 */
void PetriNet::marking2Places(Marking &m)
{
  /* FIXME
  for (set<Place *>::iterator p = places_.begin(); p != places_.end(); p++)
    (*p)->tokens = m[(*p)->marking_id];
  for (set<Place *>::iterator p = inputPlaces_.begin(); p != inputPlaces_.end(); p++)
      (*p)->tokens = m[(*p)->marking_id];
  for (set<Place *>::iterator p = outputPlaces_.begin(); p != outputPlaces_.end(); p++)
      (*p)->tokens = m[(*p)->marking_id];
  */
}




/*!
 * \brief   calculates the successor marking m' to m under Transition t
 *
 * \param   Marking &m current marking
 * \param   Transition &t defines the scope
 *
 * \return  Marking &m' successor marking
 */
Marking & PetriNet::successorMarking(Marking &m, Transition *t) const
{
  Marking *mm = new Marking(m);
  set<Arc *> activators; ///< set to gather arcs

  // gather all arcs in F that connect a place p in the preset of t with t
  for (set<Arc *>::const_iterator f = arcs_.begin(); f != arcs_.end(); f++)
    if (&(*f)->getTargetNode() == t)
    {
      Place *p = static_cast<Place *>(&(*f)->getSourceNode());
      if ((*mm)[p] >= (*f)->getWeight())
        activators.insert(*f);
      else
        break;
    }

  // if m does not activate t then m' = m
  if (t->getPreset().size() != activators.size())
    return *mm;

  for (set<Arc *>::const_iterator f = arcs_.begin(); f != arcs_.end(); f++)
    if (&(*f)->getSourceNode() == t)
    {
      Place *p = static_cast<Place *>(&(*f)->getTargetNode());
      (*mm)[p] += (*f)->getWeight();
    }

  for (set<Arc *>::iterator f = activators.begin(); f != activators.end(); f++)
  {
    Place *p = static_cast<Place *>(&(*f)->getSourceNode());
    (*mm)[p] -= (*f)->getWeight();
  }

  return *mm;
}




/*!
 * \brief   Checks if m activates t
 *
 * A transition t is activated by m if all places of the preset of t
 * are marked as high as the arc weights from those to t.
 *
 * \param   Marking m
 * \param   Transition t
 *
 * \return  true iff m activates t.
 */
bool PetriNet::activates(Marking &m, Transition &t) const
{
  if (t.getPreset().empty())
    return false;

  bool result = false;

  for (set<Node *>::const_iterator p = t.getPreset().begin(); p != t.getPreset().end(); p++)
  {
    Place *pp = static_cast<Place *>(*p);
    Arc *pt = findArc(*pp, t);
    if (pt->getWeight() <= m[pp])
    {
      result = true;
      break;
    }
  }

  return result;
}




/*!
 * \brief     looks for a living transition under marking m
 *
 * \param     Marking m
 *
 * \return    Transition t if t is living under m and NULL if not
 */
Transition *PetriNet::findLivingTransition(Marking &m) const
{
  Transition *result = NULL;

  for (set<Arc *>::const_iterator f = arcs_.begin(); f != arcs_.end(); f++)
  {
    //Node *n = (*f)->getSourceNode();
    /* FIXME
    if (n->nodeType == PLACE)
    {
      Place *p = static_cast<Place *>((*f)->getSourceNode());
      if (m[p->marking_id] >= (*f)->getWeight())
      {
        result = static_cast<Transition *>((*f)->getTargetNode());
        break;
      }
    }
    */
  }

  return result;
}




/*!
 * \brief   Sets the final condition
 */
void PetriNet::setFinalCondition(Formula *fc)
{
  finalCondition_ = fc;
}




}
