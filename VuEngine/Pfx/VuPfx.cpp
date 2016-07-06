//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Low-level Pfx class
// 
//*****************************************************************************

#include "VuPfx.h"
#include "VuPfxRegistry.h"
#include "VuPfxResources.h"
#include "VuPfxGroup.h"
#include "VuPfxSystem.h"
#include "VuPfxPattern.h"
#include "VuPfxProcess.h"
#include "VuPfxParticle.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Pfx/Shaders/VuPfxQuadShader.h"
#include "VuEngine/Pfx/Shaders/VuPfxTrailShader.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Json/VuFastContainer.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevProfile.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuPfx, VuPfx);


//*****************************************************************************
VuPfxConfig::VuPfxConfig():
	mMaxSystemCount(512),
	mMaxSystemSize(sizeof(VuPfxSystemInstance)),
	mMaxPatternCount(1024),
	mMaxPatternSize(sizeof(VuPfxPatternInstance) + 32),
	mMaxProcessCount(4096),
	mMaxProcessSize(sizeof(VuPfxProcessInstance) + 32),
	mMaxParticleCount(16384),
	mMaxParticleSize(128)
{
}

//*****************************************************************************
VuPfx::VuPfx():
	mbDraw(true),
	mbDrawDebug(false)
{
	mpRegistry = new VuPfxRegistry;
	mpResources = new VuPfxResources;
	mpQuadShader = new VuPfxQuadShader;
	mpTrailShader = new VuPfxTrailShader;
}

//*****************************************************************************
VuPfx::~VuPfx()
{
	delete mpRegistry;
	delete mpResources;
	delete mpQuadShader;
	delete mpTrailShader;
}

