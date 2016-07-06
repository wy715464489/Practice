//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Spreadsheet Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Json/VuFastContainer.h"


class VuSpreadsheetAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuSpreadsheetAsset() { unload(); }
public:

	VuSpreadsheetAsset() : mpContainer(VUNULL) {}

	int						getColumnIndex(const char *columnName) const;
	int						getRowCount() const { return mpContainer->size() - 1; }
	const VuFastContainer	&getHeader() const { return (*mpContainer)[0]; }
	const VuFastContainer	&getRow(int row) const { return (*mpContainer)[row + 1]; }
	const VuFastContainer	&getField(int row, int column) const { return (*mpContainer)[row + 1][column]; }

	const VuFastContainer	&getField(int row, const char *columnName) const { return (*mpContainer)[row + 1][getColumnIndex(columnName)]; }
	const VuFastContainer	&getField(const VuFastContainer &row, const char *columnName) const { return row[getColumnIndex(columnName)]; }

	static void				schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool				bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

protected:
	static bool				isEmptyRow(const std::string &row);
	static void				readField(const std::string &field, VuJsonContainer &container);
	static bool				readNumber(const std::string &field, VuJsonContainer &container);

	virtual bool			load(VuBinaryDataReader &reader);
	virtual void			unload();

	VuArray<VUBYTE>			mData;
	const VuFastContainer	*mpContainer;
};