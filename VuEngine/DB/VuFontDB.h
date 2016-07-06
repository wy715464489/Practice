//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Font DB class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Gfx/Font/VuFontDrawParams.h"

class VuEngine;
class VuFontAsset;
class VuFont;
class VuDBAsset;


class VuFontDB : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuFontDB)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();
	virtual void release();

public:
	class VuEntry
	{
	public:
		VuEntry() : mpFontAsset(VUNULL) {}

		VuFont					*font() const;
		const VuFontDrawParams	&params() const { return mParams; }

		VuFontAsset			*mpFontAsset;
		VuFontDrawParams	mParams;
	};

	const VuEntry		&getFont(const std::string &name) const { return getFont(name.c_str()); }
	const VuEntry		&getFont(const char *name) const;

	int					getFontCount();
	const char			*getFontName(int index);

	void				reload();

private:
	typedef std::hash_map<VUUINT32, VuEntry> Entries;

	VuDBAsset	*mpFontDBAsset;
	Entries		mEntries;
	VuEntry		mDefaultEntry;
};
