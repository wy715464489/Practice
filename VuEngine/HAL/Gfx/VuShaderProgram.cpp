//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Shader Program interface class.
// 
//*****************************************************************************

#include "VuShaderProgram.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuHash.h"

#if defined VUWIN32 && !VU_DISABLE_BAKING
	#include <d3dcompiler.h>
	#include "VuEngine/HAL/Gfx/D3d11/VuD3d11ShaderProgram.h"

    #include <sstream>
/*
	#include <shader/compiler.h>
	#pragma comment(lib, "libSceShaderCompiler.lib")*/
#endif


//*****************************************************************************
bool VuShaderProgram::bake(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer)
{
	// texture baking is only supported on Win32 for now
#if defined VUWIN32 && !VU_DISABLE_BAKING
	if ( platform == "Win32" || platform == "Windows" || platform == "Xb1")
	{
		return bakeD3d11(platform, data, lodData, pMacros, writer);
	}
	else if ( platform == "Android" || platform == "BB10" )
	{
		return bakeOgles(platform, data, lodData, pMacros, writer);
	}
	else if ( platform == "Ios" )
	{
		return bakeIos(data, lodData, pMacros, writer);
	}
	else if (platform == "Ps4")
	{
		return bakePssl(platform, data, lodData, pMacros, writer);
	}
	else
	{
		VUASSERT(0, "VuShaderProgram::bake() unsupported platform");
	}
#endif

	return false;
}

//*****************************************************************************
void VuShaderProgram::addMacros(std::string &shaderText, const Macros *pMacros)
{
	if ( pMacros )
	{
		for ( VuShaderProgram::Macros::const_iterator iter = pMacros->begin(); iter != pMacros->end(); iter++ )
		{
			char macroDef[256];
			VU_SPRINTF(macroDef, sizeof(macroDef), "#define %s %s\n", iter->first.c_str(), iter->second.c_str());
			shaderText += macroDef;
		}
	}
}

#if defined VUWIN32 && !VU_DISABLE_BAKING

//*****************************************************************************
bool VuShaderProgram::bakeOgles(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer)
{
	if ( data.hasMember("Ogles2") )
	{
		if ( !bakeOglesShader(platform, data["Ogles2"]["VertexShader"].asString(), pMacros, writer) )
			return false;

		if ( !bakeOglesShader(platform, data["Ogles2"]["PixelShader"].asString(), pMacros, writer) )
			return false;
	}
	else
	{
		writer.writeString("");
		writer.writeString("");
	}

	if ( platform == "Android" && data.hasMember("Ogles3") )
	{
		if ( !bakeOglesShader(platform, data["Ogles3"]["VertexShader"].asString(), pMacros, writer) )
			return false;

		if ( !bakeOglesShader(platform, data["Ogles3"]["PixelShader"].asString(), pMacros, writer) )
			return false;
	}
	else
	{
		writer.writeString("");
		writer.writeString("");
	}

	return true;
}

//*****************************************************************************
bool VuShaderProgram::bakeOglesShader(const std::string &platform, const std::string &data, const Macros *pMacros, VuBinaryDataWriter writer)
{
	// preprocess shader
	std::string shaderText;
	if ( platform == "Android" )
		shaderText += "#define VUANDROID\n";
	else if ( platform == "Ios" )
		shaderText += "#define VUIOS\n";
	else if ( platform == "BB10" )
		shaderText += "#define VUBB10\n";
	VuShaderProgram::addMacros(shaderText, pMacros);
	shaderText += data;

	ID3DBlob *pShaderText = VUNULL;
	if (D3DPreprocess(shaderText.c_str(), shaderText.length(), NULL, NULL, NULL, &pShaderText, NULL) != S_OK)
		return false;

	const char *rawText = static_cast<const char *>(pShaderText->GetBufferPointer());
	
	// remove whitespace
	std::string text;
	int wasAlphaNumeric = false;
	while ( char c = rawText[0] )
	{
		// remove #line macros
		if ( strncmp(rawText, "#line", 5) == 0 )
		{
			rawText = strstr(rawText, "\n");
			wasAlphaNumeric = false;
		}
		else
		{
			int isSpace = isspace(c);
			int isAlphaNumeric = isalnum(c);
			int nextAlphaNumeric = isalnum(rawText[1]);

			if ( !isSpace || (wasAlphaNumeric && nextAlphaNumeric) )
				text.push_back(c);

			wasAlphaNumeric = isAlphaNumeric;
		}

		rawText++;
	}

	// write
	writer.writeString(text);

	pShaderText->Release();

	return true;
}

