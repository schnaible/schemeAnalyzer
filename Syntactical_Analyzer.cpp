/*******************************************************************************
* File name: Syntactical_Analyzer.cpp                                          *
* Project: CS 460 Project 3 Fall 2021                                          *
* Author: Dr. Watts                                                            *
* Modified by: Kyler Schnaible                                                 *
* Date: Dec. 14, 2021                                                          *
* Description: This file contains the code that generates the proper C++ code  *
* that replicates the output from the given scheme code (.ss file)             *
*******************************************************************************/

#include <iostream>
#include <fstream>
#include "Syntactical_Analyzer.h"

using namespace std;

Syntactical_Analyzer::Syntactical_Analyzer(char *filename)
{
	lex = new Lexical_Analyzer(filename);
	cg = new Code_Generator(filename, lex);
	string name = filename;
	string p2name = name.substr(0, name.length() - 3) + ".p2";
	p2file.open(p2name.c_str());
	token = lex->Get_Token();
	int totalErrors = program();
}

Syntactical_Analyzer::~Syntactical_Analyzer()
{
	delete cg;
	delete lex;
	p2file.close();
}

static string token_lexemes[] = {
	"{}", "identifier", "numeric literal", "string literal", "if", "cond", "display",
	"newline", "list operation", "cons or append"
								 "and",
	"or", "not", "define",
	"let", "number?", "list?", "zero?", "null?", "string?", "modulo", "round",
	"else", "+", "-", "/", "*", "=", ">", "<", ">=", "<=", "(", ")", "'", "error",
	"end of file", "end of file", "end of file", "end of file", "end of file"};

