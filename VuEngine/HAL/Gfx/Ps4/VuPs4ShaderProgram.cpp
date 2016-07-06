//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the shader interface class.
//
//*****************************************************************************

#include "toolkit/shader_loader.h"
#include "VuPs4ShaderProgram.h"
#include "VuPs4Gfx.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuHash.h"


// static variables
static struct Ps4ShaderData
{
	typedef std::list<VuPs4ShaderProgram *> ShaderPrograms;
	typedef std::list<VuPs4Shader *> Shaders;
	ShaderPrograms	mShaderPrograms;
	Shaders			mShaders;
} sPs4ShaderData;


//*****************************************************************************
VuPs4ShaderProgram::VuPs4ShaderProgram():
	mConstantCount(0),
	mSamplerCount(0)
{
	memset(mapShaders, 0, sizeof(mapShaders));
}

//*****************************************************************************
VuPs4ShaderProgram::~VuPs4ShaderProgram()
{
	mapShaders[VERTEX_SHADER]->removeRef();
	mapShaders[PIXEL_SHADER]->removeRef();

	// erase shader program
	Ps4ShaderData::ShaderPrograms::iterator iter = find(sPs4ShaderData.mShaderPrograms.begin(), sPs4ShaderData.mShaderPrograms.end(), this);
	VUASSERT(iter != sPs4ShaderData.mShaderPrograms.end(), "VuPs4ShaderProgram::~VuPs4ShaderProgram() entry not found");
	sPs4ShaderData.mShaderPrograms.erase(iter);
}

//*****************************************************************************
VUHANDLE VuPs4ShaderProgram::getConstantByName(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);

	for ( int i = 0; i < mConstantCount; i++ )
		if ( mConstants[i].mHashedName == hashedName )
			return (VUHANDLE)&mConstants[i];

	return VUNULL;
}

