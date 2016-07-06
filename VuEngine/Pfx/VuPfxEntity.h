//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Entity
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Containers/VuList.h"

class Vu3dDrawComponent;
class VuGfxDrawParams;
class VuGfxDrawShadowParams;
class VuPfxSystemInstance;


class VuPfxEntity : public VuEntity, public VuListElement<VuPfxEntity>
{
	DECLARE_RTTI

public:
	VuPfxEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

	VuPfxSystemInstance	*getSystemInstance()	{ return mpPfxSystemInstance; }

	void				enableReflection(bool bEnable);
	void				enableShadow(bool bEnable);

protected:
	friend class VuPfxManager;

	void				draw(const VuGfxDrawParams &params);
	void				drawShadow(const VuGfxDrawShadowParams &params);

	// components
	Vu3dDrawComponent	*mp3dDrawComponent;

	VuPfxSystemInstance	*mpPfxSystemInstance;

	// handle
	VUUINT32			mHandleSlot;
	VUUINT32			mHandleCount;
};
