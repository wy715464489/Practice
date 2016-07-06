//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the shader interface class.
//
//*****************************************************************************

#include "VuOglesShaderProgram.h"
#include "VuOglesGfx.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector4.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


class VuOglesShader : public VuRefObj
{
public:
	~VuOglesShader();

	static VuOglesShader	*compile(GLenum type, const char *strData, int glVersion);

	VUUINT32		mHash;
	GLuint			mGlShader;
	GLenum			mGlType;
};


// static variables
static struct OglShaderData
{
	typedef std::list<VuOglesShaderProgram *> ShaderPrograms;
	typedef std::list<VuOglesShader *> Shaders;
	ShaderPrograms	mShaderPrograms;
	Shaders			mShaders;
} sOglShaderData;

struct StandardAttrib
{
	int			mIndex;
	const char	*mName;
};
static StandardAttrib sStandardAttribs[] =
{
	{ 0, "aPosition" },
	{ 0, "aPosition0" },
	{ 1, "aNormal" },
	{ 2, "aColor" },
	{ 2, "aColor0" },
	{ 3, "aTangent" },
	{ 4, "aBlendWeight" },
	{ 5, "aBlendIndices" },
	{ 6, "aTexCoord" },
	{ 6, "aTexCoord0" },
	{ 7, "aSceneColor" },
	{ 7, "aColor1" },
	{ 7, "aTexCoord1" },
};
static const int sStandardAttribCount = sizeof(sStandardAttribs)/sizeof(sStandardAttribs[0]);


//*****************************************************************************
VuOglesShaderProgram::VuOglesShaderProgram():
	mGlProgram(0),
	mConstantCount(0),
	mSamplerCount(0),
	mAttributeCount(0)
{
	memset(mapShaders, 0, sizeof(mapShaders));
}

//*****************************************************************************
VuOglesShaderProgram::~VuOglesShaderProgram()
{
	mapShaders[VERTEX_SHADER]->removeRef();
	mapShaders[PIXEL_SHADER]->removeRef();

	if ( !VuOglesGfx::IF()->getContextDestroyed() )
		glDeleteProgram(mGlProgram);

	// erase shader program
	OglShaderData::ShaderPrograms::iterator iter = find(sOglShaderData.mShaderPrograms.begin(), sOglShaderData.mShaderPrograms.end(), this);
	VUASSERT(iter != sOglShaderData.mShaderPrograms.end(), "VuOglesShaderProgram::~VuOglesShaderProgram() entry not found");
	sOglShaderData.mShaderPrograms.erase(iter);
}

//*****************************************************************************
VUHANDLE VuOglesShaderProgram::getConstantByName(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);

	for ( int i = 0; i < mConstantCount; i++ )
		if ( maConstants[i].mNameHash == hashedName )
			return (VUHANDLE)&maConstants[i];

	return VUNULL;
}

//*****************************************************************************
int VuOglesShaderProgram::getSamplerIndexByName(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);

	for ( int i = 0; i < mSamplerCount; i++ )
		if ( maSamplers[i].mNameHash == hashedName )
			return maSamplers[i].mSamplerIndex;

	return -1;
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantFloat(VUHANDLE handle, float fValue)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform1f(pConstant->mLocation, fValue);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantFloat3(VUHANDLE handle, const float *pfValues)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform3fv(pConstant->mLocation, 1, pfValues);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantFloat4(VUHANDLE handle, const float *pfValues)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform4fv(pConstant->mLocation, 1, pfValues);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantInt(VUHANDLE handle, int iValue)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform1i(pConstant->mLocation, iValue);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantMatrix(VUHANDLE handle, const VuMatrix &mat)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniformMatrix4fv(pConstant->mLocation, 1, GL_FALSE, &mat.mX.mX);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantVector2(VUHANDLE handle, const VuVector2 &vec)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform2fv(pConstant->mLocation, 1, &vec.mX);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantVector3(VUHANDLE handle, const VuVector3 &vec)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform3fv(pConstant->mLocation, 1, &vec.mX);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantVector4(VUHANDLE handle, const VuVector4 &vec)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform4fv(pConstant->mLocation, 1, &vec.mX);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantColor3(VUHANDLE handle, const VuColor &color)
{
	float aColor[3];
	color.toFloat3(aColor[0], aColor[1], aColor[2]);

	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform3f(pConstant->mLocation, aColor[0], aColor[1], aColor[2]);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantColor4(VUHANDLE handle, const VuColor &color)
{
	float aColor[4];
	color.toFloat4(aColor[0], aColor[1], aColor[2], aColor[3]);

	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform4f(pConstant->mLocation, aColor[0], aColor[1], aColor[2], aColor[3]);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantFloatArray(VUHANDLE handle, const float *pfValue, int count)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform1fv(pConstant->mLocation, count, pfValue);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantIntArray(VUHANDLE handle, const int *piValue, int count)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform1iv(pConstant->mLocation, count, piValue);
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantMatrixArray(VUHANDLE handle, const VuMatrix *pMat, int count, bool skinning)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);

	if ( skinning )
	{
		// need to convert to vec4[count*3]... thanks Qualcomm!
		VuVector4 *pVec4Start = (VuVector4 *)VuScratchPad::get(VuScratchPad::GRAPHICS);

		VuVector4 *pVec4 = pVec4Start;
		for ( int i = 0; i < count; i++ )
		{
			pVec4->mX = pMat->mX.mX;
			pVec4->mY = pMat->mY.mX;
			pVec4->mZ = pMat->mZ.mX;
			pVec4->mW = pMat->mT.mX;
			pVec4++;

			pVec4->mX = pMat->mX.mY;
			pVec4->mY = pMat->mY.mY;
			pVec4->mZ = pMat->mZ.mY;
			pVec4->mW = pMat->mT.mY;
			pVec4++;

			pVec4->mX = pMat->mX.mZ;
			pVec4->mY = pMat->mY.mZ;
			pVec4->mZ = pMat->mZ.mZ;
			pVec4->mW = pMat->mT.mZ;
			pVec4++;

			pMat++;
		}

		glUniform4fv(pConstant->mLocation, count*3, &pVec4Start->mX);
	}
	else
	{
		glUniformMatrix4fv(pConstant->mLocation, count, GL_FALSE, &pMat->mX.mX);
	}
}

