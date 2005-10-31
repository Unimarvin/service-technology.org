#ifndef PETRINET_H
#define PETRINET_H



#include <string>
#include <iostream>
#include <vector>
#include <set>
#include "bpel-kc-k.h"       // generated by kimwitu++
#include "bpel-kc-rk.h"      // generated by kimwitu++
#include "bpel-kc-unpk.h"    // generated by kimwitu++



class Node
{
  public:
    /// the (internal) name of the node
    std::string name;
    /// the set of roles (i.e. the history) of the node
    std::set<std::string> history;
};


class Arc
{
  public:
    Node *source;
    Node *target;    
    Arc(Node *source, Node *target);
};


class Transition: public Node
{
  private:
    /// type of the transition (not used yet)
    unsigned int type;

  public:
    /// constructor which adds a first role to the history
    Transition(std::string name, std::string role, unsigned int type);
};



class Place: public Node
{
  private:
    /// type of the place (not used yet)
    unsigned int type;
    
  public:
    /// constructor which adds a first role to the history
    Place(std::string name, std::string role, unsigned int type);
};



class PetriNet
{
  public:
    Place *newPlace(std::string name, std::string role, unsigned int type);
    Transition *newTransition(std::string name, std::string role, unsigned int type);
    Arc *newArc(Node *source, Node *target);

    /// statistical output
    void information();

    /// DOT (Graphviz) output
    void drawDot();

    /// merge places given two places
    void mergePlaces(Place *p1, Place *p2);
    /// merge places given two names
    void mergePlaces(std::string name1, std::string name2);
    /// merge places given two activities with roles
    void mergePlaces(kc::impl_activity* act1, std::string role1, kc::impl_activity* act2, std::string role2);
    /// merge transitions given two transitions
    void mergeTransitions(Transition *t1, Transition *t2);
    void removePlace(Place *p);
    void removeTransition(Transition *t);
    void removeArc(Arc *f);
    /// find place given a name
    Place *findPlace(std::string name);
    /// find place given a role
    Place *findPlaceRole(std::string role);
    std::set<Node *> preset(Node *n);
    std::set<Node *> postset(Node *n);

    void simplify();

    PetriNet();


  private:
    unsigned int places;
    unsigned int transitions;
    unsigned int arcs;
    std::vector<Place *> P;
    std::vector<Transition *> T;
    std::vector<Arc *> F;
};

#endif
