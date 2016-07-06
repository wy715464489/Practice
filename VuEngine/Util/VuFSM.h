//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Finite state machine
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Method/VuMethod.h"


class VuFSM
{
public:
	class VuState;
	class VuCondition;
	class VuExpression;
	class VuTrueExpression;
	class VuNotExpression;
	class VuConditionExpression;
	class VuAndExpression;
	class VuOrExpression;
	struct Token;

	VuFSM();
	virtual ~VuFSM();

	VuState			*addState(const char *strName);
	void			addTransition(const char *strFromState, const char *strToState, const char *strExpression);

	void			begin();
	void			end();

	void			setCondition(const char *strCond, bool set);
	void			pulseCondition(const char *strCond);
	void			clearAllConditions();

	void			evaluate();
	void			tick(float fdt);
	void			draw();

	float			getTimeInState()	{ return mTimeInState; }
	const VuState	*getCurState()		{ return mpCurState; }
	const VuState	*getPrevState()		{ return mpPrevState; }
	const VuState	*getNextState()		{ return mpNextState; }

	void			setDebugTag(const char *debugTag) { mDebugTag = debugTag; }

private:
	typedef std::vector<VuState *> States;
	typedef std::vector<VuCondition> Conditions;
	typedef std::vector<VuExpression *> Expressions;

	int			testExpressions();
	void		handleTransition(int newState);

	int			getStateIndex(const char *strState);
	int			getConditionIndex(const char *strCondition);

	bool			tokenizeExpression(const char *strExpression, std::vector<Token> &expression);
	VuExpression	*createExpression(Token *&pCurTok);

	VuExpression	*findTrueExpression();
	VuExpression	*findNotExpression(VuExpression *pChild);
	VuExpression	*findConditionExpression(int conditionIndex);
	VuExpression	*findAndExpression(VuExpression *pChildA, VuExpression *pChildB);
	VuExpression	*findOrExpression(VuExpression *pChildA, VuExpression *pChildB);

	VuState		*mpCurState;
	VuState		*mpPrevState;
	VuState		*mpNextState;
	States		mStates;
	Conditions	mConditions;
	Expressions	mExpressions;

	float		mTimeInState;
	const char	*mDebugTag;
};

class VuFSM::VuState
{
public:
	VuState(const char *strName);
	~VuState();

	template<class T> void	setEnterMethod(T *pObj, void (T::*method)());
	template<class T> void	setExitMethod(T *pObj, void (T::*method)());
	template<class T> void	setTickMethod(T *pObj, void (T::*method)(float fdt));
	template<class T> void	setDrawMethod(T *pObj, void (T::*method)());

	void				makeDecisionState() { mDecisionState = true; }

	const std::string	&getName() const { return mName; }

protected:
	friend class VuFSM;

	struct Transition
	{
		Transition(VuExpression *pExpression, int toState) : mpExpression(pExpression), mToState(toState) {}
		VuExpression	*mpExpression;
		int				mToState;
	};
	typedef VuMethodInterface1<void, float> TickMethod;
	typedef VuMethodInterface0<void> TransMethod;
	typedef VuMethodInterface0<void> DrawMethod;
	typedef std::vector<Transition> Transitions;

	std::string	mName;
	bool		mDecisionState;
	VUUINT32	mHashedName;
	TransMethod	*mpEnterMethod;
	TransMethod	*mpExitMethod;
	TickMethod	*mpTickMethod;
	DrawMethod	*mpDrawMethod;
	Transitions	mTransitions;
};

class VuFSM::VuCondition
{
public:
	VuCondition(const char *strName);
	VUUINT32	mHashedName;
	bool		mSet;
	bool		mPulse;
};

class VuFSM::VuExpression
{
public:
	enum eType { TYPE_TRUE, TYPE_NOT, TYPE_CONDITION, TYPE_AND, TYPE_OR };
	VuExpression(eType type) : mType(type) {}
	virtual ~VuExpression() {}	// some compilers warn about deleting base class objects with virtuals and a non-virtual destructor
	virtual bool evaluate(const VuFSM &fsm) const = 0;
	eType	mType;
};

class VuFSM::VuTrueExpression : public VuExpression
{
public:
	VuTrueExpression() : VuExpression(TYPE_TRUE) {}
	virtual bool evaluate(const VuFSM &fsm) const { return true; }
};

class VuFSM::VuNotExpression : public VuExpression
{
public:
	VuNotExpression(VuExpression *pChild) : VuExpression(TYPE_NOT), mpChild(pChild) {}
	virtual bool evaluate(const VuFSM &fsm) const { return !mpChild->evaluate(fsm); }
	VuExpression	*mpChild;
};

class VuFSM::VuConditionExpression : public VuExpression
{
public:
	VuConditionExpression(int condition) : VuExpression(TYPE_CONDITION), mCondition(condition) {}
	virtual bool evaluate(const VuFSM &fsm) const
	{
		const VuCondition &condition = fsm.mConditions[mCondition];
		return condition.mSet | condition.mPulse;
	}
	int		mCondition;
};

class VuFSM::VuAndExpression : public VuExpression
{
public:
	VuAndExpression(VuExpression *pChildA, VuExpression *pChildB) : VuExpression(TYPE_AND), mpChildA(pChildA), mpChildB(pChildB) {}
	virtual bool evaluate(const VuFSM &fsm) const { return mpChildA->evaluate(fsm) && mpChildB->evaluate(fsm); }
	VuExpression	*mpChildA;
	VuExpression	*mpChildB;
};

class VuFSM::VuOrExpression : public VuExpression
{
public:
	VuOrExpression(VuExpression *pChildA, VuExpression *pChildB) : VuExpression(TYPE_OR), mpChildA(pChildA), mpChildB(pChildB) {}
	virtual bool evaluate(const VuFSM &fsm) const { return mpChildA->evaluate(fsm) || mpChildB->evaluate(fsm); }
	VuExpression	*mpChildA;
	VuExpression	*mpChildB;
};


#include "VuFSM.inl"