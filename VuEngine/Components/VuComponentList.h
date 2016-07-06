//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  List of components
// 
//*****************************************************************************

#pragma once

class VuJsonContainer;
class VuEntity;
class VuComponent;

class VuComponentList
{
public:

	VuComponentList() : mpHead(VUNULL) {}
	~VuComponentList();

	void				load(const VuJsonContainer &data);
	void				postLoad();
	void				save(VuJsonContainer &data) const;

	void				bake();
	void				clearBaked();

	void				applyTemplate();

	void				gameInitialize();
	void				gameRelease();
	void				gameReset();

	void				add(VuComponent *pComponent);
	void				remove(VuComponent *pComponent);

	// access
	bool				hasComponents() const { return mpHead != VUNULL; }
	template<class T>T	*get() const;
	VuComponent			*getFirst() const { return mpHead; }
	int					getCount() const;
	VuComponent			*getByIndex(int index) const;

private:
	VuComponent			*mpHead;
};

#include "VuComponentList.inl"
