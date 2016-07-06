//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Run Time Type Information
// 
//*****************************************************************************

#pragma once


struct VuRTTI
{
	explicit VuRTTI(const char *strType, const VuRTTI *pBaseType = VUNULL) : mstrType(strType), mpBaseType(pBaseType) {}
	bool isDerivedFrom(const VuRTTI &rtti) const
	{
		for ( const VuRTTI *p = this; p; p = p->mpBaseType )
			if ( p == &rtti )
				return true;
		return false;
	}
	bool isDerivedFrom(const char *type) const
	{
		for ( const VuRTTI *p = this; p; p = p->mpBaseType )
			if ( strcmp(p->mstrType, type) == 0 )
				return true;
		return false;
	}

	const char		*mstrType;
	const VuRTTI	*mpBaseType;
};

//*****************************************************************************
// Macro used to declare RTTI in the class definition.
#define DECLARE_RTTI																										\
public:																														\
	virtual const VuRTTI		&getRTTI() const						{ return msRTTI; }									\
	const char					*getType() const						{ return getRTTI().mstrType; }						\
	bool						isDerivedFrom(const VuRTTI &rtti) const	{ return getRTTI().isDerivedFrom(rtti); }			\
	bool						isDerivedFrom(const char *type) const	{ return getRTTI().isDerivedFrom(type); }			\
	bool						isType(const VuRTTI &rtti) const		{ return &getRTTI() == &rtti; }						\
	bool						isType(const char *type) const			{ return strcmp(getRTTI().mstrType, type) == 0; }	\
	template <class T> const T	*safeCast() const { return (this && isDerivedFrom(T::msRTTI)) ? static_cast<const T *>(this) : VUNULL; }	\
	template <class T> T		*safeCast() { return (this && isDerivedFrom(T::msRTTI)) ? static_cast<T *>(this) : VUNULL; }				\
																															\
	static const VuRTTI msRTTI;																								\
private:

//*****************************************************************************
// Macro used to implement RTTI for the base class.
#define IMPLEMENT_RTTI_BASE(type) \
	const VuRTTI type::msRTTI(#type)

//*****************************************************************************
// Macro used to implement RTTI for a derived class.
#define IMPLEMENT_RTTI(type, baseType) \
	const VuRTTI type::msRTTI(#type, &baseType::msRTTI)
