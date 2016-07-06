//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Spreadsheet query
// 
//*****************************************************************************

#include "VuSpreadsheetQuery.h"


//*****************************************************************************
int VuSpreadsheetQuery::findFirstRow(const VuSpreadsheetAsset *pSA, const VuExpression &expression)
{
	expression.optimize(pSA);

	for ( int iRow = 0; iRow < pSA->getRowCount(); iRow++ )
	{
		const VuFastContainer &row = pSA->getRow(iRow);
		if ( expression.evaluate(row) )
			return iRow;
	}

	return pSA->getRowCount();
}

//*****************************************************************************
int VuSpreadsheetQuery::findLastRow(const VuSpreadsheetAsset *pSA, const VuExpression &expression)
{
	expression.optimize(pSA);

	for ( int iRow = pSA->getRowCount() - 1; iRow >= 0; iRow-- )
	{
		const VuFastContainer &row = pSA->getRow(iRow);
		if ( expression.evaluate(row) )
			return iRow;
	}

	return pSA->getRowCount();
}

//*****************************************************************************
int VuSpreadsheetQuery::findNextRow(const VuSpreadsheetAsset *pSA, const VuExpression &expression, int prevRowIndex)
{
	// make sure row is in range
	if ( prevRowIndex >= 0 && prevRowIndex < pSA->getRowCount() )
	{
		for ( int iRow = prevRowIndex + 1; iRow < pSA->getRowCount(); iRow++ )
		{
			const VuFastContainer &row = pSA->getRow(iRow);
			if ( expression.evaluate(row) )
				return iRow;
		}
	}

	return pSA->getRowCount();
}
