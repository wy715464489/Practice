//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Finite state machine
// 
//*****************************************************************************

#include <ctype.h>
#include "VuFSM.h"
#include "VuHash.h"


#define MAX_EXPRESSION_LENGTH 256


enum eTokenType { TOK_CONDITION, TOK_AND, TOK_OR, TOK_OPEN_PAREN, TOK_CLOSE_PAREN, TOK_NOT, TOK_END };
struct VuFSM::Token
{
	Token(eTokenType type) : mType(type), mIndex(0) {}
	eTokenType	mType;
	int			mIndex;
};


//*****************************************************************************
VuFSM::VuFSM():
	mpCurState(VUNULL),
	mpPrevState(VUNULL),
	mpNextState(VUNULL),
	mTimeInState(0.0f),
	mDebugTag(VUNULL)
{
}

//*****************************************************************************
VuFSM::~VuFSM()
{
	for ( States::iterator iter = mStates.begin(); iter != mStates.end(); iter++ )
		delete *iter;

	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
		delete *iter;
}

//*****************************************************************************
VuFSM::VuState *VuFSM::addState(const char *strName)
{
	VuState *pState = new VuState(strName);
	mStates.push_back(pState);

	if ( !mpCurState )
		mpCurState = pState;

	return pState;
}

//*****************************************************************************
void VuFSM::addTransition(const char *strFromState, const char *strToState, const char *strExpression)
{
	int fromStateIndex = getStateIndex(strFromState);
	if ( fromStateIndex == -1 && strFromState[0] )
		return;

	int toStateIndex = getStateIndex(strToState);
	if ( toStateIndex == -1 )
		return;

	// error check
	VUASSERT(strlen(strExpression) < MAX_EXPRESSION_LENGTH, "Max expression length exceeded");

	// lex (tokenize)
	std::vector<Token> expression;
	if ( tokenizeExpression(strExpression, expression) )
	{
		// build expression from tokens
		Token *pToken = &expression[0];
		VuExpression *pExpression = createExpression(pToken);
		VUASSERT(pToken->mType == TOK_END, "Expression parsing error, missing parenthesis?");

		if ( fromStateIndex == -1 )
		{
			for ( int i = 0; i < (int)mStates.size(); i++ )
				mStates[i]->mTransitions.push_back(VuState::Transition(pExpression, toStateIndex));
		}
		else
		{
			mStates[fromStateIndex]->mTransitions.push_back(VuState::Transition(pExpression, toStateIndex));
		}
	}
}

//*****************************************************************************
void VuFSM::begin()
{
	mpCurState = mStates[0];

	mTimeInState = 0.0f;

	if ( VuState::TransMethod *pMethod = mpCurState->mpEnterMethod )
		pMethod->execute();
}

//*****************************************************************************
void VuFSM::end()
{
	if ( VuState::TransMethod *pMethod = mpCurState->mpExitMethod )
		pMethod->execute();
}

//*****************************************************************************
void VuFSM::setCondition(const char *strCond, bool set)
{
	int condIndex = getConditionIndex(strCond);
	if ( condIndex >= 0 )
		mConditions[condIndex].mSet = set;
}

//*****************************************************************************
void VuFSM::pulseCondition(const char *strCond)
{
	int condIndex = getConditionIndex(strCond);
	if ( condIndex >= 0 )
		mConditions[condIndex].mPulse = true;
}

//*****************************************************************************
void VuFSM::clearAllConditions()
{
	for ( Conditions::iterator iter = mConditions.begin(); iter != mConditions.end(); iter++ )
	{
		iter->mSet = false;
		iter->mPulse = false;
	}
}

//*****************************************************************************
void VuFSM::evaluate()
{
	int newState = testExpressions();

	// handle decision states
	int depth = 0;
	while ( newState >= 0 && mStates[newState]->mDecisionState && depth < 10 )
	{
		handleTransition(newState);
		newState = testExpressions();
		VUASSERT(newState >= 0, "VuFSM decision state dead end");
	}
	VUASSERT(depth < 10, "VuFSM decision state recursion");

	// clear condition pulses
	for ( int i = 0; i < (int)mConditions.size(); i++ )
		mConditions[i].mPulse = false;

	// handle state transition
	if ( newState >= 0 )
		handleTransition(newState);
}

//*****************************************************************************
void VuFSM::tick(float fdt)
{
	if ( VuState::TickMethod *pMethod = mpCurState->mpTickMethod )
		pMethod->execute(fdt);

	mTimeInState += fdt;
}

