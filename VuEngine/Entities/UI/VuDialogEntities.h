//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dialog entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Managers/VuDialogManager.h"

class VuScriptComponent;
class VuDBEntryProperty;
class VuScriptPlug;


class VuDialogEntity : public VuEntity, VuDialog::Callback
{
	DECLARE_RTTI

public:
	VuDialogEntity();
	~VuDialogEntity();

protected:
	virtual void		onOpened() {}
	virtual void		onClosed() {}

	// scripting
	VuRetVal			Show(const VuParams &params);

	// VuDialog::Callback
	virtual void		onDialogClosed(VuDialog *pDialog);

	void				modified();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mDialogType;
	bool				mPauseGame;

	VuDialog			*mpDialog;
	VuDBEntryProperty	*mpTypeProperty;

	typedef std::vector<VuScriptPlug *> ResultPlugs;
	ResultPlugs			mResultPlugs;
};
