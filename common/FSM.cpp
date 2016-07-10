#include <ctype.h>
#include "FSM.h"
#include "Hash.h"

#define MAX_EXPRESSION_LENGTH 256

enum eTokenType { TOK_CONDITION, TOK_AND, TOK_OR, TOK_OPEN_PAREN, TOK_CLOSE_PAREN, TOK_NOT, TOK_END };
struct FSM::Token
{
	Token(eTokenType type) : mType(type), mIndex(0) {}
	eTokenType	mType;
	int			mIndex;
};

//*****************************************************************************
FSM::FSM():
	mpCurState(NULL),
	mpPrevState(NULL),
	mpNextState(NULL),
	mTimeInState(0.0f),
	mDebugTag(NULL)
{
}

//*****************************************************************************
FSM::~FSM()
{
	for ( States::iterator iter = mStates.begin(); iter != mStates.end(); iter++ )
		delete *iter;

	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
		delete *iter;
}

//*****************************************************************************
FSM::State* FSM::addState(const char *strName)
{
	State* pState = new State(strName);
	mStates.push_back(pState);

	if ( !mpCurState )
		mpCurState = pState;

	return pState;
}

//*****************************************************************************
void FSM::addTransition(const char *strFromState, const char *strToState, const char *strExpression)
{
	int fromStateIndex = getStateIndex(strFromState);
	if ( fromStateIndex == -1 && strFromState[0] )
		return;

	int toStateIndex = getStateIndex(strToState);
	if ( toStateIndex == -1 )
		return;

	// lex (tokenize)
	std::vector<Token> expression;
	if ( tokenizeExpression(strExpression, expression) )
	{
		// build expression from tokens
		Token *pToken = &expression[0];
		Expression *pExpression = createExpression(pToken);

		if ( fromStateIndex == -1 )
		{
			for ( int i = 0; i < (int)mStates.size(); i++ )
				mStates[i]->mTransitions.push_back(State::Transition(pExpression, toStateIndex));
		}
		else
		{
			mStates[fromStateIndex]->mTransitions.push_back(State::Transition(pExpression, toStateIndex));
		}
	}
}

//*****************************************************************************
void FSM::begin()
{
	mpCurState = mStates[0];

	mTimeInState = 0.0f;

	if ( State::TransMethod *pMethod = mpCurState->mpEnterMethod )
		pMethod->execute();
}

//*****************************************************************************
void FSM::end()
{
	if ( State::TransMethod *pMethod = mpCurState->mpExitMethod )
		pMethod->execute();
}

//*****************************************************************************
void FSM::setCondition(const char *strCond, bool set)
{
	int condIndex = getConditionIndex(strCond);
	if ( condIndex >= 0 )
		mConditions[condIndex].mSet = set;
}

//*****************************************************************************
void FSM::pulseCondition(const char *strCond)
{
	int condIndex = getConditionIndex(strCond);
	if ( condIndex >= 0 )
		mConditions[condIndex].mPulse = true;
}

//*****************************************************************************
void FSM::clearAllConditions()
{
	for ( Conditions::iterator iter = mConditions.begin(); iter != mConditions.end(); iter++ )
	{
		iter->mSet = false;
		iter->mPulse = false;
	}
}

//*****************************************************************************
void FSM::evaluate()
{
	int newState = testExpressions();

	// handle decision states
	int depth = 0;
	while ( newState >= 0 && mStates[newState]->mDecisionState && depth < 10 )
	{
		handleTransition(newState);
		newState = testExpressions();
	}

	// clear condition pulses
	for ( int i = 0; i < (int)mConditions.size(); i++ )
		mConditions[i].mPulse = false;

	// handle state transition
	if ( newState >= 0 )
		handleTransition(newState);
}

//*****************************************************************************
void FSM::tick(float fdt)
{
	if ( State::TickMethod *pMethod = mpCurState->mpTickMethod )
		pMethod->execute(fdt);

	mTimeInState += fdt;
}

