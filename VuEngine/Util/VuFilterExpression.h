//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Filter expression
// 
//*****************************************************************************

#pragma once


class VuFilterExpression
{
public:
	VuFilterExpression() : mResult(false) {}

	void		addVariable(const char *name, const char *value);

	bool		evaluate(const char *filter);

	bool		result() { return mResult; }
	const char	*getLastError() { return mLastError.c_str(); }

private:
	const std::string	&getValue(const std::string &name);
	bool				evaluate();

	typedef std::pair<std::string, std::string> Variable;
	typedef std::vector<Variable> Variables;

	struct Token;

	Variables	mVars;
	bool		mResult;
	std::string	mLastError;
	Token		*mpCurTok;
};
