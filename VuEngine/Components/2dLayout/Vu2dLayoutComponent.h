//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  2dLayoutComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Method/VuMethod.h"


class Vu2dLayoutComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(2dLayout)
	DECLARE_RTTI

public:
	template<class T>
	Vu2dLayoutComponent(T *pOwner, void (T::*drawMethod)(bool bSelected));
	~Vu2dLayoutComponent();

	void	draw(bool bSelected) const;

private:
	typedef VuMethodInterface1<void, bool> DrawMethod;

	DrawMethod	*mpDrawMethod;
};

#include "Vu2dLayoutComponent.inl"
