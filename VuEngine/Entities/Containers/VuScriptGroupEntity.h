//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Group Entity.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"

class VuScriptComponent;
class VuScriptPlug;
class VuVector2;


class VuScriptGroupEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuScriptGroupEntity();

	// these methods may be slow, only to be used in the Editor
	bool			isCollapsed() const { return mCollapsed; }
	int				getWidth() const { return mWidth; }
	int				getDepth() const { return mDepth; }
	void			setDepth(int depth) { mDepth = depth; }

	VuVector2		getPosition() const;
	void			setPosition(const VuVector2 &pos);
	int				getNumPlugs() const { return countPlugsRecursive(this); }
	VuScriptPlug	*getPlug(int index) const { return getPlugRecursive(this, index); }
	int				getNumPlugsOfType(bool bInput) const { return countNumPlugsOfTypeRecursive(this, bInput); }

protected:
	void			moveRecursive(const VuEntity *pEntity, const VuVector2 &delta) const;
	int				countPlugsRecursive(const VuEntity *pEntity) const;
	VuScriptPlug	*getPlugRecursive(const VuEntity *pEntity, int &index) const;
	int				countNumPlugsOfTypeRecursive(const VuEntity *pEntity, bool bInput) const;
	bool			getBoundsRecursive(const VuEntity *pEntity, VuVector2 &posMin, VuVector2 &posMax) const;

	// properties
	bool	mCollapsed;
	int		mWidth;
	int		mDepth;
};


class VuScriptGroupConnectionEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuScriptGroupConnectionEntity();

	VuScriptPlug		*getInputPlug() { return mpInputPlug; }
	VuScriptPlug		*getOutputPlug() { return mpOutputPlug; }

protected:
	VuRetVal			In(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// plugs
	VuScriptPlug		*mpInputPlug;
	VuScriptPlug		*mpOutputPlug;
};

class VuScriptGroupInputEntity : public VuScriptGroupConnectionEntity
{
	DECLARE_RTTI
};

class VuScriptGroupOutputEntity : public VuScriptGroupConnectionEntity
{
	DECLARE_RTTI
};
