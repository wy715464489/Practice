//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the shader interface class.
//
//*****************************************************************************

#include <gnmx.h>
#include "VuPs4VertexDeclaration.h"
#include "VuPs4ShaderProgram.h"
#include "VuPs4Gfx.h"
#include "VuPs4GfxTypes.h"
#include "VuEngine/Util/VuHash.h"


// static variables

static struct Ps4VertexDeclarationData
{
	typedef std::map<VUUINT32, VuPs4VertexDeclaration *> VertexDeclarations;
	VertexDeclarations mVertexDeclarations;

	typedef std::map<VUUINT32, eGfxDeclUsage> SemanticMap;
	SemanticMap mSemanticMap;
	
	void buildMap()
	{
		mSemanticMap[VuHash::fnv32String("POSITION")] = VUGFX_DECL_USAGE_POSITION;
		mSemanticMap[VuHash::fnv32String("NORMAL")] = VUGFX_DECL_USAGE_NORMAL;
		mSemanticMap[VuHash::fnv32String("TANGENT")] = VUGFX_DECL_USAGE_TANGENT;
		mSemanticMap[VuHash::fnv32String("COLOR")] = VUGFX_DECL_USAGE_COLOR;
		mSemanticMap[VuHash::fnv32String("BLENDWEIGHT")] = VUGFX_DECL_USAGE_BLENDWEIGHT;
		mSemanticMap[VuHash::fnv32String("BLENDINDICES")] = VUGFX_DECL_USAGE_BLENDINDICES;
		mSemanticMap[VuHash::fnv32String("TEXCOORD")] = VUGFX_DECL_USAGE_TEXCOORD;
	}
} sPs4VertexDeclarationData;


//*****************************************************************************
VuPs4VertexDeclaration::VuPs4VertexDeclaration(const VuVertexDeclarationParams &params):
	VuVertexDeclaration(params),
	mpFetchShader(VUNULL),
	mStreamCount(0)
{
}

//*****************************************************************************
VuPs4VertexDeclaration::~VuPs4VertexDeclaration()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpFetchShader);

	// erase VD entry
	Ps4VertexDeclarationData::VertexDeclarations::iterator iter = sPs4VertexDeclarationData.mVertexDeclarations.find(mHash);
	VUASSERT(iter != sPs4VertexDeclarationData.mVertexDeclarations.end(), "VuPs4VertexDeclaration::~VuPs4VertexDeclaration() entry not found");
	sPs4VertexDeclarationData.mVertexDeclarations.erase(iter);
}

//*****************************************************************************
Gnm::DataFormat VuPs4VertexDeclaration::convert(eGfxDeclType type)
{
	static Gnm::DataFormat sLookup[] =
	{
		Gnm::kDataFormatR32Float,          // VUGFX_DECL_TYPE_FLOAT1,
		Gnm::kDataFormatR32G32Float,       // VUGFX_DECL_TYPE_FLOAT2,
		Gnm::kDataFormatR32G32B32Float,    // VUGFX_DECL_TYPE_FLOAT3,
		Gnm::kDataFormatR32G32B32A32Float, // VUGFX_DECL_TYPE_FLOAT4,
		Gnm::kDataFormatR8G8B8A8Sint,      // VUGFX_DECL_TYPE_BYTE4,
		Gnm::kDataFormatR8G8B8A8Snorm,     // VUGFX_DECL_TYPE_BYTE4N,
		Gnm::kDataFormatR8G8B8A8Uint,      // VUGFX_DECL_TYPE_UBYTE4,
		Gnm::kDataFormatR8G8B8A8Unorm,     // VUGFX_DECL_TYPE_UBYTE4N,
		Gnm::kDataFormatR16G16Snorm,       // VUGFX_DECL_TYPE_SHORT2N,
		Gnm::kDataFormatR16G16B16A16Snorm, // VUGFX_DECL_TYPE_SHORT4N,
		Gnm::kDataFormatR16G16Float,       // VUGFX_DECL_TYPE_FLOAT16_2,
		Gnm::kDataFormatR16G16B16A16Float, // VUGFX_DECL_TYPE_FLOAT16_4,
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup) / sizeof(sLookup[0]) == VUGFX_DECL_TYPE_COUNT);

	return sLookup[type];
}

