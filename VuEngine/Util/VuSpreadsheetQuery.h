//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Spreadsheet query
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuSpreadsheetAsset.h"


namespace VuSpreadsheetQuery
{
	class VuExpression
	{
	public:
		virtual bool	evaluate(const VuFastContainer &row) const = 0;
		virtual void	optimize(const VuSpreadsheetAsset *pSA) const = 0;
	};

	class VuValueExpression : public VuExpression
	{
	public:
		VuValueExpression(const char *columnName) : mColumnName(columnName) {}

		virtual bool	evaluate(const VuFastContainer &row) const = 0;
		virtual void	optimize(const VuSpreadsheetAsset *pSA) const { mColumnIndex = pSA->getColumnIndex(mColumnName); }

		const char	*mColumnName;
		mutable int	mColumnIndex;
	};

	class VuStringEqual : public VuValueExpression
	{
	public:
		VuStringEqual(const char *columnName, const char *value) : VuValueExpression(columnName), mValue(value) {}
		virtual bool evaluate(const VuFastContainer &row) const { return strcmp(row[mColumnIndex].asCString(), mValue) == 0; }
		const char *mValue;
	};

	class VuIntEqual : public VuValueExpression
	{
	public:
		VuIntEqual(const char *columnName, int value) : VuValueExpression(columnName), mValue(value) {}
		virtual bool evaluate(const VuFastContainer &row) const { return row[mColumnIndex].asInt() == mValue; }
		int mValue;
	};

	class VuIntGreater : public VuValueExpression
	{
	public:
		VuIntGreater(const char *columnName, int value) : VuValueExpression(columnName), mValue(value) {}
		virtual bool evaluate(const VuFastContainer &row) const { return row[mColumnIndex].asInt() > mValue; }
		int mValue;
	};

	class VuIntLess : public VuValueExpression
	{
	public:
		VuIntLess(const char *columnName, int value) : VuValueExpression(columnName), mValue(value) {}
		virtual bool evaluate(const VuFastContainer &row) const { return row[mColumnIndex].asInt() < mValue; }
		int mValue;
	};

	class VuFloatGreater : public VuValueExpression
	{
	public:
		VuFloatGreater(const char *columnName, float value) : VuValueExpression(columnName), mValue(value) {}
		virtual bool evaluate(const VuFastContainer &row) const { return row[mColumnIndex].asFloat() > mValue; }
		float mValue;
	};

	class VuFloatLess : public VuValueExpression
	{
	public:
		VuFloatLess(const char *columnName, float value) : VuValueExpression(columnName), mValue(value) {}
		virtual bool evaluate(const VuFastContainer &row) const { return row[mColumnIndex].asFloat() < mValue; }
		float mValue;
	};

	class VuAnd : public VuExpression
	{
	public:
		VuAnd(const VuExpression &expression1, const VuExpression &expression2) : mExpression1(expression1), mExpression2(expression2) {}
		virtual bool	evaluate(const VuFastContainer &row) const { return mExpression1.evaluate(row) && mExpression2.evaluate(row); }
		virtual void	optimize(const VuSpreadsheetAsset *pSA) const { mExpression1.optimize(pSA); mExpression2.optimize(pSA); }
		const VuExpression	&mExpression1;
		const VuExpression	&mExpression2;
	};

	class VuOr : public VuExpression
	{
	public:
		VuOr(const VuExpression &expression1, const VuExpression &expression2) : mExpression1(expression1), mExpression2(expression2) {}
		virtual bool	evaluate(const VuFastContainer &row) const { return mExpression1.evaluate(row) || mExpression2.evaluate(row); }
		virtual void	optimize(const VuSpreadsheetAsset *pSA) const { mExpression1.optimize(pSA); mExpression2.optimize(pSA); }
		const VuExpression	&mExpression1;
		const VuExpression	&mExpression2;
	};

	class VuNot : public VuExpression
	{
	public:
		VuNot(const VuExpression &expression) : mExpression(expression) {}
		virtual bool	evaluate(const VuFastContainer &row) const { return !mExpression.evaluate(row); }
		virtual void	optimize(const VuSpreadsheetAsset *pSA) const { mExpression.optimize(pSA); }
		const VuExpression	&mExpression;
	};


	int findFirstRow(const VuSpreadsheetAsset *pSA, const VuExpression &expression);
	int findLastRow(const VuSpreadsheetAsset *pSA, const VuExpression &expression);
	int findNextRow(const VuSpreadsheetAsset *pSA, const VuExpression &expression, int prevRowIndex);
}