/*****************************************************************************\
 Wendy -- Calculating Operating Guidelines

 Copyright (C) 2009  Niels Lohmann <niels.lohmann@uni-rostock.de>

 Wendy is free software: you can redistribute it and/or modify it under the
 terms of the GNU Affero General Public License as published by the Free
 Software Foundation, either version 3 of the License, or (at your option)
 any later version.

 Wendy is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
 more details.

 You should have received a copy of the GNU Affero General Public License
 along with Wendy.  If not, see <http://www.gnu.org/licenses/>. 
\*****************************************************************************/


%token KW_STATE KW_PROG COLON COMMA ARROW NUMBER NAME

%defines
%name-prefix="graph_"

%{
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "InnerMarking.h"
#include "Label.h"
#include "Cover.h"
#include "cmdline.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

/// the current NAME token as string
std::string NAME_token;

/// the vector of the successor state numbers of the current marking
std::vector<InnerMarking_ID> currentSuccessors;

/// the labels of the outgoing edges of the current marking
std::vector<Label_ID> currentLabels;

/// names of transitions, that are enabled under the current marking (needed for cover)
std::set<std::string> currentTransitions;

/// a marking of the PN API net
std::map<const pnapi::Place*, unsigned int> marking;

extern std::ofstream *markingfile;

/// the command line parameters
extern gengetopt_args_info args_info;

extern int graph_lex();
extern int graph_error(const char *);
%}

%union {
  unsigned int val;
}

%type <val> NUMBER

%%

states:
  state
| states state
;

state:
  KW_STATE NUMBER prog markings transitions
    { InnerMarking::markingMap[$2] = new InnerMarking(currentLabels, currentSuccessors,
                                          InnerMarking::net->finalCondition().isSatisfied(pnapi::Marking(marking, InnerMarking::net)));

      if (markingfile) {
          *markingfile << $2 << ": ";
          for (std::map<const pnapi::Place*, unsigned int>::iterator p = marking.begin(); p != marking.end(); ++p) {
              if (p != marking.begin()) {
                  *markingfile << ", ";
              }
              *markingfile << p->first->getName() << ":" << p->second;
          }
          *markingfile << std::endl;
      }

      if (args_info.cover_given)
      {
        Cover::checkInnerMarking($2, marking, currentTransitions);
        currentTransitions.clear();
      }

      currentLabels.clear();
      currentSuccessors.clear();
      marking.clear(); }
;

prog:
  /* empty */
| KW_PROG NUMBER
;

markings:
  /* empty */
| NAME COLON NUMBER
    { marking[InnerMarking::net->findPlace(NAME_token)] = $3; }
| markings COMMA NAME COLON NUMBER
    { marking[InnerMarking::net->findPlace(NAME_token)] = $5; }
;

transitions:
  /* empty */
| transitions NAME ARROW NUMBER
    { 
      currentLabels.push_back(Label::name2id[NAME_token]);
      if(args_info.cover_given)
        currentTransitions.insert(NAME_token);
      currentSuccessors.push_back($4); 
    }
;
