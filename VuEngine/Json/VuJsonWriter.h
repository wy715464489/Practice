//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to write data containers.
// 
//*****************************************************************************

#pragma once


class VuJsonContainer;


class VuJsonWriter
{
public:
	VuJsonWriter();

	// configuration
	void			configCompact(bool bCompact);
	void			configTrailingCommas(bool bTrailingCommas);

	bool			saveToFile(const VuJsonContainer &container, const std::string &strFileName);
	void			saveToString(const VuJsonContainer &container, std::string &str);

private:

	void			writeContainer(const VuJsonContainer &container);
	void			writeValue(int iVal);
	void			writeValue(float fVal);
	void			writeValue(bool bVal);
	void			writeValue(const std::string &strVal);
	void			writeValue(VUINT64 i64Val);
	void			writeValue(const void *pData, int size);
	void			writeArray(const VuJsonContainer &container);
	void			writeObject(const VuJsonContainer &container);
	
	void			write(const char *str, bool bNewLine = false);
	void			indent();
	void			undent();

	bool			mbCompact;
	bool			mbTrailingCommas;
	std::string		*mpOutput;
	std::string		mstrIndentation;
};