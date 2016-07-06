//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the shader interface class.
//
//*****************************************************************************

#include "VuOglesVertexDeclaration.h"
#include "VuOglesShaderProgram.h"


// static variables

static struct OglesVertexDeclarationData
{
	typedef std::map<VUUINT32, VuOglesVertexDeclaration *> VertexDeclarations;
	VertexDeclarations mVertexDeclarations;
} sOglesVertexDeclarationData;


/*
	0 - aPosition, aPosition0
	1 - aNormal
	2 - aColor, aColor0
	3 - aTangent
	4 - aBlendWeight
	5 - aBlendIndices
	6 - aTexCoord, aTexCoord0
	7 - aSceneColor, aColor1, aTexCoord1
*/

static int sOglesUsages[VUGFX_DECL_USAGE_COUNT][7] =
{
	{  0, -1, -1, -1, -1, -1, -1 }, // VUGFX_DECL_USAGE_POSITION,
	{  1, -1, -1, -1, -1, -1, -1 }, // VUGFX_DECL_USAGE_NORMAL,
	{  2,  7, -1, -1, -1, -1, -1 }, // VUGFX_DECL_USAGE_COLOR,
	{  3, -1, -1, -1, -1, -1, -1 }, // VUGFX_DECL_USAGE_TANGENT,
	{  4, -1, -1, -1, -1, -1, -1 }, // VUGFX_DECL_USAGE_BLENDWEIGHT,
	{  5, -1, -1, -1, -1, -1, -1 }, // VUGFX_DECL_USAGE_BLENDINDICES,
	{  6,  7, -1, -1, -1, -1, -1 }, // VUGFX_DECL_USAGE_TEXCOORD,
};

struct OglesType
{
	int		mSize;
	GLenum	mType;
	bool	mNormalized;
};
static OglesType sOglesTypes[] =
{
	{ 1, GL_FLOAT,          GL_FALSE },	// VUGFX_DECL_TYPE_FLOAT1,
	{ 2, GL_FLOAT,          GL_FALSE },	// VUGFX_DECL_TYPE_FLOAT2,
	{ 3, GL_FLOAT,          GL_FALSE },	// VUGFX_DECL_TYPE_FLOAT3,
	{ 4, GL_FLOAT,          GL_FALSE },	// VUGFX_DECL_TYPE_FLOAT4,
	{ 4, GL_BYTE,           GL_FALSE },	// VUGFX_DECL_TYPE_BYTE4,
	{ 4, GL_BYTE,           GL_TRUE },	// VUGFX_DECL_TYPE_BYTE4N,
	{ 4, GL_UNSIGNED_BYTE,  GL_FALSE },	// VUGFX_DECL_TYPE_UBYTE4,
	{ 4, GL_UNSIGNED_BYTE,  GL_TRUE },	// VUGFX_DECL_TYPE_UBYTE4N,
	{ 2, GL_SHORT,          GL_TRUE },	// VUGFX_DECL_TYPE_SHORT2N,
	{ 4, GL_SHORT,          GL_TRUE },	// VUGFX_DECL_TYPE_SHORT4N,
	{ 0, 0,                 0       },	// VUGFX_DECL_TYPE_FLOAT16_2,
	{ 0, 0,                 0       },	// VUGFX_DECL_TYPE_FLOAT16_4,
};
VU_COMPILE_TIME_ASSERT(sizeof(sOglesTypes)/sizeof(sOglesTypes[0]) == VUGFX_DECL_TYPE_COUNT);


//*****************************************************************************
VuOglesVertexDeclaration::VuOglesVertexDeclaration(const VuVertexDeclarationParams &params):
	VuVertexDeclaration(params),
	mEnabledAttribCount(0),
	mDisabledAttribCount(0)
{
	memset(mEnabledAttribs, 0, sizeof(mEnabledAttribs));
	memset(mDisabledAttribs, 0, sizeof(mDisabledAttribs));
}

//*****************************************************************************
VuOglesVertexDeclaration::~VuOglesVertexDeclaration()
{
	// erase vertex declaration entry
	sOglesVertexDeclarationData.mVertexDeclarations.erase(mHash);
}

//*****************************************************************************
void VuOglesVertexDeclaration::build()
{
	// determine enabled attributes
	for ( const auto &element : mParams.mElements )
	{
		VuOglesVertexDeclaration::VuAttrib &attrib = mEnabledAttribs[mEnabledAttribCount++];

		// translate
		attrib.mUsage = sOglesUsages[element.mUsage][element.mUsageIndex];
		attrib.mSize = sOglesTypes[element.mType].mSize;
		attrib.mType = sOglesTypes[element.mType].mType;
		attrib.mNormalized = sOglesTypes[element.mType].mNormalized;
		attrib.mOffset = element.mOffset;

		VUASSERT(attrib.mUsage >= 0, "Unsupported vertex element usage");
		VUASSERT(attrib.mSize > 0, "Unsupported vertex element type");
	}

	// determine disabled attributes
	for ( int i = 0; i < MAX_ATTRIBUTES; i++ )
	{
		bool bDisabled = true;
		for ( int j = 0; j < mEnabledAttribCount; j++ )
			if ( mEnabledAttribs[j].mUsage == i )
				bDisabled = false;

		if ( bDisabled )
			mDisabledAttribs[mDisabledAttribCount++] = i;
	}
}

//*****************************************************************************
VuOglesVertexDeclaration *VuOglesVertexDeclaration::create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	VuOglesShaderProgram *pOglesShaderProgram = static_cast<VuOglesShaderProgram *>(pShaderProgram);

	// remove elements that are not in shader
	VuVertexDeclarationParams optParams;
	optParams.mStreams = params.mStreams;
	for ( const auto &element : params.mElements )
	{
		int usage = sOglesUsages[element.mUsage][element.mUsageIndex];

		VUASSERT(usage >= 0, "Unsupported vertex element usage");

		// does attribute exist in shader?
		for ( int j = 0; j < pOglesShaderProgram->mAttributeCount; j++ )
		{
			if ( pOglesShaderProgram->maAttributes[j] == usage )
			{
				optParams.mElements.push_back(element);
				break;
			}
		}
	}

	VUUINT32 hash = optParams.calcHash();
	hash = VuHash::fnv32(&pShaderProgram, sizeof(pShaderProgram), hash);

	// check if we already have the declaration loaded
	OglesVertexDeclarationData::VertexDeclarations::const_iterator iter = sOglesVertexDeclarationData.mVertexDeclarations.find(hash);
	if ( iter != sOglesVertexDeclarationData.mVertexDeclarations.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create vertex declaration entry
	VuOglesVertexDeclaration *pVertexDeclaration = new VuOglesVertexDeclaration(optParams);
	pVertexDeclaration->mHash = hash;

	// build ogl declaration
	pVertexDeclaration->build();

	// add entry
	sOglesVertexDeclarationData.mVertexDeclarations[hash] = pVertexDeclaration;

	return pVertexDeclaration;
}