//*****************************************************************************
bool VuShaderProgram::bakeD3d11(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer)
{
	std::string shaderModel = "4_0";
	if (platform == "Win32")
	{
		shaderModel = "4_0_level_9_1";
		data["Win32ShaderModel"].getValue(shaderModel);
		lodData["Win32ShaderModel"].getValue(shaderModel);
	}
	else if (platform == "Windows")
	{
		shaderModel = "4_0_level_9_1";
		data["WindowsShaderModel"].getValue(shaderModel);
		lodData["WindowsShaderModel"].getValue(shaderModel);
	}
	else if (platform == "Xb1")
	{
		shaderModel = "5_0";
	}

	std::string vsProfile = std::string("vs_") + shaderModel;
	std::string psProfile = std::string("ps_") + shaderModel;

	// vertex shader
	VuArray<VUBYTE> vertexShaderData(0);
	VuArray<VUINT> vertexShaderConstantBufferSizes(0);
	if ( !bakeD3d11Shader(platform, vsProfile.c_str(), data["D3d11"]["VertexShader"].asString(), pMacros, vertexShaderData, vertexShaderConstantBufferSizes) )
		return false;
	VUUINT32 vertexShaderHash = VuHash::fnv32(&vertexShaderData.begin(), vertexShaderData.size());

	// pixel shader
	VuArray<VUBYTE> pixelShaderData(0);
	VuArray<VUINT> pixelShaderConstantBufferSizes(0);
	if ( !bakeD3d11Shader(platform, psProfile.c_str(), data["D3d11"]["PixelShader"].asString(), pMacros, pixelShaderData, pixelShaderConstantBufferSizes) )
		return false;
	VUUINT32 pixelShaderHash = VuHash::fnv32(&pixelShaderData.begin(), pixelShaderData.size());

	// table
	VuArray<VUBYTE> reflectionData(0);
	bakeD3d11Reflection(platform, vertexShaderData, pixelShaderData, reflectionData);

	// write
	writer.writeValue(vertexShaderHash);
	writer.writeArray(vertexShaderConstantBufferSizes);
	writer.writeArray(vertexShaderData);

	writer.writeValue(pixelShaderHash);
	writer.writeArray(pixelShaderConstantBufferSizes);
	writer.writeArray(pixelShaderData);

	writer.writeArray(reflectionData);

	return true;
}

//*****************************************************************************
bool VuShaderProgram::bakeD3d11Shader(const std::string &platform, const char *profile, const std::string &data, const Macros *pMacros, VuArray<VUBYTE> &shaderData, VuArray<VUINT> &constantBufferSizes)
{
	// preprocess shader
	std::string shaderText;
	if (platform == "Win32")
		shaderText += "#define VUWIN32\n";
	else if (platform == "Windows")
		shaderText += "#define VUWINDOWS\n";
	else if (platform == "Xb1")
		shaderText += "#define VUXB1\n";
	VuShaderProgram::addMacros(shaderText, pMacros);
	shaderText += data;

	// compile shader
	std::string errors;
	if ( !VuD3d11Shader::compile(profile, shaderText, shaderData, constantBufferSizes, errors) )
		return VUWARNING("profile %s:\n%s", profile, errors.c_str());

	return true;
}

//*****************************************************************************
bool VuShaderProgram::bakeD3d11Reflection(const std::string &platform, const VuArray<VUBYTE> &vertexShaderData, const VuArray<VUBYTE> &pixelShaderData, VuArray<VUBYTE> &reflectionData)
{
	// reflect
	if (!VuD3d11Shader::reflect(vertexShaderData, pixelShaderData, reflectionData))
		return VUWARNING("Failed to build shader table");

	return true;
}

//*****************************************************************************
bool VuShaderProgram::bakeIos(const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer)
{
	// first bake ogles shader
	if ( !bakeOgles("Ios", data, lodData, pMacros, writer) )
		return false;

	// then bake metal shader

	// add macros
	std::string shaderText;
	VuShaderProgram::addMacros(shaderText, pMacros);
	shaderText += data["Metal"].asString();

	ID3DBlob *pShaderText = VUNULL;
	if (D3DPreprocess(shaderText.c_str(), shaderText.length(), NULL, NULL, NULL, &pShaderText, NULL) != S_OK)
		return false;

	const char *rawText = static_cast<const char *>(pShaderText->GetBufferPointer());
	
	// remove #line macros
	std::string text;
	while ( char c = rawText[0] )
	{
		if ( strncmp(rawText, "#line", 5) == 0 )
			rawText = strstr(rawText, "\n");
		else
			text.push_back(c);

		rawText++;
	}

	// write
	writer.writeString(text);

	pShaderText->Release();

	return true;
}

