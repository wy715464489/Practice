//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VertexDeclaration interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuGfxTypes.h"

class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;
class VuShaderProgram;

struct VuVertexDeclarationElement
{
	VuVertexDeclarationElement()
		: mStream(0), mOffset(0), mType(VUGFX_DECL_TYPE_INVALID), mUsage(VUGFX_DECL_USAGE_INVALID), mUsageIndex(0), mPad0(0), mPad1(0), mPad2(0)
		{}

	VuVertexDeclarationElement(VUUINT16 stream, VUUINT16 offset, eGfxDeclType type, eGfxDeclUsage usage, VUUINT8 usageIndex)
		: mStream(stream), mOffset(offset), mType(type), mUsage(usage), mUsageIndex(usageIndex), mPad0(0), mPad1(0), mPad2(0)
		{}

	bool operator == (VuVertexDeclarationElement other) const
	{
		return mStream		== other.mStream
			&& mOffset		== other.mOffset
			&& mType		== other.mType
			&& mUsage		== other.mUsage
			&& mUsageIndex	== other.mUsageIndex;
	}

	int size() const;

	VUUINT16		mStream;		// stream number
	VUUINT16		mOffset;		// offset from the beginning of the vertex data
	eGfxDeclType	mType;			// the data type
	eGfxDeclUsage	mUsage;			// defines what the data will be used for
	VUUINT8			mUsageIndex;	// allows the user to specify multiple usage types
	VUUINT8			mPad0;
	VUUINT8			mPad1;
	VUUINT8			mPad2;
};

class VuVertexDeclarationElements : public std::vector<VuVertexDeclarationElement>
{
public:
	void	load(VuBinaryDataReader &reader);
	void	save(VuBinaryDataWriter &writer) const;
	void	load(const VuJsonContainer &data);
	int		calcVertexSize(int stream) const;
};

struct VuVertexDeclarationStream
{
	VuVertexDeclarationStream() : mStride(0) {}
	VuVertexDeclarationStream(int stride) : mStride(stride) {}

	VUUINT32	mStride;	// The distance, in bytes, between the attribute data of two vertices in the buffer (multiple of 4 bytes).
};

class VuVertexDeclarationStreams : public std::vector<VuVertexDeclarationStream>
{
};


class VuVertexDeclarationParams
{
public:
	VUUINT32	calcHash() const;

	VuVertexDeclarationElements	mElements;
	VuVertexDeclarationStreams	mStreams;
};

class VuVertexDeclaration : public VuRefObj
{
public:
	VuVertexDeclaration(const VuVertexDeclarationParams &params) : mParams(params) {}

	VuVertexDeclarationParams	mParams;
};