//*****************************************************************************
void VuFSM::draw()
{
	if ( VuState::DrawMethod *pMethod = mpCurState->mpDrawMethod )
		pMethod->execute();
}

//*****************************************************************************
int VuFSM::testExpressions()
{
	// test conditions
	for ( VuState::Transitions::const_iterator iter = mpCurState->mTransitions.begin(); iter != mpCurState->mTransitions.end(); iter++ )
		if ( iter->mpExpression->evaluate(*this) )
			return iter->mToState;

	return -1;
}

//*****************************************************************************
void VuFSM::handleTransition(int newState)
{
	mpPrevState = mpCurState;
	mpNextState = mStates[newState];

	if ( VuState::TransMethod *pMethod = mpCurState->mpExitMethod )
		pMethod->execute();

	mpCurState = mpNextState;

	if ( VuState::TransMethod *pMethod = mpCurState->mpEnterMethod )
		pMethod->execute();

	mTimeInState = 0.0f;

#ifndef VURETAIL
	if ( mDebugTag )
		VUPRINTF("%s transition - %s -> %s\n", mDebugTag, mpPrevState->getName().c_str(), mpNextState->getName().c_str());
#endif
}

//*****************************************************************************
int VuFSM::getStateIndex(const char *strState)
{
	VUUINT32 hashedName = VuHash::fnv32String(strState);

	for ( int i = 0; i < (int)mStates.size(); i++ )
		if ( mStates[i]->mHashedName == hashedName )
			return i;

	return -1;
}

//*****************************************************************************
int VuFSM::getConditionIndex(const char *strCondition)
{
	VUUINT32 hashedName = VuHash::fnv32String(strCondition);

	for ( int i = 0; i < (int)mConditions.size(); i++ )
		if ( mConditions[i].mHashedName == hashedName )
			return i;

	return -1;
}

//*****************************************************************************
bool VuFSM::tokenizeExpression(const char *strExpression, std::vector<Token> &expression)
{
	while ( strExpression[0] )
	{
		if ( strExpression[0] == ' ' )
		{
			strExpression += 1;
		}
		else if ( strExpression[0] == '&' )
		{
			expression.push_back(Token(TOK_AND));
			strExpression += 1;
		}
		else if ( strExpression[0] == '|' )
		{
			expression.push_back(Token(TOK_OR));
			strExpression += 1;
		}
		else if ( strExpression[0] == '(' )
		{
			expression.push_back(Token(TOK_OPEN_PAREN));
			strExpression += 1;
		}
		else if ( strExpression[0] == ')' )
		{
			expression.push_back(Token(TOK_CLOSE_PAREN));
			strExpression += 1;
		}
		else if ( strExpression[0] == '!' )
		{
			expression.push_back(Token(TOK_NOT));
			strExpression += 1;
		}
		else if ( isalnum(strExpression[0]) )
		{
			// extract condition
			char strCondition[MAX_EXPRESSION_LENGTH];
			char *dst = strCondition;
			do {
				*dst++ = *strExpression++;
			} while ( isalnum(strExpression[0]) );
			*dst = '\0';

			int conditionIndex = getConditionIndex(strCondition);
			if ( conditionIndex == -1 )
			{
				conditionIndex = (int)mConditions.size();
				mConditions.push_back(VuCondition(strCondition));
			}

			Token token(TOK_CONDITION);
			token.mIndex = conditionIndex;
			expression.push_back(token);
		}
		else
		{
			VUASSERT(0, "Expression parsing error");
			return false;
		}
	}

	expression.push_back(Token(TOK_END));

	return true;
}

