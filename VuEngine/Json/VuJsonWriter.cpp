//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to write data containers.
// 
//*****************************************************************************

#include "VuJsonWriter.h"
#include "VuJsonContainer.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBase64.h"


//*****************************************************************************
VuJsonWriter::VuJsonWriter():
	mbCompact(false),
	mbTrailingCommas(false),
	mpOutput(0)
{
}

//*****************************************************************************
void VuJsonWriter::configCompact(bool bCompact)
{
	mbCompact = bCompact;
}

//*****************************************************************************
void VuJsonWriter::configTrailingCommas(bool bTrailingCommas)
{
	mbTrailingCommas = bTrailingCommas;
}

//*****************************************************************************
bool VuJsonWriter::saveToFile(const VuJsonContainer &container, const std::string &strFileName)
{
	// save data
	std::string strDoc;
	saveToString(container, strDoc);

	// open file
	VUHANDLE fp = VuFile::IF()->open(strFileName, VuFile::MODE_WRITE);
	if ( fp == VUNULL )
		return false;

	// write UTF-8 header
	VUUINT8 utf8[] = {0xef, 0xbb, 0xbf};
	VuFile::IF()->write(fp, utf8, sizeof(utf8));

	// write data
	VuFile::IF()->write(fp, strDoc.c_str(), (int)strDoc.size());

	// close file
	VuFile::IF()->close(fp);

	return true;
}

//*****************************************************************************
void VuJsonWriter::saveToString(const VuJsonContainer &container, std::string &str)
{
	str.clear();
	mpOutput = &str;

	// recursively write containers
	mstrIndentation.clear();
	writeContainer(container);
}

//*****************************************************************************
void VuJsonWriter::writeContainer(const VuJsonContainer &container)
{
	switch ( container.getType() )
	{
		case VuJsonContainer::nullValue:
		{
			write("null");
			break;
		}
		case VuJsonContainer::intValue:
		{
			writeValue(container.asInt());
			break;
		}
		case VuJsonContainer::floatValue:
		{
			writeValue(container.asFloat());
			break;
		}
		case VuJsonContainer::boolValue:
		{
			writeValue(container.asBool());
			break;
		}
		case VuJsonContainer::stringValue:
		{
			writeValue(container.asString());
			break;
		}
		case VuJsonContainer::arrayValue:
		{
			writeArray(container);
			break;
		}
		case VuJsonContainer::objectValue:
		{
			writeObject(container);
			break;
		}
		case VuJsonContainer::int64Value:
		{
			writeValue(container.asInt64());
			break;
		}
		case VuJsonContainer::binaryValue:
		{
			const void *pData = VUNULL;
			int size = 0;
			container.getValue(pData, size);
			writeValue(pData, size);
			break;
		}
		default:
		{
			VUASSERT(0, "Unknown json container type");
			break;
		}
	}
}

//*****************************************************************************
void VuJsonWriter::writeValue(int iVal)
{
	char buffer[16];
	VU_SPRINTF(buffer, sizeof(buffer), "%d", iVal);
	write(buffer);
}

//*****************************************************************************
void VuJsonWriter::writeValue(float fVal)
{
	char buffer[16];
	VU_SPRINTF(buffer, sizeof(buffer), "%.7g", fVal);
	if ( strcmp(buffer, "-0") == 0 )
		VU_STRCPY(buffer, sizeof(buffer), "0");
	write(buffer);
}

//*****************************************************************************
void VuJsonWriter::writeValue(bool bVal)
{
	write(bVal ? "true" : "false");
}

//*****************************************************************************
void VuJsonWriter::writeValue(const std::string &str)
{
	std::string strOut;
	strOut.reserve(str.length()*2 + 3); // all escaped, quotes, terminator

	strOut += '"'; // leading "

	const char *pCur = str.c_str();
	while ( char c = *pCur )
	{
		switch ( c )
		{
			case '\"': strOut += "\\\""; break;
			case '\\': strOut += "\\\\"; break;
			case '\b': strOut += "\\b"; break;
			case '\f': strOut += "\\f"; break;
			case '\n': strOut += "\\n"; break;
			case '\r': strOut += "\\r"; break;
			case '\t': strOut += "\\t"; break;
			default:
			{
				strOut += c;
			}
			break;
		}

		pCur++;
	}

	strOut += '"'; // trailing "

	write(strOut.c_str());
}

//*****************************************************************************
void VuJsonWriter::writeValue(VUINT64 i64Val)
{
	char buffer[32];
	VU_SPRINTF(buffer, sizeof(buffer), "%lld", i64Val);
	write(buffer);
}

//*****************************************************************************
void VuJsonWriter::writeValue(const void *pData, int size)
{
	std::string str;
	VuBase64::encode((VUBYTE*)pData, size, str);

	write("{\"BinaryBase64\" : \"");
	write(str.c_str());
	write("\"}");
}

//*****************************************************************************
void VuJsonWriter::writeArray(const VuJsonContainer &container)
{
	int size = container.size();
	if ( size == 0 )
	{
		write("[]");
	}
	else
	{
		write("[", true);
		indent();
		for ( int i = 0; i < size; i++ )
		{
			const VuJsonContainer &child = container[i];
			if ( !child.isArray() && !child.isObject() )
				write("", true);
			writeContainer(child);
			if ( i < size - 1 || mbTrailingCommas )
				write(",");
		}
		undent();
		write("]", true);
	}
}

//*****************************************************************************
void VuJsonWriter::writeObject(const VuJsonContainer &container)
{
	std::vector<std::string> keys;
	container.getMemberKeys(keys);
	int size = (int)keys.size();

	write("{", true);
	indent();

	for ( int i = 0; i < size; i++ )
	{
		write("", true);
		writeValue(keys[i]);
		write(mbCompact ? ":" : " : ");
		writeContainer(container[keys[i]]);
		if ( i < size - 1 || mbTrailingCommas )
			write(",");
	}

	undent();
	write("}", true);
}

//*****************************************************************************
void VuJsonWriter::write(const char *str, bool bNewLine)
{
	if ( bNewLine && !mbCompact )
	{
		*mpOutput += '\n';
		*mpOutput += mstrIndentation;
	}
	*mpOutput += str;
}

//*****************************************************************************
void VuJsonWriter::indent()
{
	mstrIndentation += "\t";
}

//*****************************************************************************
void VuJsonWriter::undent()
{
	mstrIndentation.resize(mstrIndentation.size() - 1);
}
