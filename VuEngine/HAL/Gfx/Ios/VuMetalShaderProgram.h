//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the shader interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuShaderProgram.h"


class VuMetalShaderProgram : public VuShaderProgram
{
public:
	VuMetalShaderProgram();
	~VuMetalShaderProgram();
	
	virtual VUHANDLE	getConstantByName(const char *strName) const;
	virtual int			getSamplerIndexByName(const char *strName) const;
	
	virtual void		setConstantFloat(VUHANDLE handle, float fValue);
	virtual void		setConstantFloat3(VUHANDLE handle, const float *pfValues);
	virtual void		setConstantFloat4(VUHANDLE handle, const float *pfValues);
	virtual void		setConstantInt(VUHANDLE handle, int iValue);
	virtual void		setConstantMatrix(VUHANDLE handle, const VuMatrix &mat);
	virtual void		setConstantVector2(VUHANDLE handle, const VuVector2 &vec);
	virtual void		setConstantVector3(VUHANDLE handle, const VuVector3 &vec);
	virtual void		setConstantVector4(VUHANDLE handle, const VuVector4 &vec);
	virtual void		setConstantColor3(VUHANDLE handle, const VuColor &color);
	virtual void		setConstantColor4(VUHANDLE handle, const VuColor &color);
	
	virtual void		setConstantFloatArray(VUHANDLE handle, const float *pfValue, int count);
	virtual void		setConstantIntArray(VUHANDLE handle, const int *piValue, int count);
	virtual void		setConstantMatrixArray(VUHANDLE handle, const VuMatrix *pMat, int count, bool skinning);
	virtual void		setConstantVector4Array(VUHANDLE handle, const VuVector4 *pVec, int count);

	inline void			setConstantValue(VUHANDLE handle, const void *data, int size);
	
	static VuMetalShaderProgram	*load(VuBinaryDataReader &reader);
	
	bool				buildTables();
	void				reflect(eShader shader, NSArray *args);
	
	VUUINT32			mHash;
	
	struct ConstantBuffer
	{
		ConstantBuffer() : mIndex(0), mpData(VUNULL), mSize(0) {}
		~ConstantBuffer() { delete[] mpData; }
		int		mIndex;
		VUBYTE	*mpData;
		int		mSize;
	};
	enum { MAX_CONSTANT_BUFFER_COUNT = 4 };
	struct Shader
	{
		Shader() : mConstantBufferCount(0), mConstantBufferDirtyBits(0) {}
		id<MTLFunction>	mMtlFunction;
		int				mConstantBufferCount;
		ConstantBuffer	mConstantBuffers[MAX_CONSTANT_BUFFER_COUNT];
		VUUINT32		mConstantBufferDirtyBits;
	};
	Shader			mShaders[NUM_SHADER_TYPES];
	
	// constants
	struct Constant
	{
		VUUINT32	mHashedName;
		eShader		mShader;
		int			mBuffer;
		int			mOffset;
	};
	enum { MAX_CONSTANT_COUNT = 64 };
	int				mConstantCount;
	Constant		mConstants[MAX_CONSTANT_COUNT];
	
	// samplers
	struct Sampler
	{
		VUUINT32	mHashedName;
		int			mIndex;
	};
	enum { MAX_SAMPLER_COUNT = 16 };
	int				mSamplerCount;
	Sampler			mSamplers[MAX_SAMPLER_COUNT];
};


//*****************************************************************************
inline void VuMetalShaderProgram::setConstantValue(VUHANDLE handle, const void *data, int size)
{
	Constant *pConstant = static_cast<Constant *>(handle);
	
	Shader &shader = mShaders[pConstant->mShader];
	ConstantBuffer &buffer = shader.mConstantBuffers[pConstant->mBuffer];
	
	VU_MEMCPY(buffer.mpData + pConstant->mOffset, buffer.mSize - pConstant->mOffset, data, size);
	
	shader.mConstantBufferDirtyBits |= 1<<pConstant->mBuffer;
}