#include <cassert>

#include "util.h"
#include "petrinet.h"
#include "component.h"


namespace pnapi {


  /****************************************************************************
   *** Class Node Function Definitions
   ***************************************************************************/


  /*!
   */
  Node::Node(PetriNet & net, ComponentObserver & observer, const string & name,
	     Type type) :
    net_(net), observer_(observer), type_(type)
  {
    assert(&observer.getPetriNet() == &net);

    history_.push_back(name);
  }


  /*!
   */
  Node::Node(PetriNet & net, ComponentObserver & observer, const Node & node) :
    net_(net), observer_(observer), type_(node.type_), history_(node.history_)
  {
    assert(&observer.getPetriNet() == &net);
  }


  /*!
   * Do not delete a node directly but ask the net instead:
   * PetriNet::deleteNode()
   */
  Node::~Node()
  {
    assert(!net_.containsNode(*this));
  }


  /*!
   */
  Node::Type Node::getType() const
  {
    return type_;
  }


  /*!
   */
  void Node::setType(Type type)
  {
    //assert(type != type_);

    type_ = type;
  }


  /*!
   */
  bool Node::isComplementType(Type type) const
  {
    return (type_ == INPUT && type == OUTPUT) ||
           (type_ == OUTPUT && type == INPUT);
  }


  /*!
   */
  string Node::getName() const
  {
    return *history_.begin();
  }


  /*!
   */
  void Node::prefixNameHistory(const string & prefix)
  {
    deque<string> oldHistory = history_;
    for (deque<string>::iterator it = history_.begin(); it != history_.end();
	 ++it)
      *it = prefix + *it;
    observer_.updateNodeNameHistory(*this, oldHistory);
  }


  /*!
   */
  deque<string> Node::getNameHistory() const
  {
    return history_;
  }


  /*!
   */
  PetriNet & Node::getPetriNet() const
  {
    return net_;
  }


  /*!
   */
  const set<Node *> & Node::getPreset() const
  {
    return preset_;
  }


  /*!
   */
  const set<Node *> & Node::getPostset() const
  {
    return postset_;
  }


  void Node::merge(Node & node, bool addArcWeights)
  {
    assert(&net_ == &node.net_);

    mergeNameHistory(node);

    mergeArcs(*this, node, preset_, node.preset_, addArcWeights, false);
    mergeArcs(*this, node, postset_, node.postset_, addArcWeights, true);

    observer_.updateNodesMerged(*this, node);
  }


  void Node::mergeNameHistory(Node & node)
  {
    // remove history of node
    deque<string> nodeHistory = node.history_;
    node.history_.clear();
    observer_.updateNodeNameHistory(node, nodeHistory);

    // add history of node to this
    deque<string> oldHistory = history_;
    history_.insert(history_.end(), nodeHistory.begin(), nodeHistory.end());
    observer_.updateNodeNameHistory(*this, oldHistory);
  }


  void Node::mergeArcs(Node & node1, Node & node2, const set<Node *> & set1,
		       const set<Node *> & set2, bool addWeights,
		       bool isPostset)
  {
    for (set<Node *>::iterator it = set2.begin(); it != set2.end(); ++it)
      {
	Node & node1Source = isPostset ? node1 : **it;
	Node & node2Source = isPostset ? node2 : **it;
	Node & node1Target = isPostset ? **it  : node1;
	Node & node2Target = isPostset ? **it  : node2;

	Arc * arc1 = net_.findArc(node1Source, node1Target);
	Arc * arc2 = net_.findArc(node2Source, node2Target);

	assert(arc2 != NULL);
	if (arc1 == NULL)
	  net_.createArc(node1Source, node1Target, arc2->getWeight());
	else if (addWeights)
	  arc1->merge(*arc2);
	else
	  assert(arc1->getWeight() == arc2->getWeight());
      }
  }



  /****************************************************************************
   *** Class Place Function Definitions
   ***************************************************************************/


  /*!
   */
  Place::Place(PetriNet & net, ComponentObserver & observer,
	       const string & name, Type type, unsigned int tokens,
	       unsigned int capacity) :
    Node(net, observer, name, type), tokens_(tokens), capacity_(capacity),
    wasInterface_(type == INTERNAL ? false : true)
  {
    observer_.updatePlaces(*this);
    setType(type);
  }


  /*!
   */
  Place::Place(PetriNet & net, ComponentObserver & observer,
	       const Place & place) :
    Node(net, observer, place), tokens_(place.tokens_),
    capacity_(place.capacity_), wasInterface_(place.wasInterface_)
  {
    observer_.updatePlaces(*this);
    setType(getType());
  }


  /*!
   */
  void Place::setType(Type type)
  {
    assert(type != INOUT);  // INOUT may only be used for transitions

    Type oldType = getType();
    Node::setType(type);
    observer_.updatePlaceType(*this, oldType);
  }


  /*!
   */
  unsigned int Place::getTokenCount() const
  {
    return tokens_;
  }


  /*!
   */
  unsigned int Place::getCapacity() const
  {
    return capacity_;
  }