//*****************************************************************************
int VuPs4ShaderProgram::getSamplerIndexByName(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);

	for ( int i = 0; i < mSamplerCount; i++ )
		if ( mSamplers[i].mHashedName == hashedName )
			return mSamplers[i].mIndex;

	return -1;
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantFloat(VUHANDLE handle, float fValue)
{
	setConstantValue(handle, &fValue, sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantFloat3(VUHANDLE handle, const float *pfValues)
{
	setConstantValue(handle, pfValues, 3*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantFloat4(VUHANDLE handle, const float *pfValues)
{
	setConstantValue(handle, pfValues, 4*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantInt(VUHANDLE handle, int iValue)
{
	setConstantValue(handle, &iValue, sizeof(iValue));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantMatrix(VUHANDLE handle, const VuMatrix &mat)
{
	setConstantValue(handle, &mat, 16*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantVector2(VUHANDLE handle, const VuVector2 &vec)
{
	setConstantValue(handle, &vec, 2*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantVector3(VUHANDLE handle, const VuVector3 &vec)
{
	setConstantValue(handle, &vec, 3*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantVector4(VUHANDLE handle, const VuVector4 &vec)
{
	setConstantValue(handle, &vec, 4*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantColor3(VUHANDLE handle, const VuColor &color)
{
	float aColor[3];
	color.toFloat3(aColor[0], aColor[1], aColor[2]);
	setConstantValue(handle, aColor, 3*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantColor4(VUHANDLE handle, const VuColor &color)
{
	float aColor[4];
	color.toFloat4(aColor[0], aColor[1], aColor[2], aColor[3]);
	setConstantValue(handle, aColor, 4*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantFloatArray(VUHANDLE handle, const float *pfValue, int count)
{
	setConstantValue(handle, pfValue, count*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantIntArray(VUHANDLE handle, const int *piValue, int count)
{
	setConstantValue(handle, piValue, count*sizeof(int));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantMatrixArray(VUHANDLE handle, const VuMatrix *pMat, int count, bool skinning)
{
	setConstantValue(handle, pMat, count*16*sizeof(float));
}

//*****************************************************************************
void VuPs4ShaderProgram::setConstantVector4Array(VUHANDLE handle, const VuVector4 *pVec, int count)
{
	setConstantValue(handle, pVec, count*4*sizeof(float));
}

//*****************************************************************************
VuPs4ShaderProgram *VuPs4ShaderProgram::load(VuBinaryDataReader &reader)
{
	VuPs4Shader *pVertexShader = VuPs4Shader::load(VERTEX_SHADER, reader);
	VuPs4Shader *pPixelShader = VuPs4Shader::load(PIXEL_SHADER, reader);

	// check if the shader program already exists
	for ( Ps4ShaderData::ShaderPrograms::iterator iter = sPs4ShaderData.mShaderPrograms.begin(); iter != sPs4ShaderData.mShaderPrograms.end(); iter++ )
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
	VuPs4ShaderProgram *pShaderProgram = new VuPs4ShaderProgram;
	pShaderProgram->mapShaders[VERTEX_SHADER] = pVertexShader;
	pShaderProgram->mapShaders[PIXEL_SHADER] = pPixelShader;

	// build tables
	pShaderProgram->reflect(VERTEX_SHADER);
	pShaderProgram->reflect(PIXEL_SHADER);

	// add to list
	sPs4ShaderData.mShaderPrograms.push_back(pShaderProgram);

	return pShaderProgram;
}

//*****************************************************************************
void VuPs4ShaderProgram::reflect(eShader shader)
{
	const VuPs4Shader *pPs4Shader = mapShaders[shader];
	const Shader::Binary::Program &program = pPs4Shader->mPs4BinaryProgram;

	int constantBufferIndex = 0;

	// constant buffers
	for ( int iBuffer = 0; iBuffer < program.m_numBuffers; iBuffer++ )
	{
		const Shader::Binary::Buffer &buffer = program.m_buffers[iBuffer];
		if ( buffer.m_langType == Shader::Binary::kBufferTypeConstantBuffer )
		{
			// reflect mapping
			for ( int iElement = 0; iElement < buffer.m_numElements; iElement++ )
			{
				const Shader::Binary::Element &element = program.m_elements[buffer.m_elementOffset + iElement];
				VUASSERT(element.m_elementType == Shader::Binary::kPsslConstantBufferElement, "Bad Ps4 constant buffer assumption");

				VUASSERT(mConstantCount < MAX_CONSTANT_COUNT, "Too many constants in shader program!");
				Constant &constant = mConstants[mConstantCount];
				mConstantCount++;

				const char *constantName = element.getName();

				constant.mHashedName = VuHash::fnv32String(constantName);
				constant.mShader = shader;
				constant.mBuffer = constantBufferIndex;
				constant.mOffset = element.m_byteOffset;
			}

			constantBufferIndex++;
		}
		else if ( buffer.m_langType == Shader::Binary::kBufferTypeTexture2d || buffer.m_langType == Shader::Binary::kBufferTypeTextureCube || buffer.m_langType == Shader::Binary::kBufferTypeTexture2dArray )
		{
			VUASSERT(mSamplerCount < MAX_SAMPLER_COUNT, "Too many textures in shader program!");
			Sampler &sampler = mSamplers[mSamplerCount];
			mSamplerCount++;

			const char *textureName = buffer.getName();

			sampler.mHashedName = VuHash::fnv32String(textureName);
			sampler.mIndex = buffer.m_resourceIndex;

			// make sure that a matching sampler exists
			char samplerName[128];
			VU_SPRINTF(samplerName, sizeof(samplerName), "%sSampler", textureName);
			Shader::Binary::SamplerState *pSampler = program.getSamplerStateByName(samplerName);
			if ( pSampler == VUNULL )
			{
				VUPRINTF("Missing sampler state for texture '%s'\n", textureName);
				VUASSERT(0, "Missing sampler state");
			}

			// ensure 1-1 correlation between texture & sampler
			VUASSERT(pSampler->m_resourceIndex == buffer.m_resourceIndex, "Texture-Sampler mismatch");
		}
	}
}

//*****************************************************************************
VuPs4Shader::VuPs4Shader(const VuArray<VUBYTE> &shaderData):
	mConstantBufferCount(0),
	mConstantBufferDirtyBits(false),
	mPs4BinaryData(0)
{
	mPs4BinaryData.resize(shaderData.size());
	VU_MEMCPY(&mPs4BinaryData[0], mPs4BinaryData.size(), &shaderData[0], shaderData.size());
}

//*****************************************************************************
VuPs4Shader::~VuPs4Shader()
{
	// erase shader entry
	Ps4ShaderData::Shaders::iterator iter = find(sPs4ShaderData.mShaders.begin(), sPs4ShaderData.mShaders.end(), this);
	VUASSERT(iter != sPs4ShaderData.mShaders.end(), "VuPs4Shader::~VuPs4Shader() entry not found");
	sPs4ShaderData.mShaders.erase(iter);
}

//*****************************************************************************
VuPs4Shader *VuPs4Shader::load(VuShaderProgram::eShader shader, VuBinaryDataReader &reader)
{
	VUUINT32 hash;
	VuArray<VUBYTE> shaderData;
	reader.readValue(hash);
	reader.readArray(shaderData);
	
	// check if we already have the shader compiled
	for ( Ps4ShaderData::Shaders::iterator iter = sPs4ShaderData.mShaders.begin(); iter != sPs4ShaderData.mShaders.end(); iter++ )
	{
		if ( (*iter)->mHash == hash )
		{
			(*iter)->addRef();
			return (*iter);
		}
	}

	// create shader
	VuPs4Shader *pShader = VUNULL;
	if ( shader == VuShaderProgram::VERTEX_SHADER )
	{
		pShader = new VuPs4VertexShader(shaderData);
	}
	else if ( shader == VuShaderProgram::PIXEL_SHADER )
	{
		pShader = new VuPs4PixelShader(shaderData);
	}

	pShader->mHash = hash;

	// allocate constant buffers
	pShader->allocateConstantBuffers();

	// add to list
	sPs4ShaderData.mShaders.push_back(pShader);

	return pShader;
}

//*****************************************************************************
void VuPs4Shader::allocateConstantBuffers()
{
	const Shader::Binary::Program &program = mPs4BinaryProgram;

	// constant buffers
	for ( int iBuffer = 0; iBuffer < program.m_numBuffers; iBuffer++ )
	{
		const Shader::Binary::Buffer &buffer = program.m_buffers[iBuffer];
		if ( buffer.m_langType == Shader::Binary::kBufferTypeConstantBuffer )
		{
			// allocate buffer
			VUASSERT(mConstantBufferCount < VuPs4Shader::MAX_CONSTANT_BUFFER_COUNT, "Too many constant buffers in shader!");
			VuPs4Shader::VuConstantBuffer &constantBuffer = mConstantBuffers[mConstantBufferCount];
			mConstantBufferCount++;
					
			constantBuffer.mSize = buffer.m_strideSize;
			constantBuffer.mpData = new VUBYTE[constantBuffer.mSize];
		}
	}
}

//*****************************************************************************
VuPs4VertexShader::VuPs4VertexShader(const VuArray<VUBYTE> &shaderData):
	VuPs4Shader(shaderData)
{
	mpPs4VertexShader = Gnmx::Toolkit::loadAndAllocateVertexProgram(&mPs4BinaryProgram, &mPs4BinaryData[0], mPs4BinaryData.size(), &VuPs4Gfx::IF()->garlicAllocator());
}

//*****************************************************************************
VuPs4VertexShader::~VuPs4VertexShader()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpPs4VertexShader);
}

//*****************************************************************************
VuPs4PixelShader::VuPs4PixelShader(const VuArray<VUBYTE> &shaderData):
	VuPs4Shader(shaderData)
{
	mpPs4PixelShader = Gnmx::Toolkit::loadAndAllocatePixelProgram(&mPs4BinaryProgram, &mPs4BinaryData[0], shaderData.size(), &VuPs4Gfx::IF()->garlicAllocator());
}

//*****************************************************************************
VuPs4PixelShader::~VuPs4PixelShader()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpPs4PixelShader);
}