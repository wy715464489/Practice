//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the vertex declaration interface class.
//
//*****************************************************************************

#include "VuMetalVertexDeclaration.h"
#include "VuMetalShaderProgram.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Math/VuMath.h"


// static variables

static struct MetalVertexDeclarationData
{
	typedef std::map<VUUINT32, VuMetalVertexDeclaration *> VertexDeclarations;
	VertexDeclarations mVertexDeclarations;
	
	struct VuAttributeMapElement
	{
		VuAttributeMapElement() {}
		VuAttributeMapElement(eGfxDeclUsage usage, int usageIndex) : mUsage(usage), mUsageIndex(usageIndex) {}
		eGfxDeclUsage	mUsage;
		int				mUsageIndex;
	};
	typedef std::map<VUUINT32, VuAttributeMapElement> AttributeMap;
	AttributeMap mAttributeMap;
	
	void buildMap()
	{
		mAttributeMap[VuHash::fnv32String("mPosition")] = VuAttributeMapElement(VUGFX_DECL_USAGE_POSITION, 0);
		mAttributeMap[VuHash::fnv32String("mNormal")] = VuAttributeMapElement(VUGFX_DECL_USAGE_NORMAL, 0);
		mAttributeMap[VuHash::fnv32String("mTangent")] = VuAttributeMapElement(VUGFX_DECL_USAGE_TANGENT, 0);
		
		mAttributeMap[VuHash::fnv32String("mColor")] = VuAttributeMapElement(VUGFX_DECL_USAGE_COLOR, 0);
		mAttributeMap[VuHash::fnv32String("mColor0")] = VuAttributeMapElement(VUGFX_DECL_USAGE_COLOR, 0);
		mAttributeMap[VuHash::fnv32String("mColor1")] = VuAttributeMapElement(VUGFX_DECL_USAGE_COLOR, 1);
		mAttributeMap[VuHash::fnv32String("mSceneColor")] = VuAttributeMapElement(VUGFX_DECL_USAGE_COLOR, 1);
		
		mAttributeMap[VuHash::fnv32String("mBlendWeights")] = VuAttributeMapElement(VUGFX_DECL_USAGE_BLENDWEIGHT, 0);
		mAttributeMap[VuHash::fnv32String("mBlendIndices")] = VuAttributeMapElement(VUGFX_DECL_USAGE_BLENDINDICES, 0);
		
		mAttributeMap[VuHash::fnv32String("mTexCoord")] = VuAttributeMapElement(VUGFX_DECL_USAGE_TEXCOORD, 0);
		mAttributeMap[VuHash::fnv32String("mTexCoord0")] = VuAttributeMapElement(VUGFX_DECL_USAGE_TEXCOORD, 0);
		mAttributeMap[VuHash::fnv32String("mTexCoord1")] = VuAttributeMapElement(VUGFX_DECL_USAGE_TEXCOORD, 1);
	}
} sMetalVertexDeclarationData;

static MTLVertexFormat sMetalTypes[] =
{
	MTLVertexFormatFloat,            // VUGFX_DECL_TYPE_FLOAT1,
	MTLVertexFormatFloat2,           // VUGFX_DECL_TYPE_FLOAT2,
	MTLVertexFormatFloat3,           // VUGFX_DECL_TYPE_FLOAT3,
	MTLVertexFormatFloat4,           // VUGFX_DECL_TYPE_FLOAT4,
	MTLVertexFormatChar4,            // VUGFX_DECL_TYPE_BYTE4,
	MTLVertexFormatChar4Normalized,  // VUGFX_DECL_TYPE_BYTE4N,
	MTLVertexFormatUChar4,           // VUGFX_DECL_TYPE_UBYTE4,
	MTLVertexFormatUChar4Normalized, // VUGFX_DECL_TYPE_UBYTE4N,
	MTLVertexFormatShort2Normalized, // VUGFX_DECL_TYPE_SHORT2N,
	MTLVertexFormatShort4Normalized, // VUGFX_DECL_TYPE_SHORT4N,
	MTLVertexFormatHalf2,            // VUGFX_DECL_TYPE_FLOAT16_2,
	MTLVertexFormatHalf4,            // VUGFX_DECL_TYPE_FLOAT16_4,
};
VU_COMPILE_TIME_ASSERT(sizeof(sMetalTypes)/sizeof(sMetalTypes[0]) == VUGFX_DECL_TYPE_COUNT);


