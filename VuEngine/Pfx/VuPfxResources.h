//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Resources
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Containers/VuList.h"

class VuPfxSystem;
class VuPfxPattern;
class VuPfxProcess;
class VuPfxSystemInstance;
class VuPfxPatternInstance;
class VuPfxProcessInstance;
class VuPfxParticle;


class VuPfxResources
{
public:
	VuPfxResources();
	~VuPfxResources();

	void					reallocate(const VuPfxConfig &config);

	// creation
	VuPfxSystemInstance		*allocateSystem(VuPfxSystem *pParams);
	VuPfxPatternInstance	*allocatePattern(VuPfxPattern *pParams);
	VuPfxProcessInstance	*allocateProcess(VuPfxProcess *pParams);
	VuPfxParticle			*allocateParticle(const VuPfxPattern *pParams);

	// release
	void					freeSystem(VuPfxSystemInstance *pSystem);
	void					freePattern(VuPfxPatternInstance *pPattern);
	void					freeProcess(VuPfxProcessInstance *pProcess);
	void					freeParticle(VuPfxParticle *pParticle);

	// config
	const VuPfxConfig		&config() const	{ return mConfig; }

	// stats
	int						getFreeSystemCount() const		{ return mFreeSystems.size(); }
	int						getFreePatternCount() const		{ return mFreePatterns.size(); }
	int						getFreeProcessCount() const		{ return mFreeProcesses.size(); }
	int						getFreeParticleCount() const	{ return mFreeParticles.size(); }
	int						getParticleCreatedCount() const	{ return mParticleCreatedCount; }

	void					resetParticleCreatedCount()		{ mParticleCreatedCount = 0; }

	// used for debugging only
	VUBYTE					*getSystemMemory() const	{ return mpSystemMemory; }
	void					checkForLeaks() const;

private:
	void					allocateMemory();
	void					freeMemory();

	VUBYTE					*mpSystemMemory;
	VUBYTE					*mpPatternMemory;
	VUBYTE					*mpProcessMemory;
	VUBYTE					*mpParticleMemory;

	typedef VuList<VuPfxSystemInstance> Systems;
	typedef VuList<VuPfxPatternInstance> Patterns;
	typedef VuList<VuPfxProcessInstance> Processes;
	typedef VuList<VuPfxParticle> Particles;

	Systems					mFreeSystems;
	Patterns				mFreePatterns;
	Processes				mFreeProcesses;
	Particles				mFreeParticles;

	Systems					mActiveSystems;

	VuPfxConfig				mConfig;

	// stats
	int						mParticleCreatedCount;
};