//*****************************************************************************
void VuOglesShaderProgram::setConstantVector4Array(VUHANDLE handle, const VuVector4 *pVec, int count)
{
	const Constant *pConstant = static_cast<const Constant *>(handle);
	glUniform4fv(pConstant->mLocation, count, &pVec->mX);
}

//*****************************************************************************
VuOglesShaderProgram *VuOglesShaderProgram::load(VuBinaryDataReader &reader)
{
	const char *strOgles2VS = reader.readString();
	const char *strOgles2PS = reader.readString();
	const char *strOgles3VS = reader.readString();
	const char *strOgles3PS = reader.readString();

	VuOglesShader *pVertexShader, *pPixelShader;

	if ( strOgles3VS[0] && strOgles3PS[0] && VuOglesGfx::IF()->getGlVersion() >= 3 )
	{
		// load v3 shaders
		pVertexShader = VuOglesShader::compile(GL_VERTEX_SHADER, strOgles3VS, 3);
		pPixelShader = VuOglesShader::compile(GL_FRAGMENT_SHADER, strOgles3PS, 3);
	}
	else if ( strOgles2VS[0] && strOgles2PS[0] )
	{
		// load v2 shaders
		pVertexShader = VuOglesShader::compile(GL_VERTEX_SHADER, strOgles2VS, 2);
		pPixelShader = VuOglesShader::compile(GL_FRAGMENT_SHADER, strOgles2PS, 2);
	}
	else
	{
		VUPRINTF("ERROR: Compatible shader not found!\n");
		return VUNULL;
	}

	return createProgram(pVertexShader, pPixelShader);
}

//*****************************************************************************
VuOglesShaderProgram *VuOglesShaderProgram::createProgram(VuOglesShader *pVertexShader, VuOglesShader *pPixelShader)
{
	// check if the shader program already exists
	for ( OglShaderData::ShaderPrograms::iterator iter = sOglShaderData.mShaderPrograms.begin(); iter != sOglShaderData.mShaderPrograms.end(); iter++ )
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

	// create program
	GLuint program = glCreateProgram();
	VUASSERT(program, "glCreateProgram() failed!");

	// define standard attributes
	for ( int i = 0; i < sStandardAttribCount; i++ )
		glBindAttribLocation(program, sStandardAttribs[i].mIndex, sStandardAttribs[i].mName);

	// link program
	glAttachShader(program, pVertexShader->mGlShader);
	glAttachShader(program, pPixelShader->mGlShader);
	glLinkProgram(program);

	// verify result
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if ( status == GL_FALSE )
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		char *strInfoLog = new char[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, VUNULL, strInfoLog);

		VUPRINTF(strInfoLog);
		VUERROR(strInfoLog);

		delete[] strInfoLog;
		glDeleteProgram(program);

		pVertexShader->removeRef();
		pPixelShader->removeRef();

		return VUNULL;
	}

	// create new shader program
	VuOglesShaderProgram *pShaderProgram = new VuOglesShaderProgram;
	pShaderProgram->mapShaders[VERTEX_SHADER] = pVertexShader;
	pShaderProgram->mapShaders[PIXEL_SHADER] = pPixelShader;
	pShaderProgram->mGlProgram = program;

	// build constant/sampler tables
	pShaderProgram->rebuildTables();

	// add to list
	sOglShaderData.mShaderPrograms.push_back(pShaderProgram);

	return pShaderProgram;
}

