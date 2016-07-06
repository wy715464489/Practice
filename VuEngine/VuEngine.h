//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Engine library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Util/VuCmdLineArgs.h"

class VuSystemComponent;


class VuEngine
{
public:

	// options used to initialize subsystems
	struct Options
	{
		Options() :

			mSku("Editor"),
			mbEditorMode(false),

			// required systems
			mbGfxSortAsynchronousRendering(false),

			// HALs
			mbNet(false),
			mbFile(false),
			mbGfx(false), mhGfxWindow(VUNULL), mhGfxDevice(VUNULL),
			mbAudio(false), mAudioMaxChannels(128), mstrAudioEncryptionKey(VUNULL), mUseAudioTrack(true), mSurroundSound(false),
			mbKeyboard(false),
			mbGamePad(false),
			mbAccel(false),
			mbTouch(false),

			// dev systems
			mbDev(false), mbDevDrawBuildNumber(false),
			mbDevHostComm(false),
			mbDevConfig(false),
			mbDevConsole(false),
			mbDevMenu(false),
			mbDevStat(false),
			mbDevProfile(false),
			mbDevTimer(false),

			// other systems
			mbAssetFactory(false), mpRegisterGameAssetsFn(VUNULL),
			mbAssetBaker(false),
			mbConfigManager(false),
			mbEntityFactory(false),
			mbEntityRepository(false),
			mbStringDB(false),
			mbFontDB(false),
			mbTelemetryClient(false),
			mbTelemetryServer(false),
			mb3dDrawManager(false),
			mbGfxUtil(false),
			mbGfxComposer(false),
			mbLightManager(false),
			mbWater(false), mbWaterRendererAsync(false),
			mbDynamics(false), mbDynamicsAsync(false), mDynamicsMaxSubSteps(6), mDynamicsFixedTimeStep(1.0f/60.0f),
			mbPfx(false),
			mbPfxManager(false),
			mbTriggerManager(false),
			mbProfileManager(false),
			mbInputManager(false),
			mbFoliageManager(false),
			mbLensWaterManager(false),
			mbTireTrackManager(false),
			mbHttpClient(false),
			mbCloudManager(false),
			mbExplosionManager(false),
			mbUI(false),
			mbToastManager(false),
			mbMessageBoxManager(false),
			mbDialogManager(false),
			mbLicenseManager(false),
			mbNearbyConnectionManager(false)
		{}

		// (optional)		
		std::string	mGameName;

		// sku (defaults to "Editor")
		std::string	mSku;

		// running some sort of editor?
		bool		mbEditorMode;

		// required systems
		std::string	mSysForceLanguage;
		std::string	mSysLogFileName;
		bool		mbGfxSortAsynchronousRendering;

		// optional hardware interfaces
		bool		mbNet;		// standalone
		bool		mbFile;		// standalone
			std::string	mstrFileProjectName;
			std::string	mstrFileRootPath;
		bool		mbGfx;		// standalone
			VUHANDLE	mhGfxWindow;
			VUHANDLE	mhGfxDevice;
			std::string	mGfxApi;
			int			mGfxWindowsDisplayWidth;
			int			mGfxWindowsDisplayHeight;
			float		mGfxWindowsCompositionScaleX;
			float		mGfxWindowsCompositionScaleY;
			size_t		mGfxCpuMemorySize;
			size_t		mGfxGpuMemorySize;
		bool		mbAudio;	// standalone
			int			mAudioMaxChannels;
			const char	*mstrAudioEncryptionKey;
			bool		mUseAudioTrack;
			bool		mSurroundSound;
		bool		mbKeyboard;	// standalone
		bool		mbGamePad;	// standalone
		bool		mbAccel;	// standalone
		bool		mbTouch;	// standalone

		// dev systems
		bool		mbDev;			// requires gfx
			bool		mbDevDrawBuildNumber;
		bool		mbDevHostComm;	// requires net
			std::string	mstrDevHostCommAddress; // address of host machine
		bool		mbDevConfig;	// requires file
		bool		mbDevConsole;	// requires dev, gfx, keyboard
		bool		mbDevMenu;		// requires dev, gfx, keyboard
		bool		mbDevStat;		// requires dev, gfx, keyboard
		bool		mbDevProfile;	// requires dev, gfx, keyboard
		bool		mbDevTimer;		// requires dev, gfx, keyboard

		// other systems
		bool		mbAssetFactory;			// requires file
			void (*mpRegisterGameAssetsFn)();
		bool		mbAssetBaker;			// requires asset factory
		bool		mbConfigManager;		// requires asset factory
			std::string	mDeviceType;
		bool		mbEntityFactory;		// requires asset factory
		bool		mbEntityRepository;		// standalone
		bool		mbStringDB;
		bool		mbFontDB;
		bool		mbTelemetryClient;		// requires net
		bool		mbTelemetryServer;		// requires net
		bool		mb3dDrawManager;		// requires gfx
		bool		mbGfxUtil;
		bool		mbGfxComposer;			// requires gfx
		bool		mbLightManager;			// standalone
		bool		mbWater;				// standalone
			bool		mbWaterRendererAsync;
		bool		mbDynamics;				// standalone
			bool		mbDynamicsAsync;
			int			mDynamicsMaxSubSteps;
			float		mDynamicsFixedTimeStep;
		bool		mbPfx;					// uses gfx
		bool		mbPfxManager;			// uses pfx
		bool		mbTriggerManager;		// standalone
		bool		mbProfileManager;		// standalone
		bool		mbInputManager;			// uses gamepad, keyboard
		bool		mbFoliageManager;		// uses gfx
		bool		mbLensWaterManager;		// uses gfx
		bool		mbTireTrackManager;		// uses gfx
		bool		mbHttpClient;			// requires net
		bool		mbCloudManager;			// requires cloud client
		bool		mbExplosionManager;
		bool		mbUI;
		bool		mbToastManager;
		bool		mbMessageBoxManager;
		bool		mbDialogManager;
		bool		mbLicenseManager;
		bool		mbNearbyConnectionManager;
	};

	//*****************************************************************************
	// P U B L I C   M E T H O D S
	//*****************************************************************************

	// obtaining the interface
	static VuEngine	*IF() { return &mEngine; }

	// init/release
	bool	init(const Options &options);
	void	release();

	// tick/draw/profile
	bool	tick();
	void	draw();

	// access
	const VuCmdLineArgs	&cmdLineArgs() { return mCmdLineArgs; }
	void				setCmdLineArgs(VuCmdLineArgs &args) { mCmdLineArgs = args; }
	const Options		&options() { return mOptions; }
	bool				editorMode() { return mOptions.mbEditorMode; }
	bool				gameMode() { return !mOptions.mGameName.empty(); }

private:

	//*****************************************************************************
	// P R I V A T E   M E M B E R S
	//*****************************************************************************
	typedef std::list<VuSystemComponent *> SysComponents;

	// args
	VuCmdLineArgs		mCmdLineArgs;

	// options
	Options				mOptions;

	// components
	SysComponents		mSysComponents;

	// the engine
	static VuEngine		mEngine;
};