//*****************************************************************************
int FSM::testExpressions()
{
	// test conditions
	for ( State::Transitions::const_iterator iter = mpCurState->mTransitions.begin(); iter != mpCurState->mTransitions.end(); iter++ )
		if ( iter->mpExpression->evaluate(*this) )
			return iter->mToState;

	return -1;
}

//*****************************************************************************
void FSM::handleTransition(int newState)
{
	mpPrevState = mpCurState;
	mpNextState = mStates[newState];

	if ( State::TransMethod *pMethod = mpCurState->mpExitMethod )
		pMethod->execute();

	mpCurState = mpNextState;

	if ( State::TransMethod *pMethod = mpCurState->mpEnterMethod )
		pMethod->execute();

	mTimeInState = 0.0f;
}

//*****************************************************************************
int FSM::getStateIndex(const char *strState)
{
	int hashedName = Hash::fnv32String(strState);

	for ( int i = 0; i < (int)mStates.size(); i++ )
		if ( mStates[i]->mHashedName == hashedName )
			return i;

	return -1;
}

//*****************************************************************************
int FSM::getConditionIndex(const char *strCondition)
{
	UINT32 hashedName = Hash::fnv32String(strCondition);

	for ( int i = 0; i < (int)mConditions.size(); i++ )
		if ( mConditions[i].mHashedName == hashedName )
			return i;

	return -1;
}

//*****************************************************************************
bool FSM::tokenizeExpression(const char *strExpression, std::vector<Token> &expression)
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
				mConditions.push_back(Condition(strCondition));
			}

			Token token(TOK_CONDITION);
			token.mIndex = conditionIndex;
			expression.push_back(token);
		}
		else
		{
			return false;
		}
	}

	expression.push_back(Token(TOK_END));

	return true;
}

//*****************************************************************************
FSM::Expression* FSM::createExpression(Token* &pCurTok)
{
	Expression *pExpression = NULL;

	// left side
	if ( pCurTok->mType == TOK_CONDITION )
	{
		// try to find expression
		pExpression = findConditionExpression(pCurTok->mIndex);
		if ( pExpression == NULL )
		{
			pExpression = new ConditionExpression(pCurTok->mIndex);
			mExpressions.push_back(pExpression);
		}
		pCurTok += 1;
	}
	else if ( pCurTok->mType == TOK_OPEN_PAREN )
	{
		
	}
	else if ( pCurTok->mType == TOK_NOT )
	{
		
	}
	else if ( pCurTok->mType == TOK_END )
	{
		
	}

	// right side (optional)
	if ( pCurTok->mType == TOK_AND )
	{
		
	}
	else if ( pCurTok->mType == TOK_OR )
	{
		
	}

	return pExpression;
}

//*****************************************************************************
FSM::Expression *FSM::findConditionExpression(int conditionIndex)
{
	for ( Expressions::iterator iter = mExpressions.begin(); iter != mExpressions.end(); iter++ )
	{
		if ( (*iter)->mType == Expression::TYPE_CONDITION )
		{
			ConditionExpression *p = static_cast<ConditionExpression *>(*iter);
			if ( p->mCondition == conditionIndex )
				return p;
		}
	}

	return NULL;
}


//*****************************************************************************
FSM::State::State(const char *strName):
	mName(strName),
	mDecisionState(false),
	mpEnterMethod(NULL),
	mpExitMethod(NULL),
	mpTickMethod(NULL)
{
	mHashedName = Hash::fnv32String(strName);
}

//*****************************************************************************
FSM::State::~State()
{
	delete mpEnterMethod;
	delete mpExitMethod;
	delete mpTickMethod;
}

//*****************************************************************************
FSM::Condition::Condition(const char *strName):
	mSet(false),
	mPulse(false)
{
	mHashedName = Hash::fnv32String(strName);
}