//*****************************************************************************
bool VuPfx::init()
{
	// configure w/ default settings
	configure(VuPfxConfig());

	// register engine patterns/processes

	// quad pattern specific
	{
		REGISTER_PFX_PATTERN(VuPfxQuadPattern, "Quad");
		REGISTER_PFX_PROCESS(VuPfxEmitQuadFountain, "Emit Fountain", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxEmitDirectionalQuadFountain, "Emit Directional Fountain", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxEmitQuadFountainBurst, "Emit Fountain Burst", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxEmitDirectionalQuadFountainBurst, "Emit Directional Fountain Burst", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxTickAlpha, "Tick Alpha", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxTickAlphaInOut, "Tick Alpha InOut", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxTickScale, "Tick Scale", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxTickWorldScaleZ, "Tick World Scale Z", "VuPfxQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxSoftKillFade, "Soft Kill Fade", "VuPfxQuadPattern");
	}

	// geom pattern specific
	{
		REGISTER_PFX_PATTERN(VuPfxGeomPattern, "Geom");
		REGISTER_PFX_PROCESS(VuPfxEmitGeomFountain, "Emit Fountain", "VuPfxGeomPattern");
		REGISTER_PFX_PROCESS(VuPfxEmitGeomFountainBurst, "Emit Fountain Burst", "VuPfxGeomPattern");
		REGISTER_PFX_PROCESS(VuPfxTickAlpha, "Tick Alpha", "VuPfxGeomPattern");
		REGISTER_PFX_PROCESS(VuPfxTickAlphaInOut, "Tick Alpha InOut", "VuPfxGeomPattern");
		REGISTER_PFX_PROCESS(VuPfxTickScale, "Tick Scale", "VuPfxGeomPattern");
		REGISTER_PFX_PROCESS(VuPfxSoftKillFade, "Soft Kill Fade", "VuPfxGeomPattern");
		REGISTER_PFX_PROCESS(VuPfxOrientDirGeom, "Orient Dir Geom", "VuPfxGeomPattern");
	}

	// trail pattern specific
	{
		REGISTER_PFX_PATTERN(VuPfxTrailPattern, "Trail");
	}

	// recursive pattern specific
	{
		REGISTER_PFX_PATTERN(VuPfxRecursivePattern, "Recursive");
		REGISTER_PFX_PROCESS(VuPfxEmitRecursiveFountain, "Emit Fountain", "VuPfxRecursivePattern");
		REGISTER_PFX_PROCESS(VuPfxTickAlpha, "Tick Alpha", "VuPfxRecursivePattern");
		REGISTER_PFX_PROCESS(VuPfxTickAlphaInOut, "Tick Alpha InOut", "VuPfxRecursivePattern");
		REGISTER_PFX_PROCESS(VuPfxTickScale, "Tick Scale", "VuPfxRecursivePattern");
		REGISTER_PFX_PROCESS(VuPfxSoftKillFade, "Soft Kill Fade", "VuPfxRecursivePattern");
	}

	// orbit quad pattern specific
	{
		REGISTER_PFX_PATTERN(VuPfxOrbitQuadPattern, "Orbit Quad");
		REGISTER_PFX_PROCESS(VuPfxTickAlpha, "Tick Alpha", "VuPfxOrbitQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxTickAlphaInOut, "Tick Alpha InOut", "VuPfxOrbitQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxTickScale, "Tick Scale", "VuPfxOrbitQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxTickWorldScaleZ, "Tick World Scale Z", "VuPfxOrbitQuadPattern");
		REGISTER_PFX_PROCESS(VuPfxSoftKillFade, "Soft Kill Fade", "VuPfxOrbitQuadPattern");
	}

	// common
	{
		REGISTER_PFX_PROCESS(VuPfxTickLinearAcceleration, "Tick Linear Acceleration", "");
		REGISTER_PFX_PROCESS(VuPfxTickDampenVelocity, "Tick Dampen Velocity", "");
		REGISTER_PFX_PROCESS(VuPfxSpringConstraint, "Spring Constraint", "");
	}

	// load shaders
	if ( !mpQuadShader->load() )
		return false;
	if ( !mpTrailShader->load() )
		return false;

	// set up dev menu/stats
	if ( VuDevMenu::IF() )
	{
		VuDevMenu::IF()->addBool("Pfx/Draw", mbDraw);
		VuDevMenu::IF()->addBool("Pfx/DrawDebug", mbDrawDebug);
	}
	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("Pfx", VuRect(50, 10, 40, 40));

	// start ticking
	VuTickManager::IF()->registerHandler(this, &VuPfx::tick, "Final");

	return true;
}

//*****************************************************************************
void VuPfx::release()
{
	// clean up
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
void VuPfx::configure(const VuPfxConfig &config)
{
	mpResources->reallocate(config);
}

//*****************************************************************************
bool VuPfx::addProject(const char *strName, const VuFastContainer &data)
{
	if ( getProject(strName) )
		return VUERROR("Pfx project '%s' already loaded.", strName);

	VuPfxGroup *pPfxGroup = new VuPfxGroup;
	pPfxGroup->load(data);

	mProjects[strName] = pPfxGroup;

	return true;
}

//*****************************************************************************
void VuPfx::removeProject(const char *strName)
{
	Projects::iterator iter = mProjects.find(strName);
	if ( iter != mProjects.end() )
	{
		iter->second->removeRef();
		mProjects.erase(iter);
	}
}

//*****************************************************************************
VuPfxSystemInstance *VuPfx::createSystemInstance(const char *strPath) const
{
	if ( strPath[0] == '\0' )
		return VUNULL;

	if ( VuPfxSystem *pSystem = getSystem("", strPath) )
	{
		if ( VuPfxSystemInstance *pSystemInstance = mpResources->allocateSystem(pSystem) )
		{
			if ( pSystemInstance->create() )
			{
				return pSystemInstance;
			}
			else
			{
				pSystemInstance->destroy();
				mpResources->freeSystem(pSystemInstance);
			}
		}
	}

	return VUNULL;
}

//*****************************************************************************
VuPfxSystemInstance *VuPfx::createSystemInstance(const char *strProject, const char *strPath) const
{
	if ( VuPfxSystem *pSystem = getSystem(strProject, strPath) )
	{
		if ( VuPfxSystemInstance *pSystemInstance = mpResources->allocateSystem(pSystem) )
		{
			if ( pSystemInstance->create() )
			{
				return pSystemInstance;
			}
			else
			{
				pSystemInstance->destroy();
				mpResources->freeSystem(pSystemInstance);
			}
		}
	}

	return VUNULL;
}

//*****************************************************************************
void VuPfx::releaseSystemInstance(VuPfxSystemInstance *pSystemInstance) const
{
	// destruct
	pSystemInstance->destroy();

	// deallocate
	mpResources->freeSystem(pSystemInstance);
}

//*****************************************************************************
void VuPfx::checkForLeaks() const
{
	mpResources->checkForLeaks();
}

//*****************************************************************************
VuPfxGroup *VuPfx::getProject(const char *strName) const
{
	Projects::const_iterator iter = mProjects.find(strName);
	if ( iter != mProjects.end() )
		return iter->second;

	return VUNULL;
}

//*****************************************************************************
VuPfxNode *VuPfx::getNode(const char *strProject, const char *strPath) const
{
	if ( strProject[0] == '\0' )
	{
		for ( const auto &itProject : mProjects )
		{
			if ( VuPfxNode *pNode = getNode(itProject.second, strPath) )
				return pNode;
		}
	}
	else
	{
		if ( VuPfxNode *pProject = getProject(strProject) )
			return getNode(pProject, strPath);
	}

	return VUNULL;
}

//*****************************************************************************
VuPfxGroup *VuPfx::getGroup(const char *strProject, const char *strPath) const
{
	if ( VuPfxNode *pNode = getNode(strProject, strPath) )
		if ( pNode->isDerivedFrom(VuPfxGroup::msRTTI) )
			return static_cast<VuPfxGroup *>(pNode);

	return VUNULL;
}

//*****************************************************************************
VuPfxSystem *VuPfx::getSystem(const char *strProject, const char *strPath) const
{
	if ( VuPfxNode *pNode = getNode(strProject, strPath) )
		if ( pNode->isDerivedFrom(VuPfxSystem::msRTTI) )
			return static_cast<VuPfxSystem *>(pNode);

	return VUNULL;
}

//*****************************************************************************
VuPfxPattern *VuPfx::getPattern(const char *strProject, const char *strPath) const
{
	if ( VuPfxNode *pNode = getNode(strProject, strPath) )
		if ( pNode->isDerivedFrom(VuPfxPattern::msRTTI) )
			return static_cast<VuPfxPattern *>(pNode);

	return VUNULL;
}

//*****************************************************************************
VuPfxProcess *VuPfx::getProcess(const char *strProject, const char *strPath) const
{
	if ( VuPfxNode *pNode = getNode(strProject, strPath) )
		if ( pNode->isDerivedFrom(VuPfxProcess::msRTTI) )
			return static_cast<VuPfxProcess *>(pNode);

	return VUNULL;
}

//*****************************************************************************
void VuPfx::getNamespace(VuJsonContainer &data) const
{
	for ( Projects::const_iterator iter = mProjects.begin(); iter != mProjects.end(); iter++ )
		getNamespaceRecursive(iter->second, data[iter->first]);
}

//*****************************************************************************
VuProperties *VuPfx::getProperties(const char *strPath) const
{
	if ( VuPfxNode *pNode = getNode("", strPath) )
		return &pNode->mProperties;

	return VUNULL;
}

//*****************************************************************************
VuPfxNode *VuPfx::getNode(VuPfxNode *pNode, const char *strPath) const
{
	char str[256];
	VU_STRCPY(str, sizeof(str), strPath);

	char *nextToken = NULL;
	(void)nextToken; // for unused variable warning suppression

	if ( const char *strGroup = VU_STRTOK(str, "/", &nextToken) )
	{
		do
		{
			VuPfxNode::ChildNodes::iterator itChild = pNode->mChildNodes.find(strGroup);
			if ( itChild == pNode->mChildNodes.end() )
			{
				pNode = VUNULL;
				break;
			}
			pNode = itChild->second;

			strGroup = VU_STRTOK(VUNULL, "/", &nextToken);
		}
		while ( strGroup );

		if ( pNode )
			return pNode;
	}

	return VUNULL;
}

//*****************************************************************************
void VuPfx::getNamespaceRecursive(const VuPfxNode *pNode, VuJsonContainer &data) const
{
	for ( VuPfxNode::ChildNodes::const_iterator iter = pNode->mChildNodes.begin(); iter != pNode->mChildNodes.end(); iter++ )
		getNamespaceRecursive(iter->second, data[iter->first]);
}

//*****************************************************************************
void VuPfx::tick(float fdt)
{
	updateDevStats();

	if ( mbDrawDebug )
		drawDebug();

	VuPfx::IF()->resources()->resetParticleCreatedCount();
}

//*****************************************************************************
void VuPfx::updateDevStats()
{
	// dev stats
	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "Pfx" )
			{
				pPage->clear();

				// resources
				{
					const VuPfxResources *pResources = VuPfx::IF()->resources();
					const VuPfxConfig *pConfig = &pResources->config();

					pPage->printf("Resources:\n");
					pPage->printf("  Systems:   %5d / %d\n", pConfig->mMaxSystemCount - pResources->getFreeSystemCount(), pConfig->mMaxSystemCount);
					pPage->printf("  Patterns:  %5d / %d\n", pConfig->mMaxPatternCount - pResources->getFreePatternCount(), pConfig->mMaxPatternCount);
					pPage->printf("  Processes: %5d / %d\n", pConfig->mMaxProcessCount - pResources->getFreeProcessCount(), pConfig->mMaxProcessCount);
					pPage->printf("  Particles: %5d / %d\n", pConfig->mMaxParticleCount - pResources->getFreeParticleCount(), pConfig->mMaxParticleCount);
					pPage->printf("  Entities:  %5d / %d\n", VuPfxManager::IF()->getActiveCount(), VuPfxManager::IF()->getActiveCount() + VuPfxManager::IF()->getFreeCount());

					pPage->printf("This Frame:\n");
					pPage->printf("  Particles Created: %3d\n", pResources->getParticleCreatedCount());
				}
			}
		}
	}
}

//*****************************************************************************
void VuPfx::drawDebug()
{
	int count = mpResources->config().mMaxSystemCount;
	int size = mpResources->config().mMaxSystemSize;
	VUBYTE *pMemory = mpResources->getSystemMemory();
	int flags = VUGFX_TEXT_DRAW_HCENTER|VUGFX_TEXT_DRAW_VCENTER;
	VuColor color(255,255,255);

	for ( int i = 0; i < count; i++ )
	{
		const VuPfxSystemInstance *pSystemInstance = (VuPfxSystemInstance *)&pMemory[i*size];
		if ( pSystemInstance->mpParams )
		{
			const VuMatrix &mat = pSystemInstance->mMatrix;
			VuDev::IF()->printf(mat.getTrans(), flags, color, "%s (%d)", pSystemInstance->mpParams->mName.c_str(), pSystemInstance->mParticleCount);
		}
	}
}