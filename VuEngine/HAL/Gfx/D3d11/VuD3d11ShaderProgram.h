//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the shader interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Util/VuHashedString.h"


struct ID3D11ShaderReflection;
struct ID3D11ShaderReflectionVariable;


class VuD3d11Shader : public VuRefObj
{
public:
	VuD3d11Shader();
	~VuD3d11Shader();

	static VuD3d11Shader	*load(VuShaderProgram::eShader shader, VuBinaryDataReader &reader);
	static VuD3d11Shader	*create(VuShaderProgram::eShader shader, const VuArray<VUBYTE> &shaderData, const VuArray<VUINT> &constantBufferSizes);

	static bool				compile(const char *profile, const std::string &shaderText, VuArray<VUBYTE> &shaderData, VuArray<VUINT> &constantBufferSizes, std::string &errors);
	static bool				reflect(const VuArray<VUBYTE> &vertexShaderData, const VuArray<VUBYTE> &pixelShaderData, VuArray<VUBYTE> &reflectionData);

	struct VuConstantShadowBuffer
	{
		VuConstantShadowBuffer() : mpData(VUNULL), mSize(0) {}
		~VuConstantShadowBuffer() { delete[] mpData; }
		VUBYTE				*mpData;
		VUUINT32			mSize;
	};
	VUUINT32				mHash;
	VUUINT32				mConstantBufferCount;
	VuConstantShadowBuffer	*mpConstantShadowBuffers;
	ID3D11Buffer			**mapD3d11ConstantBuffers;
	VUUINT32				mConstantBufferDirtyBits;
	VuArray<VUBYTE>			mByteCode;
};

class VuD3d11VertexShader : public VuD3d11Shader
{
public:
	~VuD3d11VertexShader() { mpVertexShader->Release(); }

	ID3D11VertexShader	*mpVertexShader;
};

class VuD3d11PixelShader : public VuD3d11Shader
{
public:
	~VuD3d11PixelShader() { mpPixelShader->Release(); }

	ID3D11PixelShader	*mpPixelShader;
};

class VuD3d11ShaderProgram : public VuShaderProgram
{
public:
	VuD3d11ShaderProgram();
	~VuD3d11ShaderProgram();

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

	ID3D11VertexShader		*getD3d11VertexShader() const { return static_cast<VuD3d11VertexShader *>(mapShaders[VERTEX_SHADER])->mpVertexShader; }
	ID3D11PixelShader		*getD3d11PixelShader() const { return static_cast<VuD3d11PixelShader *>(mapShaders[PIXEL_SHADER])->mpPixelShader; }
	
	static VuD3d11ShaderProgram	*load(VuBinaryDataReader &reader);

	void					buildTables(const VuArray<VUBYTE> &reflectionData);

	ID3D11DeviceContext		*mpDeviceContext;
	VuD3d11Shader			*mapShaders[NUM_SHADER_TYPES];

	// constants
	struct Constant
	{
		VUUINT32		mNameHash;
		int				mShader;
		int				mBuffer;
		int				mOffset;
	};
	enum { MAX_CONSTANT_COUNT = 64 };
	int			mConstantCount;
	Constant	maConstants[MAX_CONSTANT_COUNT];

	// samplers
	struct Sampler
	{
		VUUINT32		mNameHash;
		int				mIndex;
	};
	enum { MAX_SAMPLER_COUNT = 16 };
	int			mSamplerCount;
	Sampler		maSamplers[MAX_SAMPLER_COUNT];
};


//*****************************************************************************
inline void VuD3d11ShaderProgram::setConstantValue(VUHANDLE handle, const void *data, int size)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);

	VuD3d11Shader *pShader = mapShaders[pConstant->mShader];
	VuD3d11Shader::VuConstantShadowBuffer *pBuffer = pShader->mpConstantShadowBuffers + pConstant->mBuffer;

	VU_MEMCPY(pBuffer->mpData + pConstant->mOffset, pBuffer->mSize - pConstant->mOffset, data, size);

	pShader->mConstantBufferDirtyBits |= 1<<pConstant->mBuffer;
}