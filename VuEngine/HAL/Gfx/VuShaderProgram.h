//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Shader Program interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"

class VuMatrix;
class VuVector2;
class VuVector3;
class VuVector4;
class VuColor;
class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;


class VuShaderProgram : public VuRefObj
{
public:
	enum eShader { VERTEX_SHADER, PIXEL_SHADER, NUM_SHADER_TYPES };

	typedef std::map<std::string, std::string> Macros;

	virtual VUHANDLE		getConstantByName(const char *strName) const = 0;
	virtual int				getSamplerIndexByName(const char *strName) const = 0;

	virtual void			setConstantFloat(VUHANDLE handle, float fValue) = 0;
	virtual void			setConstantFloat3(VUHANDLE handle, const float *pfValues) = 0;
	virtual void			setConstantFloat4(VUHANDLE handle, const float *pfValues) = 0;
	virtual void			setConstantInt(VUHANDLE handle, int iValue) = 0;
	virtual void			setConstantMatrix(VUHANDLE handle, const VuMatrix &mat) = 0;
	virtual void			setConstantVector2(VUHANDLE handle, const VuVector2 &vec) = 0;
	virtual void			setConstantVector3(VUHANDLE handle, const VuVector3 &vec) = 0;
	virtual void			setConstantVector4(VUHANDLE handle, const VuVector4 &vec) = 0;
	virtual void			setConstantColor3(VUHANDLE handle, const VuColor &color) = 0;
	virtual void			setConstantColor4(VUHANDLE handle, const VuColor &color) = 0;

	virtual void			setConstantFloatArray(VUHANDLE handle, const float *pfValue, int count) = 0;
	virtual void			setConstantIntArray(VUHANDLE handle, const int *piValue, int count) = 0;
	virtual void			setConstantMatrixArray(VUHANDLE handle, const VuMatrix *pMat, int count, bool skinning) = 0;
	virtual void			setConstantVector4Array(VUHANDLE handle, const VuVector4 *pVec, int count) = 0;

	static bool				bake(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer);
	static void				addMacros(std::string &shaderText, const Macros *pMacros);

protected:
	static bool				bakeOgles(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer);
	static bool				bakeOglesShader(const std::string &platform, const std::string &data, const Macros *pMacros, VuBinaryDataWriter writer);
	static bool				bakeD3d11(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer);
	static bool				bakeD3d11Shader(const std::string &platform, const char *profile, const std::string &data, const Macros *pMacros, VuArray<VUBYTE> &shaderData, VuArray<VUINT> &constantBufferSizes);
	static bool				bakeD3d11Reflection(const std::string &platform, const VuArray<VUBYTE> &vertexShaderData, const VuArray<VUBYTE> &pixelShaderData, VuArray<VUBYTE> &reflectionData);
	static bool				bakeIos(const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer);
	static bool				bakePssl(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer);
	static bool				bakePsslShader(const std::string &platform, eShader shader, const std::string &data, const Macros *pMacros, VuBinaryDataWriter writer);
};