  /*!
   * Merges the properties of the given node into this one. The merged place
   * is deleted.
   *
   * The following properties are merged:
   * - NameHistory (concatenation)
   * - WasInterface (disjunction)
   * - Capacity (maximum)
   * - TokenCount (maximum)
   * - Pre-/Postset (union)
   *
   * \pre   the places must reside in the same PetriNet instance
   * \post  the place is internal after merging
   */
  void Place::merge(Place & place, bool addArcWeights)
  {
    // be sure to internalize this place
    if (getType() != INTERNAL)
      setType(INTERNAL);

    // merge place properties
    wasInterface_ = wasInterface_ || place.wasInterface_;
    capacity_ = util::max(capacity_, place.capacity_);
    tokens_ = util::max(tokens_, place.tokens_);

    // Node::merge does all the rest
    Node::merge(place, addArcWeights);
  }


  /*!
   * \brief   Sets the number of token to the place
   *
   * \note    This method is experimentally used!
   */
  void Place::mark(const unsigned int &t)
  {
    tokens_ = t;
  }



  /****************************************************************************
   *** Class Transition Function Definitions
   ***************************************************************************/


  /*!
   */
  Transition::Transition(PetriNet & net, ComponentObserver & observer,
			 const string & name, Type type) :
    Node(net, observer, name, type)
  {
    observer_.updateTransitions(*this);
  }


  /*!
   */
  Transition::Transition(PetriNet & net, ComponentObserver & observer,
			 const Transition & trans) :
    Node(net, observer, trans)
  {
    observer_.updateTransitions(*this);
  }


  /*!
   * \brief   checks if the transition is normal
   *
   * \note    This is a help method for normalize method
   *          in class PetriNet.
   */
  bool Transition::isNormal() const
  {
    int counter = 0;

    for (set<Node *>::const_iterator p = getPreset().begin(); p != getPreset().end(); p++)
      if ((*p)->getType() != Node::INTERNAL)
	counter++;
    for (set<Node *>::const_iterator p = getPostset().begin(); p != getPostset().end(); p++)
      if ((*p)->getType() != Node::INTERNAL)
	counter++;

    return counter <= 1;
  }


  /*!
   * Merges the properties of the given node into this one. The merged
   * transition is deleted.
   *
   * The following properties are merged:
   * - NameHistory (concatenation)
   * - Pre-/Postset (union)
   *
   * \pre   the transitions must reside in the same PetriNet instance
   */
  void Transition::merge(Transition & trans, bool addArcWeights)
  {
    // Node::merge does all the work
    Node::merge(trans, addArcWeights);
  }


  /*!
   */
  void Transition::updateType()
  {
    Type newType = INTERNAL;
    set<Node *> neighbors = util::setUnion(getPreset(), getPostset());

    for (set<Node *>::iterator it = neighbors.begin(); it != neighbors.end();
	 ++it)
      {
	Type itType = (*it)->getType();
	if (itType == INPUT || itType == OUTPUT)
	  {
	    if ((*it)->isComplementType(newType))
	      {
		newType = INOUT;
		break;
	      }
	    else
	      newType = itType;
	  }
      }

    setType(newType);
  }



  /****************************************************************************
   *** Class Arc Function Definitions
   ***************************************************************************/


  /*!
   */
  Arc::Arc(PetriNet & net, ComponentObserver & observer, Node & source,
	   Node & target, unsigned int weight) :
    net_(net), observer_(observer), source_(source), target_(target),
    weight_(weight)
  {
    assert(&observer.getPetriNet() == &net);

    observer_.updateArcCreated(*this);
  }


  /*!
   */
  Arc::Arc(PetriNet & net, ComponentObserver & observer, const Arc & arc) :
    net_(net), observer_(observer),
    source_(*net.findNode(arc.source_.getName())),
    target_(*net.findNode(arc.target_.getName())), weight_(arc.weight_)
  {
    assert(&observer.getPetriNet() == &net);
    assert(net.findNode(arc.source_.getName()) != NULL);
    assert(net.findNode(arc.target_.getName()) != NULL);

    observer_.updateArcCreated(*this);
  }


  /*!
   * You must not destroy an Arc directly.
   */
  Arc::~Arc()
  {
    assert(net_.findArc(source_, target_) == NULL);

    observer_.updateArcRemoved(*this);
  }


  /*!
   */
  PetriNet & Arc::getPetriNet() const
  {
    return net_;
  }


  /*!
   */
  Node & Arc::getSourceNode() const
  {
    return source_;
  }


  /*!
   */
  Node & Arc::getTargetNode() const
  {
    return target_;
  }


  /*!
   */
  Transition & Arc::getTransition() const
  {
    Transition * t = dynamic_cast<Transition *>(&source_);
    if (t != NULL)
      return *t;

    t = dynamic_cast<Transition *>(&target_);
    assert(t != NULL);
    return *t;
  }


  /*!
   */
  Place & Arc::getPlace() const
  {
    Place * t = dynamic_cast<Place *>(&source_);
    if (t != NULL)
      return *t;

    t = dynamic_cast<Place *>(&target_);
    assert(t != NULL);
    return *t;
  }


  /*!
   */
  unsigned int Arc::getWeight() const
  {
    return weight_;
  }


  /*!
   */
  void Arc::merge(Arc & arc)
  {
    weight_ += arc.weight_;
  }

}
