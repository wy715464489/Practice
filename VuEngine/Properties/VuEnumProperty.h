//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Enum property class
// 
//*****************************************************************************

#pragma once

#include "VuBasicProperty.h"
#include "VuStringProperty.h"


//*****************************************************************************
// Enum property base classes.
//*****************************************************************************
class VuStringEnumProperty : public VuStringProperty
{
public:
	VuStringEnumProperty(const char *strName, std::string &pValue) : VuStringProperty(strName, pValue) {}

	virtual eType		getType() const { return ENUM_STRING; }

	virtual int			getChoiceCount() const = 0;
	virtual const char	*getChoice(int index) const = 0;
};

class VuIntEnumProperty : public VuIntProperty
{
public:
	VuIntEnumProperty(const char *strName, int &pValue) : VuIntProperty(strName, pValue) {}

	virtual eType		getType() const { return ENUM_INT; }

	virtual void		load(const VuFastContainer &data);

	virtual void		setCurrent(const VuJsonContainer &data, bool notify = true);
	virtual void		getCurrent(VuJsonContainer &data) const;
	virtual void		getDefault(VuJsonContainer &data) const;

	virtual int			getChoiceCount() const = 0;
	virtual const char	*getChoiceName(int index) const = 0;
	virtual int			getChoiceValue(int index) const = 0;

	const char			*getCurChoiceName() const;

private:
	VuJsonContainer		translateChoice(const VuJsonContainer &value) const;
};


//*****************************************************************************
// Static int enum property.  Lowest storage overhead.
//*****************************************************************************
class VuStaticIntEnumProperty : public VuIntEnumProperty
{
public:
	struct Choice
	{
		const char	*mpName;
		int			mValue;
	};

	VuStaticIntEnumProperty(const char *strName, int &pValue, const Choice *choices);

	virtual int			getChoiceCount() const;
	virtual const char	*getChoiceName(int index) const	{ return mpChoices[index].mpName; }
	virtual int			getChoiceValue(int index) const	{ return mpChoices[index].mValue; }

private:
	const Choice	*mpChoices;
};

//*****************************************************************************
// Static string enum property.  Low storage overhead.
//*****************************************************************************
class VuStaticStringEnumProperty : public VuStringEnumProperty
{
public:
	struct Choice
	{
		const char	*mpName;
	};

	VuStaticStringEnumProperty(const char *strName, std::string &pValue, const Choice *choices);

	virtual int			getChoiceCount() const;
	virtual const char	*getChoice(int index) const	{ return mpChoices[index].mpName; }

private:
	const Choice	*mpChoices;
};

//*****************************************************************************
// Const string enum property.  Choices are stored in this class.
//*****************************************************************************
class VuConstStringEnumProperty : public VuStringEnumProperty
{
public:
	typedef std::vector<std::string> Choices;

	VuConstStringEnumProperty(const char *strName, std::string &pValue, const Choices &choices) : VuStringEnumProperty(strName, pValue), mChoices(choices) {}

	virtual int			getChoiceCount() const		{ return (int)mChoices.size(); }
	virtual const char	*getChoice(int index) const	{ return mChoices[index].c_str(); }

private:
	const Choices	&mChoices;
};

//*****************************************************************************
// Dynamic enum property.  Choices are stored in this class.
//*****************************************************************************
class VuDynamicStringEnumProperty : public VuStringEnumProperty
{
public:
	VuDynamicStringEnumProperty(const char *strName, std::string &pValue) : VuStringEnumProperty(strName, pValue) {}

	virtual int			getChoiceCount() const		{ return (int)mChoices.size(); }
	virtual const char	*getChoice(int index) const	{ return mChoices[index].c_str(); }

	void	clearChoices()							{ mChoices.clear(); }
	void	addChoice(const std::string &choice)	{ mChoices.push_back(choice); }

private:
	typedef std::vector<std::string> Choices;

	Choices	mChoices;
};

//*****************************************************************************
// Enum property for JSON array of strings.  Low storage overhead.
//*****************************************************************************
class VuJsonStringArrayEnumProperty : public VuStringEnumProperty
{
public:
	VuJsonStringArrayEnumProperty(const char *strName, std::string &pValue, const VuJsonContainer &choices) : VuStringEnumProperty(strName, pValue), mChoices(choices) {}

	virtual int			getChoiceCount() const		{ return mChoices.size(); }
	virtual const char	*getChoice(int index) const	{ return mChoices[index].asCString(); }

private:
	const VuJsonContainer	&mChoices;
};

//*****************************************************************************
// Enum property for JSON object member names.  Low storage overhead.
//*****************************************************************************
class VuJsonObjectEnumProperty : public VuStringEnumProperty
{
public:
	VuJsonObjectEnumProperty(const char *strName, std::string &pValue, const VuJsonContainer &choices) : VuStringEnumProperty(strName, pValue), mChoices(choices) {}

	virtual int			getChoiceCount() const		{ return mChoices.numMembers(); }
	virtual const char	*getChoice(int index) const	{ return mChoices.getMemberKey(index).c_str(); }

private:
	const VuJsonContainer	&mChoices;
};