//*****************************************************************************
VuPs4VertexDeclaration *VuPs4VertexDeclaration::create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	if ( sPs4VertexDeclarationData.mSemanticMap.empty() )
		sPs4VertexDeclarationData.buildMap();

	VuPs4ShaderProgram *pPs4ShaderProgram = static_cast<VuPs4ShaderProgram *>(pShaderProgram);
	VuPs4VertexShader *pPs4VertexShader = static_cast<VuPs4VertexShader *>(pPs4ShaderProgram->mapShaders[VuShaderProgram::VERTEX_SHADER]);

	VUUINT32 hash = params.calcHash();
	hash = VuHash::fnv32(&pPs4ShaderProgram->mapShaders[VuShaderProgram::VERTEX_SHADER], sizeof(VuPs4Shader *), hash);
	
	// check if we already have the declaration loaded
	Ps4VertexDeclarationData::VertexDeclarations::const_iterator iter = sPs4VertexDeclarationData.mVertexDeclarations.find(hash);
	if ( iter != sPs4VertexDeclarationData.mVertexDeclarations.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// build remap table
	std::vector<int> remapTable(0);
	remapTable.resize(params.mElements.size());
	for ( int i = 0; i < remapTable.size(); i++ )
		remapTable[i] = -1;

	const Shader::Binary::Program &program = pPs4VertexShader->mPs4BinaryProgram;
	for ( int iAttrib = 0; iAttrib < program.m_numInputAttributes; iAttrib++ )
	{
		const Shader::Binary::Attribute &attrib = program.m_inputAttributes[iAttrib];

		VUASSERT(attrib.m_psslSemantic == Shader::Binary::kSemanticUserDefined, "Bad Ps4 semantic assumption");

		const char *semanticName = attrib.getSemanticName();
		VUUINT32 hashedSemanticName = VuHash::fnv32String(semanticName);
		Ps4VertexDeclarationData::SemanticMap::const_iterator iter = sPs4VertexDeclarationData.mSemanticMap.find(hashedSemanticName);
		VUASSERT(iter != sPs4VertexDeclarationData.mSemanticMap.end(), "Unsupported semantic name!");

		// find matching element
		int elementIndex = 0;
		for ( const auto &element : params.mElements )
		{
			if ( element.mUsage == iter->second && element.mUsageIndex == attrib.m_semanticIndex )
				break;
			elementIndex++;
		}

		// make sure all shader semantics are found in declaration elements
		if ( elementIndex == params.mElements.size() )
		{
			VUPRINTF("Shader vertex semantic not found in vertex declaration!");
			return VUNULL;
		}

		remapTable[elementIndex] = attrib.m_resourceIndex;
	}

	// create new vertex declaration
	VuPs4VertexDeclaration *pVertexDeclaration = new VuPs4VertexDeclaration(params);
	pVertexDeclaration->mHash = hash;
	
	// create fetch shader
	pVertexDeclaration->mpFetchShader = VuPs4Gfx::IF()->garlicAllocator().allocate(Gnmx::computeVsFetchShaderSize(pPs4VertexShader->mpPs4VertexShader), Gnm::kAlignmentOfFetchShaderInBytes);
	Gnmx::generateVsFetchShader(pVertexDeclaration->mpFetchShader, &pVertexDeclaration->mShaderModifier, pPs4VertexShader->mpPs4VertexShader, 0, &remapTable[0], remapTable.size());

	// calc stream info
	for ( const auto &iter : params.mElements )
	{
		VuPs4VertexDeclaration::Stream &stream = pVertexDeclaration->mStreams[iter.mStream];
		VuPs4VertexDeclaration::StreamElement &se = stream.mElements[stream.mElementCount];

		se.mOffset = stream.mStride;
		se.mDataFormat = VuPs4VertexDeclaration::convert(iter.mType);

		stream.mElementCount += 1;
		stream.mStride += iter.size();

		pVertexDeclaration->mStreamCount = VuMax(pVertexDeclaration->mStreamCount, iter.mStream + 1);
	}

	// add entry
	sPs4VertexDeclarationData.mVertexDeclarations[hash] = pVertexDeclaration;
	
	return pVertexDeclaration;
}
