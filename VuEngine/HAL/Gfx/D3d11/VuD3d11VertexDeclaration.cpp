//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the shader interface class.
//
//*****************************************************************************

#include "VuD3d11VertexDeclaration.h"
#include "VuD3d11ShaderProgram.h"
#include "VuD3d11Gfx.h"


// static variables

static struct D3d11VertexDeclarationData
{
	typedef std::map<VUUINT32, VuD3d11VertexDeclaration *> VertexDeclarations;
	VertexDeclarations mVertexDeclarations;
} sD3d11VertexDeclarationData;



//*****************************************************************************
VuD3d11VertexDeclaration::VuD3d11VertexDeclaration(const VuVertexDeclarationParams &params):
	VuVertexDeclaration(params),
	mpD3dInputLayout(VUNULL),
	mpVertexShader(VUNULL)
{
}

//*****************************************************************************
VuD3d11VertexDeclaration::~VuD3d11VertexDeclaration()
{
	mpVertexShader->removeRef();
	D3DRELEASE(mpD3dInputLayout);

	sD3d11VertexDeclarationData.mVertexDeclarations.erase(mHash);
}

//*****************************************************************************
VuD3d11VertexDeclaration *VuD3d11VertexDeclaration::create(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	VuD3d11ShaderProgram *pD3d11ShaderProgram = static_cast<VuD3d11ShaderProgram *>(pShaderProgram);
	VuD3d11Shader *pD3d11VertexShader = pD3d11ShaderProgram->mapShaders[VuShaderProgram::VERTEX_SHADER];

	VUUINT32 hash = params.calcHash();
	hash = VuHash::fnv32(&pShaderProgram, sizeof(pShaderProgram), hash);

	// check if we already have the declaration loaded
	D3d11VertexDeclarationData::VertexDeclarations::const_iterator iter = sD3d11VertexDeclarationData.mVertexDeclarations.find(hash);
	if ( iter != sD3d11VertexDeclarationData.mVertexDeclarations.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create vertex declaration entry
	VuD3d11VertexDeclaration *pVertexDeclaration = new VuD3d11VertexDeclaration(params);
	pVertexDeclaration->mHash = hash;
	pVertexDeclaration->mpVertexShader = pD3d11VertexShader;
	pVertexDeclaration->mpVertexShader->addRef();

	// build d3d vertex element array
	D3D11_INPUT_ELEMENT_DESC *pD3dElements = VuD3d11VertexDeclaration::convert(params.mElements);

	// create input layout
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateInputLayout(pD3dElements, (UINT)params.mElements.size(), &pD3d11VertexShader->mByteCode[0], pD3d11VertexShader->mByteCode.size(), &pVertexDeclaration->mpD3dInputLayout));

	// clean up
	delete[] pD3dElements;

	// add entry
	sD3d11VertexDeclarationData.mVertexDeclarations[hash] = pVertexDeclaration;

	return pVertexDeclaration;
}

//*****************************************************************************
D3D11_INPUT_ELEMENT_DESC *VuD3d11VertexDeclaration::convert(const VuVertexDeclarationElements &elements)
{
	int nElements = (int)elements.size();

	D3D11_INPUT_ELEMENT_DESC *pD3dIED = new D3D11_INPUT_ELEMENT_DESC[nElements];

	for ( int i = 0; i < nElements; i++ )
	{
		pD3dIED[i].SemanticName = VuD3d11GfxTypes::convert(elements[i].mUsage);
		pD3dIED[i].SemanticIndex = elements[i].mUsageIndex;
		pD3dIED[i].Format = VuD3d11GfxTypes::convert(elements[i].mType);
		pD3dIED[i].InputSlot = elements[i].mStream;
		pD3dIED[i].AlignedByteOffset  = elements[i].mOffset;
		pD3dIED[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		pD3dIED[i].InstanceDataStepRate = 0;
	}

	return pD3dIED;
}
