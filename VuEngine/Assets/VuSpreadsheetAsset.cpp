//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Spreadsheet Asset class
// 
//*****************************************************************************

#include "VuSpreadsheetAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuStringUtil.h"


IMPLEMENT_RTTI(VuSpreadsheetAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuSpreadsheetAsset);


typedef std::pair<int, const VuJsonContainer &> SpreadsheetMatch;
typedef std::vector<SpreadsheetMatch> SpreadsheetSearch;


//*****************************************************************************
int VuSpreadsheetAsset::getColumnIndex(const char *columnName) const
{
	const VuFastContainer &fields = (*mpContainer)[0];
	for ( int i = 0; i < fields.size(); i++ )
		if ( strcmp(fields[i].asCString(), columnName) == 0 )
			return i;

	return -1;
}

//*****************************************************************************
void VuSpreadsheetAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Spreadsheets");

	VuAssetUtil::addFileProperty(schema, "File", "csv");
}

//*****************************************************************************
bool VuSpreadsheetAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	const std::string &fileName = creationInfo["File"].asString();

	// load CSV file
	VuArray<VUBYTE> fileData;
	{
		if ( !VuFileUtil::loadFile(VuFile::IF()->getRootPath() + fileName, fileData) )
			return VUWARNING("Unable to load spreadsheet '%s'", fileName.c_str());
	}

	// convert to string
	std::string data;
	{
		data.resize(fileData.size());
		VU_MEMCPY(&data[0], data.size(), &fileData[0], fileData.size());
	}

	// break string into rows
	std::vector<std::string> rows;
	VuStringUtil::tokenize(data, '\n', rows);

	// process rows
	VuJsonContainer container;
	{
		bool doExport = false;
		for ( int iRow = 0; iRow < (int)rows.size(); iRow++ )
		{
			const std::string &row = rows[iRow];
			if ( doExport )
			{
				if ( row.find_first_of("<-- end export -->") == 0 )
				{
					doExport = false;
				}
				else if ( !isEmptyRow(row) )
				{
					VuJsonContainer &dataRow = container.append();

					// break row into columns
					std::vector<std::string> columns;
					VuStringUtil::tokenize(row, ',', columns);

					for ( int iCol = 0; iCol < (int)columns.size(); iCol++ )
						readField(columns[iCol], dataRow[iCol]);
				}
			}
			else if ( row.find_first_of("<-- begin export -->") == 0 )
				doExport = true;
		}
	}

	// write container
	VuFastContainer::serialize(container, bakeParams.mWriter);

	return true;
}

//*****************************************************************************
bool VuSpreadsheetAsset::isEmptyRow(const std::string &row)
{
	for ( std::string::const_iterator iter = row.begin(); iter != row.end(); iter++ )
		if ( (*iter) != ',' )
			return false;

	return true;
}

//*****************************************************************************
void VuSpreadsheetAsset::readField(const std::string &field, VuJsonContainer &container)
{
	if ( !field.empty() )
	{
		// is it a number?
		if ( (field[0] >= '0' && field[0] <= '9') || field[0] == '-' )
		{
			if ( readNumber(field, container) )
				return;
		}

		container.putValue(field);
	}
}

//*****************************************************************************
bool VuSpreadsheetAsset::readNumber(const std::string &field, VuJsonContainer &container)
{
	const char *strField = field.c_str();

	// hex?
	bool bHex = (strField[0] == '0' && strField[1] == 'x');
	
	// get string size of number
	int size = 1;
	while ( char c = strField[size] )
	{
		if ( !(c >= '0' && c <= '9') && 
			c != '.' && 
			c != 'e' && 
			c != 'E' && 
			c != '+' && 
			c != '-' &&
			c != 'x' &&
			!((c >= 'a' && c <= 'f') ||
			  (c >= 'A' && c <= 'F')))
			break;
		size++;
	}

	// check for float
	if(!bHex)
	{
		for ( int i = 1; i < size; i++ )
		{
			if ( !(strField[i] >= '0' && strField[i] <= '9'))
			{
				float fVal;
				if ( VU_SSCANF(field.c_str(), "%g", &fVal) != 1 )
				{
					VUPRINTF("Float parsing error: %s", field.c_str());
					return false;
				}

				container.putValue(fVal);
				return true;
			}
		}
	}

	// parse as integer
	int base;
	
	if(bHex)
	{
		base = 16;
	}
	else
	{
		base = 10;
	}

	// negative?
	bool bNeg = (strField[0] == '-');

	// parse digits
	VUINT64 val = 0;
	int start = 0;

	if(bHex)
	{
		// skip the '0' and 'x'
		start = 2;
	}
	else if(bNeg)
	{
		// skip the '-'
		start = 1;
	}

	for(int i = start; i < size; i++)
	{
		char c = strField[i];

		VUINT64 digit;

		if(c >= 'a' && c <= 'f')
		{
			digit = 10 + VUINT64(c - 'a');
		}
		else if(c >= 'A' && c <= 'F')
		{
			digit = 10 + VUINT64(c - 'A');
		}
		else
		{
			digit = VUINT64(c - '0');
		}

		val = val * base + digit;
	}

	// store as 32 or 64 bit int as appropriate
	VUINT64 i64Val = bNeg ? -val : val;
	if ( i64Val < INT_MIN || i64Val > INT_MAX )
	{
		container.putValue(i64Val);
	}
	else
	{
		int iVal = int(i64Val);
		container.putValue(iVal);
	}

	return true;
}

//*****************************************************************************
bool VuSpreadsheetAsset::load(VuBinaryDataReader &reader)
{
	mData.resize(reader.remaining());
	reader.readData(&mData[0], mData.size());

	mpContainer = VuFastContainer::createInPlace(&mData[0]);

	return true;
}

//*****************************************************************************
void VuSpreadsheetAsset::unload()
{
	mData.deallocate();
	mpContainer = VUNULL;
}
