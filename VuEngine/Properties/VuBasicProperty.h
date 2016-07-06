//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Basic property class
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuRect.h"


template <class T, VuProperty::eType type> class VuBasicProperty : public VuProperty
{
public:
	VuBasicProperty(const char *strName, T &value);

	virtual eType		getType() const { return type; }

	virtual void		load(const VuJsonContainer &data);
	virtual void		save(VuJsonContainer &data) const;
	virtual void		load(const VuFastContainer &data);

	virtual void		setCurrent(const VuJsonContainer &data, bool notify = true);
	virtual void		getCurrent(VuJsonContainer &data) const;

	virtual void		getDefault(VuJsonContainer &data) const;
	virtual void		updateDefault()	{ mDefaultValue = mValue; }

	virtual void		reset() { mValue = mInitialValue; }

protected:
	// allow derived classes to transform values (e.g. degrees/radians)
	virtual T			transformToNative(const T &formalValue) const	{ return formalValue; }	
	virtual T			transformFromNative(const T &nativeValue) const	{ return nativeValue; }

	T	mDefaultValue;
	T	mInitialValue;
	T	&mValue;
};

typedef VuBasicProperty<int, VuProperty::INTEGER>			VuIntProperty;
typedef VuBasicProperty<float, VuProperty::FLOAT>			VuFloatProperty;
typedef VuBasicProperty<bool, VuProperty::BOOLEAN>			VuBoolProperty;
typedef VuBasicProperty<VuColor, VuProperty::COLOR>			VuColorProperty;
typedef VuBasicProperty<VuVector2, VuProperty::VECTOR2D>	VuVector2Property;
typedef VuBasicProperty<VuVector3, VuProperty::VECTOR3D>	VuVector3Property;
typedef VuBasicProperty<VuRect, VuProperty::RECT>			VuRectProperty;


#include "VuBasicProperty.inl"
