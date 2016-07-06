//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Filter expression
// 
//*****************************************************************************

#include <ctype.h>
#include "VuFilterExpression.h"
#include "VuEngine/Math/VuMath.h"


enum eTokenType { TOK_STRING, TOK_EQUALS, TOK_NOT_EQUALS, TOK_AND, TOK_OR, TOK_OPEN_PAREN, TOK_CLOSE_PAREN, TOK_NOT, TOK_COMMA, TOK_STARTS_WITH, TOK_CONTAINS, TOK_END };
struct VuFilterExpression::Token
{
	Token(eTokenType type) : mType(type) {}
	eTokenType	mType;
	std::string	mString;
};


//*****************************************************************************
void VuFilterExpression::addVariable(const char *name, const char *value)
{
	mVars.push_back(Variable(name, (value)));
}

//*****************************************************************************
bool VuFilterExpression::evaluate(const char *filter)
{
	mLastError.clear();

	// lex (tokenize)
	std::vector<Token> expression;

	while ( filter[0] )
	{
		if ( filter[0] == ' ' )
		{
			filter += 1;
		}
		else if ( filter[0] == '=' && filter[1] == '=' )
		{
			expression.push_back(Token(TOK_EQUALS));
			filter += 2;
		}
		else if ( filter[0] == '!' && filter[1] == '=' )
		{
			expression.push_back(Token(TOK_NOT_EQUALS));
			filter += 2;
		}
		else if ( filter[0] == '&' && filter[1] == '&' )
		{
			expression.push_back(Token(TOK_AND));
			filter += 2;
		}
		else if ( filter[0] == '|' && filter[1] == '|' )
		{
			expression.push_back(Token(TOK_OR));
			filter += 2;
		}
		else if ( filter[0] == '(' )
		{
			expression.push_back(Token(TOK_OPEN_PAREN));
			filter += 1;
		}
		else if ( filter[0] == ')' )
		{
			expression.push_back(Token(TOK_CLOSE_PAREN));
			filter += 1;
		}
		else if ( filter[0] == '!' )
		{
			expression.push_back(Token(TOK_NOT));
			filter += 1;
		}
		else if ( filter[0] == ',' )
		{
			expression.push_back(Token(TOK_COMMA));
			filter += 1;
		}
		else if ( strncmp(filter, "StartsWith", 10) == 0 )
		{
			expression.push_back(Token(TOK_STARTS_WITH));
			filter += 10;
		}
		else if ( strncmp(filter, "Contains", 8) == 0 )
		{
			expression.push_back(Token(TOK_CONTAINS));
			filter += 8;
		}
		else if ( isalnum(filter[0]) )
		{
			Token token(TOK_STRING);
			do
			{
				token.mString.push_back(filter[0]);
				filter++;
			}
			while ( isalnum(filter[0]) );

			expression.push_back(token);
		}
		else
		{
			mLastError += "Expression parsing error : ";
			mLastError += filter;
			return false;
		}
	}
	expression.push_back(Token(TOK_END));

	mpCurTok = &expression[0];

	mResult = evaluate();

	if ( mpCurTok[0].mType != TOK_END )
		mLastError += "Expression parsing error, missing parenthesis?";

	return mLastError.empty();
}

//*****************************************************************************
const std::string &VuFilterExpression::getValue(const std::string &name)
{
	for ( int i = 0; i < (int)mVars.size(); i++ )
		if ( mVars[i].first == name )
			return mVars[i].second;

	static std::string sEmtpy;
	return sEmtpy;
}

//*****************************************************************************
bool VuFilterExpression::evaluate()
{
	bool result = false;

	// left side
	if ( mpCurTok[0].mType == TOK_STRING && mpCurTok[1].mType == TOK_EQUALS && mpCurTok[2].mType == TOK_STRING )
	{
		result = getValue(mpCurTok[0].mString) == mpCurTok[2].mString;
		mpCurTok += 3;
	}
	else if ( mpCurTok[0].mType == TOK_STRING && mpCurTok[1].mType == TOK_NOT_EQUALS && mpCurTok[2].mType == TOK_STRING )
	{
		result = getValue(mpCurTok[0].mString) != mpCurTok[2].mString;
		mpCurTok += 3;
	}
	else if ( mpCurTok[0].mType == TOK_OPEN_PAREN )
	{
		mpCurTok += 1;
		result = evaluate();
		if ( mpCurTok[0].mType != TOK_CLOSE_PAREN )
		{
			mLastError += "Expression missing closing parenthesis.\n";
			return false;
		}
		mpCurTok += 1;
	}
	else if ( mpCurTok[0].mType == TOK_NOT )
	{
		mpCurTok += 1;
		result = !evaluate();
	}
	else if ( mpCurTok[0].mType == TOK_STARTS_WITH &&
	          mpCurTok[1].mType == TOK_OPEN_PAREN &&
	          mpCurTok[2].mType == TOK_STRING &&
	          mpCurTok[3].mType == TOK_COMMA &&
	          mpCurTok[4].mType == TOK_STRING &&
	          mpCurTok[5].mType == TOK_CLOSE_PAREN )
	{
		const std::string &str1 = getValue(mpCurTok[2].mString);
		const std::string &str2 = mpCurTok[4].mString;
		result = strncmp(str1.c_str(), str2.c_str(), str2.length()) == 0;
		mpCurTok += 6;
	}
	else if ( mpCurTok[0].mType == TOK_CONTAINS &&
	          mpCurTok[1].mType == TOK_OPEN_PAREN &&
	          mpCurTok[2].mType == TOK_STRING &&
	          mpCurTok[3].mType == TOK_COMMA &&
	          mpCurTok[4].mType == TOK_STRING &&
	          mpCurTok[5].mType == TOK_CLOSE_PAREN )
	{
		const std::string &str1 = getValue(mpCurTok[2].mString);
		const std::string &str2 = mpCurTok[4].mString;
		result = strstr(str1.c_str(), str2.c_str()) != VUNULL;
		mpCurTok += 6;
	}

	// right side (optional)
	if ( mpCurTok[0].mType == TOK_AND )
	{
		mpCurTok += 1;
		result = evaluate() && result;
	}
	else if ( mpCurTok[0].mType == TOK_OR )
	{
		mpCurTok += 1;
		result = evaluate() || result;
	}

	return result;
}
