//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Engine library.
// 
//*****************************************************************************

#include "VuEngine.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/HAL/Net/VuNet.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Audio/VuAudio.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/HAL/GamePad/VuGamePad.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/HAL/Accel/VuAccel.h"
#include "VuEngine/HAL/Touch/VuTouch.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Services/VuServiceManager.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/Entities/VuEntityFactory.h"
#include "VuEngine/Entities/VuEntityRepository.h"
#include "VuEngine/DB/VuStringDB.h"
#include "VuEngine/DB/VuFontDB.h"
#include "VuEngine/Telemetry/VuTelemetryClient.h"
#include "VuEngine/Telemetry/VuTelemetryServer.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxComposer/VuGfxComposer.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevHostComm.h"
#include "VuEngine/Dev/VuDevConfig.h"
#include "VuEngine/Dev/VuDevConsole.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Dev/VuDevTimer.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawManager.h"
#include "VuEngine/Math/VuRand.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Managers/VuTriggerManager.h"
#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/Managers/VuInputManager.h"
#include "VuEngine/Managers/VuFoliageManager.h"
#include "VuEngine/Managers/VuLensWaterManager.h"
#include "VuEngine/Managers/VuTireTrackManager.h"
#include "VuEngine/Managers/VuExplosionManager.h"
#include "VuEngine/Managers/VuToastManager.h"
#include "VuEngine/Managers/VuMessageBoxManager.h"
#include "VuEngine/Managers/VuDialogManager.h"
#include "VuEngine/Managers/VuLicenseManager.h"
#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Net/VuNearbyConnectionManager.h"
#include "VuEngine/Cloud/VuCloudManager.h"
#include "VuEngine/UI/VuUI.h"


// the engine
VuEngine VuEngine::mEngine;


