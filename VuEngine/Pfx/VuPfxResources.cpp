//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Resources
// 
//*****************************************************************************

#include "VuPfxResources.h"
#include "VuPfxSystem.h"
#include "VuPfxPattern.h"
#include "VuPfxProcess.h"
#include "VuPfxParticle.h"


//*****************************************************************************
VuPfxResources::VuPfxResources():
	mpSystemMemory(VUNULL),
	mpPatternMemory(VUNULL),
	mpProcessMemory(VUNULL),
	mpParticleMemory(VUNULL),
	mParticleCreatedCount(0)
{
}

//*****************************************************************************
VuPfxResources::~VuPfxResources()
{
	freeMemory();
}

//*****************************************************************************
void VuPfxResources::reallocate(const VuPfxConfig &config)
{
	mConfig = config;

	freeMemory();
	allocateMemory();

	// add all elements to free lists
	#define ADD_TO_FREE_LIST(freelist, type, memory, count, size)	\
	{																\
		freelist.clear();											\
		memset(memory, 0, count*size);								\
		for ( int i = 0; i < count; i++ )							\
			freelist.push_back((type *)&memory[i*size]);			\
	}

	ADD_TO_FREE_LIST(mFreeSystems, VuPfxSystemInstance, mpSystemMemory, mConfig.mMaxSystemCount, mConfig.mMaxSystemSize);
	ADD_TO_FREE_LIST(mFreePatterns, VuPfxPatternInstance, mpPatternMemory, mConfig.mMaxPatternCount, mConfig.mMaxPatternSize);
	ADD_TO_FREE_LIST(mFreeProcesses, VuPfxProcessInstance, mpProcessMemory, mConfig.mMaxProcessCount, mConfig.mMaxProcessSize);
	ADD_TO_FREE_LIST(mFreeParticles, VuPfxParticle, mpParticleMemory, mConfig.mMaxParticleCount, mConfig.mMaxParticleSize);

	#undef ADD_TO_FREE_LIST
}

//*****************************************************************************
VuPfxSystemInstance *VuPfxResources::allocateSystem(VuPfxSystem *pParams)
{
	if ( VuPfxSystemInstance *pInstance = mFreeSystems.pop_back() )
	{
		new (pInstance) VuPfxSystemInstance;
		pInstance->mpParams = pParams;

		mActiveSystems.push_back(pInstance);

		return pInstance;
	}
	else
	{
		VUPRINTF("Out of Pfx System resources.\n");
	}

	return VUNULL;
}

//*****************************************************************************
VuPfxPatternInstance *VuPfxResources::allocatePattern(VuPfxPattern *pParams)
{
	if ( pParams->instanceSize() <= mConfig.mMaxPatternSize )
	{
		if ( VuPfxPatternInstance *pInstance = mFreePatterns.pop_back() )
		{
			pParams->constructInstance(pInstance);
			pInstance->mpParams = pParams;

			return pInstance;
		}
		else
		{
			VUPRINTF("Out of Pfx Pattern resources.\n");
		}
	}
	else
	{
		VUWARNING("Max pattern size of %d exceeded by %s (%d).\n", mConfig.mMaxPatternSize, pParams->getType(), pParams->instanceSize());
		VUASSERT(0, "Max pfx pattern size of exceeded!");
	}

	return VUNULL;
}

//*****************************************************************************
VuPfxProcessInstance *VuPfxResources::allocateProcess(VuPfxProcess *pParams)
{
	if ( pParams->instanceSize() <= mConfig.mMaxProcessSize )
	{
		if ( VuPfxProcessInstance *pInstance = mFreeProcesses.pop_back() )
		{
			pParams->constructInstance(pInstance);
			pInstance->mpParams = pParams;

			return pInstance;
		}
		else
		{
			VUPRINTF("Out of Pfx Process resources.\n");
		}
	}
	else
	{
		VUWARNING("Max Process size of %d exceeded by %s (%d).\n", mConfig.mMaxProcessSize, pParams->getType(), pParams->instanceSize());
		VUASSERT(0, "Max pfx process size of exceeded!");
	}

	return VUNULL;
}

//*****************************************************************************
VuPfxParticle *VuPfxResources::allocateParticle(const VuPfxPattern *pParams)
{
	if ( pParams->particleSize() <= mConfig.mMaxParticleSize )
	{
		if ( VuPfxParticle *pParticle = mFreeParticles.pop_back() )
		{
			pParams->constructParticle(pParticle);

			mParticleCreatedCount++;

			return pParticle;
		}
		else
		{
			VUPRINTF("Out of Pfx Particle resources.\n");
		}
	}
	else
	{
		VUWARNING("Max Particle size of %d exceeded by %s (%d).\n", mConfig.mMaxParticleSize, pParams->getType(), pParams->particleSize());
		VUASSERT(0, "Max pfx particle size of exceeded!");
	}

	return VUNULL;
}

//*****************************************************************************
void VuPfxResources::freeSystem(VuPfxSystemInstance *pSystem)
{
	mActiveSystems.remove(pSystem);
	mFreeSystems.push_back(pSystem);
	pSystem->mpParams = VUNULL;
}

//*****************************************************************************
void VuPfxResources::freePattern(VuPfxPatternInstance *pPattern)	{ mFreePatterns.push_back(pPattern); }
void VuPfxResources::freeProcess(VuPfxProcessInstance *pProcess)	{ mFreeProcesses.push_back(pProcess); }
void VuPfxResources::freeParticle(VuPfxParticle *pParticle)			{ mFreeParticles.push_back(pParticle); }

//*****************************************************************************
void VuPfxResources::checkForLeaks() const
{
	if ( mActiveSystems.size() )
	{
		VUPRINTF("%d Pfx Resource leaks:\n", mActiveSystems.size());

		#ifndef VURETAIL
			std::map<std::string, int> leaks;
			for ( const VuPfxSystemInstance *p = mActiveSystems.front(); p; p = p->next() )
			{
				const auto &iter = leaks.find(p->mpParams->mName);
				if ( iter == leaks.end() )
					leaks[p->mpParams->mName] = 1;
				else
					iter->second++;
			}

			for ( const auto &iter : leaks )
				VUPRINTF("%s : %d\n", iter.first.c_str(), iter.second);
		#endif
	}
}

//*****************************************************************************
void VuPfxResources::allocateMemory()
{
	// align to 16 bytes
	mConfig.mMaxSystemSize = VuAlign(mConfig.mMaxSystemSize, 16);
	mConfig.mMaxPatternSize = VuAlign(mConfig.mMaxPatternSize, 16);
	mConfig.mMaxProcessSize = VuAlign(mConfig.mMaxProcessSize, 16);
	mConfig.mMaxParticleSize = VuAlign(mConfig.mMaxParticleSize, 16);

	mpSystemMemory = new VUBYTE[mConfig.mMaxSystemCount*mConfig.mMaxSystemSize];
	mpPatternMemory = new VUBYTE[mConfig.mMaxPatternCount*mConfig.mMaxPatternSize];
	mpProcessMemory = new VUBYTE[mConfig.mMaxProcessCount*mConfig.mMaxProcessSize];
	mpParticleMemory = new VUBYTE[mConfig.mMaxParticleCount*mConfig.mMaxParticleSize];
}

//*****************************************************************************
void VuPfxResources::freeMemory()
{
	delete mpSystemMemory;
	delete mpPatternMemory;
	delete mpProcessMemory;
	delete mpParticleMemory;
}