//*****************************************************************************
VuMetalVertexDeclaration::VuMetalVertexDeclaration(const VuVertexDeclarationParams &params):
	VuVertexDeclaration(params)
{
}

//*****************************************************************************
VuMetalVertexDeclaration::~VuMetalVertexDeclaration()
{
	sMetalVertexDeclarationData.mVertexDeclarations.erase(mHash);
}

//*****************************************************************************
VuMetalVertexDeclaration *VuMetalVertexDeclaration::create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	if ( sMetalVertexDeclarationData.mAttributeMap.empty() )
		sMetalVertexDeclarationData.buildMap();
	
		// remove elements that are not in shader
	VuVertexDeclarationParams optParams;
	optParams.mStreams = params.mStreams;

	std::vector<int> attribIndexLookup;
	VuMetalShaderProgram *pMetalShaderProgram = static_cast<VuMetalShaderProgram *>(pShaderProgram);
	id<MTLFunction> vertexFunction = pMetalShaderProgram->mShaders[VuShaderProgram::VERTEX_SHADER].mMtlFunction;
	for ( MTLVertexAttribute *va in [vertexFunction vertexAttributes] )
	{
		if ( va.active )
		{
			VUUINT32 nameHash = VuHash::fnv32String([va.name UTF8String]);
			MetalVertexDeclarationData::AttributeMap::const_iterator iter = sMetalVertexDeclarationData.mAttributeMap.find(nameHash);
			VUASSERT(iter != sMetalVertexDeclarationData.mAttributeMap.end(), "Unsupported attribute name!");

			// find matching element
			bool found = false;
			for ( const auto &element : params.mElements )
			{
				if ( element.mUsage == iter->second.mUsage && element.mUsageIndex == iter->second.mUsageIndex )
				{
					optParams.mElements.push_back(element);
					attribIndexLookup.push_back(va.attributeIndex);
					found = true;
					break;
				}
			}
			
			VUASSERT(found, "Shader vertex attribute not found in vertex declaration!");
		}
	}
	
	VUUINT32 hash = optParams.calcHash();
	hash = VuHash::fnv32(&pShaderProgram, sizeof(pShaderProgram), hash);
	
	// check if we already have the declaration loaded
	MetalVertexDeclarationData::VertexDeclarations::const_iterator iter = sMetalVertexDeclarationData.mVertexDeclarations.find(hash);
	if ( iter != sMetalVertexDeclarationData.mVertexDeclarations.end() )
	{
		iter->second->addRef();
		return iter->second;
	}
	
	VuMetalVertexDeclaration *pVertexDeclaration = new VuMetalVertexDeclaration(optParams);
	pVertexDeclaration->mHash = hash;
	
	pVertexDeclaration->mpMTLVertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
	
	int numActiveStreams = 1;
	
	int iElement = 0;
	for ( const auto &element : optParams.mElements )
	{
		int attribIndex = attribIndexLookup[iElement];
		
		MTLVertexAttributeDescriptor *pMTLElement = pVertexDeclaration->mpMTLVertexDescriptor.attributes[attribIndex];
		
		pMTLElement.bufferIndex = element.mStream;
		pMTLElement.offset = element.mOffset;
		pMTLElement.format = sMetalTypes[element.mType];
		
		numActiveStreams = VuMax(numActiveStreams, element.mStream + 1);
		
		iElement++;
	}
	
	for ( int iStream = 0; iStream < numActiveStreams; iStream++ )
	{
		MTLVertexBufferLayoutDescriptor *pMTLStream = pVertexDeclaration->mpMTLVertexDescriptor.layouts[iStream];
		pMTLStream.stride = params.mStreams[iStream].mStride;
	}
	
	// add entry
	sMetalVertexDeclarationData.mVertexDeclarations[hash] = pVertexDeclaration;
	
	return pVertexDeclaration;
}