// macros
#define CREATE_COMPONENT(name, initResult)				\
{														\
	if ( VuSys::IF() )									\
	{													\
		VUPRINTF("Creating %s\n", #name);				\
	}													\
	extern name *Create##name##Interface();				\
	mSysComponents.push_back(Create##name##Interface());\
	if ( !(initResult) )								\
	{													\
		return VUERROR("Failed to init %s.", #name);	\
	}													\
}

//*****************************************************************************
bool VuEngine::init(const VuEngine::Options &options)
{
	// save options
	mOptions = options;

	// required system components
	CREATE_COMPONENT(VuThread,					VuThread::IF()->init());
	CREATE_COMPONENT(VuSys,						VuSys::IF()->init(options.mSysForceLanguage.c_str(), options.mSysLogFileName.c_str()));
	CREATE_COMPONENT(VuTickManager,				VuTickManager::IF()->init());
	CREATE_COMPONENT(VuDrawManager,				VuDrawManager::IF()->init());
	CREATE_COMPONENT(VuEventManager,			VuEventManager::IF()->init());
	CREATE_COMPONENT(VuServiceManager,			VuServiceManager::IF()->init());
	CREATE_COMPONENT(VuViewportManager,			VuViewportManager::IF()->init());
	CREATE_COMPONENT(VuGfxSort,					VuGfxSort::IF()->init(options.mbGfxSortAsynchronousRendering));
	
	// hardware interfaces
	if ( mOptions.mbNet )		CREATE_COMPONENT(VuNet,			VuNet::IF()->init());
		// create these dev systems ASAP
		if ( mOptions.mbDevHostComm )	CREATE_COMPONENT(VuDevHostComm,		VuDevHostComm::IF()->init(options.mstrDevHostCommAddress));
	if ( mOptions.mbFile )		CREATE_COMPONENT(VuFile,		VuFile::IF()->init(options.mstrFileRootPath, options.mstrFileProjectName));
		// create these dev systems ASAP
		if ( mOptions.mbDevConfig )	CREATE_COMPONENT(VuDevConfig,	VuDevConfig::IF()->init());
	if ( mOptions.mbGfx )		CREATE_COMPONENT(VuGfx,			VuGfx::IF()->init(options.mhGfxWindow, options.mhGfxDevice));
	if ( mOptions.mbAudio )		CREATE_COMPONENT(VuAudio,		VuAudio::IF()->init());
	if ( mOptions.mbKeyboard )	CREATE_COMPONENT(VuKeyboard,	VuKeyboard::IF()->init());
	if ( mOptions.mbGamePad )	CREATE_COMPONENT(VuGamePad,		VuGamePad::IF()->init());
	if ( mOptions.mbAccel )		CREATE_COMPONENT(VuAccel,		VuAccel::IF()->init());
	if ( mOptions.mbTouch )		CREATE_COMPONENT(VuTouch,		VuTouch::IF()->init());

	// dev systems
	if ( mOptions.mbDev )			CREATE_COMPONENT(VuDev,			VuDev::IF()->init());
	if ( mOptions.mbDevConsole )	CREATE_COMPONENT(VuDevConsole,	VuDevConsole::IF()->init());
	if ( mOptions.mbDevMenu )		CREATE_COMPONENT(VuDevMenu,		VuDevMenu::IF()->init());
	if ( mOptions.mbDevStat )		CREATE_COMPONENT(VuDevStat,		VuDevStat::IF()->init());
	if ( mOptions.mbDevProfile )	CREATE_COMPONENT(VuDevProfile,	VuDevProfile::IF()->init());
	if ( mOptions.mbDevTimer )		CREATE_COMPONENT(VuDevTimer,	VuDevTimer::IF()->init());

	// other systems
	if ( mOptions.mbAssetFactory )				CREATE_COMPONENT(VuAssetFactory,			VuAssetFactory::IF()->init(mOptions.mpRegisterGameAssetsFn, mOptions.mSku));
	if ( mOptions.mbAssetBaker )				CREATE_COMPONENT(VuAssetBakery,				VuAssetBakery::IF()->init());
	if ( mOptions.mbConfigManager )				CREATE_COMPONENT(VuConfigManager,			VuConfigManager::IF()->init(mOptions.mDeviceType));
	if ( mOptions.mbEntityFactory )				CREATE_COMPONENT(VuEntityFactory,			VuEntityFactory::IF()->init());
	if ( mOptions.mbEntityRepository )			CREATE_COMPONENT(VuEntityRepository,		VuEntityRepository::IF()->init());
	if ( mOptions.mbGfxUtil )					CREATE_COMPONENT(VuGfxUtil,					VuGfxUtil::IF()->init());
	if ( mOptions.mbGfxComposer )				CREATE_COMPONENT(VuGfxComposer,				VuGfxComposer::IF()->init());
	if ( mOptions.mbStringDB )					CREATE_COMPONENT(VuStringDB,				VuStringDB::IF()->init());
	if ( mOptions.mbFontDB )					CREATE_COMPONENT(VuFontDB,					VuFontDB::IF()->init());
	if ( mOptions.mbTelemetryClient )			CREATE_COMPONENT(VuTelemetryClient,			VuTelemetryClient::IF()->init());
	if ( mOptions.mbTelemetryServer )			CREATE_COMPONENT(VuTelemetryServer,			VuTelemetryServer::IF()->init());
	if ( mOptions.mb3dDrawManager )				CREATE_COMPONENT(Vu3dDrawManager,			Vu3dDrawManager::IF()->init());
	if ( mOptions.mbLightManager )				CREATE_COMPONENT(VuLightManager,			VuLightManager::IF()->init());
	if ( mOptions.mbWater )						CREATE_COMPONENT(VuWater,					VuWater::IF()->init(options.mbWaterRendererAsync));
	if ( mOptions.mbDynamics )					CREATE_COMPONENT(VuDynamics,				VuDynamics::IF()->init(options.mbDynamicsAsync, options.mDynamicsMaxSubSteps, options.mDynamicsFixedTimeStep));
	if ( mOptions.mbPfx )						CREATE_COMPONENT(VuPfx,						VuPfx::IF()->init());
	if ( mOptions.mbPfxManager )				CREATE_COMPONENT(VuPfxManager,				VuPfxManager::IF()->init());
	if ( mOptions.mbTriggerManager )			CREATE_COMPONENT(VuTriggerManager,			VuTriggerManager::IF()->init());
	if ( mOptions.mbProfileManager )			CREATE_COMPONENT(VuProfileManager,			VuProfileManager::IF()->init(mOptions.mGameName));
	if ( mOptions.mbInputManager )				CREATE_COMPONENT(VuInputManager,			VuInputManager::IF()->init());
	if ( mOptions.mbFoliageManager )			CREATE_COMPONENT(VuFoliageManager,			VuFoliageManager::IF()->init());
	if ( mOptions.mbLensWaterManager )			CREATE_COMPONENT(VuLensWaterManager,		VuLensWaterManager::IF()->init());
	if ( mOptions.mbTireTrackManager )			CREATE_COMPONENT(VuTireTrackManager,		VuTireTrackManager::IF()->init());
	if ( mOptions.mbHttpClient )				CREATE_COMPONENT(VuHttpClient,				VuHttpClient::IF()->init());
	if ( mOptions.mbCloudManager )				CREATE_COMPONENT(VuCloudManager,			VuCloudManager::IF()->init());
	if ( mOptions.mbExplosionManager )			CREATE_COMPONENT(VuExplosionManager,		VuExplosionManager::IF()->init());
	if ( mOptions.mbUI )						CREATE_COMPONENT(VuUI,						VuUI::IF()->init());
	if ( mOptions.mbToastManager )				CREATE_COMPONENT(VuToastManager,			VuToastManager::IF()->init());
	if ( mOptions.mbMessageBoxManager )			CREATE_COMPONENT(VuMessageBoxManager,		VuMessageBoxManager::IF()->init());
	if ( mOptions.mbDialogManager )				CREATE_COMPONENT(VuDialogManager,			VuDialogManager::IF()->init());
	if ( mOptions.mbLicenseManager )			CREATE_COMPONENT(VuLicenseManager,			VuLicenseManager::IF()->init());
	if ( mOptions.mbNearbyConnectionManager )	CREATE_COMPONENT(VuNearbyConnectionManager,	VuNearbyConnectionManager::IF()->init());

	// post-init
	for ( SysComponents::iterator iter = mSysComponents.begin(); iter != mSysComponents.end(); iter++ )
		(*iter)->postInit();

	// init global random number seed
	VuRand::global() = VuRand(0);

	VuThread::IF()->setThreadProcessor(0);

	return true;
}

//*****************************************************************************
void VuEngine::release()
{
	// pre-release
	for ( SysComponents::reverse_iterator iter = mSysComponents.rbegin(); iter != mSysComponents.rend(); iter++ )
		(*iter)->preRelease();

	// release interfaces (in reverse order)
	for ( SysComponents::reverse_iterator iter = mSysComponents.rbegin(); iter != mSysComponents.rend(); iter++ )
	{
		(*iter)->release();
		(*iter)->resetIF();
		delete (*iter);
	}

	mSysComponents.clear();
}

//*****************************************************************************
bool VuEngine::tick()
{
	VuConfigManager::IF()->tick();
	VuTickManager::IF()->tick();

	return !VuSys::IF()->hasErrors();
}

//*****************************************************************************
void VuEngine::draw()
{
	VuDrawManager::IF()->draw();
	VuGfxSort::IF()->draw();
}