//*****************************************************************************
void VuOglesShaderProgram::rebuildTables()
{
	mConstantCount = 0;
	mSamplerCount = 0;
	mAttributeCount = 0;

	glUseProgram(mGlProgram);

	GLint uniformCount;
	glGetProgramiv(mGlProgram, GL_ACTIVE_UNIFORMS, &uniformCount);

	for ( int uniformIndex = 0; uniformIndex < uniformCount; uniformIndex++ )
	{
		GLint size;
		GLenum type;
		char name[256];
		glGetActiveUniform(mGlProgram, uniformIndex, sizeof(name), VUNULL, &size, &type, name);
		GLint location = glGetUniformLocation(mGlProgram, name);

		if ( char *p = strstr(name, "[") )
			*p = '\0';
		
		VuHashedString hashedName(name);

		if ( type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE || type == GL_SAMPLER_2D_ARRAY || type == GL_SAMPLER_2D_ARRAY_SHADOW )
		{
			VUASSERT(mSamplerCount < MAX_SAMPLER_COUNT, "MAX_SAMPLER_COUNT exceeded");

			int samplerIndex = mSamplerCount++;
			glUniform1i(location, samplerIndex);

			Sampler &sampler = maSamplers[samplerIndex];
			sampler.mNameHash = hashedName;
			sampler.mSamplerIndex = samplerIndex;
		}
		else
		{
			VUASSERT(mConstantCount < MAX_CONSTANT_COUNT, "MAX_CONSTANT_COUNT exceeded");

			Constant &constant = maConstants[mConstantCount++];
			constant.mNameHash = hashedName;
			constant.mUniformIndex = uniformIndex;
			constant.mLocation = location;
		}
	}

	GLint attributeCount;
	glGetProgramiv(mGlProgram, GL_ACTIVE_ATTRIBUTES, &attributeCount);
	for ( int attributeIndex = 0; attributeIndex < attributeCount; attributeIndex++ )
	{
		GLint size;
		GLenum type;
		char name[256];
		glGetActiveAttrib(mGlProgram, attributeIndex, sizeof(name), VUNULL, &size, &type, name);

		if ( char *p = strstr(name, "[") )
			*p = '\0';

		int index = -1;
		for ( int i = 0; i < sStandardAttribCount; i++ )
		{
			if ( strcmp(sStandardAttribs[i].mName, name) == 0 )
			{
				index = sStandardAttribs[i].mIndex;
				break;
			}
		}

		VUASSERT(index >= 0, "unknown attribute index");
		VUASSERT(mAttributeCount < MAX_ATTRIBUTE_COUNT, "MAX_ATTRIBUTE_COUNT exceeded");

		maAttributes[mAttributeCount] = index;
		mAttributeCount++;
	}
}

//*****************************************************************************
VuOglesShader::~VuOglesShader()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
		glDeleteShader(mGlShader);

	// erase shader entry
	OglShaderData::Shaders::iterator iter = find(sOglShaderData.mShaders.begin(), sOglShaderData.mShaders.end(), this);
	VUASSERT(iter != sOglShaderData.mShaders.end(), "VuOglesShader::~VuOglesShader() entry not found");
	sOglShaderData.mShaders.erase(iter);
}

//*****************************************************************************
VuOglesShader *VuOglesShader::compile(GLenum type, const char *strData, int glVersion)
{
	// preprocess shader
	std::string shaderText;
	if ( glVersion == 3 )
		shaderText += "#version 300 es\n";
	shaderText += "precision mediump float;\n";
	shaderText += strData;

	// calculate hash
	VUUINT32 hash = VuHash::fnv32String(shaderText.c_str());

	// check if we already have the shader compiled
	for ( std::list<VuOglesShader *>::iterator iter = sOglShaderData.mShaders.begin(); iter != sOglShaderData.mShaders.end(); iter++ )
	{
		if ( (*iter)->mHash == hash )
		{
			(*iter)->addRef();
			return (*iter);
		}
	}

	// create shader
	GLuint shader = glCreateShader(type);
	//GLenum error = glGetError();
	VUASSERT(shader, "glCreateShader() failed!");

	// compile shader
	const char *pShaderText = shaderText.c_str();
	glShaderSource(shader, 1, &pShaderText, VUNULL);
	glCompileShader(shader);

	// verify result
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if ( status == GL_FALSE )
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char *strInfoLog = new char[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, VUNULL, strInfoLog);

		VUERROR("%s shader:\n%s", type == GL_VERTEX_SHADER ? "vertex" : "fragment", strInfoLog);

		delete[] strInfoLog;
		glDeleteShader(shader);

		return VUNULL;
	}

	// create shader
	VuOglesShader *pShader = new VuOglesShader;
	pShader->mHash = hash;
	pShader->mGlShader = shader;
	pShader->mGlType = type;

	// add to list
	sOglShaderData.mShaders.push_back(pShader);

	return pShader;
}
