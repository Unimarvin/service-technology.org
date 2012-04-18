/*!
\file SimpleProperty.h
\author Karsten
\status new

\brief Evaluates simple property (only SET of states needs to be computed).
Actual property is virtual, default (base class) is full exploration
*/

#pragma once

#include "Exploration/SearchStack.h"

class Firelist;
class Store;

class SimpleProperty
{
    public:
        /// evaluate property by dfs. Result true = state found, false = state not found
        bool depth_first(Store &, Firelist &);

        /// evaluate property by bfs. Result true = state found, false = state not found
        bool breadth_first(Store &, Firelist &) {}

        /// the search stack
        SearchStack stack;

    protected: 
        /// value of property in current state
        bool value;

    private:
        /// prepare for search
        virtual void initProperty();

        /// check property in Marking::Current, use after fire. Argument is transition just fired.
        virtual void checkProperty(index_t) {}

        /// check property in Marking::Current, use after backfire. Argument is transition just backfired.
        virtual void updateProperty(index_t) {}
};
