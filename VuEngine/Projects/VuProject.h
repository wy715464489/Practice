//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Project class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Json/VuJsonContainer.h"

class VuProjectAsset;

class VuProject : public VuRefObj
{
public:
	VuProject() : mpRootEntity(VUNULL) {}
protected:
	~VuProject() { destroy(); }
public:

	bool					create(const std::string &strType, const std::string &strName);
	void					destroy();

	bool					load(const VuProjectAsset *pProjectAsset);
	bool					load(const std::string &strFileName);
	bool					save(const std::string &strFileName) const;
	void					saveEditorData(const std::string &strFileName) const;
	void					bake();
	void					clearBaked();
	void					gameInitialize();
	void					gameRelease();
	void					gameReset();

	VuEntity				*getRootEntity() const											{ return mpRootEntity; }
	VuEntity				*findEntity(const std::string &strLongName) const;

	VuJsonContainer			&getEditorData()												{ return mEditorData; }
	const VuJsonContainer	&getEditorData() const											{ return mEditorData; }

	const std::string		&getName() const;

	bool					load(const VuJsonContainer &data, const std::string &strFileName);
	bool					save(VuJsonContainer &data) const;

private:
	void					cleanSaveDataRecursive(VuJsonContainer &data) const;

	VuEntity				*mpRootEntity;
	VuJsonContainer			mEditorData;
	VuJsonContainer			mAssetData;
};