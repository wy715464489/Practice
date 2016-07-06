//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the shader interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include <shader.h>

#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/Containers/VuArray.h"

using namespace sce;


class VuPs4Shader;


class VuPs4ShaderProgram : public VuShaderProgram
{
public:
	VuPs4ShaderProgram();
	~VuPs4ShaderProgram();

	// external interface

	virtual VUHANDLE		getConstantByName(const char *strName) const;
	virtual int				getSamplerIndexByName(const char *strName) const;

	virtual void			setConstantFloat(VUHANDLE handle, float fValue);
	virtual void			setConstantFloat3(VUHANDLE handle, const float *pfValues);
	virtual void			setConstantFloat4(VUHANDLE handle, const float *pfValues);
	virtual void			setConstantInt(VUHANDLE handle, int iValue);
	virtual void			setConstantMatrix(VUHANDLE handle, const VuMatrix &mat);
	virtual void			setConstantVector2(VUHANDLE handle, const VuVector2 &vec);
	virtual void			setConstantVector3(VUHANDLE handle, const VuVector3 &vec);
	virtual void			setConstantVector4(VUHANDLE handle, const VuVector4 &vec);
	virtual void			setConstantColor3(VUHANDLE handle, const VuColor &color);
	virtual void			setConstantColor4(VUHANDLE handle, const VuColor &color);

	virtual void			setConstantFloatArray(VUHANDLE handle, const float *pfValue, int count);
	virtual void			setConstantIntArray(VUHANDLE handle, const int *piValue, int count);
	virtual void			setConstantMatrixArray(VUHANDLE handle, const VuMatrix *pMat, int count, bool skinning);
	virtual void			setConstantVector4Array(VUHANDLE handle, const VuVector4 *pVec, int count);

	inline void				setConstantValue(VUHANDLE handle, const void *data, int size);

	static VuPs4ShaderProgram	*load(VuBinaryDataReader &reader);

	void					reflect(eShader shader);

	VuPs4Shader				*mapShaders[NUM_SHADER_TYPES];

	// constants
	struct Constant
	{
		VUUINT32		mHashedName;
		int				mShader;
		int				mBuffer;
		int				mOffset;
	};
	enum { MAX_CONSTANT_COUNT = 64 };
	int			mConstantCount;
	Constant	mConstants[MAX_CONSTANT_COUNT];

	// samplers
	struct Sampler
	{
		VUUINT32		mHashedName;
		int				mIndex;
	};
	enum { MAX_SAMPLER_COUNT = 16 };
	int			mSamplerCount;
	Sampler		mSamplers[MAX_SAMPLER_COUNT];
};

class VuPs4Shader : public VuRefObj
{
public:
	VuPs4Shader(const VuArray<VUBYTE> &shaderData);
	~VuPs4Shader();

	static VuPs4Shader		*load(VuShaderProgram::eShader shader, VuBinaryDataReader &reader);

	void					allocateConstantBuffers();

	struct VuConstantBuffer
	{
		VuConstantBuffer() : mpData(VUNULL), mSize(0) {}
		~VuConstantBuffer() { delete[] mpData; }
		VUBYTE				*mpData;
		VUUINT32			mSize;
	};
	enum { MAX_CONSTANT_BUFFER_COUNT = 4 };
	VUUINT32				mHash;
	VUUINT32				mConstantBufferCount;
	VuConstantBuffer		mConstantBuffers[MAX_CONSTANT_BUFFER_COUNT];
	VUUINT32				mConstantBufferDirtyBits;
	VuArray<VUBYTE>			mPs4BinaryData;
	Shader::Binary::Program	mPs4BinaryProgram;
};


class VuPs4VertexShader : public VuPs4Shader
{
public:
	VuPs4VertexShader(const VuArray<VUBYTE> &shaderData);
	~VuPs4VertexShader();

	Gnmx::VsShader		*mpPs4VertexShader;
};

class VuPs4PixelShader : public VuPs4Shader
{
public:
	VuPs4PixelShader(const VuArray<VUBYTE> &shaderData);
	~VuPs4PixelShader();

	Gnmx::PsShader		*mpPs4PixelShader;
};

//*****************************************************************************
inline void VuPs4ShaderProgram::setConstantValue(VUHANDLE handle, const void *data, int size)
{
	const Constant *pConstant = static_cast<Constant *>(handle);

	VuPs4Shader *pShader = mapShaders[pConstant->mShader];
	const VuPs4Shader::VuConstantBuffer &buffer = pShader->mConstantBuffers[pConstant->mBuffer];

	VU_MEMCPY(buffer.mpData + pConstant->mOffset, buffer.mSize - pConstant->mOffset, data, size);

	pShader->mConstantBufferDirtyBits |= 1<<pConstant->mBuffer;
}

