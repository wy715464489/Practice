//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the shader interface class.
//
//*****************************************************************************

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11shader.h>
	#include <d3dcompiler.h>
#endif

#include "VuD3d11ShaderProgram.h"
#include "VuD3d11Gfx.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


// static variables
static struct D3d11ShaderData
{
	typedef std::list<VuD3d11ShaderProgram *> ShaderPrograms;
	typedef std::list<VuD3d11Shader *> Shaders;
	ShaderPrograms	mShaderPrograms;
	Shaders			mShaders;
} sD3d11ShaderData;


//*****************************************************************************
VuD3d11ShaderProgram::VuD3d11ShaderProgram():
	mpDeviceContext(VuD3d11Gfx::IF()->getD3dDeviceContext()),
	mConstantCount(0),
	mSamplerCount(0)
{
	memset(mapShaders, 0, sizeof(mapShaders));
}

//*****************************************************************************
VuD3d11ShaderProgram::~VuD3d11ShaderProgram()
{
	mapShaders[VERTEX_SHADER]->removeRef();
	mapShaders[PIXEL_SHADER]->removeRef();

	// erase shader program
	D3d11ShaderData::ShaderPrograms::iterator iter = find(sD3d11ShaderData.mShaderPrograms.begin(), sD3d11ShaderData.mShaderPrograms.end(), this);
	VUASSERT(iter != sD3d11ShaderData.mShaderPrograms.end(), "VuD3d11ShaderProgram::~VuD3d11ShaderProgram() entry not found");
	sD3d11ShaderData.mShaderPrograms.erase(iter);
}

//*****************************************************************************
VUHANDLE VuD3d11ShaderProgram::getConstantByName(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);

	for ( int i = 0; i < mConstantCount; i++ )
		if ( maConstants[i].mNameHash == hashedName )
			return (VUHANDLE)&maConstants[i];

	return VUNULL;
}

