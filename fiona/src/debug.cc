// *****************************************************************************\
// * Copyright 2005, 2006 Niels Lohmann, Peter Massuthe, Daniela Weinberg      *
// *                                                                           *
// * This file is part of Fiona.                                               *
// *                                                                           *
// * Fiona is free software; you can redistribute it and/or modify it          *
// * under the terms of the GNU General Public License as published by the     *
// * Free Software Foundation; either version 2 of the License, or (at your    *
// * option) any later version.                                                *
// *                                                                           *
// * Fiona is distributed in the hope that it will be useful, but WITHOUT      *
// * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
// * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for  *
// * more details.                                                             *
// *                                                                           *
// * You should have received a copy of the GNU General Public License along   *
// * with Fiona; if not, write to the Free Software Foundation, Inc., 51       *
// * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.                      *
// *****************************************************************************/

/*!
 * \file	debug.cc
 *
 * \brief	Some debugging tools for Fiona
 * 
 * \author  responsible: Daniela Weinberg <weinberg@informatik.hu-berlin.de>
 *
 * \note    This file is part of the tool Fiona and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/forschung/projekte/tools4bpel
 *          for details.
 */

#include "mynew.h"
#include "options.h"
#include "debug.h"

#include <string>
#include <iostream>

/// debug level
trace_level debug_level = TRACE_0;

/**
 * Provides output to stderr using different #trace_level 
 * (in order to regulate amount of output)
 *
 * \param pTraceLevel	the #trace_level
 * \param message	the output
 *
 */
void trace(trace_level pTraceLevel, std::string message) {
  if (pTraceLevel <= debug_level) {
    (*log_output) << message << std::flush;
  }
}

/**
 * Works like #trace(trace_level,std::string) with trace_level = TRACE_ALWAYS
 *
 * \param message the output
 *
 */
void trace(std::string message ) {
  trace(TRACE_0, message);
}

/*!
 * \param i standard C int
 * \return  C++ string representing i
 */
std::string intToString(int i) {
  char buffer[20];
  sprintf(buffer, "%d", i);
  
  return std::string(buffer);
}

/*!
 * This function is invoked by the parser and the lexer during the syntax
 * analysis. When an error occurs, it prints an accordant message and shows the
 * lines of the input files where the error occured.
 *
 * \param msg a message (mostly "Parse error") and some more information e.g.
 *            the location of the syntax error.
 * \return 1, since an error occured
 */
int yyerror(const char* msg)
{
  /* defined by flex */
  extern int yylineno;      ///< line number of current token
  extern char *yytext;      ///< text of the current token
  extern char * netfile;

  trace("Error while parsing!\n\n");
  trace(msg);
  trace("\n");
	
  // display passed error message
  trace("Error in '" + std::string(netfile) + "' in line ");
  trace(intToString(yylineno));
  trace(":\n");
  trace("  token/text last read was '");
  trace(yytext);
  trace("'\n\n");
  
  // if (filename != "<STDIN>")
  {
    trace("-------------------------------------------------------------------------------\n");
    
    // number of lines to print before and after errorneous line
    int environment = 4;

    unsigned int firstShowedLine = ((yylineno-environment)>0)?(yylineno-environment):1;
  
    std::ifstream inputFile(netfile);
    std::string errorLine;
    for (unsigned int i=0; i<firstShowedLine; i++)
    {
      getline(inputFile, errorLine);
    }
    // print the erroneous line (plus/minus three more)
    for (unsigned int i=firstShowedLine; i<=firstShowedLine+(2*environment); i++)
    {
      trace(intToString(i) + ": " + errorLine + "\n");
      getline(inputFile, errorLine);
      if (inputFile.eof())
	break;
    }
    inputFile.close();
    
    trace("-------------------------------------------------------------------------------\n");
  }

  exit(1);
  return 1;
}