//*****************************************************************************
VuFSM::VuExpression *VuFSM::createExpression(Token *&pCurTok)
{
	VuExpression *pExpression = VUNULL;

	// left side
	if ( pCurTok->mType == TOK_CONDITION )
	{
		// try to find expression
		pExpression = findConditionExpression(pCurTok->mIndex);
		if ( pExpression == VUNULL )
		{
			pExpression = new VuConditionExpression(pCurTok->mIndex);
			mExpressions.push_back(pExpression);
		}
		pCurTok += 1;
	}
	else if ( pCurTok->mType == TOK_OPEN_PAREN )
	{
		pCurTok += 1;
		pExpression = createExpression(pCurTok);
		VUASSERT(pCurTok->mType == TOK_CLOSE_PAREN, "Expression missing closing parenthesis.");
		pCurTok += 1;
	}
	else if ( pCurTok->mType == TOK_NOT )
	{
		pCurTok += 1;
		VuExpression *pChild = createExpression(pCurTok);

		// try to find expression
		pExpression = findNotExpression(pChild);
		if ( pExpression == VUNULL )
		{
			pExpression = new VuNotExpression(pChild);
			mExpressions.push_back(pExpression);
		}
	}
	else if ( pCurTok->mType == TOK_END )
	{
		// try to find expression
		pExpression = findTrueExpression();
		if ( pExpression == VUNULL )
		{
			pExpression = new VuTrueExpression();
			mExpressions.push_back(pExpression);
		}
	}

	VUASSERT(pExpression, "Invalid expression");

	// right side (optional)
	if ( pCurTok->mType == TOK_AND )
	{
		pCurTok += 1;
		VuExpression *pChildA = pExpression;
		VuExpression *pChildB = createExpression(pCurTok);

		// try to find expression
		pExpression = findAndExpression(pChildA, pChildB);
		if ( pExpression == VUNULL )
		{
			pExpression = new VuAndExpression(pChildA, pChildB);
			mExpressions.push_back(pExpression);
		}
	}
	else if ( pCurTok->mType == TOK_OR )
	{
		pCurTok += 1;
		VuExpression *pChildA = pExpression;
		VuExpression *pChildB = createExpression(pCurTok);

		// try to find expression
		pExpression = findOrExpression(pChildA, pChildB);
		if ( pExpression == VUNULL )
		{
			pExpression = new VuOrExpression(pChildA, pChildB);
			mExpressions.push_back(pExpression);
		}
	}

	return pExpression;
}

//*****************************************************************************
VuFSM::VuExpression *VuFSM::findTrueExpression()
{
	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
	{
		if ( (*iter)->mType == VuExpression::TYPE_TRUE )
		{
			VuTrueExpression *p = static_cast<VuTrueExpression *>(*iter);
			return p;
		}
	}

	return VUNULL;
}

//*****************************************************************************
VuFSM::VuExpression *VuFSM::findNotExpression(VuExpression *pChild)
{
	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
	{
		if ( (*iter)->mType == VuExpression::TYPE_NOT )
		{
			VuNotExpression *p = static_cast<VuNotExpression *>(*iter);
			if ( p->mpChild == pChild )
				return p;
		}
	}

	return VUNULL;
}

//*****************************************************************************
VuFSM::VuExpression *VuFSM::findConditionExpression(int conditionIndex)
{
	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
	{
		if ( (*iter)->mType == VuExpression::TYPE_CONDITION )
		{
			VuConditionExpression *p = static_cast<VuConditionExpression *>(*iter);
			if ( p->mCondition == conditionIndex )
				return p;
		}
	}

	return VUNULL;
}

//*****************************************************************************
VuFSM::VuExpression *VuFSM::findAndExpression(VuExpression *pChildA, VuExpression *pChildB)
{
	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
	{
		if ( (*iter)->mType == VuExpression::TYPE_AND )
		{
			VuAndExpression *p = static_cast<VuAndExpression *>(*iter);
			if ( (p->mpChildA == pChildA && p->mpChildB == pChildB) || (p->mpChildA == pChildB && p->mpChildB == pChildA) )
				return p;
		}
	}

	return VUNULL;
}

//*****************************************************************************
VuFSM::VuExpression *VuFSM::findOrExpression(VuExpression *pChildA, VuExpression *pChildB)
{
	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
	{
		if ( (*iter)->mType == VuExpression::TYPE_OR )
		{
			VuOrExpression *p = static_cast<VuOrExpression *>(*iter);
			if ( (p->mpChildA == pChildA && p->mpChildB == pChildB) || (p->mpChildA == pChildB && p->mpChildB == pChildA) )
				return p;
		}
	}

	return VUNULL;
}

//*****************************************************************************
VuFSM::VuState::VuState(const char *strName):
	mName(strName),
	mDecisionState(false),
	mpEnterMethod(VUNULL),
	mpExitMethod(VUNULL),
	mpTickMethod(VUNULL),
	mpDrawMethod(VUNULL)
{
	mHashedName = VuHash::fnv32String(strName);
}

//*****************************************************************************
VuFSM::VuState::~VuState()
{
	delete mpEnterMethod;
	delete mpExitMethod;
	delete mpTickMethod;
	delete mpDrawMethod;
}

//*****************************************************************************
VuFSM::VuCondition::VuCondition(const char *strName):
	mSet(false),
	mPulse(false)
{
	mHashedName = VuHash::fnv32String(strName);
}
