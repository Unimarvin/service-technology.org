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
 * \file bpel-attributes.h
 *
 * \todo
 *       - (reinert) Comment this file and its classes.
 */


#ifndef BPEL_ATTRIBUTES_H
#define BPEL_ATTRIBUTES_H


// generated by Kimwitu++
#include "bpel-kc-k.h" // phylum definitions
#include "bpel-kc-yystype.h" // data types for tokens and non-terminals

#include <map>

/// names attributes
const std::string A__CONDITION = "condition";
const std::string A__ENDPOINT_REFERENCE = "endpointReference";
const std::string A__EXPRESSION = "expression";
const std::string A__FAULT_NAME = "faultName";
const std::string A__FOR = "for";
const std::string A__NAME = "name";
const std::string A__OPERATION = "operation";
const std::string A__PART = "part";
const std::string A__PARTNER_LINK = "partnerLink";
const std::string A__PORT_TYPE = "portType";
const std::string A__PROPERTY = "property";
const std::string A__QUERY = "query";
const std::string A__VARIABLE = "variable";
const std::string A__TARGET_NAMESPACE = "targetNamespace";
const std::string A__UNTIL = "until";
/// yes or no

const std::string A__OPAQUE = "opaque";

const std::string A__ABSTRACT_PROCESS = "abstractProcess";
const std::string A__CREATE_INSTANCE = "createInstance";
const std::string A__ENABLE_INSTANCE_COMPENSATION = "enableInstanceCompensation";
const std::string A__INITIATE = "initiate";
const std::string A__SUPPRESS_JOIN_FAILURE = "suppressJoinFailure";
const std::string A__VARIABLE_ACCESS_SERIALIZABLE = "variableAccessSerializable";

/// types of attributes
const std::string T__NCNAME = "NCName";
const std::string T__QNAME = "QName";
const std::string T__ANYURI = "anyURI";
const std::string T__DURATION_EXPR = "bpws:tDuration-expr";
const std::string T__DEADLINE_EXPR = "bpws:tDeadline-expr";
const std::string T__BOOLEAN_EXPR = "bpws:tBoolean-expr";
const std::string T__STRING = "string";
const std::string T__ROLES = "bpws:tRoles";
const std::string T__BOOLEAN = "bpws:tBoolean";

class attributeManager
{
  private:
    /// an array to store attributes of XML-elements
    std::map<unsigned int, std::map<std::string, std::string> > scannerResult;
	
	///
	void traceAM(std::string traceMsg);
	
	/// 
	void printErrorMsg(std::string errorMsg);

	///
	void checkAttributeValueYesNo(std::string attributeName, std::string attributeValue);	
	
	///
	bool isValidAttributeValue(std::string attributeName, std::string attributeValue);
  
  public:
    /// constructor
    attributeManager();

    /// consecutive enumeration of XML-elements
    unsigned int nodeId;

    /// returns the value of an attribute
    kc::casestring read(kc::integer elementId, std::string attributeName);
    
    /// returns the value of an attribute
    kc::casestring read(kc::integer elementId, std::string attributeName, kc::casestring defaultValue);    

    /// increases the id
    kc::integer nextId();

    /// defines an attribute
    void define(kc::casestring attributeName, kc::casestring attributeValue);

    ///
    void check(kc::integer elementId, unsigned int elementType);
    
    /// special check function 
    void check(kc::integer elementId, kc::casestring literalValue, unsigned int elementType);
};




/*
    list of default values of attributes:
    -------------------------------------
    tProcess.queryLanguage = http://www.w3.org/TR/1999/REC-xpath-19991116
            .expressionLanguage = http://www.w3.org/TR/1999/REC-xpath-19991116
            .suppressJoinFailure = no
            .enableInstanceCompensation = no
            .abstractProcess = no

    tActivity.suppressJoinFailure = no

    tCorrelation.initiate = no

    tReceive.createInstance = no

    tPick.createInstance = no

    tScope.variableAccessSerializable = no
*/

/*
    list of data types of attributes:
    ---------------------------------
    tBoolean-expr
      tActivity.joinCondition
      tSource.transitionCondition
      tCase.condition
      tWhile.condition

    tDuration-expr
      tOnAlarm.for
      tWait.for

    tDeadline-expr
      tOnAlarm.until
      tWait.until

    tBoolean (yes|no)
      tProcess.suppressJoinFailure, tProcess.enableInstanceCompensation, tProcess.abstractProcess
      tActivity.suppressJoinFailure
      tCorrelation.initiate
      tReceive.createInstance
      tFrom.opaque
      tTo.opaque
      tPick.createInstance
      tScope.variableAccessSerializable

    tRoles (myRole|partnerRole)
      tFrom.endpointReference
      tTo.endpointReference
*/

#endif
