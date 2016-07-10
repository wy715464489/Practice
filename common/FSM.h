#pragma once

#include <vector>
#include <stdio.h>
#include <string>
#include "Method.h"

class  FSM {
public:
	class State;
	class Condition;
	class Expression;
	class ConditionExpression;
	struct Token;

	FSM();
	virtual ~FSM();
	
	State* 			addState(const char* strName);
	void			addTransition(const char* strFromState, const char* strToState, const char* strExpression);

	void			begin();
	void			end();

	void			setCondition(const char* strCond, bool set);
	void			pulseCondition(const char *strCond);
	void			clearAllConditions();

	void			evaluate();
	void			tick(float fdt);

	const State*	getCurState()		{ return mpCurState; }
	const State*	getPrevState()		{ return mpPrevState; }
	const State*	getNextState()		{ return mpNextState; }

private:
	typedef std::vector<State *> States;
	typedef std::vector<Condition> Conditions;
	typedef std::vector<Expression *> Expressions;

	int			testExpressions();
	void		handleTransition(int newState);

	int			getStateIndex(const char *strState);
	int			getConditionIndex(const char *strCondition);

	bool 		tokenizeExpression(const char *strExpression, std::vector<Token> &expression);
	Expression	*createExpression(Token *&pCurTok);

	Expression	*findConditionExpression(int conditionIndex);

	State* 		mpCurState;
	State* 		mpPrevState;
	State* 		mpNextState;
	States		mStates;
	Conditions	mConditions;
	Expressions	mExpressions;

	float		mTimeInState;
	const char	*mDebugTag;
};

class FSM::State {
public:
	State(const char* strName);
	~State();

	template<class T> void	setEnterMethod(T *pObj, void (T::*method)());
	template<class T> void	setExitMethod(T *pObj, void (T::*method)());
	template<class T> void	setTickMethod(T *pObj, void (T::*method)(float fdt));
	//template<class T> void	setDrawMethod(T *pObj, void (T::*method)());

	const std::string	&getName() const { return mName; }
protected:
	friend class FSM;
	struct Transition
	{
		Transition(Expression* pExpression, int toState):mpExpression(pExpression), mToState(toState)
		{}
		Expression	*mpExpression;
		int				mToState;	
	};

	typedef MethodInterface1<void, float> TickMethod;
	typedef MethodInterface0<void> TransMethod;
	typedef MethodInterface0<void> DrawMethod;
	typedef std::vector<Transition> Transitions;

	std::string mName;
	bool		mDecisionState;
	int			mHashedName;
	TransMethod *mpEnterMethod;
	TransMethod *mpExitMethod;
	TickMethod *mpTickMethod;
	Transitions	mTransitions;
};

class FSM::Condition
{
public:
	Condition(const char *strName);
	int			mHashedName;
	bool		mSet;
	bool		mPulse;
};

class FSM::Expression
{
public:
	enum eType { TYPE_TRUE, TYPE_NOT, TYPE_CONDITION, TYPE_AND, TYPE_OR };
	Expression(eType type) : mType(type) {}
	virtual ~Expression() {}	// some compilers warn about deleting base class objects with virtuals and a non-virtual destructor
	virtual bool evaluate(const FSM &fsm) const = 0;
	eType	mType;
};

class FSM::ConditionExpression : public Expression
{
public:
	ConditionExpression(int condition) : Expression(TYPE_CONDITION), mCondition(condition) {}
	virtual bool evaluate(const FSM &fsm) const
	{
		const Condition &condition = fsm.mConditions[mCondition];
		return condition.mSet | condition.mPulse;
	}
	int		mCondition;
};

#include "FSM.inl"

