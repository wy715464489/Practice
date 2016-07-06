//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Gfx Settings Manager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Gfx/VuGfxSettings.h"
#include "VuEngine/Containers/VuArray.h"

class VuGfxSettingsEntity;


//*****************************************************************************
// Gfx Settings Manager
//*****************************************************************************
class VuGfxSettingsManager
{
public:
	VuGfxSettingsManager();
	~VuGfxSettingsManager();

	// obtaining the interface
	static VuGfxSettingsManager	*IF() { return &mGfxSettingsManagerInterface; }

public:
	void	setGlobal(const VuGfxSettings &settings)		{ mGlobalSettings = settings; }
	void	add(VuGfxSettingsEntity *pGfxSettingsEntity)	{ mPlacedSettings.push_back(pGfxSettingsEntity); }
	void	remove(VuGfxSettingsEntity *pGfxSettingsEntity)	{ mPlacedSettings.remove(pGfxSettingsEntity); }

	void	getSettings(const VuVector3 &vPos, VuGfxSettings &settings);

private:
	// the interface
	static VuGfxSettingsManager	mGfxSettingsManagerInterface;

	// current global settings
	VuGfxSettings	mGlobalSettings;

	// placed settings
	typedef VuArray<VuGfxSettingsEntity *> PlacedSettings;
	PlacedSettings	mPlacedSettings;
};

