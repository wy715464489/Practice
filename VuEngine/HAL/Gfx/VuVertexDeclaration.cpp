//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VertexDeclaration interface class.
// 
//*****************************************************************************

#include "VuVertexDeclaration.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuHash.h"


//*****************************************************************************
void VuVertexDeclarationElements::load(VuBinaryDataReader &reader)
{
	int elementCount;
	reader.readValue(elementCount);
	resize(elementCount);
	reader.readData(&(*this)[0], elementCount*sizeof(VuVertexDeclarationElement));
}

//*****************************************************************************
void VuVertexDeclarationElements::save(VuBinaryDataWriter &writer) const
{
	int elementCount = (int)size();
	writer.writeValue(elementCount);

	for ( const auto &element : *this )
	{
		writer.writeValue(element.mStream);
		writer.writeValue(element.mOffset);
		writer.writeValue(element.mType);
		writer.writeValue(element.mUsage);
		writer.writeValue(element.mUsageIndex);
		writer.writeValue(element.mPad0);
		writer.writeValue(element.mPad1);
		writer.writeValue(element.mPad2);
	}
}

//*****************************************************************************
void VuVertexDeclarationElements::load(const VuJsonContainer &data)
{
	VUUINT16 offset = 0;

	for ( int i = 0; i < data.size(); i++ )
	{
		const std::string &element = data[i].asString();

		if ( element == "Position" )
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
			offset += 12;
		}

		if ( element == "Normal")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_NORMAL, 0));
			offset += 12;
		}

		if ( element == "Tangent")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_TANGENT, 0));
			offset += 12;
		}

		if ( element == "TexCoord0")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT2, VUGFX_DECL_USAGE_TEXCOORD, 0));
			offset += 8;
		}

		if ( element == "TexCoord1")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT2, VUGFX_DECL_USAGE_TEXCOORD, 1));
			offset += 8;
		}

		if ( element == "TexCoord2")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT2, VUGFX_DECL_USAGE_TEXCOORD, 2));
			offset += 8;
		}

		if ( element == "Color0")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 0));
			offset += 4;
		}

		if ( element == "Color1")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 1));
			offset += 4;
		}

		if ( element == "Color2")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 2));
			offset += 4;
		}

		if ( element == "Weights")
		{
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_BLENDWEIGHT, 0));
			offset += 12;
			push_back(VuVertexDeclarationElement(0, offset, VUGFX_DECL_TYPE_UBYTE4, VUGFX_DECL_USAGE_BLENDINDICES, 0));
			offset += 4;
		}
	}
}

//*****************************************************************************
int VuVertexDeclarationElements::calcVertexSize(int stream) const
{
	int vertexSize = 0;

	for ( const auto &element : *this )
		if ( element.mStream == stream )
			vertexSize += element.size();

	return vertexSize;
}

//*****************************************************************************
int VuVertexDeclarationElement::size() const
{
	static int sSizeLookup[] =
	{
		4,	// VUGFX_DECL_TYPE_FLOAT1,		
		8,	// VUGFX_DECL_TYPE_FLOAT2,		
		12,	// VUGFX_DECL_TYPE_FLOAT3,		
		16,	// VUGFX_DECL_TYPE_FLOAT4,		
		4,	// VUGFX_DECL_TYPE_BYTE4,		
		4,	// VUGFX_DECL_TYPE_BYTE4N,	
		4,	// VUGFX_DECL_TYPE_UBYTE4,		
		4,	// VUGFX_DECL_TYPE_UBYTE4N,	
		4,	// VUGFX_DECL_TYPE_SHORT2N,	
		8,	// VUGFX_DECL_TYPE_SHORT4N,	
		4,	// VUGFX_DECL_TYPE_FLOAT16_2,	
		8,	// VUGFX_DECL_TYPE_FLOAT16_4,	
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sSizeLookup)/sizeof(sSizeLookup[0]) == VUGFX_DECL_TYPE_COUNT);

	return sSizeLookup[mType];
}

//*****************************************************************************
VUUINT32 VuVertexDeclarationParams::calcHash() const
{
	VUASSERT(mElements.size() && mStreams.size(), "VuVertexDeclarationParams::calcHash() called on incomplete params");

	VUUINT32 hash = VU_FNV32_INIT;

	hash = VuHash::fnv32(&mElements[0], (int)mElements.size()*sizeof(mElements[0]), hash);
	hash = VuHash::fnv32(&mStreams[0], (int)mStreams.size()*sizeof(mStreams[0]), hash);

	return hash;
}