//*****************************************************************************
int VuD3d11ShaderProgram::getSamplerIndexByName(const char *strName) const
{
	VuHashedString nameHash(strName);

	for ( int i = 0; i < mSamplerCount; i++ )
		if ( maSamplers[i].mNameHash == nameHash )
			return maSamplers[i].mIndex;

	return -1;
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantFloat(VUHANDLE handle, float fValue)
{
	setConstantValue(handle, &fValue, sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantFloat3(VUHANDLE handle, const float *pfValues)
{
	setConstantValue(handle, pfValues, 3*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantFloat4(VUHANDLE handle, const float *pfValues)
{
	setConstantValue(handle, pfValues, 4*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantInt(VUHANDLE handle, int iValue)
{
	setConstantValue(handle, &iValue, sizeof(iValue));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantMatrix(VUHANDLE handle, const VuMatrix &mat)
{
	setConstantValue(handle, &mat, 16*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantVector2(VUHANDLE handle, const VuVector2 &vec)
{
	setConstantValue(handle, &vec, 2*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantVector3(VUHANDLE handle, const VuVector3 &vec)
{
	setConstantValue(handle, &vec, 3*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantVector4(VUHANDLE handle, const VuVector4 &vec)
{
	setConstantValue(handle, &vec, 4*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantColor3(VUHANDLE handle, const VuColor &color)
{
	float aColor[3];
	color.toFloat3(aColor[0], aColor[1], aColor[2]);

	setConstantValue(handle, aColor, 3*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantColor4(VUHANDLE handle, const VuColor &color)
{
	float aColor[4];
	color.toFloat4(aColor[0], aColor[1], aColor[2], aColor[3]);

	setConstantValue(handle, aColor, 4*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantFloatArray(VUHANDLE handle, const float *pfValue, int count)
{
	setConstantValue(handle, pfValue, count*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantIntArray(VUHANDLE handle, const int *piValue, int count)
{
	setConstantValue(handle, piValue, count*sizeof(int));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantMatrixArray(VUHANDLE handle, const VuMatrix *pMat, int count, bool skinning)
{
	setConstantValue(handle, pMat, count*16*sizeof(float));
}

//*****************************************************************************
void VuD3d11ShaderProgram::setConstantVector4Array(VUHANDLE handle, const VuVector4 *pVec, int count)
{
	setConstantValue(handle, pVec, count*4*sizeof(float));
}

//*****************************************************************************
VuD3d11ShaderProgram *VuD3d11ShaderProgram::load(VuBinaryDataReader &reader)
{
	VuD3d11Shader *pVertexShader = VuD3d11Shader::load(VERTEX_SHADER, reader);
	VuD3d11Shader *pPixelShader = VuD3d11Shader::load(PIXEL_SHADER, reader);

	VuArray<VUBYTE> reflectionData(0);
	reader.readArray(reflectionData);

	// check if the shader program already exists
	for ( D3d11ShaderData::ShaderPrograms::iterator iter = sD3d11ShaderData.mShaderPrograms.begin(); iter != sD3d11ShaderData.mShaderPrograms.end(); iter++ )
	{
		if ( (*iter)->mapShaders[VERTEX_SHADER] == pVertexShader &&
		     (*iter)->mapShaders[PIXEL_SHADER] == pPixelShader )
		{
			pVertexShader->removeRef();
			pPixelShader->removeRef();
			(*iter)->addRef();
			return (*iter);
		}
	}

	// create new shader program
	VuD3d11ShaderProgram *pShaderProgram = new VuD3d11ShaderProgram;
	pShaderProgram->mapShaders[VERTEX_SHADER] = pVertexShader;
	pShaderProgram->mapShaders[PIXEL_SHADER] = pPixelShader;

	// build tables
	pShaderProgram->buildTables(reflectionData);

	// add to list
	sD3d11ShaderData.mShaderPrograms.push_back(pShaderProgram);

	return pShaderProgram;
}

//*****************************************************************************
void VuD3d11ShaderProgram::buildTables(const VuArray<VUBYTE> &reflectionData)
{
	VuBinaryDataReader reader(reflectionData);

	reader.readValue(mConstantCount);
	for ( int i = 0; i < mConstantCount; i++ )
	{
		reader.readValue(maConstants[i].mNameHash);
		reader.readValue(maConstants[i].mShader);
		reader.readValue(maConstants[i].mBuffer);
		reader.readValue(maConstants[i].mOffset);
	}

	reader.readValue(mSamplerCount);
	for ( int i = 0; i < mSamplerCount; i++ )
	{
		reader.readValue(maSamplers[i].mNameHash);
		reader.readValue(maSamplers[i].mIndex);
	}
}

//*****************************************************************************
VuD3d11Shader::VuD3d11Shader():
	mConstantBufferCount(0),
	mpConstantShadowBuffers(VUNULL),
	mapD3d11ConstantBuffers(VUNULL),
	mConstantBufferDirtyBits(0),
	mByteCode(0)
{
}

//*****************************************************************************
VuD3d11Shader::~VuD3d11Shader()
{
	// release
	delete[] mpConstantShadowBuffers;

	for ( VUUINT32 i = 0; i < mConstantBufferCount; i++ )
		D3DRELEASE(mapD3d11ConstantBuffers[i]);
	delete[] mapD3d11ConstantBuffers;

	// erase shader entry
	D3d11ShaderData::Shaders::iterator iter = find(sD3d11ShaderData.mShaders.begin(), sD3d11ShaderData.mShaders.end(), this);
	VUASSERT(iter != sD3d11ShaderData.mShaders.end(), "VuD3d11Shader::~VuD3d11Shader() entry not found");
	sD3d11ShaderData.mShaders.erase(iter);
}

//*****************************************************************************
VuD3d11Shader *VuD3d11Shader::load(VuShaderProgram::eShader shader, VuBinaryDataReader &reader)
{
	VUUINT32 hash;
	VuArray<VUBYTE> shaderData;
	VuArray<VUINT> constantBufferSizes;
	reader.readValue(hash);
	reader.readArray(constantBufferSizes);
	reader.readArray(shaderData);

	// check if we already have the shader compiled
	for ( D3d11ShaderData::Shaders::iterator iter = sD3d11ShaderData.mShaders.begin(); iter != sD3d11ShaderData.mShaders.end(); iter++ )
	{
		if ( (*iter)->mHash == hash )
		{
			(*iter)->addRef();
			return (*iter);
		}
	}

	// create vertex/pixel shader
	VuD3d11Shader *pShader = create(shader, shaderData, constantBufferSizes);
	pShader->mHash = hash;

	// add to list
	sD3d11ShaderData.mShaders.push_back(pShader);

	return pShader;
}

//*****************************************************************************
VuD3d11Shader *VuD3d11Shader::create(VuShaderProgram::eShader shader, const VuArray<VUBYTE> &shaderData, const VuArray<int> &constantBufferSizes)
{
	// create vertex/pixel shader
	VuD3d11Shader *pShader = VUNULL;
	if ( shader == VuD3d11ShaderProgram::VERTEX_SHADER )
	{
		VuD3d11VertexShader *pVertexShader = new VuD3d11VertexShader;
		D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateVertexShader(&shaderData[0], shaderData.size(), NULL, &pVertexShader->mpVertexShader));

		pShader = pVertexShader;
	}
	else if ( shader == VuD3d11ShaderProgram::PIXEL_SHADER )
	{
		VuD3d11PixelShader *pPixelShader = new VuD3d11PixelShader;
		D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreatePixelShader(&shaderData[0], shaderData.size(), NULL, &pPixelShader->mpPixelShader));

		pShader = pPixelShader;
	}

	// save byte code
	pShader->mByteCode.resize(shaderData.size());
	VU_MEMCPY(&pShader->mByteCode[0], pShader->mByteCode.size(), &shaderData[0], shaderData.size());

	// create constant buffers
	int bufferCount = constantBufferSizes.size();
	pShader->mConstantBufferCount = bufferCount;
	if ( bufferCount )
	{
		pShader->mpConstantShadowBuffers = new VuConstantShadowBuffer[bufferCount];
		pShader->mapD3d11ConstantBuffers = new ID3D11Buffer *[bufferCount];

		for ( int i = 0; i < bufferCount; i++ )
		{
			int size = constantBufferSizes[i];

			VuConstantShadowBuffer *pShadowBuffer = pShader->mpConstantShadowBuffers + i;
			pShadowBuffer->mSize = size;
			pShadowBuffer->mpData = new VUBYTE[size];
			memset(pShadowBuffer->mpData, 0, size);

			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.ByteWidth = size;
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateBuffer(&bufferDesc, NULL, &pShader->mapD3d11ConstantBuffers[i]));
		}
	}

	return pShader;
}

#if defined VUWIN32 && !VU_DISABLE_BAKING

//*****************************************************************************
bool VuD3d11Shader::compile(const char *profile, const std::string &shaderText, VuArray<VUBYTE> &shaderData, VuArray<VUINT> &constantBufferSizes, std::string &errors)
{
	ID3D10Blob *pShaderData = VUNULL;
	{
		ID3D10Blob *pErrors = NULL;
		UINT shaderFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR | D3D10_SHADER_OPTIMIZATION_LEVEL3;
		HRESULT res = D3DCompile(shaderText.c_str(), shaderText.length(), NULL, NULL, NULL, "main", profile, shaderFlags, 0, &pShaderData, &pErrors);
		if (pErrors)
		{
			errors = (const char *)pErrors->GetBufferPointer();
			pErrors->Release();
		}
		if (res != S_OK)
			return false;
	}

	int bufferSize = pShaderData->GetBufferSize();
	shaderData.resize(bufferSize);
	memcpy(&shaderData[0], pShaderData->GetBufferPointer(), bufferSize);
	pShaderData->Release();

	// determine constant buffer size
	ID3D11ShaderReflection *pReflection;
	if (D3DReflect(&shaderData[0], shaderData.size(), IID_ID3D11ShaderReflection, (void**)&pReflection) != S_OK)
	{
		errors += "D3DReflect() failed\n";
		return false;
	}

	D3D11_SHADER_DESC shaderDesc;
	if (pReflection->GetDesc(&shaderDesc) != S_OK)
	{
		errors += "ID3D11ShaderReflection::GetDesc() failed\n";
		return false;
	}

	constantBufferSizes.resize(shaderDesc.ConstantBuffers);
	for ( VUUINT i = 0; i < shaderDesc.ConstantBuffers; i++ )
	{
		ID3D11ShaderReflectionConstantBuffer *pConstantBuffer = pReflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC shaderBufferDesc;
		pConstantBuffer->GetDesc(&shaderBufferDesc);

		constantBufferSizes[i] = shaderBufferDesc.Size;
	}

	// clean up
	pReflection->Release();

	return true;
}

struct VuD3d11ShaderProgramTables
{
	VuD3d11ShaderProgramTables() : mConstantCount(0), mSamplerCount(0) {}

	bool		build(ID3D11ShaderReflection *pReflection, int shader);
	void		serialize(VuBinaryDataWriter &writer);

	typedef VuD3d11ShaderProgram::Constant Constant;
	typedef VuD3d11ShaderProgram::Sampler Sampler;

	int			mConstantCount;
	Constant	maConstants[VuD3d11ShaderProgram::MAX_CONSTANT_COUNT];

	int			mSamplerCount;
	Sampler		maSamplers[VuD3d11ShaderProgram::MAX_SAMPLER_COUNT];
};

//*****************************************************************************
bool VuD3d11Shader::reflect(const VuArray<VUBYTE> &vertexShaderData, const VuArray<VUBYTE> &pixelShaderData, VuArray<VUBYTE> &reflectionData)
{
	// reflect
	ID3D11ShaderReflection *pVertexShaderReflection;
	if (D3DReflect(&vertexShaderData[0], vertexShaderData.size(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection) != S_OK)
		return false;

	ID3D11ShaderReflection *pPixelShaderReflection;
	if (D3DReflect(&pixelShaderData[0], pixelShaderData.size(), IID_ID3D11ShaderReflection, (void**)&pPixelShaderReflection) != S_OK)
		return false;

	VuD3d11ShaderProgramTables tables;

	if (!tables.build(pVertexShaderReflection, 0))
		return false;

	if (!tables.build(pPixelShaderReflection, 1))
		return false;

	VuBinaryDataWriter writer(reflectionData);
	tables.serialize(writer);

	// clean up
	pVertexShaderReflection->Release();
	pPixelShaderReflection->Release();

	return true;
}

//*****************************************************************************
bool VuD3d11ShaderProgramTables::build(ID3D11ShaderReflection *pReflection, int shader)
{
	D3D11_SHADER_DESC shaderDesc;
	if (pReflection->GetDesc(&shaderDesc) != S_OK)
		return false;

	// constants
	for (VUUINT buffer = 0; buffer < shaderDesc.ConstantBuffers; buffer++)
	{
		ID3D11ShaderReflectionConstantBuffer *pConstantBuffer = pReflection->GetConstantBufferByIndex(buffer);

		D3D11_SHADER_BUFFER_DESC shaderBufferDesc;
		if (pConstantBuffer->GetDesc(&shaderBufferDesc) != S_OK)
			return false;

		for (VUUINT iConst = 0; iConst < shaderBufferDesc.Variables; iConst++)
		{
			ID3D11ShaderReflectionVariable *pVariable = pConstantBuffer->GetVariableByIndex(iConst);

			D3D11_SHADER_VARIABLE_DESC variableDesc;
			pVariable->GetDesc(&variableDesc);

			if (mConstantCount >= VuD3d11ShaderProgram::MAX_CONSTANT_COUNT)
				return false;

			Constant &constant = maConstants[mConstantCount++];
			constant.mNameHash = VuHash::fnv32String(variableDesc.Name);
			constant.mShader = shader;
			constant.mBuffer = buffer;
			constant.mOffset = variableDesc.StartOffset;
		}
	}

	// samplers
	for (VUUINT iResource = 0; iResource < shaderDesc.BoundResources; iResource++)
	{
		D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
		if (pReflection->GetResourceBindingDesc(iResource, &resourceDesc) != S_OK)
			return false;

		if (resourceDesc.Type == D3D10_SIT_SAMPLER)
		{
			if (mSamplerCount >= VuD3d11ShaderProgram::MAX_SAMPLER_COUNT)
				return false;

			Sampler &sampler = maSamplers[mSamplerCount++];
			sampler.mNameHash = VuHash::fnv32String(resourceDesc.Name);
			sampler.mIndex = resourceDesc.BindPoint;

			// find matching texture
			for (VUUINT i = 0; i < shaderDesc.BoundResources; i++)
			{
				D3D11_SHADER_INPUT_BIND_DESC desc;
				if (pReflection->GetResourceBindingDesc(i, &desc) != S_OK)
					return false;

				if (desc.Type == D3D10_SIT_TEXTURE)
				{
					if (sampler.mNameHash == VuHash::fnv32String(desc.Name))
					{
						sampler.mIndex += (desc.BindPoint << 16);
						break;
					}
				}
			}
		}
	}

	return true;
}

//*****************************************************************************
void VuD3d11ShaderProgramTables::serialize(VuBinaryDataWriter &writer)
{
	writer.writeValue(mConstantCount);
	for (int i = 0; i < mConstantCount; i++)
	{
		writer.writeValue(maConstants[i].mNameHash);
		writer.writeValue(maConstants[i].mShader);
		writer.writeValue(maConstants[i].mBuffer);
		writer.writeValue(maConstants[i].mOffset);
	}

	writer.writeValue(mSamplerCount);
	for (int i = 0; i < mSamplerCount; i++)
	{
		writer.writeValue(maSamplers[i].mNameHash);
		writer.writeValue(maSamplers[i].mIndex);
	}
}

#endif // VUWIN32 && !VU_DISABLE_BAKING