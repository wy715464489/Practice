//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the shader interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/Util/VuHashedString.h"

class VuOglesShader;

class VuOglesShaderProgram : public VuShaderProgram
{
public:
	VuOglesShaderProgram();
	~VuOglesShaderProgram();

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

	static VuOglesShaderProgram	*load(VuBinaryDataReader &reader);
	static VuOglesShaderProgram	*createProgram(VuOglesShader *pVertexShader, VuOglesShader *pPixelShader);

	void					rebuildTables();

	VuOglesShader			*mapShaders[NUM_SHADER_TYPES];
	GLuint					mGlProgram;

	// constants
	struct Constant
	{
		VuHashedString		mNameHash;
		int					mUniformIndex;
		GLint				mLocation;
	};
	enum { MAX_CONSTANT_COUNT = 64 };
	int						mConstantCount;
	Constant				maConstants[MAX_CONSTANT_COUNT];

	// samplers
	struct Sampler
	{
		VuHashedString		mNameHash;
		int					mSamplerIndex;
	};
	enum { MAX_SAMPLER_COUNT = 16 };
	int						mSamplerCount;
	Sampler					maSamplers[MAX_SAMPLER_COUNT];

	// attributes
	enum { MAX_ATTRIBUTE_COUNT = 8 };
	int						mAttributeCount;
	int						maAttributes[MAX_ATTRIBUTE_COUNT];
};
