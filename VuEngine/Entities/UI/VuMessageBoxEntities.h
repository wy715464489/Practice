//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MessageBox entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Managers/VuMessageBoxManager.h"

class VuScriptComponent;
class VuDBEntryProperty;
class VuScriptPlug;


class VuMessageBoxEntity : public VuEntity, VuMessageBox::Callback
{
	DECLARE_RTTI

public:
	VuMessageBoxEntity();
	~VuMessageBoxEntity();

protected:
	virtual void		onOpened() {}
	virtual void		onClosed() {}

	// scripting
	VuRetVal			Create(const VuParams &params);

	// VuMessageBox::Callback
	virtual void		onMessageBoxClosed(VuMessageBox *pMessageBox);

	void				modified();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	VuMessageBoxParams	mMessageBoxParams;

	VuMessageBox		*mpMessageBox;
	VuDBEntryProperty	*mpTypeProperty;

	typedef std::vector<VuScriptPlug *> ResultPlugs;
	ResultPlugs			mResultPlugs;
};