int Syntactical_Analyzer::program()
{
	int errors = 0;

	string message;
	if (token == LPAREN_T)
	{ // Rule 1
		p2file << "Using Rule 1\n";
		token = lex->Get_Token();
		errors += define();
		if (token == LPAREN_T)
		{
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[LPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		errors += more_defines();
		if (token == EOF_T)
		{
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[EOF_T] + " expected";
			lex->Report_Error(message);
		}
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::more_defines()
{
	int errors = 0;

	string message;
	if (token == IDENT_T)
	{ // Rule 3
		p2file << "Using Rule 3\n";
		token = lex->Get_Token();
		errors += stmt_list();
		if (token == RPAREN_T)
		{
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
	}
	else if (token == DEFINE_T)
	{ // Rule 2
		p2file << "Using Rule 2\n";
		errors += define();
		if (token == LPAREN_T)
		{
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[LPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		errors += more_defines();
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::define()
{
	int errors = 0;
	bool isMain = false;

	string message;
	if (token == DEFINE_T)
	{ // Rule 4
		p2file << "Using Rule 4\n";
		token = lex->Get_Token();
		if (token == LPAREN_T)
		{
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[LPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		if (token == IDENT_T)
		{

			if (lex->Get_Lexeme() == "main")
				isMain = true;
			if (isMain)
				// write "int main" to code file
				cg->WriteCode(0, "int main (");
			else
			{
				// Object lexeme to the code file
				cg->WriteCode(0, "Object " + lex->Get_Lexeme() + "(");
			}

			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[IDENT_T] + " expected";
			lex->Report_Error(message);
		}
		errors += param_list();
		if (token == RPAREN_T)
		{
			cg->WriteCode(0, ")\n{\n");
			cg->WriteCode(1, "Object __RetVal;\n");
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		errors += stmt();
		errors += stmt_list();
		if (token == RPAREN_T)
		{
			if (isMain)
				cg->WriteCode(1, "return 0; \n");
			else
				cg->WriteCode(1, "return __RetVal; \n");
			cg->WriteCode(0, "}\n");
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::stmt_list()
{
	int errors = 0;

	string message;
	if (token == IDENT_T || token == LPAREN_T)
	{ // Rule 5
		p2file << "Using Rule 5\n";
		errors += stmt();
		// These if statements check our current operator and inserts it between 2 operands
		// ex: Scheme: (+ 2 3) --> C++: (Object(2) + Object(3))
		if (operatorSymbol == "PLUS")
		{
			cg->WriteCode(0, " + ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "MULT")
		{
			cg->WriteCode(0, " * ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "LT")
		{
			cg->WriteCode(0, " < ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "GT")
		{
			cg->WriteCode(0, " > ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "GTE")
		{
			cg->WriteCode(0, " >= ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "LTE")
		{
			cg->WriteCode(0, " <= ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "EQUAL")
		{
			cg->WriteCode(0, " == ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "AND")
		{
			cg->WriteCode(0, " && ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "OR")
		{
			cg->WriteCode(0, " || ");
			operatorSymbol = "NONE";
		}
		errors += stmt_list();
	}
	else if (token == RPAREN_T)
	{ // Rule 6

		operatorStack.clear();
		depth -= 1;
		// depth = 0;
		p2file << "Using Rule 6\n";
	}
	else if (token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T)
	{ // Rule 5
		// Using the same if statement checks as mentioned at the top of this function.
		p2file << "Using Rule 5\n";
		stmt();
		if (token != RPAREN_T)
		{
			if (operatorSymbol == "PLUS")
			{
				cg->WriteCode(0, " + ");
				operatorSymbol = "NONE";
			}
			else if (operatorSymbol == "MULT")
			{
				cg->WriteCode(0, " * ");
				operatorSymbol = "NONE";
			}
			else if (operatorSymbol == "LT")
			{
				cg->WriteCode(0, " < ");
				operatorSymbol = "NONE";
			}
			else if (operatorSymbol == "GTE")
			{
				cg->WriteCode(0, " >= ");
				operatorSymbol = "NONE";
			}
			else if (operatorSymbol == "LTE")
			{
				cg->WriteCode(0, " <= ");
				operatorSymbol = "NONE";
			}
			else if (operatorSymbol == "COMMA")
			{
				cg->WriteCode(0, ", ");
				operatorSymbol = "NONE";
			}
			else if (operatorSymbol == "AND")
			{
				cg->WriteCode(0, " && ");
				operatorSymbol = "NONE";
			}
			else if (operatorSymbol == "OR")
			{
				cg->WriteCode(0, " || ");
				operatorSymbol = "NONE";
			}
		}
		if (operatorStack.size() != 0)
		{

			if (token != RPAREN_T)
			{
				// cout << operatorStack.front() << endl;
				operatorSymbol = operatorStack.back();
			}
			else if (token == RPAREN_T)
			{
				operatorStack.pop_back();
				if (operatorStack.size() == 0)
					operatorSymbol = "NONE";
				else
					operatorSymbol = operatorStack.back();
			}
		}
		stmt_list();
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::stmt()
{
	int errors = 0;
	depth += 1;
	string message;
	if (token == IDENT_T)
	{ // Rule 8
		p2file << "Using Rule 8\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == LPAREN_T)
	{ // Rule 9
		p2file << "Using Rule 9\n";
		token = lex->Get_Token();
		errors += action();
		if (token == RPAREN_T)
		{
			// if (depth == 1)
			// 	cg->WriteCode(0, ")"); /*another one*/
			depth -= 1;
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
	}
	else if (token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T)
	{ // Rule 7
		p2file << "Using Rule 7\n";
		if (depth == 0 && calledDislay != true)
			cg->WriteCode(2, "__RetVal = ");
		errors += literal();
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::literal()
{
	int errors = 0;

	string message;
	if (token == NUMLIT_T)
	{ // Rule 10
		cg->WriteCode(0, "Object(" + lex->Get_Lexeme() + ")");

		p2file << "Using Rule 10\n";
		token = lex->Get_Token();
	}
	else if (token == STRLIT_T)
	{ // Rule 11
		cg->WriteCode(0, "Object(" + lex->Get_Lexeme() + ")");
		p2file << "Using Rule 11\n";
		token = lex->Get_Token();
	}
	else if (token == SQUOTE_T)
	{ // Rule 12
		cg->WriteCode(0, "Object(\"");
		p2file << "Using Rule 12\n";
		token = lex->Get_Token();
		errors += quoted_lit();
		cg->WriteCode(0, "\")");
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::quoted_lit()
{
	int errors = 0;

	string message;
	if (token == LPAREN_T || token == IDENT_T || token == NUMLIT_T || token == STRLIT_T || token == IF_T || token == DISPLAY_T || token == NEWLINE_T || token == LISTOP1_T || token == LISTOP2_T || token == AND_T || token == OR_T || token == NOT_T || token == DEFINE_T || token == LET_T || token == NUMBERP_T || token == LISTP_T || token == ZEROP_T || token == NULLP_T || token == STRINGP_T || token == PLUS_T || token == MINUS_T || token == DIV_T || token == MULT_T || token == MODULO_T || token == ROUND_T || token == EQUALTO_T || token == GT_T || token == LT_T || token == GTE_T || token == LTE_T || token == SQUOTE_T || token == COND_T || token == ELSE_T)
	{ // Rule 13
		p2file << "Using Rule 13\n";
		errors += any_other_token();
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::more_tokens()
{
	int errors = 0;

	string message;
	if (token == LPAREN_T || token == IDENT_T || token == NUMLIT_T || token == STRLIT_T || token == IF_T || token == DISPLAY_T || token == NEWLINE_T || token == LISTOP1_T || token == LISTOP2_T || token == AND_T || token == OR_T || token == NOT_T || token == DEFINE_T || token == LET_T || token == NUMBERP_T || token == LISTP_T || token == ZEROP_T || token == NULLP_T || token == STRINGP_T || token == PLUS_T || token == MINUS_T || token == DIV_T || token == MULT_T || token == MODULO_T || token == ROUND_T || token == EQUALTO_T || token == GT_T || token == LT_T || token == GTE_T || token == LTE_T || token == SQUOTE_T || token == COND_T || token == ELSE_T)
	{ // Rule 14
		p2file << "Using Rule 14\n";
		errors += any_other_token();
		cg->WriteCode(0, " ");
		errors += more_tokens();
	}
	else if (token == RPAREN_T)
	{ // Rule 15
		p2file << "Using Rule 15\n";
		;
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::param_list()
{
	int errors = 0;

	string message;
	if (token == IDENT_T)
	{ // Rule 16
		p2file << "Using Rule 16\n";
		cg->WriteCode(0, "Object " + lex->Get_Lexeme());
		token = lex->Get_Token();
		if (token != RPAREN_T)
			cg->WriteCode(0, ", ");
		errors += param_list();
	}
	else if (token == RPAREN_T)
	{ // Rule 17
		p2file << "Using Rule 17\n";
		;
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::else_part()
{
	int errors = 0;

	string message;
	if (token == IDENT_T || token == LPAREN_T || token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T)
	{ // Rule 18
		p2file << "Using Rule 18\n";
		cg->WriteCode(1, "else {\n");
		errors += stmt();
		cg->WriteCode(1, "}\n");
	}
	else if (token == RPAREN_T)
	{ // Rule 19
		p2file << "Using Rule 19\n";
		;
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::stmt_pair()
{
	int errors = 0;

	string message;
	if (token == LPAREN_T)
	{ // Rule 20
		p2file << "Using Rule 20\n";
		token = lex->Get_Token();
		errors += stmt_pair_body();
	}
	else if (token == RPAREN_T)
	{ // Rule 21
		p2file << "Using Rule 21\n";
		;
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::stmt_pair_body()
{
	int errors = 0;

	string message;
	if (token == ELSE_T)
	{ // Rule 23
		p2file << "Using Rule 23\n";
		token = lex->Get_Token();
		errors += stmt();
		if (token == RPAREN_T)
		{
			if (depth == 1)
				cg->WriteCode(0, ")");
			depth -= 1;
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
	}
	else if (token == IDENT_T || token == LPAREN_T || token == NUMLIT_T || token == STRLIT_T || token == SQUOTE_T)
	{ // Rule 22
		p2file << "Using Rule 22\n";
		cg->WriteCode(1, "if ("); // This is around the area we want to do the 'if' part for COND
		errors += stmt();
		cg->WriteCode(0, "){ \n ");
		if (operatorSymbol == "PLUS")
		{
			cg->WriteCode(0, " + ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "MULT")
		{
			cg->WriteCode(0, " * ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "LT")
		{
			cg->WriteCode(0, " < ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "GT")
		{
			cg->WriteCode(0, " > ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "GTE")
		{
			cg->WriteCode(0, " >= ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "LTE")
		{
			cg->WriteCode(0, " <= ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "EQUAL")
		{
			cg->WriteCode(0, " == ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "COMMA")
		{
			cg->WriteCode(0, ", ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "AND")
		{
			cg->WriteCode(0, " && ");
			operatorSymbol = "NONE";
		}
		else if (operatorSymbol == "OR")
		{
			cg->WriteCode(0, " || ");
			operatorSymbol = "NONE";
		}
		errors += stmt();
		if (token == RPAREN_T)
		{
			cg->WriteCode(1, "}\n");
			depth = 0;
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		// Write the else part of COND as long as there's more checks to add.
		if (token != RPAREN_T)
			cg->WriteCode(1, "else ");
		errors += stmt_pair();
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::assign_pair()
{
	int errors = 0;

	string message;
	// cg->WriteCode(0, "lexeme() = ");
	if (token == LPAREN_T)
	{ // Rule 24
		p2file << "Using Rule 24\n";
		token = lex->Get_Token();
		if (token == IDENT_T)
		{
			cg->WriteCode(1, "Object " + lex->Get_Lexeme() + " = (");
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[IDENT_T] + " expected";
			lex->Report_Error(message);
		}
		errors += stmt();
		if (token == RPAREN_T)
		{
			// if (depth == 1)
			cg->WriteCode(0, ");//End of object declaration\n");
			depth -= 1;
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::more_assigns()
{
	int errors = 0;

	string message;
	if (token == LPAREN_T)
	{ // Rule 25
		p2file << "Using Rule 25\n";
		errors += assign_pair();
		errors += more_assigns();
	}
	else if (token == RPAREN_T)
	{ // Rule 26
		p2file << "Using Rule 26\n";
		if (depth == 1)
			cg->WriteCode(0, ")");
		depth -= 1;
		;
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::action()
{
	int errors = 0;
	// depth += 1;
	string message;
	if (token == IF_T)
	{ // Rule 27
		p2file << "Using Rule 27\n";
		// Beginning of an if, we write if( then grab the statement
		cg->WriteCode(1, "if ");
		cg->WriteCode(0, "(");
		token = lex->Get_Token();
		errors += stmt();
		// After we come out of the statement we finish the if off with a '('
		// We then have a newline, {, and another newline
		cg->WriteCode(0, ")\n");
		cg->WriteCode(1, "{\n");
		errors += stmt();
		cg->WriteCode(1, "}\n");
		// Finish off the if(){}, add an else part if we have one.
		errors += else_part();
	}
	else if (token == COND_T)
	{ // Rule 28
		p2file << "Using Rule 28\n";
		token = lex->Get_Token();
		if (token == LPAREN_T)
		{
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[LPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		errors += stmt_pair_body();
	}
	else if (token == LET_T)
	{ // Rule 29
		p2file << "Using Rule 29\n";
		token = lex->Get_Token();
		if (token == LPAREN_T)
		{
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[LPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		errors += more_assigns();
		if (token == RPAREN_T)
		{
			if (depth == 1)
				cg->WriteCode(0, ")");
			depth -= 1;
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
		errors += stmt();
		errors += stmt_list();
	}
	// Most of the following rules follow a similar patten of wrapping the segment in parenthesis
	else if (token == LISTOP1_T)
	{ // Rule 30
		p2file << "Using Rule 30\n";
		cg->WriteCode(0, "listop ( \"" + lex->Get_Lexeme() + "\", ");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == LISTOP2_T)
	{ // Rule 31
		p2file << "Using Rule 31\n";
		cg->WriteCode(0, "listop ( \"" + lex->Get_Lexeme() + "\", ");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, ", "); // Add a seperator between arguments when LISTOP has more than 1 argument
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == AND_T)
	{ // Rule 32
		p2file << "Using Rule 32\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "AND";
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == OR_T)
	{ // Rule 33
		p2file << "Using Rule 33\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "OR";
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == NOT_T)
	{ // Rule 34
		p2file << "Using Rule 34\n";
		cg->WriteCode(0, " not (");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == NUMBERP_T)
	{ // Rule 35
		p2file << "Using Rule 35\n";
		token = lex->Get_Token();
		errors += stmt();
	}
	else if (token == LISTP_T)
	{ // Rule 36
		p2file << "Using Rule 36\n";
		cg->WriteCode(0, " listp (");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == ZEROP_T)
	{ // Rule 37
		p2file << "Using Rule 37\n";
		cg->WriteCode(0, " zerop (");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == NULLP_T)
	{ // Rule 38
		p2file << "Using Rule 38\n";
		cg->WriteCode(0, " nullp (");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == STRINGP_T)
	{ // Rule 39
		p2file << "Using Rule 39\n";
		cg->WriteCode(0, " stringp (");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == PLUS_T)
	{ // Rule 40
		p2file << "Using Rule 40\n";
		cg->WriteCode(0, "(");
		// Many of these rules set the operatorSymbol to their respective operators, in this case "PLUS" will be read in stmt_list() and insert a +
		operatorSymbol = "PLUS";
		operatorStack.push_back("PLUS"); // We will also add this to our math stack
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == MINUS_T)
	{ // Rule 41
		p2file << "Using Rule 41\n";
		operatorStack.push_back("MINUS");
		cg->WriteCode(0, "(");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, " - "); // Instead of changing operatorSymbol to something like "MINUS", we insert it directly between the operands
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == DIV_T)
	{ // Rule 42
		p2file << "Using Rule 42\n";
		operatorStack.push_back("DIV");
		cg->WriteCode(0, "(");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, " / "); // Same concept as MINUS_T
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == MULT_T)
	{ // Rule 43
		p2file << "Using Rule 43\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "MULT";
		operatorStack.push_back("MULT");
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == MODULO_T)
	{ // Rule 44
		p2file << "Using Rule 44\n";
		operatorStack.push_back("MODULO");
		cg->WriteCode(0, "(");
		token = lex->Get_Token();
		errors += stmt();
		cg->WriteCode(0, " % ");
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == ROUND_T)
	{ // Rule 45
		p2file << "Using Rule 45\n";
		token = lex->Get_Token();
		cg->WriteCode(0, " round (");
		errors += stmt();
		cg->WriteCode(0, ")");
	}
	else if (token == EQUALTO_T)
	{ // Rule 46
		p2file << "Using Rule 46\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "EQUAL";
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == GT_T)
	{ // Rule 47
		p2file << "Using Rule 47\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "GT";
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == LT_T)
	{ // Rule 48
		p2file << "Using Rule 48\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "LT";
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == GTE_T)
	{ // Rule 49
		p2file << "Using Rule 49\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "GTE";
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == LTE_T)
	{ // Rule 50
		p2file << "Using Rule 50\n";
		cg->WriteCode(0, "(");
		operatorSymbol = "LTE";
		token = lex->Get_Token();
		errors += stmt_list();
		cg->WriteCode(0, ")");
	}
	else if (token == IDENT_T)
	{ // Rule 51
		p2file << "Using Rule 51\n";
		// If we are at the beginning of a statement assign __RetVal to the lexeme
		// However if we have read a DISPLAY, we do not want to add the __RetVal = in the middle of a `cout <<`
		// In that case we do not assign __RetVal to anything.
		if (depth = 1)
		{
			if (calledDislay != true)
				cg->WriteCode(1, "__RetVal = " + lex->Get_Lexeme() + "(");
			else
				cg->WriteCode(0, lex->Get_Lexeme() + "(");
		}
		operatorStack.push_back("COMMA");
		operatorSymbol = "COMMA";
		token = lex->Get_Token();
		errors += stmt_list();
		depth = 0;
		// If we are not in a cout statement, finish the assignment and end the line of code. If we are in a cout, close the parenthesis and let rule 52 finish it
		if (depth == 0)
		{
			if (calledDislay != true)
				cg->WriteCode(0, ");\n");
			else
				cg->WriteCode(0, ")");
		}
	}
	else if (token == DISPLAY_T)
	{ // Rule 52
		p2file << "Using Rule 52\n";
		// This rule is used for writing c++ cout code.
		calledDislay = true; // Set this to true so we do not write a __RetVal to our code.
		cg->WriteCode(1, "cout << ");
		token = lex->Get_Token();
		errors += stmt();
		depth = 0;
		// redundant check: We will print the semicolon and newline after the content that comes after 'cout <<'
		if (depth == 0)
			cg->WriteCode(0, ";\n");
		calledDislay = false;
	}
	else if (token == NEWLINE_T)
	{ // Rule 53
		p2file << "Using Rule 53\n";
		depth = 0;
		cg->WriteCode(1, "cout << endl;\n");
		token = lex->Get_Token();
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}

int Syntactical_Analyzer::any_other_token()
{
	int errors = 0;

	string message;
	if (token == LPAREN_T)
	{ // Rule 54
		p2file << "Using Rule 54\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
		errors += more_tokens();
		if (token == RPAREN_T)
		{
			cg->WriteCode(0, lex->Get_Lexeme());
			token = lex->Get_Token();
		}
		else
		{
			errors++;
			message = token_lexemes[RPAREN_T] + " expected";
			lex->Report_Error(message);
		}
	}
	else if (token == IDENT_T)
	{ // Rule 55
		p2file << "Using Rule 55\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == NUMLIT_T)
	{ // Rule 56
		p2file << "Using Rule 56\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == STRLIT_T)
	{ // Rule 57
		p2file << "Using Rule 57\n";
		string stringLit = lex->Get_Lexeme();
		cg->WriteCode(0, stringLit.substr(1, stringLit.length() - 2)); // grabbing quotations when it shouldn't be?
		token = lex->Get_Token();
	}
	else if (token == IF_T)
	{ // Rule 58
		p2file << "Using Rule 58\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == DISPLAY_T)
	{ // Rule 59
		p2file << "Using Rule 59\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == NEWLINE_T)
	{ // Rule 60
		p2file << "Using Rule 60\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == LISTOP1_T)
	{ // Rule 61
		p2file << "Using Rule 61\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == LISTOP2_T)
	{ // Rule 62
		p2file << "Using Rule 62\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == AND_T)
	{ // Rule 63
		p2file << "Using Rule 63\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == OR_T)
	{ // Rule 64
		p2file << "Using Rule 64\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == NOT_T)
	{ // Rule 65
		p2file << "Using Rule 65\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == DEFINE_T)
	{ // Rule 66
		p2file << "Using Rule 66\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == LET_T)
	{ // Rule 67
		p2file << "Using Rule 67\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == NUMBERP_T)
	{ // Rule 68
		p2file << "Using Rule 68\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == LISTP_T)
	{ // Rule 69
		p2file << "Using Rule 69\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == ZEROP_T)
	{ // Rule 70
		p2file << "Using Rule 70\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == NULLP_T)
	{ // Rule 71
		p2file << "Using Rule 71\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == STRINGP_T)
	{ // Rule 72
		p2file << "Using Rule 72\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == PLUS_T)
	{ // Rule 73
		p2file << "Using Rule 73\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == MINUS_T)
	{ // Rule 74
		p2file << "Using Rule 74\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == DIV_T)
	{ // Rule 75
		p2file << "Using Rule 75\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == MULT_T)
	{ // Rule 76
		p2file << "Using Rule 76\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == MODULO_T)
	{ // Rule 77
		p2file << "Using Rule 77\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == ROUND_T)
	{ // Rule 78
		p2file << "Using Rule 78\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == EQUALTO_T)
	{ // Rule 79
		p2file << "Using Rule 79\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == GT_T)
	{ // Rule 80
		p2file << "Using Rule 80\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == LT_T)
	{ // Rule 81
		p2file << "Using Rule 81\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == GTE_T)
	{ // Rule 82
		p2file << "Using Rule 82\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == LTE_T)
	{ // Rule 83
		p2file << "Using Rule 83\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == SQUOTE_T)
	{ // Rule 84
		p2file << "Using Rule 84\n";
		token = lex->Get_Token();
		errors += any_other_token();
	}
	else if (token == COND_T)
	{ // Rule 85
		p2file << "Using Rule 85\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else if (token == ELSE_T)
	{ // Rule 86
		p2file << "Using Rule 86\n";
		cg->WriteCode(0, lex->Get_Lexeme());
		token = lex->Get_Token();
	}
	else
	{
		message = token_lexemes[token] + " unexpected";
		lex->Report_Error(message);
	}
	return errors;
}
