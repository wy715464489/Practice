//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuFont class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"

class VuJsonContainer;
class VuTexture;
class VuTextureAsset;
class VuBinaryDataReader;
class VuBinaryDataWriter;


class VuFont
{
public:
	VuFont();
	~VuFont();

	static bool		bake(const VuJsonContainer &data, VuBinaryDataWriter &writer);
	bool			load(const VuJsonContainer &data);
	bool			load(VuBinaryDataReader &reader);

	inline VUUINT16	codeToIndex(VUUINT32 code) const;


	struct CharEntry
	{
		VUUINT32	mCode;
		float		mSrcL;
		float		mSrcR;
		float		mSrcT;
		float		mSrcB;
		float		mDstL;
		float		mDstR;
		float		mDstT;
		float		mDstB;
		float		mAdvance;
		VUUINT16	mImageIndex;
		bool		mIsImage;
		VUUINT8		mPad[1];

		void		load(const VuJsonContainer &data);
		void		serialize(VuBinaryDataWriter &writer);
	};

	struct ImageEntry
	{
		VuTextureAsset	*mpTextureAsset;
	};

	typedef VuArray<CharEntry> Characters;
	typedef VuArray<ImageEntry> Images;
	typedef std::hash_map<VUUINT32, VUUINT16> LookupTable;

	VuTexture		*mpTexture;
	float			mAscender;
	float			mDescender;
	float			mMaxRadius;	// should only be used internally
	Characters		mCharacters;
	Images			mImages;
	LookupTable		mCharLookupTable;
	VUUINT16		mUnknownCharacter;

private:
	void			buildLookupTables();
};


//*****************************************************************************
inline VUUINT16	VuFont::codeToIndex(VUUINT32 code) const
{
	LookupTable::const_iterator iter = mCharLookupTable.find(code);
	if ( iter != mCharLookupTable.end() )
		return iter->second;
	return mUnknownCharacter;
}
