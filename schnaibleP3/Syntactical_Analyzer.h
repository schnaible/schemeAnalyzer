/*******************************************************************************
* File name: Syntactical_Analyzer.h                                            *
* Project: CS 460 Project 3 Fall 2021                                          *
* Author: Dr. Watts                                                            *
* Modified by: Kyler Schnaible                                                 *
* Date: Dec. 14, 2021                                                          *
* Description: This file contains private and public function/variable names   *
* that are used in the Syntatical_Analyzer.cpp                                 *
*******************************************************************************/

#ifndef SYNTACTICALANALYZER_H
#define SYNTACTICALANALYZER_H

#include <iostream>
#include <fstream>
#include <vector>
#include "Lexical_Analyzer.h"
#include "Code_Generator.h"

using namespace std;

class Syntactical_Analyzer
{
public:
	Syntactical_Analyzer(char *filename);
	~Syntactical_Analyzer();

private:
	Lexical_Analyzer *lex;
	Code_Generator *cg;
	token_type token;
	ofstream p2file;
	int program();
	int more_defines();
	int define();
	int stmt_list();
	int stmt();
	int literal();
	int quoted_lit();
	int more_tokens();
	int param_list();
	int else_part();
	int stmt_pair();
	int stmt_pair_body();
	int assign_pair();
	int more_assigns();
	int action();
	int any_other_token();

	string operatorSymbol;		  // We use this to detmine what the current operator symbol is, such as + or ,
	vector<string> operatorStack; // A stack that we will use to keep track of which operator (mostly for arithmetic) we are doing, especially for more than 2 operands such as (+ 1 3 6).
	int depth = 0;				  // This is to keep track of how far into a statement we have gone so we do not write too many parenthesis or semicolons & newlines.
	bool calledDislay;			  // Let's us know if we've entered a `cout <<` so we do not write __RetVal =
};

#endif