//*****************************************************************************
bool VuShaderProgram::bakePssl(const std::string &platform, const VuJsonContainer &data, const VuJsonContainer &lodData, const Macros *pMacros, VuBinaryDataWriter &writer)
{
	if ( !bakePsslShader(platform, VuShaderProgram::VERTEX_SHADER, data["Pssl"]["VertexShader"].asString(), pMacros, writer) )
		return false;

	if ( !bakePsslShader(platform, VuShaderProgram::PIXEL_SHADER, data["Pssl"]["PixelShader"].asString(), pMacros, writer) )
		return false;

	return true;
}

//*****************************************************************************
bool VuShaderProgram::bakePsslShader(const std::string &platform, eShader shader, const std::string &data, const Macros *pMacros, VuBinaryDataWriter writer)
{
	/*struct VuPs4ShaderCompileContext
	{
		static sce::Shader::Compiler::SourceFile *openFile(
			const char *fileName,
			const sce::Shader::Compiler::SourceLocation *includedFrom,
			const sce::Shader::Compiler::Options *compileOptions,
			const char **errorString)
		{
			return (sce::Shader::Compiler::SourceFile *)compileOptions->userData;
		}
	};

		// preprocess shader
	std::string shaderText;
	if ( platform == "Ps4" )
		shaderText += "#define VUPS4\n";
	VuShaderProgram::addMacros(shaderText, pMacros);
	shaderText += data;

	sce::Shader::Compiler::Options options;
	sce::Shader::Compiler::initializeOptions(&options);

	sce::Shader::Compiler::CallbackList callbacks;
	sce::Shader::Compiler::initializeCallbackList(&callbacks, sce::Shader::Compiler::kCallbackDefaultsTrivial);

	sce::Shader::Compiler::SourceFile sourceFile;
	sourceFile.fileName = VUNULL;
	sourceFile.text = shaderText.c_str();
	sourceFile.size = (int)shaderText.length();

	options.userData = &sourceFile;
	callbacks.openFile = VuPs4ShaderCompileContext::openFile;

	const char *strProfile = "";
	if ( shader == VERTEX_SHADER )
	{
		options.targetProfile = sce::Shader::Compiler::kTargetProfileVsVs;
		strProfile = "VertexShader";
	}
	else if ( shader == PIXEL_SHADER )
	{
		options.targetProfile = sce::Shader::Compiler::kTargetProfilePs;
		strProfile = "PixelShader";
	}

	const sce::Shader::Compiler::Output *output = sce::Shader::Compiler::run(&options, &callbacks);

	for ( int i = 0; i < output->diagnosticCount; i++ )
	{
		const sce::Shader::Compiler::DiagnosticMessage &diagnostic = output->diagnostics[i];

		// find line number
		std::string line;
		if ( diagnostic.location )
		{
			std::istringstream stream(shaderText);
			std::getline(stream, line);
			for ( int i = 1; i < (int)diagnostic.location->lineNumber; i++ )
				std::getline(stream, line);
			while ( line.length() && (line[0] == ' ' || line[0] == '\t') )
				line = line.substr(1);
			line = ">> " + line + "\n";
		}

		if ( diagnostic.level == sce::Shader::Compiler::kDiagnosticLevelInfo )
		{
			VUPRINTF("PSSL Info (%s): %s\n%s", strProfile, diagnostic.message, line.c_str());
		}
		else if ( diagnostic.level == sce::Shader::Compiler::kDiagnosticLevelWarning )
		{
			VUPRINTF("PSSL Warning (%s): %s\n%s", strProfile, diagnostic.message, line.c_str());
		}
		else if ( diagnostic.level == sce::Shader::Compiler::kDiagnosticLevelError )
		{
			VUPRINTF("PSSL Error (%s): %s\n%s", strProfile, diagnostic.message, line.c_str());
			return false;
		}
	}

	VUUINT32 hash;
	hash = VuHash::fnv32(output->programData, output->programSize);

	writer.writeValue(hash);
	writer.writeValue(output->programSize);
	writer.writeData(output->programData, output->programSize);

	sce::Shader::Compiler::destroyOutput(output);
*/

	return true;
}

#endif // VUWIN32
