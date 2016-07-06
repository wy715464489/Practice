//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 ProfileManager class
// 
//*****************************************************************************

#include <np.h>
#include <save_data.h>

#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/Containers/VuArray.h"

class VuPs4ProfileManager : public VuProfileManager
{
public:
	static VuPs4ProfileManager *IF() { return static_cast<VuPs4ProfileManager *>(VuProfileManager::IF()); }

	SceUserServiceUserId	getCurUser() { return mCurUser; }

protected:
	virtual bool			init(const std::string &gameName);

private:
	void					tickInput(float fdt);

	virtual void			getPath(std::string &path) {}
	virtual void			loadInternal();
	virtual void			saveInternal();
	
	void					saveThreadProc();
	static void				saveThreadProc(void *pParam) { static_cast<VuPs4ProfileManager *>(pParam)->saveThreadProc(); }

	VuJsonContainer			mCachedSaveData;
	VuJsonContainer			mSecondaryCachedSaveData;

	SceUserServiceUserId	mCurUser;
	VUHANDLE				mhSaveThread;
};

